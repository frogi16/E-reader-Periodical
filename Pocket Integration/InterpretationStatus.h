#pragma once

enum class InterpretationStatus
{
	InterpretationSuccessful,
	CommandNotFound,
	TooFewParameters,
	TooMuchParameters,
	ParameterTypeInvalid
};