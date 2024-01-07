#include "test_util.h"

#include "subsystem/resources.h"
#include "system.h"

int test_resources(void){
	// Check character resource indexes are correct for valid strings
	{
		// Note: If you fully specify names, pages will not be checked
		ASSERT(get_chr_index(NULL, "BGF0U") == 0, "[resource] BGF0U");
		ASSERT(get_chr_index(NULL, "BGF1U") == 1, "[resource] BGF1U");
		ASSERT(get_chr_index(NULL, "BGD0U") == 4, "[resource] BGD0U");
		ASSERT(get_chr_index(NULL, "BGD1U") == 5, "[resource] BGD1U");
		ASSERT(get_chr_index(NULL, "BGD2U") == 6, "[resource] BGD2U"); // extra
		ASSERT(get_chr_index(NULL, "BGD3U") == 7, "[resource] BGD2U"); // extra
		ASSERT(get_chr_index(NULL, "BGU0U") == 8, "[resource] BGU0U");
		ASSERT(get_chr_index(NULL, "BGU1U") == 9, "[resource] BGU1U");
		ASSERT(get_chr_index(NULL, "BGU2U") == 10, "[resource] BGU2U");
		ASSERT(get_chr_index(NULL, "BGU3U") == 11, "[resource] BGU3U");
		
		ASSERT(get_chr_index(NULL, "SPU0U") == 12, "[resource] SPU0");
		ASSERT(get_chr_index(NULL, "SPU7U") == 19, "[resource] SPU7");
		
		ASSERT(get_chr_index(NULL, "BGF0L") == 22, "[resource] BGF0L");
		ASSERT(get_chr_index(NULL, "BGF1L") == 23, "[resource] BGF1L");
		ASSERT(get_chr_index(NULL, "BGD0L") == 26, "[resource] BGD0L");
		ASSERT(get_chr_index(NULL, "BGD1L") == 27, "[resource] BGD1L");
		ASSERT(get_chr_index(NULL, "BGD2L") == 28, "[resource] BGD2L"); // extra
		ASSERT(get_chr_index(NULL, "BGD3L") == 29, "[resource] BGD2L"); // extra
		ASSERT(get_chr_index(NULL, "BGU0L") == 30, "[resource] BGD0L");
		ASSERT(get_chr_index(NULL, "BGU1L") == 31, "[resource] BGD1L");
		ASSERT(get_chr_index(NULL, "BGU2L") == 32, "[resource] BGD2L");
		ASSERT(get_chr_index(NULL, "BGU3L") == 33, "[resource] BGD2L");
		
		ASSERT(get_chr_index(NULL, "SPD0L") == 34, "[resource] SPD0");
		ASSERT(get_chr_index(NULL, "SPD3L") == 37, "[resource] SPD3");
	}
	
	{
		// BG=0, SP=0
		struct ptc p = {0};
		ASSERT(get_chr_index(&p, "BGF0") == 0, "[resource] BGF0 U");
		ASSERT(get_chr_index(&p, "BGF1") == 1, "[resource] BGF1 U");
		ASSERT(get_chr_index(&p, "BGD0") == 4, "[resource] BGD0 U");
		ASSERT(get_chr_index(&p, "BGD1") == 5, "[resource] BGD1 U");
		ASSERT(get_chr_index(&p, "BGD2") == 6, "[resource] BGD2 U"); // extra
		ASSERT(get_chr_index(&p, "BGD3") == 7, "[resource] BGD2 U"); // extra
		ASSERT(get_chr_index(&p, "BGU0") == 8, "[resource] BGU0 U");
		ASSERT(get_chr_index(&p, "BGU1") == 9, "[resource] BGU1 U");
		ASSERT(get_chr_index(&p, "BGU2") == 10, "[resource] BGU2 U");
		ASSERT(get_chr_index(&p, "BGU3") == 11, "[resource] BGU3 U");
		
		ASSERT(get_chr_index(&p, "SPU0") == 12, "[resource] SPU0");
		ASSERT(get_chr_index(&p, "SPU7") == 19, "[resource] SPU7");
		ASSERT(get_chr_index(&p, "SPS0") == 20, "[resource] SPS0 U");
		ASSERT(get_chr_index(&p, "SPS1") == 21, "[resource] SPS1 U");
		
		p.background.page = 1;
		
		ASSERT(get_chr_index(&p, "BGF0") == 22, "[resource] BGF0 L");
		ASSERT(get_chr_index(&p, "BGF1") == 23, "[resource] BGF1 L");
		ASSERT(get_chr_index(&p, "BGD0") == 26, "[resource] BGD0 L");
		ASSERT(get_chr_index(&p, "BGD1") == 27, "[resource] BGD1 L");
		ASSERT(get_chr_index(&p, "BGD2") == 28, "[resource] BGD2 L"); // extra
		ASSERT(get_chr_index(&p, "BGD3") == 29, "[resource] BGD2 L"); // extra
		ASSERT(get_chr_index(&p, "BGU0") == 30, "[resource] BGD0 L");
		ASSERT(get_chr_index(&p, "BGU1") == 31, "[resource] BGD1 L");
		ASSERT(get_chr_index(&p, "BGU2") == 32, "[resource] BGD2 L");
		ASSERT(get_chr_index(&p, "BGU3") == 33, "[resource] BGD2 L");
		
		ASSERT(get_chr_index(&p, "SPS0") == 20, "[resource] SPS0 U");
		ASSERT(get_chr_index(&p, "SPS1") == 21, "[resource] SPS1 U");
		
		p.background.page = 0;
		p.sprites.page = 1;
		
		ASSERT(get_chr_index(&p, "SPD0") == 34, "[resource] SPD0");
		ASSERT(get_chr_index(&p, "SPD3") == 37, "[resource] SPD3");
		ASSERT(get_chr_index(&p, "SPS0") == 42, "[resource] SPS0 L");
		ASSERT(get_chr_index(&p, "SPS1") == 43, "[resource] SPS1 L");
		
	}
	SUCCESS("test_resources success");
}
