#include "DataSelecter.h"

#include <queue>

DataSelecter::DataSelecter() noexcept
{
}

std::vector<pugi::xml_node> DataSelecter::selectNodesByAttribute(pugi::xml_node source, std::string attributeName, std::string attributeValue)
{
	selectAttributeTreeWalker.setSelectArguments(attributeName, attributeValue);
	source.traverse(selectAttributeTreeWalker);

	return selectAttributeTreeWalker.getResults();
}

std::vector<pugi::xml_node> DataSelecter::selectNodesByName(pugi::xml_node source, std::string name)
{
	selectNameTreeWalker.setSelectArguments(name);
	source.traverse(selectNameTreeWalker);

	return selectNameTreeWalker.getResults();
}

DataSelecter::~DataSelecter()
{
}
