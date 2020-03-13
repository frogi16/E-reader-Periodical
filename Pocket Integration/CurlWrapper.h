#pragma once

#include <curl/curl.h>
#include <string>
#include <filesystem>

enum class ResponseType
{
	String,
	File
};

class CurlWrapper
{
public:
	CurlWrapper();
	void reset();
	void setURL(const std::string& url);										//CURLOPT_URL
	void setPostFields(const std::string& parameters);							//CURLOPT_POSTFIELDS
	void setFollowLocation(bool value);											//CURLOPT_FOLLOWLOCATION
	void setWritingToString();													//CURLOPT_WRITEFUNCTION and CURLOPT_WRITEDATA
	void setWritingToFile(std::filesystem::path& pathToImage);					//CURLOPT_WRITEFUNCTION and CURLOPT_WRITEDATA
	void setNoBody(bool value);													//CURLOPT_NOBODY
	void setHeaderOnly(bool value);												//CURLOPT_HEADER
	void setPassword(const std::string& password);								//CURLOPT_PASSWORD
	void resetSlist();															//curl_slist_free_all
	void addToSlist(std::string value);											//curl_slist_append
	void perform();																//CURLOPT_HTTPHEADER and curl_easy_perform
	std::string& getResponseString() { return responseString; }
	~CurlWrapper();
private:
	void setResponseString();													//CURLOPT_WRITEDATA
	void setHeaderList();														//CURLOPT_HTTPHEADER

	static size_t curlWriteToString(void* contents, size_t size, size_t nmemb, std::string* s);		//needs to be static

	CURL* curl;
	CURLcode code;
	char errorBuffer[CURL_ERROR_SIZE];
	struct curl_slist* slist = NULL;

	std::string mParameters;
	ResponseType responseType;
	std::string responseString;
	FILE* fp = NULL;
};

