#pragma once

#include <regex>
#include <sstream>
#include <locale> 
#include <map>
#include <optional>

enum class ImageWidthUnit
{
	Width,
	Density
};

using optString = std::optional<std::string>;

class SrcSet
{
public:
	SrcSet();
	size_t parseAndInsertLinks(std::string srcSetText);				//parse string, detect unit and return number of inserted links
	optString getLargestImageLink() const
	{
		return links.size() > 0 ? optString{ links.rbegin()->second } : std::nullopt;
	}
	ImageWidthUnit getWidthUnit() const { return widthUnit; }
	std::map<int, std::string> links;
private:
	void removeCommaAfterUnit(std::string& imageSize) const;		//if string is at least two characters long (first is for unit) and the last one is a comma, remove it
	void replaceHTMLSpaces(std::string& str) const;					//replace all occurences of %20 with spaces
	std::optional<ImageWidthUnit> matchWidthUnit(char unit) const;

	std::map<char, ImageWidthUnit> charToImageWidthUnit;
	ImageWidthUnit widthUnit;
};