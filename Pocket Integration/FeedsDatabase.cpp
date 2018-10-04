#include "FeedsDatabase.h"
#include <fstream>
#include <iostream>

FeedsDatabase::FeedsDatabase()
{
	loadDatabase();
}

std::vector<std::string> FeedsDatabase::updateFeed(std::string feedLink, pugi::xml_node root)
{
	std::vector<std::string> newItemLinks;
	std::vector<pugi::xml_node> lastBuildNode = scraper.selectDataByName(root, "lastBuildDate");
	
	if (lastBuildNode.size() == 0)								//alternative keyword
	{
		lastBuildNode = scraper.selectDataByName(root, "updated");
	}
	std::string lastBuild;
	lastBuild = lastBuildNode[0].first_child().value();			//selectDataByName returns vector of all matching nodes, but often we search for unique node, therefore use of [0]
																//I thought about checking if size of vector is equal 1 and decided that this would be too annoying.
	auto test = feeds[feedLink].lastBuildTime;

	if (lastBuild > feeds[feedLink].lastBuildTime)				//if feed has been updated since last check (if feed can't be found in map result is always true)
	{
		std::cout << "Updating " << feedLink << std::endl;

		auto items = scraper.selectDataByName(root, "item");

		if (items.size() == 0)
		{
			items = scraper.selectDataByName(root, "entry");
		}

		if (!isFeedSaved(feedLink))										//if feed is new we have to create new FeedData and assign the link
		{
			FeedData data;
			data.link = feedLink;
			feeds[feedLink] = data;
		}

		FeedData& feed = feeds[feedLink];						//reference to feeds[feedLink]
		feed.lastBuildTime = lastBuild;

		for (auto& item : items)
		{
			auto linkNode = scraper.selectDataByName(item, "link");

			if (!linkNode[0].first_child())
			{
				linkNode = scraper.selectDataByName(item, "id");
			}


			std::string itemLink = linkNode[0].first_child().value();

			if (!isItemSaved(feed.items, itemLink))
			{
				Item newItem;

				newItem.link = itemLink;
				newItemLinks.push_back(itemLink);

				auto titleNode = scraper.selectDataByName(item, "title");
				std::string itemTitle = titleNode[0].first_child().value();
				newItem.title = itemTitle;

				auto descriptionNode = scraper.selectDataByName(item, "description");
				if (descriptionNode.size() == 0)
				{
					descriptionNode= scraper.selectDataByName(item, "content");
				}
				std::string itemDescription = descriptionNode[0].first_child().value();
				newItem.description = itemDescription;

				auto pubDateNode = scraper.selectDataByName(item, "pubDate");
				if (pubDateNode.size() == 0)
				{
					pubDateNode = scraper.selectDataByName(item, "updated");
				}
				std::string itemPubDate = pubDateNode[0].first_child().value();
				newItem.pubDate = itemPubDate;

				feed.items.push_back(newItem);
			}
		}
	}

	std::cout << "Found " << newItemLinks.size() << " new articles" << std::endl;
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

		for (auto i = std::next(feed.children().begin()); i != feed.children().end(); i++)		//TODO: next because first node is link to feed. Can be improved.
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
