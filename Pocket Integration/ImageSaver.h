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
	std::string mimeFromJson(std::string & jsonString);
	std::string mimeFromHTML(std::string & htmlString);
	CurlWrapper curlWrapper;

	static const std::map<std::string, std::string> extensionsFromMIME;
};
