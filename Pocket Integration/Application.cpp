#include "Application.h"

Application::Application() noexcept :
	authenticator(keyHolder.pocketKey),
	adder(keyHolder.pocketKey),
	parser(keyHolder.mercuryKey)
{
	curl_global_init(CURL_GLOBAL_ALL);
	updater.setUpdateFrequencyInMinutes(5);
	loadFeedsToWatch();
}

void Application::run()
{
	authenticateConnection();

	while (true)
	{
		//std::vector<std::string> newArticleLinks;
		auto newArticleLinks = checkRSS();

		createMobi(newArticleLinks);
		//addArticles(newArticleLinks);

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
		UserData user = authenticator.authenticate();			//authenticate by connecting to pocket and redirecting user to their website (uses OAuth 2.0)
		users[user.username] = user.accessToken;
		currentUser = user;
	}
}

std::vector<std::string> Application::checkRSS()
{
	return updater.checkUpdates();
}

void Application::addArticles(const std::vector<std::string> & urls)
{
	if(urls.size())
		std::cout << "Sending " << urls.size() << " articles to pocket" << std::endl;

	adder.addArticles(urls, currentUser.accessToken);
}

void Application::createMobi(const std::vector<std::string>& urls)
{
	if (urls.size())
		std::cout << "Parsing " << urls.size() << " articles" << std::endl;

	auto articles = parser.getParsedArticles(urls);

	if (urls.size())
		std::cout << "Creating epub from  " << urls.size() << " articles" << std::endl;

	ebookCreator.createEpub(articles);
	
	std::cout << "Converting to mobi" << std::endl;

	ebookCreator.convertToMobi();
	//ebookCreator.removeEpub();
}

void Application::loadFeedsToWatch()
{
	std::fstream watchedFeeds("watchedFeeds.txt", std::ios::in);

	while (!watchedFeeds.eof())
	{
		std::string feed;
		watchedFeeds >> feed;
		updater.watchFeed(feed);
	}
}

Application::~Application()
{
}
