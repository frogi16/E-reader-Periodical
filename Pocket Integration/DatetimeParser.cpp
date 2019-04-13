#include "DatetimeParser.h"

#include <sstream>
#include <iostream>

std::vector<std::string> DatetimeParser::datetimeFormats =
{
	"%a, %d %b %Y %H:%M:%S",					//RFC822, eq. Sun, 10 Feb 2019 09:21:29 +0000
	"%Y-%m-%dT%H:%M:%S"							//ISO8601 extended, eq. 2019-02-08T09:54:48Z
};

DatetimeParser::DatetimeParser()
{
}

std::tm DatetimeParser::parseToTM(const std::string & datetime) const
{
	std::tm time;
	std::istringstream ss(datetime);
	ss.imbue(std::locale("en_US.utf-8"));

	for (auto& format : datetimeFormats)
	{
		ss >> std::get_time(&time, format.c_str());	//try to parse datetime using current format

		if (!ss.fail())
			//TODO: timezone were voted to be included in C++20! Major simplification can be expected, so update this code when possible.
		{
			modifyTimeForTimezone(ss, time);
			return time;;
		}

		ss.clear();
	}

	//if all attempts to parse fail function will return std::tm completely filled with zeroes
	return time;
}

time_t DatetimeParser::parseToTime_t(const std::string & datetime) const
{
	return mktime(&parseToTM(datetime));
}

DatetimeParser::~DatetimeParser()
{
}

void DatetimeParser::modifyTimeForTimezone(std::istringstream &ss, std::tm &time) const noexcept
{
	std::string timezone;
	ss >> timezone;

	char sign = timezone.front();				//first sign determines whether we need to add or subtract
	timezone.erase(0, 1);						//pop_front()

	if (sign == '-' || sign == '+')				//incorrect or lack of sign gives away that string is not a supported timezone and no operation will be performed
	{
		try
		{
			int hours = extractTwoDigitsAsInt(timezone);
			int minutes = extractTwoDigitsAsInt(timezone);

			time.tm_hour += (sign == '+' ? hours : -hours);
			time.tm_min += (sign == '+' ? minutes : -minutes);
		}
		catch (const std::exception&) {}		//conversions to int embedded in extractTwoDigitsAsInt() can throw errors, but I decided that this function should rather return zero time, so errors are dismissed here
	}

	std::mktime(&time);							//normalization, handling overflows etc.
}

int DatetimeParser::extractTwoDigitsAsInt(std::string & timezone) const
{
	std::string tempString(timezone.begin(), timezone.begin() + 2);
	int out = std::stoi(tempString);			//potential exception thrown inside stoi will make function break here and no characters will be erased from timezone, which is good
	timezone.erase(0, 2);
	return out;
}
