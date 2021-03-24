/********************************************************************
 *   File   : float.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021
 *   Purpose: arithmetic functions 

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "term.h"
#include "bapi.h"
#include <math.h>

/* converts floats from WAM representation to
 *  the machine representation 
 */
double floatval(op)
    BPLONG op;
{
    double f;
    UW32 w1, w2, w3, *pw;

    UNTAG_ADDR(op);
    w1 = (UW32)INTVAL(*((BPLONG_PTR)op+1));
    w2 = (UW32)INTVAL(*((BPLONG_PTR)op+2));
    w3 = (UW32)INTVAL(*((BPLONG_PTR)op+3));


    pw = (UW32_PTR)&f;
#ifdef __BIG_ENDIAN__
    *pw = ((w2 << 27) | w3);
    *(pw+1) = (w1 << 22) | (w2 >> 5);
#else
    *pw = (w1 << 22) | (w2 >> 5);
    *(pw+1) = ((w2 << 27) | w3);
#endif
    return f;
}

/* "encodefloat" converts floats from the machine representation
 *             to the WAM representation.
 *  '$float'(int1,int2,int2);
 */
BPLONG encodefloat1(op)
    double op;
{
    BPLONG temp;
    UW32 *pw, w1, w2;

    temp = ADDTAG(heap_top, STR);
    NEW_HEAP_NODE((BPLONG)float_psc);  /* '$float'(int1,int2,int3) */

    pw = (UW32 *)&op;
#ifdef __BIG_ENDIAN__
    w2 = *pw;
    w1 = *(pw+1);
#else
    w1 = *pw;
    w2 = *(pw+1);
#endif
    NEW_HEAP_NODE(MAKEINT(w1 >> 22));  /* int1 */
    NEW_HEAP_NODE(MAKEINT(((w1 & 0x3fffff) << 5) | (w2 >> 27)));  /* int2 */
    NEW_HEAP_NODE(MAKEINT(w2 & 0x7ffffff));  /* int3 */
    /*
      if (arreg-1000 < heap_top) { 
      quit("control stack overflow\n");
      }
    */
    return temp;
}

