#include "ArticleAdder.h"


ArticleAdder::ArticleAdder(std::string consumerKey) : mConsumerKey(consumerKey)
{
	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/add");			//where to post	
}

void ArticleAdder::addArticles(std::vector<std::string> links, std::string accessToken)
{
	for (auto& link : links)
	{
		std::string parameters = "url=" + link + "&consumer_key=" + mConsumerKey + "&access_token=" + accessToken;
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());				//what to post
		auto result = curl_easy_perform(handle);
	}	
}

ArticleAdder::~ArticleAdder()
{
}
