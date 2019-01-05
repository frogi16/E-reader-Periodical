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
	std::vector<ArticleRSS> checkRSS();
	std::vector<ArticleRSS> getArticlesFromPocket();
	void addArticlesToPocket(const std::vector<std::string> & urls);
	void createMobi(const std::vector<ArticleRSS> & items);
	void loadFeedsToWatch();

	APIKeyHolder keyHolder;
	Authenticator authenticator;
	RSSupdater updater;
	ArticleAdder adder;
	PocketRetriever pocketRetriever;
	Parser parser;
	ArticleFilter filter;
	EbookCreator ebookCreator;
	ArticlesDatabase articlesDatabase;

	std::map<std::string, std::string> users;
	UserData currentUser;
};