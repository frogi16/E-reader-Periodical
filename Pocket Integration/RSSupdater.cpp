#include "RSSupdater.h"

#include <algorithm>

RSSupdater::RSSupdater() noexcept :
	lastUpdate(NULL),
	updateFrequency(60 * 5)
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

	return std::vector<ArticleRSS>();
}

std::vector<ArticleRSS> RSSupdater::forceUpdates()
{
	lastUpdate = time(NULL);
	return downloadFeeds();
}

void RSSupdater::setUpdateFrequencyInSeconds(size_t seconds) noexcept
{
	updateFrequency = seconds;
}

void RSSupdater::setUpdateFrequencyInMinutes(size_t minutes) noexcept
{
	updateFrequency = minutes * 60;
}

void RSSupdater::watchFeed(const std::string& link)
{
	auto position = std::find(linksToFeeds.begin(), linksToFeeds.end(), link);

	if (position == linksToFeeds.end())	//if haven't found link in feeds
	{
		linksToFeeds.push_back(link);
	}
	else
	{
		throw std::string("Feed is already being watched");
	}
}


RSSupdater::~RSSupdater()
{
}

std::vector<ArticleRSS> RSSupdater::downloadFeeds()
{
	std::cout << "Checking for new articles" << std::endl;
	std::vector<ArticleRSS> newItems;

	for (auto& link : linksToFeeds)
	{
		downloader.downloadSource(link);
		std::vector<ArticleRSS> items = feedsDatabase.updateFeed(link, downloader.getData());
		newItems.insert(newItems.end(), items.begin(), items.end());					//inefficient if size of links vector is big. TODO: prevent copying elements of links. Maybe deque?
	}

	feedsDatabase.saveDatabase();

	return newItems;
}