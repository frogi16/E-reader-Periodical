#pragma once

#include "Authenticator.h"
#include "RSSupdater.h"
#include "ArticleAdder.h"

#include <string>
#include <map>

#include "APIKeyHolder.h"

class Application
{
public:
	Application();
	void run();
	~Application();
private:
	void authenticateConnection();
	std::vector<std::string> checkRSS();
	void addArticles(std::vector<std::string> urls);
	void loadFeedsToWatch();

	APIKeyHolder keyHolder;
	Authenticator authenticator;
	RSSupdater updater;
	ArticleAdder adder;

	std::map<std::string, std::string> users;
	UserData currentUser;
};