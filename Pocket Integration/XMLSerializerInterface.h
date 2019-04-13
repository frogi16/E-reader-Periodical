#pragma once

#include "pugixml.hpp"

class XMLSerializerInterface
{
public:
	virtual void serializeXML(pugi::xml_node & node) const = 0;
	virtual void deserializeXML(const pugi::xml_node & node) = 0;
};