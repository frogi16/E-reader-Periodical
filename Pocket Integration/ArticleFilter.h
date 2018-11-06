#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "pugixml.hpp"

#include "ParsedArticle.h"
#include "DataSelecter.h"

enum class XMLFilteringRuleType
{
	NodeName,
	AttributeValue
};

class XMLFilteringRule
{
public:
	XMLFilteringRule(XMLFilteringRuleType setType) : type(setType) {}

	XMLFilteringRuleType type;
	std::string nodeName, attributeName, attributeValue;
};

struct Rule
{
	Rule()
	{
		XMLFilteringRules = std::make_shared<std::vector<XMLFilteringRule>>();
	}

	Rule(const Rule& rule)
	{
		exists = true;
		domain = rule.domain;
		minWords = rule.minWords;
		maxWords = rule.maxWords;
		XMLFilteringRules = rule.XMLFilteringRules;
	}

	bool exists = false;
	std::string domain;
	size_t minWords;
	size_t maxWords;
	std::shared_ptr<std::vector<XMLFilteringRule>> XMLFilteringRules;
};

class ArticleFilter
{
public:
	ArticleFilter();
	void filterArticles(std::vector<ParsedArticle> & articles);			//removes sections of articles and returns false if whole article should be filtered out
	~ArticleFilter();
private:
	void loadFilterRule(std::string domain);
	bool filter(ParsedArticle & article);								//removes sections of articles and returns true if whole article should be filtered out
	Rule getCombinedRule(std::string domain);							//returns rule obtained by merging global rule and specific for the domain. Specific one is prioritized
	void applyAttributeValueRule(const XMLFilteringRule & rule, ParsedArticle & article);
	void applyNodeNameRule(const XMLFilteringRule & rule, ParsedArticle & article);
	void removeNodes(std::vector<pugi::xml_node>& nodes, ParsedArticle & article);
	std::string documentToString(pugi::xml_document & doc);

	std::map<std::string, Rule> rules;									//domain is the key. Global returns global rule
	DataSelecter dataSelecter;
};

