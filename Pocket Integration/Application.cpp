#include "Application.h"



Application::Application() : authenticator(consumerKey)
{
	curl_global_init(CURL_GLOBAL_ALL);
	updater.setUpdateFrequency(5);
	updater.watchFeed(std::string("https://www.imperiumromanum.edu.pl/feed/"));
}

void Application::run()
{
	authenticateConnection();
	checkRSS();
	addArticles();
	addArticle(std::string("https://jadisco.pl/"));
}

void Application::authenticateConnection()
{
	if (users.size() > 0)
	{
		currentUser.username = users.begin()->first;
		currentUser.accessToken = users.begin()->second;
	}
	else
	{
		auto data = authenticator.authenticate();
		users[data.username] = data.accessToken;
		currentUser.username = data.username;
		currentUser.accessToken = data.accessToken;
	}
}

void Application::checkRSS()
{
	updater.forceUpdates();
	//updater.checkUpdates();
}

void Application::addArticles()
{
}

void Application::addArticle(std::string url)
{
	CURL* handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/add");			//where to post	
	std::string parameters = "url=" + url + "&consumer_key=" + consumerKey + "&access_token=" + currentUser.accessToken;
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());				//what to post
	auto result = curl_easy_perform(handle);
}


Application::~Application()
{
}
