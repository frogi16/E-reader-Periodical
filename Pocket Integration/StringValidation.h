#pragma once

#include <string>
#include <regex>

namespace EbookPeriodical
{
	inline bool isPositiveInteger(const std::string& str) { return std::regex_match(str, std::regex("[0-9]+")); }
}