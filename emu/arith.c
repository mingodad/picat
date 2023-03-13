/********************************************************************
 *   File   : arith.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2023
 *   Purpose: arithmetic functions 

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>                                                                                                     //branch
#include <math.h>
#include <errno.h>
#include "basic.h"
#include "term.h"
#include <float.h>
#include "bapi.h"

extern FILE *curr_out;

static SYM_REC_PTR
abs1,
    add1,
    add2,
    and2,
    acos1,
    asin1,
    atan1,
    sym_atan2,
    cap_psc,
    ceiling1,
    comp_psc1,
    cos1,
    div2,
    div_ge2,
    div_le2,
    exp1,
    epsilon0,
    float1,
    float_fractional_part1,
    float_int_part1,
    floor1,
    idiv2,
    idiv_div,
    log1,
    log22,
    shiftl2,
    integer1,
    max1,
    max2,
    maxint,
    gcd2,
    min1,
    min2,
    minint,
    mod2,
    mul2,
    negation1,
    or2,
    pi0,
    e0,
    ppow2,
    ran0,
    ran1,
    rem2,
    round1,
    shiftr2,
    sign1,
    sin1,
    sqrt1,
    sub1,
    sub2,
    sum1,
    prod1,
    tan1,
    truncate1, cputime1,
    xor2;

BPLONG atom_length;

/*
  taken from http://www.johndcook.com/IEEE_exceptions_in_cpp.html
*/
#define TEST_NAN(f, op) {                                               \
        if (f == f && f <= DBL_MAX && f >= -DBL_MAX) {}                 \
        else {                                                          \
            bp_exception = c_evaluation_error(et_FLOAT_OVERFLOW, op);   \
            return BP_ERROR;                                            \
        }                                                               \
    }

void init_arith_sym() {

    comp_psc1 = insert_sym("~", 1, 1);
    abs1 = insert_sym("abs", 3, 1);
    add1 = insert_sym("+", 1, 1);
    add2 = insert_sym("+", 1, 2);
    and2 = insert_sym("/\\", 2, 2);
    acos1 = insert_sym("acos", 4, 1);
    asin1 = insert_sym("asin", 4, 1);
    atan1 = insert_sym("atan", 4, 1);
    sym_atan2 = insert_sym("atan2", 5, 2);
    cap_psc = insert_sym("^", 1, 2);
    ceiling1 = insert_sym("ceiling", 7, 1);
    cos1 = insert_sym("cos", 3, 1);
    div2 = insert_sym("/", 1, 2);
    div_ge2 = insert_sym("/>", 2, 2);
    div_le2 = insert_sym("/<", 2, 2);
    exp1 = insert_sym("exp", 3, 1);
    epsilon0 = insert_sym("epsilon", 7, 0);
    float1 = insert_sym("float", 5, 1);
    float_fractional_part1 = insert_sym("float_fractional_part", 21, 1);
    float_int_part1 = insert_sym("float_integer_part", 18, 1);
    integer1 = insert_sym("integer", 7, 1);
    floor1 = insert_sym("floor", 5, 1);
    idiv2 = insert_sym("//", 2, 2);
    idiv_div = insert_sym("div", 3, 2);
    atom_length = ADDTAG(insert_sym("length", 6, 0), ATM);
    log1 = insert_sym("log", 3, 1);
    log22 = insert_sym("log", 3, 2);
    shiftl2 = insert_sym("<<", 2, 2);
    max1 = insert_sym("max", 3, 1);
    maxint = insert_sym("maxint", 6, 0);
    max2 = insert_sym("max", 3, 2);
    gcd2 = insert_sym("gcd", 3, 2);
    min1 = insert_sym("min", 3, 1);
    min2 = insert_sym("min", 3, 2);
    minint = insert_sym("minint", 6, 0);
    mod2 = insert_sym("mod", 3, 2);
    mul2 = insert_sym("*", 1, 2);
    negation1 = insert_sym("\\", 1, 1);
    or2 = insert_sym("\\/", 2, 2);
    pi0 = insert_sym("pi", 2, 0);
    e0 = insert_sym("e", 1, 0);
    ppow2 = insert_sym("**", 2, 2);
    rem2 = insert_sym("rem", 3, 2);
    round1 = insert_sym("round", 5, 1);
    ran0 = insert_sym("random", 6, 0);
    ran1 = insert_sym("random", 6, 1);
    shiftr2 = insert_sym(">>", 2, 2);
    sign1 = insert_sym("sign", 4, 1);
    sin1 = insert_sym("sin", 3, 1);
    sqrt1 = insert_sym("sqrt", 4, 1);
    sub1 = insert_sym("-", 1, 1);
    sub2 = insert_sym("-", 1, 2);
    sum1 = insert_sym("sum", 3, 1);
    prod1 = insert_sym("prod", 4, 1);
    tan1 = insert_sym("tan", 3, 1);
    truncate1 = insert_sym("truncate", 8, 1);
    cputime1 = insert_sym("cputime", 7, 0);
    xor2 = insert_sym("xor", 3, 2);
}

BPLONG bp_math_add(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) {
            return BP_ERROR;
        }
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) {
            return BP_ERROR;
        }
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op1 = INTVAL(op1) + INTVAL(op2);
            if (BP_IN_1W_INT_RANGE(op1)) {
                return MAKEINT(op1);
            } else {
                return bp_int_to_bigint(op1);
            }
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1((double)INTVAL(op1) + floatval(op2));
        } else {  /* must be bigint */
            return bp_add_bigint_bigint(bp_int_to_bigint(INTVAL(op1)), op2);
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            return encodefloat1(floatval(op1) + (double)INTVAL(op2));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(floatval(op1) + floatval(op2));
        } else {  /* op2 is bigint */
            return encodefloat1(floatval(op1) + bp_bigint_to_double(op2));
        }
    } else {  /* op1 is bigint */
        if (ISINT(op2)) {
            return bp_add_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(bp_bigint_to_double(op1) + floatval(op2));
        } else {  /* op2 is bigint */
            return bp_add_bigint_bigint(op1, op2);
        }
    }
}


BPLONG bp_math_sub(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op1 = INTVAL(op1) - INTVAL(op2);
            if (BP_IN_1W_INT_RANGE(op1)) {
                return MAKEINT(op1);
            } else {
                return bp_int_to_bigint(op1);
            }
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1((double)INTVAL(op1) - floatval(op2));
        } else {  /* op2 is bigint */
            return bp_sub_bigint_bigint(bp_int_to_bigint(INTVAL(op1)), op2);
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            return encodefloat1(floatval(op1) - (double)INTVAL(op2));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(floatval(op1) - floatval(op2));
        } else {  /* op2 is bigint */
            return encodefloat1(floatval(op1) - bp_bigint_to_double(op2));
        }
    } else {  /* op1 is bigint */
        if (ISINT(op2)) {
            return bp_sub_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(bp_bigint_to_double(op1) - floatval(op2));
        } else {  /* op2 is bigint */
            return bp_sub_bigint_bigint(op1, op2);
        }
    }
}


BPLONG bp_math_mul(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    BPLONG op3;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    //  printf("mul %x %x\n",op1,op2);
    //  write_term(op1); printf("  "); write_term(op2);   printf("\n");
    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
#ifdef M64BITS
            if (BP_IN_28B_INT_RANGE(op1) && BP_IN_28B_INT_RANGE(op2)) {
                op1 = op1*op2;
                return MAKEINT(op1);
            }
#else
            if (BP_IN_14B_INT_RANGE(op1) && BP_IN_14B_INT_RANGE(op2)) {
                op1 = op1*op2;
                return MAKEINT(op1);
            }
#endif
            if (op1 == 0 || op2 == 0) return BP_ZERO;
            return bp_mul_bigint_bigint(bp_int_to_bigint(op1), bp_int_to_bigint(op2));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(((double)op1) * floatval(op2));
        } else {
            return (op1 == 0) ? BP_ZERO : bp_mul_bigint_bigint(bp_int_to_bigint(op1), op2);
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            return encodefloat1(floatval(op1) * (double)INTVAL(op2));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(floatval(op1) * floatval(op2));
        } else {
            return encodefloat1(floatval(op1) * bp_bigint_to_double(op2));
        }
    } else {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            return (op2 == 0) ? BP_ZERO : bp_mul_bigint_bigint(op1, bp_int_to_bigint(op2));
        } else if (IS_FLOAT_PSC(op2)) {
            return encodefloat1(bp_bigint_to_double(op1) * floatval(op2));
        } else {
            return bp_mul_bigint_bigint(op1, op2);
        }
    }
}

BPLONG bp_math_divge(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            BPLONG tmp_op;
            op2 = INTVAL(op2);
            if (op2 < 0) { op1 = -op1; op2 = -op2;}
            UP_DIV(tmp_op, op1, op2);
            return MAKEINT(tmp_op);
        } else if (IS_BIGINT(op2)) {
            return bp_updiv_bigint_bigint(bp_int_to_bigint(op1), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_updiv_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_updiv_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_math_divle(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            BPLONG tmp_op;

            op2 = INTVAL(op2);
            if (op2 < 0) { op1 = -op1; op2 = -op2;}
            LOW_DIV(tmp_op, op1, op2);
            return MAKEINT(tmp_op);
        } else if (IS_BIGINT(op2)) {
            return bp_lowdiv_bigint_bigint(bp_int_to_bigint(op1), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_lowdiv_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_lowdiv_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_math_div(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1((double)INTVAL(op1) / (double)op2);
        } else if (IS_FLOAT_PSC(op2)) {
            double dop2 = floatval(op2);
            if (dop2 == 0.0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1((double)INTVAL(op1) / dop2);
        } else {
            return encodefloat1((double)INTVAL(op1) / bp_bigint_to_double(op2));
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1(floatval(op1) / (double)op2);
        } else if (IS_FLOAT_PSC(op2)) {
            double dop2 = floatval(op2);
            if (dop2 == 0.0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1(floatval(op1) / dop2);
        } else {
            return encodefloat1(floatval(op1) / bp_bigint_to_double(op2));
        }
    } else {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1(bp_bigint_to_double(op1)/(double)op2);
        } else if (IS_FLOAT_PSC(op2)) {
            double dop2 = floatval(op2);
            if (dop2 == 0.0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return encodefloat1(bp_bigint_to_double(op1) / dop2);
        } else {
            return encodefloat1(bp_bigint_to_double(op1) / bp_bigint_to_double(op2));
        }
    }
}

BPLONG bp_math_idiv(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return MAKEINT((BPLONG)(INTVAL(op1) / op2));
        } else if (IS_BIGINT(op2)) {
            return BP_ZERO;
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            int sign;
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            sign = bp_sign_bigint(op1);
            op1 = bp_abs_bigint(op1);
            if (op2 < 0) {
                sign = -sign;
                op2 = -op2;
            }
            op1 = bp_div_bigint_bigint(op1, bp_int_to_bigint(op2));
            if (ISINT(op1)) {
                return (sign == 1) ? op1 : MAKEINT(-INTVAL(op1));
            } else {
                return (sign == 1) ? op1 : bp_neg_bigint(op1);
            }
        } else if (IS_BIGINT(op2)) {
            int sign;
            sign = bp_sign_bigint(op1)*bp_sign_bigint(op2);
            op1 = bp_abs_bigint(op1);
            op2 = bp_abs_bigint(op2);
            op1 = bp_div_bigint_bigint(op1, op2);
            if (ISINT(op1)) {
                return (sign == 1) ? op1 : MAKEINT(-INTVAL(op1));
            } else {
                return (sign == 1) ? op1 : bp_neg_bigint(op1);
            }
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

/* op1 div op2 = (op1 - (op1 mod op2))/op2 */
BPLONG bp_math_idiv_div(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            op1 = INTVAL(op1);
            if (op1 > 0 && op2 > 0) {
                return MAKEINT(op1/op2);
            }
            if (BP_IN_28B_INT_RANGE(op1) && BP_IN_28B_INT_RANGE(op2)) {
                BPLONG m;
                double f;
                f = (double)op1/(double)op2;
                m = (BPLONG)f;
                if (f < m) {  /* floor */
                    m--;
                }
                return MAKEINT(m);
            } else {
                return bp_div_bigint_bigint(bp_int_to_bigint(op1), bp_int_to_bigint(op2));
            }
        } else if (IS_BIGINT(op2)) {
            return bp_div_bigint_bigint(bp_int_to_bigint(INTVAL(op1)), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return bp_div_bigint_bigint(op1, bp_int_to_bigint(op2));
        } else if (IS_BIGINT(op2)) {
            return bp_div_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1); return BP_ERROR;
    }
}

BPLONG bp_math_mod(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    double f;
    BPLONG i;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            if (op1 > 0 && op2 > 0) {
                return MAKEINT(op1%op2);
            }
            if (BP_IN_28B_INT_RANGE(op1) && BP_IN_28B_INT_RANGE(op2)) {
                f = (double)op1/(double)op2;
                i = (BPLONG)f;
                if (f < i) {  /* floor */
                    i--;
                }
                return MAKEINT(op1-i*op2);
            } else {
                return bp_mod_bigint_bigint(bp_int_to_bigint(op1), bp_int_to_bigint(op2));
            }
        } else if (IS_BIGINT(op2)) {
            return bp_mod_bigint_bigint(bp_int_to_bigint(op1), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
#ifdef M64BITS
            i = bp_bigint_to_native_long(op1);
            if (i != 0) return MAKEINT(i%op2);
#endif
            return bp_mod_bigint_bigint(op1, bp_int_to_bigint(op2));
        } else if (IS_BIGINT(op2)) {
            return bp_mod_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_math_rem(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            return MAKEINT((BPLONG)(op1 % op2));
        } else if (IS_BIGINT(op2)) {
            int sign;

            if (op1 < 0) {
                sign = -1;
                op1 = -op1;
            } else {
                sign = 1;
            }
            op2 = bp_abs_bigint(op2);
            op1 = bp_mod_bigint_bigint(bp_int_to_bigint(op1), op2);
            if (sign > 0) {
                return op1;
            } else {
                if (ISINT(op1)) {
                    return MAKEINT(-INTVAL(op1));
                } else
                    return bp_neg_bigint(op1);
            }
        } else {
            bp_exception = c_type_error(et_INTEGER, op1);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            int sign;

            op2 = INTVAL(op2);
            if (op2 == 0) {
                bp_exception = et_ZERO_DIVISOR; return BP_ERROR;
            }
            if (op2 < 0) {
                op2 = -op2;
            }
            sign = bp_sign_bigint(op1);
            if (sign < 0) op1 = bp_neg_bigint(op1);
            op1 = bp_mod_bigint_bigint(op1, bp_int_to_bigint(op2));
            if (sign == -1) {
                if (ISINT(op1)) {
                    return MAKEINT(-INTVAL(op1));
                } else {
                    return bp_neg_bigint(op1);
                }
            } else {
                return op1;
            }
        } else if (IS_BIGINT(op2)) {
            int sign = bp_sign_bigint(op1);
            op1 = bp_abs_bigint(op1);
            op2 = bp_abs_bigint(op2);
            op1 = bp_mod_bigint_bigint(op1, op2);
            if (sign == -1) {
                if (ISINT(op1)) {
                    return MAKEINT(-INTVAL(op1));
                } else {
                    return bp_neg_bigint(op1);
                }
            } else {
                return op1;
            }
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_bitwise_and(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op1 = op1 & INTVAL(op2);
            return (BP_IN_1W_INT_RANGE(op1)) ? MAKEINT(op1) : bp_int_to_bigint(op1);
        } else if (IS_BIGINT(op2)) {
            return bp_and_bigint_bigint(bp_int_to_bigint(op1), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_and_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_and_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_bitwise_or(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op1 = op1 | INTVAL(op2);
            return (BP_IN_1W_INT_RANGE(op1)) ? MAKEINT(op1) : bp_int_to_bigint(op1);
        } else if (IS_BIGINT(op2)) {
            return bp_or_bigint_bigint(bp_int_to_bigint(op1), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_or_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_or_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_bitwise_shiftl(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 < 0) return bp_bitwise_shiftr(op1, MAKEINT(-op2));
        if (op2 == 0) return op1;
        if (ISINT(op1)) {
            op1 = INTVAL(op1);
            if (op1 == 0) return BP_ZERO;
            if (BP_IN_TINYINT_RANGE(op1) && op2 <= 11) {
                return MAKEINT(op1 << op2);
            } else {
                return bp_shiftl_bigint_int(bp_int_to_bigint(op1), op2);
            }
        } else if (IS_BIGINT(op1)) {
            return bp_shiftl_bigint_int(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op1);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op2)) {
        if (op1 == BP_ZERO) return BP_ZERO;
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    } else {
        bp_exception = c_type_error(et_INTEGER, op2);
        return BP_ERROR;
    }
}


BPLONG bp_bitwise_shiftr(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 == 0) return op1;
        if (op2 < 0) return bp_bitwise_shiftl(op1, MAKEINT(-op2));
        if (ISINT(op1)) {
            op1 = INTVAL(op1);
            if (BP_IN_28B_INT_RANGE(op1)) {
                return MAKEINT(op1 >> op2);
            } else {
                return bp_shiftr_bigint_int(bp_int_to_bigint(op1), op2);
            }
        } else if (IS_BIGINT(op1)) {
            return bp_shiftr_bigint_int(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op1);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op2)) {
        return BP_ZERO;
    } else {
        bp_exception = c_type_error(et_INTEGER, op2);
        return BP_ERROR;
    }
}


BPLONG bp_bitwise_complement(op1)
    BPLONG op1;
{

    BPLONG_PTR top;

    DEREF(op1);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = ~INTVAL(op1);
        if (BP_IN_1W_INT_RANGE(op1)) {
            return MAKEINT(op1);
        } else {
            return bp_int_to_bigint(op1);
        }
    } else if (IS_BIGINT(op1)) {
        if (bp_sign_bigint(op1) > 0) {
            return bp_neg_bigint(bp_add_bigint_bigint(op1, bp_int_to_bigint(1)));
        } else {
            return bp_sub_bigint_bigint(bp_neg_bigint(op1), bp_int_to_bigint(1));
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_bitwise_xor(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    DEREF(op1); DEREF(op2);

    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            op1 = INTVAL(op2) ^ INTVAL(op1);
            if (BP_IN_1W_INT_RANGE(op1)) {
                return MAKEINT(op1);
            } else {
                return bp_int_to_bigint(op1);
            }
        } else if (IS_BIGINT(op2)) {
            return bp_xor_bigint_bigint(bp_int_to_bigint(INTVAL(op1)), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_xor_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_xor_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

BPLONG bp_math_integer(op1)
    BPLONG op1;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1) || IS_BIGINT(op1)) {
        return op1;
    } else {
        double f, intf;
        f = floatval(op1);
        TEST_NAN(f, op1);
        f = modf(f, &intf);
        if (BP_IN_1W_INT_RANGE(intf)) {
            return MAKEINT((BPLONG)intf);
        } else {
            return bp_double_to_bigint(intf);
        }
    }
}

BPLONG bp_math_sign(op1)
    BPLONG op1;
{
    double f;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (op1 > 0) {
            return BP_ONE;
        } else if (op1 == 0) {
            return BP_ZERO;
        } else {
            return BP_MONE;
        }
    } else if (IS_FLOAT_PSC(op1)) {
        f = floatval(op1);
        if (f > 0) {
            return BP_ONE;
        } else if (f == 0.0) {
            return BP_ZERO;
        } else {
            return BP_MONE;
        }
    } else {
        int sign = bp_sign_bigint(op1);
        return (sign > 0) ? BP_ONE : BP_MONE;
    }
}

int b_FLOAT_SIGN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_sign(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_f_atom(op2, op1);
    return BP_TRUE;
}

BPLONG bp_math_max(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    double f1, f2;
    BPLONG i1, i2;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        i1 = INTVAL(op1);
        if (ISINT(op2)) {
            i2 = INTVAL(op2);
            return (i1 > i2) ? op1 : op2;
        } else if (IS_FLOAT_PSC(op2)) {
            f2 = floatval(op2);
            return (i1 > f2) ? op1 : op2;
        } else {
            int sign = bp_sign_bigint(op2);
            return (sign < 0) ? op1 : op2;
        }
    } else if (IS_FLOAT_PSC(op1)) {
        f1 = floatval(op1);
        if (ISINT(op2)) {
            i2 = INTVAL(op2);
            return (f1 > i2) ? op1 : op2;
        } else if (IS_FLOAT_PSC(op2)) {
            f2 = floatval(op2);
            return (f1 > f2) ? op1 : op2;
        } else {
            f2 = bp_bigint_to_double(op2);
            return (f1 > f2) ? op1 : op2;
        }
    } else {
        if (ISINT(op2)) {
            return (bp_sign_bigint(op1) > 0) ? op1 : op2;
        } else if (IS_FLOAT_PSC(op2)) {
            f1 = bp_bigint_to_double(op1);
            f2 = floatval(op2);
            return (f1 > f2) ? op1 : op2;
        } else {
            int res = bp_compare_bigint_bigint(op1, op2);
            return (res >= 0) ? op1 : op2;
        }
    }
}

BPLONG bp_math_min(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    double f1, f2;
    BPLONG i1, i2;

    DEREF(op1);
    DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        i1 = INTVAL(op1);
        if (ISINT(op2)) {
            i2 = INTVAL(op2);
            return (i1 > i2) ? op2 : op1;
        } else if (IS_FLOAT_PSC(op2)) {
            f2 = floatval(op2);
            return (i1 > f2) ? op2 : op1;
        } else {
            int sign = bp_sign_bigint(op2);
            return (sign < 0) ? op2 : op1;
        }
    } else if (IS_FLOAT_PSC(op1)) {
        f1 = floatval(op1);
        if (ISINT(op2)) {
            i2 = INTVAL(op2);
            return (f1 > i2) ? op2 : op1;
        } else if (IS_FLOAT_PSC(op2)) {
            f2 = floatval(op2);
            return (f1 > f2) ? op2 : op1;
        } else {
            f2 = bp_bigint_to_double(op2);
            return (f1 > f2) ? op2 : op1;
        }
    } else {
        if (ISINT(op2)) {
            return (bp_sign_bigint(op1) > 0) ? op2 : op1;
        } else if (IS_FLOAT_PSC(op2)) {
            f1 = bp_bigint_to_double(op1);
            f2 = floatval(op2);
            return (f1 > f2) ? op2 : op1;
        } else {
            int res = bp_compare_bigint_bigint(op1, op2);
            return (res >= 0) ? op2 : op1;
        }
    }
}


BPLONG bp_math_min1(op1)
    BPLONG op1;
{
    BPLONG min_elm, cur_elm, op0;
    BPLONG_PTR top, ptr;

    DEREF(op1);
    op0 = op1;
    if (!ISLIST(op1)) {
        bp_exception = c_type_error(et_LIST, op1);
        return BP_ERROR;
    }
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    min_elm = FOLLOW(ptr); DEREF(min_elm);
    op1 = FOLLOW(ptr+1); DEREF(op1);
    if (!ISNUM(min_elm)) {
        min_elm = eval_arith(min_elm);
        if (min_elm == BP_ERROR) return BP_ERROR;
    }
    while (ISLIST(op1)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        cur_elm = FOLLOW(ptr);
        min_elm = bp_math_min(min_elm, cur_elm);
        if (min_elm == BP_ERROR) return BP_ERROR;
        op1 = FOLLOW(ptr+1); DEREF(op1);
    }
    if (!ISNIL(op1)) {
        bp_exception = c_type_error(et_LIST, op0);
        return BP_ERROR;
    }
    return min_elm;
}

BPLONG bp_math_max1(op1)
    BPLONG op1;
{
    BPLONG max_elm, cur_elm, op0;
    BPLONG_PTR top, ptr;

    DEREF(op1);
    op0 = op1;
    if (!ISLIST(op1)) {
        bp_exception = c_type_error(et_LIST, op1);
        return BP_ERROR;
    }
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    max_elm = FOLLOW(ptr); DEREF(max_elm);
    op1 = FOLLOW(ptr+1); DEREF(op1);
    if (!ISNUM(max_elm)) {
        max_elm = eval_arith(max_elm);
        if (max_elm == BP_ERROR) return BP_ERROR;
    }
    while (ISLIST(op1)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        cur_elm = FOLLOW(ptr);
        max_elm = bp_math_max(max_elm, cur_elm);
        if (max_elm == BP_ERROR) return BP_ERROR;
        op1 = FOLLOW(ptr+1); DEREF(op1);
    }
    if (!ISNIL(op1)) {
        bp_exception = c_type_error(et_LIST, op0);
        return BP_ERROR;
    }
    return max_elm;
}

BPLONG bp_math_sum1(op1)
    BPLONG op1;
{
    BPLONG sum, cur_elm, op0;
    BPLONG_PTR top, ptr;


    sum = BP_ZERO;
    DEREF(op1);
    op0 = op1;
    if (b_IS_ARRAY_c(op1)) {
        SYM_REC_PTR sym_ptr;
        int i;
        if (ISATOM(op1)) return sum;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
        i = GET_ARITY(sym_ptr);
        while (i > 0) {
            cur_elm = FOLLOW(ptr+i);
            sum = bp_math_add(sum, cur_elm);
            if (sum == BP_ERROR) return BP_ERROR;
            i--;
        }
    } else {
        while (ISLIST(op1)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            cur_elm = FOLLOW(ptr);
            sum = bp_math_add(sum, cur_elm);
            if (sum == BP_ERROR) return BP_ERROR;
            op1 = FOLLOW(ptr+1); DEREF(op1);
        }
        if (!ISNIL(op1)) {
            bp_exception = c_type_error(et_LIST, op0);
            return BP_ERROR;
        }
    }
    return sum;
}

BPLONG bp_math_prod1(op1)
    BPLONG op1;
{
    BPLONG prod, cur_elm, op0;
    BPLONG_PTR top, ptr;


    prod = BP_ONE;
    DEREF(op1);
    op0 = op1;
    if (b_IS_ARRAY_c(op1)) {
        SYM_REC_PTR sym_ptr;
        int i;
        if (ISATOM(op1)) return prod;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
        i = GET_ARITY(sym_ptr);
        while (i > 0) {
            cur_elm = FOLLOW(ptr+i);
            prod = bp_math_mul(prod, cur_elm);
            if (prod == BP_ERROR) return BP_ERROR;
            i--;
        }
    } else {
        while (ISLIST(op1)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            cur_elm = FOLLOW(ptr);
            prod = bp_math_mul(prod, cur_elm);
            if (prod == BP_ERROR) return BP_ERROR;
            op1 = FOLLOW(ptr+1); DEREF(op1);
        }
        if (!ISNIL(op1)) {
            bp_exception = c_type_error(et_LIST, op0);
            return BP_ERROR;
        }
    }
    return prod;
}


BPLONG bp_math_fract_part(op1)
    BPLONG op1;
{
    double f, intf;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f);
    f = modf(f, &intf);
    return encodefloat1(f);
}

BPLONG bp_math_int_part(op1)
    BPLONG op1;
{
    double f, intf;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f);
    f = modf(f, &intf);
    return encodefloat1(intf);
}


BPLONG bp_math_random0() {
    return MAKEINT(rand());
}

BPLONG bp_math_random1(op1)
    BPLONG op1;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (ISINT(op1)) {
        op1 = INTVAL(op1);
    } else if (IS_FLOAT_PSC(op1)) {
        op1 = (BPLONG)floatval(op1);
    } else {
        op1 = bp_bigint_to_int(op1);
    }
    srand(op1);
    return MAKEINT(rand());
}

BPLONG bp_gcd_int_int(BPLONG i1, BPLONG i2) {
    BPLONG temp;
    if (i1 < 0) i1 = -i1;
    if (i2 < 0) i2 = -i2;
    if (i1 > i2) {
        temp = i1;
        i1 = i2;
        i2 = temp;
    }
    while (i1 != 0) {
        temp = i2 % i1;
        i2 = i1;
        i1 = temp;
    }
    return i2;
}

BPLONG bp_math_gcd(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    DEREF(op2);
    if (!ISINT(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISINT(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            return MAKEINT(bp_gcd_int_int(INTVAL(op1), INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_gcd_bigint_bigint(bp_int_to_bigint(INTVAL(op1)), op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else if (IS_BIGINT(op1)) {
        if (ISINT(op2)) {
            return bp_gcd_bigint_bigint(op1, bp_int_to_bigint(INTVAL(op2)));
        } else if (IS_BIGINT(op2)) {
            return bp_gcd_bigint_bigint(op1, op2);
        } else {
            bp_exception = c_type_error(et_INTEGER, op2);
            return BP_ERROR;
        }
    } else {
        bp_exception = c_type_error(et_INTEGER, op1);
        return BP_ERROR;
    }
}

int b_RAND_MAX_f(rand_max)
    BPLONG rand_max;
{
    return unify(rand_max, MAKEINT(RAND_MAX));
}

int b_GCD_ccf(op1, op2, gcd)
    BPLONG op1, op2, gcd;
{
    op1 = bp_math_gcd(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(gcd, op1);
    return BP_TRUE;
}

/* originally in float.c */
int equal_to(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    /*  printf("compre  %x %x",op1,op2); write_term(op1); printf(" "); write_term(op2); printf("\n"); */

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            return (op1 == op2) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {  /* op1 is int, op2 is not int */
            return prettymuch_equal((double)INTVAL(op1), floatval(op2));
        } else {
            return BP_FALSE;
        }
    } if (IS_FLOAT_PSC(op1)) {  /* op1 is not int */
        if (ISINT(op2)) {
            return prettymuch_equal(floatval(op1), (double)INTVAL(op2));
        } else if (IS_FLOAT_PSC(op2)) {  /* op1 and op2 are both floats */
            return prettymuch_equal(floatval(op1), floatval(op2));
        } else {
            return prettymuch_equal(floatval(op1), bp_bigint_to_double(op2));
        }
    } else {
        if (ISINT(op2)) {
            return BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return prettymuch_equal(bp_bigint_to_double(op1), floatval(op2));
        } else {
            return (bp_compare_bigint_bigint(op1, op2) == 0) ? BP_TRUE : BP_FALSE;
        }
    }
}

int greater_than(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            return (INTVAL(op1) > INTVAL(op2)) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return ((double)INTVAL(op1) > floatval(op2)) ? BP_TRUE : BP_FALSE;
        } else {
            return (bp_sign_bigint(op2) == -1) ? BP_TRUE : BP_FALSE;
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            return (floatval(op1) > (double)INTVAL(op2)) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return (floatval(op1) > floatval(op2)) ? BP_TRUE : BP_FALSE;
        } else {
            return (floatval(op1) > bp_bigint_to_double(op2)) ? BP_TRUE : BP_FALSE;
        }
    } else {
        if (ISINT(op2)) {
            return (bp_sign_bigint(op1) == 1) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return (bp_bigint_to_double(op1) > floatval(op2)) ? BP_TRUE : BP_FALSE;
        } else {
            return (bp_compare_bigint_bigint(op1, op2) == 1) ? BP_TRUE : BP_FALSE;
        }
    }
}

int greater_equal(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        if (ISINT(op2)) {
            return (INTVAL(op1) >= INTVAL(op2)) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return ((double)INTVAL(op1) >= floatval(op2)) ? BP_TRUE : BP_FALSE;
        } else {
            return (bp_sign_bigint(op2) == -1) ? BP_TRUE : BP_FALSE;
        }
    } else if (IS_FLOAT_PSC(op1)) {
        if (ISINT(op2)) {
            return (floatval(op1) >= (double)INTVAL(op2)) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return (floatval(op1) >= floatval(op2));
        } else {
            return (floatval(op1) >= bp_bigint_to_double(op2)) ? BP_TRUE : BP_FALSE;
        }
    } else {
        if (ISINT(op2)) {
            return (bp_sign_bigint(op1) == 1) ? BP_TRUE : BP_FALSE;
        } else if (IS_FLOAT_PSC(op2)) {
            return (bp_bigint_to_double(op1) >= floatval(op2)) ? BP_TRUE : BP_FALSE;
        } else {
            return (bp_compare_bigint_bigint(op1, op2) >= 0) ? BP_TRUE : BP_FALSE;
        }
    }
}

int b_FLOAT_MINUS_cf(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (IS_FLOAT_PSC(op1)) {
        op1 = encodefloat1(-floatval(op1));
    } else {
        bp_exception = float_format_expected;
        return BP_ERROR;
    }
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_log(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    if (f <= (double)0.0) {
        bp_exception = c_domain_error(et_NUMBER, op1);
        return BP_ERROR;
    }
    return encodefloat1(log(f));
}

int b_FLOAT_LOG_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_log(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_log2(BPLONG op1, BPLONG op2) {
    BPLONG_PTR top;
    double f1, f2;


    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    DEREF(op2);
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f1);
    BP_DOUBLE_VAL(op2, f2);
    if (f1 <= (double)0.0 || f2 <= (double)0.0) {
        bp_exception = c_domain_error(et_NUMBER, make_struct2("log2", op1, op2));
        return BP_ERROR;
    }
    return encodefloat1(log10(f2)/log10(f1));
}

int b_FLOAT_LOG2_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_float_log2(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op3, op1);
    return BP_TRUE;
}


/*  base**ex, ex>0 */
BPLONG bp_pow_int_int(BPLONG base, BPLONG ex) {
    BPLONG result;
    int sign;
    //  unsigned long long tmp;
    double tmp;

    if (base == 0) {
        return (ex == 0) ? BP_ONE : BP_ZERO;
    } else if (base < 0) {
        base = -base;
        sign = (ex % 2 == 0) ? 1 : -1;
    } else {
        sign = 1;
    }
    if (base == 1) return (sign == 1) ? BP_ONE : BP_MONE;

    result = 1;
    for (; ; ) {
        if (ex & 1) {
            tmp = (double)result*(double)base;
#ifdef M64BITS
            if (!BP_IN_28B_INT_RANGE(tmp))  // avoid overflow
                return BP_ERROR;
#else
            if (!BP_IN_14B_INT_RANGE(tmp))  // avoid overflow
                return BP_ERROR;
#endif
            result = (BPLONG)tmp;
        }
        ex >>= 1;
        if (ex == 0) break;
        tmp = (double)base*(double)base;
#ifdef M64BITS
        if (!BP_IN_28B_INT_RANGE(tmp))
            return BP_ERROR;
#else
        if (!BP_IN_14B_INT_RANGE(tmp))
            return BP_ERROR;
#endif
        base = (BPLONG)tmp;
    }
    return (sign == 1) ? MAKEINT(result) : MAKEINT(-result);
}


BPLONG bp_math_pow(op1, op2)
    BPLONG op1, op2;
{
    double f1, f2, res;
    BPLONG_PTR top;

    DEREF(op1); DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 >= 0) {
                BPLONG res = bp_pow_int_int(op1, op2);
                if (res == BP_ERROR) res = bp_pow_bigint_int(bp_int_to_bigint(op1), op2);
                return res;
            } else {  /* op2<=0 */
                f1 = (double)op1; f2 = (double)op2;
            }
        } else if (IS_FLOAT_PSC(op2)) {
            f1 = (double)op1; f2 = floatval(op2);
        } else {
            if (bp_sign_bigint(op2) > 0) {
                if (op1 == 0) return BP_ZERO;
                if (op1 == 1) return BP_ONE;
                if (op1 == -1) return (bp_and_bigint_bigint(op2, bp_int_to_bigint(1)) == BP_ZERO) ? BP_ONE : BP_MONE;
                bp_exception = et_OUT_OF_MEMORY;
                return BP_ERROR;
            } else {
                f1 = (double)op1;
                f2 = bp_bigint_to_double(op2);
            }
        }
    } else if (IS_FLOAT_PSC(op1)) {
        f1 = floatval(op1);
        BP_DOUBLE_VAL(op2, f2);
    } else {
        if (ISINT(op2)) {
            op2 = INTVAL(op2);
            if (op2 > 0) {
                return bp_pow_bigint_int(op1, op2);
            } else {
                f1 = bp_bigint_to_double(op1);
                f2 = (double)op2;
            }
        } else if (IS_FLOAT_PSC(op2)) {
            f1 = bp_bigint_to_double(op1);
            f2 = floatval(op2);
        } else {
            if (bp_sign_bigint(op2) > 0) {
                bp_exception = et_OUT_OF_MEMORY;
                return BP_ERROR;
            } else {
                f1 = bp_bigint_to_double(op1);
                f2 = bp_bigint_to_double(op2);
            }
        }
    }
    errno = 0;
    res = pow(f1, f2);
    if (errno > 0) {
        bp_exception = c_domain_error(et_NUMBER, make_struct2("**", encodefloat1(f1), encodefloat1(f2)));
        return BP_ERROR;
    }
    return encodefloat1(res);
}

int b_FLOAT_POW_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_math_pow(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op3, op1);
    return BP_TRUE;
}

int b_REM_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_math_rem(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op3, op1);
    return BP_TRUE;
}

BPLONG bp_float_sqrt(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);

    if (f >= (double)-0.0) {
        return encodefloat1(sqrt(f));
    } else {
        bp_exception = c_domain_error(et_NUMBER, op1);
        return BP_ERROR;
    }
}

int b_FLOAT_SQRT_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_sqrt(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_abs(BPLONG op1) {

    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1)) {
        BPLONG i;
        i = INTVAL(op1);
        if (i < 0) return MAKEINT(-i); else return op1;
    } else if (IS_FLOAT_PSC(op1)) {
        double f;
        f = floatval(op1);
        if (f < 0.0) return encodefloat1(-f); else return op1;
    } else {
        int sign;
        sign = bp_sign_bigint(op1);
        if (sign < 0) return bp_neg_bigint(op1); else return op1;
    }
}

int b_FLOAT_ABS_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_abs(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_exp(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(exp(f));
}

int b_FLOAT_EXP_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_exp(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_sin(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(sin(f));
}

int b_FLOAT_SIN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_sin(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_cos(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(cos(f));
}

int b_FLOAT_COS_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_cos(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_tan(BPLONG op1) {
    BPLONG_PTR top;
    double f;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(tan(f));
}


int b_FLOAT_TAN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_tan(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_atan(BPLONG op1) {
    double f;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(atan(f));
}

int b_FLOAT_ATAN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_atan(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_atan2(op1, op2)
    BPLONG op1, op2;
{
    double f1, f2;
    BPLONG_PTR top;

    DEREF(op1);
    DEREF(op2);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) {
            return BP_ERROR;
        }
    }
    if (!ISNUM(op2)) {
        op2 = eval_arith(op2);
        if (op2 == BP_ERROR) {
            return BP_ERROR;
        }
    }
    BP_DOUBLE_VAL(op1, f1);
    BP_DOUBLE_VAL(op2, f2);
    return encodefloat1(atan2(f1, f2));
}

BPLONG bp_float_asin(BPLONG op1) {
    double f;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    if (f > (double)1.0 || f < (double)-1.0) {
        bp_exception = c_domain_error(et_NUMBER, op1);
        return BP_ERROR;
    }
    return encodefloat1(asin(f));
}

int b_FLOAT_ASIN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_asin(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_acos(BPLONG op1) {
    double f;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    BP_DOUBLE_VAL(op1, f);
    if (f > (double)1.0 || f < (double)-1.0) {
        bp_exception = c_domain_error(et_NUMBER, op1);
        return BP_ERROR;
    }
    return encodefloat1(acos(f));
}

int b_FLOAT_ACOS_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_acos(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}


int b_FLOAT_WRITE_c(op)
    BPLONG op;
{
    BPLONG_PTR top;

    DEREF(op);
    fprintf(curr_out, "%g", floatval(op));
    return 1;
}

BPLONG bp_float_floor(op1)
    BPLONG op1;
{
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (ISINT(op1) || IS_BIGINT(op1)) {
        return op1;
    } else {
        double f, intf;

        f = floatval(op1);
        TEST_NAN(f, op1);
        modf(f, &intf);
        //              printf("%f %f \n", f, intf);
        if (intf > f) {
            intf -= 1;
        }
        if (BP_IN_1W_INT_RANGE(intf)) {
            return MAKEINT((BPLONG)intf);
        } else {
            return bp_double_to_bigint(intf);
        }
    }
}

int b_FLOAT_FLOOR_cf(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
	op1 = bp_float_floor(op1);
	if (op1 == BP_ERROR)
	  return BP_ERROR;
    ASSIGN_f_atom(op2, op1);
	return 1;
}


BPLONG bp_float_float(BPLONG op1) {
    double f;
    BPLONG_PTR top;

    DEREF(op1);
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    BP_DOUBLE_VAL(op1, f);
    return encodefloat1(f);
}

int b_FLOAT_FLOAT_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_float(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_round(op1)
    BPLONG op1;
{
    double f;

    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1) || IS_BIGINT(op1)) {
        return op1;
    } else {
        int sign;
        f = floatval(op1);
#ifdef M64BITS
        f = roundl(f);
#else
        if (f < 0.0) {
            f = -f+0.5;
            sign = -1;
        } else {
            f = f+0.5;
            sign = 1;
        }
        TEST_NAN(f, op1);
        modf(f, &f);
#endif
        if (BP_IN_1W_INT_RANGE(f)) {
            return MAKEINT((BPLONG)f);
        } else {
            return bp_double_to_bigint(f);
        }
    }
}

int b_FLOAT_ROUND_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_float_round(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_f_atom(op2, op1);
    return BP_TRUE;
}

BPLONG bp_float_truncate(op1)
    BPLONG op1;
{
    double f;
    DEREF(op1);

    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }

    if (ISINT(op1) || IS_BIGINT(op1)) {
        return op1;
    } else {
        f = floatval(op1);
        TEST_NAN(f, op1);
        modf(f, &f);
        if (BP_IN_1W_INT_RANGE(f)) {
            return MAKEINT((BPLONG)f);
        } else {
            return bp_double_to_bigint(f);
        }
    }
}

int b_FLOAT_TRUNCATE_cf(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    DEREF(op1);
    op1 = bp_float_truncate(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_f_atom(op2, op1);
    return BP_TRUE;
}


BPLONG bp_float_ceiling(op1)
    BPLONG op1;
{
    if (!ISNUM(op1)) {
        op1 = eval_arith(op1);
        if (op1 == BP_ERROR) return BP_ERROR;
    }
    if (ISINT(op1) || IS_BIGINT(op1)) {
        return op1;
    } else {
        double f, intf;

        f = floatval(op1);
        TEST_NAN(f, op1);
        modf(f, &intf);
        if (f > intf) {
            intf += 1;
        }
        if (BP_IN_1W_INT_RANGE(intf)) {
            return MAKEINT((BPLONG)intf);
        } else {
            return bp_double_to_bigint(intf);
        }
    }
}

int b_FLOAT_CEILING_cf(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    op1 = bp_float_ceiling(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_f_atom(op2, op1);
    return BP_TRUE;
}


int b_XOR_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_bitwise_xor(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_f_atom(op3, op1);
    return BP_TRUE;
}

int b_FLOAT_PI_f(op1)
    BPLONG op1;
{
    BPLONG_PTR top;
    DEREF(op1);
    ASSIGN_sv_heap_term(op1, encodefloat1(3.141592653589793));
    return 1;
}

int b_FLOAT_E_f(op1)
    BPLONG op1;
{
    BPLONG_PTR top;
    DEREF(op1);
    ASSIGN_sv_heap_term(op1, encodefloat1(2.7182818284590455));
    return 1;
}

int b_RANDOM_f(op1)
    BPLONG op1;
{
    BPLONG_PTR top;
    DEREF(op1);
    ASSIGN_sv_heap_term(op1, MAKEINT(rand()));
    return 1;
}

int b_RANDOM_cf(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR top;
    op1 = bp_math_random1(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    DEREF(op2);
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_MAX_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_math_max(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op3, op1);
    return 1;
}


int b_MIN_ccf(op1, op2, op3)
    BPLONG op1, op2, op3;
{
    op1 = bp_math_min(op1, op2);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op3, op1);
    return 1;
}

int b_MAX_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_max1(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_MIN_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_min1(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_SUM_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_sum1(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_PROD_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_prod1(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_FLOAT_FRACT_PART_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_fract_part(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int b_FLOAT_INT_PART_cf(op1, op2)
    BPLONG op1, op2;
{
    op1 = bp_math_int_part(op1);
    if (op1 == BP_ERROR) return BP_ERROR;
    ASSIGN_sv_heap_term(op2, op1);
    return 1;
}

int prettymuch_equal(op1, op2)
    double op1, op2;
{
    double min, diff;

    if ((op1 <= -EPSILON && op2 >= EPSILON) || (op2 <= -EPSILON && op1 >= EPSILON))
        return 0;
    if (op1 < 0.0)
        op1 = -op1;
    if (op2 < 0.0)
        op2 = -op2;
    diff = op1 - op2;
    if (diff < 0.0)
        diff = -diff;
    min = (op1 < op2) ? op1 : op2;
    if (min == 0.0)
        return (op1 == op2);
    else
        return ((diff/min) < EPSILON) ? 1 : 0;
}

BPLONG bp_access_array(arr, indexes)
    BPLONG arr, indexes;
{
    /*  write_term(arr);write_term(indexes); printf("\n"); */

    DEREF(indexes);
    if (ISATOM(indexes)) {  /* length? */
        return bp_access_one_array(arr, indexes);
    }
    if (ISSTRUCT(indexes)) {  /* A^..^..^ */
        BPLONG_PTR ptr;
        BPLONG index;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(indexes);
        if (FOLLOW(ptr) != (BPLONG)cap_psc) {
            bp_exception = illegal_arguments;
            return BP_ERROR;
        }
        index = FOLLOW(ptr+1);
        arr = bp_access_array(arr, index);
        if (arr == BP_ERROR) return BP_ERROR;
        indexes = FOLLOW(ptr+2);
        return bp_access_array(arr, indexes);
    }
    if (ISLIST(indexes)) {  /* A^[...] */
        BPLONG_PTR ptr;
        BPLONG index;
        do {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(indexes);
            index = FOLLOW(ptr);
            arr = bp_access_one_array(arr, index);
            if (arr == BP_ERROR) return BP_ERROR;
            indexes = FOLLOW(ptr+1); DEREF(indexes);
        } while (ISLIST(indexes));
        return arr;
    }
    bp_exception = illegal_arguments;
    return BP_ERROR;
}

BPLONG bp_access_one_array(arr, index)
    BPLONG arr, index;
{
    BPLONG_PTR arr_ptr;
    BPLONG arity;

    DEREF(arr);
    DEREF(index);
    if (!ISINT(index) && index != atom_length) {
        index = eval_arith(index);
        if (index == BP_ERROR) return BP_ERROR;
    }
    if (ISATOM(arr)) {
        if (index == atom_length) {
            return BP_ZERO;
        }
        goto error_end;
    }
    if (ISSTRUCT(arr)) {
        SYM_REC_PTR sym_ptr;
        arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(arr);
        if ((BPLONG)arr_ptr == (BPLONG)cap_psc) {  /* A^indexes^index */
            arr = bp_access_array(FOLLOW(arr_ptr+1), FOLLOW(arr_ptr+2));
            return bp_access_one_array(arr, index);
        }

        sym_ptr = (SYM_REC_PTR)FOLLOW(arr_ptr);
        if (index == atom_length) {
            return MAKEINT(GET_ARITY(sym_ptr));
        }
        if (!ISINT(index)) goto error_end;
        index = INTVAL(index);
        arity = GET_ARITY(sym_ptr);
        if (index <= 0 || index > arity) {
            bp_exception = out_of_bound;
            return BP_ERROR;
        }
        return FOLLOW(arr_ptr+index);
    } else if (ISLIST(arr)) {
        if (index == atom_length) {
            return MAKEINT(list_length(arr, arr));
        }
        if (!ISINT(index)) goto error_end;
        index = INTVAL(index);
        while (index > 1 && ISLIST(arr)) {
            arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(arr);
            arr = FOLLOW(arr_ptr+1); DEREF(arr);
            index--;
        }
        if (index == 1 && ISLIST(arr)) {
            arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(arr);
            return FOLLOW(arr_ptr);
        } else {
            bp_exception = out_of_bound;
            return BP_ERROR;
        }
    }
error_end:
    bp_exception = illegal_arguments;
    return BP_ERROR;
}


/************************************************************/
BPLONG eval_arith(ex)
    BPLONG ex;
{
    register char *ch_ptr;
    SYM_REC_PTR ptr;
    register BPLONG ex1, ex2;
    BPLONG res;
    BPLONG_PTR top;

    SWITCH_OP(ex, l1,
              {bp_exception = et_INSTANTIATION_ERROR; return BP_ERROR;},

              {if (ISINT(ex)) {
                      return ex;
                  } else goto eval_ex;},

              {bp_exception = c_type_error(et_EVALUABLE, ex);
                  return BP_ERROR;},

              { if (IS_FLOAT_PSC(ex)) {
                      return ex;
                  } else if (IS_BIGINT_PSC(ex)) {
                      return ex;
                  } goto eval_ex;},

              { bp_exception = et_INSTANTIATION_ERROR; return BP_ERROR;});

eval_ex:  /* ex is an exression */
    ptr = (SYM_REC_PTR)GET_SYM_REC(ex);
    ch_ptr = GET_NAME(ptr);

    switch (*ch_ptr) {
    case '~':
        if (ptr == comp_psc1) {
            ex = GET_ARG(ex, 1);
            return bp_bitwise_complement(ex);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '^':
        if (ptr == cap_psc) {
#ifdef PICAT
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_or(ex1, ex2);
#else
            BPLONG arr, indexes;

            arr = GET_ARG(ex, 1);
            indexes = GET_ARG(ex, 2);
            arr = bp_access_array(arr, indexes);
            if (arr == BP_ERROR) return BP_ERROR;
            return eval_arith(arr);
#endif
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '+':
        if (ptr == add2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_add(ex1, ex2);
        } else if (ptr == add1) {
            ex1 = GET_ARG(ex, 1);
            return eval_arith(ex1);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '-':
        if (ptr == sub2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_sub(ex1, ex2);
        } else if (ptr == sub1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_sub(BP_ZERO, ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '*':
        if (ptr == ppow2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_pow(ex1, ex2);
        }
        else if (ptr == mul2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_mul(ex1, ex2);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '/':
        if (ptr == idiv2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_idiv(ex1, ex2);
        } else if (ptr == and2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_and(ex1, ex2);
        } else if (ptr == div2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_div(ex1, ex2);
        } else if (ptr == div_ge2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_divge(ex1, ex2);
        } else if (ptr == div_le2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_divle(ex1, ex2);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'd':
        if (ptr == idiv_div) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_idiv_div(ex1, ex2);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '<':
        if (ptr == shiftl2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_shiftl(ex1, ex2);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '>':
        if (ptr == shiftr2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_shiftr(ex1, ex2);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case '\\':
        if (ptr == or2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_or(ex1, ex2);
        } else if (ptr == negation1) {
            ex1 = GET_ARG(ex, 1);
            return bp_bitwise_complement(ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'a':
        if (ptr == abs1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_abs(ex1);
        } else if (ptr == atan1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_atan(ex1);
        } else if (ptr == sym_atan2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_float_atan2(ex1, ex2);
        } else if (ptr == asin1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_asin(ex1);
        } else if (ptr == acos1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_acos(ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'c':
        if (ptr == cos1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_cos(ex1);
        } else if (ptr == ceiling1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_ceiling(ex1);
        } else if (ptr == cputime1) {
            res = (BPLONG)heap_top++;
            b_CPUTIME_f(res);
            return FOLLOW(res);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'g':
        if (ptr == gcd2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_gcd(ex1, ex2);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'e':
        if (ptr == exp1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_exp(ex1);
        } else if (ptr == e0) {
            return encodefloat1(2.7182818284590455);
        } else if (ptr == epsilon0) {
            return encodefloat1(EPSILON);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'f':
        if (ptr == float1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_float(ex1);
        } else if (ptr == floor1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_floor(ex1);
        } else if (ptr == float_fractional_part1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_fract_part(ex1);
        } else if (ptr == float_int_part1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_int_part(ex1);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'i':
        if (ptr == integer1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_integer(ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'l':
        if (ptr == log1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_log(ex1);
        } else if (ptr == log22) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_float_log2(ex1, ex2);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'm':
        if (ptr == mod2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_mod(ex1, ex2);
        } if (ptr == max2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_max(ex1, ex2);
        } if (ptr == max1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_max1(ex1);
        } else if (ptr == min2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_min(ex1, ex2);
        } else if (ptr == min1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_min1(ex1);
        } else if (ptr == maxint) {
            return MAKEINT(BP_MAXINT_1W);
        } else if (ptr == minint) {
            return MAKEINT(BP_MININT_1W);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'p':
        if (ptr == pi0) {
            return encodefloat1(3.141592653589793);
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 'r':
        if (ptr == rem2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_math_rem(ex1, ex2);
        } else if (ptr == round1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_round(ex1);
        } else if (ptr == ran1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_random1(ex1);
        } else if (ptr == ran0) {
            return bp_math_random0();
        } else {
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 's':
        if (ptr == sin1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_sin(ex1);
        }
        else if (ptr == sqrt1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_sqrt(ex1);
        } else if (ptr == sign1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_sign(ex1);
        } else if (ptr == sum1) {
            ex1 = GET_ARG(ex, 1);
            return bp_math_sum1(ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }

    case 't':
        if (ptr == tan1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_tan(ex1);
        } else if (ptr == truncate1) {
            ex1 = GET_ARG(ex, 1);
            return bp_float_truncate(ex1);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }
    case 'x':
        if (ptr == xor2) {
            ex1 = GET_ARG(ex, 1);
            ex2 = GET_ARG(ex, 2);
            return bp_bitwise_xor(ex1, ex2);
        } else{
            bp_exception = c_type_error(et_EVALUABLE, ex);
            return BP_ERROR;
        }
    default:
        bp_exception = c_type_error(et_NUMBER, ex);
        return BP_ERROR;
    }
}

/* evaluate an arithmetic exression */
int b_EVAL_ARITH_cf(ex, res)
    BPLONG ex, res;
{
    BPLONG res1;

    res1 = eval_arith(ex);
    if (res1 == BP_ERROR) {
        return BP_ERROR;
    }
    ASSIGN_sv_heap_term(res, res1);
    return BP_TRUE;
}

/* res = x*y mod z */
int c_MUL_MOD_cccf() {
    BPLONG x, y, z, res;
    BPLONG res0;
    BPLONG_PTR top;

    x = ARG(1, 4); DEREF(x);
    y = ARG(2, 4); DEREF(y);
    z = ARG(3, 4); DEREF(z);
    res = ARG(4, 4);

    //  printf("MUL_MOD "); write_term(x); printf(" "); write_term(y); printf(" "); write_term(z); printf("\n");
    if (ISINT(x) && ISINT(y) && ISINT(z)) {
        x = INTVAL(x); y = INTVAL(y); z = INTVAL(z);

        if (z == 0) {
            bp_exception = divide_by_zero;
            return BP_ERROR;
        }
        if (z > 0) {
            if (y >= z) y = y%z;
            if (x >= z) x = x%z;
        }
        if (x == 0 || y == 0) {
            res0 = BP_ZERO;
        } else {
#ifdef M64BITS
            if (BP_IN_28B_INT_RANGE(x) && BP_IN_28B_INT_RANGE(y)) {
                res0 = MAKEINT((x*y)%z);
            }
#else
            if (BP_IN_14B_INT_RANGE(x) && BP_IN_14B_INT_RANGE(x)) {
                res0 = MAKEINT((x*y)%z);
            }
#endif
            else {
                res0 = bp_mul_bigint_bigint(bp_int_to_bigint(x), bp_int_to_bigint(y));
                if (ISINT(res0)){
                    res0 = bp_int_to_bigint(INTVAL(res0));
                }
                res0 = bp_mod_bigint_bigint(res0, bp_int_to_bigint(z));
            }
        }
    } else {
        if (ISINT(x)) {
            x = INTVAL(x); x = bp_int_to_bigint(x);
        } else if (!IS_BIGINT(x)) {
            bp_exception = integer_expected;
            return BP_ERROR;
        }
        if (ISINT(y)) {
            y = INTVAL(y); y = bp_int_to_bigint(y);
        } else if (!IS_BIGINT(y)) {
            bp_exception = integer_expected;
            return BP_ERROR;
        }
        if (ISINT(z)) {
            z = INTVAL(z);
            if (z == 0) {
                bp_exception = divide_by_zero;
                return BP_ERROR;
            }
            z = bp_int_to_bigint(z);
        } else if (!IS_BIGINT(z)) {
            bp_exception = integer_expected;
            return BP_ERROR;
        }
        if (bp_compare_bigint_bigint(x, z) >= 0){
            x = bp_mod_bigint_bigint(x, z);
            if (ISINT(x)) {
                x = bp_int_to_bigint(INTVAL(x));
            }
        }
        if (bp_compare_bigint_bigint(y, z) >= 0){
            y = bp_mod_bigint_bigint(y, z);
            if (ISINT(y)) {
                y = bp_int_to_bigint(INTVAL(y));
            }
        }
        res0 = bp_mul_bigint_bigint(x, y);
        if (ISINT(res0)){
            res0 = bp_int_to_bigint(INTVAL(res0));
        }
        res0 = bp_mod_bigint_bigint(res0, z);
    }
    return unify(res, res0);
}


