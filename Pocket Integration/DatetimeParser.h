#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <vector>
#include <map>

class DatetimeParser
{
public:
	DatetimeParser();
	std::tm parse(const std::string & datetime) const;
	~DatetimeParser();
private:
	std::vector<std::string> datetimeFormats =
	{
		"%a, %d %b %Y %H:%M:%S",		//RFC822, eq. Sun, 10 Feb 2019 09:21:29 +0000
		"%Y-%m-%dT%H:%M:%S"				//ISO8601 extended, eq. 2019-02-08T09:54:48Z
	};
};

