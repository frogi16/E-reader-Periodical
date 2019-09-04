#pragma once

#include <vector>

#include "ArticleRSS.h"
#include "CurlWrapper.h"

class PocketRetriever
{
public:
	PocketRetriever(const std::string& consumerKey);
	std::vector<ArticleRSS> retrieveArticles(const std::string& accessToken);
	~PocketRetriever();
private:
	std::vector<ArticleRSS> extractLinks(const std::string& input);
	std::vector<std::string> extractItemIDs(const std::string& input);
	void archiveArticles(const std::string& accessToken, const std::vector<std::string>& itemIDs);

	std::string mConsumerKey;
	CurlWrapper curlRetriever;
	CurlWrapper curlModifier;
};

