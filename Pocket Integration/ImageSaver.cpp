#include "ImageSaver.h"

#include <iostream>

ImageSaver::ImageSaver()
{
}
void ImageSaver::saveImage(std::string link, std::experimental::filesystem::path pathToImage)
{
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
		imgresult = curl_easy_perform(image);
		if (imgresult) {
			std::cout << "Cannot grab the image!\n";
		}
	}

	// Clean up the resources 
	curl_easy_cleanup(image);
	// Close the file 
	fclose(fp);
}

ImageSaver::~ImageSaver()
{
}
