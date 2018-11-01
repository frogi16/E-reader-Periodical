#pragma once

#include <curl/curl.h>

#include <filesystem>
#include <map>
#include <string>

class ImageSaver
{
public:
	ImageSaver();
	void saveImage(const std::string &link, std::experimental::filesystem::path &pathToImage);
	~ImageSaver();
private:
	std::string detectExtenstion(const std::string &link, const std::experimental::filesystem::path &pathToImage);

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	std::string response;

	static const std::map<std::string, std::string> extensionsFromMIME;
};
