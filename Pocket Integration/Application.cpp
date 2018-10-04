#include "Application.h"

#include <fstream>

Application::Application() : 
	authenticator(consumerKey),
	adder(consumerKey)
{
	curl_global_init(CURL_GLOBAL_ALL);
	updater.setUpdateFrequency(5);		//update every 5 minutes

	std::fstream watchedFeeds("watchedFeeds.txt", std::ios::in);

	while (!watchedFeeds.eof())
	{
		std::string feed;
		watchedFeeds >> feed;
		updater.watchFeed(feed);
	}
}

void Application::run()
{
	authenticateConnection();

	while (true)
	{
		auto newArticleLinks = checkRSS();
		addArticles(newArticleLinks);

		Sleep(1000 * 60);
	}
}

void Application::authenticateConnection()
{
	std::cout << "Authenticating connection" << std::endl;

	if (users.size() > 0)
	{
		currentUser.username = users.begin()->first;			//for now application just uses first user on the list. Later, maybe with GUI, there should be some possibility to choose user.
		currentUser.accessToken = users.begin()->second;
	}
	else
	{
		UserData user = authenticator.authenticate();
		users[user.username] = user.accessToken;
		currentUser = user;
	}
}

std::vector<std::string> Application::checkRSS()
{
	std::cout << "Checking for new articles" << std::endl;
	return updater.checkUpdates();
}

void Application::addArticles(std::vector<std::string> urls)
{
	//std::cout << "Sending articles to pocket" << std::endl;
	//adder.addArticles(urls, currentUser.accessToken);
}

Application::~Application()
{
}
