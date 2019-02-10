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

		if (isFeedChanged(feedLink, lastBuild))								//if feed has been updated since last check (if feed can't be found in map result is always true)
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
																			//right now I encountered only one problematic RSS generator, if more is found, more general approach should be developed
				{
					itemLink = getLinkFromAlternateHref(itemNode);
				}

				if (!isItemSaved(feed.items, itemLink))						//if this item was already saved there is no need to do anything and we can just go to the next one
				{
					auto item = createItem(itemLink, itemNode);
					newItems.push_back(item);
					feed.items.push_back(item);
				}

				std::cout << "Found " << newItems.size() << " new articles" << std::endl;
			}
		}
		else
		{
			std::cout << "Processing the site content was skipped because site hasn't been updated since last check." << std::endl;
		}
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
			Left for future improvements and upgrades. Versions, compatibility, additional info etc.
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

		auto updateNode = feedNode.append_child("lastUpdate").append_child(pugi::node_pcdata);
		updateNode.set_value(feed.second.lastBuildTime.c_str());

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

		std::string lastUpdate = feed.child("lastUpdate").child_value();
		feeds[feedLink].lastBuildTime = lastUpdate;

		for (auto i = std::next(feed.children().begin()); i != feed.children().end(); i++)		//TODO: have to use next because first node is a link to a feed. Can be improved.
		{
			try
			{
				pugi::xml_node item = (*i);
				ArticleRSS target;
				target.link = item.child("link").child_value();
				target.title = item.child("title").child_value();
				target.description = item.child("description").child_value();
				target.pubDate = item.child("pubDate").child_value();
				feeds[feedLink].items.push_back(target);
			}
			catch (const std::exception& e)
			{
				throw std::exception(std::string("Loading news from database failed while loading " + feedLink + ". Data is probably corrupted.").c_str());
			}
		}
	}
}

void FeedsDatabase::loadKeywords()
{
	pugi::xml_document doc;
	doc.load_file("keywords.xml");

	for (auto& group : doc.children())
	{
		std::string mainKeywordString = group.child("main_keyword").first_child().value();	//main keyword identifies predefined keywords group which is implemented by given node
		auto mainKeywordType = keywords.mapStringToKeywordType.at(mainKeywordString);
		Keyword * keywords;

		switch (mainKeywordType)
		{
		case KeywordType::pubDate:
			keywords = &pubDateKeyword;
			break;
		case KeywordType::item:
			keywords = &itemKeyword;
			break;
		case KeywordType::link:
			keywords = &linkKeyword;
			break;
		case KeywordType::description:
			keywords = &descriptionKeyword;
			break;
		case KeywordType::title:
			keywords = &titleKeyword;
			break;
		default:
			throw std::string("\"" + mainKeywordString + "\" keywords group not found. Program will continue execution, but you are advised to check keywords database. Further stability is not guaranteed.\n");
			break;
		}

		keywords->alternatives.push_back(mainKeywordString);

		for (auto& alternative : group.child("alternatives").children())
		{
			keywords->alternatives.push_back(alternative.child_value());
		}

	}
}

bool FeedsDatabase::isFeedSaved(const std::string & feedLink) const
{
	return (feeds.find(feedLink) != feeds.end());
}

bool FeedsDatabase::isFeedChanged(const std::string & feedLink, const std::string & buildTime) const
{
	std::tm oldTime;

	auto searchedFeed = feeds.find(feedLink);
	if (searchedFeed == feeds.end() || searchedFeed->second.lastBuildTime.empty())		//newly added feed isn't saved in feeds map, but of course should be updated. Similarly if because of some mystic reasons (bugs, failed parse, unexpected exceptions etc.) date string is empty
		return true;
	else
	{
		std::time_t oldTime_t = parseDatetime(searchedFeed->second.lastBuildTime);
		std::time_t newTime_t = parseDatetime(buildTime);

		if (oldTime_t == -1 || newTime_t == -1)											//last resort of error-proofing is check if time_t is equal -1. It may happen if date couldn't be parsed at all. -1 seems to be treated by difftime() as special case and it returns 0, so boolean result has to be handled manually
			return true;

		return (difftime(oldTime_t, newTime_t) > 0);									//true if newTime is greater then oldTime
	}
}

bool FeedsDatabase::isItemSaved(const std::vector<ArticleRSS> & savedItems, const std::string & itemLink) const
{
	return (std::find(savedItems.begin(), savedItems.end(), itemLink) != savedItems.end());
}

std::time_t FeedsDatabase::parseDatetime(const std::string & datetime) const
{
	return mktime(&datetimeParser.parse(datetime));
}

std::vector<pugi::xml_node> FeedsDatabase::searchForKeyword(const pugi::xml_node & root, const Keyword & keyword, size_t minimalResultNumber, bool checkForChild)
{
	bool noChildren = false;			//flag set when results were rejected because of lack of children. Used to determine which error should be thrown at the end of function

	for (auto& alternative : keyword.alternatives)
	{
		std::vector<pugi::xml_node> result = dataSelecter.selectNodesByName(root, alternative);

		if (result.size() >= minimalResultNumber && !checkForChild)
			return result;
		else if (result.size() >= minimalResultNumber && checkForChild && result[0].first_child())
			return result;
		else if (result.size() >= minimalResultNumber && checkForChild && !result[0].first_child())
			noChildren = true;
	}

	if (noChildren)
		throw std::exception(std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned results without children.").c_str());
	else
		throw std::exception(std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned less than expected " + std::to_string(minimalResultNumber) + " results.").c_str());
}

ArticleRSS FeedsDatabase::createItem(std::string itemLink, pugi::xml_node itemNode)
{
	ArticleRSS newItem;
	newItem.link = itemLink;

	auto titleNode = searchForKeyword(itemNode, titleKeyword, 1, true);
	newItem.title = titleNode[0].child_value();

	auto descriptionNode = searchForKeyword(itemNode, descriptionKeyword, 1, true);
	newItem.description = descriptionNode[0].child_value();

	auto pubDateNode = searchForKeyword(itemNode, pubDateKeyword, 1, true);
	newItem.pubDate = pubDateNode[0].child_value();

	return newItem;
}

std::string FeedsDatabase::getLinkFromAlternateHref(pugi::xml_node root)
{
	auto links = dataSelecter.selectNodesByAttribute(root, "rel", "alternate");

	if (links.size())
		return links[0].attribute("href").value();
	else
		throw std::string("Haven't found link in node with rel=\'alternate\' attribute!\n");
}
