#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

class ArticleAdder
{
public:
	ArticleAdder(std::string consumerKey);
	void addArticles(std::vector<std::string> links, std::string accessToken);
	~ArticleAdder();
private:
	CURL * handle;
	std::string mConsumerKey;
};

