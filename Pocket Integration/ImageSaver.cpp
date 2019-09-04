#include "ImageSaver.h"

#include <iostream>
#include <cctype>
#include <string>

#include "json_eprd.h"

using json = nlohmann::json;

ImageSaver::ImageSaver()
{
}

void ImageSaver::saveImage(const std::string& link, std::experimental::filesystem::path& pathToImage)
{
	std::string extension = extensionDetector.get(link);
	pathToImage.replace_extension(extension);
	configureCurlToImageDownloading(link, pathToImage);
	curlWrapper.perform();
}

ImageSaver::~ImageSaver()
{
}

void ImageSaver::configureCurlToImageDownloading(const std::string& link, std::experimental::filesystem::path& pathToImage)
{
	curlWrapper.setWritingToFile(pathToImage);
	curlWrapper.setURL(link);
}
