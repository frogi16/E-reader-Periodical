#pragma once

#include <string>

#include "XMLFilteringRuleType.h"

class XMLFilteringRule
{
public:
	XMLFilteringRule(XMLFilteringRuleType setType) : type(setType) {}

	XMLFilteringRuleType type;
	std::string nodeName, attributeName, attributeValue, substring;
};