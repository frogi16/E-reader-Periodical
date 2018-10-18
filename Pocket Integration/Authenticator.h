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
	UserData loadUserFromFile(std::string filepath);
	void saveUserToFile(std::string filepath, const UserData& userData);

	CURLcode getRequestToken();
	CURLcode getAccessToken(std::string requestToken);
	CURLcode makePOST(std::string url, std::string parameters);														//function makes POST to sent site with sent parameters using previously set settings - headers, write function etc.

	std::string extractRequestToken(std::string source);
	UserData extractAccessTokenAndUsername(std::string source);

	std::unique_ptr<MyServer> server;
	CURL* handle;
	std::string mConsumerKey;
	std::string response;	

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//function writing web response to string. Needs to be static
	void listen(utility::string_t address);

	utility::string_t callbackString_t;
	std::string callbackString;

	bool recievedCallback = false;
};

