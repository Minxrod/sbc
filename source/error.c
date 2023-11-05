#include "error.h"

const char * error_messages[] = {
	"OK",
	"BC_SMALL_NUMBER data out of range",
	"Invalid command ID",
	"Invalid function ID",
	"Invalid operator ID",
	"Invalid bytecode",
	"Failed to create variable",
	"PRINT - Invalid operator provided",
	"PRINT - Invalid stack item",
	"Operator types do not match",
	"Functionality currently unimplemented",
	"Operator argument types invalid",
	"Operator argument type invalid",
	"Can't assign to literal",
	"Can't assign to provided type",
	"Can't find a matching FOR for this NEXT",
	"Missing variable initialization in FOR",
	"Can't find ELSE or ENDIF",
	"Wrong type provided",
	"RETURN without GOSUB",
	"Command should not exist",
	"Handling for type is unspecified",
	"Resource type is invalid",
	"String too long",
	"Wrong number of arguments",
	"Label not found",
	"No arguments provided",
	"Missing semicolon",
	"READ command failed",
	"Out of DATA",
	"Type mismatch (function)",
	"Syntax error",
	"Invalid sysvar ID",
	"Incorrect argument value",
	"Can't find a matching NEXT for this FOR",
	"Entered BC_BEGIN_LOOP with no FOR call!",
	"Failed to load file",
};
