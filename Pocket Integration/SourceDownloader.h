#pragma once

/*	
	Reused from my Scrapper project. I was already using libcurl in this project, so there is only one new library - pugixml. Tidy is not needed because RSS uses plain XML, no HTML.
	That's why I commented out convertToXML(). Without formatting and tidying converting can be done in one line.
	TODO: Research if there is any way to avoid using std::string sourceCode and download source directly into pugi document.
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

class SourceDownloader
{
public:
	SourceDownloader();
	void downloadSource(std::string url);
	//void convertToXML();					//tidy xml is
	pugi::xml_node getData();
	std::vector<pugi::xml_node> selectData(std::string attributeName=0, std::string attributeValue=0);
	~SourceDownloader();
private:
	static size_t curlWriteToString(void *contents, size_t size, size_t nmemb, std::string *s);
	std::string sourceCode;
	//TidyBuffer XMLdata = { 0 };
	pugi::xml_document document;
};

