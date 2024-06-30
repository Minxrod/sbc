#include "common.h"
#include "test_util.h"

#include "error.h"
#include "system.h"

int test_int_math(){
	// Overflow on simple number
	{
		ASSERT(check_code_error("?567855788\r", ERR_OVERFLOW), "[number] Overflow on large number");
		ASSERT(check_code_error("?-567855788\r", ERR_OVERFLOW), "[number] Overflow on large number");
	}
	
	// ABS works as expected
	{
		struct ptc* p = run_code("A=ABS(36)\rB=ABS(0)\rC=ABS(-36)\rD=ABS(-524287.999994)\rE=ABS(524287.999994)\rF=&H80000\rG=ABS(F)\r");
		
		CHECK_VAR_NUM("A", INT_TO_FP(36));
		CHECK_VAR_NUM("B", 0);
		CHECK_VAR_NUM("C", INT_TO_FP(36));
		CHECK_VAR_NUM("D", 0x7fffffff);
		CHECK_VAR_NUM("E", 0x7fffffff);
		CHECK_VAR_NUM("F", -2147483648);
		CHECK_VAR_NUM("G", -2147483648);
		
		free_code(p);
	}
	
	// ABS Error checks
	{
		DENY(check_code_error("?ABS(\"\")\r", ERR_NONE), "[abs] Error on string value");
		DENY(check_code_error("?ABS()\r", ERR_NONE), "[abs] Error on missing argument");
		DENY(check_code_error("?ABS(2,3)\r", ERR_NONE), "[abs] Error on extra argument");
	}
	
	// SGN works as expected
	{
		struct ptc* p = run_code("A=SGN(0)\rB=SGN(-1)\rC=SGN(1)\rD=SGN(524287.999994)\rE=SGN(&H80000)\r");
		
		CHECK_VAR_NUM("A", 0);
		CHECK_VAR_NUM("B", -INT_TO_FP(1));
		CHECK_VAR_NUM("C", INT_TO_FP(1));
		CHECK_VAR_NUM("D", INT_TO_FP(1));
		CHECK_VAR_NUM("E", -INT_TO_FP(1));
		
		free_code(p);
	}
	
	// SGN Error checks
	{
		DENY(check_code_error("?SGN(\"\")\r", ERR_NONE), "[sgn] Error on string value");
		DENY(check_code_error("?SGN()\r", ERR_NONE), "[sgn] Error on missing argument");
		DENY(check_code_error("?SGN(2,3)\r", ERR_NONE), "[sgn] Error on extra argument");
	}

	// POW normal function
	{
		CHECK_INT_EXP("POW(0,0)",1);
		CHECK_INT_EXP("POW(2,4)",16);
		CHECK_INT_EXP("POW(-3,4)",81);
		// there's some float rounding nonsense here, but it seems to work
		CHECK_NUM_EXP("POW(524287.984,1)", INT_TO_FP(524287)+3968);
	}

	// POW errors
	{
		CHECK_ERR_EXP("POW(999,999)", ERR_ILLEGAL_FUNCTION_CALL);
		CHECK_ERR_EXP("POW(999,9)", ERR_OVERFLOW);
		CHECK_ERR_EXP("POW(524287.985,1)", ERR_OVERFLOW);
	}
	
	SUCCESS("test_int_math success");
}
