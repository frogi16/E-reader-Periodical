#pragma once

#include <vector>

#include "XMLSerializerInterface.h"
#include "ArticleRSS.h"

class FeedData :
	public XMLSerializerInterface
{
public:
	void serializeXML(pugi::xml_node& node) const override;
	void deserializeXML(const pugi::xml_node& node) override;

	std::string link, lastBuildTime;
	std::vector<ArticleRSS> items;
};

