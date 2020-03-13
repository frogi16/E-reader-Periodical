#pragma once

#include <filesystem>

#include "URLExtensionDetector.h"

class ImageSaver
{
public:
	ImageSaver();
	void saveImage(const std::string& link, std::filesystem::path& pathToImage);
	~ImageSaver();
private:
	void configureCurlToImageDownloading(const std::string& link, std::filesystem::path& pathToImage);

	CurlWrapper curlWrapper;
	URLExtensionDetector extensionDetector;
};
