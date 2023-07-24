#pragma once

enum err_code {
	ERR_NONE,
	ERR_NUM_INVALID,
	ERR_PTC_COMMAND_INVALID,
	ERR_PTC_FUNCTION_INVALID,
	ERR_PTC_OPERATOR_INVALID,
	ERR_INVALID_BC,
};

extern const char * error_messages[];
