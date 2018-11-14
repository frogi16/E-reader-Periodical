#pragma once

#include <vector>

#include "ArticleRSS.h"
#include "CurlWrapper.h"

class PocketRetriever
{
public:
	PocketRetriever(std::string consumerKey);
	std::vector<ArticleRSS> retrieveArticles(std::string accessToken);
	~PocketRetriever();
private:
	std::vector<ArticleRSS> extractLinks(std::string & input);
	std::vector<std::string> extractItemIDs(std::string & input);
	void archiveArticles(std::string accessToken, std::vector<std::string> itemIDs);

	std::string mConsumerKey;
	CurlWrapper curlRetriever;
	CurlWrapper curlModifier;
};

