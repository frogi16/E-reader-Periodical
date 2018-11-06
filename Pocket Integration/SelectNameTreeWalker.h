#pragma once

#include <string>
#include <vector>

#include <pugixml.hpp>

struct SelectNameTreeWalker : public pugi::xml_tree_walker
{
	SelectNameTreeWalker() {}

	SelectNameTreeWalker(std::string searchedNodeName)
	{
		setSelectArguments(searchedNodeName);
	}

	void setSelectArguments(std::string searchedNodeName)
	{
		mSearchedNodeName = searchedNodeName;
		updated = true;
	}

	virtual bool begin(pugi::xml_node& node)
	{
		if (updated)
		{
			results.clear();
			updated = false;
			return true;
		}
		else
		{
			return false;
		}
	}

	virtual bool for_each(pugi::xml_node& node)
	{
		std::string nodeName = node.name();

		if (nodeName == mSearchedNodeName)
		{
			results.push_back(node);
		}

		return true;
	}

	std::vector<pugi::xml_node> getResults()
	{
		return results;
	}
protected:
	bool updated;
	std::string mSearchedNodeName;
	std::vector<pugi::xml_node> results;
};