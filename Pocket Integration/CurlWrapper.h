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
	void setURL(std::string url);
	void setPostFields(std::string& parameters);
	void setFollowLocation(bool value);
	void setWritingToString();
	void setWritingToFile(std::experimental::filesystem::path &pathToImage);
	void setNoBody(bool value);
	void setHeaderOnly(bool value);
	void resetSlist();
	void addToSlist(std::string value);
	void perform();
	std::string& getResponseString() { return responseString; }
	~CurlWrapper();
private:
	void setResponseString();
	void setHeaderList();

	static size_t curlWriteToString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	CURL *curl;
	CURLcode code;
	char errorBuffer[CURL_ERROR_SIZE];
	struct curl_slist* slist = NULL;

	std::string mParameters;
	ResponseType responseType;
	std::string responseString;
	FILE *fp = NULL;
};

