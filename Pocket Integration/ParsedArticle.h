#pragma once

#include <memory>

#include "pugixml.hpp"

struct ParsedArticle
{
	ParsedArticle() : wordCount(0)
	{
		filteringDescription = std::make_shared<std::string>();
	}

	std::string title;
	std::string author;
	std::string content;
	std::string domain;
	std::string domainFromRSS;
	std::string pubDate;
	size_t wordCount;
	std::shared_ptr<pugi::xml_document> xmlDocument;

	bool shouldBeRemoved = false;
	bool filteringDescriptionExists = false;
	std::shared_ptr<std::string> filteringDescription;
};