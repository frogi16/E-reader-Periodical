#pragma once

#include <string>
#include <vector>
#include <ctime>

#include "SourceDownloader.h"

class RSSupdater
{
public:
	RSSupdater();
	void checkUpdates();
	void forceUpdates();
	void setUpdateFrequency(double seconds);
	void setUpdateFrequency(int minutes);
	void watchFeed(std::string link);
	~RSSupdater();
private:
	void downloadFeeds();

	static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);		//needs to be static

	SourceDownloader downloader;
	std::vector<std::string> feeds;
	time_t lastUpdate;
	double updateFrequency;
};

