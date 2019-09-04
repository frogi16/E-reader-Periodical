//	Suppress warnings when using Visual Studio's code analysis feature. Casablanca doesn't exactly meet its standards and clutters output.

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning (disable: ALL_CODE_ANALYSIS_WARNINGS)

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

#pragma warning(pop)