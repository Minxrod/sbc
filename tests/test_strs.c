#include "test_util.h"

#include "interpreter/strs.h"
#include "data.h"
#include "system.h"

#include <string.h>

int test_strs(){
	/*
	string creation tests
	*/
	// Create string slots
	{
		struct strings strs;
		const u32 max = 8;
		
		init_mem_str(&strs, max, STRING_CHAR);
		
		ASSERT(strs.type == STRING_CHAR, "[init_mem_str] Correct string type");
		ASSERT(strs.strs_max == max, "[init_mem_str] Correct string count");
		ASSERT(strs.str_data != NULL, "[init_mem_str] Successful memory allocation");
		
		free_mem_str(&strs);
	}
	
	// Get unused string slots
	{
		struct strings strs;
		const u32 max = 8;
		
		init_mem_str(&strs, max, STRING_CHAR);
		
		struct string* new_str = get_new_str(&strs);
		
		ASSERT(strs.strs == new_str, "[get_new_str] Get string memory");
		new_str->uses += 1; // Mark string as used
		new_str = get_new_str(&strs);
		
		ASSERT(&strs.strs[1] == new_str, "[get_new_str] Get fresh string memory");
		
		free_mem_str(&strs);
	}
	
	// String comparison appears to work
	{
		char* s1 = "S\006ABC345";
		struct string s2 = {STRING_CHAR, 6, 1, {.s = (u8*)&s1[2]}};
		char* s3 = "S\005ABC34";
		
		s2.ptr.s = (u8*)&s1[2];
		
		ASSERT(str_comp(s1, s1), "[str_comp] Compare same string (true)");
		ASSERT(str_comp(s1, &s2), "[str_comp] Compare two strings (true)");
		DENY(str_comp(&s2, s3), "[str_comp] Compare two strings (false)");
		DENY(str_comp(s1, s3), "[str_comp] Compare two strings (false)");
	}
	
	// String copy
	{
		char* s = "abcDEF";
		char buf[8];
		struct string s1 = {STRING_CHAR, 6, 0, {(u8*)s}};
		struct string s2 = {STRING_CHAR, 0, 0, {(u8*)buf}};
		str_copy(&s1, &s2);
		
		ASSERT(s2.type == STRING_CHAR, "[str_copy] Type is copied");
		ASSERT(s2.len == 6, "[str_copy] Length is copied");
		ASSERT(str_comp(&s1, &s2), "[str_copy] Strings compare the same");
	}
	
	// String concatenate
	{
		char* a = "abcdef";
		char* b = "1234";
		char* c = "S\012abcdef1234";
		char buf[16];
		
		struct string s1 = {STRING_CHAR, 6, 1, {(u8*)a}};
		struct string s2 = {STRING_CHAR, 4, 1, {(u8*)b}};
		struct string s3 = {STRING_CHAR, 0, 0, {(u8*)buf}};
		
		str_concat(&s1, &s2, &s3);
		ASSERT(s3.len == 10, "[str_concat] Length of combined strings is correct");
		ASSERT(str_comp(&s3, c), "[str_concat] Concatenate works correctly");
	}
	
	// Character conversion is invertible
	{
		u8 i = 0;
		do {
			iprintf("%d,%x,%d\n", i, to_wide(i), to_char(to_wide(i)));
			ASSERT(i == to_char(to_wide(i)), "[to_wide,to_char] Conversion inverses");
			i++; //unsigned overflow OK!
		} while (i);
	}
	
	// Number to string conversion
	{
		// setup
		u8 buf[32];
		buf[0] = STRING_INLINE_CHAR;
		u8* str = buf;
		
		// tests
		fixp_to_str(INT_TO_FP(1235), str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0041235", (void*)buf), "[str_num] Convert 1235 to string");
		
		fixp_to_str(0, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0010", (void*)buf), "[str_num] Convert 0 to string");
		
		fixp_to_str(5570, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0041.36", (void*)buf), "[str_num] Convert 1.36 to string");
		
		fixp_to_str(11386, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0042.78", (void*)buf), "[str_num] Convert 2.78 to string");
		
		fixp_to_str(4, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0050.001", (void*)buf), "[str_num] Convert 0.001 to string");
		
		fixp_to_str(INT_TO_FP(524287), str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\006524287", (void*)buf), "[str_num] Convert largest number to string");
		
		fixp_to_str(-(INT_TO_FP(524287)), str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\007-524287", (void*)buf), "[str_num] Convert smallest number to string");
		
		fixp_to_str(-(INT_TO_FP(23456)), str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\006-23456", (void*)buf), "[str_num] Convert negative number to string");
		
		fixp_to_str(-4, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\006-0.001", (void*)buf), "[str_num] Convert -0.001 to string");
		
		fixp_to_str(-2, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\002-0", (void*)buf), "[str_num] Convert -2/4096 to string");
		
		fixp_to_str(2, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0010", (void*)buf), "[str_num] Convert 2/4096 to string");
		
		fixp_to_str(3, str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\0050.001", (void*)buf), "[str_num] Convert 3/4096 to string");
		
		fixp_to_str(INT_TO_FP(10), str);
		iprintf("%s\n", (char*)buf);
		ASSERT(str_comp("S\00210", (void*)buf), "[str_num] Convert 10 to string");
	}
	
	// Test read_one numbers
	{
		s32 num = 0;
		struct stack_entry s = {VAR_NUMBER | VAR_VARIABLE, {.ptr = &num}};
		u8 input[] = {'4','6',BC_DATA_DELIM,'5'};
		int count = read_one_u8(NULL, input, 4, &s);
		
		ASSERT(count == 2, "[read_one] Read two characters");
		ASSERT(num == INT_TO_FP(46), "[read_one] Value read is 46");
		
		count = read_one_u8(NULL, &input[3], 1, &s);
		
		ASSERT(count == 1, "[read_one] Read one character");
		ASSERT(num == INT_TO_FP(5), "[read_one] Value read is 5");
	}
	
	SUCCESS("test_strs success");
}
