#include <string>
#include <vector>

#include <pugixml.hpp>

struct SelectSubstringTreeWalker : public pugi::xml_tree_walker
{
	SelectSubstringTreeWalker() {}

	SelectSubstringTreeWalker(std::string searchedSubstring)
	{
		setSelectArguments(searchedSubstring);
	}

	void setSelectArguments(std::string searchedSubstring)
	{
		mSearchedSubstring = searchedSubstring;
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
		std::string text = node.value();
		auto pos = text.find(mSearchedSubstring);

		if (pos != text.npos)
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
	std::string mSearchedSubstring;
	std::vector<pugi::xml_node> results;
	bool updated;
};