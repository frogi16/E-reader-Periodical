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
	void filterArticles(std::vector<ParsedArticle>& articles);					//removes fragments and whole articles if they match previously set rules
	~ArticleFilter();
private:
	void loadFilteringRule(const std::string& domain);							//tries to load rule reffering to given domain
	void filter(ParsedArticle& article);										//applies rules and checks conditions reffering to given article. Sets shouldBeRemoved flag if necessary
	FilteringRule getCombinedRule(const std::string& domain);					//returns rule obtained by merging global rule and specific for the domain. Specific one is prioritized as more precise (cascade known for example from CSS)

	bool applyXMLRule(const XMLFilteringRule& rule, ParsedArticle& article);	//detects type of rule and applies it. Returns true if something changed. Otherwise returns false.
	bool applyNodeNameRule(const XMLFilteringRule& rule, ParsedArticle& article);
	bool applyTextSubstringRule(const XMLFilteringRule& rule, ParsedArticle& article);
	bool applyAttributeValueRule(const XMLFilteringRule& rule, ParsedArticle& article);

	size_t removeNodes(std::vector<pugi::xml_node>& nodes, ParsedArticle& article);	//returns number of removed words
	std::string documentToString(pugi::xml_document& doc);

	bool isRuleLoaded(const std::string& domain) { return rules.count(domain) > 0; }
	bool tooFewWords(const FilteringRule& rule, const ParsedArticle& article);
	bool tooManyWords(const FilteringRule& rule, const ParsedArticle& article);

	void addDescriptionTooFewWords(const FilteringRule& rule, ParsedArticle& article);
	void addDescriptionTooManyWords(const FilteringRule& rule, ParsedArticle& article);

	std::map<std::string, FilteringRule> rules;									//stores FilteringRules with domains as keys. Key to global rule is of course "global"
};

