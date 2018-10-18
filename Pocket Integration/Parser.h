#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

struct ParsedArticle
{
	std::string title;
	std::string author;
	std::string content;
};

class Parser
{
public:
	Parser(std::string mercuryKey);
	std::vector<ParsedArticle> getParsedArticles(const std::vector<std::string> & links);
	~Parser();
private:
	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	void callMercury(std::string link);
	ParsedArticle parseArticle(std::string & article);

	std::string mMercuryKey;
	std::string response;
	CURL* curl;
};

