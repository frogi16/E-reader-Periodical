#include "FeedsDatabase.h"
#include <fstream>
#include <iostream>

FeedsDatabase::FeedsDatabase() noexcept
{
	loadKeywords();
	loadDatabase();
}

std::vector<ArticleRSS> FeedsDatabase::updateFeed(std::string feedLink, pugi::xml_node root)
{
	std::vector<ArticleRSS> newItems;

	try
	{
		auto search = searchForKeyword(root, pubDateKeyword, 1);			//selectDataByName returns vector of all matching nodes, but often we search for unique node (or specified number of them),
																			//therefore use of searchForKeyword function. It allows to use defined lists of alternative keywords, which are used in order
																			//of pushing to container, only if previous search doesn't return expected number of results.
		std::string lastBuild = search[0].child_value();

		if (lastBuild > feeds[feedLink].lastBuildTime)						//if feed has been updated since last check (if feed can't be found in map result is always true)
		{
			std::cout << "Updating " << feedLink << std::endl;

			auto itemNodes = searchForKeyword(root, itemKeyword, 1);

			FeedData& feed = feeds[feedLink];								//reference to feeds[feedLink]
			feed.link = feedLink;											//assigning link to feed. It will have no effect if feed was already used.
			feed.lastBuildTime = lastBuild;

			for (auto& itemNode : itemNodes)
			{
				auto search = searchForKeyword(itemNode, linkKeyword, 1, true);
				std::string itemLink = search[0].child_value();

				if (itemLink.find("http") == std::string::npos)		
																			//if there is no "http" (or "https") in link site probably uses some weird structure - for example blogspot.com shows something
																			//like this: "<id>tag:blogger.com,1999:blog-2442024767695998453.post-2644038318746445557</id>" and valid link is hidden in here:
																			//"<link rel='alternate' type='text/html' href='https://czajniczek-pana-russella.blogspot.com/2018/10/gdzie-sie-podziali-kanibale.html' title='Gdzie siê podziali kanibale?'/>"
																			//right now I encountered only one problematic RSS generator, if I find more, more general approach should be developed
				{
					itemLink = getLinkFromAlternateHref(itemNode);
				}

				if (!isItemSaved(feed.items, itemLink))						//if this item was already saved there is no need to do anything and we can just go to the next one
				{
					auto item = createItem(itemLink, itemNode);
					newItems.push_back(item);
					feed.items.push_back(item);
				}
			}
		}

		std::cout << "Found " << newItems.size() << " new articles" << std::endl;
	}
	catch (const std::string& e)
	{
		std::cout << "Unexpected behavior:\n" << e << "\nUpdating this feed failed.\n";
	}
	
	return newItems;
}

void FeedsDatabase::saveDatabase()
{
	/*
		<header>
		</header>

		<feeds>
			<feed>
				<link></link>
				<item>
					<title></title>
					<link></link>
					<pubDate></pubDate>
					<description></description>
				</item>
			</feed>
		</feeds>
	*/

	pugi::xml_document doc;
	doc.append_child("header");
	auto feedsNode = doc.append_child("feeds");

	for (auto& feed : feeds)
	{
		auto feedNode = feedsNode.append_child("feed");
		auto linkNode = feedNode.append_child("link").append_child(pugi::node_pcdata);
		linkNode.set_value(feed.first.c_str());

		for (auto& item : feed.second.items)
		{
			auto itemNode = feedNode.append_child("item");

			auto titleNode = itemNode.append_child("title").append_child(pugi::node_pcdata);
			titleNode.set_value(item.title.c_str());

			auto linkNode = itemNode.append_child("link").append_child(pugi::node_pcdata);
			linkNode.set_value(item.link.c_str());

			auto pubDateNode = itemNode.append_child("pubDate").append_child(pugi::node_pcdata);
			pubDateNode.set_value(item.pubDate.c_str());

			auto descriptionNode = itemNode.append_child("description").append_child(pugi::node_pcdata);
			descriptionNode.set_value(item.description.c_str());
		}
	}

	doc.save_file("database.xml");
}


FeedsDatabase::~FeedsDatabase()
{
}

void FeedsDatabase::loadDatabase()
{
	pugi::xml_document doc;
	doc.load_file("database.xml");
	auto node = doc.child("feeds");

	for (auto& feed : node.children())
	{
		std::string feedLink = feed.child("link").child_value();
		feeds[feedLink] = FeedData();
		feeds[feedLink].link = feedLink;

		for (auto i = std::next(feed.children().begin()); i != feed.children().end(); i++)		//TODO: have to use next because first node is a link to a feed. Can be improved.
		{
			pugi::xml_node item = (*i);
			ArticleRSS target;
			target.link = item.child("link").child_value();
			target.title = item.child("title").child_value();
			target.description = item.child("description").child_value();
			target.pubDate = item.child("pubDate").child_value();
			feeds[feedLink].items.push_back(target);
		}
	}
}

void FeedsDatabase::loadKeywords()
{
	pugi::xml_document doc;
	doc.load_file("keywords.xml");
	
	for (auto& group : doc.children())
	{
		std::string mainKeyword = group.child("main_keyword").first_child().value();
		Keyword * keywords;

		if (mainKeyword == "pubDate")
		{
			keywords = &pubDateKeyword;
		}
		else if (mainKeyword == "item")
		{
			keywords = &itemKeyword;
		}
		else if (mainKeyword == "link")
		{
			keywords = &linkKeyword;
		}
		else if (mainKeyword == "description")
		{
			keywords = &descriptionKeyword;
		}
		else if (mainKeyword == "title")
		{
			keywords = &titleKeyword;
		}
		else
		{
			throw std::string("\"" + mainKeyword + "\" keywords group not found. Program will continue execution, but you are advised to check keywords database. Further stability is not guaranteed.\n");
		}

		keywords->alternatives.push_back(mainKeyword);

		for (auto& alternative : group.child("alternatives").children())
		{
			keywords->alternatives.push_back(alternative.child_value());
		}
		
	}
}

bool FeedsDatabase::isFeedSaved(std::string feedLink)
{
	return (feeds.find(feedLink) != feeds.end());
}

bool FeedsDatabase::isItemSaved(const std::vector<ArticleRSS> & savedItems, std::string itemLink)
{
	return (std::find(savedItems.begin(), savedItems.end(), itemLink) != savedItems.end());
}

std::vector<pugi::xml_node> FeedsDatabase::searchForKeyword(pugi::xml_node root, Keyword keyword, size_t minimalResultNumber, bool checkForChild)
{
	bool noChildren = false;											//flag set when results were rejected because of lack of children. Used to determine which error should be thrown at the end of function

	for (auto& alternative : keyword.alternatives)
	{
		std::vector<pugi::xml_node> result = dataSelecter.selectNodesByName(root, alternative);

		if (result.size() >= minimalResultNumber)
		{
			if (checkForChild)
			{
				if (result[0].first_child())
				{
					return result;
				}
				else
				{
					noChildren = true;
				}
			}
			else
			{
				return result;
			}


			/*if(!checkForChild || result[0].first_child())				//notation above is basically an implication: checkForChild => result[0].first_child() and could be written in this way. Probably faster, but harder to read
				return result;*/
		}
	}

	if(noChildren)
		throw std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned results without children.");
	else
		throw std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned less than expected " + std::to_string(minimalResultNumber) + " results.");
}

ArticleRSS FeedsDatabase::createItem(std::string itemLink, pugi::xml_node itemNode)
{
	ArticleRSS newItem;
	newItem.link = itemLink;

	auto titleNode = searchForKeyword(itemNode, titleKeyword, 1);
	newItem.title = titleNode[0].child_value();

	auto descriptionNode = searchForKeyword(itemNode, descriptionKeyword, 1);
	newItem.description = descriptionNode[0].child_value();

	auto pubDateNode = searchForKeyword(itemNode, pubDateKeyword, 1);
	newItem.pubDate = pubDateNode[0].child_value();

	return newItem;
}

std::string FeedsDatabase::getLinkFromAlternateHref(pugi::xml_node root)
{
	auto links = dataSelecter.selectNodesByAttribute(root, "rel", "alternate");

	if (links.size())
	{
		std::string link = links[0].attribute("href").value();
		return link;
	}
	else
	{
		throw std::string("Haven't found link in node with rel=\'alternate\' attribute!\n");
	}

	return std::string();
}
