#pragma once

#include <string>
#include <regex>

namespace eprd
{
	std::string stringToLower(const std::string &string);
	std::string getDomain(const std::string &link);
	std::string mergePathFragments(const std::string &left, const std::string &right);
}