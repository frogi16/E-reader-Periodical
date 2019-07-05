#include "CommandInterpreter.h"

std::regex CommandInterpreter::digitsRegex("[0-9]+");

CommandInterpreter::CommandInterpreter() : matchedCommand(commands.end())
{
}

void CommandInterpreter::addCommand(Command command)
{
	if (std::find(commands.begin(), commands.end(), command) == commands.end())
		commands[command.getName()] = command;
}

InterpretationResult CommandInterpreter::pushToken(const std::string & token)
{
	InterpretationResult result;

	if (matchedCommand == commands.end())		//initial state
	{
		auto iterator = std::find(commands.begin(), commands.end(), token);

		if (iterator == commands.end())
		{
			//no command was matched
		}
		else
		{
			TokenType tokenType = (isPositiveInteger(token) ? TokenType::Integer : TokenType::String);

			MatchResult match = iterator->second.matchToken(tokenType);

			if (match.isMatching && match.isLastToken)
				return iterator->second;
			{
				result.status = InterpretationStatus::InterpretationSuccessful;
				result.commandType =
			}

			if (match.isMatching && !match.isLastToken)
				return iterator->second;
		}
	}

	return Command();
}


CommandInterpreter::~CommandInterpreter()
{
}
