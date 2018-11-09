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
	ImageWidthUnit widthUnit;
	std::map<int, std::string> links;

	SrcSet(std::string source)
	{
		source = std::regex_replace(source, std::regex("%20"), " ");			//replace all occurences of %20 with spaces

		std::istringstream stream(source);
		std::string link;														//link extracted from set
		std::string imageSize;													//size extracted from set

		//first link is treated differently because function needs to determin widthUnit, later it is done with less effort by just removing useless characters
		std::getline(stream, link, ' ');
		std::getline(stream, imageSize, ' ');

		if (imageSize.back() == ',')											//sizes except the last are separated by comma AND space, so it is easier to remove it ASAP
			imageSize.pop_back();

		char unit = imageSize.back();											//determinig unit based on last character
		if (unit == 'w')
			widthUnit = ImageWidthUnit::Width;
		else if (unit == 'x')
			widthUnit = ImageWidthUnit::Density;

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
};