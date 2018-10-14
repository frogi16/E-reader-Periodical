#include "Authenticator.h"

#include <fstream>

Authenticator::Authenticator(std::string consumerKey) : mConsumerKey(consumerKey)
{	
	//initialize curl
	handle = curl_easy_init();

	//set response string as responses container
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

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
		CURLcode requestResult = getRequestToken();

		if (requestResult == CURLcode::CURLE_OK)
		{
			std::string requestToken = extractRequestToken(response);

			//redirecting user to pocket site in order to log in to account
			ShellExecute(0, 0, std::string("https://getpocket.com/auth/authorize?request_token=" + requestToken + "&redirect_uri=" + callbackString).c_str(), 0, 0, SW_SHOW);

			//run server
			listen(callbackString_t);

			while (!recievedCallback)
			{
				Sleep(100);
			}

			requestResult = getAccessToken(requestToken);

			if (requestResult == CURLcode::CURLE_OK)
			{
				UserData userData = extractAccessTokenAndUsername(response);
				saveUserToFile(pathToUserFile, userData);
				return userData;
			}
		}
	}
}

CURLcode Authenticator::getRequestToken()
{
	curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/oauth/request");				//where to post
	std::string parameters = "consumer_key=" + mConsumerKey + "&redirect_uri=" + callbackString;
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());								//what to post
	return curl_easy_perform(handle);
}

CURLcode Authenticator::getAccessToken(std::string requestToken)
{
	curl_easy_setopt(handle, CURLOPT_URL, "https://getpocket.com/v3/oauth/authorize");
	std::string parameters = "consumer_key=" + mConsumerKey + "&code=" + requestToken;
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, parameters.c_str());
	return curl_easy_perform(handle);
}

std::string Authenticator::extractRequestToken(std::string source)
{
	source.erase(0, 5);											//erasing "code=" at the beginning
	return source;
}

UserData Authenticator::extractAccessTokenAndUsername(std::string source)
{
	int equalSign = 0;
	bool saveAccessToken = false;
	bool saveUsername = false;

	std::string accessToken, username;

	for (size_t i = 0; i < response.size(); i++)				//cutting string from '=' to '&'
	{
		if (response[i] == '&')
		{
			saveAccessToken = false;
		}

		if (saveAccessToken)
		{
			accessToken.push_back(response[i]);
		}

		if (saveUsername)
		{
			username.push_back(response[i]);
		}

		if (response[i] == '=')
		{
			equalSign++;

			if (equalSign == 2)
			{
				saveAccessToken = true;
			}
			else if (equalSign == 3)
			{
				saveUsername = true;
			}
		}
	}
	return UserData{ accessToken, username };
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
	curl_easy_cleanup(handle);
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
	loginFile << userData.accessToken<< " " << userData.username;
	loginFile.close();
}

size_t Authenticator::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
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

void Authenticator::listen(utility::string_t address)
{
	server = std::unique_ptr<MyServer>(new MyServer(address, this));
	server->open().wait();
}
