#pragma once

#include <string>
#include <vector>

#include "json_eprd.h"

#include "ParsedArticle.h"
#include "ArticleRSS.h"

class Parser
{
public:
	std::vector<ParsedArticle> getParsedArticles(const std::vector<ArticleRSS>& items);							//takes vector of ArticleRSS (url alone is completely sufficient, title, description etc. are completely optional), parse articles and returns corresponding vector
	~Parser();
private:
	ParsedArticle parseArticle(const ArticleRSS& articleRSS);
	std::string getRandomFilename(std::string& path, std::string& extension) const;
	void shellExecuteAndWait(std::string& verb, std::string& file, std::string& parameters) const;
	void resolveConflicts(ParsedArticle& mercuryArticle, const ArticleRSS& rssArticle);							//determines which data is more reliable: parsed by Mercury or downloaded from RSS feed
	void loadToXML(ParsedArticle& article);																		//loads ParsedArticle.content into xml tree and places pointer to it in ParsedArticle.xmlDocument
	void countWords(ParsedArticle& article);																		//traverses xml tree, counts visible words (no markups etc.) and writes it into article.wordCount
	void loadParsedData(ParsedArticle& article, nlohmann::json& data);											//loads data json content into ParsedArticle
	void detectAndThrowParserError(const nlohmann::json& response) const;											//tries to detect any errors or warnings in response received from Mercury and throws corresponding error describing problem in details

	bool isResponseValid(const nlohmann::json& response) const;
};

