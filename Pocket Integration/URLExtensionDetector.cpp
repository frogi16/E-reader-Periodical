#include "URLExtensionDetector.h"

#include <iostream>
#include <cctype>
#include <string>

#include "json.h"

using json = nlohmann::json;

const std::map<std::string, std::string> URLExtensionDetector::extensionsFromMIME = {
	{ "image/tiff", ".tiff" },
	{ "image/gif", ".gif" },
	{ "image/jpeg", ".jpg" },
	{ "image/png", ".png" },
	{ "image/svg+xml", ".svg" },
};

URLExtensionDetector::URLExtensionDetector()
{
	curlWrapper.setWritingToString();
	curlWrapper.setHeaderOnly(true);
	curlWrapper.setNoBody(true);
	curlWrapper.addToSlist("Content-Type: application/json");
}

std::string URLExtensionDetector::get(const std::string & link)
{
	configureCurlToHeaderDownloading(link);
	curlWrapper.perform();
	std::string response = curlWrapper.getResponseString();
	std::string MIME = detectExtension(response);	

	auto foundMIME = extensionsFromMIME.find(MIME);								//if map doesn't contain MIME program can't correctly detect extension
	if (foundMIME == extensionsFromMIME.end())
		throw std::invalid_argument(std::string("Detecting the extension failed: " + link));
	else
		return  (*foundMIME).second;
}

void URLExtensionDetector::configureCurlToHeaderDownloading(const std::string & url)
{
	curlWrapper.setURL(url);
}

std::string URLExtensionDetector::detectExtension(const std::string & header)
{
	std::string MIME;

	try//using JSON
	{
		MIME = mimeFromJsonHeader(header);
	}
	catch (const std::exception& e)												//otherwise try detecting content-type by searching string
	{
		MIME = mimeFromHTMLHeader(header);
	}

	return MIME;
}

std::string URLExtensionDetector::mimeFromJsonHeader(const std::string & header)
{
	nlohmann::json json = json::parse(header);

	if (!json["Content-Type"].is_null())
		return json["Content-Type"].get<std::string>();
	else
		throw std::exception("Couldn't identify content-type");
}

std::string URLExtensionDetector::mimeFromHTMLHeader(const std::string & header)
{
	std::string searched("Content-Type:");
	std::string result;

	size_t position = header.find(searched);									//get position of first occurence of searched string

	if (position == std::string::npos)											//nothing was found
	{
		throw std::invalid_argument("Couldn't identify content-type");
	}
	else
	{
		position += searched.size() + 1;										//go one character after end of searched string
		while (!std::isspace(static_cast<unsigned char>(header[position])))		//iterate until you encounter space character
		{
			result.push_back(header[position++]);
		}
	}

	return result;
}

URLExtensionDetector::~URLExtensionDetector()
{
}
