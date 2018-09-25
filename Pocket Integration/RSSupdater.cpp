#include "RSSupdater.h"

#include <algorithm>

RSSupdater::RSSupdater() : lastUpdate(time(NULL))
{
	downloadFeeds();		//skip checkUpdate(), just download feeds
}

void RSSupdater::checkUpdates()
{
	time_t now;
	now = time(NULL);

	double elapsedTime = difftime(now, lastUpdate);

	if (elapsedTime >= updateFrequency)
	{
		lastUpdate = now;
		downloadFeeds();
	}
}

void RSSupdater::forceUpdates()
{
	lastUpdate = time(NULL);
	downloadFeeds();
}

void RSSupdater::setUpdateFrequency(double seconds)
{
	updateFrequency = seconds;
}

void RSSupdater::setUpdateFrequency(int minutes)
{
	updateFrequency = minutes * 60;
}

void RSSupdater::watchFeed(std::string link)
{
	auto iterator = std::find(feeds.begin(), feeds.end(), link);

	if (iterator == feeds.end())	//haven't found link in feeds
	{
		feeds.push_back(link);
	}
	else
	{
		throw std::string("Feed already being watched");
	}
}


RSSupdater::~RSSupdater()
{
}

void RSSupdater::downloadFeeds()
{
	for (auto& feed : feeds)
	{
		downloader.downloadSource(feed);
		downloader.convertToXML();
		
		auto data = downloader.getData();
	}
}

size_t RSSupdater::CurlWrite_CallbackFunc_StdString(void * contents, size_t size, size_t nmemb, std::string * s)
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