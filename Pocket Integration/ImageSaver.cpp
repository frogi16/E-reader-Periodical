#include "ImageSaver.h"

#include <iostream>
#include <cctype>
#include <string>
#include "json.h"

using json = nlohmann::json;

const std::map<std::string, std::string> ImageSaver::extensionsFromMIME = {
	{ "image/tiff", "TIFF" },
	{ "image/gif", "GIF" },
	{ "image/jpeg", "JPG" },
	{ "image/png", "PNG" },
	{ "image/svg+xml", "SVG" },
};

ImageSaver::ImageSaver()
{
}

void ImageSaver::saveImage(const std::string &link, std::experimental::filesystem::path &pathToImage)
{
	pathToImage.replace_extension(detectExtenstion(link, pathToImage));
	configureCurlToImageDownloading(link, pathToImage);
	curlWrapper.perform();
}

ImageSaver::~ImageSaver()
{
}

std::string ImageSaver::detectExtenstion(const std::string & link, const std::experimental::filesystem::path & pathToImage)
{
	std::string extension(".jpg");
	std::string MIME;

	try
	{
		configureCurlToHeaderDownloading(link);
		curlWrapper.perform();

		std::string response = curlWrapper.getResponseString();

		try//using JSON
		{
			MIME = mimeFromJson(response);
		}
		catch (const std::exception& e)//else try detecting content-type by searching string
		{
			MIME = mimeFromHTML(response);
		}

		if (extensionsFromMIME.count(MIME))
			extension = "." + extensionsFromMIME.at(MIME);
	}
	catch (const std::exception& e)
	{
		std::cout << "Couldn't download header: " << link << std::endl << e.what() << std::endl << "Attempting to use default .jpg extension." << std::endl;
	}

	return extension;
}

std::string ImageSaver::mimeFromJson(const std::string & jsonString)
{
	nlohmann::json json = json::parse(jsonString);

	if (!json["Content-Type"].is_null())
	{
		return json["Content-Type"].get<std::string>();
	}
	else
	{
		throw std::exception("Couldn't identify content-type");
	}
}

std::string ImageSaver::mimeFromHTML(const std::string & htmlString)
{
	std::string searched("Content-Type:");
	std::string result;

	size_t position = htmlString.find(searched);				//get position of first occurence of searched string
	if (position != std::string::npos)
	{
		position += searched.size() + 1;						//go one character after end of searched string
		while (!std::isspace(static_cast<unsigned char>(htmlString[position])))	//iterate until you encounter space character
		{
			result.push_back(htmlString[position++]);
		}
	}

	return result;
}

void ImageSaver::configureCurlToHeaderDownloading(const std::string & url)
{
	curlWrapper.reset();
	curlWrapper.setWritingToString();
	curlWrapper.setURL(url);
	curlWrapper.setHeaderOnly(true);
	curlWrapper.setNoBody(true);
	curlWrapper.addToSlist("Content-Type: application/json");
}

void ImageSaver::configureCurlToImageDownloading(const std::string & link, std::experimental::filesystem::path & pathToImage)
{
	curlWrapper.reset();
	curlWrapper.setWritingToFile(pathToImage);
	curlWrapper.setURL(link);
}
