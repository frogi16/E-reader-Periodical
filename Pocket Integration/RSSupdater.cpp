#include "RSSupdater.h"

#include <algorithm>

RSSupdater::RSSupdater() noexcept :
	lastUpdate(NULL),
	updateFrequency(60*5)
{
}

std::vector<ArticleRSS> RSSupdater::checkUpdates()
{
	time_t now;
	now = time(NULL);

	double elapsedTime = difftime(now, lastUpdate);

	if (elapsedTime >= updateFrequency)
	{
		lastUpdate = now;
		return downloadFeeds();
	}

	return std::vector<ArticleRSS> ();
}

std::vector<ArticleRSS> RSSupdater::forceUpdates()
{
	lastUpdate = time(NULL);
	return downloadFeeds();
}

void RSSupdater::setUpdateFrequencyInSeconds(size_t seconds)
{
	updateFrequency = seconds;
}

void RSSupdater::setUpdateFrequencyInMinutes(size_t minutes)
{
	updateFrequency = minutes * 60;
}

void RSSupdater::watchFeed(std::string link)
{
	auto iterator = std::find(feeds.begin(), feeds.end(), link);

	if (iterator == feeds.end())	//if haven't found link in feeds
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

std::vector<ArticleRSS> RSSupdater::downloadFeeds()
{
	std::cout << "Checking for new articles" << std::endl;
	std::vector<ArticleRSS> newItems;

	for (auto& feed : feeds)
	{
		downloader.downloadSource(feed);
		std::vector<ArticleRSS> items = feedsDatabase.updateFeed(feed, downloader.getData());
		newItems.insert(newItems.end(), items.begin(), items.end());					//inefficient if size of links vector is big. TODO: prevent copying elements of links. Maybe deque?
	}

	feedsDatabase.saveDatabase();

	return newItems;
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