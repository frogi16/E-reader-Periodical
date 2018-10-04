#include "Scraper.h"

#include <queue>

Scraper::Scraper()
{
}

std::vector<pugi::xml_node> Scraper::selectDataByAttribute(pugi::xml_node source, std::string attributeName, std::string attributeValue)
{
	std::vector<pugi::xml_node> results;
	std::queue<pugi::xml_node> childs;
	childs.push(source);

	while (!childs.empty())
	{
		pugi::xml_node currentNode = childs.front();

		for (pugi::xml_node currentChild = currentNode.first_child(); currentChild; currentChild = currentChild.next_sibling())
		{
			childs.push(currentChild);

			if (currentChild.attribute(attributeName.c_str()).as_string() == attributeValue)
			{
				results.push_back(currentChild);
			}
		}

		childs.pop();
	}

	return results;
}

std::vector<pugi::xml_node> Scraper::selectDataByName(pugi::xml_node source, std::string name)
{
	std::vector<pugi::xml_node> results;
	std::queue<pugi::xml_node> childs;
	childs.push(source);

	while (!childs.empty())
	{
		pugi::xml_node currentNode = childs.front();

		for (pugi::xml_node currentChild = currentNode.first_child(); currentChild; currentChild = currentChild.next_sibling())
		{
			childs.push(currentChild);

			if (currentChild.name() == name)
			{
				results.push_back(currentChild);
			}
		}

		childs.pop();
	}

	return results;
}

void Scraper::removeUnits(std::string & string)
{
	//removing everything after first space (space too)
	for (size_t i = 0; i < string.size(); i++)
	{
		if (string[i] == ' ')
		{
			string.erase(i, string.size() - i);
			break;
		}
	}
}

void Scraper::tidyString(std::string & input)
{
	//replacing non-breaking spaces with standard spaces
	while (input.find("&nbsp;") != std::string::npos)								//find() returns string::npos if it doesn't find substring
	{
		auto size = std::string("&nbsp;").size();
		input.replace(input.find("&nbsp;"), size, " ");
	}

	//replacing line brakes with spaces
	std::replace(input.begin(), input.end(), '\n', ' ');
}

Scraper::~Scraper()
{
}
