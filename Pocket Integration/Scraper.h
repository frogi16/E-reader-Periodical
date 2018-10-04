#pragma once

#include <vector>
#include <pugixml.hpp>

class Scraper
{
public:
	Scraper();
	~Scraper();
	std::vector<pugi::xml_node> selectDataByAttribute(pugi::xml_node source, std::string attributeName = 0, std::string attributeValue = 0);
	std::vector<pugi::xml_node> selectDataByName(pugi::xml_node source, std::string name);
protected:
	void removeUnits(std::string & string);			//removes first space and every character after it
	void tidyString(std::string &input);			//replaces line breaks and &nbsp; with spaces
};

