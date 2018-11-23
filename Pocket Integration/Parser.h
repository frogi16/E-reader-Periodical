#pragma once

#include <string>
#include <vector>
#include "json.h"

#include "ParsedArticle.h"
#include "ArticleRSS.h"
#include "CurlWrapper.h"

class Parser
{
public:
	Parser(const std::string & mercuryKey);
	std::vector<ParsedArticle> getParsedArticles(const std::vector<ArticleRSS> & items);							//takes vector of ArticleRSS (url alone is completely sufficient, title, description etc. are completely optional), parse articles and returns corresponding vector
	~Parser();
private:
	void callMercury(const std::string & link);
	ParsedArticle parseArticle(const std::string & article);
	void resolveConflicts(ParsedArticle& mercuryArticle, const ArticleRSS & rssArticle);							//determines which data is more reliable: parsed by Mercury or downloaded from RSS feed
	void loadToXML(ParsedArticle & article);																		//loads ParsedArticle.content into xml tree and places pointer to it in ParsedArticle.xmlDocument
	void countWords(ParsedArticle & article);																		//traverses xml tree, counts visible words (no markups etc.) and writes it into article.wordCount
	void loadParsedData(ParsedArticle & article, nlohmann::json & data);											//loads data json content into ParsedArticle
	void detectAndThrowParserError(const nlohmann::json & response) const;											//tries to detect any errors or warnings in response received from Mercury and throws corresponding error describing problem in details
	void configureCurl();
	bool isResponseValid(const nlohmann::json & response) const;
	
	std::string mMercuryKey;
	CurlWrapper curlWrapper;
};

