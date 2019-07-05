#include "Command.h"



Command::Command(const std::string & name) :
	name(name),
	currentTokenTypeIt(tokenTypes.begin())
{
}

MatchResult Command::matchToken(const TokenType & tokenType)
{
	MatchResult result;

	result.isMatching = (tokenType == *currentTokenTypeIt);
	result.isLastToken = (currentTokenTypeIt == tokenTypes.end());

	if (currentTokenTypeIt == tokenTypes.end() || result.isMatching == false)
		currentTokenTypeIt = tokenTypes.begin();
	else
		currentTokenTypeIt++;

	return result;
}

Command::~Command()
{
}
