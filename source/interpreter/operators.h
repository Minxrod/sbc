#pragma once
/// 
/// @file
/// @brief Various PTC operator functions. Arithmetic, comparison, etc.
/// 

struct sbc;

/// Implements the addition operation of PTC.
/// 
/// Addition is a binary operator. Can be used to add two numbers or 
/// concatenate two strings.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_add(struct sbc* p);

/// Implements the subtraction operation of PTC.
/// 
/// Subtraction is a binary operator.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_subtract(struct sbc* p);

/// Implements the multiplication operation of PTC.
/// 
/// Multiplication is a binary operator.
/// Can be used to add two numbers, or repeat a string some number of times.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_multiply(struct sbc* p);

/// Implements the division operation of PTC.
/// 
/// Division is a binary operator.
/// 
/// On errors such as overflow or division by zero, 
/// sets the system error code in `p`.
/// 
/// @param p System struct
void op_divide(struct sbc* p);
void op_modulo(struct sbc* p);

void op_assign(struct sbc* p);

void op_comma(struct sbc* p);
void op_semicolon(struct sbc* p);

void op_negate(struct sbc* p);

void op_equal(struct sbc* p);
void op_not_equal(struct sbc* p);
void op_less(struct sbc* p);
void op_greater(struct sbc* p);
void op_less_equal(struct sbc* p);
void op_greater_equal(struct sbc* p);

void op_and(struct sbc* p);
void op_or(struct sbc* p);
void op_xor(struct sbc* p);
void op_not(struct sbc* p);
void op_logical_not(struct sbc* p);

