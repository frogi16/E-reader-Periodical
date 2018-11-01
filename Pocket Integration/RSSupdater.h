#pragma once

#include <string>
#include <vector>
#include <ctime>

#include "SourceDownloader.h"
#include "FeedsDatabase.h"

class RSSupdater
{
public:
	RSSupdater() noexcept;
	std::vector<ArticleRSS> checkUpdates();					//updates if time from last update is bigger than updateFrequency. Returns vector of links to new items
	std::vector<ArticleRSS> forceUpdates();					//updates no matter how long ago was the last update. Returns vector of links to new items
	void setUpdateFrequencyInSeconds(size_t seconds);
	void setUpdateFrequencyInMinutes(size_t minutes);
	void watchFeed(std::string link);
	~RSSupdater();
private:
	std::vector<ArticleRSS> downloadFeeds();					//returns vector of links to new items so they can be passed on to the checkUpdates (or forceUpdates)

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	SourceDownloader downloader;
	FeedsDatabase feedsDatabase;

	std::vector<std::string> feeds;
	time_t lastUpdate;
	size_t updateFrequency;
};

