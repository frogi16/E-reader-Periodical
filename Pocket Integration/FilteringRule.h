#pragma once

#include "XMLFilteringRule.h"

#include <memory>
#include <vector>

class FilteringRule
{
public:
	FilteringRule();
	FilteringRule(const FilteringRule& rule);
	~FilteringRule();

	bool exists = false;
	std::string domain;
	size_t minWords;
	size_t maxWords;
	std::shared_ptr<std::vector<XMLFilteringRule>> XMLFilteringRules;
};