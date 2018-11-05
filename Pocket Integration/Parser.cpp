#include "Parser.h"
#include "ArticleRSS.h"
#include "CountWordsTreeWalker.h"

#include "json.h"
#include <iostream>

using json = nlohmann::json;

Parser::Parser(std::string mercuryKey) : mMercuryKey(mercuryKey)
{
	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);			//set response string as responses container
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, (std::string("x-api-key: ") + mMercuryKey).c_str());		//set header with api key
	chunk = curl_slist_append(chunk, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
}

std::vector<ParsedArticle> Parser::getParsedArticles(const std::vector<ArticleRSS>& items)
{
	std::vector<ParsedArticle> parsedArticles;

	for (auto& item : items)
	{
		callMercury(item.link);
		std::cout << "*";

		auto parsedArticle = parseArticle(response);
		resolveConflicts(parsedArticle, item);
		loadToXML(parsedArticle);
		countWords(parsedArticle);
		parsedArticles.push_back(parsedArticle);
		response.clear();
	}
	std::cout << std::endl;

	return parsedArticles;
}

void Parser::callMercury(std::string link)
{
	std::string url = std::string("https://mercury.postlight.com/parser") + "?url=" + link;
	
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	auto result = curl_easy_perform(curl);
}

ParsedArticle Parser::parseArticle(std::string & article)
{
	ParsedArticle parsedArticle;

	auto json = json::parse(response);
	if(!json["author"].is_null())
		parsedArticle.author = json["author"].get<std::string>();
	if (!json["title"].is_null())
		parsedArticle.title = json["title"].get<std::string>();
	if (!json["content"].is_null())
		parsedArticle.content = json["content"].get<std::string>();
	if (!json["domain"].is_null())
		parsedArticle.domain = json["domain"].get<std::string>();
	if (!json["date_published"].is_null())
		parsedArticle.pubDate = json["date_published"].get<std::string>();

	return parsedArticle;
}

void Parser::resolveConflicts(ParsedArticle & mercuryArticle, const ArticleRSS & rssArticle)
{
	if (rssArticle.title.size())							//Mercury sometimes uses site name as an article title, RSS data is much more reliable
		mercuryArticle.title = rssArticle.title;	
}
#include <tidy.h>
#include <tidybuffio.h>
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

Parser::~Parser()
{
}

size_t Parser::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
{
	size_t newLength = size * nmemb;
	size_t oldLength = s->size();
	try
	{
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e)
	{
		//handle memory problem
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size * nmemb;
}
