#include "sbc_memory.h"

#include "system.h"
#include "error.h"
#include "stack.h"
#include "resources.h" // for get_resource, get_resource_type
#include "strs.h"

#define SBC_POKE(type,invalid_align,opt_shift){\
	void* addr = (void*)STACK_NUM(0);\
	if (addr == NULL){\
		ERROR(ERR_NULL_ADDRESS);\
	}\
	if (p->memapi.sys_memsafe){\
		if (addr < (void*)0x02000000 || addr >= (void*)0x02400000){\
			ERROR(ERR_ILLEGAL_ADDRESS);\
		} else if ((uintptr_t)addr & invalid_align){\
			ERROR(ERR_MISALIGNED_ADDRESS);\
		}\
	}\
	type val = STACK_NUM(1) >> opt_shift;\
	*(type*)addr = val;\
}

void cmd_poke(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_POKE(fixp,0x3,0);
#endif
}

void cmd_pokeh(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_POKE(u16,0x1,FIXPOINT);
#endif
}

void cmd_pokeb(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_POKE(u16,0x0,FIXPOINT);
#endif
}

void cmd_memcopy(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	// MEMCOPY dest src size
	void* dest = (void*)STACK_NUM(0);
	void* src = (void*)STACK_NUM(1);
	if (!dest || !src){
		ERROR(ERR_NULL_ADDRESS);
	}
	int size;
	STACK_INT_MIN(2,0,size);
	if (p->memapi.sys_memsafe){
		// TODO:CODE:MED Main RAM boundary constants (allow for alternate systems)
		if (dest < (void*)0x02000000 || dest >= (void*)0x02400000){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if (src < (void*)0x02000000 || src >= (void*)0x02400000){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if ((u8*)src + size >= (u8*)0x02400000){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		} else if ((u8*)dest + size >= (u8*)0x02400000){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		}
	}
	// TODO:ERR:LOW prevent dest, src overlap
	if (src <= dest && (u8*)dest <= (u8*)src + size){
		// destination range lies partially within src range: illegal
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	} else if (dest <= src && (u8*)src <= (u8*)dest + size){
		// src range lies partially within dest range: illegal
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	
	memcpy(dest, src, size);
#endif
}

void cmd_memfill(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	// MEMFILL dest value size
	
	void* dest = (void*)STACK_NUM(0);
	u8 value = STACK_INT(1);
	if (!dest){
		ERROR(ERR_NULL_ADDRESS);
	}
	int size;
	STACK_INT_MIN(2,0,size);
	if (p->memapi.sys_memsafe){
		// TODO:CODE:MED Main RAM boundary constants (allow for alternate systems)
		// TODO:PERF:NONE one of these checks is possibly redundant?
		if (dest < (void*)0x02000000 || dest >= (void*)0x02400000){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if ((u8*)dest + size >= (u8*)0x02400000){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		}
	}
	
	memset(dest, value, size);
#endif
}

#define SBC_PEEK(type,invalid_align,opt_shift){\
	void* addr = (void*)STACK_REL_NUM(0);\
	--p->stack.stack_i;\
	if (addr == NULL){\
		ERROR(ERR_NULL_ADDRESS);\
	}\
	if (p->memapi.sys_memsafe){\
		if (addr < (void*)0x02000000 || addr >= (void*)0x02400000){\
			ERROR(ERR_ILLEGAL_ADDRESS);\
		} else if ((uintptr_t)addr & invalid_align){\
			ERROR(ERR_MISALIGNED_ADDRESS);\
		}\
	}\
	fixp ret = (*(type*)addr) << opt_shift;\
	STACK_RETURN_NUM(ret);\
}

void func_peek(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_PEEK(fixp,0x3,0);
#endif
}

void func_peekh(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_PEEK(fixp,0x1,FIXPOINT);
#endif
}

void func_peekb(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	SBC_PEEK(fixp,0x0,FIXPOINT);
#endif
}

void func_addr(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // not supported due to fixp being too small for pointers
#endif
#ifdef ARM9
	void* resource;
	int ofs;
	if (p->exec.argcount == 1){
		resource = STACK_REL_STR(-1);
		ofs = 0;
	} else {
		resource = STACK_REL_STR(-2);
		ofs = STACK_REL_INT(-1);
	}
	p->stack.stack_i -= p->exec.argcount;
	int len = str_len(resource);
	
	void* res_ptr = NULL;
	if (len <= 5){
		res_ptr = str_to_resource(p, resource);
		if (p->exec.error == ERR_INVALID_RESOURCE_TYPE){
			p->exec.error = ERR_NONE; // allowed here, retry with variable name
		}
		if (ofs < 0 || ofs > resource_size[get_resource_type(resource)]){
			ERROR(ERR_ILLEGAL_ADDRESS);
		}
		// TODO:IMPL:LOW allow access to PRG source and bytecode (PBC?)
	}
	if (!res_ptr){
		int type = VAR_EMPTY;
		if (len > 2 && str_at_char(resource, len-1) == ']' && str_at_char(resource, len-2) == '['){
			len -= 2;
			type |= VAR_ARRAY;
		}
		if (len > 1 && str_at_char(resource, len-1) == '$'){
			--len;
			type |= VAR_STRING;
		} else {
			type |= VAR_NUMBER;
		}
		if (len > 16){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL); // name too long
		}
		// get_var
		char var_name[16];
		str_char_copy(resource, (u8*)var_name);
		struct named_var* v = get_var(&p->vars, var_name, len, type);
		
		res_ptr = &v->value; // pointer to number || pointer to string pointer || pointer to array data
		if (ofs != 0){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL); // no offsets allowed on variable pointers
		}
	}
	
	if (!res_ptr){
		STACK_RETURN_INT(0);
	}
	// offset only allowed on non-null
	STACK_RETURN_NUM(((fixp)res_ptr) + ofs);
#endif
}

void func_ptr(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); // might allow formatting anyways
#endif
	ERROR(ERR_UNIMPLEMENTED); // might allow formatting anyways
}

void sys_memsafe(struct ptc* p){
#ifdef PC
	ERROR(ERR_UNIMPLEMENTED); //maybe a no-op in the future
#endif
	ERROR(ERR_UNIMPLEMENTED); // might allow formatting anyways
}
