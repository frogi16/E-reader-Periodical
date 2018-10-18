#include "Parser.h"
#include "json.h"

using json = nlohmann::json;

Parser::Parser(std::string mercuryKey) : mMercuryKey(mercuryKey)
{
	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);	//set response string as responses container
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, (std::string("x-api-key: ") + mMercuryKey).c_str());		//set header with api key
	chunk = curl_slist_append(chunk, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
}

std::vector<ParsedArticle> Parser::getParsedArticles(const std::vector<std::string>& links)
{
	std::vector<ParsedArticle> parsedArticles;
	for (auto& link : links)
	{
		callMercury(link);

		auto parsedArticle = parseArticle(response);
		parsedArticles.push_back(parsedArticle);
		response.clear();
	}
	return parsedArticles;
}

void Parser::callMercury(std::string link)
{
	std::string url = std::string("https://mercury.postlight.com/parser") + "?url=" + link;

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	auto result = curl_easy_perform(curl);
}

ParsedArticle Parser::parseArticle(std::string & article)
{
	ParsedArticle parsedArticle;

	auto json = json::parse(response);
	parsedArticle.author = json["author"].get<std::string>();
	parsedArticle.title = json["title"].get<std::string>();
	parsedArticle.content = json["content"].get<std::string>();

	return parsedArticle;
}

Parser::~Parser()
{
}

size_t Parser::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
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
