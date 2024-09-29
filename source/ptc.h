#pragma once
/// 
/// @file
/// @brief Defines enumerations for PTC functions and bytecode instructions.
/// 
/// These enums are used both during the tokenization process to store the
/// command/function/operator types, as well as in the compiled bytecode to
/// indicate what commands etc. get executed.

/// Bytecode instruction types. Each one denotes some specific operation to execute.
///
/// Most are two bytes long, though some are larger and some are variable-length.
enum bytecode_ops {
	BC_ARGCOUNT = 'A',
	BC_ARRAY_NAME = 'a',
	BC_BEGIN_LOOP = 'B',
	BC_COMMAND_FIRST = 'c',
	BC_COMMAND = 'C',
	BC_DIM = 'D',
	BC_DATA = 'd',
	BC_ERROR = 'E', // compile error deferred to runtime error
	BC_FUNCTION = 'F',
	BC_GOTO = 'G',
	BC_VARIABLE_ID_SMALL = 'i', //for ID based variable system
	BC_VARIABLE_ID = 'I', //for ID based variable system
	BC_LABEL = 'L',
	BC_LABEL_STRING = 'l',
	BC_OPERATOR = 'O',
	BC_SMALL_NUMBER = 'n', //special case for tiny numbers
	BC_NUMBER = 'N',
	BC_STRING = 'S', //read-only
	BC_VARIABLE_NAME = 'V', //for name based variables
	BC_WIDE_STRING = 'W', //read-only
	BC_SYSVAR = 'Y',
	BC_SYSVAR_VALIDATE = 'y',
};

/// Size of compiled BC_NUMBER
#define BC_NUMBER_SIZE 6
/// Size of generic compiled operation. Some special cases can be larger than this, or variable-length.
#define BC_INSTR_SIZE 2

// Temporary workaround until this ptc.h can be split or moved or removed
#include "func_info.h"

