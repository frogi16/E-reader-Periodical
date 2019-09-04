#pragma once

#include <cwctype>

#include <pugixml_eprd.hpp>

struct CountWordsTreeWalker : public pugi::xml_tree_walker
{
	size_t words = 0;

	virtual bool begin(pugi::xml_node& node)
	{
		words = 0;
		return true;
	}

	virtual bool for_each(pugi::xml_node& node)
	{
		std::string text = node.value();
		std::wstring wideText;
		wideText.assign(text.begin(), text.end());

		size_t add = 0;

		for (auto& wchar : wideText)
			if (std::iswspace(wchar))
				++add;

		if (add)
			words += add + 1;

		return true;
	}
};