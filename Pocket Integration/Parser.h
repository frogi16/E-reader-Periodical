#pragma once

#include <curl/curl.h>
#include <string>
#include <vector>

#include "ParsedArticle.h"
#include "ArticleRSS.h"

class Parser
{
public:
	Parser(std::string mercuryKey);
	std::vector<ParsedArticle> getParsedArticles(const std::vector<ArticleRSS> & items);
	~Parser();
private:
	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	void callMercury(std::string link);
	ParsedArticle parseArticle(std::string & article);
	void resolveConflicts(ParsedArticle& mercuryArticle, const ArticleRSS & rssArticle);							//determines which data is more reliable: parsed by Mercury or downloaded from RSS feed

	std::string mMercuryKey;
	std::string response;
	CURL* curl;
};

