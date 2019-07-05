#include "Application.h"
#include <iostream>


Application::Application() noexcept :
	authenticator(keyHolder.pocketKey),
	adder(keyHolder.pocketKey),
	pocketRetriever(keyHolder.pocketKey),
	parser()
{
	srand(time(NULL));
	currentUser.username = "";
	currentUser.accessToken = "";
	updater.setUpdateFrequencyInMinutes(5);
	loadFeedsToWatch();
}

void Application::run()
{
	authenticateConnection();
	std::vector<ArticleRSS> articlesRSS = articlesDatabase.loadDatabase();		//info about articles. Title, link, description etc. No actual content
	std::vector<ParsedArticle> articles;

	while (true)
	{
		std::cout << std::endl << "There are " << articlesRSS.size() << " new articles!" << std::endl;
		std::cout << "Type \"update\" to check for new articles, \"parse\" to process and filter articles, \"select [NUMBER]\" to take specififc number of articles and \"book\" to create epub and mobi files." << std::endl;
		
		std::string input;

		while (true)
		{			
			std::cin >> input;
		}
		

		if (input == "update")
		{
			auto newArticles = pocketRetriever.retrieveArticles(currentUser.accessToken);
			articlesRSS.insert(articlesRSS.end(), newArticles.begin(), newArticles.end());

			newArticles = checkRSS();
			articlesRSS.insert(articlesRSS.end(), newArticles.begin(), newArticles.end());
			articlesDatabase.saveDatabase(articlesRSS);
		}
		else if (input == "parse")
		{
			articles = parseArticles(articlesRSS);
		}
		else if (input == "select")
		{
			std::cin >> input;

			try
			{
				int quantity = std::atoi(input.c_str());

				if (quantity >= 0 && quantity < articlesRSS.size())
				{
					articlesRSS.erase(articlesRSS.begin(), articlesRSS.begin() + quantity);
					articles.erase(articles.begin() + quantity, articles.end());

					articlesDatabase.saveDatabase(articlesRSS);
				}
				else
				{
					std::cout << "Invalid quantity!" << std::endl;
				}
			}
			catch (const std::exception& e)
			{
				std::cout << "Command invalid, selection unsuccessful." << std::endl;
			}
		}
		else if (input == "book")
		{
			createMobi(articles);
			articles.clear();
			articlesRSS.clear();
			articlesDatabase.saveDatabase(articlesRSS);
		}
		else
		{
			std::cout << "Incorrect input" << std::endl;
		}

		//addArticlesToPocket(newArticles);
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
		try
		{
			UserData user = authenticator.authenticate();		//authenticate by connecting to pocket and redirecting user to their website (uses OAuth 2.0)
			users[user.username] = user.accessToken;
			currentUser = user;
		}
		catch (const std::exception &e)
		{
			std::cout << "Pocket authentication failed: " << e.what() << std::endl;
		}
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

std::vector<ParsedArticle> Application::parseArticles(const std::vector<ArticleRSS>& items)
{
	if (items.size())
	{
		std::cout << "Parsing " << items.size() << " articles" << std::endl;
		auto articles = parser.getParsedArticles(items);						//after parsing ParsedArticle contains all informations about article - title, description, full content in string and xml tree simultaneously etc.

		std::cout << "Filtering articles" << std::endl;
		filter.filterArticles(articles);										//filtering out fragments of articles, removing too short and too long ones

		return articles;
	}

	return std::vector<ParsedArticle>();
}

void Application::createMobi(std::vector<ParsedArticle>& articles)
{
	if (articles.size())
	{
		std::cout << "Creating epub from  " << articles.size() << " articles" << std::endl;
		ebookCreator.createEpub(articles);

		std::cout << "Converting to mobi" << std::endl;
		ebookCreator.convertToMobi();
		//ebookCreator.removeEpub();
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
