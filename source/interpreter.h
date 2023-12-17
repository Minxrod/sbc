/**
 * @file Combination header that inclues core interpreter functionality
 */

// Used more or less everywhere
#include "common.h"

// Core data storage types and functions to operate on them
#include "interpreter/arrays.h"
#include "interpreter/vars.h"
#include "interpreter/strs.h"

// Common functions on core types
#include "interpreter/mathptc.h"
#include "interpreter/strfuncs.h"

// Program initialization and execution
#include "interpreter/tokens.h"
#include "interpreter/runner.h"
#include "interpreter/flow.h"
#include "interpreter/stack.h"

