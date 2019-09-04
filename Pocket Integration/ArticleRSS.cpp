#include "ArticleRSS.h"


//					XML template
//
//	<item>
//		<title>			PCDATA		</title>
//		<link>			PCDATA		</link>
//		<pubDate>		PCDATA		</pubDate>
//		<description>	PCDATA		</description>
//	</item>

void ArticleRSS::serializeXML(pugi::xml_node& node) const
{
	node.append_child("title").text().set(title.c_str());
	node.append_child("link").text().set(link.c_str());
	node.append_child("pubDate").text().set(pubDate.c_str());
	node.append_child("description").text().set(description.c_str());
}

void ArticleRSS::deserializeXML(const pugi::xml_node& node)
{
	link = node.child("link").child_value();
	title = node.child("title").child_value();
	description = node.child("description").child_value();
	pubDate = node.child("pubDate").child_value();
}

const bool ArticleRSS::operator==(const std::string& rhs) const										//comparing items basing on their links, because they change less often than titles
{
	return this->link == rhs;
}

const bool ArticleRSS::operator!=(const std::string& rhs) const
{
	return !((*this) == rhs);
}