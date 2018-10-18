#include "ArticleAdder.h"
#include "json.h"

using njson = nlohmann::json;

ArticleAdder::ArticleAdder(std::string consumerKey) : mConsumerKey(consumerKey)
{
	handle = curl_easy_init();

	//set response string as responses container
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

	//set headers: parameters will be transmitted in form of JSON in UTF8 and site is expected to respond in JSON as well
	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, "Content-Type: application/json; charset=UTF8");
	chunk = curl_slist_append(chunk, "X-Accept: application/json");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, chunk);
}

void ArticleAdder::addArticles(const std::vector<std::string> & links, std::string accessToken) const
{
	for (auto& link : links)
	{
		njson jsonParameters;
		jsonParameters["consumer_key"] = mConsumerKey;
		jsonParameters["url"] = link;
		jsonParameters["access_token"] = accessToken;

		std::string parameters = jsonParameters.dump();

		curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/add");			//where to post
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());				//what to post
		auto result = curl_easy_perform(handle);
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
