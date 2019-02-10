#include "DatetimeParser.h"

DatetimeParser::DatetimeParser()
{
}

std::tm DatetimeParser::parse(const std::string & datetime) const
{
	std::tm t = {};
	std::istringstream ss(datetime);
	ss.imbue(std::locale("en_US.utf-8"));

	for (auto& i : datetimeFormats)				//iterate through vector containing different datetime formats until parsing will be successfull
	{
		ss >> std::get_time(&t, i.c_str());

		if (!ss.fail())
			break;
		
		ss.clear();
	}
	
	//TODO: function will return std::tm completely filled with zeroes, there could be a necessity to implement a better solution

	return t;
}

DatetimeParser::~DatetimeParser()
{
}
