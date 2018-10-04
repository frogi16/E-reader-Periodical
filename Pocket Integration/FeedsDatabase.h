#pragma once

#include <string>
#include <vector>
#include <map>

#include "pugixml.hpp"

#include "Scraper.h"

struct Item
{
	std::string title, link, description, pubDate;

	bool operator==(const std::string& rhs)
	{
		return this->link == rhs;
	}
	bool operator!=( const std::string& rhs)
	{
		return !( (*this) == rhs);
	}
};

struct FeedData
{
	std::string link;
	std::string lastBuildTime;
	std::vector<Item> items;
};

class FeedsDatabase
{
public:
	FeedsDatabase();
	std::vector<std::string> updateFeed(std::string feedLink, pugi::xml_node root);		//function returns vector of links to the new items
	void saveDatabase();
	~FeedsDatabase();
private:
	void loadDatabase();
	bool isFeedSaved(std::string feedLink);
	bool isItemSaved(std::vector<Item> savedItems, std::string itemLink);
	std::map<std::string, FeedData> feeds;										//link to feed and feed
	Scraper scraper;
};

