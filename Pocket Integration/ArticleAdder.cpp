#include "ArticleAdder.h"


ArticleAdder::ArticleAdder(std::string consumerKey) : mConsumerKey(consumerKey)
{
	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/add");				//where to post																	
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);	//set response string as responses container
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
}

void ArticleAdder::addArticles(std::vector<std::string> links, std::string accessToken)
{
	for (auto& link : links)
	{
		std::string parameters = "url=" + link + "&consumer_key=" + mConsumerKey + "&access_token=" + accessToken;
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());				//what to post
		auto result = curl_easy_perform(handle);
		response;
		bool test = true;
	}	
}

ArticleAdder::~ArticleAdder()
{
}

size_t ArticleAdder::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
{
	size_t newLength = size * nmemb;
	size_t oldLength = s->size();
	try
	{
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e)
	{
		//handle memory problem
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size * nmemb;
}