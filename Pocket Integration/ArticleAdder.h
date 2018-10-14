#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

class ArticleAdder
{
public:
	ArticleAdder(std::string consumerKey);
	void addArticles(const std::vector<std::string> & links, std::string accessToken) const;
	~ArticleAdder();
private:
	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static
	CURL * handle;
	std::string mConsumerKey;
	std::string response;
};

