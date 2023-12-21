#include "test_util.h"

#include "mathptc.h"

int test_math(){
	// Test sine function
	{
		fixp angle[] = {
			0,42,480,773,814,1225,1734,2118,2677,2823,3103,3217,3774,4050,4543,5120,5698,5965,
			84<<12,210<<12,349<<12,586<<12,813<<12,873<<12,
			130<<12,446<<12,748<<12,
			-64,-258,-679,-1275,-1314,-1832,-2392,-2741,-3074,-3545,-3899,-4374,-4617,-4806,-5013,-5417,-5649,
			524287<<FIXPOINT,
		};
		fixp sine[] = {
			0,41,478,768,808,1206,1682,2024,2490,2604,2814,2896,3262,3421,3667,3887,4030,4069,
			3003,1915,-1145,4078,2551,-1454,
			-3810,-435,1214,
			-64,-258,-676,-1255,-1292,-1772,-2259,-2541,-2794,-3119,-3337,-3589,-3700,-3777,-3852,-3971,-4022,
			-3027
		};
		
		for (u32 i = 0; i < sizeof angle / sizeof angle[0]; ++i){
			fixp sin_result = func_sin_internal(angle[i]);
			iprintf("theta=%d expected=%d sine=%d\n",angle[i], sine[i], sin_result);
			CHECK(sine[i] == sin_result, "[math] Sine function");
		}
	}
	
#ifdef TEST_FULL
	// Test sine function for positive fixedpoint values in [0,262143]
	{
		const char* test_data = "tests/data/sin_data.bin";
		FILE* f = fopen(test_data,"rb");
		if (!f){
			iprintf("File %s load failed!\n", test_data);
			abort();
		}
		int expected;
		//TODO:CODE:MED this only works on little-endian devices
		
		// Iterate over all fixedpoint values from 0 to 262143/4096
		for (fixp i = 0; i <= 262143; i += 1){
			fread(&expected, sizeof(int), 1, f);
			if (ferror(f)){
				iprintf("Failed to read file at %d\n", i);
				abort();
			}
			
			fixp s = func_sin_internal(i);
			
			iprintf("theta=%d expected=%d sine=%d ", i, expected, s);
			CHECK(s == expected, "[math] Sine function");
			iprintf("\n");
		}
		fclose(f);
	}
#endif // TEST_FULL
	
	// Test log function for selected values
	{
		// Logs from 1 to 64
		// First zero exists to pad indexes
		fixp logs[] = {
			0,0,2839,4500,5678,
			6592,7339,7970,8517,8999,9431,9821,10179,10507,10810,11093,11357,11605,
			11839,12061,12271,12471,12661,12843,13018,13185,13346,13500,13649,13793,
			13932,14066,14196,14322,14444,14563,14678,14791,14900,15006,15110,15211,
			15310,15406,15500,15592,15682,15770,15857,15941,16024,16104,16185,16262,
			16339,16414,16488,16560,16632,16701,16771,16838,16905,16970,17035
		};
		
		for (u32 i = 1; i < sizeof logs / sizeof logs[0]; ++i){
			fixp log_result = func_log_internal(INT_TO_FP(i));
			iprintf("input=%d expected=%d log=%d",(int)i, logs[i], log_result);
			CHECK(logs[i] == log_result, "[math] Log function");
			iprintf("\n");
		}
	}
	
	// Test cos function for selected values
	{
		// Cosine from -63 to 0
		// These values are all symmetric
		fixp cosine[] = {
			4038,2758,-1058,-3902,-3159,488,3685,3494,90,-3397,-3762,-668,
			3039,3952,1231,-2623,-4065,-1771,2151,4095,2273,-1639,-4045,
			-2732,1092,3911,3135,-525,-3702,-3476,-55,3416,3746,631,-3065,
			-3943,-1197,2649,4059,1737,-2183,-4096,-2244,1671,4049,2704,
			-1128,-3923,-3112,560,3716,3456,18,-3437,-3732,-596,3087,3932,1161,
			-2678,-4056,-1705,2213,
			4096,
		};
		
		for (u32 i = 0; i < sizeof cosine / sizeof cosine[0]; ++i){
			fixp cos_result = func_cos_internal(INT_TO_FP(i-63));
			iprintf("input=%d expected=%d cos=%d",(int)i, cosine[i], cos_result);
			CHECK(cosine[i] == cos_result, "[math] Cosine function");
			iprintf("\n");
			// via symmetry
			cos_result = func_cos_internal(INT_TO_FP(i));
			iprintf("input=%d expected=%d cos=%d",(int)i, cosine[63-i], cos_result);
			CHECK(cosine[63-i] == cos_result, "[math] Cosine function");
			iprintf("\n");
		}
	}
	
	SUCCESS("test_math success");
}
