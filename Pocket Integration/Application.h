#pragma once

#include "Authenticator.h"
#include "RSSupdater.h"
#include "ArticleAdder.h"
#include "PocketRetriever.h"
#include "Parser.h"
#include "ArticleFilter.h"
#include "EbookCreator.h"
#include "ArticlesDatabase.h"

#include <string>
#include <map>

#include "APIKeyHolder.h"

class Application
{
public:
	Application() noexcept;
	void run();
	~Application();
private:
	void authenticateConnection();
	std::vector<ArticleRSS> checkRSS();									//checks all RSS feeds and returns vector of new articles
	std::vector<ArticleRSS> getArticlesFromPocket();					//requests Pocket and returns vector of new articles
	void addArticlesToPocket(const std::vector<std::string> & urls);	//saves given urls to Pocket
	void createMobi(const std::vector<ArticleRSS> & items);				//creates ebook in .mobi format and leaves it in program directory with current date in name
	void loadFeedsToWatch();

	APIKeyHolder keyHolder;
	Authenticator authenticator;
	RSSupdater updater;
	ArticleAdder adder;
	PocketRetriever pocketRetriever;
	Parser parser;
	ArticleFilter filter;
	EbookCreator ebookCreator;
	ArticlesDatabase articlesDatabase;		//database holding info about new articles which weren't parsed and converted into an ebook yet.

	std::map<std::string, std::string> users;
	UserData currentUser;
};