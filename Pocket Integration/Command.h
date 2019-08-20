#pragma once

#include <vector>
#include <string>
#include <variant>
#include <functional>

#include "TokenType.h"

namespace eprd = EbookPeriodical;
using ParamVariant = std::variant<std::string, int>;

struct MatchResult
{
	bool isMatching;
	bool isLastToken;
	bool isBeyondLastToken;
};

class Command
{
public:
	Command() = default;
	Command(const std::string& name, std::function<void(std::vector<ParamVariant>&)> callback) : name(name), callbackFunction(callback) {}
	void setCallbackFunction(std::function<void(std::vector<ParamVariant>&)> callback) { callbackFunction = callback; }
	std::string getName() const { return name; }
	void addExpectedParameter(eprd::TokenType type) { expectedParameterTypes.push_back(type); }
	bool areParemetrsExpected() const { return expectedParameterTypes.size() > 0; }
	void invokeCommand() const;
	MatchResult matchToken(const ParamVariant& parameter, const eprd::TokenType & tokenType) const;
private:
	std::string name;
	std::vector<eprd::TokenType> expectedParameterTypes;
	std::function<void(std::vector<ParamVariant>&)> callbackFunction;
	mutable std::vector<ParamVariant> parameters;
};

