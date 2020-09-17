/********************************************************************
 *   File   : cfd.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: Primitives on composite finite domains (tuples)

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "term.h"
#include "bapi.h"
#include "event.h"
#include "clpfd.h"

#define A2_GET(a, i, j, nj) FOLLOW(a+i*nj+j)
#define A3_GET(a, i, j, k, nj, nk) FOLLOW(a+(i*nj+j)*nk+k)

/*
  Create a new bit vector fd variable. The bit vector has the range from..to and 
  the initial attribute values (first,last,and size) are given. The bit vector 
  has all the bits set to 0 if bv_word=0x0. In this case, the elements are to 
  be added to the domain later. This is unusual but convenient.
*/

BPLONG_PTR new_bv_domain_var(from, to, first, last, size, bv_word)
    BPLONG from, to, first, last, size;
BPULONG bv_word;
{
    BPLONG_PTR dv_ptr, top;
    BPLONG i;

    dv_ptr = heap_top;
    DV_var(dv_ptr) = ((BPLONG)heap_top | SUSP);
    DV_ins_cs(dv_ptr) = nil_sym;
    DV_minmax_cs(dv_ptr) = nil_sym;
    DV_dom_cs(dv_ptr) = nil_sym;
    DV_outer_dom_cs(dv_ptr) = nil_sym;
    DV_attached(dv_ptr) = nil_sym;
    heap_top += SIZE_OF_DV;

    DV_first(dv_ptr) = first;
    DV_last(dv_ptr) = last;
    DV_size(dv_ptr) = size;

    top = A_DV_bit_vector_ptr(dv_ptr);

    from = WORD_NUMBER(from);
    to = WORD_NUMBER(to);
    FOLLOW(top) = (BPLONG)heap_top;

    FOLLOW(heap_top++) = from*NBITS_IN_LONG;  /* BV_low_val */
    FOLLOW(heap_top++) = (to+1)*NBITS_IN_LONG-1;  /* BV_up_val */

    for (i = from; i <= to; i++) {
        *heap_top++ = bv_word;
    }

    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 0");
    }

    return dv_ptr;
}

/*
  updates are not trailed because it is assumed that 
  no choice point has been created since creation of 
  the domain variable
*/
void domain_set_true_bv(dv_ptr, elm)
    BPLONG_PTR dv_ptr;
    BPLONG elm;
{
    BPULONG w, mask, offset;
    BPLONG_PTR w_ptr, bv_ptr;

    bv_ptr = (BPLONG_PTR)DV_bit_vector_ptr(dv_ptr);
    WORD_OFFSET(bv_ptr, elm, w, w_ptr, offset);
    mask = ((BPULONG)0x1 << offset);
    if (w & mask) return;  /* already on */
    FOLLOW(w_ptr) = (w | mask);
    DV_size(dv_ptr) += 1;
    if (elm < DV_first(dv_ptr)) {
        DV_first(dv_ptr) = elm;
    }
    if (elm > DV_last(dv_ptr)) {
        DV_last(dv_ptr) = elm;
    }
}

/* no bit-vector is used if last = first+1 */
BPLONG_PTR new_it_domain_var(first, last)
    BPLONG first, last;
{
    BPLONG_PTR dv_ptr;

    dv_ptr = heap_top;
    DV_var(dv_ptr) = ((BPLONG)heap_top | SUSP);
    DV_ins_cs(dv_ptr) = nil_sym;
    DV_minmax_cs(dv_ptr) = nil_sym;
    DV_dom_cs(dv_ptr) = nil_sym;
    DV_outer_dom_cs(dv_ptr) = nil_sym;
    DV_attached(dv_ptr) = nil_sym;
    heap_top += SIZE_OF_DV;

    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 1");
    }

    DV_first(dv_ptr) = first;
    DV_last(dv_ptr) = last;
    DV_size(dv_ptr) = last-first+1;
    DV_type(dv_ptr) = IT_DOMAIN;

    return dv_ptr;
}

int b_CFD_COMPUTE_MINS_MAXS(Arity, Tuples, Mins, Maxs)
    BPLONG Arity, Tuples, Mins, Maxs;
{
    BPLONG_PTR MinArray, MaxArray, HasStarArray, mins_ptr, maxs_ptr, ptr, tuple_ptr;
    BPLONG i, n, tuple;

    DEREF_NONVAR(Arity); n = INTVAL(Arity);

    /* compute MinArray[i] and MaxArray[i] */
    MinArray = local_top - n - 1;
    MaxArray = MinArray - n - 1;
    HasStarArray = MaxArray - n - 1;

    if (HasStarArray-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 2");
    }

    for (i = 1; i <= n; i++) {
        MinArray[i] = BP_MAXINT_1W;
        MaxArray[i] = BP_MININT_1W;
        HasStarArray[i] = 0;
    }
    DEREF_NONVAR(Tuples);
    while (ISLIST(Tuples)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuples);
        tuple = FOLLOW(ptr); DEREF_NONVAR(tuple);
        Tuples = FOLLOW(ptr+1); DEREF_NONVAR(Tuples);
        tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tuple);

        for (i = 1; i <= n; i++) {
            BPLONG e;
            e = FOLLOW(tuple_ptr+i); DEREF(e);
            if (ISINT(e)) {
                e = INTVAL(e);
                if (e < MinArray[i]) {MinArray[i] = e;}
                if (e > MaxArray[i]) {MaxArray[i] = e;}
            } else if (e == star_atom) {
                HasStarArray[i] = 1;
            } else {
                bp_exception = illegal_arguments;
                return BP_ERROR;
            }
        }
    }

    DEREF_NONVAR(Mins);
    mins_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Mins);
    DEREF_NONVAR(Maxs);
    maxs_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Maxs);
    for (i = 1; i <= n; i++) {
        if (HasStarArray[i] == 0) {
            unify(FOLLOW(mins_ptr+i), MAKEINT(MinArray[i]));
            unify(FOLLOW(maxs_ptr+i), MAKEINT(MaxArray[i]));
        } else {
            unify(FOLLOW(mins_ptr+i), star_atom);
            unify(FOLLOW(maxs_ptr+i), star_atom);
        }
    }
    return BP_TRUE;
}

/*  Normalize the tuples so that the minimum element in every column is 0 */
void cfd_transform_tuples(n, Tuples, MinArray)
    BPLONG n, Tuples;
BPLONG_PTR MinArray;
{
    BPLONG tuple, i, e;
    BPLONG_PTR ptr;

    DEREF_NONVAR(Tuples);
    while (ISLIST(Tuples)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuples);
        tuple = FOLLOW(ptr); DEREF_NONVAR(tuple);
        Tuples = FOLLOW(ptr+1); DEREF_NONVAR(Tuples);
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(tuple);

        for (i = 1; i <= n; i++) {
            BPLONG_PTR arg_ptr;
            if (MinArray[i] < 0) {
                arg_ptr = ptr+i;
                e = FOLLOW(arg_ptr); DEREF_NONVAR(e);
                e = INTVAL(e)-MinArray[i];
                PUSHTRAIL_h(arg_ptr);
                FOLLOW(arg_ptr) = MAKEINT(e);
            }
        }
    }
}

int b_CFD_TRANSFORM_TUPLES(Arity, Tuples, Mins)
    BPLONG Arity, Tuples, Mins;
{
    BPLONG_PTR MinArray, ptr;
    BPLONG i, n;

    DEREF_NONVAR(Arity); n = INTVAL(Arity);

    DEREF_NONVAR(Mins);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(Mins);
    MinArray = local_top-n-1;

    if (MinArray-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 3");
    }

    for (i = 1; i <= n; i++) {
        BPLONG e = FOLLOW(ptr+i);
        DEREF_NONVAR(e);
        MinArray[i] = INTVAL(e);
    }
    cfd_transform_tuples(n, Tuples, MinArray);
    return BP_TRUE;
}

/* b_CFD_BUILD_TRIES_IN(Maxes,Tuples,A2Tries) converts tuples into tries.

   Maxs=t(M1,...,Mn),

   Tuples=[t(a11,...,a1n),...,t(am1,...,amn)],

   A2Tries=t(t(_,T12,...,T1n),
   ...,
   t(Tn1,Tn2,...,Tn(n-1),_)) 
   
   Tij takes the form trie(Sij0,...,Sijk,...) where Sijk denotes k's supports (fd var) in Vj
   (k is an element in Vi's domain).

   For each tuple t(a1,...,an) and for each pair (i j) (1=<i<j<=n), 
   add aj as a support of ai and add ai as a support of aj.

   Note that the tuples have been normalized such that the minimum is 0.
*/
int b_CFD_BUILD_TRIES_IN(Maxs, Tuples, A2Tries)
    BPLONG Maxs, Tuples, A2Tries;
{
    BPLONG_PTR local_top0;
    BPLONG_PTR MinArray, MaxArray, CompVarMaxArray, tries_ptr, maxs_ptr;
    BPLONG n, nk, array_size, i;
    SYM_REC_PTR sym_ptr;
    BPLONG max_domain_size();

    DEREF_NONVAR(Maxs);
    maxs_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Maxs);
    sym_ptr = (SYM_REC_PTR)FOLLOW(maxs_ptr); n = GET_ARITY(sym_ptr);
    DEREF_NONVAR(Tuples);

    //  printf("=>BUILD_TRIES_IN %x %x\n",local_top,heap_top);

    local_top0 = local_top;  /* reuse the Prolog stack */
    local_top -= n;
    CompVarMaxArray = local_top;
    nk = 0;  /* nk is the largest value in any domain */

    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 4");
    }

    for (i = 0; i < n; i++) {
        BPLONG e = FOLLOW(maxs_ptr+i+1); DEREF_NONVAR(e); e = INTVAL(e);
        CompVarMaxArray[i] = e;
        if (e > nk) nk = e;
    }

    /* compute minimum and maximum supports in Vj of each element in Vi */
    nk++;
    array_size = n*n*nk;
    local_top -= array_size;
    MinArray = local_top;
    local_top -= array_size;
    MaxArray = local_top;
    local_top--;  /* just in case */
    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 5");
    }
    initialize_min_max_arrays(n, nk, MinArray, MaxArray);

    compute_mins_maxs_in(n, nk, Tuples, MinArray, MaxArray);

    DEREF_NONVAR(A2Tries);
    tries_ptr = (BPLONG_PTR)UNTAGGED_ADDR(A2Tries);
    initialize_supports(n, nk, tries_ptr, CompVarMaxArray, MinArray, MaxArray);

    /* complete the construction of supports Sijk (supports in Vj of k (k in Vi))  */
    compute_supports_in(n, Tuples, tries_ptr, MinArray, MaxArray);

    //  printf("<=BUILD_TRIES_IN %x %x\n",local_top,heap_top);
    local_top = local_top0;

    return BP_TRUE;
}

/* b_CFD_BUILD_TRIES_NOTIN(CompVars,HTable,A2Tries) converts tuples into tries.
   for each tuple (a1,...,an) that is not in HTable and for each pair (i j) (1=<i<j<=n), 
   add aj as a support of ai in i and add ai as a support of aj in j.
*/
int b_CFD_BUILD_TRIES_NOTIN(CompVars, HTable, A2Tries)
    BPLONG CompVars, HTable, A2Tries;
{
    BPLONG_PTR comp_vars_ptr, local_top0;
    BPLONG_PTR MinArray, MaxArray, CompVarArray, CompVarMaxArray, tries_ptr, tuple_ptr, htable_ptr;
    BPLONG compvar, tuple, n, nk, array_size, i, htable_size;
    SYM_REC_PTR sym_ptr;
    BPLONG max_domain_size();

    DEREF_NONVAR(HTable);
    htable_ptr = (BPLONG_PTR)UNTAGGED_ADDR(HTable);
    sym_ptr = (SYM_REC_PTR)FOLLOW(htable_ptr);
    htable_size = GET_ARITY(sym_ptr);

    DEREF_NONVAR(CompVars);  /* CompVars=t(V1,...,Vn) */
    comp_vars_ptr = (BPLONG_PTR)UNTAGGED_ADDR(CompVars);
    sym_ptr = (SYM_REC_PTR)FOLLOW(comp_vars_ptr); n = GET_ARITY(sym_ptr);


    DEREF_NONVAR(A2Tries);  /* A2Tries=t(t(_,T12,...,T1n),...,t(Tn1,Tn2,...,Tn(n-1),_))
                               where Tij is a trie trie(Sij0,Sij1,...,Sijk)*/

    local_top0 = local_top;  /* reuse the Prolog stack */

    /* copy CompVars to CompVarArray */
    local_top -= n;
    CompVarArray = local_top;
    local_top -= n;
    CompVarMaxArray = local_top;
    local_top--;  /* just in case */
    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 6");
    }

    nk = 0;
    for (i = 0; i < n; i++) {
        compvar = FOLLOW(comp_vars_ptr+1+i); DEREF_NONVAR(compvar);
        if (IS_SUSP_VAR(compvar)) {
            BPLONG_PTR dv_ptr;
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
            CompVarMaxArray[i] = DV_last(dv_ptr);
        } else {
            CompVarMaxArray[i] = INTVAL(compvar);
        }
        if (nk < CompVarMaxArray[i]) nk = CompVarMaxArray[i];
        CompVarArray[i] = compvar;
    }

    /* compute minimum and maximum supports in Vj of each element in Vi */
    nk++;
    array_size = n*n*nk+1;
    local_top -= array_size;
    MinArray = local_top;
    local_top -= array_size;
    MaxArray = local_top;

    initialize_min_max_arrays(n, nk, MinArray, MaxArray);

    local_top -= (n+2);

    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 8");
    }

    tuple_ptr = local_top;  /* space for a tuple */
    FOLLOW(tuple_ptr) = (BPLONG)sym_ptr;
    tuple = ADDTAG(tuple_ptr, STR);


    compute_mins_maxs_notin(0, n, nk, htable_ptr, htable_size, tuple_ptr, CompVarArray, MinArray, MaxArray);

    tries_ptr = (BPLONG_PTR)UNTAGGED_ADDR(A2Tries);
    initialize_supports(n, nk, tries_ptr, CompVarMaxArray, MinArray, MaxArray);

    /* complete the construction of supports Sijk (supports in Vj of k in Vi)  */
    compute_supports_notin(0, n, htable_ptr, htable_size, tuple_ptr, CompVarArray, tries_ptr, MinArray, MaxArray);

    local_top = local_top0;

    return BP_TRUE;
}

/* initialize the arrays */
void initialize_min_max_arrays(n, nk, MinArray, MaxArray)
    BPLONG n, nk;
BPLONG_PTR MinArray, MaxArray;
{
    BPLONG i, j, k;

    for (i = 0; i < n-1; i++) {
        for (j = i+1; j < n; j++) {
            for (k = 0; k < nk; k++) {
                A3_GET(MinArray, i, j, k, n, nk) = BP_MAXINT_1W;
                A3_GET(MaxArray, i, j, k, n, nk) = BP_MININT_1W;
                A3_GET(MinArray, j, i, k, n, nk) = BP_MAXINT_1W;
                A3_GET(MaxArray, j, i, k, n, nk) = BP_MININT_1W;
            }
        }
    }
}

/* for i in 0..n-2, j in i+1..n-1, compute supports[i,j,_] and supports[j,i,_] */
void compute_mins_maxs_in(n, nk, Tuples, MinArray, MaxArray)
    BPLONG n, nk;
BPLONG Tuples;
BPLONG_PTR MinArray, MaxArray;
{
    BPLONG i, j;

    while (ISLIST(Tuples)) {
        BPLONG tuple;
        BPLONG_PTR tuples_ptr, tuple_ptr;
        tuples_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuples);
        tuple = FOLLOW(tuples_ptr); DEREF_NONVAR(tuple);
        tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tuple);
        Tuples = FOLLOW(tuples_ptr+1); DEREF_NONVAR(Tuples);

        for (i = 1; i <= n; i++) {
            BPLONG a;
            a = FOLLOW(tuple_ptr+i); DEREF_NONVAR(a); FOLLOW(local_top-i+1) = INTVAL(a);  /* reuse the stack */
        }
        for (i = 0; i < n-1; i++) {
            for (j = i+1; j < n; j++) {
                BPLONG ai, aj;
                ai = FOLLOW(local_top-i);
                aj = FOLLOW(local_top-j);
                if (aj < A3_GET(MinArray, i, j, ai, n, nk)) A3_GET(MinArray, i, j, ai, n, nk) = aj;
                if (aj > A3_GET(MaxArray, i, j, ai, n, nk)) A3_GET(MaxArray, i, j, ai, n, nk) = aj;
                if (ai < A3_GET(MinArray, j, i, aj, n, nk)) A3_GET(MinArray, j, i, aj, n, nk) = ai;
                if (ai > A3_GET(MaxArray, j, i, aj, n, nk)) A3_GET(MaxArray, j, i, aj, n, nk) = ai;
            }
        }
    }
}

/* iterate through all possible tuples from V1*V2*...*Vn that are not in hashtable */
void compute_mins_maxs_notin(arg_no, n, nk, htable_ptr, htable_size, tuple_ptr, CompVarArray, MinArray, MaxArray)
    BPLONG arg_no, n, nk, htable_size;
BPLONG_PTR tuple_ptr, CompVarArray, MinArray, MaxArray, htable_ptr;
{
    BPLONG compvar, i, j;

    if (arg_no == n) {
        if (!htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) {
            for (i = 0; i < n; i++) {
                FOLLOW(local_top-i) = INTVAL(FOLLOW(tuple_ptr+i+1));  /* reuse the stack */
            }
            for (i = 0; i < n-1; i++) {
                for (j = i+1; j < n; j++) {
                    BPLONG ai, aj;
                    ai = FOLLOW(local_top-i);
                    aj = FOLLOW(local_top-j);
                    if (aj < A3_GET(MinArray, i, j, ai, n, nk)) A3_GET(MinArray, i, j, ai, n, nk) = aj;
                    if (aj > A3_GET(MaxArray, i, j, ai, n, nk)) A3_GET(MaxArray, i, j, ai, n, nk) = aj;
                    if (ai < A3_GET(MinArray, j, i, aj, n, nk)) A3_GET(MinArray, j, i, aj, n, nk) = ai;
                    if (ai > A3_GET(MaxArray, j, i, aj, n, nk)) A3_GET(MaxArray, j, i, aj, n, nk) = ai;
                }
            }
        }
    } else {
        compvar = CompVarArray[arg_no];
        if (ISINT(compvar)) {
            FOLLOW(tuple_ptr+arg_no+1) = compvar;
            compute_mins_maxs_notin(arg_no+1, n, nk, htable_ptr, htable_size, tuple_ptr, CompVarArray, MinArray, MaxArray);
        } else {
            BPLONG e, last;
            BPLONG_PTR compvar_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
            e = DV_first(compvar_dv_ptr);
            last = DV_last(compvar_dv_ptr);
            for (; ; ) {
                FOLLOW(tuple_ptr+arg_no+1) = MAKEINT(e);
                compute_mins_maxs_notin(arg_no+1, n, nk, htable_ptr, htable_size, tuple_ptr, CompVarArray, MinArray, MaxArray);
                if (e == last) break;
                e++;
                if (!IS_IT_DOMAIN(compvar_dv_ptr)) e = domain_next_bv(compvar_dv_ptr, e);
            }
        }
    }
}

/* initialize supports Sijk (supports in Vj of k in Vi)  */
void initialize_supports(n, nk, tries_ptr, CompVarMaxArray, MinArray, MaxArray)
    BPLONG n, nk;
BPLONG_PTR tries_ptr, MinArray, MaxArray, CompVarMaxArray;
{
    BPLONG i, j, k, tries_ij, min, max;
    BPLONG_PTR tries_ij_ptr;

    for (i = 0; i < n; i++) {
        BPLONG tries_i;
        BPLONG_PTR tries_i_ptr;
        tries_i = FOLLOW(tries_ptr+i+1); DEREF_NONVAR(tries_i);
        tries_i_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_i);
        for (k = 0; k <= CompVarMaxArray[i]; k++) {
            for (j = 0; j < n; j++) {
                if (i != j) {
                    tries_ij = FOLLOW(tries_i_ptr+j+1); DEREF_NONVAR(tries_ij);  /* tries_ij = trie(Sij0,Sij1,...Sij_max(Vi)) */
                    tries_ij_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_ij);

                    min = A3_GET(MinArray, i, j, k, n, nk);
                    max = A3_GET(MaxArray, i, j, k, n, nk);
                    /*  printf("min=%d max=%d\n",min,max); */
                    if (min > max) {
                        /* this occurs when k does not occur in column i */
                    } else if (min == max) {
                        FOLLOW(tries_ij_ptr+k+1) = MAKEINT(min);
                    } else if (min+1 == max) {
                        FOLLOW(tries_ij_ptr+k+1) = (BPLONG)new_it_domain_var(min, max);
                    } else {
                        FOLLOW(tries_ij_ptr+k+1) = (BPLONG)new_bv_domain_var(min, max, max, min, 0, 0x0L);
                    }
                }
            }
        }
    }
}

void compute_supports_in(n, Tuples, tries_ptr, MinArray, MaxArray)
    BPLONG n;
    BPLONG Tuples;
BPLONG_PTR tries_ptr, MinArray, MaxArray;
{

    BPLONG i, j;
    while (ISLIST(Tuples)) {
        BPLONG_PTR tuples_ptr, tuple_ptr;
        BPLONG tuple;
        tuples_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuples);
        tuple = FOLLOW(tuples_ptr); DEREF_NONVAR(tuple);
        tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tuple);
        Tuples = FOLLOW(tuples_ptr+1); DEREF_NONVAR(Tuples);

        for (i = 1; i <= n; i++) {
            BPLONG a;
            a = FOLLOW(tuple_ptr+i); DEREF_NONVAR(a); FOLLOW(local_top-i+1) = INTVAL(a);  /* reuse the stack */
        }
        for (i = 0; i < n; i++) {
            BPLONG tries_i, ai;
            BPLONG_PTR tries_i_ptr;
            ai = FOLLOW(local_top-i);
            tries_i = FOLLOW(tries_ptr+i+1); DEREF_NONVAR(tries_i);
            tries_i_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_i);

            for (j = 0; j < n; j++) {
                BPLONG aj, supports, tries_ij;
                BPLONG_PTR dv_ptr, tries_ij_ptr;

                if (i == j) continue;
                aj = FOLLOW(local_top-j);
                tries_ij = FOLLOW(tries_i_ptr+j+1); DEREF_NONVAR(tries_ij);
                tries_ij_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_ij);

                supports = FOLLOW(tries_ij_ptr+ai+1); DEREF(supports);
                if (IS_SUSP_VAR(supports)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(supports);
                    if (!IS_IT_DOMAIN(dv_ptr)) {
                        domain_set_true_bv(dv_ptr, aj);  /* aj supports ai */
                    }
                }
            }
        }
    }
}

void compute_supports_notin(arg_no, n, htable_ptr, htable_size, tuple_ptr, CompVarArray, tries_ptr, MinArray, MaxArray)
    BPLONG arg_no, n;
BPLONG htable_size;
BPLONG_PTR tuple_ptr, CompVarArray, tries_ptr, MinArray, MaxArray, htable_ptr;
{
    BPLONG i, j;

    if (arg_no == n) {
        if (!htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) {
            /* printf("current tuple : "); write_term(tuple); printf("\n"); */
            for (i = 0; i < n; i++) {
                FOLLOW(local_top-i) = INTVAL(FOLLOW(tuple_ptr+i+1));  /* reuse the stack */
            }
            for (i = 0; i < n; i++) {
                BPLONG tries_i, ai;
                BPLONG_PTR tries_i_ptr;
                ai = FOLLOW(local_top-i);
                tries_i = FOLLOW(tries_ptr+i+1); DEREF_NONVAR(tries_i);
                tries_i_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_i);

                for (j = 0; j < n; j++) {
                    BPLONG aj, supports, tries_ij;
                    BPLONG_PTR dv_ptr, tries_ij_ptr;

                    if (i == j) continue;
                    aj = FOLLOW(local_top-j);
                    tries_ij = FOLLOW(tries_i_ptr+j+1); DEREF_NONVAR(tries_ij);
                    tries_ij_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_ij);

                    supports = FOLLOW(tries_ij_ptr+ai+1); DEREF(supports);
                    if (IS_SUSP_VAR(supports)) {
                        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(supports);
                        if (!IS_IT_DOMAIN(dv_ptr)) {
                            domain_set_true_bv(dv_ptr, aj);
                        }
                    }
                }
            }
        }
    } else {
        BPLONG compvar = CompVarArray[arg_no];
        if (ISINT(compvar)) {
            FOLLOW(tuple_ptr+arg_no+1) = compvar;
            compute_supports_notin(arg_no+1, n, htable_ptr, htable_size, tuple_ptr, CompVarArray, tries_ptr, MinArray, MaxArray);
        } else {
            BPLONG e, last;
            BPLONG_PTR compvar_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
            e = DV_first(compvar_dv_ptr);
            last = DV_last(compvar_dv_ptr);
            for (; ; ) {
                FOLLOW(tuple_ptr+arg_no+1) = MAKEINT(e);
                compute_supports_notin(arg_no+1, n, htable_ptr, htable_size, tuple_ptr, CompVarArray, tries_ptr, MinArray, MaxArray);
                if (e == last) break;
                e++;
                if (!IS_IT_DOMAIN(compvar_dv_ptr)) e = domain_next_bv(compvar_dv_ptr, e);
            }
        }
    }
}

/* exclude those elements that have no support */
int exclude_ac_unsupported_from_fd(X, Y, trie_xy_ptr)
    BPLONG X, Y;
BPLONG_PTR trie_xy_ptr;
{
    BPLONG k, Sk, max;
    SYM_REC_PTR sym_ptr;

    sym_ptr = (SYM_REC_PTR)FOLLOW(trie_xy_ptr);
    max = GET_ARITY(sym_ptr);

    if (varorint_domain_region(X, 0, max-1) == BP_FALSE) return BP_FALSE;
    DEREF_NONVAR(X);
    if (ISINT(X)) {
        k = INTVAL(X);
        Sk = FOLLOW(trie_xy_ptr+k+1); DEREF(Sk);
        if ((!ISINT(Sk) && !IS_SUSP_VAR(Sk)) || b_DM_DISJOINT_cc(Y, Sk)) {
            return BP_FALSE;
        }
    } else {
        BPLONG_PTR dv_ptr;
        BPLONG last;
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        k = DV_first(dv_ptr);
        last = DV_last(dv_ptr);
        for (; ; ) {
            Sk = FOLLOW(trie_xy_ptr+k+1); DEREF(Sk);
            if ((!ISINT(Sk) && !IS_SUSP_VAR(Sk)) || b_DM_DISJOINT_cc(Y, Sk)) {
                /* printf("domain set false %d",k); write_term(dv_ptr); printf("\n"); */
                if (domain_set_false_aux(dv_ptr, k) == BP_FALSE) return BP_FALSE;
            }
            if (k == last) break;  /* break for (;;) */
            k++;
            if (!IS_IT_DOMAIN(dv_ptr)) k = domain_next_bv(dv_ptr, k);
        }
    }
    return BP_TRUE;
}

/* For each element in each domain, exclude the element from the domain
   if it has no support in the domain of any of the connected variables. */
int b_CFD_REMOVE_AC_UNSUPPORTED(CompVars, Tries)
    BPLONG CompVars, Tries;
{
    BPLONG i, j, n, trie, X, Y;
    BPLONG_PTR comp_vars_ptr, tries_ptr;
    SYM_REC_PTR sym_ptr;

    //  printf("=>b_CFD_REMOVE_AC_UNSUPPORTED %x %x ",local_top,heap_top); write_term(Tries); printf("\n");
    DEREF_NONVAR(CompVars);
    comp_vars_ptr = (BPLONG_PTR)UNTAGGED_ADDR(CompVars);
    sym_ptr = (SYM_REC_PTR)FOLLOW(comp_vars_ptr);
    n = GET_ARITY(sym_ptr);
    for (i = 1; i <= n; i++) {
        BPLONG compvar;
        compvar = FOLLOW(comp_vars_ptr+i); DEREF_NONVAR(compvar);
        if (IS_SUSP_VAR(compvar)) {
            BPLONG_PTR dv_ptr;
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
            if (DV_size(dv_ptr) != 2) {
                if (IS_IT_DOMAIN(dv_ptr)) {
                    it_to_bv(dv_ptr);
                }
            }
        }
    }

    DEREF_NONVAR(Tries);
    tries_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tries);  /* Tries=t(t(_,T12,...,T1n),...,t(Tn1,Tn2,...,Tn(n-1),_)) */
    for (i = 0; i < n; i++) {
        BPLONG tries_i;
        BPLONG_PTR tries_i_ptr;
        tries_i = FOLLOW(tries_ptr+i+1); DEREF_NONVAR(tries_i);
        tries_i_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tries_i);

        X = FOLLOW(comp_vars_ptr+i+1);
        for (j = 0; j < n; j++) {
            if (i == j) continue;
            Y = FOLLOW(comp_vars_ptr+j+1);
            trie = FOLLOW(tries_i_ptr+j+1);  /* Tij */
            DEREF_NONVAR(trie);
            if (exclude_ac_unsupported_from_fd(X, Y, (BPLONG_PTR)UNTAGGED_ADDR(trie)) == BP_FALSE) return BP_FALSE;
        }
    }
    //  printf("<=b_CFD_REMOVE_AC_UNSUPPORTED %x %x ",local_top,heap_top); printf("\n");
    return BP_TRUE;
}

/* Called when X is instantiated. Constr = t(Y,TrieXY,TrieYX).
   Let Dy be Y's domain and Sx be X's supports.

   Dy := Dy /\ Sx
*/
int b_CFD_INS(X, Constr)
    BPLONG X, Constr;
{
    BPLONG Y, TrieXY, Sx;
    BPLONG_PTR ptr, trie_ptr, dv_ptr_sx, dv_ptr_y;

    /* printf("=>CFD_INS "); write_term(Constr); printf("\n"); */
    DEREF_NONVAR(X); X = INTVAL(X);
    DEREF_NONVAR(Constr);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(Constr);
    Y = FOLLOW(ptr+1);
    TrieXY = FOLLOW(ptr+2);
    DEREF_NONVAR(TrieXY);
    trie_ptr = (BPLONG_PTR)UNTAGGED_ADDR(TrieXY);
    Sx = FOLLOW(trie_ptr+X+1); DEREF_NONVAR(Sx);

    DEREF_NONVAR(Y);
    if (ISINT(Y)) {
        if (ISINT(Sx)) {
            return (Y == Sx);
        } else {  /* IS_SUSP_VAR(Sx) */
            dv_ptr_sx = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Sx);
            return dm_true(dv_ptr_sx, INTVAL(Y));
        }
    } else {
        dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
        if (ISINT(Sx)) {
            if (dm_true(dv_ptr_y, INTVAL(Sx))) {
                ASSIGN_DVAR(dv_ptr_y, Sx);
                return BP_TRUE;
            } else return BP_FALSE;
        } else {  /* IS_SUSP_VAR(Sx) */
            dv_ptr_sx = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Sx);
            return dm_intersect(dv_ptr_y, dv_ptr_sx);
        }
    }
}

/* Called when Ex is excluded from X. Constr = t(Y,TrieXY,TrieYX).
   Let Dy be Y's domain and Sx be X's supports. For each element y in Sx/\Dy, 
   exclude y from Dy if y is not supported by any value in X's domain.
*/
int b_CFD_DOM(X, Ex, Constr)
    BPLONG X, Ex, Constr;
{
    BPLONG y, Y, Sx, Sy, last, TrieXY, TrieYX;
    BPLONG_PTR ptr, trie_ptr, dv_ptr_sx, dv_ptr_sy, dv_ptr_x, dv_ptr_y;

    //  printf("=>cfd_mac_bin_dom %d",x); write_term(Y); printf(";;");write_term(TrieXY);printf(";;"); write_term(TrieYX); printf("\n");
    DEREF_NONVAR(X); if (ISINT(X)) return BP_TRUE;  /* will be taken care of by b_CFD_INS */

    DEREF_NONVAR(Constr);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(Constr);
    Y = FOLLOW(ptr+1); DEREF_NONVAR(Y);
    if (!IS_SUSP_VAR(Y)) return BP_TRUE;  /* will be taken care of by b_CFD_INS */
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    Ex = INTVAL(Ex);

    TrieXY = FOLLOW(ptr+2);
    DEREF_NONVAR(TrieXY);
    trie_ptr = (BPLONG_PTR)UNTAGGED_ADDR(TrieXY);
    Sx = FOLLOW(trie_ptr+Ex+1); DEREF_NONVAR(Sx);

    TrieYX = FOLLOW(ptr+3);
    DEREF_NONVAR(TrieYX);
    trie_ptr = (BPLONG_PTR)UNTAGGED_ADDR(TrieYX);

    if (ISINT(Sx)) {
        y = last = INTVAL(Sx);
    } else if (IS_SUSP_VAR(Sx)) {
        dv_ptr_sx = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Sx);
        y = DV_first(dv_ptr_sx);
        last = DV_last(dv_ptr_sx);
    }
    for (; ; ) {
        if (dm_true(dv_ptr_y, y)) {
            Sy = FOLLOW(trie_ptr+y+1);
            DEREF_NONVAR(Sy);
            if (ISINT(Sy)) {
                if (!dm_true(dv_ptr_x, INTVAL(Sy)))
                    if (domain_set_false_aux(dv_ptr_y, y) == BP_FALSE) return BP_FALSE;
            } else {
                dv_ptr_sy = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Sy);
                if (dm_disjoint(dv_ptr_x, dv_ptr_sy)) {
                    if (domain_set_false_aux(dv_ptr_y, y) == BP_FALSE) return BP_FALSE;
                }
            }
        }
        if (y == last) break;
        y++;
        if (!IS_IT_DOMAIN(dv_ptr_sx)) y = domain_next_bv(dv_ptr_sx, y);
    }
    return BP_TRUE;
}

int b_CFD_DIFF_TUPLE(Tuple, CompVars)
    BPLONG Tuple, CompVars;
{
    BPLONG n, i, compvar0, e0;
    BPLONG_PTR comp_vars_ptr, tuple_ptr, dv_ptr;
    SYM_REC_PTR sym_ptr;

    DEREF_NONVAR(Tuple);
    tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuple);
    sym_ptr = (SYM_REC_PTR)FOLLOW(tuple_ptr);
    n = GET_ARITY(sym_ptr);
    DEREF_NONVAR(CompVars);
    comp_vars_ptr = (BPLONG_PTR)UNTAGGED_ADDR(CompVars);
    compvar0 = 0;
    for (i = 1; i <= n; i++) {
        BPLONG compvar, e;
        compvar = FOLLOW(comp_vars_ptr+i); DEREF_NONVAR(compvar);
        e = FOLLOW(tuple_ptr+i); DEREF_NONVAR(e);
        if (e != compvar) {
            if (IS_SUSP_VAR(compvar)) {
                compvar0 = compvar;
                e0 = e;
            } else {
                return BP_TRUE;
            }
        }
    }
    if (compvar0 == 0) return BP_FALSE;
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar0);
    domain_set_false_noint(dv_ptr, INTVAL(e0));
    return BP_TRUE;
}

/* Let CompVars=t(a1,...,a(i-1),V,a(i+1),...,an) where V is the only variable remaining.
   Eor each element k in the domain of V, if t(a1,...,a(i-1),k,a(i+1),...,an) is not in the 
   hash table, then exclude k from the domain of V.
*/
int b_CFD_IN_FORWARD_CHECKING(HTable, CompVars)
    BPLONG CompVars, HTable;
{
    BPLONG i, j, k, last, n, compvar, htable_size;
    BPLONG_PTR tuple_ptr, comp_vars_ptr, htable_ptr, dv_ptr;
    SYM_REC_PTR sym_ptr;

    //  printf("=>_in_fc \n"); write_term(CompVars);  printf("\n"); write_term(HTable); printf("\n");
    DEREF_NONVAR(HTable);
    htable_ptr = (BPLONG_PTR)UNTAGGED_ADDR(HTable);
    sym_ptr = (SYM_REC_PTR)FOLLOW(htable_ptr);
    htable_size = GET_ARITY(sym_ptr);

    DEREF_NONVAR(CompVars);
    comp_vars_ptr = (BPLONG_PTR)UNTAGGED_ADDR(CompVars);
    sym_ptr = (SYM_REC_PTR)FOLLOW(comp_vars_ptr);
    n = GET_ARITY(sym_ptr);

    tuple_ptr = local_top-n-2;  /* space for a tuple */

    if (tuple_ptr-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 12");
    }

    FOLLOW(tuple_ptr) = (BPLONG)sym_ptr;
    compvar = 0;
    for (i = 1; i <= n; i++) {
        BPLONG t;
        t = FOLLOW(comp_vars_ptr+i); DEREF_NONVAR(t);
        if (ISINT(t)) {
            FOLLOW(tuple_ptr+i) = t;
        } else {
            compvar = t;
            j = i;
        }
    }
    if (compvar == 0) {
        if (htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) return BP_TRUE; else return BP_FALSE;
    }

    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
    k = DV_first(dv_ptr);
    last = DV_last(dv_ptr);

    for (; ; ) {
        FOLLOW(tuple_ptr+j) = MAKEINT(k);
        if (!htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) {
            if (domain_set_false_aux(dv_ptr, k) == BP_FALSE) return BP_FALSE;
        }

        if (k == last) break;  /* break for (;;) */
        k++;
        if (!IS_IT_DOMAIN(dv_ptr)) k = domain_next_bv(dv_ptr, k);
    }
    //  printf("<=_in_fc \n"); write_term(CompVars);  printf("\n");
    return BP_TRUE;
}

/* Let CompVars=t(a1,...,a(i-1),V,a(i+1),...,an) where V is the only variable remaining.
   Eor each element k in the domain of V, if t(a1,...,a(i-1),k,a(i+1),...,an) is in the negative 
   table, then exclude k from the domain of V.
*/
int b_CFD_NOTIN_FORWARD_CHECKING(HTable, CompVars)
    BPLONG CompVars, HTable;
{
    BPLONG i, j, k, last, n, compvar, htable_size;
    BPLONG_PTR tuple_ptr, comp_vars_ptr, htable_ptr, dv_ptr;
    SYM_REC_PTR sym_ptr;

    /*  printf("=>fc \n"); write_term(CompVars);  printf("\n"); write_term(HTable); printf("\n");  */
    DEREF_NONVAR(HTable);
    htable_ptr = (BPLONG_PTR)UNTAGGED_ADDR(HTable);
    sym_ptr = (SYM_REC_PTR)FOLLOW(htable_ptr);
    htable_size = GET_ARITY(sym_ptr);

    DEREF_NONVAR(CompVars);
    comp_vars_ptr = (BPLONG_PTR)UNTAGGED_ADDR(CompVars);
    sym_ptr = (SYM_REC_PTR)FOLLOW(comp_vars_ptr);
    n = GET_ARITY(sym_ptr);

    tuple_ptr = local_top-n-2;  /* space for a tuple */

    if (tuple_ptr-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "cfd 12");
    }

    FOLLOW(tuple_ptr) = (BPLONG)sym_ptr;
    compvar = 0;
    for (i = 1; i <= n; i++) {
        BPLONG t;
        t = FOLLOW(comp_vars_ptr+i); DEREF_NONVAR(t);
        if (ISINT(t)) {
            FOLLOW(tuple_ptr+i) = t;
        } else {
            compvar = t;
            j = i;
        }
    }
    if (compvar == 0) {
        if (htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) return BP_FALSE; else return BP_TRUE;
    }

    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(compvar);
    k = DV_first(dv_ptr);
    last = DV_last(dv_ptr);

    for (; ; ) {
        FOLLOW(tuple_ptr+j) = MAKEINT(k);
        if (htable_contains_tuple(htable_ptr, htable_size, tuple_ptr, n)) {
            if (domain_set_false_aux(dv_ptr, k) == BP_FALSE) return BP_FALSE;
        }

        if (k == last) break;  /* break for (;;) */
        k++;
        if (!IS_IT_DOMAIN(dv_ptr)) k = domain_next_bv(dv_ptr, k);
    }

    return BP_TRUE;
}

