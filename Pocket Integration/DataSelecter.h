#pragma once

#include "SelectAttributeTreeWalker.h"
#include "SelectNameTreeWalker.h"

class DataSelecter
{
public:
	DataSelecter() noexcept;
	~DataSelecter();
	std::vector<pugi::xml_node> selectNodesByAttribute(pugi::xml_node source, std::string attributeName = 0, std::string attributeValue = 0);
	std::vector<pugi::xml_node> selectNodesByName(pugi::xml_node source, std::string name);
protected:
	SelectAttributeTreeWalker selectAttributeTreeWalker;
	SelectNameTreeWalker selectNameTreeWalker;
};

