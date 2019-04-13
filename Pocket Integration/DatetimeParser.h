#pragma once

#include <iomanip>
#include <vector>

class DatetimeParser
{
public:
	DatetimeParser();
	std::tm parseToTM(const std::string & datetime) const;								//parses timezone form given string and returns std::tm object
	time_t parseToTime_t(const std::string & datetime) const;							//calls mktime on parseToTM and returns result (time_t object)
	~DatetimeParser();
protected:
	void modifyTimeForTimezone(std::istringstream &ss, std::tm &time) const noexcept;	//parses timezone in +0300-like format and changes given time object accordingly
	int extractTwoDigitsAsInt(std::string &timezone) const;								//extracts two first digits from string and returns their interpretation as int

	static std::vector<std::string> datetimeFormats;									//vector of predetermined detatime format templates. TODO: consider allowing user of this class to set their own set of templates
};

