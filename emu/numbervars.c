/********************************************************************
 *   File   : numbervars.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: number_vars, copy_term, etc.

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 ********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "basic.h"
#include "term.h"
#include "bapi.h"

#define NUMVARS_MAX 26

SYM_REC_PTR dollar_var, dollar_float, dollar_and;

void Cboot_numbervars()
{
    dollar_var = BP_NEW_SYM("$VAR", 1);
    dollar_float = BP_NEW_SYM("$float", 3);
    dollar_and = BP_NEW_SYM(",", 2);
    insert_cpred("vars_set", 2, c_VARS_SET);
    insert_cpred("c_VARS_SET_INTERSECT", 3, c_VARS_SET_INTERSECT);
    insert_cpred("c_SINGLETON_VARS", 3, c_SINGLETON_VARS);
}

/************ Primitives on numbered terms *********/
void numberVarTermOpt(term)
    BPLONG term;
{
    BPLONG_PTR top, ptr;
    BPLONG arity, i;

    SWITCH_OP(term, start,
              {ASSIGN_TRAIL_VALUE(term, NumberVar(global_var_num));
                  global_var_num++;},
              {},
              {if (IsNumberedVar(term)) return;
                  ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
                  if (!IS_HEAP_REFERENCE(ptr)) return;
                  numberVarTermOpt(*ptr);
                  term = *(ptr+1);
                  goto start;},
              {ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
                  if (!IS_HEAP_REFERENCE(ptr)) return;
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
                  for (i = 1; i <= arity; i++) {
                      numberVarTermOpt(*(ptr + i));
                  }
              },
              {number_var_exception = 1;
                  ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
                  PUSHTRAILC_ATOMIC(ptr, term);
                  FOLLOW(ptr) = NumberVar(global_var_num);
                  global_var_num++;
              });
}

/* make sure there is enough space on the heap befor calling this function */
BPLONG unnumberVarTermOpt(term)
    BPLONG term;
{
    BPLONG_PTR ptr;
    BPLONG varNo;
    BPLONG_PTR top, term_ptr;
    BPLONG i, arity;
    SYM_REC_PTR sym_ptr;

unnumberVarTermSwitch:
    switch (TAG(term)) {
    case REF:
        NDEREF(term, unnumberVarTermSwitch);

    case ATM: return term;

    case LST:
        if (IsNumberedVar(term)) {
            varNo = INTVAL(term);
            if (varNo > global_unnumbervar_max) {
                global_unnumbervar_max = varNo;
                global_unnumbervar_watermark = global_unnumbervar_ptr-global_unnumbervar_max;
                FOLLOW(global_unnumbervar_ptr-varNo) = (BPLONG)heap_top;
                NEW_HEAP_FREE;
                return FOLLOW(global_unnumbervar_ptr-varNo);
            } else {
                return FOLLOW(global_unnumbervar_ptr-varNo);
            }
        } else {
            return unnumberVarListOpt(term);
        }

    case STR:
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
        if (sym_ptr == comma_psc) {
            return unnumberVarCommaOpt(term);
        }
        arity = GET_ARITY(sym_ptr);
        ptr = heap_top; heap_top += arity+1;

        if (global_unnumbervar_watermark-heap_top <= LARGE_MARGIN) {
            myquit(STACK_OVERFLOW, "uv");
        }

        FOLLOW(ptr) = (BPLONG)sym_ptr;
        for (i = 1; i <= arity; i++) {
            term = FOLLOW(term_ptr+i);
            if (TAG(term) == ATM) {
                FOLLOW(ptr+i) = term;
            } else {
                FOLLOW(ptr+i) = unnumberVarTermOpt(term);
            }
        }
        return ADDTAG(ptr, STR);
    default:
        return(0);  /* impossible to come here */
    }
}

/* term in the form of [a,b,...] */
BPLONG unnumberVarListOpt(term)
    BPLONG term;
{
    BPLONG_PTR term_ptr, ret_term_ptr, ptr;
    BPLONG ret_term;

    ret_term_ptr = &ret_term;
    do {
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        ptr = heap_top; heap_top += 2;

        if (global_unnumbervar_watermark-heap_top <= LARGE_MARGIN) {
            myquit(STACK_OVERFLOW, "uv");
        }

        FOLLOW(ret_term_ptr) = ADDTAG(ptr, LST);
        term = FOLLOW(term_ptr);
        if (TAG(term) == ATM) {
            FOLLOW(ptr) = term;
        } else {
            FOLLOW(ptr) = unnumberVarTermOpt(term);
        }
        term = FOLLOW(term_ptr+1);
        ret_term_ptr = ptr+1;
    } while (ISLIST(term));
    if (term == nil_sym) {
        FOLLOW(ret_term_ptr) = nil_sym;
    } else {
        FOLLOW(ret_term_ptr) = unnumberVarTermOpt(term);
    }
    return ret_term;
}

/* term in the form of (a,b,...) */
BPLONG unnumberVarCommaOpt(term)
    BPLONG term;
{
    BPLONG_PTR term_ptr, ptr, ret_term_ptr;
    BPLONG ret_term;

    ret_term_ptr = &ret_term;
loop:
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    if (ISSTRUCT(term) && FOLLOW(term_ptr) == (BPLONG)comma_psc) {
        ptr = heap_top; heap_top += 3;

        if (global_unnumbervar_watermark-heap_top <= LARGE_MARGIN) {
            myquit(STACK_OVERFLOW, "uv");
        }

        FOLLOW(ret_term_ptr) = ADDTAG(ptr, STR);
        FOLLOW(ptr) = (BPLONG)comma_psc;
        term = FOLLOW(term_ptr+1);
        if (TAG(term) == ATM) {
            FOLLOW(ptr+1) = term;
        } else {
            FOLLOW(ptr+1) = unnumberVarTermOpt(term);
        }
        ret_term_ptr = ptr+2;
        term = FOLLOW(term_ptr+2);
        goto loop;
    } else {
        FOLLOW(ret_term_ptr) = unnumberVarTermOpt(term);
    }
    return ret_term;
}

BPLONG unnumberVarTerm(term, varVector, maxVarNo)
    BPLONG term;
    BPLONG_PTR varVector;
    BPLONG_PTR maxVarNo;
{
    BPLONG res;

    global_unnumbervar_ptr = varVector;
    global_unnumbervar_max = *maxVarNo;
    global_unnumbervar_watermark = global_unnumbervar_ptr-global_unnumbervar_max;
    res = unnumberVarTermOpt(term);
    if (local_top-heap_top <= LARGE_MARGIN) {
        myquit(STACK_OVERFLOW, "tb");
    }
    *maxVarNo = global_unnumbervar_max;
    return res;
}

int c_NUMBER_VARS() {
    register BPLONG op1, op2, op3;
    register BPLONG_PTR top;

    op1 = ARG(1, 3); op2 = ARG(2, 3); op3 = ARG(3, 3);
    DEREF(op2);
    if (!ISINT(op2)) {
        bp_exception = c_type_error(et_INTEGER, op2); return BP_ERROR;
    }
    op2 = INTVAL(op2);
    if (op2 < 0) {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    op1 = aux_number_vars__3(op1, op2);
    if (op1 == BP_ERROR) {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    return unify(op3, MAKEINT(op1));
}

int aux_number_vars__3(op1, n0)
    BPLONG op1;
    BPLONG n0;
{
    BPLONG_PTR sreg;
    BPLONG op4;
    BPLONG i, arity;
    BPLONG_PTR top;

    LOCAL_OVERFLOW_CHECK("number_vars");
label1:
number_vars:
    switch (TAG(op1)) {
    case REF:
        NDEREF(op1, number_vars);
        ASSIGN_STRUCT(op1, dollar_var);
        NEW_HEAP_NODE(MAKEINT(n0));
        return n0+1;
        break;
    case ATM:
        return n0;
    case LST:
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op4 = *sreg++;
        op1 = *sreg;
        n0 = aux_number_vars__3(op4, n0);
        if (n0 == BP_ERROR) return BP_ERROR;
        goto label1;
    case STR:
        if (op1 < 0) {
            fprintf(stderr, "Suspension variables cannot occur in number_vars/3!\n");
            bp_exception = illegal_arguments;
            return BP_ERROR;
        }
        UNTAG_ADDR(op1);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op1));
        for (i = 1; i <= arity; i++) {
            n0 = aux_number_vars__3(*((BPLONG_PTR)op1+i), n0);
            if (n0 == BP_ERROR) return BP_ERROR;
        }
        return n0;
    }
    return BP_TRUE;  /* unreachable */
}

int c_COPY_TERM() {
    BPLONG term, cterm, temp;

    term = ARG(1, 2); cterm = ARG(2, 2);
    temp = copy_term(term);
    if (temp == BP_FALSE) return BP_FALSE;
    return unify(cterm, temp);
}

/* for a compound term, only copy the skelton, not the arguments */

int c_COPY_TERM_SHALLOW() {
    BPLONG term, cterm, temp;

    term = ARG(1, 2); cterm = ARG(2, 2);
    DEREF(term);

copy_term_shallow:
    switch (TAG(term)) {
    case REF:
        NDEREF(term, copy_term_shallow);
        return BP_TRUE;

    case ATM:
        return unify(cterm, term);

    case LST: {
        BPLONG_PTR ptr;

        ptr = heap_top;
        if (heap_top+2 >= local_top) {
            myquit(STACK_OVERFLOW, "cp");
        }
        NEW_HEAP_FREE;
        NEW_HEAP_FREE;
        return unify(cterm, ADDTAG(ptr, LST));
    }

    case STR:{
        BPLONG_PTR term_ptr, ptr;
        SYM_REC_PTR sym_ptr;
        BPLONG i, arity;

        if (IS_SUSP_VAR(term)) {
            return BP_TRUE;
        }
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
        arity = GET_ARITY(sym_ptr);
        ptr = heap_top;
        if (heap_top+arity >= local_top) {
            myquit(STACK_OVERFLOW, "cp");
        }
        NEW_HEAP_NODE((BPLONG)sym_ptr);
        for (i = 1; i <= arity; i++) {
            NEW_HEAP_FREE;
        }
        return unify(cterm, ADDTAG(ptr, STR));
    }

    default:
        return(0);  /* impossible to come here */
    }
}

BPLONG copy_term(term)
    BPLONG term;
{
    BPLONG_PTR varVector, trail_top0;
    BPLONG initial_diff0;
    BPLONG maxVarNo = -1;
    BPLONG temp;

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    varVector = local_top;

    PREPARE_NUMBER_TERM(0);
    numberVarTermOpt(term);
    if (number_var_exception != 0) {
        trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
        UNDO_TRAILING;
        return BP_FALSE;
    }
    temp = unnumberVarTerm(term, varVector, &maxVarNo);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;

    return temp;
}


int c_VARS_SET() {
    BPLONG term, set, margin;
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;

    /* printf("vars_set local_top=%x heap_top=%x\n",local_top,heap_top); */

    term = ARG(1, 2);

    // printf("=>vars_set ");  write_term(term); printf("\n");

    local_top0 = local_top;
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    vars_set_extract_vars(term, nil_sym);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    set = nil_sym;

    margin = 2*(local_top0 - local_top)+100;
    //  printf("margin=%d\n",margin);
    if (local_top - heap_top <= margin) {
        myquit(STACK_OVERFLOW, "vars_set 0");
    }
    while (local_top < local_top0) {
        BPLONG tmp_term;
        local_top++;
        tmp_term = FOLLOW(local_top);
        if ((BPLONG_PTR)tmp_term > heap_top){  /* a stack var */
            FOLLOW(heap_top) = (BPLONG)heap_top;
            PUSHTRAIL(tmp_term);
            FOLLOW(tmp_term) = (BPLONG)heap_top;
        } else {
            FOLLOW(heap_top) = FOLLOW(local_top);
        }
        FOLLOW(heap_top+1) = set;
        set = ADDTAG(heap_top, LST);
        heap_top += 2;
    }
    // printf("<=vars_set "); write_term(set); printf("\n");
    return unify(ARG(2, 2), set);
}

/* find the set of variables in both terms: ex_term and term, ex_term must be part of term */
int c_VARS_SET_INTERSECT() {
    BPLONG term, ex_term, set;
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;

    //  printf("vars_set_intersect local_top=%x heap_top=%x\n",local_top,heap_top);
    term = ARG(1, 3);
    ex_term = ARG(2, 3);
    DEREF(ex_term);

    //  printf("term = "); write_term(term); printf("\n");
    //  printf("ex_term = "); write_term(ex_term); printf("\n");

    if (ISREF(ex_term)) {
        FOLLOW(heap_top) = ex_term;
        FOLLOW(heap_top+1) = nil_sym;
        set = ADDTAG(heap_top, LST);
        heap_top += 2;
        return unify(ARG(3, 3), set);
    }

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    local_top0 = local_top;
    vars_set_extract_vars(term, ex_term);  /* vars stored in local_top0 .. local_top+1 */
    set = collect_shared_vars(ex_term, nil_sym);
    local_top = local_top0;
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    return unify(ARG(3, 3), set);
}

/* extract the variables in term but not ex_term */
void vars_set_extract_vars(term, ex_term)
    BPLONG term, ex_term;
{
start:
    if (term == ex_term || TAG(term) == ATM ) return;
    if (ISREF(term)) {
        if ((BPULONG)term > (BPULONG)heap_top && (BPULONG)term <= (BPULONG)local_top0) return;  /* marked already */
        if (ISFREE(term)) {
            FOLLOW(local_top) = term;
            ASSIGN_TRAIL_VALUE(term, (BPLONG)local_top);  /* form a cycle */
            local_top--;
            LOCAL_OVERFLOW_CHECK("vars_set 1");
            return;
        }
        term = FOLLOW(term);
        goto start;
    } else if (ISLIST(term)) {
        UNTAG_ADDR(term);
        vars_set_extract_vars(*(BPLONG_PTR)term, ex_term);
        term = *((BPLONG_PTR)term+1);
        goto start;
    } else if (ISSTRUCT(term)) {
        BPLONG_PTR struct_ptr;
        BPLONG i, arity;

        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(struct_ptr));
        for (i = 1; i <= arity-1; i++) {
            vars_set_extract_vars(FOLLOW(struct_ptr + i), ex_term);
        }
        term = FOLLOW(struct_ptr+arity);
        goto start;
    } else {
        BPLONG_PTR ptr;
        BPLONG_PTR susp_var_ptr;

        susp_var_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
        FOLLOW(local_top) = (BPLONG)susp_var_ptr;
        PUSHTRAILC(ADDTAG3(susp_var_ptr, TRAIL_VAL_NONATOMIC), term);
        FOLLOW(susp_var_ptr) = (BPLONG)local_top;  /* form a cycle */
        local_top--;
        LOCAL_OVERFLOW_CHECK("vars_set 2");
    }
}

/* collect the variables that are already extracted in local_top0..local_top and that also occur in term */
BPLONG collect_shared_vars(term, list0)
    BPLONG term, list0;
{
start:
    if (TAG(term) == ATM ) return list0;
    if (ISREF(term)) {
        if ((BPULONG)term > (BPULONG)heap_top && (BPULONG)term <= (BPULONG)local_top0) {  /* marked */
            if (FOLLOW(term) != nil_sym) {
                FOLLOW(heap_top) = FOLLOW(term);
                FOLLOW(heap_top+1) = list0;
                list0 = ADDTAG(heap_top, LST);
                FOLLOW(term) = nil_sym;  /* mark it as collected */
                heap_top += 2;
                LOCAL_OVERFLOW_CHECK("vars_set 3");
            }  /* else the var has been collected */
            return list0;
        }
        if (ISFREE(term)) {  /* this was not marked before */
            return list0;
        }
        term = FOLLOW(term);
        goto start;
    } else if (ISLIST(term)) {
        UNTAG_ADDR(term);
        list0 = collect_shared_vars(*(BPLONG_PTR)term, list0);
        term = *((BPLONG_PTR)term+1);
        goto start;
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity;
        BPLONG_PTR struct_ptr;

        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(struct_ptr));
        for (i = 1; i <= arity-1; i++) {
            list0 = collect_shared_vars(*(struct_ptr + i), list0);
        }
        term = FOLLOW(struct_ptr+arity);
        goto start;
    } else {  /* susp var */
        return list0;
    }
}

/* get the list of names of the singleton variables in term */
int c_SINGLETON_VARS() {
    BPLONG dict, term, vars, pair, name, var;
    BPLONG_PTR trail_top0, top, ptr;
    char *name_ptr;
    SYM_REC_PTR sym_ptr;
    BPLONG initial_diff0;

    dict = ARG(1, 3);  /* [(Name=Var,...,Name=Var)] */
    term = ARG(2, 3);
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    local_top0 = local_top;

    vars_set_extract_singleton_vars(term);  /* vars stored in local_top0 .. local_top+1 */

    vars = nil_sym;
    DEREF(dict);
    while (ISLIST(dict)) {
        pair = GET_CAR(dict);
        dict = GET_CDR(dict);
        DEREF(pair);
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
        name = FOLLOW(ptr+1); DEREF(name);
        var = FOLLOW(ptr+2);
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(name);
        name_ptr = GET_NAME(sym_ptr);
        if (is_marked_as_singleton(var) && *name_ptr != '_') {
            FOLLOW(heap_top) = pair;
            FOLLOW(heap_top+1) = vars;
            vars = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("singleton_vars");
        }
    }
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    local_top = local_top0;
    return unify(ARG(3, 3), vars);
}

/* extract the variables in term but not ex_term */
void vars_set_extract_singleton_vars(term)
    BPLONG term;
{
    //  printf("local_top=%x heap_top=%x\n",local_top,heap_top);

start:
    if (TAG(term) == ATM ) return;
    if (ISREF(term)) {
        if ((BPULONG)term > (BPULONG)heap_top && (BPULONG)term < (BPULONG)arreg) {
            FOLLOW(term) = nil_sym;  /* visited second time */
            return;
        }
        if (ISFREE(term)) {
            FOLLOW(local_top) = term;
            ASSIGN_TRAIL_VALUE(term, (BPLONG)local_top);  /* form a cycle */
            local_top--;
            LOCAL_OVERFLOW_CHECK("vars_set 4");
            return;
        }
        term = FOLLOW(term);
        goto start;
    } else if (ISLIST(term)) {
        UNTAG_ADDR(term);
        vars_set_extract_singleton_vars(*(BPLONG_PTR)term);
        term = *((BPLONG_PTR)term+1);
        goto start;
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity;
        BPLONG_PTR struct_ptr;
        UNTAG_ADDR(term);
        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(struct_ptr));
        for (i = 1; i <= arity-1; i++) {
            vars_set_extract_singleton_vars(FOLLOW(struct_ptr+i));
        }
        term = FOLLOW(struct_ptr+arity);
        goto start;
    } else {
        bp_exception = illegal_arguments;
        fprintf(stderr, "Suspension variables cannot occur in copy_term/2, number_vars/3, assert/1, vars_set/2, or tabled calls!\n");
        exit(1);
    }
}


/* a variable is marked as singleton if it refers to local_top-1..local_top0 and the content is not nil_sym */
int is_marked_as_singleton(var)
    BPLONG var;
{
    while (ISREF(var)) {
        if ((BPULONG)var < (BPULONG)arreg && (BPULONG)var > (BPULONG)heap_top)
            return (FOLLOW(var) != nil_sym);
        if (ISFREE(var)) {  /* unlikely, test it anyway*/
            return 0;
        }
        var = FOLLOW(var);  /* dereference */
    }
    return 1;
}
