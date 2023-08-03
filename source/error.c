#include "error.h"

const char * error_messages[] = {
	"OK",
	"BC_SMALL_NUMBER data out of range",
	"Invalid command ID",
	"Invalid function ID",
	"Invalid operator ID",
	"Failed to create variable",
	"PRINT - Invalid operator provided",
	"PRINT - Invalid stack item",
	"Operator types do not match",
	"Functionality currently unimplemented",
	"Operator argument types invalid",
	"Operator argument type invalid",
	"Can't assign to literal",
	"Can't assign to provieded type",
	"Can't find a matching FOR for this NEXT",
	"Missing variable initialization in FOR",
	"Can't find ELSE or ENDIF",
};
