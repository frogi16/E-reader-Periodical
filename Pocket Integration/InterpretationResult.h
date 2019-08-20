#pragma once

#include <optional>

namespace EbookPeriodical
{
	enum class InterpretationStatus
	{
		Success,
		CommandNotFound,
		TooFewParameters,
		TooManyParameters,
		ParameterTypesInvalid
	};

	struct InterpretationResult
	{
		InterpretationStatus status = InterpretationStatus::CommandNotFound;
		std::optional<CommandType> commandType;
	};
}