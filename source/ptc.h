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

/// IDs that correspond to what command is being executed.
/// These appear in the compiled bytecode after a BC_COMMAND instruction and take up one byte.
enum command_ids {
	CMD_PRINT=0, CMD_LOCATE, CMD_COLOR, CMD_DIM,
	CMD_FOR, CMD_TO, CMD_STEP, CMD_NEXT,
	CMD_IF, CMD_THEN, CMD_ELSE, CMD_ENDIF,
	CMD_GOTO, CMD_GOSUB, CMD_ON, CMD_RETURN,
	CMD_END, CMD_STOP,
	CMD_CLS, CMD_VISIBLE, CMD_ACLS, CMD_VSYNC, CMD_WAIT,
	CMD_INPUT, CMD_LINPUT,
	CMD_APPEND, CMD_BEEP,
	CMD_BGCLIP, CMD_BGCLR, CMD_BGCOPY, CMD_BGFILL, CMD_BGMCLEAR, 
	CMD_BGMPLAY, CMD_BGMPRG, CMD_BGMSET, CMD_BGMSETD, CMD_BGMSETV, CMD_BGMSTOP, CMD_BGMVOL, 
	CMD_BGOFS, CMD_BGPAGE, CMD_BGPUT, CMD_BGREAD, CMD_BREPEAT, CMD_CHRINIT, CMD_CHRREAD, 
	CMD_CHRSET, CMD_CLEAR, CMD_COLINIT, CMD_COLREAD, CMD_COLSET, CMD_CONT, 
	CMD_DATA, CMD_DELETE, CMD_DTREAD, CMD_EXEC, CMD_FILES, CMD_GBOX,
	CMD_GCIRCLE, CMD_GCLS, CMD_GCOLOR, CMD_GCOPY, CMD_GDRAWMD, CMD_GFILL, CMD_GLINE, 
	CMD_GPAGE, CMD_GPAINT, CMD_GPSET, CMD_GPRIO, CMD_GPUTCHR, CMD_ICONCLR, CMD_ICONSET, 
	CMD_KEY, CMD_LIST, CMD_LOAD, CMD_NEW, 
	CMD_PNLSTR, CMD_PNLTYPE, CMD_READ, CMD_REBOOT, CMD_RECVFILE, CMD_RENAME, 
	CMD_RESTORE, CMD_RSORT, CMD_RUN, CMD_SAVE, CMD_SENDFILE, CMD_SORT, CMD_SPANGLE, 
	CMD_SPANIM, CMD_SPCHR, CMD_SPCLR, CMD_SPCOL, CMD_SPCOLVEC, CMD_SPHOME, CMD_SPOFS, CMD_SPPAGE,
	CMD_SPREAD, CMD_SPSCALE, CMD_SPSET, CMD_SPSETV, CMD_SWAP,
	CMD_TMREAD,
	CMD_TALK, CMD_TALKSTOP,
};

/// IDs that correspond to what function is being executed.
/// These appear in the compiled bytecode after a BC_FUNCTION instruction and take up one byte.
enum functions {
	FUNC_ABS, FUNC_ASC, FUNC_ATAN, FUNC_BGCHK, FUNC_BGMCHK, FUNC_BGMGETV, FUNC_BTRIG, FUNC_BUTTON,
	FUNC_CHKCHR, FUNC_CHR, FUNC_COS, FUNC_DEG, FUNC_EXP, FUNC_FLOOR, FUNC_GSPOIT, FUNC_HEX, FUNC_ICONCHK,
	FUNC_INKEY, FUNC_INSTR, FUNC_LEFT, FUNC_LEN, FUNC_LOG, FUNC_MID, FUNC_PI, FUNC_POW, FUNC_RAD,
	FUNC_RIGHT, FUNC_RND, FUNC_SGN, FUNC_SIN, FUNC_SPCHK, FUNC_SPGETV, FUNC_SPHIT, FUNC_SPHITRC,
	FUNC_SPHITSP, FUNC_SQR, FUNC_STR, FUNC_SUBST, FUNC_TAN, FUNC_VAL,
	FUNC_TALKCHK,
};

/// IDs that correspond to what system variable is being used by BC_SYSVAR or BC_SYSVAR_VALIDATE.
/// These appear in the compiled bytecode after the instruction and take up one byte.
enum sysvars {
	SYS_TRUE, SYS_FALSE, SYS_CANCEL, SYS_VERSION,
	SYS_TIME, SYS_DATE, SYS_MAINCNTL, SYS_MAINCNTH,
	SYS_FREEVAR, SYS_FREEMEM, SYS_PRGNAME, SYS_PACKAGE, SYS_RESULT,
	SYS_TCHST, SYS_TCHX, SYS_TCHY, SYS_TCHTIME,
	SYS_CSRX, SYS_CSRY, SYS_TABSTEP,
	SYS_SPHITNO, SYS_SPHITX, SYS_SPHITY, SYS_SPHITT,
	SYS_KEYBOARD, SYS_FUNCNO,
	SYS_ICONPUSE, SYS_ICONPAGE, SYS_ICONPMAX,
	SYS_ERL, SYS_ERR,
	SYS_MEM,
	// extra
	SYS_MEMSAFE,
};

/// IDs that correspond to what operation is being calculated by BC_OPERATOR.
/// These appear in the compiled bytecode after the instruction and take up one byte.
enum operations {
	OP_ADD=0, OP_COMMA, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_SEMICOLON,
	OP_ASSIGN, OP_NEGATE,
	OP_EQUAL, OP_NOT_EQUAL, OP_LESS, OP_GREATER, OP_LESS_EQUAL, OP_GREATER_EQUAL,
	OP_MODULO,
	OP_AND, OP_OR, OP_XOR, OP_NOT, OP_LOGICAL_NOT,
	OP_OPEN_PAREN, OP_CLOSE_PAREN, OP_OPEN_BRACKET, OP_CLOSE_BRACKET,
};
