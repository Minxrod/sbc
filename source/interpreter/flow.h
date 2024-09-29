#pragma once
/// 
/// @file
/// @brief File contains the various commands that modfiy program control flow.
/// 
#include "common.h"

struct sbc;

void cmd_for(struct sbc* p);
void cmd_to(struct sbc* p);
void cmd_step(struct sbc* p);

/// PTC command marking the end of a `FOR` loop.
/// 
/// Format: 
/// * `NEXT [variable]`
/// 
/// Arguments:
/// * variable - Optional variable for iteration
/// 
/// @param a Arguments
void cmd_next(struct sbc* p) ITCM_CODE;

// IF uses the value on the stack to determine where to jump to next, either the
// THEN/GOTO block or the ELSE block.
void cmd_if(struct sbc* p) ITCM_CODE;

// This command should never exist
void cmd_then(struct sbc* p);

// When hitting an ELSE instruction (only hit from THEN block)
// jump to the next ENDIF.
void cmd_else(struct sbc* p) ITCM_CODE;
void cmd_endif(struct sbc* p);

void cmd_goto(struct sbc* p);
void cmd_gosub(struct sbc* p);
void cmd_on(struct sbc* p);
void cmd_return(struct sbc* p);

/// Implements the `END` command of PTC.
///
/// Terminates the program. Does not allow continues.
///
/// Syntax: `END`
///
/// @param p System struct
void cmd_end(struct sbc* p);

/// Implements the `STOP` command of PTC.
///
/// Breaks the program. Can be continued with `CONT`.
///
/// Syntax: `STOP`
///
/// @param p System struct
void cmd_stop(struct sbc* p);
void cmd_cont(struct sbc* p);
