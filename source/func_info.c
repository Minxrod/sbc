#include "func_info.h"

#include "common.h"
const char* commands = 
"PRINT   LOCATE  COLOR   DIM     FOR     TO      STEP    NEXT    IF      "
"THEN    ELSE    ENDIF   GOTO    GOSUB   ON      RETURN  END     STOP    "
"CLS     VISIBLE ACLS    VSYNC   WAIT    INPUT   LINPUT  APPEND  BEEP    "
"BGCLIP  BGCLR   BGCOPY  BGFILL  BGMCLEARBGMPLAY BGMPRG  BGMSET  BGMSETD "
"BGMSETV BGMSTOP BGMVOL  BGOFS   BGPAGE  BGPUT   BGREAD  BREPEAT CHRINIT "
"CHRREAD CHRSET  CLEAR   COLINIT COLREAD COLSET  CONT    DATA    DELETE  "
"DTREAD  EXEC    FILES   GBOX    GCIRCLE GCLS    GCOLOR  GCOPY   GDRAWMD "
"GFILL   GLINE   GPAGE   GPAINT  GPSET   GPRIO   GPUTCHR ICONCLR ICONSET "
"KEY     LIST    LOAD    NEW     PNLSTR  PNLTYPE READ    REBOOT  "
"RECVFILERENAME  RESTORE RSORT   RUN     SAVE    SENDFILESORT    SPANGLE "
"SPANIM  SPCHR   SPCLR   SPCOL   SPCOLVECSPHOME  SPOFS   SPPAGE  SPREAD  "
"SPSCALE SPSET   SPSETV  SWAP    TMREAD  TALK    TALKSTOPPOKE    POKEH   "
"POKEB   MEMCOPY MEMFILL "
;

const char* functions = 
"ABS     ASC     ATAN    BGCHK   BGMCHK  BGMGETV BTRIG   BUTTON  CHKCHR  "
"CHR$    COS     DEG     EXP     FLOOR   GSPOIT  HEX$    ICONCHK INKEY$  "
"INSTR   LEFT$   LEN     LOG     MID$    PI      POW     RAD     RIGHT$  "
"RND     SGN     SIN     SPCHK   SPGETV  SPHIT   SPHITRC SPHITSP SQR     "
"STR$    SUBST$  TAN     VAL     TALKCHK PEEK    PEEKH   PEEKB   ADDR    "
"PTR$    "
;

const char* sysvars = 
"TRUE    FALSE   CANCEL  VERSION TIME$   DATE$   MAINCNTLMAINCNTHFREEVAR "
"FREEMEM PRGNAME$PACKAGE$RESULT  TCHST   TCHX    TCHY    TCHTIME CSRX    "
"CSRY    TABSTEP SPHITNO SPHITX  SPHITY  SPHITT  KEYBOARDFUNCNO  "
"ICONPUSEICONPAGEICONPMAXERL     ERR     MEM$    MEMSAFE "
;

const char* operators = 
"+       ,       -       *       /       ;       =       -       ==      "
"!=      <       >       <=      >=      %       AND     OR      XOR     "
"NOT     (       )       [       ]       !       "
;

DTCM_DATA const sbc_call sbc_commands[] = {
    cmd_print, cmd_locate, cmd_color, cmd_dim, cmd_for, cmd_to, cmd_step,
    cmd_next, cmd_if, cmd_then, cmd_else, cmd_endif, cmd_goto, cmd_gosub,
    cmd_on, cmd_return, cmd_end, cmd_stop, cmd_cls, cmd_visible, cmd_acls,
    cmd_vsync, cmd_wait, cmd_input, cmd_linput, cmd_append, cmd_beep,
    cmd_bgclip, cmd_bgclr, cmd_bgcopy, cmd_bgfill, cmd_bgmclear, cmd_bgmplay,
    cmd_bgmprg, cmd_bgmset, cmd_bgmsetd, cmd_bgmsetv, cmd_bgmstop, cmd_bgmvol,
    cmd_bgofs, cmd_bgpage, cmd_bgput, cmd_bgread, cmd_brepeat, cmd_chrinit,
    cmd_chrread, cmd_chrset, cmd_clear, cmd_colinit, cmd_colread, cmd_colset,
    cmd_cont, cmd_data, cmd_delete, cmd_dtread, cmd_exec, cmd_files, cmd_gbox,
    cmd_gcircle, cmd_gcls, cmd_gcolor, cmd_gcopy, cmd_gdrawmd, cmd_gfill,
    cmd_gline, cmd_gpage, cmd_gpaint, cmd_gpset, cmd_gprio, cmd_gputchr,
    cmd_iconclr, cmd_iconset, cmd_key, cmd_list, cmd_load, cmd_new, cmd_pnlstr,
    cmd_pnltype, cmd_read, cmd_reboot, cmd_recvfile, cmd_rename, cmd_restore,
    cmd_rsort, cmd_run, cmd_save, cmd_sendfile, cmd_sort, cmd_spangle,
    cmd_spanim, cmd_spchr, cmd_spclr, cmd_spcol, cmd_spcolvec, cmd_sphome,
    cmd_spofs, cmd_sppage, cmd_spread, cmd_spscale, cmd_spset, cmd_spsetv,
    cmd_swap, cmd_tmread, cmd_talk, cmd_talkstop, cmd_poke, cmd_pokeh,
    cmd_pokeb, cmd_memcopy, cmd_memfill
};

DTCM_DATA const sbc_call sbc_functions[] = {
    func_abs, func_asc, func_atan, func_bgchk, func_bgmchk, func_bgmgetv,
    func_btrig, func_button, func_chkchr, func_chr, func_cos, func_deg,
    func_exp, func_floor, func_gspoit, func_hex, func_iconchk, func_inkey,
    func_instr, func_left, func_len, func_log, func_mid, func_pi, func_pow,
    func_rad, func_right, func_rnd, func_sgn, func_sin, func_spchk, func_spgetv,
    func_sphit, func_sphitrc, func_sphitsp, func_sqr, func_str, func_subst,
    func_tan, func_val, func_talkchk, func_peek, func_peekh, func_peekb,
    func_addr, func_ptr
};

DTCM_DATA const sbc_call sbc_sysvars[] = {
    sys_true, sys_false, sys_cancel, sys_version, sys_time, sys_date,
    sys_maincntl, sys_maincnth, sys_freevar, sys_freemem, sys_prgname,
    sys_package, sys_result, sys_tchst, sys_tchx, sys_tchy, sys_tchtime,
    sys_csrx, sys_csry, sys_tabstep, sys_sphitno, sys_sphitx, sys_sphity,
    sys_sphitt, sys_keyboard, sys_funcno, sys_iconpuse, sys_iconpage,
    sys_iconpmax, sys_erl, sys_err, sys_mem, sys_memsafe
};

DTCM_DATA const sbc_call sbc_operators[] = {
    op_add, op_comma, op_subtract, op_multiply, op_divide, op_semicolon,
    op_assign, op_negate, op_equal, op_not_equal, op_less, op_greater,
    op_less_equal, op_greater_equal, op_modulo, op_and, op_or, op_xor, op_not,
    op_open_paren, op_close_paren, op_open_bracket, op_close_bracket,
    op_logical_not
};

