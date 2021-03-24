/********************************************************************
 *   File   : findall.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021
 *   Purpose: memory manager for the faa (find-all-area) used by findall/setof/bagof

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>
#include "bprolog.h"
#include "gc.h"
/*
  defined in basic.h
  typedef struct {
  BPLONG_PTR low_addr,up_addr,top;
  GTERMS_HTABLE_PTR gterms_htable_ptr;
  } NUMBERED_TERM_AREA_RECORD;
*/

NUMBERED_TERM_AREA_RECORD faa_record;
NUMBERED_TERM_AREA_RECORD_PTR faa_record_ptr;

BPLONG findall_no = 0;

void init_findall_area() {
    findall_no = 0;
    faa_record_ptr = &faa_record;
    faa_record_ptr->low_addr = NULL;
    faa_record_ptr->num_expansions = 0;
}

/*
  not used anymore.
*/
int b_FINDALL_COPY_ARGS() {
    return BP_TRUE;
}

/* c_findall_pre(SymPtr): return sym_ptr to '$findall_result'/findall_no as an integer */
int c_findall_pre() {
    SYM_REC_PTR sym_ptr;
    int success;

    if (faa_record_ptr->low_addr == NULL) {
        ADD_NEW_NUMBERED_TERM_AREA_BLOCK(faa_record_ptr, success);
        if (!success) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
    }
    //  printf("=> find_all pre\n");
    switch (findall_no) {
    case 0: sym_ptr = findall_result0; break;
    case 1: sym_ptr = findall_result1; break;
    case 2: sym_ptr = findall_result2; break;
    default:
        sym_ptr = insert_sym("$findall_result", 15, findall_no);
    }
    findall_no++;
    GET_EP(sym_ptr) = (int (*)(void))nil_sym;
    GET_ETYPE(sym_ptr) = T_DYNA;
    return unify(ARG(1, 1), ADDTAG((BPLONG)sym_ptr, INT_TAG));
}

/* c_findall_post(SymPtr): deallocate memory but the frist block if findall_no==0 */
int c_findall_post() {
    BPLONG SymPtr;
    SYM_REC_PTR sym_ptr;
    int i;
    //  printf("=> find_all post\n");
    SymPtr = ARG(1, 1);

    DEREF_NONVAR(SymPtr);
    sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(SymPtr);
    GET_ETYPE(sym_ptr) = T_ORDI;
    GET_EP(sym_ptr) = (int (*)(void))nil_sym;

    findall_no--;

    /* release FAA blocks but the first block */
    if (findall_no == 0) {
        BPLONG_PTR faa_low_addr, prev_faa_low_addr;
        faa_low_addr = faa_record_ptr->low_addr;
        prev_faa_low_addr = (BPLONG_PTR)FOLLOW(faa_low_addr);
        while (prev_faa_low_addr != NULL) {
            free(faa_low_addr);
            faa_low_addr = prev_faa_low_addr;
            prev_faa_low_addr = (BPLONG_PTR)FOLLOW(faa_low_addr);
        };
        faa_record_ptr->low_addr = faa_low_addr;
        faa_record_ptr->top = faa_low_addr+1;

        faa_record_ptr->up_addr = faa_low_addr+NUMBERED_TERM_BLOCK_SIZE;
    }
    return BP_TRUE;
}

int b_FINDALL_INSERT_cc(BPLONG SymPtr, BPLONG value)
{
    SYM_REC_PTR sym_ptr;

    DEREF_NONVAR(SymPtr);
    sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(SymPtr);

    DEREF(value);
    if (TAG(value) == ATM) goto after_copy;
    value = copy_term_heap_to_faa(value);
    if (value == BP_ERROR) return BP_ERROR;
after_copy:
    value = make_cons_in_faa(value, (BPLONG)GET_EP(sym_ptr));
    if (value == BP_ERROR) return BP_ERROR;

    GET_EP(sym_ptr) = (int (*)(void))value;

    return BP_TRUE;
}

int c_FINDALL_AREA_SIZE() {
    return unify(ARG(1, 1), MAKEINT(numbered_area_size(faa_record_ptr)));
}

BPLONG numbered_area_size(NUMBERED_TERM_AREA_RECORD_PTR area_record_ptr) {
    BPLONG size;
    BPLONG_PTR prev_faa_low_addr;

    size = area_record_ptr->top - area_record_ptr->low_addr;
    prev_faa_low_addr = (BPLONG_PTR)FOLLOW((area_record_ptr->low_addr));
    while (prev_faa_low_addr != NULL) {
        size += NUMBERED_TERM_BLOCK_SIZE;
        prev_faa_low_addr = (BPLONG_PTR)FOLLOW(prev_faa_low_addr);
    }
    return size;
}

/* enough heap space has been secured */
int c_FINDALL_GET()
{
    SYM_REC_PTR sym_ptr;
    BPLONG SymPtr, Answers;

    SymPtr = ARG(1, 2);
    DEREF_NONVAR(SymPtr);
    sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(SymPtr);
    //  printf("findall_no=%x\n",findall_no);
    Answers = copy_answers_faa_to_heap((BPLONG)GET_EP(sym_ptr));
    return unify(ARG(2, 2), Answers);
}

BPLONG copy_answers_faa_to_heap(BPLONG list) {
    BPLONG_PTR list_ptr;
    BPLONG list_cp = nil_sym;

    while (ISLIST(list)) {
        BPLONG car;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        PREPARE_UNNUMBER_TERM(local_top);
        car = unnumberVarTermOpt(FOLLOW(list_ptr));
        FOLLOW(heap_top) = car;
        FOLLOW(heap_top+1) = list_cp;
        list_cp = ADDTAG(heap_top, LST);
        heap_top += 2;
        if (local_top-heap_top <= LARGE_MARGIN) {
            myquit(STACK_OVERFLOW, "fan");
        }

        list = FOLLOW(list_ptr+1);
    }
    return list_cp;
}

BPLONG copy_term_heap_to_faa(BPLONG value)
{
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;
    BPLONG temp;

    local_top0 = local_top;
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    PREPARE_NUMBER_TERM(0);
    temp = numberVarCopyToFindallArea(faa_record_ptr, value);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    local_top = local_top0;

    return temp;
}

int check_ground_using_faa(BPLONG term) {
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;
    BPLONG temp;
    int success;

    if (faa_record_ptr->low_addr == NULL) {
        ADD_NEW_NUMBERED_TERM_AREA_BLOCK(faa_record_ptr, success);
        if (!success) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
    }

    local_top0 = local_top;
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    PREPARE_NUMBER_TERM(0);
    numberVarCopyToFindallArea(faa_record_ptr, term);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    local_top = local_top0;

    return (global_var_num == 0) ? BP_TRUE : BP_FALSE;
}


BPLONG make_cons_in_faa(BPLONG car, BPLONG cdr)
{
    BPLONG temp;
    BPLONG_PTR ptr;

    ALLOCATE_FROM_NUMBERED_TERM_AREA(faa_record_ptr, ptr, 2);
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    temp = (BPLONG)ADDTAG(ptr, LST);
    *ptr++ = car;
    *ptr = cdr;
    return temp;
}

/*
  Copies term from the heap to the findall area.
*/
BPLONG numberVarCopyToFindallArea(NUMBERED_TERM_AREA_RECORD_PTR faa_record_ptr, BPLONG term)
{
    BPLONG_PTR term_ptr, dest_ptr;
    BPLONG_PTR top;
    BPLONG term_cp, term_cp1;
    BPLONG i, arity, size;
    SYM_REC_PTR sym_ptr;

l_number_var_copy_faa:
    switch(TAG(term)) {
    case REF:
        NDEREF(term, l_number_var_copy_faa);
        ASSIGN_TRAIL_VALUE(term, NumberVar(global_var_num));
        global_var_num++;
        return FOLLOW(term);

    case ATM:
        return term;

    case LST:
        if (IsNumberedVar(term)) {
            return term;
        } else {
            return numberVarCopyListToFindallArea(faa_record_ptr, term);
        }

    case STR:
        if (term > 0) {
            term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
            arity = GET_ARITY(sym_ptr);
            size = arity+1;
            ALLOCATE_FROM_NUMBERED_TERM_AREA(faa_record_ptr, dest_ptr, size);
            if (dest_ptr == NULL) {
                bp_exception = et_OUT_OF_MEMORY;
                return BP_ERROR;
            }
            FOLLOW(dest_ptr) = FOLLOW(term_ptr);

            /* copy first argument */
            term = FOLLOW(term_ptr+1);
            if (TAG(term) == ATM) {
                term_cp = term;
            } else {
                term_cp = numberVarCopyToFindallArea(faa_record_ptr, FOLLOW(term_ptr+1));
                if (term_cp == BP_ERROR) return BP_ERROR;
            }
            FOLLOW(dest_ptr+1) = term_cp;
            for (i = 2; i <= arity; i++) {
                term = FOLLOW(term_ptr+i);
                if (TAG(term) == ATM) {
                    term_cp = term;
                } else {
                    term_cp = numberVarCopyToFindallArea(faa_record_ptr, FOLLOW(term_ptr+i));
                    if (term_cp == BP_ERROR) return BP_ERROR;
                }
                FOLLOW(dest_ptr+i) = term_cp;
            }
            term_cp = ADDTAG(dest_ptr, STR);
            return term_cp;
        } else {  /* SUSP var */
            dest_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
            PUSHTRAILC_ATOMIC(dest_ptr, term);
            FOLLOW(dest_ptr) = NumberVar(global_var_num);
            global_var_num++;
            return FOLLOW(dest_ptr);
        }
    }
}

BPLONG numberVarCopyListToFindallArea(NUMBERED_TERM_AREA_RECORD_PTR faa_record_ptr, BPLONG term) {
    BPLONG_PTR term_ptr, ret_term_ptr, dest_ptr, top;
    BPLONG ret_term, car_cp;

    ret_term_ptr = &ret_term;
    while (ISLIST(term)) {
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        ALLOCATE_FROM_NUMBERED_TERM_AREA(faa_record_ptr, dest_ptr, 2);
        if (dest_ptr == NULL) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        FOLLOW(ret_term_ptr) = ADDTAG(dest_ptr, LST);
        car_cp = numberVarCopyToFindallArea(faa_record_ptr, FOLLOW(term_ptr));
        if (car_cp == BP_ERROR) return BP_ERROR;
        FOLLOW(dest_ptr) = car_cp;

        ret_term_ptr = dest_ptr+1;
        term = FOLLOW(term_ptr+1);
        DEREF(term);
    }
    FOLLOW(ret_term_ptr) = numberVarCopyToFindallArea(faa_record_ptr, term);
    return ret_term;
}

