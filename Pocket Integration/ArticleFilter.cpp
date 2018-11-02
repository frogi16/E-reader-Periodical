#include "ArticleFilter.h"

#include <sstream>
#include <iostream>

ArticleFilter::ArticleFilter()
{
	loadFilterRule("global");
}

void ArticleFilter::filterArticles(std::vector<ParsedArticle>& articles)
{
	std::vector<ParsedArticle>::iterator iter;
	for (iter = articles.begin(); iter != articles.end();)
	{
		std::cout << "*";

		if (!rules[iter->domain].exists)
		{
			loadFilterRule(iter->domain);
		}

		if (filter(*iter))													//filter function returns true if article needs to be completely erased
			iter = articles.erase(iter);
		else
			++iter;
	}

	std::cout << std::endl;
}

bool ArticleFilter::filter(ParsedArticle & article)
{
	auto combinedRule = getCombinedRule(article.domain);

	if (article.wordCount)													//wordCount equals zero almost always means incorrect data returned by parser and articles shouldn't be filtered using corrupted data
	{
		if (combinedRule.minWords && article.wordCount < combinedRule.minWords)
			return true;

		if (combinedRule.maxWords && article.wordCount > combinedRule.maxWords)
			return true;
	}

	for (auto& XMLRule : (*combinedRule.XMLFilteringRules))
	{
		if (XMLRule.type == XMLFilteringRuleType::AttributeValue)			//if else was used instead of switch because of variable initialization inside (possible error:"transfer of control bypasses initialization of")
		{
			applyAttributeValueRule(XMLRule, article);
		}
		else if (XMLRule.type == XMLFilteringRuleType::NodeName)			//if else was used instead of switch because of variable initialization inside (possible error:"transfer of control bypasses initialization of")
		{
			applyNodeNameRule(XMLRule, article);
		}
	}

	return false;
}

Rule ArticleFilter::getCombinedRule(std::string domain)
{
	Rule & globalRule = rules["global"];
	Rule & specificRule = rules[domain];
	Rule combinedRule;

	if (specificRule.exists)								//if specific rule doesn't exist, variables would be initialized with random data
	{
		combinedRule = specificRule;

		if (!combinedRule.minWords)							//zero assigned to variable allows deriving rule from global
			combinedRule.minWords = globalRule.minWords;

		if (!combinedRule.maxWords)
			combinedRule.maxWords = globalRule.maxWords;
	}
	else if(globalRule.exists)
	{
		combinedRule = globalRule;
	}

	return combinedRule;
}

void ArticleFilter::applyAttributeValueRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	pugi::xml_document doc;
	doc.load_string(article.content.c_str());
	auto dataToFilter = scraper.selectDataByAttribute(doc, rule.attributeName, rule.attributeValue);
	removeNodes(dataToFilter);
	article.content = documentToString(doc);
}

void ArticleFilter::applyNodeNameRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	pugi::xml_document doc;
	doc.load_string(article.content.c_str());
	auto dataToFilter = scraper.selectDataByName(doc, rule.nodeName);
	removeNodes(dataToFilter);
	article.content = documentToString(doc);
}

void ArticleFilter::removeNodes(std::vector<pugi::xml_node>& nodes)
{
	for (auto& data : nodes)
	{
		data.parent().remove_child(data);
	}
}

std::string ArticleFilter::documentToString(pugi::xml_document & doc)
{
	std::stringstream ss;
	doc.save(ss, "  ");
	return ss.str();
}

void ArticleFilter::loadFilterRule(std::string domain)
{
	pugi::xml_document doc;
	doc.load_file("filters.xml");
	auto node = doc.child("rules");

	for (auto& loadedRule : node.children())
	{
		if (domain == loadedRule.child("domain").child_value())
		{
			Rule rule;
			rule.domain = domain;
			rule.exists = true;
			rule.minWords = loadedRule.child("minWords").text().as_int();
			rule.maxWords = loadedRule.child("maxWords").text().as_int();

			auto XML = loadedRule.child("XMLFilteringRules");

			for (auto& loadedXMLRule : XML.children())
			{
				std::string type = loadedXMLRule.child("type").child_value();

				if (type == "attribute-value")
				{
					XMLFilteringRule XMLRule(XMLFilteringRuleType::AttributeValue);
					XMLRule.attributeName = loadedXMLRule.child("attribute").child_value();
					XMLRule.attributeValue = loadedXMLRule.child("value").child_value();
					rule.XMLFilteringRules->emplace_back(XMLRule);
				}
				else if (type == "node-name")
				{
					XMLFilteringRule XMLRule(XMLFilteringRuleType::NodeName);
					XMLRule.nodeName = loadedXMLRule.child("nodeName").child_value();
					rule.XMLFilteringRules->emplace_back(XMLRule);
				}
				
			}

			rules[domain] = rule;
			//rules.insert(std::make_pair(domain, std::move(rule)));
		}
	}
}

ArticleFilter::~ArticleFilter()
{
}