#include "test_util.h"

#include "common.h"
#include "interpreter/tokens.h"
#include "program.h"
#include "ptc.h"
#include "interpreter/label.h" // for label generation verification

#include <string.h>

int test_tokens(void){
	// Command stack validation
	{
		ASSERT(!check_cmd("N", 1, "NN"), "[args] 1 for 2");
		ASSERT( check_cmd("NN", 2, "NN"), "[args] 2 for 2");
		ASSERT(!check_cmd("NNN", 3, "NN"), "[args] 3 for 2");
		
		ASSERT( check_cmd("N", 1, "N,NN"), "[args] 1 for 1,2");
		ASSERT( check_cmd("NN", 2, "N,NN"), "[args] 2 for 1,2");
		ASSERT(!check_cmd("NNN", 3, "N,NN"), "[args] 3 for 1,2");
		
		ASSERT(check_cmd("LLLLL", 5, "L,l"), "[args] Multiple labels");
		ASSERT(check_cmd("n", 1, "v,S;v"), "[args] Variable list 1");
		ASSERT(check_cmd("nn", 2, "v,S;v"), "[args] Variable list 2");
		ASSERT(check_cmd("nnn", 3, "v,S;v"), "[args] Variable list 3");
		ASSERT(check_cmd("nnnn", 4, "v,S;v"), "[args] Variable list 4");
		
		ASSERT(check_cmd("", 0, "0"), "[args] Empty args list");
		
		ASSERT(!check_cmd("S", 1, "N"), "[args] Wrong type");
		ASSERT(!check_cmd("N", 1, "S"), "[args] Wrong type");

	}
	
	// Tokenization of array declaration
	{
		char* code = "DIM A[16]\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\020A\001DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of 2D array declaration
	{
		char* code = "DIM A[3,5]\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\003n\005A\002DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM 2");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of multiple array declaration
	{
		char* code = "DIM A[6],B[3,2]\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\006A\001DAn\003n\002A\002DB";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM 1,2");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of complex array declaration
	{
		char* code = "DIM A[6+B[5]]\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\006n\005A\001VBO\000A\001DA";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 14; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Tokenize DIM with enclosed array access");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?PI()\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "A\000F\027c\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 6; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Call function with no arguments");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?ATAN(2,4)\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\002n\004A\002F\002c\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Call function with two simple arguments");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of function call with variable lengths
	{
		char* code = "?ATAN(ABS(2),ABS(6))\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\002A\001F\000n\006A\001F\000A\002F\002c\000";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 18; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Call function with two expression arguments");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Tokenization of array access with assignment
	{
		char* code = "A[0]=7\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\000A\001VAn\007O\006";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 10; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Array access assignment");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// DIM of string array
	{
		char* code = "DIM A$[1]\rA$[0]=\"A\"\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "n\001A\001D\002A$n\000A\001V\002A$S\001A\0O\06";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 22; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Array access assignment");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == 8, "[tokens] Correct line length");
		ASSERT(o.line_length[1] == 14, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// PRINT special eval rules
	{
		char* code = "PRINT A,B,,C;D,E,;F G\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "VAO\1VBO\1O\1VCO\5VDO\1VEO\1O\5VFVGc\0";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 30; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] PRINT implicit and explicit commas/semis");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length"); // Check line length correct
		free_bytecode(o);
	}
	
	// FOR tokenization
	{
		char* code = "FOR I=0 TO 9:?I:NEXT\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		char* bytecode = "VIc\4n\0O\6n\11C\5B\0VIc\0c\7";
		// compile program
		tokenize(&p, &o);
		for (int i = 0; i < 20; i+=1){
			iprintf("%c:%d,", o.data[i], o.data[i]);
			ASSERT(o.data[i] == bytecode[i], "[tokens] Test FOR tokenization");
		}
		iprintf("\n");
		ASSERT(o.line_length[0] == o.size, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// IF tokenization
	{
		ASSERT(
			token_code(
				"IF 0+1 THEN ?\"A\" ELSE A=0:?\"B\"\r",
				"n\0n\1O\0c\10S\1A\0C\0C\12VAn\0O\6S\1B\0c\0C\13",
				30
			), "[tokens] Test IF tokenization"
		);
	}
	
	// Sysvar tokenization
	{
		char bc[] = {
			BC_SYSVAR, SYS_TRUE,
			BC_COMMAND_FIRST, CMD_PRINT
		};
		ASSERT(
			token_code(
				"?TRUE\r",
				bc,
				4
			), "[tokens] Sysvar tokenization"
		);
	}
	
	// IF tokenization II
	{
		ASSERT(
			token_code(
				"IF TRUE THEN A=0:B=1 ELSE B=0:A=1\r",
				"Y\0c\10VAn\0O\6VBn\1O\6C\12VBn\0O\6VAn\1O\6C\13",
				30
			), "[tokens] Test IF tokenization with sysvar"
		);
	}
	
	// Label and GOTO tokenization
	{
		char bc[] = {
			BC_LABEL, 4, 'A', 'B', 'C', 'D',
			BC_LABEL_STRING, 4, 'A', 'B', 'C', 'D',
			BC_COMMAND_FIRST, CMD_GOTO,
		};
		ASSERT(
			token_code(
				"@ABCD\rGOTO @ABCD\r",
				bc, 14
			), "[tokens] Label, GOTO tokenization"
		);
	}
	
	// LOCATE bc generation
	{
		char bc[] = {
			BC_SMALL_NUMBER, 5,
			BC_SMALL_NUMBER, 11,
			BC_COMMAND_FIRST, CMD_LOCATE,
		};
		ASSERT(
			token_code(
				"LOCATE 5,11\r",
				bc, 6
			), "[tokens] LOCATE tokenization"
		);
	}
	
	// DATA tokenization
	{
		char bc[] = "d\0120\000AA\000BCdefd\5 G H \0";
		ASSERT(
			token_code(
				"DATA 0,AA , BCdef   \rDATA \" G H \"\r",
				bc, 20
			), "[tokens] DATA tokenization"
		);
	}
	
	// SAMPLE6 segment that broke (bug with argcount stack)
	{
		char bc[] = "VIVYV\2SYO\2n\2O\4A\1F\15O\6";
		ASSERT(
			token_code(
				"I=FLOOR((Y-SY)/2)\r",
				bc, sizeof bc - 1
			), "[tokens] FLOOR argcount sample6"
		)
	}
	
	// MAINCNTL
	{
		ASSERT(token_code("?MAINCNTL\r", "Y\6c\0", 4), "[tokens] Sysvar with eight character name");
	}
	
	// Line lengths checking
	{
		char* code = "?I\r?I,\r?PI()+43\r?PI()+123456\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		// compile program
		tokenize(&p, &o);
		ASSERT(o.line_length[0] == 4, "[tokens] Correct line length");
		ASSERT(o.line_length[1] == 6, "[tokens] Correct line length");
		ASSERT(o.line_length[2] == 10, "[tokens] Correct line length");
		ASSERT(o.line_length[3] == 14, "[tokens] Correct line length");
		free_bytecode(o);
	}
	
	// Label checking
	{
		char* code = "@0\rI=I+1\r@1\rI=I+1\r@2\r";
		// run program
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		// compile program
		tokenize(&p, &o);
		ASSERT(label_index(o.labels, "0", 1) == 0, "[tokens] Label @0 generated successfully");
		ASSERT(label_index(o.labels, "1", 1) == 14, "[tokens] Label @1 generated successfully");
		ASSERT(label_index(o.labels, "2", 1) == 28, "[tokens] Label @2 generated successfully");
		free_bytecode(o);
	}
	
	// Numbers in other bases
	{
		ASSERT(
			token_code(
				"A=&H4+&H1234+&B00101+&B101010\r",
				"VAn\4N\0\x01\x23\x40\x00O\0n\5O\0n\x2aO\0O\6",
				22
			), "[tokens] Test hex, binary tokenization"
		);
	}
	
	// DATA tokenization of odd length
	{
		// Test with shortcut strings
		ASSERT(
			token_code(
				"DATA A\rDATA AA\rDATA AAA\r DATA AAAA\r",
				"d\1A\0d\2AAd\3AAA\0d\4AAAA",
				20
			), "[tokens] DATA tokenization of odd length"
		);
		// Test with full strings
		ASSERT(
			token_code(
				"DATA \"A\"\rDATA \"AA\"\rDATA \"AAA\"\r DATA \"AAAA\"\r",
				"d\1A\0d\2AAd\3AAA\0d\4AAAA",
				20
			), "[tokens] DATA tokenization of odd length"
		);
	}
	
	SUCCESS("test_tokens success");
}
