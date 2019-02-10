#pragma once

#include <vector>
#include <string>

#include "pugixml.hpp"

#include "DataSelecter.h"
#include "ArticleRSS.h"
#include "Keyword.h"
#include "DatabaseKeywords.h"
#include "DatetimeParser.h"

struct FeedData
{
	std::string link;
	std::string lastBuildTime;
	std::vector<ArticleRSS> items;
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

	bool isFeedSaved(const std::string & feedLink) const;
	bool isFeedChanged(const std::string & feedLink, const std::string & buildTime) const;
	bool isItemSaved(const std::vector<ArticleRSS> & savedItems, const std::string & itemLink) const;

	std::time_t parseDatetime(const std::string & datetime) const;

	std::vector<pugi::xml_node> searchForKeyword(const pugi::xml_node & root, const Keyword & keyword, size_t minimalResultNumber = 1, bool checkForChild = false);			//checkForChild makes function to check whether found result have valid child. It is done to filter out nodes storing values using attributes instead of children
	ArticleRSS createItem(std::string itemLink, pugi::xml_node itemNode);
	
	std::string getLinkFromAlternateHref(pugi::xml_node root);												//for example: "<link rel='alternate' type='text/html' href='https://czajniczek-pana-russella.blogspot.com/2018/10/gdzie-sie-podziali-kanibale.html' title='Gdzie siê podziali kanibale?'/>"

	std::map<std::string, FeedData> feeds;																	//link to feed and feed itself
	DataSelecter dataSelecter;																				//object containing functions facilitating scraping and filtering xml nodes

	Keyword pubDateKeyword, itemKeyword, linkKeyword, descriptionKeyword, titleKeyword;
	DatabaseKeywords keywords;

	DatetimeParser datetimeParser;
};

