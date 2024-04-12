#include "sbc_memory.h"

#include "system.h"
#include "error.h"
#include "stack.h"
#include "resources.h" // for get_resource, get_resource_type
#include "strs.h"

// pointer conversion PC
#ifdef PC
#define ADDR_MIN 0x02000000
#define ADDR_MAX (ADDR_MIN + MAX_MEMORY - 1)
// only matters on ARM/NDS
#define ADDR_ALIGN false
// note: do not convert NULL
// offset by 0x02000000 is aesthetic to have NDS address format (the values will differ)
#define FIXP_TO_PTR(num) (num ? (u8*)sbc_membase + (num - 0x02000000) : NULL)
#define PTR_TO_FIXP(ptr) (ptr ? (u8*)ptr - (u8*)sbc_membase + 0x02000000 : 0)
#endif
// pointer conversion NDS
#ifdef ARM9
#define ADDR_MIN 0x02000000
#define ADDR_MAX (0x02400000 - 1)
#define ADDR_ALIGN true
#define FIXP_TO_PTR(num) ((void*)num)
#define PTR_TO_FIXP(ptr) ((fixp)ptr)
#endif

#define SBC_POKE(type,invalid_align,opt_shift){\
	fixp addr_num = STACK_NUM(0);\
	void* addr = FIXP_TO_PTR(addr_num);\
	iprintf("POKE at %p\n", addr);\
	if (addr == NULL){\
		ERROR(ERR_NULL_ADDRESS);\
	}\
	if (p->memapi.sys_memsafe){\
		if (addr_num < ADDR_MIN || addr_num > ADDR_MAX){\
			ERROR(ERR_ILLEGAL_ADDRESS);\
		} else if (ADDR_ALIGN && ((uintptr_t)addr & invalid_align)){\
			ERROR(ERR_MISALIGNED_ADDRESS);\
		}\
	}\
	type val = STACK_NUM(1) >> opt_shift;\
	*(type*)addr = val;\
}

void cmd_poke(struct ptc* p){
	SBC_POKE(fixp,0x3,0);
}

void cmd_pokeh(struct ptc* p){
	SBC_POKE(u16,0x1,FIXPOINT);
}

void cmd_pokeb(struct ptc* p){
	SBC_POKE(u8,0x0,FIXPOINT);
}

void cmd_memcopy(struct ptc* p){
	// MEMCOPY dest src size
	fixp dest_num = STACK_NUM(0);
	void* dest = FIXP_TO_PTR(dest_num);
	fixp src_num = STACK_NUM(1);
	void* src = FIXP_TO_PTR(src_num);
	iprintf("MEMCOPY from %p to %p\n", src, dest);\
	if (!dest || !src){
		ERROR(ERR_NULL_ADDRESS);
	}
	int size;
	STACK_INT_MIN(2,0,size);
	if (p->memapi.sys_memsafe){
		if (dest_num < ADDR_MIN || dest_num > ADDR_MAX){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if (src_num < ADDR_MIN || src_num > ADDR_MAX){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if (src_num + size >= ADDR_MAX){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		} else if (dest_num + size >= ADDR_MAX){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		}
	}
	// prevent dest, src overlap
	if (src <= dest && (u8*)dest <= (u8*)src + size){
		// destination range lies partially within src range: illegal
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	} else if (dest <= src && (u8*)src <= (u8*)dest + size){
		// src range lies partially within dest range: illegal
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	
	memcpy(dest, src, size);
}

void cmd_memfill(struct ptc* p){
	// MEMFILL dest value size
	fixp dest_num = STACK_NUM(0);
	void* dest = FIXP_TO_PTR(dest_num);
	u8 value = STACK_INT(1);
	iprintf("MEMCOPY to %p with %d\n", dest, value);\
	if (!dest){
		ERROR(ERR_NULL_ADDRESS);
	}
	int size;
	STACK_INT_MIN(2,0,size);
	if (p->memapi.sys_memsafe){
		if (dest_num < ADDR_MIN || dest_num > ADDR_MAX){
			ERROR(ERR_ILLEGAL_ADDRESS);
		} else if (dest_num + size >= ADDR_MAX){
			// this check is separate to have a different error when offset is
			// out of range vs the initial address
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		}
	}
	
	memset(dest, value, size);
}

#define SBC_PEEK(type,invalid_align,opt_shift){\
	fixp addr_num = STACK_REL_NUM(-1);\
	void* addr = FIXP_TO_PTR(addr_num);\
	--p->stack.stack_i;\
	iprintf("PEEK at %p\n", addr);\
	if (addr == NULL){\
		ERROR(ERR_NULL_ADDRESS);\
	}\
	if (p->memapi.sys_memsafe){\
		if (addr_num < ADDR_MIN || addr_num > ADDR_MAX){\
			ERROR(ERR_ILLEGAL_ADDRESS);\
		} else if (ADDR_ALIGN && ((uintptr_t)addr & invalid_align)){\
			ERROR(ERR_MISALIGNED_ADDRESS);\
		}\
	}\
	fixp ret = (*(type*)addr) << opt_shift;\
	STACK_RETURN_NUM(ret);\
}

void func_peek(struct ptc* p){
	SBC_PEEK(fixp,0x3,0);
}

void func_peekh(struct ptc* p){
	SBC_PEEK(u16,0x1,FIXPOINT);
}

void func_peekb(struct ptc* p){
	SBC_PEEK(u8,0x0,FIXPOINT);
}

#define FUNC_ADDR_VAR_STRUCT '.'
#define FUNC_ADDR_VAR_DATA '&'
#define FUNC_ADDR_INTERNAL '!'
#define FUNC_ADDR_DEREF '*'

void func_addr(struct ptc* p){
	void* resource;
	fixp ofs_num;
	if (p->exec.argcount == 1){
		resource = STACK_REL_STR(-1);
		ofs_num = 0;
	} else {
		resource = STACK_REL_STR(-2);
		ofs_num = STACK_REL_NUM(-1);
	}
	int ofs = FP_TO_INT(ofs_num);
	p->stack.stack_i -= p->exec.argcount;
	
	int len = str_len(resource);
	if (!len){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL); // no string passed
	}
	int addr_type = str_at_char(resource, 0);
	
	void* res_ptr = NULL;
	// -+= * !' ,./ ;:<>
	// resource: GRP0
	// variable (struct entry): .ABC
	// variable (pointer to data): &ABC
	// internal: !SPRITE
	// pointer: (ptr=offset): * (this reads a new pointer from address `offset`)
	if (addr_type == FUNC_ADDR_DEREF){
		if (p->memapi.sys_memsafe){
			if (ofs_num < ADDR_MIN || ofs_num > ADDR_MAX){
				ERROR(ERR_ILLEGAL_ADDRESS);
			} else if (ADDR_ALIGN && (ofs_num & 0x3)){
				ERROR(ERR_MISALIGNED_ADDRESS);
			} else if (FIXP_TO_PTR(ofs_num) == NULL){
				ERROR(ERR_NULL_ADDRESS);
			}
		}
		// dereference this address once
		res_ptr = *(void**)(FIXP_TO_PTR(ofs_num));
		// return here
		STACK_RETURN_NUM(PTR_TO_FIXP(res_ptr));
	} else if (addr_type == FUNC_ADDR_VAR_DATA || addr_type == FUNC_ADDR_VAR_STRUCT){
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
		if (len > 17){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL); // name too long
		}
		// get_var
		char var_name[17]; // skip = at beginning
		str_char_copy(resource, (u8*)var_name);
		struct named_var* v = get_var(&p->vars, var_name+1, len-1, type);
		
		if (addr_type == FUNC_ADDR_VAR_STRUCT){
			res_ptr = v;
		} else {
			// pointer to var data
			// (pointer to number || pointer to string data || pointer to array data)
			if (type == VAR_NUMBER){
				res_ptr = &v->value.number;
			} else if (type == VAR_STRING){
				res_ptr	= str_at(v->value.ptr, 0);
			} else if (type & VAR_ARRAY){ // array type
				res_ptr = v->value.ptr;
			} else {
				ERROR(ERR_ILLEGAL_FUNCTION_CALL);
			}
		}
	} else if (addr_type == FUNC_ADDR_INTERNAL){
		// access to subsystems
		int res_max = 0;
		
		if (str_comp(resource, "S\10!CONSOLE")){
			res_ptr = &p->console;
			res_max = sizeof(p->console);
		} else if (str_comp(resource, "S\3!BG")){
			res_ptr = &p->background;
			res_max = sizeof(p->background);
		} else if (str_comp(resource, "S\7!SPRITE")){
			res_ptr = &p->sprites;
			res_max = sizeof(p->sprites);
		} else if (str_comp(resource, "S\11!GRAPHICS")){
			res_ptr = &p->graphics;
			res_max = sizeof(p->graphics);
		} else if (str_comp(resource, "S\6!PANEL")){
			res_ptr = &p->panel;
			res_max = sizeof(p->panel);
		} else if (str_comp(resource, "S\11!RESOURCE")){
			res_ptr = &p->res;
			res_max = sizeof(p->res);
		} else if (str_comp(resource, "S\6!INPUT")){
			res_ptr = &p->input;
			res_max = sizeof(p->input);
		} else if (str_comp(resource, "S\4!VAR")){
			res_ptr = &p->vars;
			res_max = sizeof(p->vars);
		} else if (str_comp(resource, "S\4!STR")){
			res_ptr = &p->strs;
			res_max = sizeof(p->strs);
		} else if (str_comp(resource, "S\6!ARRAY")){
			res_ptr = &p->arrs;
			res_max = sizeof(p->arrs);
		} else if (str_comp(resource, "S\7!SYSTEM")){
			res_ptr = p;
			res_max = sizeof(*p);
		} else if (str_comp(resource, "S\5!EXEC")){
			res_ptr = &p->exec;
			res_max = sizeof(p->exec);
		} else if (str_comp(resource, "S\7!SOURCE")){
			res_ptr = &p->exec.prg;
			res_max = sizeof(p->exec.prg);
		} else if (str_comp(resource, "S\11!BYTECODE")){
			res_ptr = &p->exec.code;
			res_max = sizeof(p->exec.code);
		} else {
			ERROR(ERR_ILLEGAL_FUNCTION_CALL); // can't find system element
		}
		assert(res_max > (int)sizeof(void*)); // ensure that check is not for pointer instead of value
		if (p->memapi.sys_memsafe){
			if (ofs < 0 || ofs >= res_max){
				ERROR(ERR_ILLEGAL_OFFSET);
			}
		}
	} else if (len <= 5){
		res_ptr = str_to_resource(p, resource);
		if (p->exec.error == ERR_INVALID_RESOURCE_TYPE){
			p->exec.error = ERR_NONE; // allowed here, retry with variable name
		}
		if (p->memapi.sys_memsafe){
			if (ofs < 0 || ofs >= resource_size[get_resource_type(resource)]){
				ERROR(ERR_ILLEGAL_OFFSET);
			}
		}
	}
	
	STACK_RETURN_NUM((PTR_TO_FIXP(res_ptr) + ofs));
}

void func_ptr(struct ptc* p){
	struct string* out_str = get_new_str(&p->strs);
	
	fixp ptr = STACK_REL_NUM(-1);
	--p->stack.stack_i;
	
	out_str->ptr.s[0] = '0';
	out_str->ptr.s[1] = 'x';
	for (int i = 0; i < 8; ++i){
		int d = ptr & 0xf;
		ptr >>= 4;
		out_str->ptr.s[9-i] = hex_digits[d];
	}
	out_str->len = 10;
	out_str->uses = 1;
	
	STACK_RETURN_STR(out_str);
}

void sys_memsafe(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER | VAR_VARIABLE, .value.ptr = &p->memapi.sys_memsafe});
}

void syschk_memsafe(struct ptc* p){
	p->memapi.sys_memsafe = INT_TO_FP(!!p->memapi.sys_memsafe);
}
