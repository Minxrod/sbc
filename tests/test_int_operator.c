#include "input.h"

//#include "common.h"
#include "test_util.h"

#include "system.h"

// This checks for errors due to missing values. Extant value must be valid.
#define TEST_BINARY_OP_MISSING(msg,op,value) do {\
	DENY(check_code_error("A="op" "value"\r", ERR_NONE), "["msg"] Error on missing argument 1");\
	DENY(check_code_error("A="value" "op"\r", ERR_NONE), "["msg"] Error on missing argument 2");\
	DENY(check_code_error("A="op"\r", ERR_NONE), "["msg"] Error on missing argument 1+2");\
} while(0)\

// This checks for errors due to invalid operand types
#define TEST_BINARY_OP_TYPE(msg,op,valid,invalid) do {\
	DENY(check_code_error("A="invalid" "op" "valid"\r", ERR_NONE), "["msg"] Error on incorrect type 1");\
	DENY(check_code_error("A="valid" "op" "invalid"\r", ERR_NONE), "["msg"] Error on incorrect type 2");\
	DENY(check_code_error("A="invalid" "op" "invalid"\r", ERR_NONE), "["msg"] Error on incorrect types 1+2");\
	ASSERT(check_code_error("A="valid" "op" "valid"\r", ERR_NONE), "["msg"] Valid types are accepted");\
} while(0)

#define TEST_BINARY_OP_ERR_MIX_TYPE(msg,op) do {\
	DENY(check_code_error("A=1 "op" \"\"\r", ERR_NONE), "["msg"] Error on N=NS");\
	DENY(check_code_error("A=\"\" "op" 1\r", ERR_NONE), "["msg"] Error on N=SN");\
	DENY(check_code_error("A$=1 "op" \"\"\r", ERR_NONE), "["msg"] Error on S=NS");\
	DENY(check_code_error("A$=\"\" "op" 1\r", ERR_NONE), "["msg"] Error on S=SN");\
} while(0)

#define TEST_BINARY_OP_VALID_DUAL_TYPE(msg,op) do {\
	ASSERT(check_code_error("A=1 "op" 1\r", ERR_NONE), "["msg"] Valid N=NN accepted");\
	ASSERT(check_code_error("A=\"\" "op" \"\"\r", ERR_NONE), "["msg"] Valid N=SS accepted");\
} while(0)

// This checks for the tokenization errors common to exclusively numeric binary operators.
#define TEST_BINARY_NUM_OP(msg,op) do {\
	/* Error on missing operand */\
	TEST_BINARY_OP_MISSING(msg,op,"1");\
	/* Error on invalid types */\
	TEST_BINARY_OP_TYPE(msg,op,"1","\"\"");\
} while(0)

/// This checks for tokenization errors for types of form N=NN or N=SS
#define TEST_BINARY_DUAL_OP(msg,op) do {\
	/* Missing operand on numeric types */\
	TEST_BINARY_OP_MISSING(msg,op,"1");\
	/* Missing operand on string types */\
	TEST_BINARY_OP_MISSING(msg,op,"\"\"");\
	/* Error on mixed types*/\
	TEST_BINARY_OP_ERR_MIX_TYPE(msg,op);\
	/* Valid type combinations */\
	TEST_BINARY_OP_VALID_DUAL_TYPE(msg,op);\
} while(0)

#define TEST_UNARY_NUM_OP(msg,op) do {\
	DENY(check_code_error("A="op" \"\"\r", ERR_NONE), "["msg"] Error on unary string operand");\
	DENY(check_code_error("A="op"\r", ERR_NONE), "["msg"] Error on missing operand");\
	ASSERT(check_code_error("A="op" 1\r", ERR_NONE), "["msg"] Valid unary numeric operand");\
} while(0)

// check expression result
#define CHECK_NUM_EXP(c,z) do {\
	char* code = "A="c"\r";\
	struct ptc* p = run_code(code);\
	ASSERT(p->exec.error == ERR_NONE, "[result] No error");\
	CHECK_VAR_NUM("A",z);\
	free_code(p);\
} while(0)

// check expression result
#define CHECK_INT_EXP(c,z) do {\
	CHECK_NUM_EXP(c,INT_TO_FP(z));\
} while(0)

// check string result
#define CHECK_STR_EXP(c,s) do {\
	char* code = "A$="c"\r";\
	struct ptc* p = run_code(code);\
	ASSERT(p->exec.error == ERR_NONE, "[result] No error");\
	CHECK_VAR_STR("A",s);\
	free_code(p);\
} while(0)

// check that expression causes an error
#define CHECK_ERR_EXP(c,e) do {\
	ASSERT(check_code_error("_="c"\r", e), "[error] Error: "#e);\
} while(0)

// check that a string expression causes an error
#define CHECK_ERR_STR_EXP(c,e) do {\
	ASSERT(check_code_error("_$="c"\r", e), "[error] Error: "#e);\
} while(0)

// TODO:PERF:MED reuse system to save a lot of pointless allocations
int test_int_operator(void){
	/* 
	 * operation error testing
	 * note that all of these depend on assignment and variables
	 */
	// "normal" binary ops
	{
		TEST_BINARY_NUM_OP("divide","/");
		TEST_BINARY_NUM_OP("modulo","%");
		TEST_BINARY_NUM_OP("less","<");
		TEST_BINARY_NUM_OP("greater",">");
		TEST_BINARY_NUM_OP("lessequal","<=");
		TEST_BINARY_NUM_OP("greaterequal",">=");
		TEST_BINARY_DUAL_OP("equal","==");
		TEST_BINARY_DUAL_OP("inequal","!=");
		TEST_BINARY_NUM_OP("xor","XOR");
		TEST_BINARY_NUM_OP("and","AND");
		TEST_BINARY_NUM_OP("or","OR");
	}
	
	// + is special because the return type matches the arguments
	{
		TEST_BINARY_OP_MISSING("plus","+","1"); // missing numeric
		TEST_BINARY_OP_MISSING("plus","+","\"\""); // missing string
		TEST_BINARY_OP_ERR_MIX_TYPE("plus","+");
		// N=NN and S=SS
		ASSERT(check_code_error("A=1 + 1\r", ERR_NONE), "[plus] Valid N=N+N accepted");
		ASSERT(check_code_error("A$=\"\" + \"\"\r", ERR_NONE), "[plus] Valid S=S+S accepted");
	}
	
	// * is special because it can be N*N or S*N (and return type changes)
	{
		TEST_BINARY_OP_MISSING("multiply","*","1"); // missing numeric
		DENY(check_code_error("A=1 * \"\"\r", ERR_NONE), "[multiply] Error on N=NS");
		DENY(check_code_error("A=\"\" * 1\r", ERR_NONE), "[multiply] Error on N=SN");
		DENY(check_code_error("A$=1 * \"\"\r", ERR_NONE), "[multiply] Error on S=NS");
		DENY(check_code_error("A$=\"\" * \"\"\r", ERR_NONE), "[multiply] Error on S=SS");
		// N=NN and S=SS
		ASSERT(check_code_error("A=1 * 1\r", ERR_NONE), "[multiply] Valid N=N*N accepted");
		ASSERT(check_code_error("A$=\"\" * 1\r", ERR_NONE), "[multiply] Valid S=S*N accepted");
	}
	
	// - is special because it can be unary or binary
	{
		TEST_UNARY_NUM_OP("negate","-");
		// check binary cases that doesn't become unary
		DENY(check_code_error("A=1 -\r", ERR_NONE), "[minus] Error on missing argument 2");
		// check binary arguments must match
		TEST_BINARY_OP_TYPE("minus","-","1","\"\"");
	}
	
	// "normal" unary ops
	{
		TEST_UNARY_NUM_OP("bitwise_not","NOT");
		TEST_UNARY_NUM_OP("logical_not","!");
	}
	
	/*
	 * operator behavior testing
	 */
	// addition
	{
		// correct behavior
		CHECK_INT_EXP("1+1",2);
		CHECK_INT_EXP("1234+5678",6912);
		CHECK_NUM_EXP(".1+.1", (4096/10)*2);
		
		// runtime errors
		CHECK_ERR_EXP("500000+500000", ERR_OVERFLOW);
		CHECK_ERR_EXP("-500000+-500000", ERR_OVERFLOW);
		
		// string behavior
		CHECK_STR_EXP("\"A\"+\"B\"", "S\2AB");
		CHECK_STR_EXP("\"\"+\"\"", "S\0");
		
		// string error (yeah this relies on string multiplication)
		// that's the purpose of the extra test here, to check it works first
		CHECK_STR_EXP("\"AAAA\"*63", 
			"S\374"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		);
		CHECK_ERR_STR_EXP("\"AAAA\"*63+\"BBBBB\"", ERR_STRING_TOO_LONG);
	}
	
	// subtraction
	{
		// correct behavior
		CHECK_INT_EXP("1-1",0);
		CHECK_INT_EXP("1234-5678",-4444);
		CHECK_NUM_EXP(".1-.1",0);
		
		// runtime errors
		CHECK_ERR_EXP("500000--500000", ERR_OVERFLOW);
		CHECK_ERR_EXP("-500000-500000", ERR_OVERFLOW);
	}
	
	// multiplication
	{
		// correct
		CHECK_INT_EXP("36*4",144);
		CHECK_INT_EXP("9*-6",-54);
		CHECK_NUM_EXP("10*.1",10*(4096/10));
		
		// runtime errors
		CHECK_ERR_EXP("1024*1024", ERR_OVERFLOW);
		CHECK_ERR_EXP("1024*-1024", ERR_OVERFLOW);
		
		// string
		CHECK_STR_EXP("\"ABC\"*3", "S\11ABCABCABC");
		//(max size is 256 but inline strings don't work for 256 chars)
		CHECK_STR_EXP("\"A\"*255", 
			"S\377"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		);
		// weirder valid cases
		CHECK_STR_EXP("\"ABC\"*0", "S\0"); // zero results in empty string
		CHECK_STR_EXP("\"ABC\"*1.5", "S\3ABC"); // numbers round down to ints
		CHECK_STR_EXP("\"ABC\"*-17", "S\0"); // negatives are treated like zero
		
		// string runtime errors
		CHECK_ERR_STR_EXP("\"A\"*257", ERR_STRING_TOO_LONG);
	}
	
	// division
	{
		// valid
		CHECK_INT_EXP("8/4",2);
		CHECK_NUM_EXP("1/2",4096/2);
		CHECK_NUM_EXP("-36/7",INT_TO_FP(-36)/7);
		
		// errors
		CHECK_ERR_EXP("500000/.1", ERR_OVERFLOW);
		CHECK_ERR_EXP("-500000/.1", ERR_OVERFLOW);
		CHECK_ERR_EXP("1/0", ERR_DIVIDE_BY_ZERO);
	}
	
	// modulo
	{
		// valid
		CHECK_INT_EXP("3%1", 0);
		CHECK_INT_EXP("-5%2", -1);
		CHECK_INT_EXP("5%-2", 1);
		CHECK_NUM_EXP("6%3.6", 9831);
		CHECK_NUM_EXP("-5%3.8", -4916);
		CHECK_NUM_EXP("-6%-2.7", -2458);
		
		// errors
		CHECK_ERR_EXP("1%0", ERR_DIVIDE_BY_ZERO);
	}
	
	// equality
	{
		// valid [1 = true]
		CHECK_INT_EXP("1 == 1", 1);
		CHECK_INT_EXP("1 == 0", 0);
		CHECK_INT_EXP("0 == 0", 1);
		CHECK_INT_EXP("77==77", 1);
		CHECK_INT_EXP("-3==-3", 1);
		
		// string valid
		CHECK_INT_EXP("\"A\"==\"A\"", 1);
		CHECK_INT_EXP("\"B\"==\"A\"", 0);
		CHECK_INT_EXP("\"\"==\"\"", 1);
	}
	
	// inequality
	{
		// valid [1 = true, 0 = false]
		CHECK_INT_EXP("1 != 1", 0);
		CHECK_INT_EXP("1 != 0", 1);
		CHECK_INT_EXP("0 != 0", 0);
		CHECK_INT_EXP("77!= 7", 1);
		CHECK_INT_EXP("-3!= 3", 1);
		
		// string valid
		CHECK_INT_EXP("\"A\"!=\"A\"", 0);
		CHECK_INT_EXP("\"B\"!=\"A\"", 1);
		CHECK_INT_EXP("\"\"!=\"\"", 0);
	}
	
	// less
	{
		// valid
		CHECK_INT_EXP("1 < 2", 1);
		CHECK_INT_EXP("1 < 1", 0);
		CHECK_INT_EXP("1 < 0", 0);
		CHECK_INT_EXP("36 < 72", 1);
		CHECK_INT_EXP("-12 < 1", 1);
		CHECK_INT_EXP("0 < 0", 0);
	}
	
	// less or equal
	{
		// valid
		CHECK_INT_EXP("1 <= 2", 1);
		CHECK_INT_EXP("1 <= 1", 1);
		CHECK_INT_EXP("1 <= 0", 0);
		CHECK_INT_EXP("36 <= 72", 1);
		CHECK_INT_EXP("-12 <= 1", 1);
		CHECK_INT_EXP("0 <= 0", 1);
	}
	
	// greater or equal
	{
		// valid
		CHECK_INT_EXP("1 >= 2", 0);
		CHECK_INT_EXP("1 >= 1", 1);
		CHECK_INT_EXP("1 >= 0", 1);
		CHECK_INT_EXP("36 >= 72", 0);
		CHECK_INT_EXP("-12 >= 1", 0);
		CHECK_INT_EXP("0 >= 0", 1);
	}
	
	// greater
	{
		// valid
		CHECK_INT_EXP("1 > 2", 0);
		CHECK_INT_EXP("1 > 1", 0);
		CHECK_INT_EXP("1 > 0", 1);
		CHECK_INT_EXP("36 > -72", 1);
		CHECK_INT_EXP("12 > 1", 1);
		CHECK_INT_EXP("0 > 0", 0);
	}
	
	// unary ops
	// logical invert
	{
		CHECK_INT_EXP("!1", 0);
		CHECK_INT_EXP("!0.001", 0);
		CHECK_INT_EXP("!0", 1);
		CHECK_INT_EXP("!TRUE", 0);
		CHECK_INT_EXP("!FALSE", 1);
		CHECK_INT_EXP("!CANCEL", 0);
	}
	
	// negation
	{
		CHECK_INT_EXP("- 3", -3);
		CHECK_INT_EXP("- 0", 0);
		CHECK_INT_EXP("-- 3", 3);
	}
	
	// bitwise invert
	{
		CHECK_INT_EXP("NOT 1", -2);
		CHECK_INT_EXP("NOT 0", -1);
		CHECK_INT_EXP("NOT -1", 0);
		CHECK_INT_EXP("NOT 2.6", -3); // discard decimal component
	}
	
	// bitwise AND
	{
		CHECK_INT_EXP("1 AND 1", 1);
		CHECK_INT_EXP("0 AND 1", 0);
		CHECK_INT_EXP("1 AND 0", 0);
		CHECK_INT_EXP("0 AND 0", 0);
		CHECK_INT_EXP("&HA AND &HC", 8);
		CHECK_INT_EXP("123.4 AND 123.4", 123); //removes decimal component
		CHECK_INT_EXP("36.7 AND -1", 36);
	}
	
	// bitwise OR
	{
		CHECK_INT_EXP("1 OR 1", 1);
		CHECK_INT_EXP("0 OR 1", 1);
		CHECK_INT_EXP("1 OR 0", 1);
		CHECK_INT_EXP("0 OR 0", 0);
		CHECK_INT_EXP("&HA OR &HC", 14);
		CHECK_INT_EXP("123.4 OR 123.4", 123); //removes decimal component
		CHECK_INT_EXP("36.7 OR -1", -1);
	}
	
	// bitwise XOR
	{
		CHECK_INT_EXP("1 XOR 1", 0);
		CHECK_INT_EXP("0 XOR 1", 1);
		CHECK_INT_EXP("1 XOR 0", 1);
		CHECK_INT_EXP("0 XOR 0", 0);
		CHECK_INT_EXP("&HA XOR &HC", 6);
		CHECK_INT_EXP("123.4 XOR 123", 0); //removes decimal component
		CHECK_INT_EXP("36.7 XOR -1", (36^0xfffff));
	}
	
	SUCCESS("test_int_operator success");
}
