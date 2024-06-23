#pragma once
/// 
/// @file
/// @brief Defines the header format of a PTC SD-card file.
///
#include "common.h"

#define HEADER_TYPE_STR_SIZE 12
#define HEADER_SIZE 48
#define PRG_HEADER_SIZE 60

/// Represents a PTC SD file format header. Should correspond directly
/// to the byte representation of the header; however, this is only usable
/// directly on little-endian platforms.
struct ptc_header {
	/// Valid files will contain magic string "PX01"
	char magic[4];
	/// Size of file after MD5 (includes type string and data)
	/// Can be considered size of "internal file" of PTC
	u32 size_after_md5;
	/// PRG, CHR, MEM, etc. as ID number
	u32 type;
	/// The filename (max 8 chars, null-padded if needed)
	char name[8];
	/// MD5 (exactly)
	char md5[16];
	/// Type string (of form PETC0*00R***
	char type_str[HEADER_TYPE_STR_SIZE];
	/// Package bits (used for programs)
	u32 package_high;
	/// More package bits (used for programs)
	u32 package_low;
	/// Program size in bytes (excludes extra nulls at end)
	u32 prg_size;
};
