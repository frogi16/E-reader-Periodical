#include "FeedsDatabase.h"

#include <fstream>
#include <iostream>
#include <algorithm>

FeedsDatabase::FeedsDatabase() noexcept
{
	loadDatabase();
	loadKeywords();
}

std::vector<ArticleRSS> FeedsDatabase::updateFeed(const std::string& feedLink, const pugi::xml_node& root)
{
	std::vector<ArticleRSS> newItems;

	try
	{
		std::string lastBuild = searchForKeyword(root, keywords["pubDate"], 1).front().child_value();
		//selectDataByName returns vector of all matching nodes, but often we search for unique node(or specified number of them),
		//therefore use of auxillary searchForKeyword function. It allows to use defined lists of alternative keywords, which are used in
		//predictable order, only if previous one doesn't return expected number of results.
		//.front().child_value() is necessary and will be repeatedly used all across this object

		if (isFeedChanged(feedLink, lastBuild))								//if feed has been updated since last check (if feed can't be found in map result is always true)
		{
			std::cout << "Updating " << feedLink << std::endl;

			auto itemNodes = searchForKeyword(root, keywords["item"], 1);

			FeedData& feed = feeds[feedLink];								//reference to feeds[feedLink]
			feed.link = feedLink;											//assigning link to feed. It will have no effect if feed was already used.
			feed.lastBuildTime = lastBuild;

			for (auto& itemNode : itemNodes)
			{
				std::string itemLink = getLinkToItem(itemNode);

				if (!isItemSaved(feed.items, itemLink))						//if this item was already saved there is no need to do anything and we can just go to the next one
				{
					auto item = createItem(itemLink, itemNode);
					newItems.push_back(item);
					feed.items.push_back(item);
				}
			}

			std::cout << "Found " << newItems.size() << " new articles" << std::endl;
		}
		else
		{
			std::cout << "Processing " << feedLink << " content was skipped because site hasn't been updated since last check." << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Unexpected behavior:\n" << e.what() << "\nUpdating this feed failed.\n";
	}

	std::reverse(newItems.begin(), newItems.end());
	return newItems;
}

void FeedsDatabase::saveDatabase()
{
	pugi::xml_document doc;

	doc.append_child("header");
	//there is nothing in the header right now

	auto feedsNode = doc.append_child("feeds");

	for (auto& feed : feeds)
	{
		auto feedNode = feedsNode.append_child("feed");
		feed.second.serializeXML(feedNode);					//FeedData (and other classes implementing XMLSerializerInterface) can serialize itself to given node. It takes care of itself and everything inside.
	}

	doc.save_file("database.xml");
}

void FeedsDatabase::loadDatabase()
{
	pugi::xml_document doc;
	doc.load_file("database.xml");
	auto node = doc.child("feeds");

	for (auto& feed : node.children())
	{
		FeedData feedData;
		feedData.deserializeXML(feed);						//FeedData (and other classes implementing XMLSerializerInterface) can deserialize itself from given node. It takes care of itself and everything inside.
		feeds[feedData.link] = feedData;
	}
}

void FeedsDatabase::loadKeywords()
{
	pugi::xml_document doc;
	doc.load_file("keywords.xml");

	for (auto& group : doc.children())
	{
		Keyword newKeyword;

		std::string mainKeywordString = group.child("main_keyword").first_child().value();	//main keyword identifies predefined keywords group which is implemented by given node
		newKeyword.alternatives.push_back(mainKeywordString);

		for (auto& alternative : group.child("alternatives").children())
		{
			newKeyword.alternatives.push_back(alternative.child_value());
		}

		keywords[mainKeywordString] = std::move(newKeyword);
	}
}

bool FeedsDatabase::isFeedSaved(const std::string& feedLink) const
{
	return feeds.find(feedLink) != feeds.end();
}

bool FeedsDatabase::isFeedChanged(const std::string& feedLink, const std::string& buildTime) const
{
	if (auto searchedFeed = feeds.find(feedLink); searchedFeed == feeds.end() || searchedFeed->second.lastBuildTime.empty())		//newly added feed isn't saved in feeds, but of course should be updated. Similarly if because of some mystic reasons (bugs, failed parsing, unexpected exceptions etc.) date string is empty
		return true;
	else
	{
		std::time_t oldTime_t = datetimeParser.parseToTime_t(searchedFeed->second.lastBuildTime);
		std::time_t newTime_t = datetimeParser.parseToTime_t(buildTime);

		if (oldTime_t == -1 || newTime_t == -1)											//last resort of error-proofing is check if time_t is equal -1. It may happen if date couldn't be parsed at all. -1 seems to be treated by difftime() as special case and it returns 0, so boolean result has to be handled manually
			return true;

		return difftime(newTime_t, oldTime_t) > 0;									//true if newTime is greater then oldTime
	}
}

bool FeedsDatabase::isItemSaved(const std::vector<ArticleRSS>& savedItems, const std::string& itemLink) const
{
	return std::find(savedItems.begin(), savedItems.end(), itemLink) != savedItems.end();
}

std::vector<pugi::xml_node> FeedsDatabase::searchForKeyword(const pugi::xml_node& root, const Keyword& keyword, size_t minimalResultNumber, bool checkForChild)
{
	bool noChildren = false;															//flag set when results were rejected because of lack of children. Used to determine which error should be thrown at the end of function

	for (auto& alternative : keyword.alternatives)
	{
		std::vector<pugi::xml_node> result = EbookPeriodical::selectNodes<SelectNameTreeWalker>(root, alternative);

		if (result.size() >= minimalResultNumber && !checkForChild)
			return result;

		if (result.size() >= minimalResultNumber && checkForChild && result[0].first_child())
			return result;

		if (result.size() >= minimalResultNumber && checkForChild && !result[0].first_child())
			noChildren = true;
	}

	if (noChildren)
		throw std::exception(std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned results without children.").c_str());
	else
		throw std::exception(std::string("Filtering nodes for \"" + keyword.mainKeyword() + "\" keyword returned less than expected " + std::to_string(minimalResultNumber) + " results.").c_str());
}

ArticleRSS FeedsDatabase::createItem(const std::string& itemLink, const pugi::xml_node& itemNode)
{
	ArticleRSS newItem;

	newItem.link = itemLink;
	newItem.title = searchForKeyword(itemNode, keywords["title"], 1, true).front().child_value();
	newItem.description = searchForKeyword(itemNode, keywords["description"], 1, true).front().child_value();
	newItem.pubDate = searchForKeyword(itemNode, keywords["pubDate"], 1, true).front().child_value();

	return newItem;
}

std::string FeedsDatabase::getLinkToItem(const pugi::xml_node& itemNode)
{
	//most RSS feeds embeds link to item in <link> or <item> markups, but apparently some sites use a different approach
	//for example blogspot.com shows something like this:
	//"<id>tag:blogger.com,1999:blog-2442024767695998453.post-2644038318746445557</id>"
	//and valid link is hidden in here:
	//"<link rel='alternate' type='text/html' href='[...]' title='[...]'/>"
	//simple search for "http" substring decides whether link is valid or not
	//right now I encountered only one problematic RSS generator, if more would be found, more general approach should be developed

	std::string itemLink = getLinkFromStandardHref(itemNode);

	if (itemLink.find("http") == std::string::npos)
	{
		itemLink = getLinkFromAlternateHref(itemNode);
	}

	return itemLink;
}

std::string FeedsDatabase::getLinkFromStandardHref(const pugi::xml_node& root)
{
	return searchForKeyword(root, keywords["link"], 1, true).front().child_value();
}

std::string FeedsDatabase::getLinkFromAlternateHref(const pugi::xml_node& root)
{
	auto links = EbookPeriodical::selectNodes<SelectAttributeTreeWalker>(root, "rel", "alternate");

	if (links.size())
		return links[0].attribute("href").value();
	else
		throw std::exception("Haven't found link in node with rel=\'alternate\' attribute!\n");
}

FeedsDatabase::~FeedsDatabase()
{
}