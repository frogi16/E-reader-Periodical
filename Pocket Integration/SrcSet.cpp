#include "SrcSet.h"

#include "StringValidation.h"

SrcSet::SrcSet() : widthUnit(ImageWidthUnit::Width)
{
	charToImageWidthUnit['w'] = ImageWidthUnit::Width;
	charToImageWidthUnit['d'] = ImageWidthUnit::Density;
}

size_t SrcSet::parseAndInsertLinks(std::string srcSetText)
{
	size_t insertsCounter{ 0 };
	replaceHTMLSpaces(srcSetText);

	std::string link, imageSize;

	for(std::istringstream stream(srcSetText); std::getline(stream, link, ' ') && std::getline(stream, imageSize, ' '); )
	{
		if (insertsCounter == 0)
		{
			removeCommaAfterUnit(imageSize);
			widthUnit = matchWidthUnit(imageSize.back()).value_or(ImageWidthUnit::Width);		//if match failed fall back to default unit
		}

		imageSize.erase(std::remove_if(imageSize.begin(), imageSize.end(),						//remove everything but digits
			[](char c) { return !std::isdigit(c);}), imageSize.end());

		if (EbookPeriodical::isPositiveInteger(imageSize))
		{
			links[std::stoi(imageSize)] = link;
			insertsCounter++;
		}
	}

	return insertsCounter;
}

void SrcSet::removeCommaAfterUnit(std::string& imageSize) const
{
	if (imageSize.size() >= 2 && imageSize.back() == ',')
		imageSize.pop_back();
}

void SrcSet::replaceHTMLSpaces(std::string& str) const
{
	str = std::regex_replace(str, std::regex("%20"), " ");
}

std::optional<ImageWidthUnit> SrcSet::matchWidthUnit(char unit) const
{
	if (const std::map<char, ImageWidthUnit>::const_iterator pos = charToImageWidthUnit.find(unit); pos == charToImageWidthUnit.end())
		return std::nullopt;
	else
		return pos->second;
}
