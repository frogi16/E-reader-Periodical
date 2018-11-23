#include "ArticleFilter.h"

#include "CountWordsTreeWalker.h"

#include <sstream>
#include <iostream>

ArticleFilter::ArticleFilter()
{
	loadFilteringRule("global");
}

void ArticleFilter::filterArticles(std::vector<ParsedArticle>& articles)
{
	std::vector<ParsedArticle>::iterator iter;
	for (iter = articles.begin(); iter != articles.end();)		//loop designed to dynamically erase items from container
	{
		std::cout << "*";
		ParsedArticle& article = *iter;

		if (!isRuleLoaded(article.domain))
		{
			loadFilteringRule(article.domain);
		}

		filter(article);										//applies all rules and checks article for all conditions. Sets shouldBeRemoved flag if necessary

		if (article.shouldBeRemoved)
			iter = articles.erase(iter);
		else
			++iter;
	}

	std::cout << std::endl;
}

void ArticleFilter::filter(ParsedArticle & article)
{
	auto combinedRule = getCombinedRule(article.domain);
	bool contentChanged = false;

	for (auto& rule : (*combinedRule.XMLFilteringRules))
	{
		if (applyXMLRule(rule, article))
			contentChanged = true;
	}

	if (tooFewWords(combinedRule, article))
		article.shouldBeRemoved = true;

	if (tooManyWords(combinedRule, article))
		article.shouldBeRemoved = true;

	if (!article.shouldBeRemoved && contentChanged)			//actualize content only if necessary! Article marked to removal won't be updated
		article.content = documentToString((*article.xmlDocument));
}

FilteringRule ArticleFilter::getCombinedRule(const std::string & domain)
{
	FilteringRule & globalRule = rules["global"];
	FilteringRule & specificRule = rules[domain];
	FilteringRule combinedRule;

	if (specificRule.exists)								//if specific rule doesn't exist, variables would be initialized with random data
	{
		combinedRule = specificRule;

		if (!combinedRule.minWords)							//zero assigned to variable allows deriving from global rule
			combinedRule.minWords = globalRule.minWords;

		if (!combinedRule.maxWords)
			combinedRule.maxWords = globalRule.maxWords;
	}
	else if (globalRule.exists)
	{
		combinedRule = globalRule;
	}

	return combinedRule;
}

bool ArticleFilter::applyXMLRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	if (rule.type == XMLFilteringRuleType::AttributeValue)			//if else was used instead of switch because of variable initialization inside (possible error:"transfer of control bypasses initialization of")
	{
		applyAttributeValueRule(rule, article);
		return true;
	}
	else if (rule.type == XMLFilteringRuleType::NodeName)
	{
		applyNodeNameRule(rule, article);
		return true;
	}
	else if (rule.type == XMLFilteringRuleType::TextSubstring)
	{
		applyTextSubstringRule(rule, article);
		return true;
	}

	return false;
}

void ArticleFilter::applyNodeNameRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	auto dataToFilter = dataSelecter.selectNodesByName((*article.xmlDocument), rule.nodeName);
	removeNodes(dataToFilter, article);
}

void ArticleFilter::applyTextSubstringRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	auto dataToFilter = dataSelecter.selectNodesByTextSubstring((*article.xmlDocument), rule.substring);
	removeNodes(dataToFilter, article);
}

void ArticleFilter::applyAttributeValueRule(const XMLFilteringRule & rule, ParsedArticle & article)
{
	auto dataToFilter = dataSelecter.selectNodesByAttribute((*article.xmlDocument), rule.attributeName, rule.attributeValue);
	removeNodes(dataToFilter, article);
}

void ArticleFilter::removeNodes(std::vector<pugi::xml_node>& nodes, ParsedArticle & article)
{
	size_t removedWords = 0;
	CountWordsTreeWalker walker;

	for (auto& data : nodes)
	{
		data.traverse(walker);
		removedWords += walker.words;
		data.parent().remove_child(data);
	}

	article.wordCount -= removedWords;
}

std::string ArticleFilter::documentToString(pugi::xml_document & doc)
{
	std::stringstream ss;
	doc.save(ss, "  ");
	return ss.str();
}

bool ArticleFilter::tooFewWords(const FilteringRule & rule, const ParsedArticle & article)
{
	return (rule.minWords && article.wordCount < rule.minWords);
}

bool ArticleFilter::tooManyWords(const FilteringRule & rule, const ParsedArticle & article)
{
	return (rule.maxWords && article.wordCount > rule.maxWords);
}

void ArticleFilter::loadFilteringRule(const std::string & domain)
{
	pugi::xml_document doc;
	doc.load_file("filters.xml");
	auto node = doc.child("rules");

	for (auto& loadedRule : node.children())
	{
		if (domain == loadedRule.child("domain").child_value())
		{
			FilteringRule rule;
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
				else if (type == "text-substring")
				{
					XMLFilteringRule XMLRule(XMLFilteringRuleType::TextSubstring);
					XMLRule.substring = loadedXMLRule.child("substring").child_value();
					rule.XMLFilteringRules->emplace_back(XMLRule);
				}

			}

			rules[domain] = rule;
		}
	}
}

ArticleFilter::~ArticleFilter()
{
}