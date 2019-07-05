#pragma once

#include <map>
#include <regex>

#include "Command.h"
#include "CommandType.h"
#include "InterpretationStatus.h"

struct InterpretationResult
{
	InterpretationStatus status;
	CommandType commandType;
};

class CommandInterpreter
{
public:
	CommandInterpreter();
	void addCommand(Command command);
	InterpretationResult pushToken(const std::string& token);
	~CommandInterpreter();
private:
	std::map<std::string, CommandType> commandTypes;
	std::map<std::string, Command> commands;
	std::map<std::string, Command>::iterator matchedCommand;

	static std::regex digitsRegex;
	inline bool isPositiveInteger(const std::string & str) const { return (std::regex_search(str, digitsRegex)); }
};

