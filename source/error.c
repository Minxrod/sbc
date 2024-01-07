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
	"Arugment out of range!",
	"Division by zero!",
	"Overflow!",
	"Subscript out of range",
	"Duplicate definition",
	"Out of memory",
	"Subscript out of range [X]",
	"Subscript out of range [Y]",
	"Command has incorrect arguments",
	"Operator has incorrect arguments",
	"Function has incorrect arguments",
	"Array has incorrect arguments",
	"Unknown token type in tok_test!",
	"Stack not empty after tok_test!",
	"Unknown token type in tok_code!",
	"Failed to generate label entry",
	"File load failed!",
	"Program manually stopped",
	"Illegal function call",
};
