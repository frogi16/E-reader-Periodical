#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "pugixml.hpp"

#include "FilteringRule.h"
#include "ParsedArticle.h"
#include "DataSelecter.h"

class ArticleFilter
{
public:
	ArticleFilter();
	void filterArticles(std::vector<ParsedArticle> & articles);					//removes sections of articles and returns false if whole article should be filtered out
	~ArticleFilter();
private:
	void loadFilteringRule(const std::string & domain);
	bool filter(ParsedArticle & article);										//removes sections of articles and returns true if whole article should be filtered out
	FilteringRule getCombinedRule(const std::string & domain);					//returns rule obtained by merging global rule and specific for the domain. Specific one is prioritized
	
	bool applyXMLRule(const XMLFilteringRule &rule, ParsedArticle & article);	//detects type of rule and applies it. Returns true if something changed. Otherwise returns false.
	void applyNodeNameRule(const XMLFilteringRule & rule, ParsedArticle & article);
	void applyTextSubstringRule(const XMLFilteringRule & rule, ParsedArticle & article);
	void applyAttributeValueRule(const XMLFilteringRule & rule, ParsedArticle & article);

	void removeNodes(std::vector<pugi::xml_node>& nodes, ParsedArticle & article);
	std::string documentToString(pugi::xml_document & doc);
	bool isRuleLoaded(const std::string & domain) { return rules[domain].exists; }

	std::map<std::string, FilteringRule> rules;									//domain is the key. Global returns global rule
	DataSelecter dataSelecter;
};

