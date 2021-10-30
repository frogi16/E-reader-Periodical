#pragma once

#include "StringUtils.h"

namespace eprd
{
	std::string stringToLower(const std::string &string)
	{
		std::string lowerString{ string };

		for (auto &c : lowerString)
			c = (char)tolower(c);

		return lowerString;
	}

	std::string getDomain(const std::string &link)
	{
		static const std::regex rgx("https?\://[^/]*");
		std::smatch match;

		return std::regex_search(link.begin(), link.end(), match, rgx) ? match[0] : std::string{};
	}

	std::string mergePathFragments(const std::string &left, const std::string &right)
	{
		const bool isSlashLeft{ left.back() == '/' };
		const bool isSlashRight{ right.front() == '/' };

		if ((isSlashLeft && !isSlashRight) || (!isSlashLeft && isSlashRight))
		{
			return left + right;
		}
		if (!isSlashLeft && !isSlashRight)
		{
			return left + '/' + right;
		}
		else
		{
			return left.substr(0, left.size() - 1) + right;
		}

	}
}