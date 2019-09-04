#include "FilteringRule.h"

FilteringRule::FilteringRule()
{
	XMLFilteringRules = std::make_shared<std::vector<XMLFilteringRule>>();
}

FilteringRule::FilteringRule(const FilteringRule& rule)
{
	domain = rule.domain;
	minWords = rule.minWords;
	maxWords = rule.maxWords;
	XMLFilteringRules = rule.XMLFilteringRules;
}

FilteringRule::~FilteringRule()
{
}
