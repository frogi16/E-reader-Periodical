#pragma once

#include <memory>

#include "pugixml.hpp"

struct ParsedArticle
{
	std::string title;
	std::string author;
	std::string content;
	std::string domain;
	std::string pubDate;
	size_t wordCount;
	std::shared_ptr<pugi::xml_document> xmlDocument;

	bool shouldBeRemoved = false;
};