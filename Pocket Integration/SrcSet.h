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

class SrcSet
{
public:
	SrcSet(std::string source);
	std::string getLargestImageLink() { return links.rbegin()->second; }

	ImageWidthUnit widthUnit;
	std::map<int, std::string> links;
private:
	void replaceHTMLSpaces(std::string & str);				//replace all occurences of %20 with spaces
	ImageWidthUnit detectUnit(char ch);
};