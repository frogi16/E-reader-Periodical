#pragma once

#include <regex>
#include <sstream>
#include <locale> 
#include <map>

enum class ImageWidthUnit
{
	Width,
	Density
};

struct SrcSet
{
	SrcSet(std::string source);
	std::string getLargestImageLink() { return links.rbegin()->second; }

	ImageWidthUnit widthUnit;
	std::map<int, std::string> links;
};