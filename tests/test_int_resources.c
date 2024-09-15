#include "error.h"
#include "resources.h"
#include "test_util.h"
#include "tokens.h"

#include "system.h"

#include <errno.h>

int test_int_resources(void){

	// get_resource tests
	{
		struct ptc* p = init_system(0, 0, 0, true);
#define VERIFY_CHR_RESOURCE(str) ASSERT(get_resource_ptr(p, str) == p->res.chr[get_chr_index(p, str)], "[get_resource] Check resource pointers are correct: " str);

		VERIFY_CHR_RESOURCE("BGF0");
		VERIFY_CHR_RESOURCE("BGF0L");
		VERIFY_CHR_RESOURCE("BGF0U");
		VERIFY_CHR_RESOURCE("SPS0");
		VERIFY_CHR_RESOURCE("SPS0L");
		VERIFY_CHR_RESOURCE("SPS0U");
		VERIFY_CHR_RESOURCE("SPD0");
		VERIFY_CHR_RESOURCE("SPK0");
		VERIFY_CHR_RESOURCE("SPD0L");
		VERIFY_CHR_RESOURCE("SPK0L");
		VERIFY_CHR_RESOURCE("SPU0");
		VERIFY_CHR_RESOURCE("SPU7");

		ASSERT(get_resource_ptr(p, "COL0") == p->res.col[0], "[get_resource] Check resource pointers are correct: COL0");
		ASSERT(get_resource_ptr(p, "COL1") == p->res.col[1], "[get_resource] Check resource pointers are correct: COL1");
		ASSERT(get_resource_ptr(p, "COL2") == p->res.col[2], "[get_resource] Check resource pointers are correct: COL2");
		ASSERT(get_resource_ptr(p, "COL0U") == p->res.col[0], "[get_resource] Check resource pointers are correct: COL0");
		ASSERT(get_resource_ptr(p, "COL1U") == p->res.col[1], "[get_resource] Check resource pointers are correct: COL1");
		ASSERT(get_resource_ptr(p, "COL2U") == p->res.col[2], "[get_resource] Check resource pointers are correct: COL2");
		ASSERT(get_resource_ptr(p, "COL0L") == p->res.col[3], "[get_resource] Check resource pointers are correct: COL3");
		ASSERT(get_resource_ptr(p, "COL1L") == p->res.col[4], "[get_resource] Check resource pointers are correct: COL4");
		ASSERT(get_resource_ptr(p, "COL2L") == p->res.col[5], "[get_resource] Check resource pointers are correct: COL5");

		ASSERT(get_resource_ptr(p, "MEM") == p->res.mem.data, "[get_resource] Check resource pointers are correct: MEM");
		ASSERT(get_resource_ptr(p, "PRG") == p->exec.prg.data, "[get_resource] Check resource pointers are correct: PRG");

		ASSERT(get_resource_ptr(p, "SCU0U") == p->res.scr[2], "[get_resource] Check resource pointers are correct: SCU0U");
		ASSERT(get_resource_ptr(p, "SCU0L") == p->res.scr[6], "[get_resource] Check resource pointers are correct: SCU0L");
		ASSERT(get_resource_ptr(p, "SCU1") == p->res.scr[3], "[get_resource] Check resource pointers are correct: SCU1");

		ASSERT(get_resource_ptr(p, "GRP0") == p->res.grp[0], "[get_resource] Check resource pointers are correct: GRP0");
		ASSERT(get_resource_ptr(p, "GRP") == p->res.grp[0], "[get_resource] Check resource pointers are correct: GRP");
		ASSERT(get_resource_ptr(p, "GRP2") == p->res.grp[2], "[get_resource] Check resource pointers are correct: GRP2");
		ASSERT(get_resource_ptr(p, "GRP3") == p->res.grp[3], "[get_resource] Check resource pointers are correct: GRP3");

		free_system(p);
	}

	// Test MEM resource loading
	{
		struct ptc* p = run_code("LOAD\"MEM:MCHRENC\",0\r");
		u8 str[256];
		for (int i = 0; i < 256; ++i){
			str[i] = i;
		}

		struct string expected = {
			STRING_CHAR, .len=256, 1, {.s = str}
		};

		ASSERT(str_comp(&p->res.mem_str, &expected), "[mem] Loaded MEM$ correctly");

		free_code(p);
	}

	// Test regular MEM$ assignment
	{
		struct ptc* p = run_code("MEM$=\"ABCD\"\r");

		ASSERT(str_comp(&p->res.mem_str, "S\4ABCD"), "[mem] Wrote MEM$ correctly");

		free_code(p);
	}

	// MEM$ reading
	{
		struct ptc* p = run_code("MEM$=\"ABCD\"\rA$=MEM$\r");

		ASSERT(str_comp(&p->res.mem_str, "S\4ABCD"), "[mem] Wrote MEM$ correctly");
		CHECK_VAR_STR("A","S\4ABCD");

		free_code(p);
	}

	// MEM$ saved past CLEAR
	{
		struct ptc* p = run_code("MEM$=\"ABCD\"\rCLEAR\r");

		ASSERT(str_comp(&p->res.mem_str, "S\4ABCD"), "[mem] MEM$ is not deleted");

		free_code(p);
	}

	// Test CHR resource loading
	{
		struct ptc* p = run_code("LOAD\"BGF0:BGF0\",0\r");
		u8 expected[] = {
			0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			0xff,0,0,0xf, 0xf,0xf0,0,0xff, 0xf0,0xff,0xf,0xf0, 0,0xff,0xf,0xf0
		};

		ASSERT(memcmp(expected, p->res.chr[0], sizeof(expected)), "[load] Loaded CHR correctly");
		ASSERT(p->res.result == 1, "[load] RESULT set correctly");

		free_code(p);
	}

	// Test CHR resource load failure (file not found)
	{
		struct ptc* p = run_code("LOAD\"BGF0:NOTEXIST\",0\r");

		ASSERT(p->res.result == 0, "[load] RESULT is zero after failed load");

		free_code(p);
	}

	// Test LOAD of program
	// Note: Strictly, this shouldn't be allowed in a PTC program, since it would overwrite the running program.
	// However, SBC compiles the program first, and as such this is safe to do.
	{
		struct ptc* p = run_code("LOAD\"PRG:IFELSE\",0\r");

		ASSERT(p->exec.error == ERR_NONE, "[load] Loaded with no error");
		ASSERT(p->res.result == 1, "[load] RESULT is one after load");

		free_code(p);
	}

	// Test LOAD with no resource type + execution
	{
		struct ptc* p = run_code("LOAD\"IFELSE\",0\r");

		ASSERT(p->exec.error == ERR_NONE, "[load] Loaded with no error");
		ASSERT(p->res.result == 1, "[load] RESULT is one after load");

		// try to run loaded program to check that it worked
		token_and_run(p, &p->exec.prg, &p->exec.code, 0);

		CHECK_VAR_INT("TY",13); // indicates program executed correctly

		free_code(p);
	}

	// Simple test of CHR* commands (CHRSET, CHRREAD, simple CHRINIT)
	{
		struct ptc* p = run_code(
			"CHRSET \"BGF0\",0,\"42\"*32\r"
			"CHRREAD (\"BGF0\",0),A$\r"
			"CHRINIT \"BGF0\"\r"
			"CHRREAD (\"BGF0\",0),B$\r"
		);

		CHECK_VAR_STR("A","S\1004242424242424242424242424242424242424242424242424242424242424242");
		CHECK_VAR_STR("B","S\1000000000000000000000000000000000000000000000000000000000000000000");

		free_code(p);
	}

	// Simple test for COL* comands (COLSET, COLREAD, simple COLINIT)
	{
		struct ptc* p = run_code(
			"COLSET \"BG\",0,\"080808\"\r"
			"COLSET \"SP\",0,\"101010\"\r"
			"COLREAD(\"BG\",0),BR,BG,BB\r"
			"COLREAD(\"SP\",0),SR,SG,SB\r"
			"COLINIT \"BG\"\r"
			"COLREAD(\"BG\",0),BR2,BG2,BB2\r"
			"COLREAD(\"SP\",0),SR2,SG2,SB2\r"
		);

		CHECK_VAR_INT("BG",8);
		CHECK_VAR_INT("BR",8);
		CHECK_VAR_INT("BB",8);
		CHECK_VAR_INT("SR",16);
		CHECK_VAR_INT("SG",16);
		CHECK_VAR_INT("SB",16);
		CHECK_VAR_INT("BG2",0);
		CHECK_VAR_INT("BR2",0);
		CHECK_VAR_INT("BB2",0);
		CHECK_VAR_INT("SR2",16);
		CHECK_VAR_INT("SG2",16);
		CHECK_VAR_INT("SB2",16);

		free_code(p);
	}

	// Test for precision of COLSET/COLREAD commands
	{
		struct ptc* p = run_code(
			"COLSET \"BG\",0,\"050507\r"
			"COLREAD(\"BG\",0),R,G,B\r"
			"COLSET \"BG\",1,\"08090A\r"
			"COLREAD(\"BG\",1),X,Y,Z\r"
			"COLSET \"BG\",1,\"808080\r"
			"COLREAD(\"BG\",1),U,V,W\r"
			"COLSET \"BG\",1,\"FFFFFF\r"
			"COLREAD(\"BG\",1),I,J,K\r"
		);

		CHECK_VAR_INT("R",0);
		CHECK_VAR_INT("G",4);
		CHECK_VAR_INT("B",0);
		CHECK_VAR_INT("X",8);
		CHECK_VAR_INT("Y",8);
		CHECK_VAR_INT("Z",8);
		CHECK_VAR_INT("U",0x84);
		CHECK_VAR_INT("V",0x82);
		CHECK_VAR_INT("W",0x84);
		CHECK_VAR_INT("I",0xff);
		CHECK_VAR_INT("J",0xff);
		CHECK_VAR_INT("K",0xff);

		free_code(p);
	}

	// Test MEM$ saving
	{
		// ensure file does not exist
		const char* test_file = "tests/data/MSAVTST.PTC";
		if (remove(test_file) && errno != ENOENT){
			iprintf("Failed to remove previous test file\n");
			abort(); // note: ENOENT means file did not exist, which is what we want anyways
		}

		struct ptc* p = run_code("MEM$=\"ABCDEFG\"\rSAVE\"MEM:MSAVTST\"\r");
		ASSERT(p->res.result == 1, "[save] RESULT is one after correct save");
		free_code(p);

		// check file validity independent of interpreter
		FILE* f = fopen(test_file, "rb");
		ASSERT(f, "[save] Saved file opens");
		u8 file_content[12+MEM_SIZE];
		ASSERT(sizeof(file_content) == fread(file_content, sizeof(char), sizeof(file_content), f), "[save] Saved file expected size");
		for (int i = 0; i < 7; ++i){
//			iprintf("Checking: %c\n", 'A' + i);
			ASSERT((((u16*)(file_content + 12))[i] == to_wide('A' + i)), "[save] MEM contents as expected in saved file");
		}
		ASSERT(file_content[12+512] == 7, "[save] MEM length saved correctly");

		// remove file as cleanup
		if (remove(test_file) && errno != ENOENT){
			iprintf("Failed to remove previous test file\n");
			abort(); // note: ENOENT means file did not exist, which is what we want anyways
		}
	}

	// Simple APPEND test
	{
		struct ptc* p = run_code("LOAD\"IFELSE\",0\rAPPEND\"IFELSE\"\r");

		ASSERT(p->res.result == 1, "[append] RESULT is one (successful APPEND)");

		int append_dest = p->exec.prg.size / 2;
		for (int i = 0; i < append_dest; ++i){
			iprintf("%d=%d?\n", p->exec.prg.data[i], p->exec.prg.data[i+append_dest]);
			ASSERT(p->exec.prg.data[i] == p->exec.prg.data[i+append_dest], "[append] Data matches");
		}

		free_code(p);
	}

	// APPEND on nonexistent file test
	{
		struct ptc* p = run_code("APPEND\"IFELSE2\"\r");

		ASSERT(p->res.result == 0, "[append] RESULT is one (successful APPEND)");

		free_code(p);
	}

	// Package test
	{
		struct ptc* p = run_code("EXEC\"PACKTEST\r");

		char zero[] =
		"\x00\x00\x00\x00"
		"\x00\x00\x00\x00"
		"\x00\x00\x00\x00"
		"\xf0\xff\xf0\xff"
		"\xf0\xf0\xf0\xf0"
		"\xf0\xf0\xf0\xf0"
		"\xf0\xff\xf0\xff"
		"\x00\x00\x00\x00";
		ASSERT(!memcmp(p->res.chr[12], zero, CHR_UNIT_SIZE), "[package] Something loaded (nonzero first CHR)");
		CHECK_VAR_STR("A","S\0011");

		free_code(p);
	}

	SUCCESS("test_int_resources success");
}
