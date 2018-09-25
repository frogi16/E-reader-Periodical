#pragma once

#include <string>

#include <curl/curl.h>

#include "MyServer.h"

struct UserData
{
	std::string accessToken;
	std::string username;
};

class Authenticator
{
public:
	Authenticator(std::string consumerKey);
	UserData authenticate();
	void receivedCallback(web::uri address);
	~Authenticator();
private:
	CURLcode getRequestToken();
	CURLcode getAccessToken(std::string requestToken);

	std::string extractRequestToken(std::string source);
	std::pair<std::string, std::string> extractAccessTokenAndUsername(std::string source);

	std::unique_ptr<MyServer> server;
	CURL* handle;
	std::string mConsumerKey;
	std::string response;	

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static
	void listen(utility::string_t address);

	utility::string_t scheme = U("http");
	utility::string_t host = U("localhost");
	int port = 21568;
	utility::string_t path = U("/callback/");
	web::uri_builder callbackURI;
	utility::string_t callbackString_t;
	std::string callbackString;

	bool gotCallback = false;
};

