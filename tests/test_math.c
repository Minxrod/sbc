#include "test_math.h"

#include "test_util.h"

#include "mathptc.h"

int test_math(){
	// Test sine function
	{
		fixp angle[] = {
			0,42,480,773,814,1225,1734,2118,2677,2823,3103,3217,3774,4050,4543,5120,5698,5965,
			84<<12,210<<12,349<<12,586<<12,813<<12,873<<12,
			130<<12,446<<12,748<<12,
			-64,-258,-679,-1275,-1314,-1832,-2392,-2741,-3074,-3545,-3899,-4374,-4617,-4806,-5013,-5417,-5649
		};
		fixp sine[] = {
			0,41,478,768,808,1206,1682,2024,2490,2604,2814,2896,3262,3421,3667,3887,4030,4069,
			3003,1915,-1145,4078,2551,-1454,
			-3810,-435,1214,
			-64,-258,-676,-1255,-1292,-1772,-2259,-2541,-2794,-3119,-3337,-3589,-3700,-3777,-3852,-3971,-4022
		};
		
		for (u32 i = 0; i < sizeof angle / sizeof angle[0]; ++i){
			fixp sin_result = func_sin_internal(angle[i]);
			iprintf("theta=%d expected=%d sine=%d\n",angle[i], sine[i], sin_result);
			ASSERT(sine[i] == sin_result, "[math] Sine function");
		}
	}
	
	SUCCESS("test_math success");
}
