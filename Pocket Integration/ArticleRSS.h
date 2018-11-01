#pragma once

#include <string>

class ArticleRSS
{
public:
	std::string title, link, description, pubDate;

	const bool operator==(const std::string& rhs) const														//comparing items based on their links (titles can be changed, links almost never)
	{
		return this->link == rhs;
	}

	const bool operator!=(const std::string& rhs) const
	{
		return !((*this) == rhs);
	}
};