#pragma once

#include <string>
#include <vector>
#include <list>
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

struct Keyword
{
	std::list<std::string> alternatives;
	std::string mainKeyword()
	{
		return (alternatives.size() ? alternatives.front() : std::string());
	}

	bool operator==(const std::string& rhs)
	{
		for (auto& alternative : alternatives)
		{
			if (rhs == alternative)
				return true;
		}

		return false;
	}
	bool operator!=(const std::string& rhs)
	{
		return !((*this) == rhs);
	}
};

class FeedsDatabase
{
public:
	FeedsDatabase();
	std::vector<std::string> updateFeed(std::string feedLink, pugi::xml_node root);							//function returns vector of links to the new items
	void saveDatabase();
	~FeedsDatabase();
private:
	void loadDatabase();																					//loading database into RAM
	bool isFeedSaved(std::string feedLink);
	bool isItemSaved(std::vector<Item> savedItems, std::string itemLink);
	std::vector<pugi::xml_node> searchForKeyword(pugi::xml_node root, Keyword keyword, size_t minimalResultNumber = 1, bool checkForChild = false);			//checkForChild makes function to check whether found result have valid child. It is done to filter out nodes storing values using attributes instead of children
	Item createItem(std::string itemLink, pugi::xml_node itemNode);
	
	std::map<std::string, FeedData> feeds;																	//link to feed and feed
	Scraper scraper;
	Keyword lastBuildDateKeyword, pubDateKeyword, itemKeyword, linkKeyword, descriptionKeyword, titleKeyword;
};
