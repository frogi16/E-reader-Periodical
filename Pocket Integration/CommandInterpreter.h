#pragma once

#include <map>
#include <queue>

#include "Command.h"
#include "CommandType.h"
#include "InterpretationResult.h"

class CommandInterpreter
{
	using Tokens = std::queue<std::string>;														//type of container which stores tokens while interpretating the command. It is a little easier to implement the logic using queue, but the algorithm can be easily modified and use vector as well.
	using CmdType = eprd::CommandType;
	using Status = eprd::InterpretationStatus;

public:
	CommandInterpreter() = default;
	void addIfUnique(const Command& command, CmdType commandType);								//adds a command if its name is unique
	void addIfUnique(Command&& command, CmdType commandType);
	eprd::InterpretationResult interpret(const std::string& command) const;						//interprets given string and tries to match it to stored commands

private:
	//splits input string into tokens based on whitespaces. Always returns at least 1 token, possibly empty string, which is useful because allows to handle this edge efortlessly case when searching for command name (empty string simply won't be found).
	[[nodiscard]] Tokens tokenize(const std::string& input) const;

	//identifies TokenType and returns proper variant of the parameter - int, string etc.
	[[nodiscard]] std::pair<ParamVariant, eprd::TokenType> convertToParameter(const std::string& token) const;

	//returns status if it may be uniquely, otherwise interpretation should carry on to the next token
	[[nodiscard]] std::optional<Status> getInterpretationStatus(const MatchResult& matchResult, const Tokens& tokens) const;

	inline bool isTooManyParameters(const MatchResult& matchResult, const Tokens& tokens) const;
	inline bool isCommandFullyMatched(const MatchResult& matchResult, const Tokens& tokens) const;

	std::map<std::string, CmdType> commandTypeMap;												//CmdType indexed with command name
	std::map<std::string, Command> commandMap;													//Command indexed with command name
};

