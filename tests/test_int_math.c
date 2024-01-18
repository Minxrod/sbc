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
		
		CHECK_VAR_NUM("S", 1); // only one instance of !I becoming 1
		CHECK_VAR_NUM("X", 1); // confirm correct instance
		
		free_code(p);
#endif
	}
	
	// Overflow on simple number
	{	
		// TODO:ERR:LOW
//		ASSERT(check_code_error("?567855788", ERR_OVERFLOW), "[number] Overflow on large number");
	}
	
	SUCCESS();
}
