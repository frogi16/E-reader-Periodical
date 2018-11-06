#include <string>
#include <vector>

#include <pugixml.hpp>

struct SelectAttributeTreeWalker : public pugi::xml_tree_walker
{
	SelectAttributeTreeWalker() {}

	SelectAttributeTreeWalker(std::string searchedAttributeName, std::string searchedAttributeValue)
	{
		setSelectArguments(searchedAttributeName, searchedAttributeValue);
	}

	void setSelectArguments(std::string searchedAttributeName, std::string searchedAttributeValue)
	{
		mSearchedAttributeName = searchedAttributeName;
		mSearchedAttributeValue = searchedAttributeValue;
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
		std::string attributeValue = node.attribute(mSearchedAttributeName.c_str()).as_string();

		if (attributeValue == mSearchedAttributeValue)
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
	std::string mSearchedAttributeName;
	std::string mSearchedAttributeValue;
	std::vector<pugi::xml_node> results;
	bool updated;
};