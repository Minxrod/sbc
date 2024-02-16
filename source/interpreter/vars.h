#pragma once
/// 
/// @file
/// @brief Variable management struct and functions.
/// 

#include "common.h"
#include "error.h"

/**
 * Variable and value types. Used for both @sa stack_entry and @sa named_var.
 */
enum types {
	/// Special value denoting an empty variable slot.
	/// Cannot be combined with any other type.
	VAR_EMPTY=0,
	/// Indicates a numeric 20.12 fixed-point value.
	/// @warn Only one of VAR_NUMBER and VAR_STRING should be selected.
	VAR_NUMBER=1,
	/// Indicates a string of maximum MAX_STRLEN characters.
	/// @warn Only one of VAR_NUMBER and VAR_STRING should be selected.
	VAR_STRING=2,
	/// Denotes that the value contained is a variable pointer.
	/// Can be combined with VAR_NUMBER or VAR_STRING.
	VAR_VARIABLE=4,
	/// Denotes that the value contained is an array type.
	/// Can be combined with VAR_NUMBER or VAR_STRING.
	VAR_ARRAY=8,
	/// This one is silly. It marks an operator such as , or ;.
	STACK_OP=16,
};

// This file defines the data structures used to store variable information.

// Notes on vars:
// There are 6 types of variables.
// Number - 20.12 fixed point
// String - 0-256 character sequence
// Array - One of the above types, but an array. Either 1D or 2D.

/// Contains either a number or a pointer to a number/string/array.
/// Or pointer to variable (this takes the form of a pointer to number or pointer to string/array)
union value {
	fixp number;
	void* ptr;
};

/// Variable table entry. Contains the value directly in the case of numbers,
/// otherwise contains a pointer to the value. Contains the type to determine
/// what type of value is contained.
struct named_var {
	/// Type of variable stored. See @sa types.
	uint_fast8_t type;
	/// The name of this variable. Maxmimum of 16 characters.
	char name[16];
	/// The value stored in this variable.
	union value value;
};

struct strings;
struct arrays;

struct variables {
	/// Maximum variables allowed
	uint_fast16_t vars_max;
	/// Current variable usage count
	uint_fast16_t var_count;
	/// Strings table
	struct strings* strs;
	/// Arrays table
	struct arrays* arrs;
	/// Variable table
	struct named_var* vars;
	/// Error status for variable accesses
	enum err_code error;
	/// Indicates to clear types (default) or clear values and keep type info
	/// Type info is used with TOKOPT_VARIABLE_IDS
	bool clear_values;
};

// allocate var table for `var_count` variables
void init_mem_var(struct variables* v, uint_fast16_t var_count);
void reset_var(struct variables* v);
void free_mem_var(struct variables* v);

// Expects null-terminated name
struct named_var* test_var(struct variables* v, char*, enum types type);
// Name and length required

/**
 * Gets a pointer to a variable using the variable name.
 * The name itself does not determine the variable type. It must be passed separately.
 */
struct named_var* get_var(struct variables* v, char*, u32, enum types type);
// Gets a pointer to a value within an array
union value* get_arr_entry(struct variables* v, char*, u32, enum types type, u32 ix, u32 iy);
// Gets a pointer to a value within an array, but the array is passed via value pointer
union value* get_arr_entry_via_ptr(struct variables* v, union value* arr, u32 ix, u32 iy, int type);

struct named_var* get_new_arr_var(struct variables* v, char* name, u32 len, enum types type, u32 dim1, u32 dim2);

