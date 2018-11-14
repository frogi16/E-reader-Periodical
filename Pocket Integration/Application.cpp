#include "Application.h"

Application::Application() noexcept :
	authenticator(keyHolder.pocketKey),
	adder(keyHolder.pocketKey),
	pocketRetriever(keyHolder.pocketKey),
	parser(keyHolder.mercuryKey)
{
	updater.setUpdateFrequencyInMinutes(5);
	loadFeedsToWatch();
}

void Application::run()
{
	authenticateConnection();

	while (true)
	{
		auto links = pocketRetriever.retrieveArticles(currentUser.accessToken);
		auto newArticlesRSS = checkRSS();						//info about articles recieved from RSS. Title, link, description etc. No actual article
		links.insert(links.end(), newArticlesRSS.begin(), newArticlesRSS.end());

		createMobi(newArticlesRSS);
		//addArticlesToPocket(newArticles);

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

std::vector<ArticleRSS> Application::checkRSS()
{
	return updater.checkUpdates();
}

std::vector<ArticleRSS> Application::getArticlesFromPocket()
{
	return std::vector<ArticleRSS>();
}

void Application::addArticlesToPocket(const std::vector<std::string> & urls)
{
	if (urls.size())
		std::cout << "Sending " << urls.size() << " articles to pocket" << std::endl;

	adder.addArticles(urls, currentUser.accessToken);
}

void Application::createMobi(const std::vector<ArticleRSS>& items)
{
	if (items.size())
	{
		std::cout << "Parsing " << items.size() << " articles" << std::endl;

		auto articles = parser.getParsedArticles(items);						//after parsing ParsedArticle contains all informations about article - title, description, full content in string and xml tree simultaneously etc.

		std::cout << "Filtering articles" << std::endl;

		filter.filterArticles(articles);										//filtering out fragments of articles, removing too short and too long ones

		if (articles.size())
		{
			std::cout << "Creating epub from  " << articles.size() << " articles" << std::endl;

			ebookCreator.createEpub(articles);

			std::cout << "Converting to mobi" << std::endl;

			ebookCreator.convertToMobi();
			//ebookCreator.removeEpub();
		}
	}
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
