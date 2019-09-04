#pragma once

#include <string>

#include <curl/curl.h>

#include "CurlWrapper.h"
#include "MyServer.h"

struct UserData
{
	std::string accessToken;
	std::string username;
};

class Authenticator
{
public:
	Authenticator(const std::string& consumerKey);
	UserData authenticate();
	void receivedCallback(web::uri address);
	~Authenticator();
private:
	UserData loadUserFromFile(std::string filepath);
	void saveUserToFile(std::string filepath, const UserData& userData);

	void getRequestToken();
	void getAccessToken(std::string requestToken);
	void makePOST(std::string url, std::string parameters);														//function makes POST to sent site with sent parameters using previously set settings - headers, write function etc.

	std::string extractRequestToken();
	UserData extractAccessTokenAndUsername();

	std::unique_ptr<MyServer> server;
	CurlWrapper curlWrapper;
	std::string mConsumerKey;
	std::string response;

	void listen(utility::string_t address);

	utility::string_t callbackString_t;
	std::string callbackString;

	bool recievedCallback = false;
};

