#include "CommandInterpreter.h"

#include <sstream>
#include <utility>

#include "StringValidation.h"

void CommandInterpreter::addIfUnique(const Command& command, CmdType commandType)
{
	if (commandMap.find(command.getName()) == commandMap.end())
	{
		commandTypeMap[command.getName()] = commandType;
		commandMap[command.getName()] = command;
	}
}

void CommandInterpreter::addIfUnique(Command&& command, CmdType commandType)
{
	if (commandMap.find(command.getName()) == commandMap.end())
	{
		commandTypeMap[command.getName()] = commandType;
		commandMap.insert(std::pair<std::string, Command>(command.getName(), std::move(command)));
	}
}

eprd::InterpretationResult CommandInterpreter::interpret(const std::string& command) const
{
	eprd::InterpretationResult result;

	auto tokens = tokenize(command);
	auto commandName = tokens.front();
	auto commandIterator = commandMap.find(commandName);

	if (commandIterator == commandMap.end())
	{
		result.status = Status::CommandNotFound;
		return result;
	}

	const Command& matchedCommand = commandIterator->second;
	result.commandType = commandTypeMap.at(commandName);
	tokens.pop();													//after popping a command name all strings in the containter represent parameters

	if (tokens.empty())
	{
		if (matchedCommand.areParemetrsExpected())
			result.status = Status::TooFewParameters;
		else
		{
			matchedCommand.invokeCommand();							//command name was matched, no parameters required and no delivered, so command will should be invoked
			result.status = Status::Success;
		}
		return result;
	}

	while (!tokens.empty())
	{
		const auto token = tokens.front();
		tokens.pop();
		auto [parameter, tokenType] = convertToParameter(token);
		MatchResult matchResult = matchedCommand.matchToken(parameter, tokenType);

		if (auto interpretationStatus = getInterpretationStatus(matchResult, tokens); interpretationStatus)
		{
			if (interpretationStatus == Status::Success)
				matchedCommand.invokeCommand();

			result.status = interpretationStatus.value();
			return result;
		}
	}

	return result;
}

CommandInterpreter::Tokens CommandInterpreter::tokenize(const std::string& input) const
{
	Tokens tokens;
	std::stringstream tokenizer(input);

	for (std::string token; !tokenizer.eof(); tokens.push(token))
		tokenizer >> token;

	return tokens;
}

std::pair<ParamVariant, eprd::TokenType> CommandInterpreter::convertToParameter(const std::string& token) const
{
	eprd::TokenType tokenType = eprd::isPositiveInteger(token) ? eprd::TokenType::Integer : eprd::TokenType::String;
	ParamVariant parameter;

	switch (tokenType)
	{
	case EbookPeriodical::TokenType::Integer:
		parameter = std::stoi(token);
		break;
	default:
		parameter = token;
		break;
	}

	return std::make_pair(parameter, tokenType);
}

std::optional<CommandInterpreter::Status> CommandInterpreter::getInterpretationStatus(const MatchResult& matchResult, const Tokens& tokens) const
{
	if (isTooManyParameters(matchResult, tokens))
		return Status::TooManyParameters;

	if (!matchResult.isMatching)
		return Status::ParameterTypesInvalid;

	if (isCommandFullyMatched(matchResult, tokens))
		return Status::Success;

	return std::nullopt;
}

inline bool CommandInterpreter::isTooManyParameters(const MatchResult& matchResult, const Tokens& tokens) const
{
	return matchResult.isBeyondLastToken ||
		(matchResult.isMatching && matchResult.isLastToken && !tokens.empty());
}

inline bool CommandInterpreter::isCommandFullyMatched(const MatchResult& matchResult, const Tokens& tokens) const
{
	return matchResult.isMatching && matchResult.isLastToken && tokens.empty();
}
