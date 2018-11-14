#include "PocketRetriever.h"
#include "json.h"

using json = nlohmann::json;

PocketRetriever::PocketRetriever(std::string consumerKey) : mConsumerKey(consumerKey)
{
	curlModifier.setWritingToString();
	curlModifier.addToSlist("Content-Type: application/json; charset=UTF8");
	curlModifier.addToSlist("X-Accept: application/json");
	curlModifier.setURL("https://getpocket.com/v3/send");

	curlRetriever.setWritingToString();
	curlRetriever.addToSlist("Content-Type: application/json; charset=UTF8");
	curlRetriever.addToSlist("X-Accept: application/json");
	curlRetriever.setURL("https://getpocket.com/v3/get");
}

std::vector<ArticleRSS> PocketRetriever::retrieveArticles(std::string accessToken)
{
	json jsonParameters;
	jsonParameters["consumer_key"] = mConsumerKey;
	jsonParameters["access_token"] = accessToken;
	jsonParameters["count"] = 15;
	std::string parameters = jsonParameters.dump();

	curlRetriever.setPostFields(parameters);
	curlRetriever.perform();
	
	auto response = curlRetriever.getResponseString();

	auto IDs = extractItemIDs(response);
	archiveArticles(accessToken, IDs);

	return extractLinks(response);
}


PocketRetriever::~PocketRetriever()
{
}

std::vector<ArticleRSS> PocketRetriever::extractLinks(std::string & input)
{
	std::vector<ArticleRSS> articles;
	auto json = json::parse(input);

	if (!json["list"].is_null())
	{
		for (auto& item : json["list"])
		{
			if (!item["given_url"].is_null())
			{
				ArticleRSS article;
				article.link = item["given_url"].get<std::string>();
				articles.push_back(article);
			}
		}
	}

	return articles;
}

std::vector<std::string> PocketRetriever::extractItemIDs(std::string & input)
{
	std::vector<std::string> IDs;
	auto json = json::parse(input);

	if (!json["list"].is_null())
	{
		for (auto& item : json["list"])
		{
			if (!item["item_id"].is_null())
			{
				IDs.push_back(item["item_id"].get<std::string>());
			}
		}
	}

	return IDs;
}

void PocketRetriever::archiveArticles(std::string accessToken, std::vector<std::string> itemIDs)
{
	json jsonParameters;
	jsonParameters["consumer_key"] = mConsumerKey;
	jsonParameters["access_token"] = accessToken;

	for (auto& ID : itemIDs)
	{
		jsonParameters["actions"].push_back({ { "action", "archive" },{ "item_id", ID } });
	}

	std::string parameters = jsonParameters.dump();

	curlModifier.setPostFields(parameters);
	curlModifier.perform();
}
