#pragma once

#include <filesystem>
#include <map>
#include <string>

#include "CurlWrapper.h"

class ImageSaver
{
public:
	ImageSaver();
	void saveImage(const std::string &link, std::experimental::filesystem::path &pathToImage);
	~ImageSaver();
private:
	std::string detectExtenstion(const std::string &link, const std::experimental::filesystem::path &pathToImage);
	std::string mimeFromJson(const std::string & jsonString);
	std::string mimeFromHTML(const std::string & htmlString);
	void configureCurlToHeaderDownloading(const std::string & url);
	void configureCurlToImageDownloading(const std::string &link, std::experimental::filesystem::path &pathToImage);
	CurlWrapper curlWrapper;

	static const std::map<std::string, std::string> extensionsFromMIME;
};
