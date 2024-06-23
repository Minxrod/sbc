#include "test_util.h"

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
		ASSERT(get_resource_ptr(p, "PRG") == &p->exec.prg, "[get_resource] Check resource pointers are correct: PRG")

		ASSERT(get_resource_ptr(p, "SCU0U") == p->res.scr[2], "[get_resource] Check resource pointers are correct: SCU0U")
		ASSERT(get_resource_ptr(p, "SCU0L") == p->res.scr[6], "[get_resource] Check resource pointers are correct: SCU0L")
		ASSERT(get_resource_ptr(p, "SCU1") == p->res.scr[3], "[get_resource] Check resource pointers are correct: SCU1")

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

		ASSERT(memcmp(expected, p->res.chr[0], sizeof(expected)/sizeof(expected[0])), "[load] Loaded CHR correctly");
		ASSERT(p->res.result == 1, "[load] RESULT set correctly");

		free_code(p);
	}

	// Test CHR resource load failure (file not found)
	{
		struct ptc* p = run_code("LOAD\"BGF0:NOTEXIST\",0\r");

		ASSERT(p->res.result == 0, "[load] RESULT is zero after failed load");

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


	SUCCESS("test_int_resources success");
}
