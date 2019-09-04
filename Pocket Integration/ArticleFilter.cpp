#include "ArticleFilter.h"

#include "CountWordsTreeWalker.h"

#include <sstream>
#include <iostream>
#include <fstream>

ArticleFilter::ArticleFilter()
{
	loadFilteringRule("global");
}

void ArticleFilter::filterArticles(std::vector<ParsedArticle>& articles)
{
	std::fstream filteringResultFile;
	filteringResultFile.open("filteringResult.txt", std::ios::out);

	std::vector<ParsedArticle>::iterator iter;
	for (iter = articles.begin(); iter != articles.end();)		//loop designed to dynamically erase items from container
	{
		std::cout << "*";
		ParsedArticle& article = *iter;

		if (!isRuleLoaded(article.domain))
			loadFilteringRule(article.domain);

		filter(article);										//applies all rules and tests article for all conditions. Sets shouldBeRemoved and filteringDescription flags if necessary

		if (article.filteringDescriptionExists)
			filteringResultFile << (*article.filteringDescription) << std::endl;

		if (article.shouldBeRemoved)
			iter = articles.erase(iter);
		else
			++iter;
	}

	filteringResultFile.close();
	std::cout << std::endl;
}

void ArticleFilter::filter(ParsedArticle& article)
{
	auto combinedRule = getCombinedRule(article.domain);
	bool contentChanged = false;

	article.filteringDescription->append("Article \"" + article.title + "\" from \"" + article.domain + "\":\n");

	for (auto& rule : (*combinedRule.XMLFilteringRules))
	{
		if (applyXMLRule(rule, article))
		{
			contentChanged = true;
			article.filteringDescriptionExists = true;
		}
	}

	if (tooFewWords(combinedRule, article))
	{
		article.shouldBeRemoved = true;
		article.filteringDescriptionExists = true;
		addDescriptionTooFewWords(combinedRule, article);
	}

	if (tooManyWords(combinedRule, article))
	{
		article.shouldBeRemoved = true;
		article.filteringDescriptionExists = true;
		addDescriptionTooManyWords(combinedRule, article);
	}

	if (!article.shouldBeRemoved && contentChanged)				//actualize content only if necessary! Article marked to removal won't be updated
		article.content = documentToString((*article.xmlDocument));
}

FilteringRule ArticleFilter::getCombinedRule(const std::string& domain)
{
	FilteringRule& globalRule = rules["global"];				//we can assume that global rule is always there
	FilteringRule combinedRule;

	auto findSpecificRule = rules.find(domain);
	if (findSpecificRule != rules.end())						//specific rule does exist and will be prefered over global one
	{
		combinedRule = findSpecificRule->second;

		if (!combinedRule.minWords)								//zero assigned to variable allows deriving from global rule
			combinedRule.minWords = globalRule.minWords;

		if (!combinedRule.maxWords)
			combinedRule.maxWords = globalRule.maxWords;
	}
	else
		combinedRule = globalRule;

	return combinedRule;
}

bool ArticleFilter::applyXMLRule(const XMLFilteringRule& rule, ParsedArticle& article)
{
	if (rule.type == XMLFilteringRuleType::AttributeValue)			//if else was used instead of switch because of variable initialization inside (possible error:"transfer of control bypasses initialization of")
	{
		return applyAttributeValueRule(rule, article);
	}
	else if (rule.type == XMLFilteringRuleType::NodeName)
	{
		return applyNodeNameRule(rule, article);
	}
	else if (rule.type == XMLFilteringRuleType::TextSubstring)
	{
		return applyTextSubstringRule(rule, article);
	}

	return false;
}

bool ArticleFilter::applyNodeNameRule(const XMLFilteringRule& rule, ParsedArticle& article)
{
	auto dataToFilter = dataSelecter.selectNodesByName((*article.xmlDocument), rule.nodeName);
	size_t removedWords = removeNodes(dataToFilter, article);

	if (removedWords > 0)
	{
		article.filteringDescription->append("Applying filter based on node name: " + rule.nodeName + "\" removed " + std::to_string(removedWords) + " words\n");
	}

	return removedWords;
}

bool ArticleFilter::applyTextSubstringRule(const XMLFilteringRule& rule, ParsedArticle& article)
{
	auto dataToFilter = dataSelecter.selectNodesByTextSubstring((*article.xmlDocument), rule.substring);
	size_t removedWords = removeNodes(dataToFilter, article);

	if (removedWords > 0)
	{
		article.filteringDescription->append("Applying filter based on searching for text: \"" + rule.substring + "\" removed " + std::to_string(removedWords) + " words\n");
	}

	return removedWords;
}

bool ArticleFilter::applyAttributeValueRule(const XMLFilteringRule& rule, ParsedArticle& article)
{
	auto dataToFilter = dataSelecter.selectNodesByAttribute((*article.xmlDocument), rule.attributeName, rule.attributeValue);
	size_t removedWords = removeNodes(dataToFilter, article);

	if (removedWords > 0)
	{
		article.filteringDescription->append("Applying filter based on attribute and its value: " + rule.attributeName + "\"" + rule.attributeValue + "\" removed " + std::to_string(removedWords) + " words\n");
	}

	return removedWords;
}

size_t ArticleFilter::removeNodes(std::vector<pugi::xml_node>& nodes, ParsedArticle& article)
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
	return removedWords;
}

std::string ArticleFilter::documentToString(pugi::xml_document& doc)
{
	std::stringstream ss;
	doc.save(ss, "  ");
	return ss.str();
}

bool ArticleFilter::tooFewWords(const FilteringRule& rule, const ParsedArticle& article)
{
	return (rule.minWords && article.wordCount < rule.minWords);
}

bool ArticleFilter::tooManyWords(const FilteringRule& rule, const ParsedArticle& article)
{
	return (rule.maxWords && article.wordCount > rule.maxWords);
}

void ArticleFilter::addDescriptionTooFewWords(const FilteringRule& rule, ParsedArticle& article)
{
	article.filteringDescription->append("Article was removed because had only " + std::to_string(article.wordCount) + " words instead of at least " + std::to_string(rule.minWords) + ".\n");
}

void ArticleFilter::addDescriptionTooManyWords(const FilteringRule& rule, ParsedArticle& article)
{
	article.filteringDescription->append("Article was removed because had only " + std::to_string(article.wordCount) + " words instead of maximum " + std::to_string(rule.maxWords) + " .\n");
}

void ArticleFilter::loadFilteringRule(const std::string& domain)
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