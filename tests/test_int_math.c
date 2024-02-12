#include "test_util.h"

#include "error.h"
#include "system.h"

int test_int_math(){
	// Check for overflow
	{
		char* code = "A=500000+500000\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_OVERFLOW, "[math] Error on addition overflow");
		
		free_code(p);
	}
	
	// Check for overflow
	{
		char* code = "A=500000--500000\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_OVERFLOW, "[math] Error on subtraction overflow");
		
		free_code(p);
	}
	
	// Check for underflow
	{
		char* code = "A=-500000-500000\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_OVERFLOW, "[math] Error on subtraction underflow");
		
		free_code(p);
	}
	
	// Check for overflow
	{
		char* code = "A=50000*50000\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_OVERFLOW, "[math] Error on multiplication overflow");
		
		free_code(p);
	}
	
	// Check for division by zero error
	{
		char* code = "A=1/0\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_DIVIDE_BY_ZERO, "[math] Error on divide by zero");
		
		free_code(p);
	}
	
	// Check for overflow
	{
		char* code = "A=500000/.001\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_OVERFLOW, "[math] Error on division overflow");
		
		free_code(p);
	}
	
	// Check for division by zero
	{
		char* code = "A=3 % 0\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(p->exec.error == ERR_DIVIDE_BY_ZERO, "[math] Modulo by zero");
		
		free_code(p);
	}
	
	// Check modulo behavior
	{
		char* code = "A=3%1\rB=-5%2\rC=5%-2\rD=6%3.6\rE=-5%3.8\rF=-6%-2.7\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("A", 0);
		CHECK_VAR_NUM("B", -INT_TO_FP(1));
		CHECK_VAR_INT("C", 1);
		CHECK_VAR_NUM("D", 9831); //2.4
		CHECK_VAR_NUM("E", -4916); //-1.2
		CHECK_VAR_NUM("F", -2458); //-0.6
		
		free_code(p);
	}
	
	// Negation has self inverse
	{
#ifdef TEST_FULL
		struct ptc* p = run_code("E=1/4096\rFOR I=-524287 TO 524287 STEP E\rIF I!=--I THEN S=S+1\rNEXT\r");
		
		CHECK_VAR_NUM("S", 0);
		
		free_code(p);
#endif
	}
	
	// Negation errors
	{
		DENY(check_code_error("?-\"\"\r", ERR_NONE), "[negate] Error on string value");
		DENY(check_code_error("?-\r", ERR_NONE), "[negate] Error on no value");
	}
	
	// Subtraction errors
	{
		// Parsed as "" followed by -0 in PRINT. Would be incorrect in most locations due to missing comma, TODO:ERR:LOW determine whether this is important enough to fix
//		DENY(check_code_error("?\"\"-0\r", ERR_NONE), "[negate] Error on string value");
		DENY(check_code_error("?0-\"\"\r", ERR_NONE), "[negate] Error on string value");
		DENY(check_code_error("?0-\r", ERR_NONE), "[negate] Error on incomplete expression");
		DENY(check_code_error("?-500000-500000\r", ERR_NONE), "[negate] Error on underflow");
		DENY(check_code_error("?500000--500000\r", ERR_NONE), "[negate] Error on overflow");
	}
	
	// Subtraction with self is always zero
	{
#ifdef TEST_FULL
		struct ptc* p = run_code("E=1/4096\rFOR I=-524287 TO 524287 STEP E\rIF I-I!=0 THEN S=S+1\rNEXT\r");
		
		CHECK_VAR_NUM("S", 0);
		
		free_code(p);
#endif
	}
	
	// Logical invert errors
	{
		DENY(check_code_error("?!\r", ERR_NONE), "[negate] Error on no value");
		DENY(check_code_error("?!\"\"\r", ERR_NONE), "[negate] Error on string value");
	}
	
	// Logical invert is zero unless argument is zero
	{
#ifdef TEST_FULL
		struct ptc* p = run_code("X=!0\rE=1/4096\rFOR I=-524287 TO 524287 STEP E\rS=S+!I\rNEXT\r");
		
		CHECK_VAR_INT("S", 1); // only one instance of !I becoming 1
		CHECK_VAR_NUM("X", 1); // confirm correct instance
		
		free_code(p);
#endif
	}
	
	// Overflow on simple number
	{	
		// TODO:ERR:LOW
//		ASSERT(check_code_error("?567855788", ERR_OVERFLOW), "[number] Overflow on large number");
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
	
	// Regular AND operation
	{
		struct ptc* p = run_code("A=0 AND 0\rB=2 AND 1\rC=&H7FF AND &H030\rD=-27 AND -1\r");
		
		CHECK_VAR_NUM("A", 0);
		CHECK_VAR_NUM("B", 0);
		CHECK_VAR_NUM("C", INT_TO_FP(0x030));
		CHECK_VAR_NUM("D", -INT_TO_FP(27));
		
		free_code(p);
	}
	
	// AND operator errors
	{
		DENY(check_code_error("?\"\" AND 0\r", ERR_NONE), "[and] Error on string value 1");
		DENY(check_code_error("?0 AND \"\"\r", ERR_NONE), "[and] Error on string value 2");
		DENY(check_code_error("?\"\" AND \"\"\r", ERR_NONE), "[and] Error on string value 1+2");
		DENY(check_code_error("?AND 0\r", ERR_NONE), "[sgn] Error on missing argument 1");
		DENY(check_code_error("?0 AND\r", ERR_NONE), "[sgn] Error on missing argument 2");
		DENY(check_code_error("?AND\r", ERR_NONE), "[sgn] Error on missing argument 1+2");
	}
	
	SUCCESS("test_int_math success");
}
