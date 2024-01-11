/********************************************************************
 *   File   : delay.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024
 *   Purpose: Primitives for suspension variables and agents

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include <string.h>
#include "bprolog.h"
#include "event.h"
#include "frame.h"

BPLONG true;

BPLONG build_delayed_call_on_the_heap(BPLONG_PTR frame)
{
    SYM_REC_PTR sym_ptr;
    BPLONG arity, i;
    BPLONG call, arg;
    BPLONG_PTR top;

    sym_ptr = (SYM_REC_PTR)FOLLOW((BPLONG_PTR)AR_REEP(frame)+2);
    arity = GET_ARITY(sym_ptr);

    /*  printf("name=%s/arity=%d\n",GET_NAME(sym_ptr),arity); */

    for (i = arity; i > 0; i--) {
        arg = FOLLOW(frame+i);
        if (ISREF(arg) && ISFREE(arg)) {
            FOLLOW(arg) = (BPLONG)heap_top;
            PUSHTRAIL(arg);
            NEW_HEAP_FREE;
        }
    }

    call = ADDTAG(heap_top, STR);
    FOLLOW(heap_top++) = (BPLONG)sym_ptr;
    for (i = arity; i > 0; i--) {
        arg = FOLLOW(frame+i);
        DEREF(arg);
        if (IS_SUSP_VAR(arg))
            NEW_HEAP_NODE(UNTAGGED_TOPON_ADDR(arg));
        else
            NEW_HEAP_NODE(arg);
    }
    LOCAL_OVERFLOW_CHECK("delay");
    return call;
}


int c_frozen_cf() {
    register BPLONG var, return_goal;
    BPLONG goal;
    BPLONG_PTR dv_ptr, dcs, list;
    BPLONG_PTR top;

    var = ARG(1, 2);
    return_goal = ARG(2, 2);

    DEREF(var);
    list = heap_top;
    goal = (BPLONG)list;
    *heap_top = (BPLONG)heap_top; heap_top++;
    if (IS_SUSP_VAR(var)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(var);
        dcs = (BPLONG_PTR)DV_ins_cs(dv_ptr);
        list = frozen_cs(dcs, list);
        if ((BPLONG)list == -1) return BP_ERROR;

        dcs = (BPLONG_PTR)DV_minmax_cs(dv_ptr);
        list = frozen_cs(dcs, list);
        if ((BPLONG)list == -1) return BP_ERROR;

        dcs = (BPLONG_PTR)DV_dom_cs(dv_ptr);
        list = frozen_cs(dcs, list);
        if ((BPLONG)list == -1) return BP_ERROR;

        dcs = (BPLONG_PTR)DV_outer_dom_cs(dv_ptr);
        list = frozen_cs(dcs, list);
        if ((BPLONG)list == -1) return BP_ERROR;

        FOLLOW(list) = nil_sym;

        return unify(return_goal, goal);
    }
    else return unify(return_goal, true);
}

BPLONG_PTR frozen_cs(BPLONG_PTR cs, BPLONG_PTR Plist)
{
    BPLONG tmp;
    BPLONG_PTR frame;

    while (ISLIST((BPLONG)cs)) {
        cs = (BPLONG_PTR)UNTAGGED_ADDR(cs);
        frame = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(cs)));

        if (!FRAME_IS_DEAD(frame)) {
            tmp = build_delayed_call_on_the_heap(frame);
            if (tmp == -1) return (BPLONG_PTR)-1;
            FOLLOW(Plist) = ADDTAG(heap_top, LST);
            NEW_HEAP_NODE(tmp);
            Plist = heap_top; heap_top++;
        }
        cs = (BPLONG_PTR)LIST_NEXT(cs);
    }
    return Plist;
}

int c_frozen_f() {
    BPLONG P_goal, P_goal_rest;
    BPLONG cell, tmp;
    BPLONG_PTR frame;
    BPLONG_PTR top;

    P_goal = ARG(1, 1);
    DEREF(P_goal);
    if (!ISREF(P_goal)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }

    frame = sfreg;
    while (AR_PREV(frame) != (BPLONG)frame) {  /* end of chain */
        if (!FRAME_IS_DEAD(frame)) {
            tmp = build_delayed_call_on_the_heap(frame);
            if (tmp == -1) return BP_ERROR;
            cell = bp_build_list();
            unify(bp_get_car(cell), tmp);
            P_goal_rest = bp_get_cdr(cell);
            unify(P_goal, cell);
            P_goal = P_goal_rest;
        }
        frame = (BPLONG_PTR)AR_PREV(frame);
    }
    return unify(P_goal, bp_build_nil());
}

/*
  susp_attach_term(Var,Term) 
  attach T to the suspension variable Var.
  Exception if Var is non-variable
*/
int b_SUSP_ATTACH_TERM_cc(BPLONG Var, BPLONG Term)
{

    BPLONG_PTR top, dv_ptr;

    DEREF(Var);
    DEREF(Term);
    if (ISREF(Term)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    if (ISREF(Var)) {
        CREATE_SUSP_VAR_nocs(Var);  /* dv_ptr set */
        DV_attached(dv_ptr) = Term;
        return 1;
    } else if (IS_SUSP_VAR(Var)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Var);
        top = A_DV_attached(dv_ptr);
        PUSHTRAIL_H_NONATOMIC(top, FOLLOW(top));
        DV_attached(dv_ptr) = Term;
        return 1;
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
}

/*
  susp_attached_term(Var,Term) 
  the attached term to Var is Term
  Exception if Var is not a suspension variable
*/
int b_SUSP_ATTACHED_TERM_cf(BPLONG Var, BPLONG Term)
{
    BPLONG_PTR top, dv_ptr;

    DEREF(Var);
    if (!IS_SUSP_VAR(Var)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    } else {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Var);
        ASSIGN_sv_heap_term(Term, DV_attached(dv_ptr));
        return 1;
    }
}

int b_SUSP_VAR_c(BPLONG var)
{
    BPLONG_PTR top;

    DEREF(var);
    if (IS_SUSP_VAR(var)) return 1; else return 0;
}

/* skip all the dead constraints in the list */
BPLONG next_alive_susp_call(BPLONG cs_list, BPLONG_PTR breg)
{
    BPLONG_PTR constr_ar, ptr;

    while (cs_list != nil_sym) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs_list);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(ptr)));
        if (AR_STATUS(constr_ar) == SUSP_EXIT && constr_ar < breg) {  /* permanently dead */
            /*
              printf("skip %x (breg=%x)\n",constr_ar,breg);
              show_frame(constr_ar);
            */
            cs_list = FOLLOW(ptr+1);
        } else return cs_list;
    }
    return cs_list;
}

void print_cs(BPLONG cs_list)
{
    BPLONG_PTR constr_ar, ptr;

    while (cs_list != nil_sym) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs_list);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(ptr)));
        if (AR_STATUS(constr_ar) == SUSP_EXIT) {
            printf("-");
        } else {
            printf("*");
        }
        cs_list = FOLLOW(ptr+1);
    }
    printf("\n");
}

void Cboot_delay()
{

    insert_cpred("c_frozen_cf", 2, c_frozen_cf);
    insert_cpred("c_frozen_f", 1, c_frozen_f);
    true = ADDTAG(BP_NEW_SYM("true", 0), ATM);
}


