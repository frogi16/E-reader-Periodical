#pragma once

/*	
	Reused from my previous project - Scraper. This project was already using libcurl, so I only had to include pugixml. Tidy is not needed because RSS uses XML and no plain HTML has to be tidyied.
	That's why I removed convertToXML() and changed few things. Full source code can be found in Scraper repo.
*/

/*
	This class returns nicely formatted XML using only internet adress of the website. Extracting data is left for other parts of program in order to preserve single responsibility principle.
	
	1 step is connecting to internet and downloading website source. It is handled by CURLlib
	2 step is converting source code into XML which is done using tidy
*/

#include <curl/curl.h>

#include <tidy.h>
#include <tidybuffio.h>

#include "pugixml.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

class SourceDownloader
{
public:
	SourceDownloader() noexcept;
	void downloadSource(std::string url);
	pugi::xml_node getData();
	std::vector<pugi::xml_node> selectData(std::string attributeName=0, std::string attributeValue=0);
	~SourceDownloader();
private:
	static size_t curlWriteToString(void *contents, size_t size, size_t nmemb, std::string *s);
	std::string sourceCode;
	pugi::xml_document document;
};

