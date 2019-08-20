#pragma once

#include "Authenticator.h"
#include "RSSupdater.h"
#include "ArticleAdder.h"
#include "PocketRetriever.h"
#include "Parser.h"
#include "ArticleFilter.h"
#include "EbookCreator.h"
#include "ArticlesDatabase.h"
#include "CommandInterpreter.h"

#include <string>
#include <map>

#include "APIKeyHolder.h"

class Application
{
public:
	Application() noexcept;
	void run();
	//~Application() noexcept;
private:
	void addCommands(CommandInterpreter& commandInterpreter, std::vector<ArticleRSS>& articlesRSS, std::vector<ParsedArticle>& articles);	//creates lambda callbacks and adds commands to the interpreter
	void authenticateConnection();
	std::vector<ArticleRSS> checkRSS();													//check all RSS feeds and return vector of new articles
	void addArticlesToPocket(const std::vector<std::string> & urls);					//save given urls to Pocket
	std::vector<ParsedArticle> parseArticles(const std::vector<ArticleRSS> & items);	//parse ArticleRSSs, return vector of ParsedArticles
	void filterArticles(std::vector<ParsedArticle>& articles);							//filter ParsedArticles in place
	void createMobi(std::vector<ParsedArticle> & articles);								//create ebook in .mobi format and save it
	void loadFeedsToWatch();
	void promptCommandInterpretationStatus(const eprd::InterpretationResult &interpretation) const;

	APIKeyHolder keyHolder;
	Authenticator authenticator;
	RSSupdater updater;
	ArticleAdder adder;
	PocketRetriever pocketRetriever;
	Parser parser;
	ArticleFilter filter;
	EbookCreator ebookCreator;
	ArticlesDatabase articlesDatabase;													//database holding info about new articles which weren't parsed and converted into an ebook yet.

	std::map<std::string, std::string> users;
	UserData currentUser;
};