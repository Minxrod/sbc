#pragma once

enum err_code {
	ERR_NONE,
	ERR_NUM_INVALID,
	ERR_PTC_COMMAND_INVALID,
	ERR_PTC_FUNCTION_INVALID,
	ERR_PTC_OPERATOR_INVALID,
	ERR_INVALID_BC,
	ERR_VARIABLE_CREATION_FAIL,
	ERR_PRINT_INVALID_OP,
	ERR_PRINT_INVALID_STACK,
	ERR_OP_DIFFERENT_TYPES,
	ERR_UNIMPLEMENTED,
	ERR_OP_INVALID_TYPES,
	ERR_OP_INVALID_TYPE,
	ERR_OP_ASSIGN_TO_LITERAL,
	ERR_OP_ASSIGN_INVALID_TYPE,
	ERR_NEXT_WITHOUT_FOR,
	ERR_MISSING_OP_ASSIGN_FOR,
	ERR_MISSING_ELSE_AND_ENDIF,
	ERR_TYPE_MISMATCH,
	ERR_RETURN_WITHOUT_GOSUB,
	ERR_COMMAND_EXISTS,
	ERR_UNKNOWN_TYPE,
	ERR_INVALID_RESOURCE_TYPE,
	ERR_STRING_TOO_LONG,
	ERR_WRONG_ARG_COUNT,
	ERR_LABEL_NOT_FOUND,
	ERR_NO_ARGUMENTS,
	ERR_MISSING_SEMICOLON,
	ERR_READ_FAILURE,
	ERR_OUT_OF_DATA,
	ERR_FUNC_INVALID_TYPE,
	ERR_SYNTAX,
	ERR_PTC_SYSVAR_INVALID,
	ERR_INVALID_ARGUMENT_VALUE,
	ERR_FOR_WITHOUT_NEXT,
	ERR_BEGIN_LOOP_FAIL,
	ERR_FILE_LOAD_FAILED,
	ERR_OUT_OF_RANGE,
};

extern const char * error_messages[];
