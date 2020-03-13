#include "Parser.h"

#include <Windows.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include <tidy.h>
#include <tidybuffio.h>

#include "ArticleRSS.h"
#include "CountWordsTreeWalker.h"

namespace fs = std::filesystem;

std::vector<ParsedArticle> Parser::getParsedArticles(const std::vector<ArticleRSS>& items)
{
	std::vector<ParsedArticle> parsedArticles;

	for (auto& item : items)
	{
		try
		{
			auto parsedArticle = parseArticle(item);
			resolveConflicts(parsedArticle, item);
			loadToXML(parsedArticle);
			countWords(parsedArticle);
			parsedArticles.push_back(parsedArticle);

			std::cout << "*";
		}
		catch (const std::exception & e)						//if anything went wrong it would be better not to show this article to end user inside an ebook, so parsedArticle won't be added to the output
															//(it is probably an internal error which returns no real content, only short message and article title)
		{
			std::cout << std::endl << e.what() << std::endl;
		}

	}
	std::cout << std::endl;

	return parsedArticles;
}

ParsedArticle Parser::parseArticle(const ArticleRSS& articleRSS)
{
	static std::string verb{ "open" };
	static std::string file{ "cmd.exe" };
	static std::string tempDirectory{ "temp/" };
	static std::string jsonExtension{ ".json" };

	std::string filename = getRandomFilename(tempDirectory, jsonExtension);

	//command consists of "/C" (carry out the command and terminate), "mercury-parser" (CLI run by npm), article link (parameter for parser), " > filename" (redirecting output stream to file)
	std::string command = "/C mercury-parser " + articleRSS.link + " > " + filename;

	shellExecuteAndWait(verb, file, command);

	std::ifstream fileParseResult(filename, std::ios::in);
	nlohmann::json jsonResponse = nlohmann::json::parse(fileParseResult);			//load json from file to nlohmann representation
	fileParseResult.close();
	fs::remove(fs::path(filename));													//temporary files should be removed

	if (isResponseValid(jsonResponse))												//if something was sent using message field it means that something went wrong
	{
		ParsedArticle parsedArticle;
		loadParsedData(parsedArticle, jsonResponse);
		return parsedArticle;
	}
	else
	{
		detectAndThrowParserError(jsonResponse);
		return ParsedArticle();														//function above will throw exception so this line will be never executed. Unfortunately Code Analysis can't understand it and C4715 warning (not all control paths return a value) occures.
	}
}

std::string Parser::getRandomFilename(std::string& path, std::string& extension) const
{
	int numberFilename = rand();
	std::stringstream stream;
	stream << std::hex << numberFilename;											//converting integer number to its hex string representation (53 453 456 -> "32fa290")
	std::string filename(stream.str());
	filename.insert(0, path);
	filename.append(extension);

	return filename;
}

void Parser::shellExecuteAndWait(std::string& verb, std::string& file, std::string& parameters) const
{
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = verb.c_str();
	ShExecInfo.lpFile = file.c_str(); ;
	ShExecInfo.lpParameters = parameters.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	if (ShExecInfo.hProcess)
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
}

void Parser::resolveConflicts(ParsedArticle& mercuryArticle, const ArticleRSS& rssArticle)
{
	if (rssArticle.title.size())													//Mercury sometimes uses site name as an article title, RSS data is much more reliable
		mercuryArticle.title = rssArticle.title;
}

void Parser::loadToXML(ParsedArticle& article)
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

void Parser::countWords(ParsedArticle& article)
{
	CountWordsTreeWalker walker;
	article.xmlDocument->traverse(walker);
	article.wordCount = walker.words;
}

void Parser::loadParsedData(ParsedArticle& article, nlohmann::json& data)
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

void Parser::detectAndThrowParserError(const nlohmann::json& response) const
{
	std::string message = response["message"].get<std::string>();
	if (message == "Internal server error")
	{
		throw(std::exception("Parser couldn't parse one of the articles due to internal parser error. Please try again later."));
	}
	else
	{
		throw(std::exception(std::string("Parser couldn't parse one of the articles. Reason is: " + message).c_str()));
	}

	//TODO: Expand list. Mercury has no detailed documentation so all message types have to be discovered by experiencing it.
}

bool Parser::isResponseValid(const nlohmann::json& response) const
{
	return response.count("message") == 0;
}

Parser::~Parser()
{
}