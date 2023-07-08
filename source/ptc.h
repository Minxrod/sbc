
enum bytecode_ops {
	BC_ARGCOUNT = 'A',
	BC_COMMAND = 'C',
	BC_FUNCTION = 'F',
	BC_GOTO = 'G',
	BC_OPERATOR = 'O',
	BC_SMALL_NUMBER = 'n', //special case for tiny numbers
	BC_NUMBER = 'N',
	BC_STRING = 'S', //read-only
	BC_VARIABLE_ID = 'v', //for ID based variables
	BC_VARIABLE_NAME = 'V', //for name based variables
	BC_WIDE_STRING = 'W', //read-only
};

enum command_ids {
	CMD_PRINT=0, CMD_LOCATE, CMD_COLOR,
	/*
	CMD_ACLS, CMD_APPEND, CMD_BEEP, CMD_BGCLIP, CMD_BGCLR, CMD_BGCOPY, CMD_BGFILL, CMD_BGMCLEAR, 
	CMD_BGMPLAY, CMD_BGMPRG, CMD_BGMSET, CMD_BGMSETD, CMD_BGMSETV, CMD_BGMSTOP, CMD_BGMVOL, 
	CMD_BGOFS, CMD_BGPAGE, CMD_BGPUT, CMD_BGREAD, CMD_BREPEAT, CMD_CHRINIT, CMD_CHRREAD, 
	CMD_CHRSET, CMD_CLEAR, CMD_CLS, CMD_COLINIT, CMD_COLREAD, CMD_COLSET, CMD_CONT, 
	CMD_DATA, CMD_DELETE, CMD_DIM, CMD_DTREAD, CMD_ELSE, CMD_END, CMD_EXEC, CMD_FOR, CMD_GBOX, 
	CMD_GCIRCLE, CMD_GCLS, CMD_GCOLOR, CMD_GCOPY, CMD_GDRAWMD, CMD_GFILL, CMD_GLINE, CMD_GOSUB, 
	CMD_GOTO, CMD_GPAGE, CMD_GPAINT, CMD_GPSET, CMD_GPRIO, CMD_GPUTCHR, CMD_ICONCLR, CMD_ICONSET, 
	CMD_IF, CMD_INPUT, CMD_KEY, CMD_LINPUT, CMD_LIST, CMD_LOAD, CMD_LOCATE, CMD_NEW, CMD_NEXT, 
	CMD_ON, CMD_PNLSTR, CMD_PNLTYPE, CMD_READ, CMD_REBOOT, CMD_RECVFILE, CMD_RENAME, 
	CMD_RESTORE, CMD_RETURN, CMD_RSORT, CMD_RUN, CMD_SAVE, CMD_SENDFILE, CMD_SORT, CMD_SPANGLE, 
	CMD_SPANIM, CMD_SPCHR, CMD_SPCLR, CMD_SPCOL, CMD_SPCOLVEC, CMD_SPHOME, CMD_SPOFS, CMD_SPPAGE,
	CMD_SPREAD, CMD_SPSCALE, CMD_SPSET, CMD_SPSETV, CMD_STEP, CMD_STOP, CMD_SWAP, CMD_THEN, 
	CMD_TMREAD, CMD_TO, CMD_VISIBLE, CMD_VSYNC, CMD_WAIT*/
};

enum functions {
	FUNC_PI=0,
	/*
	FUN_ABS=0, FUN_ASC, FUN_ATAN, FUN_BGCHK, FUN_BGMCHK, FUN_BGMGETV, FUN_BTRIG, FUN_BUTTON,
	FUN_CHKCHR, FUN_CHR$, FUN_COS, FUN_DEG, FUN_EXP, FUN_FLOOR, FUN_GSPOIT, FUN_HEX$, FUN_ICONCHK,
	FUN_INKEY$, FUN_INSTR, FUN_LEFT$, FUN_LEN, FUN_LOG, FUN_MID$, FUN_PI, FUN_POW, FUN_RAD,
	FUN_RIGHT$, FUN_RND, FUN_SGN, FUN_SIN, FUN_SPCHK, FUN_SPGETV, FUN_SPHIT, FUN_SPHITRC,
	FUN_SPHITSP, FUN_SQR, FUN_STR$, FUN_SUBST$, FUN_TAN, FUN_VAL,*/
};

enum sysvars {
	SYS_TRUE, SYS_FALSE, SYS_CANCEL, SYS_VERSION,
	SYS_TIME$, SYS_DATE$, SYS_MAINCNTL, SYS_MAINCNTH,
	SYS_FREEVAR, SYS_FREEMEM, SYS_PRGNAME$, SYS_PACKAGE$, SYS_RESULT,
	SYS_TCHST, SYS_TCHX, SYS_TCHY, SYS_TCHTIME,
	SYS_CSRX, SYS_CSRY, SYS_TABSTEP,
	SYS_SPHITNO, SYS_SPHITX, SYS_SPHITY, SYS_SPHITT,
	SYS_KEYBOARD, SYS_FUNCNO,
	SYS_ICONPUSE, SYS_ICONPAGE, SYS_ICONPMAX,
	SYS_ERL, SYS_ERR
};

enum operations {
	OP_ADD=0, OP_COMMA, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_SEMICOLON,
	OP_ASSIGN, OP_NEGATE,
	/*OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_MODULO, 
	OP_AND, OP_OR, OP_NOT, OP_XOR, OP_NEGATE, OP_LOGICAL_NOT,
	OP_GREATER, OP_GREATER_EQUAL, OP_LESS, OP_LESS_EQUAL, OP_EQUAL, OP_NOT_EQUAL,
	OP_ASSIGN, OP_SEMICOLON,*/
};
