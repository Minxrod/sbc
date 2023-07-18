#include "test_util.h"

#include "common.h"
#include "tokens.h"
#include "program.h"

#include <string.h>

int test_tokens(void){
	// Tokenization of array declaration
	{
		char* code = "DIM A[16]\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\020A\001DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM");
		}
		iprintf("\n");
	}
	
	// Tokenization of 2D array declaration
	{
		char* code = "DIM A[3,5]\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\003n\005A\002DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM 2");
		}
		iprintf("\n");
	}
	
	// Tokenization of multiple array declaration
	{
		char* code = "DIM A[6],B[3,2]\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\006A\001DAn\003n\002A\002DB";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM 1,2");
		}
		iprintf("\n");
	}
	
	// Tokenization of complex array declaration
	{
		char* code = "DIM A[6+B[5]]\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\006n\005A\001VBO\000A\001DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 14; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM with enclosed array access");
		}
		iprintf("\n");
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?PI()\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "A\000F\027C\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM with enclosed array access");
		}
		iprintf("\n");
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?ATAN(2,4)\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\002n\004A\002F\002C\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM with enclosed array access");
		}
		iprintf("\n");
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?ATAN(ABS(2),ABS(6))\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\002A\001F\000n\006A\001F\000A\002F\002C\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 18; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM with enclosed array access");
		}
		iprintf("\n");
	}
	
	// Tokenization of array access with assignment
	{
		char* code = "A[0]=7\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\000A\001VAn\007O\006";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Array access assignment");
		}
		iprintf("\n");
	}
	
	// DIM of string array
	{
		char* code = "DIM A$[1]\rA$[0]=\"A\"\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "n\001A\001D\002A$n\000A\001V\002A$S\001A\0O\06";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 22; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Array access assignment");
		}
		iprintf("\n");
	}
	
	{
		char* code = "PRINT A,B,,C;D,E,;F G\r";
		char code2[64];
		// run program
		struct program p = {
			strlen(code), code,
		};
		struct program o = {
			0, code2,
		};
		
		char* bytecode = "VAO\1VBO\1O\1VCO\5VDO\1VEO\1O\5VFVGC\0";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 30; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] PRINT implicit and explicit commas/semis");
		}
		iprintf("\n");
	}
	
	
	SUCCESS("test_tokens success");
}
