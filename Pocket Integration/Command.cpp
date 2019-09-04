#include "Command.h"


void Command::invokeCommand() const
{
	if (parameters.size() == expectedParameterTypes.size() && callbackFunction)
	{
		std::invoke(callbackFunction, parameters);
		parameters.clear();
	}
}

MatchResult Command::matchToken(const ParamVariant& parameter, const eprd::TokenType& tokenType) const
{
	MatchResult result;
	result.isLastToken = parameters.size() == expectedParameterTypes.size() - 1;
	result.isBeyondLastToken = (parameters.size() >= expectedParameterTypes.size());
	result.isMatching = (!result.isBeyondLastToken && tokenType == expectedParameterTypes[parameters.size()]);

	parameters.push_back(parameter);

	if (!result.isMatching)
		parameters.clear();

	return result;
}