#include "FeedsDatabase.h"
#include <fstream>
#include <iostream>
//lastBuildDate, item, link, description;
FeedsDatabase::FeedsDatabase()
{
	lastBuildDateKeyword.alternatives.push_back("lastBuildDate");			//defining lists of keywords we will be searching for. Keywords will be used one by one, only when needed.
	lastBuildDateKeyword.alternatives.push_back("updated");

	pubDateKeyword.alternatives.push_back("pubDate");
	pubDateKeyword.alternatives.push_back("updated");

	itemKeyword.alternatives.push_back("item");
	itemKeyword.alternatives.push_back("entry");

	linkKeyword.alternatives.push_back("link");
	linkKeyword.alternatives.push_back("id");

	descriptionKeyword.alternatives.push_back("description");
	descriptionKeyword.alternatives.push_back("content");

	titleKeyword.alternatives.push_back("title");

	loadDatabase();
}

std::vector<std::string> FeedsDatabase::updateFeed(std::string feedLink, pugi::xml_node root)
{
	std::vector<std::string> newItemLinks;

	try
	{
		auto search = searchForKeyword(root, lastBuildDateKeyword, 1);		//selectDataByName returns vector of all matching nodes, but often we search for unique node (or specified number of them),
																			//therefore use of searchForKeyword function. It allows to use defined lists of alternative keywords, which are used in order
																			//of pushing to container, only if previous search doesn't return expected number of results.
		std::string lastBuild = search[0].first_child().value();

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
				std::string itemLink = search[0].first_child().value();

				if (!isItemSaved(feed.items, itemLink))						//if this item was already saved there is no need to do anything and we can just go to the next one
				{
					newItemLinks.push_back(itemLink);
					feed.items.push_back(createItem(itemLink, itemNode));
				}
			}
		}

		std::cout << "Found " << newItemLinks.size() << " new articles" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "Unexpected behavior:\n" << e.what() << "\nUpdating this feed failed.\n";
	}
	
	return newItemLinks;
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
		std::string feedLink = feed.child("link").first_child().value();
		feeds[feedLink] = FeedData();
		feeds[feedLink].link = feedLink;

		for (auto i = std::next(feed.children().begin()); i != feed.children().end(); i++)		//TODO: have to use next because first node is a link to a feed. Can be improved.
		{
			pugi::xml_node item = (*i);
			Item target;
			target.link = item.child("link").first_child().value();
			target.title = item.child("title").first_child().value();
			target.description = item.child("description").first_child().value();
			target.pubDate = item.child("pubDate").first_child().value();
			feeds[feedLink].items.push_back(target);
		}
	}
}

bool FeedsDatabase::isFeedSaved(std::string feedLink)
{
	return (feeds.find(feedLink) != feeds.end());
}

bool FeedsDatabase::isItemSaved(std::vector<Item> savedItems, std::string itemLink)
{
	return (std::find(savedItems.begin(), savedItems.end(), itemLink) != savedItems.end());
}

std::vector<pugi::xml_node> FeedsDatabase::searchForKeyword(pugi::xml_node root, Keyword keyword, size_t minimalResultNumber, bool checkForChild)
{
	for (auto& alternative : keyword.alternatives)
	{
		std::vector<pugi::xml_node> result = scraper.selectDataByName(root, alternative);

		if (result.size() >= minimalResultNumber)
		{
			if (checkForChild)
			{
				if (result[0].first_child())
				{
					return result;
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

	throw std::string("Filtering nodes for " + keyword.mainKeyword() + " returned less than expected " + std::to_string(minimalResultNumber) + " results.");
}

Item FeedsDatabase::createItem(std::string itemLink, pugi::xml_node itemNode)
{
	Item newItem;
	newItem.link = itemLink;

	auto titleNode = searchForKeyword(itemNode, titleKeyword, 1);
	newItem.title = titleNode[0].first_child().value();

	auto descriptionNode = searchForKeyword(itemNode, descriptionKeyword, 1);
	newItem.description = descriptionNode[0].first_child().value();

	auto pubDateNode = searchForKeyword(itemNode, pubDateKeyword, 1);
	newItem.pubDate = pubDateNode[0].first_child().value();

	return newItem;
}
