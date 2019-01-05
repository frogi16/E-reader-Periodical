#include "Parser.h"
#include "ArticleRSS.h"
#include "CountWordsTreeWalker.h"

#include <iostream>

#include <tidy.h>
#include <tidybuffio.h>

Parser::Parser(const std::string & mercuryKey) : mMercuryKey(mercuryKey)
{
	configureCurl();
}

std::vector<ParsedArticle> Parser::getParsedArticles(const std::vector<ArticleRSS>& items)
{
	std::vector<ParsedArticle> parsedArticles;

	for (auto& item : items)
	{
		try
		{
			callMercury(item.link);
			std::cout << "*";

			auto parsedArticle = parseArticle(curlWrapper.getResponseString());

			resolveConflicts(parsedArticle, item);
			loadToXML(parsedArticle);
			countWords(parsedArticle);
			parsedArticles.push_back(parsedArticle);
		}
		catch (const std::exception& e)						//if anything went wrong it will be better not to show this article to end user in form of ebook so parsedArticle WON'T be added to output
															//(it probably is internal error which returnes no real content, so only article title is visible)
		{
			std::cout << std::endl << e.what() << std::endl;
		}
		
	}
	std::cout << std::endl;

	return parsedArticles;
}

void Parser::callMercury(const std::string & link)
{
	std::string url = std::string("https://mercury.postlight.com/parser") + "?url=" + link;
	curlWrapper.setURL(url);
	curlWrapper.perform();
}

ParsedArticle Parser::parseArticle(const std::string & article)
{
	nlohmann::json jsonResponse = nlohmann::json::parse(curlWrapper.getResponseString());
	ParsedArticle parsedArticle;

	if (isResponseValid(jsonResponse))						//if something was sent using message field it means that something went wrong
		loadParsedData(parsedArticle, jsonResponse);
	else
		detectAndThrowParserError(jsonResponse);

	return parsedArticle;
}

void Parser::resolveConflicts(ParsedArticle & mercuryArticle, const ArticleRSS & rssArticle)
{
	if (rssArticle.title.size())							//Mercury sometimes uses site name as an article title, RSS data is much more reliable
		mercuryArticle.title = rssArticle.title;
}

void Parser::loadToXML(ParsedArticle & article)
{
	std::shared_ptr<pugi::xml_document> document(std::make_shared<pugi::xml_document>());
	
	TidyBuffer XMLdata = { 0 };
	TidyBuffer errbuf = { 0 };
	int rc = -1;				//return code
	Bool ok;

	TidyDoc tidyDoc = tidyCreate();													//initialize "document"

	ok = tidyOptSetBool(tidyDoc, TidyXhtmlOut, yes);								//convert to XHTML
	ok = tidyOptSetBool(tidyDoc, TidyPreserveEntities, yes);						//leaving it on false causes text to show many &nbsp strings

	if (ok)
		rc = tidySetErrorBuffer(tidyDoc, &errbuf);									//link tidy document to error buffer
	if (rc >= 0)
		rc = tidyParseString(tidyDoc, article.content.c_str());						//load source code and parse it
	if (rc >= 0)
		rc = tidyCleanAndRepair(tidyDoc);											//tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tidyDoc);											//reports the document type and diagnostic statistics on parsed and repaired markup. 

	if (rc > 1)																		//if error, force output.
		rc = (tidyOptSetBool(tidyDoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tidyDoc, &XMLdata);										//save output into XMLdata variable

	if (rc >= 0)
	{
	}
	else
		printf("A severe error (%d) occurred.\n", rc);

	tidyBufFree(&errbuf);
	tidyRelease(tidyDoc);
	
	document->load_buffer(XMLdata.bp, XMLdata.size);								//loading data into pugi document
	tidyBufFree(&XMLdata);															//removing xml data from tidy buffer

	article.xmlDocument = std::move(document);
}

void Parser::countWords(ParsedArticle & article)
{
	CountWordsTreeWalker walker;
	article.xmlDocument->traverse(walker);
	article.wordCount = walker.words;
}

void Parser::loadParsedData(ParsedArticle & article, nlohmann::json & data)
{
	if (!data["author"].is_null())
		article.author = data["author"].get<std::string>();
	if (!data["title"].is_null())
		article.title = data["title"].get<std::string>();
	if (!data["content"].is_null())
		article.content = data["content"].get<std::string>();
	if (!data["domain"].is_null())
		article.domain = data["domain"].get<std::string>();
	if (!data["date_published"].is_null())
		article.pubDate = data["date_published"].get<std::string>();
}

void Parser::detectAndThrowParserError(const nlohmann::json & response) const
{
	std::string message = response["message"].get<std::string>();
	if (message == "Internal server error")
	{
		throw(std::exception("Parser couldn't parse one of the articles due to internal parser server error. Please try again later."));
	}
	else
	{
		throw(std::exception(std::string("Parser couldn't parse one of the articles. Reason is: " + message).c_str()));
	}

	//TODO: Expand list. Mercury has no detailed documentation so all message types have to be discovered by experiencing it.
}

void Parser::configureCurl()
{
	curlWrapper.setWritingToString();
	curlWrapper.addToSlist((std::string("x-api-key: ") + mMercuryKey).c_str());
	curlWrapper.addToSlist("Content-Type: application/json");
}

bool Parser::isResponseValid(const nlohmann::json & response) const
{
	return response.count("message") == 0;
}

Parser::~Parser()
{
}