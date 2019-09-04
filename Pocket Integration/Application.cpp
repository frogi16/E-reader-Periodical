#include "Application.h"

#include <iostream>

#include "CommandInterpreter.h"

Application::Application() noexcept :
	authenticator(keyHolder.pocketKey),
	adder(keyHolder.pocketKey),
	pocketRetriever(keyHolder.pocketKey),
	parser()
{
	srand(static_cast<unsigned int>(time(NULL)));
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

	CommandInterpreter commandIntepreter;
	addCommands(commandIntepreter, articlesRSS, articles);						//articlesRSS and articles are required because they need to be captured by callback function invoked by Command

	commandIntepreter.interpret("help");										//prompt instructions for user at the beginning

	for (std::string input; true;)
	{
		std::cout << std::endl << "There are " << articlesRSS.size() << " fresh articles and " << articles.size() << " of them are already parsed!" << std::endl << ":\\>";
		std::getline(std::cin, input);

		auto interpretationResult = commandIntepreter.interpret(input);
		promptCommandInterpretationStatus(interpretationResult);
	}
}

void Application::addCommands(CommandInterpreter& commandInterpreter, std::vector<ArticleRSS>& articlesRSS, std::vector<ParsedArticle>& articles)
{
	using CmdType = eprd::CommandType;

	commandInterpreter.addIfUnique(Command("help", [&](std::vector<ParamVariant>&)
	{
		std::cout << "Type:" << std::endl;
		std::cout << "	\"update\" - check RSS and Pocket for new articles" << std::endl;
		std::cout << "	\"parse\" - process articles for further filtering and ebook generation" << std::endl;
		//std::cout << "	\"select [NUMBER]\" - take specififc number of already parsed articles" << std::endl;
		std::cout << "	\"filter\" - filter parsed articles" << std::endl;
		std::cout << "	\"book\" - create epub and mobi files" << std::endl;
		std::cout << "	\"help\" - display this text anytime you like" << std::endl;
	}), CmdType::Help);

	commandInterpreter.addIfUnique(Command("update", [&](std::vector<ParamVariant>&)
	{
		auto newArticles = pocketRetriever.retrieveArticles(currentUser.accessToken);
		articlesRSS.insert(articlesRSS.end(), newArticles.begin(), newArticles.end());

		newArticles = checkRSS();
		articlesRSS.insert(articlesRSS.end(), newArticles.begin(), newArticles.end());
		articlesDatabase.saveDatabase(articlesRSS);
	}), CmdType::Update);

	commandInterpreter.addIfUnique(Command("parse", [&](std::vector<ParamVariant>&)
	{
		articles = parseArticles(articlesRSS);
	}), CmdType::Parse);

	commandInterpreter.addIfUnique(Command("filter", [&](std::vector<ParamVariant>&)
	{
		filterArticles(articles);
	}), CmdType::Filter);

	Command selectCmd("select", [&](std::vector<ParamVariant>& params)
	{
		size_t quantity = std::get<int>(params[0]);

		if (quantity < 0 || quantity > articlesRSS.size())
			std::cout << "Invalid quantity of articles!" << std::endl;
		else
		{
			articlesRSS.erase(articlesRSS.begin(), articlesRSS.begin() + quantity);
			articles.erase(articles.begin() + quantity, articles.end());

			articlesDatabase.saveDatabase(articlesRSS);
		}

	});

	selectCmd.addExpectedParameter(eprd::TokenType::Integer);
	commandInterpreter.addIfUnique(std::move(selectCmd), CmdType::Select);

	commandInterpreter.addIfUnique(Command("book", [&](std::vector<ParamVariant>&)
	{
		createMobi(articles);
		articles.clear();
		articlesRSS.clear();
		articlesDatabase.saveDatabase(articlesRSS);
	}), CmdType::Book);
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
		catch (const std::exception& e)
		{
			std::cout << "Pocket authentication failed: " << e.what() << std::endl;
		}
	}
}

std::vector<ArticleRSS> Application::checkRSS()
{
	return updater.checkUpdates();
}

void Application::addArticlesToPocket(const std::vector<std::string>& urls)
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
		return articles;
	}

	return std::vector<ParsedArticle>();
}

void Application::filterArticles(std::vector<ParsedArticle>& articles)
{
	if (articles.size())
	{
		std::cout << "Filtering articles" << std::endl;
		filter.filterArticles(articles);										//filtering out fragments of articles, removing too short and too long ones
	}
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

	for (std::string feed; !watchedFeeds.eof(); updater.watchFeed(feed))
		watchedFeeds >> feed;
}

void Application::promptCommandInterpretationStatus(const eprd::InterpretationResult& interpretation) const
{
	using Status = eprd::InterpretationStatus;

	switch (interpretation.status)
	{
	case Status::Success:		//left blank deliberately, if command succeeded no additional messages should be prompted.
		break;
	case Status::CommandNotFound:
		std::cout << "Command couldn't be find!" << std::endl;
		break;
	case Status::TooFewParameters:
		std::cout << "Too few parameters were delivered." << std::endl;
		break;
	case Status::TooManyParameters:
		std::cout << "Too many parameters were delivered." << std::endl;
		break;
	case Status::ParameterTypesInvalid:
		std::cout << "Types of delivered parameters don't match expected ones." << std::endl;
		break;
	default:
		break;
	}
}
