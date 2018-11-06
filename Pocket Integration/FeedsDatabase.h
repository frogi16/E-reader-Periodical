#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include "pugixml.hpp"

#include "DataSelecter.h"
#include "ArticleRSS.h"

struct FeedData
{
	std::string link;
	std::string lastBuildTime;
	std::vector<ArticleRSS> items;
};

struct Keyword
{
	std::list<std::string> alternatives;
	std::string mainKeyword()
	{
		return (alternatives.size() ? alternatives.front() : std::string());								//if possible, return first keyword. Otherwise return empty string
	}

	const bool operator==(const std::string& rhs) const
	{
		for (auto& alternative : alternatives)
		{
			if (rhs == alternative)
				return true;
		}

		return false;
	}

	const bool operator!=(const std::string& rhs) const
	{
		return !((*this) == rhs);
	}
};

class FeedsDatabase
{
public:
	FeedsDatabase() noexcept;
	std::vector<ArticleRSS> updateFeed(std::string feedLink, pugi::xml_node root);							//function returns vector of links to the new items
	void saveDatabase();																					//write to file
	~FeedsDatabase();
private:
	void loadDatabase();																					//loading database into RAM
	void loadKeywords();																					//loading groups of keywords from file

	bool isFeedSaved(std::string feedLink);
	bool isItemSaved(const std::vector<ArticleRSS> & savedItems, std::string itemLink);

	std::vector<pugi::xml_node> searchForKeyword(pugi::xml_node root, Keyword keyword, size_t minimalResultNumber = 1, bool checkForChild = false);			//checkForChild makes function to check whether found result have valid child. It is done to filter out nodes storing values using attributes instead of children
	ArticleRSS createItem(std::string itemLink, pugi::xml_node itemNode);
	
	std::string getLinkFromAlternateHref(pugi::xml_node root);												//for example: "<link rel='alternate' type='text/html' href='https://czajniczek-pana-russella.blogspot.com/2018/10/gdzie-sie-podziali-kanibale.html' title='Gdzie siê podziali kanibale?'/>"

	std::map<std::string, FeedData> feeds;																	//link to feed and feed itself
	DataSelecter dataSelecter;																				//object containing functions facilitating scraping and filtering xml nodes
	Keyword pubDateKeyword, itemKeyword, linkKeyword, descriptionKeyword, titleKeyword;
};

