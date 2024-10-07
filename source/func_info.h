#pragma once

enum bc_commands {
    CMD_PRINT, CMD_LOCATE, CMD_COLOR, CMD_DIM, CMD_FOR, CMD_TO, CMD_STEP,
    CMD_NEXT, CMD_IF, CMD_THEN, CMD_ELSE, CMD_ENDIF, CMD_GOTO, CMD_GOSUB,
    CMD_ON, CMD_RETURN, CMD_END, CMD_STOP, CMD_CLS, CMD_VISIBLE, CMD_ACLS,
    CMD_VSYNC, CMD_WAIT, CMD_INPUT, CMD_LINPUT, CMD_APPEND, CMD_BEEP,
    CMD_BGCLIP, CMD_BGCLR, CMD_BGCOPY, CMD_BGFILL, CMD_BGMCLEAR, CMD_BGMPLAY,
    CMD_BGMPRG, CMD_BGMSET, CMD_BGMSETD, CMD_BGMSETV, CMD_BGMSTOP, CMD_BGMVOL,
    CMD_BGOFS, CMD_BGPAGE, CMD_BGPUT, CMD_BGREAD, CMD_BREPEAT, CMD_CHRINIT,
    CMD_CHRREAD, CMD_CHRSET, CMD_CLEAR, CMD_COLINIT, CMD_COLREAD, CMD_COLSET,
    CMD_CONT, CMD_DATA, CMD_DELETE, CMD_DTREAD, CMD_EXEC, CMD_FILES, CMD_GBOX,
    CMD_GCIRCLE, CMD_GCLS, CMD_GCOLOR, CMD_GCOPY, CMD_GDRAWMD, CMD_GFILL,
    CMD_GLINE, CMD_GPAGE, CMD_GPAINT, CMD_GPSET, CMD_GPRIO, CMD_GPUTCHR,
    CMD_ICONCLR, CMD_ICONSET, CMD_KEY, CMD_LIST, CMD_LOAD, CMD_NEW, CMD_PNLSTR,
    CMD_PNLTYPE, CMD_READ, CMD_REBOOT, CMD_RECVFILE, CMD_RENAME, CMD_RESTORE,
    CMD_RSORT, CMD_RUN, CMD_SAVE, CMD_SENDFILE, CMD_SORT, CMD_SPANGLE,
    CMD_SPANIM, CMD_SPCHR, CMD_SPCLR, CMD_SPCOL, CMD_SPCOLVEC, CMD_SPHOME,
    CMD_SPOFS, CMD_SPPAGE, CMD_SPREAD, CMD_SPSCALE, CMD_SPSET, CMD_SPSETV,
    CMD_SWAP, CMD_TMREAD, CMD_TALK, CMD_TALKSTOP, CMD_POKE, CMD_POKEH,
    CMD_POKEB, CMD_MEMCOPY, CMD_MEMFILL, CMD_STRNCOPY, CMD_TOTAL_COUNT
};

enum bc_functions {
    FUNC_ABS, FUNC_ASC, FUNC_ATAN, FUNC_BGCHK, FUNC_BGMCHK, FUNC_BGMGETV,
    FUNC_BTRIG, FUNC_BUTTON, FUNC_CHKCHR, FUNC_CHR, FUNC_COS, FUNC_DEG,
    FUNC_EXP, FUNC_FLOOR, FUNC_GSPOIT, FUNC_HEX, FUNC_ICONCHK, FUNC_INKEY,
    FUNC_INSTR, FUNC_LEFT, FUNC_LEN, FUNC_LOG, FUNC_MID, FUNC_PI, FUNC_POW,
    FUNC_RAD, FUNC_RIGHT, FUNC_RND, FUNC_SGN, FUNC_SIN, FUNC_SPCHK, FUNC_SPGETV,
    FUNC_SPHIT, FUNC_SPHITRC, FUNC_SPHITSP, FUNC_SQR, FUNC_STR, FUNC_SUBST,
    FUNC_TAN, FUNC_VAL, FUNC_TALKCHK, FUNC_PEEK, FUNC_PEEKH, FUNC_PEEKB,
    FUNC_ADDR, FUNC_PTR, FUNC_TOTAL_COUNT
};

enum bc_sysvars {
    SYS_TRUE, SYS_FALSE, SYS_CANCEL, SYS_VERSION, SYS_TIME, SYS_DATE,
    SYS_MAINCNTL, SYS_MAINCNTH, SYS_FREEVAR, SYS_FREEMEM, SYS_PRGNAME,
    SYS_PACKAGE, SYS_RESULT, SYS_TCHST, SYS_TCHX, SYS_TCHY, SYS_TCHTIME,
    SYS_CSRX, SYS_CSRY, SYS_TABSTEP, SYS_SPHITNO, SYS_SPHITX, SYS_SPHITY,
    SYS_SPHITT, SYS_KEYBOARD, SYS_FUNCNO, SYS_ICONPUSE, SYS_ICONPAGE,
    SYS_ICONPMAX, SYS_ERL, SYS_ERR, SYS_MEM, SYS_MEMSAFE, SYS_TOTAL_COUNT
};

enum bc_operators {
    OP_ADD, OP_COMMA, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_SEMICOLON,
    OP_ASSIGN, OP_NEGATE, OP_EQUAL, OP_NOT_EQUAL, OP_LESS, OP_GREATER,
    OP_LESS_EQUAL, OP_GREATER_EQUAL, OP_MODULO, OP_AND, OP_OR, OP_XOR, OP_NOT,
    OP_OPEN_PAREN, OP_CLOSE_PAREN, OP_OPEN_BRACKET, OP_CLOSE_BRACKET,
    OP_LOGICAL_NOT, OP_TOTAL_COUNT
};

struct sbc;

void cmd_print(struct sbc* p);
void cmd_locate(struct sbc* p);
void cmd_color(struct sbc* p);
void cmd_dim(struct sbc* p);
void cmd_for(struct sbc* p);
void cmd_to(struct sbc* p);
void cmd_step(struct sbc* p);
void cmd_next(struct sbc* p);
void cmd_if(struct sbc* p);
void cmd_then(struct sbc* p);
void cmd_else(struct sbc* p);
void cmd_endif(struct sbc* p);
void cmd_goto(struct sbc* p);
void cmd_gosub(struct sbc* p);
void cmd_on(struct sbc* p);
void cmd_return(struct sbc* p);
void cmd_end(struct sbc* p);
void cmd_stop(struct sbc* p);
void cmd_cls(struct sbc* p);
void cmd_visible(struct sbc* p);
void cmd_acls(struct sbc* p);
void cmd_vsync(struct sbc* p);
void cmd_wait(struct sbc* p);
void cmd_input(struct sbc* p);
void cmd_linput(struct sbc* p);
void cmd_append(struct sbc* p);
void cmd_beep(struct sbc* p);
void cmd_bgclip(struct sbc* p);
void cmd_bgclr(struct sbc* p);
void cmd_bgcopy(struct sbc* p);
void cmd_bgfill(struct sbc* p);
void cmd_bgmclear(struct sbc* p);
void cmd_bgmplay(struct sbc* p);
void cmd_bgmprg(struct sbc* p);
void cmd_bgmset(struct sbc* p);
void cmd_bgmsetd(struct sbc* p);
void cmd_bgmsetv(struct sbc* p);
void cmd_bgmstop(struct sbc* p);
void cmd_bgmvol(struct sbc* p);
void cmd_bgofs(struct sbc* p);
void cmd_bgpage(struct sbc* p);
void cmd_bgput(struct sbc* p);
void cmd_bgread(struct sbc* p);
void cmd_brepeat(struct sbc* p);
void cmd_chrinit(struct sbc* p);
void cmd_chrread(struct sbc* p);
void cmd_chrset(struct sbc* p);
void cmd_clear(struct sbc* p);
void cmd_colinit(struct sbc* p);
void cmd_colread(struct sbc* p);
void cmd_colset(struct sbc* p);
void cmd_cont(struct sbc* p);
void cmd_data(struct sbc* p);
void cmd_delete(struct sbc* p);
void cmd_dtread(struct sbc* p);
void cmd_exec(struct sbc* p);
void cmd_files(struct sbc* p);
void cmd_gbox(struct sbc* p);
void cmd_gcircle(struct sbc* p);
void cmd_gcls(struct sbc* p);
void cmd_gcolor(struct sbc* p);
void cmd_gcopy(struct sbc* p);
void cmd_gdrawmd(struct sbc* p);
void cmd_gfill(struct sbc* p);
void cmd_gline(struct sbc* p);
void cmd_gpage(struct sbc* p);
void cmd_gpaint(struct sbc* p);
void cmd_gpset(struct sbc* p);
void cmd_gprio(struct sbc* p);
void cmd_gputchr(struct sbc* p);
void cmd_iconclr(struct sbc* p);
void cmd_iconset(struct sbc* p);
void cmd_key(struct sbc* p);
void cmd_list(struct sbc* p);
void cmd_load(struct sbc* p);
void cmd_new(struct sbc* p);
void cmd_pnlstr(struct sbc* p);
void cmd_pnltype(struct sbc* p);
void cmd_read(struct sbc* p);
void cmd_reboot(struct sbc* p);
void cmd_recvfile(struct sbc* p);
void cmd_rename(struct sbc* p);
void cmd_restore(struct sbc* p);
void cmd_rsort(struct sbc* p);
void cmd_run(struct sbc* p);
void cmd_save(struct sbc* p);
void cmd_sendfile(struct sbc* p);
void cmd_sort(struct sbc* p);
void cmd_spangle(struct sbc* p);
void cmd_spanim(struct sbc* p);
void cmd_spchr(struct sbc* p);
void cmd_spclr(struct sbc* p);
void cmd_spcol(struct sbc* p);
void cmd_spcolvec(struct sbc* p);
void cmd_sphome(struct sbc* p);
void cmd_spofs(struct sbc* p);
void cmd_sppage(struct sbc* p);
void cmd_spread(struct sbc* p);
void cmd_spscale(struct sbc* p);
void cmd_spset(struct sbc* p);
void cmd_spsetv(struct sbc* p);
void cmd_swap(struct sbc* p);
void cmd_tmread(struct sbc* p);
void cmd_talk(struct sbc* p);
void cmd_talkstop(struct sbc* p);
void cmd_poke(struct sbc* p);
void cmd_pokeh(struct sbc* p);
void cmd_pokeb(struct sbc* p);
void cmd_memcopy(struct sbc* p);
void cmd_memfill(struct sbc* p);
void cmd_strncopy(struct sbc* p);

void func_abs(struct sbc* p);
void func_asc(struct sbc* p);
void func_atan(struct sbc* p);
void func_bgchk(struct sbc* p);
void func_bgmchk(struct sbc* p);
void func_bgmgetv(struct sbc* p);
void func_btrig(struct sbc* p);
void func_button(struct sbc* p);
void func_chkchr(struct sbc* p);
void func_chr(struct sbc* p);
void func_cos(struct sbc* p);
void func_deg(struct sbc* p);
void func_exp(struct sbc* p);
void func_floor(struct sbc* p);
void func_gspoit(struct sbc* p);
void func_hex(struct sbc* p);
void func_iconchk(struct sbc* p);
void func_inkey(struct sbc* p);
void func_instr(struct sbc* p);
void func_left(struct sbc* p);
void func_len(struct sbc* p);
void func_log(struct sbc* p);
void func_mid(struct sbc* p);
void func_pi(struct sbc* p);
void func_pow(struct sbc* p);
void func_rad(struct sbc* p);
void func_right(struct sbc* p);
void func_rnd(struct sbc* p);
void func_sgn(struct sbc* p);
void func_sin(struct sbc* p);
void func_spchk(struct sbc* p);
void func_spgetv(struct sbc* p);
void func_sphit(struct sbc* p);
void func_sphitrc(struct sbc* p);
void func_sphitsp(struct sbc* p);
void func_sqr(struct sbc* p);
void func_str(struct sbc* p);
void func_subst(struct sbc* p);
void func_tan(struct sbc* p);
void func_val(struct sbc* p);
void func_talkchk(struct sbc* p);
void func_peek(struct sbc* p);
void func_peekh(struct sbc* p);
void func_peekb(struct sbc* p);
void func_addr(struct sbc* p);
void func_ptr(struct sbc* p);

void sys_true(struct sbc* p);
void sys_false(struct sbc* p);
void sys_cancel(struct sbc* p);
void sys_version(struct sbc* p);
void sys_time(struct sbc* p);
void sys_date(struct sbc* p);
void sys_maincntl(struct sbc* p);
void sys_maincnth(struct sbc* p);
void sys_freevar(struct sbc* p);
void sys_freemem(struct sbc* p);
void sys_prgname(struct sbc* p);
void sys_package(struct sbc* p);
void sys_result(struct sbc* p);
void sys_tchst(struct sbc* p);
void sys_tchx(struct sbc* p);
void sys_tchy(struct sbc* p);
void sys_tchtime(struct sbc* p);
void sys_csrx(struct sbc* p);
void sys_csry(struct sbc* p);
void sys_tabstep(struct sbc* p);
void sys_sphitno(struct sbc* p);
void sys_sphitx(struct sbc* p);
void sys_sphity(struct sbc* p);
void sys_sphitt(struct sbc* p);
void sys_keyboard(struct sbc* p);
void sys_funcno(struct sbc* p);
void sys_iconpuse(struct sbc* p);
void sys_iconpage(struct sbc* p);
void sys_iconpmax(struct sbc* p);
void sys_erl(struct sbc* p);
void sys_err(struct sbc* p);
void sys_mem(struct sbc* p);
void sys_memsafe(struct sbc* p);

void op_add(struct sbc* p);
void op_comma(struct sbc* p);
void op_subtract(struct sbc* p);
void op_multiply(struct sbc* p);
void op_divide(struct sbc* p);
void op_semicolon(struct sbc* p);
void op_assign(struct sbc* p);
void op_negate(struct sbc* p);
void op_equal(struct sbc* p);
void op_not_equal(struct sbc* p);
void op_less(struct sbc* p);
void op_greater(struct sbc* p);
void op_less_equal(struct sbc* p);
void op_greater_equal(struct sbc* p);
void op_modulo(struct sbc* p);
void op_and(struct sbc* p);
void op_or(struct sbc* p);
void op_xor(struct sbc* p);
void op_not(struct sbc* p);
void op_open_paren(struct sbc* p);
void op_close_paren(struct sbc* p);
void op_open_bracket(struct sbc* p);
void op_close_bracket(struct sbc* p);
void op_logical_not(struct sbc* p);

#define MAX_SPECIAL_NAME_SIZE 8
extern const char* commands;
extern const char* functions;
extern const char* sysvars;
extern const char* operators;

struct sbc;
typedef void(*sbc_call)(struct sbc*);

extern const sbc_call sbc_commands[CMD_TOTAL_COUNT];
extern const sbc_call sbc_functions[FUNC_TOTAL_COUNT];
extern const sbc_call sbc_sysvars[SYS_TOTAL_COUNT];
extern const sbc_call sbc_operators[OP_TOTAL_COUNT];

