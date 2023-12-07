/* 
 * CS:APP Data Lab 
 * 
 * <P>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  /* Draw the truth table of the & operator,
  which is easy to obtain with the ~ operator.
   */
  return ~(~(~x & y) & ~(x & ~y));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /* 10000...00 */
  return 0x1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /* 0111...111
  firstly, because only maximum return boolean value 1
  otherwise return 0. We can change x to all zeros and then take !
  we can use ~((x+1) ^ x) to determine whether x is maximum and 
  change x to all zeros, but we then find 0xffffffff also satisfy
  the conditions, we must exclude it, we can use !(x+1) 0r !(~x)to distinguish
  between maximum and -1 (0xffffffff), only two conditions both satified
  (both equals 0)can return 0x0, so we select | to connect two conditions
  */
  return !(~((x+1) ^ x) | !(~x));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /* 1010 & x = 1010, x satisfied the request, we extend the 1010
  we get 0xAAAAAAAAA, if 0xAAAAAAAA & x = 0xAAAAAAAA, we satistied
  */
  int right = 0xAA + (0xAA << 8); // 0xAA00, right = 0xAA | (0xAA << 8)
  int all = right + (right << 16); // 0xAAAA AAAA, all = right | (right << 16)
  //return (x & all) == all;
  return !(((x & all) + ~all) + 1);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /* 0x30 = 0011 0000
     0x39 = 0011 1001
     we first need to confirm the left part is 0011, we make it using (~(x >> 4)) + 0x3.
     Secondly, we need to make sure the right part is not bigger than 1001,
     we can set a constant, if x + consatand = overflow, we see overflow, then we know it not satisfied.
     1001 + 1 = 1010, 1010(10) + 0110(6) = 10000(overflow 1)， 0011 1010 + 0110 = 0100 0000
     we take the addition result shifting right 6 positions, if it equals to zero, it not overflow, if not
     we know it has overflow, it not satisfied.
     Two conditions must both equals 0, we use | and make sure the result is 0, and finally take !
  */
  //return !((x >> 4 != 0x3) | (x + 0x6) >> 6);
  return !( (~((~(x >> 4)) + 0x3)) | ((x + 0x6) >> 6));


}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /* take !x we get 0x1 when x is 0x0, we get 0x0 when x is true value
  we make a mask which can change 0x1 to 0x00000000, change 0x0 to 0xffffffff
  we need some bit operations, when mask is 0x00000000(x is false), it should return z
  when mask is 0xffffffff(x is true), it should return y, we can from below equality get 
  the answer. Notice: we take one byte of mask to represent the mask.
  0000 ^ x = x               1111 ^ x = ~x
  0000 & x = 0000            1111 & x = x
  0000 | x = x               1111 | x = 1111
  */
  // mask can make !!x = 0001->1111(0x1->0xffffffff), 0000->0000(0x0->0x00000000)
  //int mask = !x - 1;
  //int mask = (~!!x) + 1;
  int mask = ~(!!x) + 1;
  //mask = 0000 -> z      |  mask = 0000 -> 0000 = z
  //mask = 1111 -> 0000   |  mask = 1111 -> y   = y
  return ((mask ^ z) & z) | (mask & y);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /* We want to use y - x < 0 to represent the function result, but it can be 
  overflow consulting the result can not correct
  so we need to consider three special situations and one regular situation.
  Three special situations:
   1. x > 0; y < 0, return 0x0, !((x >> 31 == 0) & (y >> 31 < 0))
      x = 0111 1111 1111 ...
      y = 1000 0000 0000 ...
   2. x < 0; y > 0, return 0x1, (x >> 31 < 0) & (y >> 31 == 0)
      x = 1000 0000 0000 ...
      y = 0111 1111 1111 ...
   3. x = y < 0, return 0x1, satisfy ! ((y - x) >> 31) is true (-x using ~x+1 to replacement)
      x = 1000 0000 0000 ...
      y = 1000 0000 0000 ...
  One regular situation, y - x < 0, ! ((y - x) >> 31) is true (-x using ~x+1 to replacement):
   1. x = 010
      y = 011
  */
  return ((!!(x >> 31) & !(y >> 31)) | !((y + (~x) + 1) >> 31)) & !(!!(y >> 31) & !(x >> 31));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  /* When x = 0x0, after ~x + 1 = 0x0, the most significant bit not change(0),
  if x != 0x0, after !x + 1, the most siginificant bit is different， at that time,
  we have (0x0 | 0xffffffff) + 1 = 0xffffffff + 1 = 0x1
  */
  return ((~x + 1) >> 31 | x >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4 
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  /* For positive x, we need to find out the zero bits number before the first 1 bit,
  then add 1 sign position.
  For negative x, like -5 = 1011 = 1111 1011 = 1111 1111 1011 = 0xffffffb,
   x --> ~x,  0000 0000 0100, we can change negative problem to positive problem,
  we need to find out the zero bits number before the first 1 bit,
  then add 1 sign position, that is the minumum number of the negative x.
  */
  int b0, b16, b8, b4, b2, b1;
  // when x < 0, x = ~x, except x = 0x80000000 --> 0x7fffffff
  // when x > 0, x not change
  int sign = x >> 31;
  x = (sign & ~x)|(~sign & x);
  b16 = !!(x >> 16) << 4; // return 16 if the highest 16 bits has 1 
  x = x >> b16;
  b8 = !!(x >> 8) << 3; //return 8 if the highest 8 bits has 1
  x = x >> b8;
  b4 = !!(x >> 4) << 2; // return 4 if the highesst 4 bits has 1
  x = x >> b4;
  b2 = !!(x >> 2) << 1; // return 2 if the highest 2 bits has 1
  x = x >> b2;
  b1 = !!(x >> 1); //return 1 if the highest 1 bits is 1
  x = x >> b1;
  b0 = x;
  return b16 + b8 + b4 + b2 + b1 + b0 + 1; // 1 is sign position;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned exp = ((uf >> 23) << 24);
  if (uf == 0x0 || uf == (1 << 31)) { // uf is +0 or -0
    return uf;
  } else if (exp == 0x0) { // exponent part is all zeros, two times fraction part, add sign bits
    return (uf * 2) + ((1 << 31) & uf);
  } else if (!~(exp + (exp >> 8) + (exp >> 16) + (exp >> 24))) { // exponent part is all ones
    return uf;
  } else { // all one to the exponent part
    return (uf + (1 << 23));
  }
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  //the least significant of the int has exp bits
  int ans;
  unsigned expNum, fract;
  unsigned sign = uf >> 31;
  unsigned exp = (uf << 1) >> 24;
  // Anything out of range (including NaN and infinity) should return 0x80000000u.
  if (exp == 0xff) {
    return (1 << 31);
  } else if (exp < 0x7f) { 
    //if the exponent part is less than 0111 1111, the floating is less than one
    return 0x0;
  } else if (exp == 0x7f && sign == 0) { // floating is 1
    return 0x1;
  } else if (exp == 0x7f && sign == 1) { // floating is -1
    return ~0x0;
  }
  //the expNum least significant of the int is the same as the first expNum bits of the 
  // float fraction part 
  expNum = exp - 0x7f; // E = exp -127
  if (expNum > 23) {
    return (1 << 31);
  } 
  //fract is consisted of the expNum most significant bits of the fraction part
  fract = (uf << 9) >> (32 - expNum);
  if (sign == 0x0) { // positive float
    ans = fract + (1 << expNum);
  } else { // negative float
    ans = fract + ((1 << 31) >> (31 - expNum)); 
  }
  return ans;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  unsigned exp;
  // when exp > 0111 1111, return +IFN
  if (x > 0x7f) {
    return (0xff << 23);
  } else if (x < ((0xf2 << 24) >> 24)) {
    // when exp < 1 - 127 = -126(1111 ... 1111 1000 0010), return 0
    return 0x0;
  }
  exp = 0x7f + x;
  return (exp << 23);
}
