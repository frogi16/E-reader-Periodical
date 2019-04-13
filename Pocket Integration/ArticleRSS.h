#pragma once

#include <string>

#include "XMLSerializerInterface.h"

class ArticleRSS :
	public XMLSerializerInterface
{
public:
	void serializeXML(pugi::xml_node & node) const override;
	void deserializeXML(const pugi::xml_node & node) override;

	const bool operator==(const std::string& rhs) const;									//comparing items basing on their links, because they change less often than titles
	const bool operator!=(const std::string& rhs) const;

	std::string title, link, description, pubDate;
};