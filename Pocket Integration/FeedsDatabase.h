#pragma once

#include <vector>
#include <map>
#include <string>

#include "pugixml.hpp"

#include "DataSelecter.h"
#include "ArticleRSS.h"
#include "Keyword.h"
#include "DatetimeParser.h"
#include "FeedData.h"

class FeedsDatabase
{
public:
	FeedsDatabase() noexcept;

	std::vector<ArticleRSS> updateFeed(const std::string& feedLink, const pugi::xml_node& root);			//update feed, which already exists in database, and return vector of links to new items. Can be called from multiple threads at once
	void saveDatabase();																					//save database to database.xml file
	void createFeedIfNeeded(const std::string& feedLink) { feeds[feedLink]; }								//create empty feed in database
	~FeedsDatabase();
private:
	void loadDatabase();																					//load database into memory
	void loadKeywords();																					//load keywords into memory

	bool isFeedSaved(const std::string& feedLink) inline const;
	bool isItemSaved(const std::vector<ArticleRSS>& savedItems, const std::string& itemLink) inline const;

	bool isFeedChanged(const std::string& feedLink, const std::string& buildTime) const;

	//Search in given xml data for keywords in predictible order. When first keyword finds aprropriate results, they are returned without using any other keywords.
	//checkIfFirstValueValid parameter forces function to check whether the first found result has valid child.
	//This approach is useful to filter out nodes storing values in attributes instead of PCDATA.
	std::vector<pugi::xml_node> searchForKeyword(const pugi::xml_node& root, const Keyword& keyword, size_t minimalResultNumber = 1, bool checkIfFirstValueValid = false);

	ArticleRSS createItem(const std::string& itemLink, const pugi::xml_node& itemNode);						//extracts ArticleRSS data from given xml node and item link and returns it

	std::string getLinkToItem(const pugi::xml_node& itemNode);												//extracts link to item from xml data
	std::string getLinkFromStandardHref(const pugi::xml_node& root);										//standard method to extract link to item from xml data, used only be getLinkToItem() function!
	std::string getLinkFromAlternateHref(const pugi::xml_node& root);										//alternative method to extract link to item from xml data, used only be getLinkToItem() function!

	std::map<std::string, FeedData> feeds;																	//link to feed
	std::map<std::string, Keyword> keywords;																//main keyword to keyword object

	DatetimeParser datetimeParser;																			//parser matching datetime string representation to different templates and converting it to std::tm object if possible
};

