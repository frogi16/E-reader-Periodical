#pragma once

#include <vector>

#include "TokenType.h"

struct MatchResult
{
	bool isMatching;
	bool isLastToken;
};

class Command
{
public:
	Command(const std::string & name);
	inline std::string getName() const { return name; }
	inline void addParameter(TokenType type) { tokenTypes.push_back(type); }
	MatchResult matchToken(const TokenType & tokenType);
	~Command();
private:
	std::string name;
	std::vector<TokenType> tokenTypes;
	std::vector<TokenType>::iterator currentTokenTypeIt;
};

