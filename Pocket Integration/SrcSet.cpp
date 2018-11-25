#include "SrcSet.h"

SrcSet::SrcSet(std::string source)
{
	replaceHTMLSpaces(source);

	std::istringstream stream(source);
	std::string link;														//link extracted from set
	std::string imageSize;													//size extracted from set

																			//first link is treated differently because function needs to determin widthUnit, later it is done with less effort by just removing useless characters
	std::getline(stream, link, ' ');
	std::getline(stream, imageSize, ' ');

	if (imageSize.back() == ',')											//sizes except the last are separated by comma AND space, so it is easier to remove it ASAP
		imageSize.pop_back();

	try
	{
		widthUnit = detectUnit(imageSize.back());							//determinig unit based on last character
	}
	catch (const std::exception& e)
	{
		widthUnit = ImageWidthUnit::Width;
	}

	imageSize.pop_back();													//after determinig unit last character can be removed. There are only digits left right now

	links[std::stoi(imageSize)] = link;										//adding to the map link to image, where key is its width

	while (std::getline(stream, link, ' '))									//link to image and its width are paired, so lack of link indicates lack of width. Hence only one check
	{
		std::getline(stream, imageSize, ' ');

		imageSize.erase(std::remove_if(imageSize.begin(), imageSize.end(),	//remove all characters except digits (unit and comma if it exists)
			[](char c) { return !(c >= 48 && c <= 57); }),					//basically !isdigit(c), but I had problem with getting it work
			imageSize.end());

		links[std::stoi(imageSize)] = link;
	}
}

void SrcSet::replaceHTMLSpaces(std::string & str)
{
	std::regex_replace(str, std::regex("%20"), " ");
}

ImageWidthUnit SrcSet::detectUnit(char ch)
{
	if (ch == 'w')
		return ImageWidthUnit::Width;
	else if (ch == 'x')
		return ImageWidthUnit::Density;
	else
		throw("Unit couldn't be recognised.");
}
