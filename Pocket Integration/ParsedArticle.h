#pragma once

struct ParsedArticle
{
	std::string title;
	std::string author;
	std::string content;
	std::string domain;
	std::string pubDate;
	size_t wordCount;
};