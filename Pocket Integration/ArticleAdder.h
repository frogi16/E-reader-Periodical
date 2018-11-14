#pragma once

#include <string>
#include <vector>

#include "CurlWrapper.h"

class ArticleAdder
{
public:
	ArticleAdder(std::string consumerKey);
	void addArticles(const std::vector<std::string> & links, std::string accessToken);
	~ArticleAdder();
private:
	CurlWrapper curlWrapper;
	std::string mConsumerKey;
	std::string response;
};

