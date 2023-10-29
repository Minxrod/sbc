#include "panel.h"

#include "common.h"
#include "error.h"
#include "console.h"
#include "system.h"

void cmd_pnltype(struct ptc* p){
	void* type = STACK_STR(0);
	
	if (str_comp(type, "S\3OFF")){
		p->panel.type = PNL_OFF;
	} else if (str_comp(type, "S\3PNL")){
		p->panel.type = PNL_PNL;
	} else if (str_comp(type, "S\3KYA")){
		p->panel.type = PNL_KYA;
	} else if (str_comp(type, "S\3KYM")){
		p->panel.type = PNL_KYM;
	} else if (str_comp(type, "S\3KYK")){
		p->panel.type = PNL_KYK;
	} else {
		ERROR(ERR_INVALID_ARGUMENT_VALUE);
	}
}

void cmd_pnlstr(struct ptc* p){
	(void)p;
}
