#include "ArticlesDatabase.h"



ArticlesDatabase::ArticlesDatabase()
{
}

void ArticlesDatabase::saveDatabase(std::vector<ArticleRSS>& articles)
{
	pugi::xml_document doc;
	auto articlesNode = doc.append_child("articles");

	for (auto & article : articles)
	{
		auto articleNode = articlesNode.append_child("article");

		//title, link, description, pubDate

		auto titleNode = articleNode.append_child("title").append_child(pugi::node_pcdata);
		titleNode.set_value(article.title.c_str());

		auto linkNode = articleNode.append_child("link").append_child(pugi::node_pcdata);
		linkNode.set_value(article.link.c_str());

		auto pubDateNode = articleNode.append_child("pubDate").append_child(pugi::node_pcdata);
		pubDateNode.set_value(article.pubDate.c_str());

		auto descriptionNode = articleNode.append_child("description").append_child(pugi::node_pcdata);
		descriptionNode.set_value(article.description.c_str());
	}

	doc.save_file("articles.xml");
}

std::vector<ArticleRSS> ArticlesDatabase::loadDatabase()
{
	std::vector<ArticleRSS> articles;
	pugi::xml_document doc;
	doc.load_file("articles.xml");
	auto node = doc.child("articles");

	for (auto& article : node.children())
	{
		ArticleRSS target;
		target.link = article.child("link").child_value();
		target.title = article.child("title").child_value();
		target.description = article.child("description").child_value();
		target.pubDate = article.child("pubDate").child_value();
		articles.push_back(target);
	}

	return articles;
}


ArticlesDatabase::~ArticlesDatabase()
{
}
