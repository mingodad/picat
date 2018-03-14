/********************************************************************
 *   File   : cpreds.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018
 *   Purpose: Implementation of =../2

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "term.h"
#include "bapi.h"

BPLONG univ_lst2str(L)
    BPLONG L;
{
    BPLONG temp,Head,top1,orig_L;
    SYM_REC_PTR     psc_ptr;
    BPLONG_PTR top;
    BPLONG n;

    orig_L = L;
    DEREF(L);
    if (!ISLIST(L)){
        if (ISNIL(L)){
            exception = c_domain_error(et_NON_EMPTY_LIST,L);
        } else if (ISREF(L)) {
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_LIST,L);
        }
        return BP_ERROR;
    }
    UNTAG_ADDR(L);
    Head = *(BPLONG_PTR)L;
    DEREF(Head);
    L = *((BPLONG_PTR)L+1);
    DEREF(L);

    if (ISLIST(L)){
        if (ISREF(Head)){
            exception = et_INSTANTIATION_ERROR;
            return BP_ERROR;    
        } else if (!ISATOM(Head)){
            exception = c_type_error(et_ATOM,Head);
            return BP_ERROR;
        }
    } else if (ISNIL(L)){          /* ATM */
        if (ISATOM(Head) || ISNUM(Head)){
            return Head;
        }
        if (ISREF(Head)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_ATOMIC,Head);
        }
        return BP_ERROR;
    } else if (ISREF(L) || IS_SUSP_VAR(L)){
        exception = et_INSTANTIATION_ERROR;
        return BP_ERROR;
    } else {
        exception = c_type_error(et_LIST,orig_L);
        return BP_ERROR;
    }
    
    n = list_length(L,orig_L);
    if (n<0) return BP_ERROR; /* exception set */
    if (n>MAX_ARITY){
        exception = c_representation_error(et_MAX_ARITY);
        return BP_ERROR;
    }
    if (local_top-heap_top <= n + LARGE_MARGIN) {
        myquit(STACK_OVERFLOW,"univ"); 
    }
    if (Head == period_sym && n==2){ /* LST */
        top1 = ADDTAG(heap_top,LST);
    } else {
        top1 = ADDTAG(heap_top,STR);  /* STR */
        psc_ptr = GET_ATM_SYM_REC(Head);
        FOLLOW(heap_top) = (BPLONG)insert_sym(GET_NAME(psc_ptr),GET_LENGTH(psc_ptr),n);
        heap_top++;
    }
restart:
    while (ISLIST(L)){
        UNTAG_ADDR(L);
        temp = *(BPLONG_PTR)L;
        L = *((BPLONG_PTR)L+1);
        NEW_HEAP_NODE(temp);
    }
    DEREF(L); if (ISLIST(L)) goto restart;
    return top1;
}


BPLONG univ_str2lst(op1)
    BPLONG op1;
{
    BPLONG temp,tempL,Head;
    SYM_REC_PTR     psc_ptr;
    BPLONG_PTR top;
    BPLONG_PTR ptr;
    BPLONG i,n;

    DEREF(op1);
    switch (TAG(op1)) {
    case REF:
        exception = et_INSTANTIATION_ERROR;
        return BP_ERROR;
    case ATM:
        n = 0;
        Head = op1;
        ptr = NULL;                                                                                                             
        break;
    case LST:
        n = 2;
        Head = period_sym;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1)-1;
        break;
    case STR:
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        psc_ptr = (SYM_REC_PTR)FOLLOW(ptr);
        if (psc_ptr==bigint_psc || psc_ptr==float_psc){
            n = 0;
            Head = op1;
        } else {
            n = GET_ARITY(psc_ptr);
            Head = ADDTAG(insert_sym(GET_NAME(psc_ptr),GET_LENGTH(psc_ptr),0),ATM);
        }
        break;
    default:                                                                                                            
        fprintf(stderr,"No case for op1 in univ_str2lst()\n");                                                          
        n = 0;                                                                                                          
        ptr = NULL;                                                                                                             
        Head = 0;                                                                                                               
    }
    if (local_top-heap_top <= 2*n + LARGE_MARGIN) {
        myquit(STACK_OVERFLOW,"univ"); 
    }
    tempL = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(Head);
    for (i = 1; i<=n; i++){
        temp = ADDTAG(((BPLONG_PTR)heap_top+1),LST);
        NEW_HEAP_NODE(temp);
        NEW_HEAP_NODE(FOLLOW(ptr+i));
    }
    NEW_HEAP_NODE(nil_sym);
    return tempL;
}

int list_length(L,orig_L)
    BPLONG L,orig_L;
{
    BPLONG_PTR top;
    int i;
    i = 0;
restart:
    while (ISLIST(L)){
        i++;
        L = *((BPLONG_PTR)UNTAGGED_ADDR(L)+1);
    }
    DEREF(L); if (ISLIST(L)) goto restart;
    if (!ISNIL(L)){
        if (ISREF(L)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_LIST,orig_L);
        }
        return BP_ERROR;
    }
    return i;
}


int b_UNIV_cc(op1,op2)
    BPLONG op1,op2;
{
    BPLONG_PTR top;
    BPLONG tmp_op;
    DEREF(op1);
    if (ISREF(op1) || IS_SUSP_VAR(op1)){
        tmp_op = univ_lst2str(op2);
        if (tmp_op==BP_ERROR) return BP_ERROR;
        return unify(op1,tmp_op);
    } else {
        tmp_op = univ_str2lst(op1);
        if (tmp_op==BP_ERROR) return BP_ERROR;
        return unify(tmp_op,op2);
    }
}

/*
  b_UNIV_cf(op1,op2)
  BPLONG op1,op2;
  {
  return unify(univ_str2lst(op1),op2);
  }

  b_UNIV_fc(op1,op2)
  BPLONG op1,op2;
  {
  return unify(op1,univ_lst2str(op2));
  }
*/

