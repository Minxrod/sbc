#include "common.h"
#include "header.h"
#include "strs.h"
#include "test_util.h"

#include "subsystem/resources.h"
#include "system.h"
#include "extension/compress.h"

#include <stdio.h>
#include <errno.h>

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
	
	// type verification check
	{
		ASSERT(verify_file_type(TEST_SEARCH_PATH"IFELSE.PTC", TYPE_PRG), "PRG detected as PRG");
		DENY(verify_file_type(TEST_SEARCH_PATH"IFELSE.PTC", TYPE_MEM), "PRG not detected as MEM");
		ASSERT(verify_file_type(TEST_SEARCH_PATH"MCHRENC.PTC", TYPE_MEM), "MEM detected as MEM");
		DENY(verify_file_type(TEST_SEARCH_PATH"MCHRENC.PTC", TYPE_PRG), "MEM not detected as PRG");
		DENY(verify_file_type(TEST_SEARCH_PATH"BGF0.PTC", TYPE_MEM), "CHR not detected as MEM");
		DENY(verify_file_type(TEST_SEARCH_PATH"BGF0.PTC", TYPE_PRG), "CHR not detected as PRG");
		ASSERT(verify_file_type(TEST_SEARCH_PATH"BGF0.PTC", TYPE_CHR), "CHR not detected as PRG");

		ASSERT(verify_search_file_type(TEST_SEARCH_PATH, "IFELSE", TYPE_PRG), "PRG found with search path");
		ASSERT(verify_search_file_type(TEST_SEARCH_PATH, "MCHRENC", TYPE_MEM), "MEM found with search path");
		ASSERT(verify_search_file_type(TEST_SEARCH_PATH, "BGF0", TYPE_CHR), "PRG found with search path");
	}

	// validate resource types
	{
#define VERIFY_STR(str)\
do {\
	ASSERT(verify_resource_type(str), "Verify type "str)\
	ASSERT(verify_resource_type(str":"), "Verify type "str":")\
} while (0)
#define DENY_STR(str)\
do {\
	DENY(verify_resource_type(str), "Deny type "str)\
	DENY(verify_resource_type(str":"), "Deny type "str":")\
} while (0)
// BGF variations
		VERIFY_STR("BGF");
		VERIFY_STR("BGF0");
		DENY_STR("BGF1"); // not allowed in PTC
		DENY_STR("BGF2");
		DENY_STR("BGF3");
		VERIFY_STR("BGF0L");
		VERIFY_STR("BGF0U");
		// BGU variations
		VERIFY_STR("BGU");
		VERIFY_STR("BGU0");
		VERIFY_STR("BGU1");
		VERIFY_STR("BGU2");
		VERIFY_STR("BGU3");
		VERIFY_STR("BGU0L");
		VERIFY_STR("BGU1L");
		VERIFY_STR("BGU2L");
		VERIFY_STR("BGU3L");
		VERIFY_STR("BGU0U");
		VERIFY_STR("BGU1U");
		VERIFY_STR("BGU2U");
		VERIFY_STR("BGU3U");
		// BGD variations
		VERIFY_STR("BGD");
		VERIFY_STR("BGD0");
		VERIFY_STR("BGD1");
		DENY_STR("BGD2");
		DENY_STR("BGD3");
		VERIFY_STR("BGD0L");
		VERIFY_STR("BGD1L");
		DENY_STR("BGD2L");
		DENY_STR("BGD3L");
		VERIFY_STR("BGD0U");
		VERIFY_STR("BGD1U");
		DENY_STR("BGD2U");
		DENY_STR("BGD3U");
		// SPU variations
		VERIFY_STR("SPU");
		VERIFY_STR("SPU0");
		VERIFY_STR("SPU1");
		VERIFY_STR("SPU2");
		VERIFY_STR("SPU3");
		VERIFY_STR("SPU4");
		VERIFY_STR("SPU5");
		VERIFY_STR("SPU6");
		VERIFY_STR("SPU7");
		VERIFY_STR("SPU0U");
		VERIFY_STR("SPU1U");
		VERIFY_STR("SPU2U");
		VERIFY_STR("SPU3U");
		VERIFY_STR("SPU4U");
		VERIFY_STR("SPU5U");
		VERIFY_STR("SPU6U");
		VERIFY_STR("SPU7U");
		// SPS variations
		VERIFY_STR("SPS");
		VERIFY_STR("SPS0");
		VERIFY_STR("SPS1");
		VERIFY_STR("SPS0L");
		VERIFY_STR("SPS1L");
		VERIFY_STR("SPS0U");
		VERIFY_STR("SPS1U");
		// SPD variations
		VERIFY_STR("SPD");
		VERIFY_STR("SPD0");
		VERIFY_STR("SPD1");
		VERIFY_STR("SPD2");
		VERIFY_STR("SPD3");
		VERIFY_STR("SPD0L");
		VERIFY_STR("SPD1L");
		VERIFY_STR("SPD2L");
		VERIFY_STR("SPD3L");
		// COL variations
		VERIFY_STR("COL");
		VERIFY_STR("COL0");
		VERIFY_STR("COL1");
		VERIFY_STR("COL2");
		VERIFY_STR("COL0L");
		VERIFY_STR("COL1L");
		VERIFY_STR("COL2L");
		VERIFY_STR("COL0U");
		VERIFY_STR("COL1U");
		VERIFY_STR("COL2U");
		// MEM variants
		VERIFY_STR("MEM");
		VERIFY_STR("MEM9"); // basically anything is valid
		VERIFY_STR("MEM!!");
		// PRG variants
		VERIFY_STR("PRG");
		VERIFY_STR("PRG6"); // extra chars get ignored in PTC
		VERIFY_STR("PRG??");
		// GRP
		VERIFY_STR("GRP");
		VERIFY_STR("GRP0");
		VERIFY_STR("GRP1");
		VERIFY_STR("GRP2");
		VERIFY_STR("GRP3");
		VERIFY_STR("GRP0U"); // last char is ignored, one again
		VERIFY_STR("GRP1L");
		VERIFY_STR("GRP2P");
		VERIFY_STR("GRP3F");
		// SCR
		VERIFY_STR("SCU");
		VERIFY_STR("SCU0");
		VERIFY_STR("SCU1");
		VERIFY_STR("SCU0L");
		VERIFY_STR("SCU1L");
		VERIFY_STR("SCU0U");
		VERIFY_STR("SCU1U");
	}

	// check valid resource names
	{
		ASSERT(verify_resource_name("BGF0:FONT"), "Check valid name");
		ASSERT(verify_resource_name("PRG:MYPROG"), "Check valid name");
		ASSERT(verify_resource_name("SPS0L:TOUCHSPR"), "Check valid name");
		ASSERT(verify_resource_name("GAME4"), "Check valid name");
		DENY(verify_resource_name("##GAME##"), "Check invalid name");
		DENY(verify_resource_name("PRG:?"), "Check invalid name");
		DENY(verify_resource_name(":8"), "Check invalid name");
		DENY(verify_resource_name(":"), "Check invalid name");
		DENY(verify_resource_name("MEM:"), "Check invalid name");
		DENY(verify_resource_name("MEM:ABCDEFGHI"), "Check invalid name");
		DENY(verify_resource_name("ABCDEFGHI"), "Check invalid name");
	}

	// test string wrapper
	{
		ASSERT(verify_resource_type_str("S\3BGF"), "[verify] Valid type string");
		DENY(verify_resource_type_str("S\6AAAAAA"), "[verify] Invalid type string");
		DENY(verify_resource_type_str("S\4BGF4"), "[verify] Invalid type string");
	}

	// Test file path creation
	{
		// Simple case
		char path[MAX_FILEPATH_LENGTH+1];
		bool res = create_path(path, "test/path/", "filename", ".txt");
		ASSERT(res, "[create_path] Path creation successful")
		ASSERT(!strcmp(path, "test/path/filename.txt"), "[create_path] Path is correct");

		// No base
		res = create_path(path, "", "filename", ".txt");
		ASSERT(res, "[create_path] Path creation successful")
		ASSERT(!strcmp(path, "filename.txt"), "[create_path] Path is correct");

		// No extension
		res = create_path(path, "test/", "filename", "");
		ASSERT(res, "[create_path] Path creation successful")
		ASSERT(!strcmp(path, "test/filename"), "[create_path] Path is correct");

		// Maximum length
		res = create_path(
			path,
			"test/"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789/", // 246 chars
			"long", // 250
			".test" // 255 (MAX_FILEPATH_LENGTH)
		);
		ASSERT(res, "[create_path] Path creation successful")
		ASSERT(!strcmp(
			path,
			"test/"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"/long.test"
		), "[create_path] Path is correct");

		// Error on larger length
		res = create_path(
			path,
			"test/"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789/", // 246 chars
			"longer", // 252
			".tst" // 256 (larger than MAX_FILEPATH_LENGTH)
		);
		DENY(res, "[create_path] Error on too-long path")
	}

	// Test path creation from SBC string
	{
		// Simple case
		char path[MAX_FILEPATH_LENGTH+1];
		bool res = create_path_from_str(path, "test/path/", "S\10filename", ".txt");
		ASSERT(res, "[create_path] Path creation successful")
		ASSERT(!strcmp(path, "test/path/filename.txt"), "[create_path] Path is correct");

		// Error on larger length
		res = create_path_from_str(
			path,
			"test/"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789"
			"012345678901234567890123456789012345678901234567890123456789/", // 246 chars
			"S\6longer", // 252
			".tst" // 256 (larger than MAX_FILEPATH_LENGTH)
		);
		DENY(res, "[create_path] Error on too-long path")
	}
	
	SUCCESS("test_resources success");
}
