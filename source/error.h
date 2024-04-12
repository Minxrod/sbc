#pragma once
/// 
/// @file
/// @brief Contains error codes and associated error messages.
/// 
/// Contains all error codes for SBC. Note that this includes a large variety 
/// of implementation-specific internal errors.

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
	ERR_DIVIDE_BY_ZERO,
	ERR_OVERFLOW,
	ERR_SUBSCRIPT_OUT_OF_RANGE,
	ERR_DUPLICATE_DIM,
	ERR_OUT_OF_MEMORY,
	ERR_SUBSCRIPT_OUT_OF_RANGE_X,
	ERR_SUBSCRIPT_OUT_OF_RANGE_Y,
	ERR_TEST_COMMAND,
	ERR_TEST_OPERATION,
	ERR_TEST_FUNCTION,
	ERR_TEST_ARRAY,
	ERR_TEST_UNKNOWN,
	ERR_TEST_STACK,
	ERR_UNKNOWN_TOKEN_TYPE,
	ERR_LABEL_ADD_FAILURE,
	ERR_BREAK,
	ERR_ILLEGAL_FUNCTION_CALL,
	ERR_INVALID_OPTS,
	ERR_LENGTH_INVARIANT_BROKEN,
	ERR_LABEL_TOO_LONG,
	ERR_ILLEGAL_ADDRESS,
	ERR_NULL_ADDRESS,
	ERR_MISALIGNED_ADDRESS,
	ERR_UNKNOWN_TRANSITION,
	ERR_BUTTON_SIGNAL,
	ERR_USER_SIGNAL,
	ERR_FILE_SAVE_FAILED,
	ERR_ILLEGAL_OFFSET,
	ERR_SORT_SAME_ARRAY,
	ERR_BEGIN_LOOP_NO_END,
	ERR_PAINT_STACK_OVERFLOW,
};

extern const char * error_messages[];
