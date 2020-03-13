#include "RSSupdater.h"

#include <algorithm>
#include <execution>
#include <mutex>

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

std::vector<ArticleRSS> RSSupdater::downloadFeeds()
{
	std::cout << "Checking for new articles" << std::endl;

	std::vector<ArticleRSS> newArticles;
	std::mutex newArticlesMutex;

	//if feed is not known, updateFeed() will add new object to its internal storage. It is not safe to modify container from multiple threads at once, so every feed
	//has to be saved in database before updateFeed() execution in order to achieve thread-safety and shorten critical path to minimum
	for (auto& link : linksToFeeds)
		feedsDatabase.createFeedIfNeeded(link);

	std::for_each(std::execution::par, linksToFeeds.begin(), linksToFeeds.end(),
		[&newArticles, &newArticlesMutex, &feedsDatabase = feedsDatabase](std::string link)
	{
		SourceDownloader srcDownloader;
		srcDownloader.downloadSource(link);
		std::vector<ArticleRSS> items = feedsDatabase.updateFeed(link, srcDownloader.getData());

		std::lock_guard lock(newArticlesMutex);
		newArticles.insert(newArticles.end(), items.begin(), items.end());
	});

	feedsDatabase.saveDatabase();

	return newArticles;
}

RSSupdater::~RSSupdater()
{
}