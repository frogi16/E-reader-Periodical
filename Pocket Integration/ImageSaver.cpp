#include "ImageSaver.h"

#include <iostream>

#include "json.h"
#include <cctype>

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

	CURL *image;
	CURLcode imgresult;
	FILE *fp = NULL;
	errno_t err;

	image = curl_easy_init();
	if (image) {
		// Open file 
		err = fopen_s(&fp, pathToImage.generic_string().c_str(), "wb");
		if (fp == NULL) std::cout << "File cannot be opened";

		curl_easy_setopt(image, CURLOPT_URL, link.c_str());
		curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);


		// Grab image 
		try
		{
			imgresult = curl_easy_perform(image);
		}
		catch (const std::exception& e)
		{
			std::cout << "Error while downloading image. " << e.what() << std::endl;
		}

		if (imgresult)
		{
			std::cout << "Cannot grab the image!\n";
		}
	}

	// Clean up the resources 
	curl_easy_cleanup(image);
	// Close the file 
	fclose(fp);
}

size_t ImageSaver::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
{
	size_t newLength = size * nmemb;
	size_t oldLength = s->size();
	try
	{
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e)
	{
		//handle memory problem
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size * nmemb;
}

ImageSaver::~ImageSaver()
{
}

std::string ImageSaver::detectExtenstion(const std::string & link, const std::experimental::filesystem::path & pathToImage)
{
	CURLcode resultCode;
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);			//set response string as responses container
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

	resultCode = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	std::string extension(".");
	nlohmann::json json;
	std::string MIME;

	try//using JSON
	{
		json = json::parse(response);

		if (!json["Content-Type"].is_null())
		{
			MIME = json["Content-Type"].get<std::string>();
		}
	}
	catch (const std::exception& e)//else try detecting content-type by searching string
	{
		std::string searched("Content-Type:");
		size_t position = response.find(searched);
		if (position != std::string::npos)
		{
			position += searched.size() + 1;
			while (!(std::isspace(static_cast<unsigned char>(response[position]))))
			{
				MIME.push_back(response[position++]);
			}
		}
	}

	extension.append(extensionsFromMIME.at(MIME));
	return extension;
}
