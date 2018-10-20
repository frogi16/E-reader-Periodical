#pragma once

#include "Authenticator.h"
#include "RSSupdater.h"
#include "ArticleAdder.h"
#include "Parser.h"
#include "EbookCreator.h"

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
	std::vector<std::string> checkRSS();
	void addArticles(const std::vector<std::string> & urls);
	void createMobi(const std::vector<std::string> & urls);
	void loadFeedsToWatch();

	APIKeyHolder keyHolder;
	Authenticator authenticator;
	RSSupdater updater;
	ArticleAdder adder;
	Parser parser;
	EbookCreator ebookCreator;

	std::map<std::string, std::string> users;
	UserData currentUser;
};