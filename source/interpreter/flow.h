#pragma once

struct ptc;

void cmd_for(struct ptc* p);
void cmd_to(struct ptc* p);
void cmd_step(struct ptc* p);

/// PTC command marking the end of a `FOR` loop.
/// 
/// Format: 
/// * `NEXT [variable]`
/// 
/// Arguments:
/// * variable - Optional variable for iteration
/// 
/// @param a Arguments
void cmd_next(struct ptc* p);

// IF uses the value on the stack to determine where to jump to next, either the
// THEN/GOTO block or the ELSE block.
void cmd_if(struct ptc* p);

// This command should never exist
void cmd_then(struct ptc* p);

// When hitting an ELSE instruction (only hit from THEN block)
// jump to the next ENDIF.
void cmd_else(struct ptc* p);
void cmd_endif(struct ptc* p);


void cmd_goto(struct ptc* p);
void cmd_gosub(struct ptc* p);
void cmd_on(struct ptc* p);
void cmd_return(struct ptc* p);

void cmd_end(struct ptc* p);
void cmd_stop(struct ptc* p);