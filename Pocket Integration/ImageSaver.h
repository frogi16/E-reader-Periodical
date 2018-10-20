#pragma once

#include <curl/curl.h>

#include <filesystem>

class ImageSaver
{
public:
	ImageSaver();
	void saveImage(std::string link, std::experimental::filesystem::path pathToImage);
	~ImageSaver();
};

