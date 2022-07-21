/********************************************************************
 *   File   : debug.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2022
 *   Purpose: debugging primitives

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <string.h>
#include "basic.h"
#include "bapi.h"
#include "term.h"
#include "frame.h"
#include "dynamic.h"

BPLONG skip_call_no;  /*all frames under this frame are skipped */
static BPLONG dg_print_depth = 8;
static int dg_undo_mode = 0;

/* Functions for manipulating the db_flag_word    */
/* ---------------------------------------------  */
/*  |repeat | skip | leap | creep | spy | debug | */
/* ---------------------------------------------  */
#ifdef M64BITS
#define DG_FLAG_DEBUG 0x1LL
#define DG_FLAG_SPY 0x2LL
#define DG_FLAG_SPY_DEBUG 0x3LL
#define DG_FLAG_C 0x4LL
#define DG_FLAG_L 0x8LL
#define DG_FLAG_S 0x10LL
#define DG_FLAG_R 0x20LL
#else
#define DG_FLAG_DEBUG 0x1
#define DG_FLAG_SPY 0x2
#define DG_FLAG_SPY_DEBUG 0x3
#define DG_FLAG_C 0x4
#define DG_FLAG_L 0x8
#define DG_FLAG_S 0x10
#define DG_FLAG_R 0x20
#endif

int c_init_dg_flag()
{
    BPLONG a;
    a = ARG(1, 1); DEREF(a); a = INTVAL(a);
    dg_flag_word = (UW16)a;
    return BP_TRUE;
}

int c_set_dg_flag()
{
    BPLONG a;
    a = ARG(1, 1); DEREF(a); a = INTVAL(a);
    dg_flag_word = (UW16)(((BPLONG)dg_flag_word & 0x3L) | a);
    return BP_TRUE;
}

int c_get_dg_flag()
{
    BPLONG a;

    a = ARG(1, 1);
    return unify(a, MAKEINT(dg_flag_word));
}

int c_remove_spy_point() {
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG name, arity;

    name = ARG(1, 2);
    arity = ARG(2, 2);
    GET_GLOBAL_SYM(name, arity, sym_ptr);
    if (GET_SPY(sym_ptr) == 1) {
        GET_SPY(sym_ptr) = 0;
        number_of_spy_points--;
        if (number_of_spy_points == 0) {
            dg_flag_word &= ~((UW16)DG_FLAG_SPY);
        }
        return BP_TRUE;
    } else
        return BP_FALSE;
}

int c_is_spy_point() {
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG call;

    call = ARG(1, 1); DEREF(call);
    sym_ptr = GET_SYM_REC(call);
    return (GET_SPY(sym_ptr) == 1) ? BP_TRUE : BP_FALSE;
}

int c_add_spy_point() {
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    BPLONG name, arity;

    name = ARG(1, 2);
    arity = ARG(2, 2);
    GET_GLOBAL_SYM(name, arity, sym_ptr);
	if (dg_flag_word == 0){
	  printf("Not in debug mode. Use the command 'debug' to switch to debug mode before cl.\n");
	  return BP_FALSE;
	} else {
	  dg_flag_word = DG_FLAG_SPY;
	  if (GET_SPY(sym_ptr) == 0) {
        GET_SPY(sym_ptr) = 1;
        number_of_spy_points++;
        return BP_TRUE;
	  } else
        return BP_FALSE;
	}
}

int c_get_spy_points() {
    BPLONG list, temp0, temp1, cell;
    BPLONG i;
    SYM_REC_PTR sym_ptr;

    list = ARG(1, 1);

    temp1 = nil_sym;
    for (i = 0; i < BUCKET_CHAIN; ++i) {
        sym_ptr = hash_table[i];
        while (sym_ptr != NULL) {
            if (GET_SPY(sym_ptr) == 1) {
                cell = ADDTAG(insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 0), ATM);
                cell = make_struct2("/", cell, MAKEINT(GET_ARITY(sym_ptr)));
                temp0 = ADDTAG((BPLONG)heap_top, LST);
                NEW_HEAP_NODE(cell);
                NEW_HEAP_NODE(temp1);
                temp1 = temp0;
                LOCAL_OVERFLOW_CHECK("spy");
            }
            sym_ptr = GET_NEXT(sym_ptr);
        }
    }
    return unify(list, temp1);
}

int c_remove_spy_points() {
    BPLONG i;
    SYM_REC_PTR sym_ptr;


    for (i = 0; i < BUCKET_CHAIN; ++i) {
        sym_ptr = hash_table[i];
        while (sym_ptr != NULL) {
            if (GET_SPY(sym_ptr) == 1) GET_SPY(sym_ptr) = 0;
            sym_ptr = GET_NEXT(sym_ptr);
        }
    }
    number_of_spy_points = 0;
    dg_flag_word &= ~((UW16)DG_FLAG_SPY);

    return BP_TRUE;
}

int c_is_debug_mode() {
    if (dg_flag_word & DG_FLAG_SPY_DEBUG)
        return BP_TRUE;
    else
        return BP_FALSE;
}

int b_IS_DEBUG_MODE() {
    if (dg_flag_word & DG_FLAG_SPY_DEBUG)
        return BP_TRUE;
    else
        return BP_FALSE;
}

int c_is_spy_mode() {
    if (number_of_spy_points > 0 && (dg_flag_word & DG_FLAG_SPY))
        return BP_TRUE;
    else
        return BP_FALSE;
}

void set_global_call_number(BPLONG CallNo) {
    global_call_number = CallNo;
    dg_undo_mode = 0;
}

int c_set_global_call_number() {
    BPLONG CallNo;

    CallNo = ARG(1, 1);
    DEREF(CallNo);
    global_call_number = INTVAL(CallNo);
    dg_undo_mode = 0;
    return BP_TRUE;
}

int c_next_global_call_number() {
    BPLONG a;
    a = ARG(1, 1);

    unify(a, MAKEINT(global_call_number));
    global_call_number++;
    return BP_TRUE;
}

int c_set_skip_call_no() {
    BPLONG call_no;
    call_no = ARG(1, 1);
    DEREF(call_no);
    skip_call_no = call_no;
    return BP_TRUE;
}


int c_is_skip_call_no() {
    BPLONG call_no;
    call_no = ARG(1, 1);
    DEREF(call_no);
    return (skip_call_no == call_no) ? BP_TRUE : BP_FALSE;
}

int c_backtrace() {
    BPLONG uptoCallNo, Backtrace, L;
    BPLONG_PTR f, prev_f;
    SYM_REC_PTR traced_call_psc;

    traced_call_psc = BP_NEW_SYM("_$traced_call", 1);

    uptoCallNo = ARG(1, 2);
    Backtrace = ARG(2, 2);

    L = nil_sym;
    DEREF(uptoCallNo);
    uptoCallNo = INTVAL(uptoCallNo);

    f = arreg;
    for (; ; ) {
        BPLONG traced_call, callNo;
        BPLONG_PTR btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
        if (btm_ptr-f == 3) {  /* $eval_and_monitor_call('_$traced_call'(Call),CallNo,PCP) */
            traced_call = FOLLOW(btm_ptr);
            DEREF(traced_call);
            if (ISSTRUCT(traced_call) && GET_STR_SYM_REC(traced_call) == traced_call_psc) {
                BPLONG_PTR traced_call_ptr;
                BPLONG call;
                callNo = FOLLOW(btm_ptr-1);
                DEREF(callNo);
                if (INTVAL(callNo) >= uptoCallNo) {
                    BPLONG pair;
                    traced_call_ptr = (BPLONG_PTR)UNTAGGED_ADDR(traced_call);
                    call = FOLLOW(traced_call_ptr+1); DEREF(call);
                    pair = ADDTAG(heap_top, STR);
                    FOLLOW(heap_top++) = (BPLONG)comma_psc;
                    FOLLOW(heap_top++) = call;
                    FOLLOW(heap_top++) = callNo;
                    FOLLOW(heap_top) = pair;
                    FOLLOW(heap_top+1) = L;
                    L = ADDTAG(heap_top, LST);
                    heap_top += 2;
                } else {
                    return bp_unify(L, Backtrace);
                }
            }
        }
        prev_f = (BPLONG_PTR)AR_AR(f);
        if (f == prev_f) {
            return bp_unify(L, Backtrace);
        }
        f = prev_f;
    }
}


int c_get_dg_print_depth() {
    BPLONG Depth = ARG(1, 1);
    unify(Depth, MAKEINT(dg_print_depth));
    return BP_TRUE;
}

int c_set_dg_print_depth() {
    BPLONG Depth = ARG(1, 1);
    DEREF(Depth);
    dg_print_depth = INTVAL(Depth);
    return BP_TRUE;
}


int c_get_dg_choice_point() {
    BPLONG uptoCallNo, CP;
    BPLONG_PTR f, prev_f;
    SYM_REC_PTR traced_call_psc;

    traced_call_psc = BP_NEW_SYM("_$traced_call", 1);
    uptoCallNo = ARG(1, 2);
    CP = ARG(2, 2);

    DEREF(uptoCallNo);
    uptoCallNo = INTVAL(uptoCallNo);

    //  printf("uptoCallNo=%d\n",uptoCallNo);

    f = breg;
    for (; ; ) {
        BPLONG traced_call, callNo;
        BPLONG_PTR btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
        if (btm_ptr == f+3) {  /* $eval_and_monitor_call('_$traced_call'(Call),CallNo,PCP) */
            traced_call = FOLLOW(btm_ptr);
            DEREF(traced_call);
            if (ISSTRUCT(traced_call) && GET_STR_SYM_REC(traced_call) == traced_call_psc) {
                callNo = FOLLOW(btm_ptr-1);
                DEREF(callNo);
                if (INTVAL(callNo) <= uptoCallNo) {
                    dg_undo_mode = 1;
                    return unify(CP, ADDTAG((BPULONG)stack_up_addr-(BPULONG)f, INT_TAG));
                }
            }
        }
        prev_f = (BPLONG_PTR)AR_B(f);
        if (f == prev_f) {
            return BP_FALSE;
        }
        f = prev_f;
    }
}

int c_dg_in_undo_mode() {
    return (dg_undo_mode == 1) ? BP_TRUE : BP_FALSE;
}

void Cboot_debug()
{
    insert_cpred("c_init_dg_flag", 1, c_init_dg_flag);
    insert_cpred("c_get_dg_flag", 1, c_get_dg_flag);
    insert_cpred("c_set_dg_flag", 1, c_set_dg_flag);
    insert_cpred("c_add_spy_point", 2, c_add_spy_point);
    insert_cpred("c_remove_spy_point", 2, c_remove_spy_point);
    insert_cpred("c_remove_spy_points", 0, c_remove_spy_points);
    insert_cpred("c_get_spy_points", 1, c_get_spy_points);
    insert_cpred("c_is_debug_mode", 0, c_is_debug_mode);
    insert_cpred("c_is_spy_mode", 0, c_is_spy_mode);
    insert_cpred("c_next_global_call_number", 1, c_next_global_call_number);
    insert_cpred("c_is_spy_point", 1, c_is_spy_point);
    insert_cpred("c_set_skip_call_no", 1, c_set_skip_call_no);
    insert_cpred("c_is_skip_call_no", 1, c_is_skip_call_no);
    insert_cpred("c_backtrace", 2, c_backtrace);
    insert_cpred("c_set_dg_print_depth", 1, c_set_dg_print_depth);
    insert_cpred("c_get_dg_print_depth", 1, c_get_dg_print_depth);
    insert_cpred("c_set_global_call_number", 1, c_set_global_call_number);
    insert_cpred("c_get_dg_choice_point", 2, c_get_dg_choice_point);
    insert_cpred("c_dg_in_undo_mode", 0, c_dg_in_undo_mode);

}





