#pragma once

#include <list>
#include <string>

struct Keyword
{
	std::list<std::string> alternatives;
	std::string mainKeyword() const
	{
		return (alternatives.size() ? alternatives.front() : std::string());				//if possible, return first keyword. Otherwise return empty string
	}

	bool operator==(const std::string& rhs) const											//true if any of alternatives matches given string
	{
		for (auto& alternative : alternatives)
		{
			if (rhs == alternative)
				return true;
		}

		return false;
	}

	bool operator!=(const std::string& rhs) const
	{
		return !((*this) == rhs);
	}
};