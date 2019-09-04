#include "ArticleAdder.h"

#include "json_eprd.h"

using njson = nlohmann::json;

ArticleAdder::ArticleAdder(std::string consumerKey) : mConsumerKey(consumerKey)
{
	curlWrapper.setWritingToString();
	curlWrapper.addToSlist("Content-Type: application/json; charset=UTF8");
	curlWrapper.addToSlist("X-Accept: application/json");
	curlWrapper.setURL("https://getpocket.com/v3/add");
}

void ArticleAdder::addArticles(const std::vector<std::string>& links, const std::string& accessToken)
{
	for (auto& link : links)
	{
		njson jsonParameters;
		jsonParameters["consumer_key"] = mConsumerKey;
		jsonParameters["url"] = link;
		jsonParameters["access_token"] = accessToken;

		std::string parameters = jsonParameters.dump();

		curlWrapper.setPostFields(parameters);
		curlWrapper.perform();
	}
}

ArticleAdder::~ArticleAdder()
{
}