#pragma once

#include <string>
#include <vector>

#include "ParsedArticle.h"
#include "ArticleRSS.h"
#include "CurlWrapper.h"

class Parser
{
public:
	Parser(std::string mercuryKey);
	std::vector<ParsedArticle> getParsedArticles(const std::vector<ArticleRSS> & items);
	~Parser();
private:
	void callMercury(std::string link);
	ParsedArticle parseArticle(std::string & article);
	void resolveConflicts(ParsedArticle& mercuryArticle, const ArticleRSS & rssArticle);							//determines which data is more reliable: parsed by Mercury or downloaded from RSS feed
	void loadToXML(ParsedArticle & article);																		//loads ParsedArticle.content into xml tree and places pointer to it in ParsedArticle.xmlDocument
	void countWords(ParsedArticle & article);																		//traverses xml tree, counts visible words (no markups etc.) and writes it into article.wordCount

	std::string mMercuryKey;
	CurlWrapper curlWrapper;
};

