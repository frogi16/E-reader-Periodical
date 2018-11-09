#pragma once

#include "SelectAttributeTreeWalker.h"
#include "SelectNameTreeWalker.h"
#include "SelectSubstringTreeWalker.h"

class DataSelecter
{
public:
	DataSelecter() noexcept;
	~DataSelecter();
	std::vector<pugi::xml_node> selectNodesByAttribute(pugi::xml_node source, std::string attributeName, std::string attributeValue);
	std::vector<pugi::xml_node> selectNodesByName(pugi::xml_node source, std::string name);
	std::vector<pugi::xml_node> selectNodesByTextSubstring(pugi::xml_node source, std::string substring);
protected:
	SelectAttributeTreeWalker selectAttributeTreeWalker;
	SelectNameTreeWalker selectNameTreeWalker;
	SelectSubstringTreeWalker selectSubstringTreeWalker;
};

