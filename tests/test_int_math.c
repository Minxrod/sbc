#include "test_util.h"

#include "error.h"

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
		
		CHECK_VAR_INT("A", 0)
		CHECK_VAR_NUM("B", -INT_TO_FP(1))
		CHECK_VAR_INT("C", 1)
		CHECK_VAR_NUM("D", 9831) //2.4
		CHECK_VAR_NUM("E", -4916) //-1.2
		CHECK_VAR_NUM("F", -2458) //-0.6
		
		free_code(p);
	}
	
	SUCCESS();
}
