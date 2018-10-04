#pragma once

#include <string>
#include <vector>
#include <ctime>

#include "SourceDownloader.h"
#include "FeedsDatabase.h"

class RSSupdater
{
public:
	RSSupdater();
	std::vector<std::string> checkUpdates();		//returns vector of links to new items
	std::vector<std::string> forceUpdates();		//returns vector of links to new items
	void setUpdateFrequency(double seconds);
	void setUpdateFrequency(int minutes);
	void watchFeed(std::string link);
	~RSSupdater();
private:
	std::vector<std::string> downloadFeeds();		//returns vector of links to new items

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	SourceDownloader downloader;
	FeedsDatabase feedsDatabase;

	std::vector<std::string> feeds;
	time_t lastUpdate;
	double updateFrequency;
};

