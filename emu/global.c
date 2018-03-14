/********************************************************************
 *   File   : global.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018
 *   Purpose: Management of the global database

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include <stdlib.h>
#include "bprolog.h"
#include "dynamic.h"
/* extern char *malloc(); */

int b_IS_CONSULTED_c(goal)
    BPLONG goal;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
  
is_consulted:
    if (ISREF(goal)){
        NDEREF(goal,is_consulted);
        return 0;
    }
    if (ISSTRUCT(goal))
        sym_ptr = GET_STR_SYM_REC(goal);
    else if (ISATOM(goal))
        sym_ptr = GET_ATM_SYM_REC(goal);
    else 
        return 0;    
    return (GET_ETYPE(sym_ptr)==T_DYNA || GET_ETYPE(sym_ptr)==T_INTP);
}

int b_SET_DYNAMIC_cc(name,arity)
    BPLONG name,arity;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
  
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    GET_ETYPE(sym_ptr)=T_DYNA;
    return 1;
}

int b_GET_SYM_TYPE_ccf(name,arity,type)
    BPLONG name,arity,type;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;

    GET_GLOBAL_SYM(name,arity,sym_ptr);
    ASSIGN_f_atom(type,MAKEINT(GET_ETYPE(sym_ptr)));
    return 1;
}

int b_IS_DYNAMIC_cc(name,arity)
    BPLONG name,arity;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
  
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    return (GET_ETYPE(sym_ptr)==T_DYNA) ? BP_TRUE : BP_FALSE;
}

int b_IS_ORDINARY_cc(name,arity)
    BPLONG name,arity;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;

    GET_GLOBAL_SYM(name,arity,sym_ptr);
    return (GET_ETYPE(sym_ptr)==T_ORDI) ? 1 : 0;
}

int b_GLOBAL_SET_cccc(name,arity,value,part)
    BPLONG name,arity,value,part;
{
    return b_GLOBAL_SET_ccc(name,arity,value);
}

int b_GLOBAL_SET_ccc(name,arity,value)
    BPLONG name,arity,value;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG oldValue;
    BPLONG res;
  
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    if (GET_ETYPE(sym_ptr)==T_DYNA)
        oldValue = (BPLONG)GET_EP(sym_ptr);
    else {
        GET_ETYPE(sym_ptr) = T_DYNA;
        oldValue = nil_sym;
    }
    DEREF(value);
    if (TAG(value) == ATM) 
        GET_EP(sym_ptr) = (int (*)(void))value;
    else {
        release_term_space(oldValue);
        res = copy_term_heap_to_parea(value);
        if (res==BP_ERROR) return BP_ERROR;
        GET_EP(sym_ptr) = (int (*)(void))res;
    }
    return BP_TRUE;
}

int c_OLD_GLOBAL_SET(){
    BPLONG name,arity,value;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;
    BPLONG res;

    name = ARG(1,3);
    arity = ARG(2,3);
    value = ARG(3,3);
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    GET_ETYPE(sym_ptr) = T_DYNA;
    res = copy_term_heap_to_parea(value);
    if (res==BP_ERROR) return BP_ERROR;
    GET_EP(sym_ptr) = (int (*)(void))res;
    return BP_TRUE;
}
  
int c_OLD_GLOBAL_GET(){
    BPLONG name,arity,value;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;

    name = ARG(1,3);
    arity = ARG(2,3);
    value = ARG(3,3);
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    if (GET_ETYPE(sym_ptr)!=T_DYNA) {
        return 0;
    } else {
        return unify(value,(BPLONG)GET_EP(sym_ptr));
    }
}
  
int b_GLOBAL_GET_ccf(name,arity,value)
    BPLONG name,arity,value;
{
    BPLONG_PTR top,varVector;
    SYM_REC_PTR sym_ptr;
    BPLONG term;
    BPLONG maxVarNo= -1;

    GET_GLOBAL_SYM(name,arity,sym_ptr);
    if (GET_ETYPE(sym_ptr)!=T_DYNA) {
        return 0;
    }
    term = (BPLONG)GET_EP(sym_ptr);
    if (TAG(term)==ATM){
        ASSIGN_f_atom(value,term);
    } else {
        varVector = arreg-1000; /* safe? */
        ASSIGN_sv_heap_term(value,unnumberVarTerm(term,varVector,&maxVarNo)); 
        LOCAL_OVERFLOW_CHECK("global");
    }
    return 1;
}

int b_ISGLOBAL_cc(name,arity)
    BPLONG name,arity;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    return (GET_ETYPE(sym_ptr)!=T_DYNA) ? BP_FALSE : BP_TRUE;
}

int b_GLOBAL_DEL_cc(name,arity)
    BPLONG name,arity;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
  
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    GET_ETYPE(sym_ptr)= T_ORDI;
    return 1;
}

int c_SET_ARG_PAREA(){
    BPLONG no = ARG(1,3); 
    BPLONG s = ARG(2,3);
    BPLONG arg = ARG(3,3);
    BPLONG parea_arg;
    BPLONG res;
    res = copy_term_heap_to_parea(arg);
    if (res==BP_ERROR) return BP_ERROR;
    parea_arg = res;
    return b_DESTRUCTIVE_SET_ARG_ccc(no,s,parea_arg);
}
  
int b_GLOBAL_INSERT_HEAD_cccc(name,arity,value,part)
    BPLONG name,arity,value,part;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG temp1;
 
    GET_GLOBAL_SYM(name,arity,sym_ptr);
    /*  DEREF(part); part = INTVAL(part); */
    temp1 = copy_term_heap_to_parea(value);
    if (temp1==BP_ERROR) return BP_ERROR;
    GET_EP(sym_ptr) = (int (*)(void))make_cons_in_parea(temp1,(BPLONG)GET_EP(sym_ptr));
    if ((BPLONG)GET_EP(sym_ptr)==BP_ERROR) return BP_ERROR;
    return BP_TRUE;
}

int b_GLOBAL_INSERT_TAIL_ccc(name,arity,value)
    BPLONG name,arity,value;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG temp1,temp2;

    GET_GLOBAL_SYM(name,arity,sym_ptr);
    temp1 = copy_term_heap_to_parea(value);
    if (temp1==BP_ERROR) return BP_ERROR;
    temp2 = make_cons_in_parea(temp1,nil_sym);
    if (temp2==BP_ERROR) return BP_ERROR;   
    assert_tail(sym_ptr,temp2);
    return BP_TRUE;
}

void assert_tail(sym_ptr,term)
    SYM_REC_PTR sym_ptr;
    BPLONG term;
{
    BPLONG_PTR top,ptr;
    BPLONG op;
  
    if (ISNIL((BPLONG)GET_EP(sym_ptr)))
        GET_EP(sym_ptr) = (int (*)(void))term;
    else {
        op = (BPLONG)GET_EP(sym_ptr);
        DEREF(op);
        ptr = NULL;     
        while (ISLIST(op)){
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(op)+1;
            op = FOLLOW(ptr);
            DEREF(op);
        }
        FOLLOW(ptr) = term;
    }
}
  
BPLONG make_cons_in_parea(car,cdr)
    BPLONG car,cdr;
{
    BPLONG temp;
    BPLONG_PTR ptr;

    ALLOCATE_FROM_PAREA(ptr,2);
    if (ptr==NULL){
        exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    temp = (BPLONG)ADDTAG(ptr,LST);    
    *ptr++ = car;
    *ptr = cdr;
    return temp;
}  

BPLONG copy_term_heap_to_parea(value) 
    BPLONG value;
{
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;
    BPLONG temp,varno=1;

    local_top0 = local_top;
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    temp = numberVarCopyToParea(value,&varno);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    local_top = local_top0;
    if (temp==BP_ERROR) return BP_ERROR;
    return temp;
}

BPLONG copy_term_heap_to_parea_with_varno(value,varno) 
    BPLONG value;
    BPLONG *varno;
{
    BPLONG_PTR trail_top0;
    BPLONG temp;
    BPLONG initial_diff0;

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    local_top0 = local_top;
    temp = numberVarCopyToParea(value,varno);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    local_top = local_top0;
    if (temp==BP_ERROR) return BP_ERROR;
    return temp;
}
  

int c_UNNUMBER_VARS(){
    BPLONG term,unnumberedTerm,temp;
    BPLONG_PTR varVector;
    BPLONG maxVarNo = -1;

    varVector = arreg-NONDET_FRAME_SIZE;

    term = ARG(1,2);
    unnumberedTerm = ARG(2,2); 
    temp = unnumberVarTerm(term,varVector,&maxVarNo);
    if (local_top - heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW,"uv");
    }
    if (temp == -1){
        exception = illegal_arguments;
        return BP_ERROR;
    }
    return unify(temp,unnumberedTerm);
}
  

     
  
  


