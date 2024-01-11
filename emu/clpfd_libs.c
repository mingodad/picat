/********************************************************************
 *   File   : clpfd_libs.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024
 *   Purpose: Primitives on linear constraints

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "bprolog.h"
#include "event.h"
#include "clpfd.h"

extern FILE *curr_out;

/*
  #ifdef ToamProfileFD
  int domain_updated_in_reducer;
  int n_useful_reducer=0;
  #endif
*/
#define coes_all_one_bool 1
#define coes_all_one 2
#define coes_all_mone_bool 3
#define coes_all_mone 4
#define coes_all_pos_bool 5
#define coes_all_pos 6
#define coes_all_neg_bool 7
#define coes_all_neg 8
#define coes_one_mone_bool 9
#define coes_one_mone 10
#define cv_ge_v 12

#define MAX_NUM_VARS_EQ 551
/* c+a1*x1+...+an*xn = 0 */
int nary_interval_consistent_eq(BPLONG n)
{
    register BPLONG_PTR dv_ptr, var_ptr, coe_ptr;
    register BPLONG i, first, last;
    register BPLONG a, x;
    register BPLONG_PTR top;
    BPLONG tl[MAX_NUM_VARS_EQ], tu[MAX_NUM_VARS_EQ],
        aa[MAX_NUM_VARS_EQ], xx[MAX_NUM_VARS_EQ];  /* assume no constraint contain more than MAX_NUM_VARS_EQ variables */
    BPLONG first0, last0, dv_ptr_first, dv_ptr_last, size, temp_i, c;
    int k;
    BPLONG type;

    var_ptr = arreg+n+1;
    coe_ptr = var_ptr+n;
    c = INTVAL(FOLLOW(coe_ptr));  /* c+a1*x1+...+ak*xk where x1,...,xk are integers */
    type = INTVAL(FOLLOW(coe_ptr+1));
    /*
      printf("c = %d type=%d\n",c,type);
      for (i = 1; i<=n; i++){
      write_term(FOLLOW(coe_ptr-i)); printf("*"); write_term(FOLLOW(var_ptr-i)); printf(" ");
      }
      printf("\n");
    */

    switch (type) {
    case 0:{  /* general case, overflow may occur */
        tl[0] = tu[0] = c;
        for (i = 1; i < n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a > 0) {
                if (a == 1) {
                    COMPUTE_LOW_UPPER_BOUNDS_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    COMPUTE_LOW_UPPER_BOUNDS_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    COMPUTE_LOW_UPPER_BOUNDS_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    COMPUTE_LOW_UPPER_BOUNDS_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }

        a = FOLLOW(coe_ptr-n); a = INTVAL(a);  /* T(n-1)+a*x=0 */
        x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
        if (a > 0) {
            if (a == 1) {
                REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);
            } else {
                REDUCE_LAST_TERM_DOMAIN_p(a, x, tl[n-1], tu[n-1]);
            }
        } else {
            if (a == -1) {
                REDUCE_LAST_TERM_DOMAIN_m1(x, tl[n-1], tu[n-1]);
            } else {
                a = -a;
                REDUCE_LAST_TERM_DOMAIN_m(a, x, tl[n-1], tu[n-1]);
            }
        }

        for (i = n-1; i >= 1; i--) {  /* reduce domains */
            a = aa[i];  /* FOLLOW(coe_ptr-i); a = INTVAL(a); */
            x = xx[i];  /* FOLLOW(var_ptr-i);DEREF_NONVAR(x); */
            if (a > 0) {
                if (a == 1) {
                    REDUCE_DOMAIN_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    REDUCE_DOMAIN_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    REDUCE_DOMAIN_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    REDUCE_DOMAIN_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }
        return BP_TRUE;
    }

    case coes_all_mone:  /* c-x1-...-xk  */
        c = -c;

    case coes_all_one:  /* c+x1+...+xk  */
        /* compute low and upper bounds */
        tl[0] = tu[0] = c;
        for (i = 1; i < n; i++) {  /* compute low and upper bounds */
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            xx[i] = x;
            COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
        }

        x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
        REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);

        for (i = n-1; i >= 1; i--) {  /* reduce domains */
            x = xx[i];
            REDUCE_DOMAIN_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
        }
        return BP_TRUE;

    case coes_all_one_bool:  /* c+x1+...+xk  where xi's are boolean vars */
        c = -c;

    case coes_all_mone_bool:  /* c-x1-...-xk  where xi's are boolean vars */
    {
        int nvs;

        nvs = 0;
        for (i = 1; i <= n; i++) {  /* count variables */
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ONE) {
                c--;
            } else if (x != BP_ZERO) {
                xx[nvs++] = x;
            }
        }
        /* printf("c=%d n1s=%d n0s=%d\n",c,n1s,n0s); */
        if (c <= 0) {
            if (c < 0) return BP_FALSE;
            for (i = 0; i < nvs; i++) {  /* bind remaining vars to 0 */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(xx[i]);
                ASSIGN_DVAR(dv_ptr, BP_ZERO);
            }
            KILL_SUSP_FRAME;
        } else if (c >= nvs) {
            if (c > nvs) return BP_FALSE;
            for (i = 0; i < nvs; i++) {  /* bind remaining vars to 1 */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(xx[i]);
                ASSIGN_DVAR(dv_ptr, BP_ONE);
            }
            KILL_SUSP_FRAME;
        }
        return BP_TRUE;
    }

    case coes_all_pos:
        tl[0] = tu[0] = c;
        for (i = 1; i < n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a == 1) {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }

        a = FOLLOW(coe_ptr-n); a = INTVAL(a);  /* T(n-1)+a*x=0 */
        x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
        if (a == 1) {
            REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);
        } else {
            REDUCE_LAST_TERM_DOMAIN_nooverflow_p(a, x, tl[n-1], tu[n-1]);
        }

        for (i = n-1; i >= 1; i--) {  /* reduce domains */
            a = aa[i];  /* FOLLOW(coe_ptr-i); a = INTVAL(a); */
            x = xx[i];  /* FOLLOW(var_ptr-i);DEREF_NONVAR(x); */
            if (a == 1) {
                REDUCE_DOMAIN_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                REDUCE_DOMAIN_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }
        return BP_TRUE;

    case coes_one_mone_bool: {  /* c+A1*V1+...+An*Vn = 0 where Ai=1 or -1 and Vi is bool*/
        int npvs, nnvs;
        npvs = nnvs = 0;
        for (i = 1; i <= n; i++) {  /* count variables */
            a = FOLLOW(coe_ptr-i);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ONE) {
                if (a == BP_ONE)
                    c++;
                else
                    c--;
            } else if (x != BP_ZERO) {  /* x is a domain var */
                if (a == BP_ONE)
                    npvs++;
                else
                    nnvs++;
            }
        }

        if (c >= nnvs) {  /* all outgoing arcs must be on */
            if (c > nnvs) return BP_FALSE;
            for (i = 1; i <= n; i++) {
                a = FOLLOW(coe_ptr-i);
                x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
                if (IS_SUSP_VAR(x)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
                    if (a == BP_ONE) {
                        ASSIGN_DVAR(dv_ptr, BP_ZERO);
                    } else {
                        ASSIGN_DVAR(dv_ptr, BP_ONE);
                    }
                }
            }
            KILL_SUSP_FRAME;
        } else if (c <= -npvs) {  /* all incoming arcs must be on */
            if (c < -npvs) return BP_FALSE;
            for (i = 1; i <= n; i++) {
                a = FOLLOW(coe_ptr-i);
                x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
                if (IS_SUSP_VAR(x)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
                    if (a == BP_ONE) {
                        ASSIGN_DVAR(dv_ptr, BP_ONE);
                    } else {
                        ASSIGN_DVAR(dv_ptr, BP_ZERO);
                    }
                }
            }
            KILL_SUSP_FRAME;
        }
        return BP_TRUE;
    }

    case coes_one_mone:
        tl[0] = tu[0] = c;
        for (i = 1; i < n; i++) {  /* compute low and upper bounds */
            aa[i] = FOLLOW(coe_ptr-i);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (aa[i] == BP_ONE) {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }

        a = FOLLOW(coe_ptr-n);  /* T(n-1)+a*x=0 */
        x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
        if (a == BP_ONE) {
            REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);
        } else {
            REDUCE_LAST_TERM_DOMAIN_m1(x, tl[n-1], tu[n-1]);
        }

        for (i = n-1; i >= 1; i--) {  /* reduce domains */
            x = xx[i];  /* FOLLOW(var_ptr-i);DEREF_NONVAR(x); */
            if (aa[i] == BP_ONE) {
                REDUCE_DOMAIN_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                REDUCE_DOMAIN_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }
        return 1;

    default:  /* no overflow */
        tl[0] = tu[0] = c;
        for (i = 1; i < n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a > 0) {
                if (a == 1) {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }

        a = FOLLOW(coe_ptr-n); a = INTVAL(a);  /* T(n-1)+a*x=0 */
        x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
        if (a > 0) {
            if (a == 1) {
                REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);
            } else {
                REDUCE_LAST_TERM_DOMAIN_nooverflow_p(a, x, tl[n-1], tu[n-1]);
            }
        } else {
            if (a == -1) {
                REDUCE_LAST_TERM_DOMAIN_m1(x, tl[n-1], tu[n-1]);
            } else {
                a = -a;
                REDUCE_LAST_TERM_DOMAIN_nooverflow_m(a, x, tl[n-1], tu[n-1]);
            }
        }

        for (i = n-1; i >= 1; i--) {  /* reduce domains */
            a = aa[i];  /* FOLLOW(coe_ptr-i); a = INTVAL(a); */
            x = xx[i];  /* FOLLOW(var_ptr-i);DEREF_NONVAR(x); */
            if (a > 0) {
                if (a == 1) {
                    REDUCE_DOMAIN_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    REDUCE_DOMAIN_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    REDUCE_DOMAIN_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    REDUCE_DOMAIN_nooverflow_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }
        return 1;
    }
}


#define MAX_NUM_VARS_NOCOE_EQ 6
/* c+x1+...+xn = 0 */
int nary_interval_consistent_nocoe(BPLONG n)
{
    register BPLONG_PTR dv_ptr, var_ptr;
    register BPLONG i, first, last;
    register BPLONG x;
    register BPLONG_PTR top;
    BPLONG tl[MAX_NUM_VARS_NOCOE_EQ], tu[MAX_NUM_VARS_NOCOE_EQ],
        xx[MAX_NUM_VARS_NOCOE_EQ];  /* assume no constraint contain more than 10 variables */
    BPLONG dv_ptr_first, dv_ptr_last, size, c;
    int k;

    /*
      #ifdef ToamProfileFD
      domain_updated_in_reducer = 0;
      #endif
    */
    var_ptr = arreg+n+1;
    c = INTVAL(FOLLOW(var_ptr+n));  /* c+x1+...+xk where x1,...,xk are integers */
    /* compute low and upper bounds */
    tl[0] = tu[0] = c;
    for (i = 1; i < n; i++) {  /* compute low and upper bounds */
        x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
        xx[i] = x;
        COMPUTE_LOW_UPPER_BOUNDS_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
    }

    x = FOLLOW(var_ptr-n); DEREF_NONVAR(x);
    REDUCE_LAST_TERM_DOMAIN_p1(x, tl[n-1], tu[n-1]);

    for (i = n-1; i >= 1; i--) {  /* reduce domains */
        /* x = FOLLOW(var_ptr-i);DEREF_NONVAR(x);*/
        x = xx[i];
        REDUCE_DOMAIN_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
    }
    return 1;
}

/* c+a1*x1+...+an*xn >= 0 */
/* t(i) = t(i-1)+a(i)x(i),
   t(0) = c
   l(n) = 0;
   t(n) >= l(n)
   t(i-1)+a(i)x(i) >= l(i);

   x(i) >= (l(i)-max(t(i-1)))/a(i)  if a(i) > 0
   x(i) =< (l(i)-min(t(i-1)))/a(i)  if a(i) < 0
   
   t(i-1) >= l(i)-a(i)*max(x(i)) if a(i) > 0
   t(i-1) >= l(i)-a(i)*min(x(i)) if a(i) < 0
*/
#define MAX_NUM_VARS_GE 551
int nary_interval_consistent_ge(BPLONG n)
{
    register BPLONG_PTR dv_ptr, var_ptr, coe_ptr;
    register BPLONG i, first, last;
    register BPLONG a, x;
    register BPLONG_PTR top;
    BPLONG tl[MAX_NUM_VARS_GE], tu[MAX_NUM_VARS_GE],
        aa[MAX_NUM_VARS_GE], xx[MAX_NUM_VARS_GE];  /* assume no constraint contain more than 10 variables */
    BPLONG first0, last0, dv_ptr_first, dv_ptr_last, size, temp_i, c;
    int k;
    BPLONG type;

    var_ptr = arreg+n+1;
    coe_ptr = var_ptr+n;
    c = INTVAL(FOLLOW(coe_ptr));  /* c+a1*x1+...+ak*xk where x1,...,xk are integers */
    type = INTVAL(FOLLOW(coe_ptr+1));

    /*
      printf("c = %d type=%d\n",c,type);
      for (i = 1; i<=n; i++){
      write_term(FOLLOW(coe_ptr-i)); printf("*"); write_term(FOLLOW(var_ptr-i)); printf(" ");
      }
      printf("\n");
    */

    /*  printf("ge n= %d type =%d\n",n,type); */
    switch (type) {
    case 0:  /* general case, overflow may occur */
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a > 0) {
                if (a == 1) {
                    COMPUTE_LOW_UPPER_BOUNDS_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    COMPUTE_LOW_UPPER_BOUNDS_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    COMPUTE_LOW_UPPER_BOUNDS_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    COMPUTE_LOW_UPPER_BOUNDS_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }

        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }

        tl[n] = 0;

        for (i = n; i >= 1; i--) {  /* reduce domains */
            a = aa[i];
            x = xx[i];

            if (a > 0) {
                if (a == 1) {
                    REDUCE_DOMAIN_GE_p1(x, tl[i-1], tu[i-1], tl[i]);
                } else {
                    REDUCE_DOMAIN_GE_p(a, x, tl[i-1], tu[i-1], tl[i]);
                }
            } else {
                if (a == -1) {
                    REDUCE_DOMAIN_GE_m1(x, tl[i-1], tu[i-1], tl[i]);
                } else {
                    a = -a;
                    REDUCE_DOMAIN_GE_m(a, x, tl[i-1], tu[i-1], tl[i]);
                }
            }
        }
        return 1;

    case coes_all_one: {  /*c+x1+x2+...+xn>=0*/
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {  /* compute low and upper bounds */
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
        }

        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }
        tl[n] = 0;

        for (i = n; i >= 1; i--) {  /* reduce domains */
            x = xx[i];
            REDUCE_DOMAIN_GE_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i]);
        }
        return 1;
    }

    case coes_all_mone: {  /* c-x1-x2-...-xn>=0 */
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
        }
        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }

        tl[n] = 0;
        for (i = n; i >= 1; i--) {  /* reduce domains */
            x = xx[i];
            REDUCE_DOMAIN_GE_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i]);
        }
        return 1;
    }

    case coes_all_one_bool:{  /*c+x1+x2+...+xn>=0 where xi's are boolean variables */
        int nvs = 0;
        c = -c;
        for (i = 1; i <= n; i++) {  /* count vars */
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ONE) {
                c--;
            } else if (x != BP_ZERO) {
                xx[nvs++] = x;
            }
        }
        if (c <= 0) {
            KILL_SUSP_FRAME;
        } else if (nvs <= c) {
            if (nvs < c) return BP_FALSE;
            for (i = 0; i < nvs; i++) {  /* bind remaining vars to 1 */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(xx[i]);
                ASSIGN_DVAR(dv_ptr, BP_ONE);
            }
            KILL_SUSP_FRAME;
        }
        return BP_TRUE;
    }

    case coes_all_mone_bool:{  /*c-x1-x2-...-xn>=0 where xi's are boolean variables */
        int nvs = 0;
        for (i = 1; i <= n; i++) {  /* count vars */
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ONE) {
                c--;
            } else if (x != BP_ZERO) {
                xx[nvs++] = x;
            }
        }
        if (c <= 0) {
            if (c < 0) return BP_FALSE;
            for (i = 0; i < nvs; i++) {  /* bind remaining vars to 0 */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(xx[i]);
                ASSIGN_DVAR(dv_ptr, BP_ZERO);
            }
            KILL_SUSP_FRAME;
        } else if (c >= nvs) {
            KILL_SUSP_FRAME;
        }
        return BP_TRUE;
    }

    case cv_ge_v: {  /* c+x1-x2 >= 0 */
        tl[0] = tu[0] = c;
        x = FOLLOW(var_ptr-1); DEREF_NONVAR(x); xx[1] = x;
        COMPUTE_LOW_UPPER_BOUNDS_p1(x, tl[0], tu[0], tl[1], tu[1]);
        x = FOLLOW(var_ptr-2); DEREF_NONVAR(x); xx[2] = x;
        COMPUTE_LOW_UPPER_BOUNDS_m1(x, tl[1], tu[1], tl[2], tu[2]);

        if (tl[2] >= 0) {
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[2] < 0) {
            return BP_FALSE;
        }
        tl[2] = 0;

        x = xx[2];
        REDUCE_DOMAIN_GE_m1(x, tl[1], tu[1], tl[2]);
        x = xx[1];
        REDUCE_DOMAIN_GE_p1(x, tl[0], tu[0], tl[1]);
        return BP_TRUE;
    }

    case coes_all_pos:
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a == 1) {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }
        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }

        tl[n] = 0;

        for (i = n; i >= 1; i--) {  /* reduce domains */
            a = aa[i];
            x = xx[i];

            if (a == 1) {
                REDUCE_DOMAIN_GE_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i]);
            } else {
                REDUCE_DOMAIN_GE_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i]);
            }
        }
        return 1;

    case coes_all_neg_bool:  /* c+a1*B1+...+an*Bn>=0 where Bi's boolean variables and ai<0 */
        for (i = 1; i <= n; i++) {  /* update the capacity */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (x == BP_ONE) {
                c += a;
            }
        }
        if (c < 0) return BP_FALSE;
        for (i = 1; i <= n; i++) {  /* make sure the capacity is not exceeded */
            if (IS_SUSP_VAR(xx[i])) {
                if (c+aa[i] < 0) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(xx[i]);
                    ASSIGN_DVAR(dv_ptr, BP_ZERO);
                }
            }
        }
        return BP_TRUE;

    case coes_one_mone:
    case coes_one_mone_bool:
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {  /* compute low and upper bounds */
            aa[i] = FOLLOW(coe_ptr-i);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (aa[i] == BP_ONE) {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            } else {
                COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
            }
        }

        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }

        tl[n] = 0;

        for (i = n; i >= 1; i--) {  /* reduce domains */
            x = xx[i];
            if (aa[i] == BP_ONE) {
                REDUCE_DOMAIN_GE_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i]);
            } else {
                REDUCE_DOMAIN_GE_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i]);
            }
        }
        return 1;

    default :
        tl[0] = tu[0] = c;
        for (i = 1; i <= n; i++) {  /* compute low and upper bounds */
            a = FOLLOW(coe_ptr-i); aa[i] = a = INTVAL(a);
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x); xx[i] = x;
            if (a > 0) {
                if (a == 1) {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            } else {
                if (a == -1) {
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i], tu[i]);
                } else {
                    a = -a;
                    COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m(a, x, tl[i-1], tu[i-1], tl[i], tu[i]);
                }
            }
        }

        if (tl[n] >= 0) {  /* its fruitless to reduce the domain */
            KILL_SUSP_FRAME;
            return BP_TRUE;
        }
        if (tu[n] < 0) {
            return BP_FALSE;
        }

        tl[n] = 0;

        for (i = n; i >= 1; i--) {  /* reduce domains */
            a = aa[i];
            x = xx[i];

            if (a > 0) {
                if (a == 1) {
                    REDUCE_DOMAIN_GE_nooverflow_p1(x, tl[i-1], tu[i-1], tl[i]);
                } else {
                    REDUCE_DOMAIN_GE_nooverflow_p(a, x, tl[i-1], tu[i-1], tl[i]);
                }
            } else {
                if (a == -1) {
                    REDUCE_DOMAIN_GE_nooverflow_m1(x, tl[i-1], tu[i-1], tl[i]);
                } else {
                    a = -a;
                    REDUCE_DOMAIN_GE_nooverflow_m(a, x, tl[i-1], tu[i-1], tl[i]);
                }
            }
        }
        return 1;
    }
}

/******** region *************/
/* X must be either a int or a fd variable */
int varorint_domain_region(BPLONG X, BPLONG from, BPLONG to)
{
    BPLONG_PTR dv_ptr;

    DEREF_NONVAR(X);
    if (ISINT(X)) {
        X = INTVAL(X);
        if (X < from || X > to) return BP_FALSE;
        return BP_TRUE;
    } else {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        return domain_region_noint(dv_ptr, from, to);
    }
}

int domain_region_noint(BPLONG_PTR dv_ptr, BPLONG from, BPLONG to)
{
    BPLONG dv_ptr_first, dv_ptr_last, size;
    int k;

    dv_ptr_first = DV_first(dv_ptr);
    dv_ptr_last = DV_last(dv_ptr);
    CALL_DOMAIN_REGION_noseed(dv_ptr, from, to);
    return 1;
}

int domain_region_aux(BPLONG_PTR dv_ptr, BPLONG from, BPLONG to)
{
    BPLONG dv_ptr_first, dv_ptr_last, size, val;
    int k;

    if (ISINT(FOLLOW(dv_ptr))) {
        val = INTVAL(FOLLOW(dv_ptr));
        if (val < from || val > to) return BP_FALSE;
        return BP_TRUE;
    }
    dv_ptr_first = DV_first(dv_ptr);
    dv_ptr_last = DV_last(dv_ptr);
    CALL_DOMAIN_REGION_noseed(dv_ptr, from, to);
    return 1;
}

void print_linear_constr(BPLONG n)
{
    BPLONG i;
    BPLONG c = FOLLOW(arreg+2*n+1);
    printf("n=" BPLONG_FMT_STR "\n", n);
    /*
      for (i=n;i>0;i--){
      printf("%x ", FOLLOW(arreg+n+i));
      }
      printf("\n");
    */
    write_term(c); fprintf(curr_out, "+");
    for (i = n; i > 0; i--) {
        write_term(FOLLOW(arreg+n+i)); fprintf(curr_out, "*");
        write_term(FOLLOW(arreg+i)); fprintf(curr_out, "+");
    }
    fprintf(curr_out, "=0 \n");
}

int b_CONSTR_COES_TYPE(BPLONG n)
{
    BPLONG_PTR dv_ptr, var_ptr, coe_ptr;
    BPLONG a, c, x, first, last;
    int property, type, term_lb, term_ub, all_bool_vars, i;

    n = INTVAL(n);
    property = 0;
    all_bool_vars = 1;

    var_ptr = arreg+n+1;
    coe_ptr = var_ptr+n;
    c = FOLLOW(coe_ptr); DEREF_NONVAR(c); FOLLOW(coe_ptr) = c; c = INTVAL(c);

    term_lb = BP_MININT_1W/(n+1); term_ub = BP_MAXINT_1W/(n+1);
    if (c < term_lb || c > term_ub) {FOLLOW(coe_ptr+1) = BP_ZERO; return BP_TRUE;}

    for (i = 1; i <= n; i++) {
        a = FOLLOW(coe_ptr-i); DEREF_NONVAR(a); FOLLOW(coe_ptr-i) = a; a = INTVAL(a);
        x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
        if (IS_SUSP_VAR(x)) {
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
            first = DV_first(dv_ptr);
            last = DV_last(dv_ptr);
            if (first == BP_MININT_1W || last == BP_MAXINT_1W) {FOLLOW(coe_ptr+1) = BP_ZERO; return BP_TRUE;}
        } else {
            first = last = INTVAL(x);
        }
        if (a*first < term_lb || a*last > term_ub) {FOLLOW(coe_ptr+1) = BP_ZERO; return BP_TRUE;}
        if (first != 0 || last != 1) all_bool_vars = 0;

        if (a == 1) {
            property |= 1;
        } else if (a == -1) {
            property |= 2;
        } else if (a > 0) {
            property |= 4;
        } else {
            property |= 8;
        }
    }
    switch (property) {
    case 1:  /* all 1's */
        if (all_bool_vars)
            type = coes_all_one_bool;
        else
            type = coes_all_one;
        break;
    case 2:  /* all -1's */
        if (all_bool_vars)
            type = coes_all_mone_bool;
        else
            type = coes_all_mone;
        break;
    case 3:  /* 1 and -1 */
        if (all_bool_vars)
            type = coes_one_mone_bool;
        else
            type = coes_one_mone;
        break;
    case 4:  /* all positive */
    case 5:  /* all positive */
        if (all_bool_vars)
            type = coes_all_pos_bool;
        else
            type = coes_all_pos;
        break;
    case 8:  /* all negative */
    case 10:  /* all positive */
        if (all_bool_vars)
            type = coes_all_neg_bool;
        else
            type = coes_all_neg;
        break;
    default:
        type = 14;
    }
    FOLLOW(coe_ptr+1) = MAKEINT(type);
    return BP_TRUE;
}

int b_IS_BOOLEAN_VAR_CONSTR(BPLONG coes_type)
{
    DEREF_NONVAR(coes_type);
    coes_type = INTVAL(coes_type);
    if (coes_type & 0x1) return BP_TRUE; return BP_FALSE;
}

/* Install the coefficients and variables on to the stack so that
   the nary_interval_consistent_eq or nary_interval_consistent_ge
   can be called.
*/
void install_coes_vars_to_stack(int n, BPLONG Terms, BPLONG Const) {
    int i;
    BPLONG_PTR var_ptr, coe_ptr;

    var_ptr = arreg+n+1;
    coe_ptr = var_ptr+n;
    FOLLOW(coe_ptr+1) = BP_ZERO;  /* type is 0, indicating the general case */
    FOLLOW(coe_ptr) = Const;

    i = 1;
    while (ISLIST(Terms)) {
        BPLONG pair, tmp;
        BPLONG_PTR lst_ptr, pair_ptr;
        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Terms);
        pair = FOLLOW(lst_ptr); DEREF(pair);
        pair_ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
        tmp = FOLLOW(pair_ptr+1); DEREF(tmp); FOLLOW(coe_ptr-i) = tmp;
        tmp = FOLLOW(pair_ptr+2); FOLLOW(var_ptr-i) = tmp;
        i++;
        Terms = FOLLOW(lst_ptr+1); DEREF(Terms);
    }
}

/* install the terms in reversed order */
void install_coes_vars_to_stack_rev(int n, BPLONG Terms, BPLONG Const) {
    int i;
    BPLONG_PTR var_ptr, coe_ptr;

    var_ptr = arreg+n+1;
    coe_ptr = var_ptr+n;
    FOLLOW(coe_ptr+1) = BP_ZERO;  /* type is 0, indicating the general case */
    FOLLOW(coe_ptr) = Const;

    i = n;
    while (ISLIST(Terms)) {
        BPLONG pair, tmp;
        BPLONG_PTR lst_ptr, pair_ptr;
        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Terms);
        pair = FOLLOW(lst_ptr); DEREF(pair);
        pair_ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
        tmp = FOLLOW(pair_ptr+1); DEREF(tmp); FOLLOW(coe_ptr-i) = tmp;
        tmp = FOLLOW(pair_ptr+2); FOLLOW(var_ptr-i) = tmp;
        i--;
        Terms = FOLLOW(lst_ptr+1); DEREF(Terms);
    }
}

/* Terms=[(An,Vn),...,(A1,V1)].
   Reduce domains of Vi's such that the constraint An*Vn+...,A1*V1+Const=0 is interval consistent
*/
int c_REDUCE_DOMAINS_IC_EQ() {
    BPLONG Terms, Const;
    BPLONG_PTR old_arreg;
    int n, res;

    Terms = ARG(1, 2); DEREF(Terms);
    Const = ARG(2, 2); DEREF(Const);
    /*
      printf("=>reduce ");
      write_term(Terms); printf("\n");
      write_term(Const); printf("\n");
    */
    n = list_length(Terms, Terms);
    if (n > MAX_NUM_VARS_EQ) return BP_TRUE;  /* constraint too large */
    old_arreg = arreg;
    arreg = local_top-2*n-2;
    install_coes_vars_to_stack(n, Terms, Const);
    res = nary_interval_consistent_eq(n);
    if (res == BP_FALSE){
        arreg = old_arreg;
        return BP_FALSE;
    }
        
    install_coes_vars_to_stack_rev(n, Terms, Const);
    res = nary_interval_consistent_eq(n);
    arreg = old_arreg;
    /*
      printf("<=reduce %d\n",res);  
      write_term(Terms); printf("\n");
      write_term(Const); printf("\n");
    */
    return res;
}

/* Terms=[(An,Vn),...,(A1,V1)].
   Reduce domains of Vi's such that the constraint An*Vn+...,A1*V1+Const>=0 is interval consistent
*/
int c_REDUCE_DOMAINS_IC_GE() {
    BPLONG Terms, Const;
    BPLONG_PTR old_arreg;
    int n, res;

    Terms = ARG(1, 2); DEREF(Terms);
    Const = ARG(2, 2); DEREF(Const);

    n = list_length(Terms, Terms);
    if (n > MAX_NUM_VARS_GE) return BP_TRUE;  /* constraint too large */
    old_arreg = arreg;
    arreg = local_top-2*n-2;
    install_coes_vars_to_stack( n, Terms, Const);
    res = nary_interval_consistent_ge(n);
    if (res == BP_FALSE){
        arreg = old_arreg;
        return BP_FALSE;
    }
        
    install_coes_vars_to_stack_rev(n, Terms, Const);
    res = nary_interval_consistent_ge(n);
        
    arreg = old_arreg;
    return res;
}

/* X1 = (X2 or ... or Xn), n<=15, Xi's are in the current frame */
int b_BOOL_OR_c(BPLONG n) {
    BPLONG_PTR dv_ptr, var_ptr;
    BPLONG x1, x;
    int i;
    //  printf("=> bool_or\n");
    DEREF_NONVAR(n); n = INTVAL(n);
    var_ptr = arreg+n;
    x1 = FOLLOW(var_ptr); DEREF_NONVAR(x1);
    if (IS_SUSP_VAR(x1)) {
        int num_of_vars = 0;
        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (IS_SUSP_VAR(x)) {
                num_of_vars++;
            } else if (x == BP_ONE) {  /* if x=1, then assign x1 to 1 and kill this propagator */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
                ASSIGN_DVAR(dv_ptr, BP_ONE);
                AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                return BP_TRUE;
            }
        }
        if (num_of_vars == 0) {  /* all xi's (i=2,...,n) are 0 */
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
            ASSIGN_DVAR(dv_ptr, BP_ZERO);
            AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
            return BP_TRUE;
        }
    } else if (x1 == BP_ZERO) {  /* assign xi to 0 for (i=2,...,n) */
        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (IS_SUSP_VAR(x)) {
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
                ASSIGN_DVAR(dv_ptr, BP_ZERO);
            } else if (x == BP_ONE) {
                return BP_FALSE;
            }
        }
        return BP_TRUE;
    } else {  /* x1=1, perform unit propagation */
        int num_of_vars = 0;
        BPLONG last_var;

        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ONE) {  /* it's done */
                AR_STATUS(arreg) = SUSP_EXIT;
                return BP_TRUE;
            } else if (x != BP_ZERO) {  /* x is a var */
                num_of_vars++;
                last_var = x;
            }
        }
        if (num_of_vars == 1) {  /* unit clause */
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(last_var);
            ASSIGN_DVAR(dv_ptr, BP_ONE);
            AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
            return BP_TRUE;
        } else if (num_of_vars == 0) {  /* all xi (i=2,...,n) are 0 */
            return BP_FALSE;
        }
        return BP_TRUE;
    }
    return BP_TRUE;  /* not reachable */
}

/* X1 = (X2 and ... and Xn), n<=15, Xi's are in the current frame */
int b_BOOL_AND_c(BPLONG n) {
    BPLONG_PTR dv_ptr, var_ptr;
    BPLONG x1, x;
    int i;

    //  printf("=> bool_and\n");
    DEREF_NONVAR(n); n = INTVAL(n);
    /*
      for (i=n; i>=1; i--){
      write_term(FOLLOW(arreg+i)); printf(" ");
      }
      printf("\n");
    */
    var_ptr = arreg+n;
    x1 = FOLLOW(var_ptr); DEREF_NONVAR(x1);
    if (IS_SUSP_VAR(x1)) {
        int num_of_vars = 0;
        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (IS_SUSP_VAR(x)) {
                num_of_vars++;
            } else if (x == BP_ZERO) {  /* if x=0, then assign x1 to 0, and kill this propagator */
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
                ASSIGN_DVAR(dv_ptr, BP_ZERO);
                AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                return BP_TRUE;
            }
        }
        if (num_of_vars == 0) {  /* all xi's (i=2,...,n) are 1 */
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
            ASSIGN_DVAR(dv_ptr, BP_ONE);
            AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
            return BP_TRUE;
        }
    } else if (x1 == BP_ONE) {  /* assign xi to 1 (i=2,...,n) */
        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (IS_SUSP_VAR(x)) {
                dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
                ASSIGN_DVAR(dv_ptr, BP_ONE);
            } else if (x == BP_ZERO) {
                return BP_FALSE;
            }
        }
        return BP_TRUE;
    } else {  /* x1=0, perform unit propagation (sort of) */
        int num_of_vars = 0;
        BPLONG last_var;

        for (i = 1; i < n; i++) {
            x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
            if (x == BP_ZERO) {  /* it's done */
                AR_STATUS(arreg) = SUSP_EXIT;
                return BP_TRUE;
            } else if (x != BP_ONE) {  /* x is a var */
                num_of_vars++;
                last_var = x;
            }
        }
        if (num_of_vars == 1) {  /* unit clause */
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(last_var);
            ASSIGN_DVAR(dv_ptr, BP_ZERO);
            AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
            return BP_TRUE;
        } else if (num_of_vars == 0) {  /* all xi (i=2,...,n) are 1 */
            return BP_FALSE;
        }
        return BP_TRUE;
    }
    return BP_TRUE;  /* not reachable */
}

/* X1 = max(X2, ..., Xn), n<=15, Xi's are in the current frame */
int b_PROP_MAX_c(BPLONG n) {
    BPLONG_PTR dv_ptr1, dv_ptr, var_ptr;
    BPLONG x1, x, first1, last1, first, last, num_of_vars, acc_min, acc_max;
    int i;

    //  printf("=> prop_max\n");
    DEREF_NONVAR(n); n = INTVAL(n);
    var_ptr = arreg+n;
    x1 = FOLLOW(var_ptr); DEREF_NONVAR(x1);
    if (IS_SUSP_VAR(x1)) {
        dv_ptr1 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
        first1 = DV_first(dv_ptr1);
        last1 = DV_last(dv_ptr1);
    } else {
        first1 = last1 = INTVAL(x1);
    }

    num_of_vars = 0;
    acc_min = BP_MAXINT_1W;
    acc_max = BP_MININT_1W;
    for (i = 1; i < n; i++) {
        x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
        if (IS_SUSP_VAR(x)) {
            num_of_vars++;
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);

            first = DV_first(dv_ptr);
            last = DV_last(dv_ptr);
            if (last > last1) {
                if (domain_region_noint(dv_ptr, first, last1) == BP_FALSE) return BP_FALSE;
            }
            if (first < acc_min) acc_min = first;
        } else {
            x = INTVAL(x);
            if (x > last1) return BP_FALSE;
            if (x < acc_min) acc_min = x;
            if (x > acc_max) acc_max = x;
        }
    }
    if (num_of_vars == 0) {
        if (first1 != last1) {  /* x1 is a var */
            if (dm_true(dv_ptr1, acc_max)) {
                ASSIGN_DVAR(dv_ptr1, MAKEINT(acc_max));
                AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                return BP_TRUE;
            } else {
                return BP_FALSE;
            }
        } else {
            return (first1 == acc_max) ? BP_TRUE : BP_FALSE;
        }
    } else {
        if (first1 != last1) {  /* x1 is a var */
            if (first1 < acc_min) {
                if (domain_region_noint(dv_ptr1, acc_min, last1) == BP_FALSE) return BP_FALSE;
            }
            return BP_TRUE;
        } else if (num_of_vars == 1 && acc_max < first1) {  /* do something like unit propagation */
            if (IS_SUSP_VAR(FOLLOW(dv_ptr))) {  /* dv_ptr points to the unique var, ensure that it's not instantiated */
                if (dm_true(dv_ptr, first1)) {
                    ASSIGN_DVAR(dv_ptr, MAKEINT(first1));
                    AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                    return BP_TRUE;
                } else {
                    return BP_FALSE;
                }
            }
        }
    }
    return BP_TRUE;
}


/* X1 = min(X2, ..., Xn), n<=15, Xi's are in the current frame */
int b_PROP_MIN_c(BPLONG n) {
    BPLONG_PTR dv_ptr1, dv_ptr, var_ptr;
    BPLONG x1, x, first1, last1, first, last, num_of_vars, acc_min, acc_max;
    int i;

    //  printf("=> prop_min\n");
    DEREF_NONVAR(n); n = INTVAL(n);
    var_ptr = arreg+n;
    x1 = FOLLOW(var_ptr); DEREF_NONVAR(x1);
    if (IS_SUSP_VAR(x1)) {
        dv_ptr1 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x1);
        first1 = DV_first(dv_ptr1);
        last1 = DV_last(dv_ptr1);
    } else {
        first1 = last1 = INTVAL(x1);
    }

    num_of_vars = 0;
    acc_min = BP_MAXINT_1W;
    acc_max = BP_MININT_1W;
    for (i = 1; i < n; i++) {
        x = FOLLOW(var_ptr-i); DEREF_NONVAR(x);
        if (IS_SUSP_VAR(x)) {
            num_of_vars++;
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
            first = DV_first(dv_ptr);
            last = DV_last(dv_ptr);
            if (first < first1) {
                if (domain_region_noint(dv_ptr, first1, last) == BP_FALSE) return BP_FALSE;
            }
            if (last > acc_max) acc_max = last;
        } else {
            x = INTVAL(x);
            if (x < first1) return BP_FALSE;
            if (x < acc_min) acc_min = x;
            if (x > acc_max) acc_max = x;
        }
    }
    if (num_of_vars == 0) {
        if (first1 != last1) {  /* x1 is var */
            if (dm_true(dv_ptr1, acc_min)) {
                ASSIGN_DVAR(dv_ptr1, MAKEINT(acc_min));
                AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                return BP_TRUE;
            } else {
                return BP_FALSE;
            }
        } else {
            return (first1 == acc_min) ? BP_TRUE : BP_FALSE;
        }
    } else {
        if (first1 != last1) {  /* x1 is var */
            if (acc_max < last1) {
                if (domain_region_noint(dv_ptr1, first1, acc_max) == BP_FALSE) return BP_FALSE;
            }
            return BP_TRUE;
        } else if (num_of_vars == 1 && acc_min > first1) {  /* do something like unit propagation */
            if (IS_SUSP_VAR(FOLLOW(dv_ptr))) {  /* dv_ptr points to the unique var, ensure that it's not instantiated */
                if (dm_true(dv_ptr, first1)) {
                    ASSIGN_DVAR(dv_ptr, MAKEINT(first1));
                    AR_STATUS(arreg) = SUSP_EXIT;  /* kill this propagator */
                    return BP_TRUE;
                } else {
                    return BP_FALSE;
                }
            }
        }
    }
    return BP_TRUE;
}

/* Maitain arc consistency for X+Y = Z when one of the domains is a bit-vector domain
   (a domain with holes), and Z's domain is small (which means both X's and Y's domains
   are small as well). Note that the constraint is already interval consistent.

   For each ex in X's domain and ey in Y's domain, the value ex+ey is supported. 
   For each ez in Z's domain, if ez is not supported, then remove ez from Z's domain.

   For each ey in Y's domain and ez in Z's domain, the value ez-ey is supported. 
   For each ex in X's domain, if ex is not supported, then remove ex from X's domain.
*/
/*
  #define NUM_MARK_WORDS 3200

  int c_REDUCE_DOMAIN_AC_ADD(){
  BPLONG X, Y, Z, ex, min_x, max_x, ey, min_y, max_y, ez, min_z, max_z, size;
  BPLONG_PTR dv_ptr_x, dv_ptr_y, dv_ptr_z, tmp_dv_ptr;
  int i, flag = 0;
  int mark_words[NUM_MARK_WORDS];
  
  X = ARG(1,3);
  Y = ARG(2,3);
  Z = ARG(3,3);

  printf("=> AC_ADD "); write_term(X); printf(" "); write_term(Y); printf(" "); write_term(Z); printf("\n");
  
  DEREF_NONVAR(Z);
  if (IS_SUSP_VAR(Z)){
  dv_ptr_z = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Z);
  if (!IS_SMALL_DOMAIN(dv_ptr_z)) return BP_TRUE;
  if (!IS_IT_DOMAIN(dv_ptr_z)) flag = 1;        
  min_z = DV_first(dv_ptr_z);  max_z = DV_last(dv_ptr_z);
  } else {
  dv_ptr_z = NULL;
  min_z = max_z = INTVAL(Z);
  }
  DEREF_NONVAR(X);
  if (IS_SUSP_VAR(X)){
  dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
  if (!IS_IT_DOMAIN(dv_ptr_x)) flag = 1;
  min_x = DV_first(dv_ptr_x); max_x = DV_last(dv_ptr_x);
  } else {
  dv_ptr_x = NULL;
  min_x = max_x = INTVAL(X);
  }
  DEREF_NONVAR(Y);
  if (IS_SUSP_VAR(Y)){
  dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
  if (!IS_IT_DOMAIN(dv_ptr_y)) flag = 1;        
  min_y = DV_first(dv_ptr_y); max_y = DV_last(dv_ptr_y);
  } else {
  dv_ptr_y = NULL;
  min_y = max_y = INTVAL(Y);
  }
  if (flag == 0) return BP_TRUE;                     // none of the domains is a bit-vector domain 

  if (dv_ptr_z != NULL && IS_IT_DOMAIN(dv_ptr_z))
  goto lab_ac_on_z;
  if (dv_ptr_x != NULL && IS_IT_DOMAIN(dv_ptr_x))
  goto lab_ac_on_x;
  if (dv_ptr_y != NULL && IS_IT_DOMAIN(dv_ptr_y)) {  // swap X with Y 
  tmp_dv_ptr = dv_ptr_x; dv_ptr_x = dv_ptr_y; dv_ptr_y = tmp_dv_ptr;
  min_y = min_x; max_y = max_x;
  min_x = DV_first(dv_ptr_x); max_x = DV_last(dv_ptr_x);
  goto lab_ac_on_x;
  }
  return BP_TRUE;                                    // none of the domains is interval 

  lab_ac_on_x:  
  size = max_x - min_x + 1;
  if (size > NUM_MARK_WORDS) return BP_TRUE;         // no enough space for marking 

  for (i = 0; i < size; i++){
  mark_words[i] = 0;                               // initialize the marks, no value is supported 
  }
  
  for (ez = min_z; ez <= max_z; ez++){
  if (dv_ptr_z != NULL && !dm_true(dv_ptr_z, ez)) continue;
  for (ey = min_y; ey <= max_y; ey++){
  if (dv_ptr_y != NULL && !dm_true(dv_ptr_y, ey)) continue;
  ex = ez-ey;
  mark_words[ex - min_x] = 1;
  }
  }
  for (ex = min_x+1; ex < max_x; ex++){
  if (mark_words[ex - min_x] == 0){
  domain_set_false_noint(dv_ptr_x, ex);
  }
  }
  printf("<= AC_ADD "); write_term(X); printf(" "); write_term(Y); printf(" "); write_term(Z); printf("\n");
  return BP_TRUE;
  
  lab_ac_on_z:
  size = max_z - min_z + 1;
  if (size > NUM_MARK_WORDS) return BP_TRUE;         // no enough space for marking 

  for (i = 0; i < size; i++){
  mark_words[i] = 0;                               // initialize the marks, no value is supported 
  }
  
  for (ex = min_x; ex <= max_x; ex++){
  if (dv_ptr_x != NULL && !dm_true(dv_ptr_x, ex)) continue;
  for (ey = min_y; ey <= max_y; ey++){
  if (dv_ptr_y != NULL && !dm_true(dv_ptr_y, ey)) continue;
  ez = ex+ey;
  mark_words[ez - min_z] = 1;
  }
  }
  for (ez = min_z+1; ez < max_z; ez++){
  if (mark_words[ez - min_z] == 0){
  domain_set_false_noint(dv_ptr_z, ez);
  }
  }
  printf("<= AC_ADD "); write_term(X); printf(" "); write_term(Y); printf(" "); write_term(Z); printf("\n");
  return BP_TRUE;
  }
*/

