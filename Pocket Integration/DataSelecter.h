#pragma once

#include <vector>
#include <pugixml.hpp>

#include "SelectAttributeTreeWalker.h"
#include "SelectNameTreeWalker.h"
#include "SelectSubstringTreeWalker.h"

namespace EbookPeriodical
{
	template <typename TreeWalker, typename... Args>
	std::vector<pugi::xml_node> selectNodes(pugi::xml_node source, Args&& ... args)
	{
		TreeWalker walker(std::forward<Args>(args)...);
		source.traverse(walker);
		return walker.getResults();
	}
}