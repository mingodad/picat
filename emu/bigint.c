/********************************************************************
 *   File   : bigint.c
 *   Author : Neng-Fa ZHOU
 *   Updated: Last updated Aug. 2013
 *   Purpose: Simple (but slow) implementation of arithmetic on big integers 
 *            Based on the  C++ Big Integer Library
 *            http://mattmccutchen.net/bigint/
 *            Matt McCutchen <matt@mattmccutchen.net>

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "basic.h"
#include "bapi.h"
#include "term.h"

/* A big integer is represented as a structure $bigint(SignSize,Ds)
   where Ds is a list of base-(2^28) "digits" (from the most significant
   to the least significant digits), and SignSize is a primitive integer 
   whose sign indicates the sign of the big integer, and magnitude 
   indicates the length of Ds. A big integer x must be 

   x > 268435455  or x < -268435455

   Example: 

   1111222233334444555566667777888899990000 

   is represented as 

   $bigint(5,[214013,165733330,21745286,83495097,146178544]).

*/
#define BIGINT_COMPUTE_CARRY_VAL(x, val, carry) {       \
        if (x >= BP_BIGINT_BASE) {                      \
            val = x-BP_BIGINT_BASE;                     \
            carry = 1;                                  \
        } else {                                        \
            val = x;                                    \
            carry = 0;                                  \
        }                                               \
        }

#define BIGINT_COMPUTE_BORROW_VAL(x, val, carry) {      \
        if (x < 0) {                                    \
            val = x+BP_BIGINT_BASE;                     \
            borrow = 1;                                 \
        } else {                                        \
            val = x;                                    \
            borrow = 0;                                 \
        }                                               \
        }


#define BP_DECOMPOSE_BIGINT(op, sign, size, DLst) {                     \
        BPLONG_PTR ptr;                                                 \
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);                            \
        size = FOLLOW(ptr+1); size = INTVAL(size);                      \
        if (size < 0) {size = -size; sign = -1;} else {sign = 1;}       \
        DLst = FOLLOW(ptr+2);                                           \
        }

#define BP_MAKE_BIGINT_FROM_DLST(sign, size, DLst, op) {        \
        BPLONG_PTR ptr = heap_top;                              \
        FOLLOW(heap_top) = (BPLONG)bigint_psc;                  \
        heap_top += 3;                                          \
        FOLLOW(ptr+1) = MAKEINT(sign*size);                     \
        FOLLOW(ptr+2) = DLst;                                   \
        op = ADDTAG(ptr, STR);                                  \
        }

// An unsigned bigint is to be stored in an array x[0],x[1],...,x[xsize-1]
// where x[0] is the lowest digit and x[xsize-1] is the highest digit
// DLst!=nil_sym, xsize > 0
#define BP_MAKE_UBIG_FROM_DLST(DLst, size, x) {         \
        BPLONG_PTR cell_ptr;                            \
        BPLONG i, lst;                                  \
        i = 0; lst = DLst;                              \
        do {                                            \
            cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);  \
            x[i] = INTVAL(FOLLOW(cell_ptr));            \
            lst = FOLLOW(cell_ptr+1);                   \
            i++;                                        \
        } while (ISLIST(lst));                          \
        }

/* size>0 */
#define BP_MAKE_BIGINT_FROM_UBIG(sign, size, x, op) {   \
        BPLONG_PTR cell_ptr;                            \
        BPLONG i;                                       \
        BPLONG DLst = nil_sym;                          \
        for (i = size-1; i >= 0; i--) {                 \
            cell_ptr = heap_top;                        \
            NEW_HEAP_NODE(MAKEINT(x[i]));               \
            NEW_HEAP_NODE(DLst);                        \
            DLst = ADDTAG(cell_ptr, LST);               \
        }                                               \
        BP_MAKE_BIGINT_FROM_DLST(sign, size, DLst, op); \
        }


int c_test_bigint() {
    BPLONG res;

    res = bp_mul_bigint_bigint(bp_int_to_bigint(-536870910), bp_int_to_bigint(1));
    write_term(res);


    /*
      printf("bp_int_to_bigint(999999999) \n");
      write_term(bp_int_to_bigint(999999999L)); printf("\n");

      printf("bp_double_to_bigint(999999999.0) \n");
      write_term(bp_double_to_bigint(999999999.0));printf("\n");

      res = bp_call_string("fail");
      printf("res=%d bp_exception=%x\n",res,bp_exception);
    */


    /*
      BPLONG m3 = bp_int_to_bigint(-3);
      BPLONG m4 = bp_int_to_bigint(-4);
      BPLONG m5 = bp_int_to_bigint(-5);
      BPLONG m6 = bp_int_to_bigint(-6);
      BPLONG p3 = bp_int_to_bigint(3);

      printf("-3/>3 expect -1 \n");
      write_term(bp_updiv_bigint_bigint(m3,p3)); printf("\n");
      printf("-3/<3 expect -1 \n");
      write_term(bp_lowdiv_bigint_bigint(m3,p3)); printf("\n");
      printf("-4/>3 expect -1 \n");
      write_term(bp_updiv_bigint_bigint(m4,p3)); printf("\n");
      printf("-4/<3 expect -2 \n");
      write_term(bp_lowdiv_bigint_bigint(m4,p3)); printf("\n");
      printf("4/>3 expect 2 \n");
      write_term(bp_updiv_bigint_bigint(bp_int_to_bigint(4),p3)); printf("\n");
      printf("4/<3 expect 1 \n");
      write_term(bp_lowdiv_bigint_bigint(bp_int_to_bigint(4),p3)); printf("\n");
    */

    /*
      BPLONG m3 = bp_int_to_bigint(-3);
      BPLONG m4 = bp_int_to_bigint(-4);
      BPLONG m5 = bp_int_to_bigint(-5);
      BPLONG m6 = bp_int_to_bigint(-6);
      BPLONG p3 = bp_int_to_bigint(3);
      printf("-3 3 expect -1 0\n");
      write_term(bp_div_bigint_bigint(m3,p3)); printf("\n");
      write_term(bp_mod_bigint_bigint(m3,p3)); printf("\n");
      printf("-4 3 expect -2 2\n");
      write_term(bp_div_bigint_bigint(m4,p3)); printf("\n");
      write_term(bp_mod_bigint_bigint(m4,p3)); printf("\n");
      printf("-5 3 expect -2 1\n");
      write_term(bp_div_bigint_bigint(m5,p3)); printf("\n");
      write_term(bp_mod_bigint_bigint(m5,p3));  printf("\n");
      printf("-6 3 expect -2 0\n");
      write_term(bp_div_bigint_bigint(m6,p3)); printf("\n");
      write_term(bp_mod_bigint_bigint(m6,p3)); printf("\n");
    */

    /*
      BPLONG zero = bp_int_to_bigint(0);
      BPLONG op = bp_int_to_bigint(536870910);
      BPLONG mop = bp_sub_bigint_bigint(zero,op);
      bp_print_bigint(mop);
      printf("-op= "); write_term(mop);  printf("\n");  
      BPLONG ten = bp_int_to_bigint(10);
      BPLONG di = bp_div_bigint_bigint(op,ten);
      BPLONG re = bp_mod_bigint_bigint(op,ten);
      printf("op= "); write_term(op);  printf("\n");
      printf("ten= ");write_term(ten); printf("\n");
      printf("di= ");write_term(di);  printf("\n");
      printf("re= ");write_term(re); printf("\n");
    */
    return BP_TRUE;
}


/* decrease the size if the leading digit is zero */
INLINE void zap_leading_zeros(BPLONG_PTR xsize_ptr, UBIGINT x) {
    BPLONG size = *xsize_ptr;
    while (size > 0 && x[size-1] == 0) size--;
    *xsize_ptr = size;
}

/* x must not be 0 and must be either =< -BP_BIGINT_BASE but not too small to cause underflow
   or >= BP_BIGINT_BASE but not too big to cause overflow,
*/
BPLONG bp_int_to_bigint(BPLONG a) {
    int sign, i, size;
    BPLONG op;
    UBIGINT x;

    sign = 1;
    if (a < 0) {
        sign = -1; a = -a;
    }
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 8);
    x = local_top - 8;  /* enough room for the digits */
    i = 0;
    while (a != 0) {
        x[i] = a % BP_BIGINT_BASE;
        a /= BP_BIGINT_BASE;
        i++;
    }
    size = i;
    BP_MAKE_BIGINT_FROM_UBIG(sign, size, x, op);

    return op;
}

// size > 0, overflow not checked
BPLONG bp_ubig_to_int(BPLONG size, UBIGINT x)
{
    BPLONG res;
    BPLONG i;

    res = x[size-1];
    for (i = size-2; i >= 0; i--) {
        res = res*BP_BIGINT_BASE+x[i];
    }
    return res;
}

BPLONG bp_bigint_to_int(BPLONG op) {
    BPLONG sign, size, DLst;
    UBIGINT x;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", size+1);
    x = (local_top-size-1);
    BP_MAKE_UBIG_FROM_DLST(DLst, size, x);
    return bp_ubig_to_int(size, x);
}

/* if op is non-negative, 2**56 < op <= 2**63, convert it to long; otherwise, return 0 */
BPLONG bp_bigint_to_native_long(BPLONG op) {
    BPLONG size, sign, DLst;
    UBIGINT x;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    if (sign != 1 || size != 3) return 0;
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", size+1);
    x = (local_top-size-1);
    BP_MAKE_UBIG_FROM_DLST(DLst, size, x);
    if (x[2] >= 128) return 0;
    return bp_ubig_to_int(size, x);
}

// size > 0
double bp_ubig_to_double(BPLONG size, UBIGINT x)
{
    double d;
    BPLONG i;

    d = (double)x[size-1];
    for (i = size-2; i >= 0; i--) {
        d = d*(double)BP_BIGINT_BASE+(double)x[i];
    }
    return d;
}

double bp_bigint_to_double(BPLONG op) {
    double d;
    BPLONG sign, size, DLst;
    UBIGINT x;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    x = (local_top-size-1);
    BP_MAKE_UBIG_FROM_DLST(DLst, size, x);
    d = bp_ubig_to_double(size, x);
    return (sign < 0) ? -d : d;
}


BPLONG bp_double_to_bigint(double a) {
    double f, modf();
    int sign, i, size;
    BPLONG op;
    UBIGINT x;

    sign = 1;
    if (a < 0) {
        sign = -1; a = -a;
    }
    modf(a, &a);

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 100);
    x = local_top - 100;  /* enough for the digits? */
    i = 0;

    while (a >= BP_BIGINT_BASE) {
        f = a/BP_BIGINT_BASE;
        modf(f, &f);
        x[i] = (BPLONG)(a-f*BP_BIGINT_BASE);
        a = f;
        i++;
    }
    op = (BPLONG)a;
    if (op != 0) x[i++] = op;
    size = i;
    BP_MAKE_BIGINT_FROM_UBIG(sign, size, x, op);
    return op;
}

/* y = y+1 */
void bp_inc_ubig(BPLONG_PTR ysize_ptr, UBIGINT y) {
    BPLONG i, sum, carry, ysize;

    ysize = *ysize_ptr;
    sum = y[0]+1;
    BIGINT_COMPUTE_CARRY_VAL(sum, y[0], carry);

    for (i = 1; i < ysize && carry == 1; i++) {
        sum = y[i]+1;
        BIGINT_COMPUTE_CARRY_VAL(sum, y[i], carry);
    }
    if (carry == 1) {
        y[ysize] = 1;
        *ysize_ptr = ysize+1;
    }
}

/* z = x+y */
void bp_add_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, sum, carry;

    if (xsize > ysize) {
        BPLONG tmp_size;
        UBIGINT tmp;
        tmp_size = xsize; xsize = ysize; ysize = tmp_size;
        tmp = x; x = y; y = tmp;
    }
    /* xsize <= ysize */
    carry = 0;
    for (i = 0; i < xsize; i++) {
        sum = x[i]+y[i]+carry;
        BIGINT_COMPUTE_CARRY_VAL(sum, z[i], carry);
    }
    for (; i < ysize && carry == 1; i++) {
        sum = y[i]+1;
        BIGINT_COMPUTE_CARRY_VAL(sum, z[i], carry);
    }
    if (carry == 1) {
        z[ysize] = 1;
        *zsize_ptr = ysize+1;
    } else {
        for (; i < ysize; i++) {
            z[i] = y[i];
        }
        *zsize_ptr = ysize;
    }
}

/* y = y-1 */
void bp_dec_ubig(BPLONG_PTR ysize_ptr, UBIGINT y) {
    BPLONG i, borrow, temp, ysize;

    ysize = *ysize_ptr;
    temp = y[0]-1;
    BIGINT_COMPUTE_BORROW_VAL(temp, y[0], borrow);

    for (i = 1; i < ysize && borrow == 1; i++) {
        temp = y[i]-1;
        BIGINT_COMPUTE_BORROW_VAL(temp, y[i], borrow);
    }
    zap_leading_zeros(ysize_ptr, y);
}

/* z = y-x, y>=x */
void bp_sub_ubig_ubig(BPLONG ysize, UBIGINT y, BPLONG xsize, UBIGINT x, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, borrow, temp;

    borrow = 0;
    for (i = 0; i < xsize; i++) {
        temp = y[i]-x[i]-borrow;
        BIGINT_COMPUTE_BORROW_VAL(temp, z[i], borrow);
    }
    for (; i < ysize && borrow == 1; i++) {
        temp = y[i]-1;
        BIGINT_COMPUTE_BORROW_VAL(temp, z[i], borrow);
    }
    for (; i < ysize; i++) {
        z[i] = y[i];
    }
    *zsize_ptr = ysize;
    zap_leading_zeros(zsize_ptr, z);
}

/* returns the ith digit of x << y,
   (0 =< y <= 27) and (0 =< i <= xsize)
*/
INLINE BPLONG get_shifted_digit(BPLONG xsize, UBIGINT x, BPLONG i, BPLONG y) {
    BPLONG part1, part2;
    part1 = (i == 0 || y == 0) ? 0 : (x[i - 1] >> (28 - y));
    part2 = (i == xsize) ? 0 : ((x[i] << y) & MASK_LOW28);
    return part1 | part2;
}

/* z = x*y, x>0, y>0, z has been allocated
 * Overall method:
 *
 * Set z = 0.
 * For each 1-bit of x (say the i2th bit of block i):
 *    Add `y << (i blocks and i2 bits)' to z
 * NOTE: the result may not be a big-int if either operand is not a big-int
 */
void bp_mul_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, j, k, i2, temp, carry, zsize;
    // Zero out z
    zsize = xsize+ysize;
    for (i = 0; i < zsize; i++) z[i] = 0;

    // For each digit of the first number...
    for (i = 0; i < xsize; i++) {
        // For each 1-bit of that digit...
        for (i2 = 0; i2 < 28; i2++) {
            if ((x[i] & (1 << i2)) == 0) continue;
            carry = 0;
            for (j = 0, k = i; j <= ysize; j++, k++) {
                temp = z[k] + get_shifted_digit(ysize, y, j, i2) + carry;
                BIGINT_COMPUTE_CARRY_VAL(temp, z[k], carry);
            }
            while (carry == 1) {
                temp = z[k]+1;
                BIGINT_COMPUTE_CARRY_VAL(temp, z[k], carry);
                k++;
            }
        }
    }
    // Zap possible leading zero
    if (z[zsize - 1] == 0) zsize--;
    *zsize_ptr = zsize;
}

/* x = y*q+r: q is the quotient and r is the remainder
 * precond: x>=y, ysize>=1, x, y, q, and r are already allocated
 * Overall method:
 *
 * copy x to r
 * For each appropriate i and i2, decreasing:
 *    Subtract (y << (i blocks and i2 bits)) from r, storing the
 *      result in subtractBuf.
 *    If the subtraction succeeds with a nonnegative result:
 *        Turn on bit i2 of block i of the quotient q.
 *        Copy subtractBuf back into r.
 *    Otherwise bit i2 of block i of q remains off, and r is unchanged.
 * 
 * Eventually q will contain the entire quotient, and r will
 * be left with the remainder.
 */
void bp_div_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR qsize_ptr, UBIGINT q, BPLONG_PTR rsize_ptr, UBIGINT r, UBIGINT subtractBuf) {
    BPLONG i, j, k, i2, temp, borrow, qsize, rsize;

    qsize = xsize-ysize+1;
    // copy x to r
    for (i = 0; i < xsize; i++) r[i] = x[i];
    rsize = xsize+1;
    r[xsize] = 0;

    // Zero out the quotient
    for (i = 0; i < qsize; i++) q[i] = 0;

    // For each possible left-shift of y in blocks...
    i = qsize;
    while (i > 0) {
        i--;
        // For each possible left-shift of y in bits...
        q[i] = 0;
        i2 = 28;
        while (i2 > 0) {
            i2--;
            /*
             * Subtract y, shifted left i blocks and i2 bits, from r,
             * and store the answer in subtractBuf.  
             */
            borrow = 0;
            for (j = 0, k = i; j <= ysize; j++, k++) {
                temp = r[k] - get_shifted_digit(ysize, y, j, i2) - borrow;
                BIGINT_COMPUTE_BORROW_VAL(temp, subtractBuf[k], borrow);
            }

            for (; k < xsize && borrow == 1; k++) {
                temp = r[k] - 1;
                BIGINT_COMPUTE_BORROW_VAL(temp, subtractBuf[k], borrow);
            }
            if (borrow == 0) {
                q[i] |= (1 << i2);
                while (k > i) {
                    k--;
                    r[k] = subtractBuf[k];
                }
            }
        }
    }
    // Zap possible leading zero in quotient
    if (q[qsize - 1] == 0) qsize--;
    *qsize_ptr = qsize;
    // Zap any/all leading zeros in remainder
    *rsize_ptr = rsize;
    zap_leading_zeros(rsize_ptr, r);
}


void bp_and_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, zsize;

    zsize = (xsize >= ysize) ? ysize : xsize;
    for (i = 0; i < zsize; i++) {
        z[i] = x[i] & y[i];
    }
    *zsize_ptr = zsize;
    zap_leading_zeros(zsize_ptr, z);
}

void bp_or_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i;
    if (xsize > ysize) {
        BPLONG tmp;
        UBIGINT tmp_ubig;
        tmp = xsize; xsize = ysize; ysize = tmp;
        tmp_ubig = x; x = y; y = tmp_ubig;
    }
    for (i = 0; i < xsize; i++)
        z[i] = x[i] | y[i];
    for (; i < ysize; i++)
        z[i] = y[i];
    *zsize_ptr = ysize;
}

void bp_xor_ubig_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i;
    if (xsize > ysize) {
        BPLONG tmp;
        UBIGINT tmp_ubig;
        tmp = xsize; xsize = ysize; ysize = tmp;
        tmp_ubig = x; x = y; y = tmp_ubig;
    }
    for (i = 0; i < xsize; i++)
        z[i] = x[i] ^ y[i];
    for (; i < ysize; i++)
        z[i] = y[i];
    *zsize_ptr = ysize;
    zap_leading_zeros(zsize_ptr, z);
}

/* y>0 */
void bp_shiftl_ubig_int(BPLONG xsize, UBIGINT x, BPLONG y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, j, zsize, shiftBlocks, shiftBits;

    shiftBlocks = y / 28;
    shiftBits = y % 28;
    zsize = xsize + shiftBlocks + 1;

    for (i = 0; i < shiftBlocks; i++) z[i] = 0;
    for (j = 0, i = shiftBlocks; j <= xsize; j++, i++) {
        z[i] = get_shifted_digit(xsize, x, j, shiftBits);
    }
    if (z[zsize - 1] == 0) zsize--;
    *zsize_ptr = zsize;
}

/* y>0 */
void bp_shiftr_ubig_int(BPLONG xsize, UBIGINT x, BPLONG y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, j, zsize, rightShiftBlocks, leftShiftBits;

    // This calculation is wacky, but expressing the shift as a left bit shift
    // within each block lets us use getShiftedBlock.
    rightShiftBlocks = (y + 27) / 28;
    leftShiftBits = 28 * rightShiftBlocks - y;
    // Now (28 * rightShiftBlocks - leftShiftBits) == y
    // and 0 <= leftShiftBits < 28.
    if (rightShiftBlocks >= xsize + 1) {
        *zsize_ptr = 0;
        return;
    }
    // Now we're allocating a positive amount.
    // + 1: room for high bits nudged left into another block
    zsize = xsize + 1 - rightShiftBlocks;
    for (j = rightShiftBlocks, i = 0; j <= xsize; j++, i++) {
        z[i] = get_shifted_digit(xsize, x, j, leftShiftBits);
    }
    if (z[zsize - 1] == 0) zsize--;
    *zsize_ptr = zsize;
}

/* xsize>0, x = ~x */
void bp_onescomplement_ubig(BPLONG_PTR xsize_ptr, UBIGINT x) {
    BPLONG i, xsize;

    xsize = *xsize_ptr;

    for (i = 0; i < xsize; i++) {
        x[i] = ((~x[i]) & MASK_LOW28);
    }
    zap_leading_zeros(xsize_ptr, x);
}

/* xsize>0, x = ~x+1 */
void bp_twoscomplement_ubig(BPLONG xsize, UBIGINT x) {
    BPLONG carry, i;
    /*
      printf("original       :");
      for (i=xsize-1;i>=0;i--) printf("x[%d]=%d ",i, x[i]);
      printf("\n");
    */
    for (i = 0; i < xsize; i++) {
        x[i] = ((~x[i]) & MASK_LOW28);
    }

    carry = 1; i = 0;
    while (i < xsize && carry == 1) {
        x[i] += carry;
        BIGINT_COMPUTE_CARRY_VAL(x[i], x[i], carry);
        i++;
    }
    x[xsize-1] |= BP_BIGINT_BASE;  // the imaginary sign bit
    /*
      printf("2's complement :");
      for (i=xsize-1;i>=0;i--) printf("x[%d]=%d ",i, x[i]);
      printf("\n");
    */
}

void bp_twoscomplement_magnitude(BPLONG_PTR zsize_ptr, UBIGINT z) {
    bp_dec_ubig(zsize_ptr, z);
    bp_onescomplement_ubig(zsize_ptr, z);
    zap_leading_zeros(zsize_ptr, z);
}

/* x is two's complement of a positive bigint, y>0, the imaginary sign bit stays while being shifted right */
void bp_shiftr_twoscomplement_int(BPLONG xsize, UBIGINT x, BPLONG y, BPLONG_PTR zsize_ptr, UBIGINT z) {
    BPLONG i, j, zsize, rightShiftBlocks, leftShiftBits, remainingBits;

    // This calculation is wacky, but expressing the shift as a left bit shift
    // within each block lets us use getShiftedBlock.
    //  x[xsize-1] = (x[xsize-1] & MASK_LOW28);
    rightShiftBlocks = (y + 27) / 28;
    leftShiftBits = 28 * rightShiftBlocks - y;
    remainingBits = y - 28*(rightShiftBlocks-1);
    zsize = xsize;
    if (rightShiftBlocks <= xsize) {
        for (j = rightShiftBlocks, i = 0; j <= xsize; j++, i++) {
            z[i] = get_shifted_digit(xsize, x, j, leftShiftBits);
        }
        i--;
        z[i] = z[i] | ((MASK_LOW28 >> leftShiftBits) << leftShiftBits);  // Build the pattern like 111...10000 with remaingBits 0's
    } else {
        i = -1;
    }
    for (j = i+1; j < zsize; j++) {
        z[j] = MASK_LOW28;
    }
    z[zsize-1] |= BP_BIGINT_BASE;  // the imaginary sign bit
    *zsize_ptr = zsize;
}

/* copy x to y, adding padding zero's, xsize<ysize */
void bp_copy_ubig(BPLONG xsize, UBIGINT x, BPLONG ysize, UBIGINT y) {
    BPLONG i;

    for (i = 0; i < xsize; i++) y[i] = x[i];
    for (i = xsize; i < ysize; i++) y[i] = 0;
}

/* abs(op), op is known to be a bigint */
BPLONG bp_abs_bigint(BPLONG op) {
    BPLONG size, sign, DLst;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    if (sign > 0) return op;
    BP_MAKE_BIGINT_FROM_DLST(1, size, DLst, op);
    return op;
}

/* -op, op is known to be a bigint */
BPLONG bp_neg_bigint(BPLONG op) {
    BPLONG size, sign, DLst;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    sign = -sign;
    BP_MAKE_BIGINT_FROM_DLST(sign, size, DLst, op);
    return op;
}

/* sign(op), op is known to be a bigint */
int bp_sign_bigint(BPLONG op) {
    BPLONG size, sign, DLst;

    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    return sign;
}

/* number of "digits" in op */
int c_bigint_sign_size() {
    BPLONG size, sign, DLst;
    BPLONG op = ARG(1, 3);  /* op must be a big integer */

    DEREF(op);
    BP_DECOMPOSE_BIGINT(op, sign, size, DLst);
    unify(MAKEINT(sign), ARG(2, 3));
    unify(MAKEINT(size), ARG(3, 3));
    return BP_TRUE;
}

/* Compare the magnitudes of two unsigned bigints that are of the same length */
int bp_compare_mag_mag(BPLONG size, UBIGINT x, UBIGINT y) {
    BPLONG i;
    for (i = size-1; i >= 0; i--) {
        if (x[i] > y[i]) return 1;
        if (y[i] > x[i]) return -1;
    }
    return 0;
}

/* op1+op2, op1 and op2 are known to be bigints */
BPLONG bp_add_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, z;
    BPLONG xsize, ysize, zsize, max_size, xsign, ysign, zsign, xDLst, yDLst;
    int comp_res;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    max_size = (xsize > ysize) ? xsize : ysize;
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 4*(max_size+1));
    z = local_top-max_size-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    // If the arguments have the same sign, take the
    // common sign and add their magnitudes.
    if (xsign > 0 && ysign > 0) {
        zsign = 1;
        bp_add_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsign < 0 && ysign < 0) {
        zsign = -1;
        bp_add_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsize > ysize) {
        zsign = xsign;
        bp_sub_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsize < ysize) {
        zsign = ysign;
        bp_sub_ubig_ubig(ysize, y, xsize, x, &zsize, z);
    } else {
        comp_res = bp_compare_mag_mag(xsize, x, y);
        if (comp_res == 0) {
            return BP_ZERO;  /* primitive 0 */
        } else if (comp_res > 0) {
            zsign = xsign;
            bp_sub_ubig_ubig(xsize, x, ysize, y, &zsize, z);
        } else {
            zsign = ysign;
            bp_sub_ubig_ubig(ysize, y, xsize, x, &zsize, z);
        }
    }
    /* zsize!=0 in here */
    if (zsize == 1) return MAKEINT(zsign*z[0]);
#ifdef M64BITS
    if (zsize == 2) return MAKEINT(zsign*(z[1]*BP_BIGINT_BASE+z[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

/* op1-op2, op1 and op2 are known to be bigints */
BPLONG bp_sub_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, z;
    BPLONG xsize, ysize, zsize, max_size, xsign, ysign, zsign, xDLst, yDLst;
    int comp_res;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    max_size = (xsize > ysize) ? xsize : ysize;
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 4*(max_size+1));
    z = local_top-max_size-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    // If the arguments have the same sign, take the
    // common sign and add their magnitudes.
    if (xsign > 0 && ysign < 0) {
        zsign = 1;
        bp_add_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsign < 0 && ysign > 0) {
        zsign = -1;
        bp_add_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsize > ysize) {
        zsign = xsign;
        bp_sub_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    } else if (xsize < ysize) {
        zsign = -1*ysign;
        bp_sub_ubig_ubig(ysize, y, xsize, x, &zsize, z);
    } else {
        comp_res = bp_compare_mag_mag(xsize, x, y);
        if (comp_res == 0) {
            return BP_ZERO;  /* primitive 0 */
        } else if (comp_res > 0) {
            zsign = xsign;
            bp_sub_ubig_ubig(xsize, x, ysize, y, &zsize, z);
        } else {
            zsign = -1*ysign;
            bp_sub_ubig_ubig(ysize, y, xsize, x, &zsize, z);
        }
    }
    /* zsize!=0 in here */
    if (zsize == 1) return MAKEINT(zsign*z[0]);
#ifdef M64BITS
    if (zsize == 2) return MAKEINT(zsign*(z[1]*BP_BIGINT_BASE+z[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

/* op1!=0 && op2!=0 */
BPLONG bp_mul_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, z;
    BPLONG xsize, ysize, zsize, xsign, ysign, zsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    //  printf("=>mul "); write_term(op1); printf(" ");  write_term(op2); printf("\n");

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 5*(xsize+ysize+1));
    z = local_top - (xsize+ysize)-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    // If the arguments have the same sign, take the
    // common sign and add their magnitudes.
    if (xsign > 0) {
        if (ysign > 0) {
            zsign = 1;
        } else {
            zsign = -1;
        }
    } else {
        if (ysign > 0) {
            zsign = -1;
        } else {
            zsign = 1;
        }
    }
    bp_mul_ubig_ubig(xsize, x, ysize, y, &zsize, z);
    if (zsize == 1) return MAKEINT(zsign*z[0]);  /* impossible, since abs(x) >= 2^28-1 and abs(y) >= 2^28-1 */
#ifdef M64BITS
    if (zsize == 2) return MAKEINT(zsign*(z[1]*BP_BIGINT_BASE+z[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

BPLONG bp_div_bigint_bigint(BPLONG op1, BPLONG op2) {
    //  BPLONG_PTR top;
    UBIGINT x, y, q, r, tempBuff;
    BPLONG xsize, ysize, qsize, rsize, xsign, ysign, qsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    //  printf("div "); bp_print_bigint(op1); bp_print_bigint(op2); printf("\n");

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 6*(xsize+1));
    q = local_top - xsize-1;
    r = q - xsize-1;
    x = r-xsize-1;
    y = x - ysize-1;
    tempBuff = y - xsize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    // If the arguments have the same sign, take the
    // common sign and add their magnitudes.
    if (xsize == 0) return BP_ZERO;  /* x==0 */

    if (xsign == ysign) {
        qsign = 1;
    } else {
        qsign = -1;
    }

    /*
     * It appears that we need a total of 3 corrections:
     * Decrease the magnitude of x; 
     * Increase the magnitude of q;
     * Find r = (y - 1) - r and give it the desired sign.
     */

    if (qsign == -1) {
        bp_dec_ubig(&xsize, x);
    }
    /* do this since x's magnitude is assumed to be greater than y's */
    if (xsize < ysize || (xsize == ysize && bp_compare_mag_mag(xsize, x, y) < 0)) {
        if (qsign == 1) {
            return BP_ZERO;
        } else {
            return BP_MONE;
        }
    }
    bp_div_ubig_ubig(xsize, x, ysize, y, &qsize, q, &rsize, r, tempBuff);
    if (qsign == -1) {
        bp_inc_ubig(&qsize, q);
    }

    /* q!= 0 once here */
    if (qsize == 1) return MAKEINT(qsign*q[0]);
#ifdef M64BITS
    if (qsize == 2) return MAKEINT(qsign*(q[1]*BP_BIGINT_BASE+q[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(qsign, qsize, q, op1);
    return op1;
}

BPLONG bp_mod_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, q, r, tempBuff;
    BPLONG xsize, ysize, qsize, rsize, xsign, ysign, qsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 6*(xsize+1));
    q = local_top - xsize-1;
    r = q - xsize-1;
    x = r-xsize-1;
    y = x - ysize-1;
    tempBuff = y - xsize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    if (xsize == 0) return BP_ZERO;

    if (xsign == ysign) {
        qsign = 1;
    } else {
        qsign = -1;
    }

    /*
     * It appears that we need a total of 3 corrections:
     * Decrease the magnitude of x; 
     * Increase the magnitude of q;
     * Find r = (y - 1) - r and give it the desired sign.
     */
    if (qsign == -1) {
        bp_dec_ubig(&xsize, x);
    }
    /* do this since x's magnitude is assumed to be greater than y's */
    if (xsize < ysize || (xsize == ysize && bp_compare_mag_mag(xsize, x, y) < 0)) {
        rsize = xsize; r = x;
    } else {
        bp_div_ubig_ubig(xsize, x, ysize, y, &qsize, q, &rsize, r, tempBuff);
    }
    if (qsign == -1) {
        // Modify the remainder.
        bp_sub_ubig_ubig(ysize, y, rsize, r, &rsize, r);
        bp_dec_ubig(&rsize, r);
    }

    if (rsize == 0) return BP_ZERO;
    if (rsize == 1) return MAKEINT(ysign*r[0]);
#ifdef M64BITS
    if (rsize == 2) return MAKEINT(ysign*(r[1]*BP_BIGINT_BASE+r[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(ysign, rsize, r, op1);
    return op1;
}

/* op1 vs op2, op1 and op2 are known to be bigints */
int bp_compare_bigint_bigint(BPLONG op1, BPLONG op2) {  /* stack overflow not checked */
    UBIGINT x, y;
    BPLONG xsize, ysize, xsign, ysign, xDLst, yDLst;

    //  printf("compre bigint "); write_term(op1); printf(" "); write_term(op2); printf("\n");
    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    if (xsign > 0 && ysign < 0) {
        return 1;
    } else if (xsign < 0 && ysign > 0) {
        return -1;
    }

    x = local_top-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    if (xsize > ysize) {
        return (xsign == 1) ? 1 : -1;
    } else if (xsize < ysize) {
        return (ysign == 1) ? -1 : 1;
    } else {
        return xsign*bp_compare_mag_mag(xsize, x, y);
    }
}

BPLONG bp_and_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, x1, y1, z;
    BPLONG xsize, ysize, size, zsize, xsign, ysign, zsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    size = (xsize > ysize) ? xsize : ysize;

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 5*(size+1));
    x1 = local_top - size-1;
    y1 = x1 - size-1;
    z = y1 - size-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    if (xsize < size)
        bp_copy_ubig(xsize, x, size, x1);
    else
        x1 = x;

    if (ysize < size)
        bp_copy_ubig(ysize, y, size, y1);
    else
        y1 = y;

    if (xsign < 0) bp_twoscomplement_ubig(size, x1);
    if (ysign < 0) bp_twoscomplement_ubig(size, y1);

    bp_and_ubig_ubig(size, x1, size, y1, &zsize, z);

    if (zsize == 0)
        return BP_ZERO;
    if (zsize == 1)  /* must be positive */
        return MAKEINT(z[0]);
    if (z[zsize-1] >= BP_BIGINT_BASE) {  /* negative */
        bp_twoscomplement_magnitude(&zsize, z);
        if (zsize == 0) return BP_ZERO;
        if (zsize == 1) return MAKEINT(-z[0]);
#ifdef M64BITS
        if (zsize == 2) return MAKEINT(-(z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = -1;
    } else {
#ifdef M64BITS
        if (zsize == 2) return MAKEINT((z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = 1;
    }
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

BPLONG bp_or_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, x1, y1, z;
    BPLONG xsize, ysize, size, zsize, xsign, ysign, zsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    size = (xsize > ysize) ? xsize : ysize;

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 5*(size+1));
    x1 = local_top - size-1;
    y1 = x1 - size-1;
    z = y1 - size-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    if (xsize < size)
        bp_copy_ubig(xsize, x, size, x1);
    else
        x1 = x;

    if (ysize < size)
        bp_copy_ubig(ysize, y, size, y1);
    else
        y1 = y;

    if (xsign < 0) bp_twoscomplement_ubig(size, x1);
    if (ysign < 0) bp_twoscomplement_ubig(size, y1);

    bp_or_ubig_ubig(size, x1, size, y1, &zsize, z);

    if (zsize == 0) return BP_ZERO;
    if (zsize == 1) return MAKEINT(z[0]);  /* must be positive */
    if (z[zsize-1] >= BP_BIGINT_BASE) {  /* negative */
        bp_twoscomplement_magnitude(&zsize, z);
        if (zsize == 0) return BP_ZERO;
        if (zsize == 1) return MAKEINT(-z[0]);
#ifdef M64BITS
        if (zsize == 2) return MAKEINT(-(z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = -1;
    } else {
#ifdef M64BITS
        if (zsize == 2) return MAKEINT((z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = 1;
    }
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

BPLONG bp_xor_bigint_bigint(BPLONG op1, BPLONG op2) {
    UBIGINT x, y, x1, y1, z;
    BPLONG xsize, ysize, size, zsize, xsign, ysign, zsign, xDLst, yDLst;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);
    BP_DECOMPOSE_BIGINT(op2, ysign, ysize, yDLst);

    size = (xsize > ysize) ? xsize : ysize;

    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 5*(size+1));
    x1 = local_top - size-1;
    y1 = x1 - size-1;
    z = y1 - size-1;
    x = z-xsize-1;
    y = x - ysize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    BP_MAKE_UBIG_FROM_DLST(yDLst, ysize, y);

    if (xsize < size)
        bp_copy_ubig(xsize, x, size, x1);
    else
        x1 = x;

    if (ysize < size)
        bp_copy_ubig(ysize, y, size, y1);
    else
        y1 = y;

    if (xsign < 0) bp_twoscomplement_ubig(size, x1);
    if (ysign < 0) bp_twoscomplement_ubig(size, y1);

    bp_xor_ubig_ubig(size, x1, size, y1, &zsize, z);

    if (zsize == 0)
        return BP_ZERO;
    if (zsize == 1)  /* must be positive */
        return MAKEINT(z[0]);
    if (z[zsize-1] >= BP_BIGINT_BASE) {  /* negative */
        bp_twoscomplement_magnitude(&zsize, z);
        if (zsize == 0) return BP_ZERO;
        if (zsize == 1) return MAKEINT(-z[0]);
#ifdef M64BITS
        if (zsize == 2) return MAKEINT(-(z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = -1;
    } else {
#ifdef M64BITS
        if (zsize == 2) return MAKEINT((z[1]*BP_BIGINT_BASE+z[0]));
#endif
        zsign = 1;
    }
    BP_MAKE_BIGINT_FROM_UBIG(zsign, zsize, z, op1);
    return op1;
}

/* op2 is untagged long int */
BPLONG bp_shiftl_bigint_int(BPLONG op1, BPLONG op2) {
    UBIGINT x, z;
    BPLONG xsize, zsize, xsign, xDLst;

    if (op2 < 0) return bp_shiftr_bigint_int(op1, -op2);
    if (op2 == 0) return op1;
    if (op2 > BP_MAXINT_1W/10) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);

    zsize = xsize + 10 + op2;
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 3*(xsize+zsize));
    x = local_top-xsize-1;
    z = x - zsize-1;

    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);

    bp_shiftl_ubig_int(xsize, x, op2, &zsize, z);

    if (zsize == 0) return BP_ZERO;
    if (zsize == 1)return MAKEINT(xsign*z[0]);
#ifdef M64BITS
    if (zsize == 2) return MAKEINT(xsign*(z[1]*BP_BIGINT_BASE+z[0]));
#endif
    BP_MAKE_BIGINT_FROM_UBIG(xsign, zsize, z, op1);
    return op1;
}

BPLONG bp_shiftr_bigint_int(BPLONG op1, BPLONG op2) {
    UBIGINT x, z;
    BPLONG xsize, zsize, xsign, xDLst;

    if (op2 < 0) return bp_shiftl_bigint_int(op1, -op2);
    if (op2 == 0) return op1;

    BP_DECOMPOSE_BIGINT(op1, xsign, xsize, xDLst);

    zsize = xsize;
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 3*(xsize+zsize));
    x = local_top-xsize-1;
    z = x - zsize-1;
    if (z <= heap_top) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);

    if (xsign > 0) {
        bp_shiftr_ubig_int(xsize, x, op2, &zsize, z);
        if (zsize == 0) return BP_ZERO;
        if (zsize == 1) return MAKEINT(z[0]);
#ifdef M64BITS
        if (zsize == 2) return MAKEINT(z[1]*BP_BIGINT_BASE+z[0]);
#endif
        xsign = 1;
    } else {
        bp_twoscomplement_ubig(xsize, x);
        //    printf("before shiftr "); for (i=xsize-1;i>=0;i--) printf("x[%d]=%d ",i,x[i]); printf("\n");
        bp_shiftr_twoscomplement_int(xsize, x, op2, &zsize, z);
        //    printf("after  shiftr "); for (i=xsize-1;i>=0;i--) printf("z[%d]=%d ",i,z[i]); printf("\n");
        bp_twoscomplement_magnitude(&zsize, z);
        if (zsize == 0) return BP_ZERO;
        if (zsize == 1) return MAKEINT(-z[0]);
#ifdef M64BITS
        if (zsize == 2) return MAKEINT(-(z[1]*BP_BIGINT_BASE+z[0]));
#endif
        xsign = -1;
    }
    BP_MAKE_BIGINT_FROM_UBIG(xsign, zsize, z, op1);
    return op1;
}

/* op1 /> op2: smallest integer greater than or equal to op1/op2 */
BPLONG bp_updiv_bigint_bigint(BPLONG op1, BPLONG op2) {
    BPLONG tmp, rem, sign;

    if (bp_sign_bigint(op2) < 0) {
        op1 = bp_neg_bigint(op1);
        op2 = bp_neg_bigint(op2);
    }
    sign = bp_sign_bigint(op1);
    op1 = bp_abs_bigint(op1);
    tmp = bp_div_bigint_bigint(op1, op2);
    if (tmp == BP_ERROR) return BP_ERROR;
    //  write_term(op1); printf("/");write_term(op2); printf("="); write_term(tmp);printf("\n");
    if (sign > 0) {
        rem = bp_mod_bigint_bigint(op1, op2);
        if (rem == BP_ERROR) return BP_ERROR;
        if (rem != BP_ZERO) {
            if (ISINT(tmp)) {
                tmp = INTVAL(tmp)+1;
                tmp = (tmp <= BP_MAXINT_1W) ? MAKEINT(tmp) : bp_int_to_bigint(tmp);
            } else {
                tmp = bp_add_bigint_bigint(tmp, bp_int_to_bigint(1));
                if (tmp == BP_ERROR) return BP_ERROR;
            }
        }
    } else {
        if (ISINT(tmp)) {
            tmp = MAKEINT(-INTVAL(tmp));
        } else {
            tmp = bp_neg_bigint(tmp);
        }
    }
    return tmp;
}

/* op1 /< op2: largest integer less than or equal to op1/op2 */
BPLONG bp_lowdiv_bigint_bigint(BPLONG op1, BPLONG op2) {
    BPLONG tmp, rem, sign;

    if (bp_sign_bigint(op2) < 0) {
        op1 = bp_neg_bigint(op1);
        op2 = bp_neg_bigint(op2);
    }
    sign = bp_sign_bigint(op1);
    op1 = bp_abs_bigint(op1);
    tmp = bp_div_bigint_bigint(op1, op2);
    if (tmp == BP_ERROR) return BP_ERROR;
    if (sign < 0) {
        rem = bp_mod_bigint_bigint(op1, op2);
        if (rem != BP_ZERO) {
            if (ISINT(tmp)) {
                tmp = MAKEINT(-INTVAL(tmp)-1);
            } else {
                tmp = bp_sub_bigint_bigint(bp_neg_bigint(tmp), bp_int_to_bigint(1));
                if (tmp == BP_ERROR) return BP_ERROR;
            }
        }
    }
    return tmp;
}


BPLONG bp_gcd_bigint_bigint(BPLONG i1, BPLONG i2) {
    BPLONG temp;

    if (bp_sign_bigint(i1) < 0) i1 = bp_neg_bigint(i1);
    if (bp_sign_bigint(i2) < 0) i2 = bp_neg_bigint(i2);
    if (bp_compare_bigint_bigint(i1, i2) > 0) {
        temp = i1;
        i1 = i2;
        i2 = temp;
    }
    for (; ; ) {
        temp = bp_mod_bigint_bigint(i2, i1);
        if (temp == BP_ERROR) return BP_ERROR;
        if (temp == BP_ZERO)
            return bp_add_bigint_bigint(i1, bp_int_to_bigint(0));
        i2 = i1;
        if (ISINT(temp)) {
            i1 = bp_int_to_bigint(INTVAL(temp));
        } else {
            i1 = temp;
        }
    }
}

/* base^ex, base is bigint (abs(base)>1) and ex is int (ex>0), return bigint or int */
BPLONG bp_pow_bigint_int(BPLONG base, BPLONG ex) {
    BPLONG result;

    result = bp_int_to_bigint(1);

    for (; ; ) {
        if (ISINT(result)) result = bp_int_to_bigint(INTVAL(result));
        if ((BPULONG)ex & (BPULONG)1) {
            result = bp_mul_bigint_bigint(result, base);
            if (result == BP_ERROR) return BP_ERROR;
        }
        ex >>= 1;
        if (ex == 0) break;
        base = bp_mul_bigint_bigint(base, base);
        if (base == BP_ERROR) return BP_ERROR;
        if (ISINT(base)) base = bp_int_to_bigint(INTVAL(base));
    }
    return result;
}

/* write a bigint into a string buffer whose size is buf_size, and return the starting index.
   op must be a bigint.
*/
int bp_write_bigint_to_str(BPLONG op, char *buf, BPLONG buf_size) {
    UBIGINT x, y, q, r, tempBuff;
    BPLONG sign, size, dLst, qsize, rsize;
    char loc_buf[10];
    int i, j;

    BP_DECOMPOSE_BIGINT(op, sign, size, dLst);
    LOCAL_OVERFLOW_CHECK_WITH_MARGIN("bigint", 6*(size+1));
    q = local_top - size-1;
    r = q - size-1;
    x = r-size-1;
    y = x - 1;  /* one slot for holding 10^8 */
    tempBuff = y - size-1;
    BP_MAKE_UBIG_FROM_DLST(dLst, size, x);
    y[0] = 100000000;  /* 10^8 */

    i = buf_size-1;
    buf[i--] = '\0';
    do {
        BPLONG remainder;
        bp_div_ubig_ubig(size, x, 1, y, &qsize, q, &rsize, r, tempBuff);  /* q*(10^8)+r = x */
        remainder = r[0];  /* must have only one block */
        if (i <= 8) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        for (j = 0; j < 8; j++) {
            buf[i--] = remainder%10+'0';
            remainder = remainder/10;
        }
        for (j = 0; j < qsize; j++) {  /* q becomes the next x to be divided */
            x[j] = q[j];
        }
        size = qsize;
    } while (size > 1);

    sprintf(loc_buf, "%d", (int)x[0]);  /* write the first block */
    j = strlen(loc_buf);
    if (i <= j) {  /* leave a slot for the sign */
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    j--;
    while (j >= 0) {
        buf[i] = loc_buf[j];
        j--; i--;
    }
    if (sign < 0) {
        buf[i] = '-';
    } else {
        i++;
    }
    return i;
}

void bp_print_bigint(BPLONG op) {
    UBIGINT x;
    BPLONG xsize, xsign, xDLst, i;

    BP_DECOMPOSE_BIGINT(op, xsign, xsize, xDLst);
    if (xsign < 0) printf("-");
    x = local_top-xsize-1;
    BP_MAKE_UBIG_FROM_DLST(xDLst, xsize, x);
    printf("[");
    for (i = xsize-1; i > 0; i--) {
        printf(BPLONG_FMT_STR ",", x[i]);
    }
    printf(BPLONG_FMT_STR "]", x[0]);
}

/*
  Build an integer that is equal to w1*(2^28)+w0, where w0 and w1 are greater than -2^28 and less than 2^28.
  On a 32-bit computer, it is a bigint, while on a 64-bit computer it is a one-word int
*/
int b_BUILD_56B_INT_ccf(BPLONG w1, BPLONG w0, BPLONG v) {
    BPLONG res;
    DEREF_NONVAR(w1); w1 = INTVAL(w1);
    DEREF_NONVAR(w0); w0 = INTVAL(w0);
#ifdef M64BITS
    ASSIGN_f_atom(v, MAKEINT((w1 << 28 | w0)));
#else
    res = bp_int_to_bigint(w1);
    res = bp_shiftl_bigint_int(res, 28);
    res = bp_or_bigint_bigint(res, bp_int_to_bigint(w0));
    ASSIGN_v_heap_term(v, res);
#endif
    return BP_TRUE;
}




