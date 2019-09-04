#include "Authenticator.h"

#include "json_eprd.h"

#include <fstream>

using njson = nlohmann::json;

Authenticator::Authenticator(const std::string& consumerKey) : mConsumerKey(consumerKey)
{
	curlWrapper.setWritingToString();
	curlWrapper.addToSlist("Content-Type: application/json; charset=UTF8");
	curlWrapper.addToSlist("X-Accept: application/json");

	callbackString_t = utility::string_t(L"http://localhost:21568/callback/");
	callbackString = utility::conversions::to_utf8string(callbackString_t);
}

UserData Authenticator::authenticate()
{
	std::string pathToUserFile("user.txt");
	UserData userData = loadUserFromFile(pathToUserFile);

	if (userData.username.size() && userData.accessToken.size())
	{
		std::cout << "Using saved user credentials" << std::endl;
		return userData;
	}
	else
	{
		std::cout << "Authenticating using Pocket API" << std::endl;
		try
		{
			getRequestToken();
			std::string requestToken = extractRequestToken();

			//redirecting user to pocket site in order to log in to account
			ShellExecute(0, 0, std::string("https://getpocket.com/auth/authorize?request_token=" + requestToken + "&redirect_uri=" + callbackString).c_str(), 0, 0, SW_SHOW);

			//run server
			listen(callbackString_t);

			while (!recievedCallback)
			{
				Sleep(100);
			}

			getAccessToken(requestToken);
			UserData userData = extractAccessTokenAndUsername();
			saveUserToFile(pathToUserFile, userData);
			return userData;
		}
		catch (const std::exception& e)
		{
			std::cout << "Authentication failed: " << e.what() << std::endl;
			throw;
		}
	}
}

void Authenticator::getRequestToken()
{
	njson jsonParameters;
	jsonParameters["consumer_key"] = mConsumerKey;
	jsonParameters["redirect_uri"] = callbackString;

	std::string parameters = jsonParameters.dump();
	return makePOST("https://getpocket.com/v3/oauth/request", parameters);
}

void Authenticator::getAccessToken(std::string requestToken)
{
	njson jsonParameters;
	jsonParameters["consumer_key"] = mConsumerKey;
	jsonParameters["code"] = requestToken;

	std::string parameters = jsonParameters.dump();
	return makePOST("https://getpocket.com/v3/oauth/authorize", parameters);
}

void Authenticator::makePOST(std::string url, std::string parameters)
{
	curlWrapper.setURL(url);
	curlWrapper.setPostFields(parameters);
	curlWrapper.perform();
}

std::string Authenticator::extractRequestToken()
{
	auto json = njson::parse(curlWrapper.getResponseString());
	return json["code"].get<std::string>();
}

UserData Authenticator::extractAccessTokenAndUsername()
{
	UserData data;

	auto json = njson::parse(curlWrapper.getResponseString());
	data.accessToken = json["access_token"].get<std::string>();
	data.username = json["username"].get<std::string>();

	return data;
}

void Authenticator::receivedCallback(web::uri address)
{
	if (address.to_string() == callbackString_t)	//if addresses match
	{
		recievedCallback = true;					//flag used by authenticate(). I had a problem determining the best approach to do it. This object needs to make one request more and return
													//some data to main Application, so the simplest solution is to return it at the end of authenticate(). It stops execution of whole program though.
													//If parallel execution were needed it would be recommended to use some kind of callback to notify Application about result of authentication.

		server->close().wait();						//stop listening
	}
}


Authenticator::~Authenticator()
{
}

UserData Authenticator::loadUserFromFile(std::string filepath)
{
	std::fstream loginFile;
	loginFile.open(filepath.c_str(), std::ios::in);
	UserData user;
	loginFile >> user.accessToken >> user.username;
	loginFile.close();
	return user;
}

void Authenticator::saveUserToFile(std::string filepath, const UserData& userData)
{
	std::fstream loginFile;
	loginFile.open(filepath, std::ios::out);
	loginFile << userData.accessToken << " " << userData.username;
	loginFile.close();
}

void Authenticator::listen(utility::string_t address)
{
	server = std::unique_ptr<MyServer>(new MyServer(address, this));
	server->open().wait();
}
