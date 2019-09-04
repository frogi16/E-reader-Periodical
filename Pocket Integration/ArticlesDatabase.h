#pragma once

#include <vector>

#include "pugixml.hpp"

#include "ArticleRSS.h"

class ArticlesDatabase
{
public:
	ArticlesDatabase();
	void saveDatabase(std::vector<ArticleRSS>& articles);
	std::vector<ArticleRSS> loadDatabase();
	~ArticlesDatabase();
};

