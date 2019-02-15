#include "DatetimeParser.h"

DatetimeParser::DatetimeParser()
{
}

std::tm DatetimeParser::parse(const std::string & datetime) const
{
	std::tm time = {};
	std::istringstream ss(datetime);
	ss.imbue(std::locale("en_US.utf-8"));

	for (auto& i : datetimeFormats)				//iterate through vector containing different datetime formats until parsing will be successfull
	{
		ss >> std::get_time(&time, i.c_str());

		if (!ss.fail())
												//TODO: timezone were voted to be included in C++20! Update this code when possible.
		{
			std::string timezone;
			ss >> timezone;

			char sign = timezone.front();		//(nearly) all timezones look like this: +0100 or -0200, so first sign determines if whether we need to add or subtract
			timezone.erase(0, 1);

			if (sign == '-' || sign == '+')
			{
				try
				{
					std::string hours;
					hours.append(timezone.begin(), timezone.begin() + 2);
					timezone.erase(0, 2);
					int h = std::stoi(hours);

					std::string minutes;
					minutes.append(timezone.begin(), timezone.begin() + 2);
					timezone.erase(0, 2);
					int m = std::stoi(minutes);

					if (sign == '-')
					{
						m *= (-1);
						h *= (-1);
					}

					time.tm_hour += h;
					time.tm_min += m;
				}
				catch (const std::exception&){}
			}

			std::mktime(&time);			//normalization, handling overflows etc.

			break;
		}

		ss.clear();
	}

	//TODO: function will return std::tm completely filled with zeroes, there could be a necessity to implement a better solution

	return time;
}

DatetimeParser::~DatetimeParser()
{
}
