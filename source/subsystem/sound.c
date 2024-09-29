// #include "sound.h"
// or something

#include "system.h"

void cmd_beep(struct sbc* s){
	(void)s;
}

void cmd_bgmplay(struct sbc* s){
	(void)s;
}

void cmd_bgmset(struct sbc* s){
	(void)s;
}

void cmd_bgmsetd(struct sbc* s){
	(void)s;
}

void cmd_bgmvol(struct sbc* s){
	(void)s;
}

void cmd_bgmstop(struct sbc* s){
	(void)s;
}

void cmd_bgmclear(struct sbc* s){
	(void)s;
}

void cmd_bgmprg(struct sbc* s){
	(void)s;
}

void cmd_bgmsetv(struct sbc* s){
	(void)s;
}

// These functions aren't really implemented evne normally but still need to exist
// If anyone has a japanese copy of the game...
void cmd_talk(struct sbc* s){
	(void)s;
}

void cmd_talkstop(struct sbc* s){
	(void)s;
}

// Functions
void func_bgmchk(struct sbc* p){
	p->stack.stack_i -= p->exec.argcount;
	STACK_RETURN_INT(0);
}

void func_bgmgetv(struct sbc* p){
	p->stack.stack_i -= p->exec.argcount;
	STACK_RETURN_INT(0);
}

void func_talkchk(struct sbc* p){
	p->stack.stack_i -= p->exec.argcount;
	STACK_RETURN_INT(0);
}
