#include "FeedData.h"


//				XML template
//
//	<feed>
//		<link>			PCDATA			</link>
//		<lastUpdate>	PCDATA			</lastUpdate>
//		<item>			ArticleRSS		</item>
//		...
//		<item>			ArticleRSS		</item>
//	</feed>

void FeedData::serializeXML(pugi::xml_node & node) const
{
	node.append_child("link").append_child(pugi::node_pcdata).set_value(link.c_str());
	node.append_child("lastUpdate").append_child(pugi::node_pcdata).set_value(lastBuildTime.c_str());

	for (auto& article : items)
	{
		auto itemNode = node.append_child("item");
		article.serializeXML(itemNode);
	}
}

void FeedData::deserializeXML(const pugi::xml_node & node)
{
	link = node.child("link").child_value();
	lastBuildTime = node.child("lastUpdate").child_value();

	for (auto iterator = std::next(node.children().begin(), 2); iterator != node.children().end(); iterator++)
	{
		try
		{
			pugi::xml_node articleXML = (*iterator);
			ArticleRSS article;
			article.deserializeXML(articleXML);
			items.push_back(article);
		}
		catch (const std::exception& e)
		{
			throw std::exception(std::string("Loading news from database failed while loading " + link + ". Data is probably corrupted.").c_str());
		}
	}
}
