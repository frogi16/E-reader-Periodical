#pragma once

#include <map>
#include <string>

enum class KeywordType
{
	pubDate,
	item,
	link,
	description,
	title
};

struct DatabaseKeywords
{
	std::map<std::string, KeywordType> mapStringToKeywordType =
	{
		{ "pubDate", KeywordType::pubDate },
		{ "item", KeywordType::item },
		{ "link", KeywordType::link },
		{ "description", KeywordType::description },
		{ "title", KeywordType::title }
	};

	std::map<KeywordType, std::string> mapKeywordTypeToString
	{
		{ KeywordType::pubDate, "pubDate" },
		{ KeywordType::item, "item" },
		{ KeywordType::link, "link" },
		{ KeywordType::description, "description" },
		{ KeywordType::title, "title" }
	};
};

