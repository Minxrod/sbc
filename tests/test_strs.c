#include "strs.h"

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
	
	SUCCESS("test_strs success");
}
