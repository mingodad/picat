/********************************************************************
 *   File   : clause.c 
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2022
 *   Purpose: auxiliary functions for defining clause/2

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include "bprolog.h"
#include "dynamic.h"
#include "gc.h" /* borrow gcQueue to use as a stack to implement recursion */

int aux_copy_clause(ref, head, body)
    BPLONG ref, head, body;
{
    InterpretedClausePtr clause_record_ptr;
    BPLONG cl_head, cl_body;
    BPLONG_PTR cl_head_ptr, head_ptr;
    BPLONG head_arg, cl_head_arg;
    BPLONG varNo;
    int arity, i;

    clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(ref);
    cl_head = clause_record_ptr->head;  /* clause(CellRef,Head,Body,TimeStamp) */
    cl_body = clause_record_ptr->body;

    PREPARE_UNNUMBER_TERM(local_top);

    //   printf("=>clause "); write_term(cl_head); write_term(cl_body);  printf("\n");
    gcQueueInit;
    GCQueueAdd(body, cl_body);
    DEREF_NONVAR(head);
    if (ISSTRUCT(head)) {
        head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(head);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(head_ptr));
        cl_head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl_head);
        for (i = arity; i > 1; i--) {
            GCQueueAdd(FOLLOW(head_ptr+i), FOLLOW(cl_head_ptr+i));
        }
        head_arg = FOLLOW(head_ptr+1);
        cl_head_arg = FOLLOW(cl_head_ptr+1);
        goto lab_after_get;
    }

loop:
    GCQueuePop(head_arg, cl_head_arg);
lab_after_get:
    if (IsNumberedVar(cl_head_arg)) {
        varNo = INTVAL(cl_head_arg);
        if (varNo > global_unnumbervar_max) {
            global_unnumbervar_max = varNo;
            global_unnumbervar_watermark = global_unnumbervar_ptr-global_unnumbervar_max;
            FOLLOW(global_unnumbervar_ptr-varNo) = head_arg;
        } else {
            if (unify(FOLLOW(global_unnumbervar_ptr-varNo), head_arg) == BP_FALSE) return BP_FALSE;
        }
        if (gcQueueCount > 0) goto loop; else return BP_TRUE;
    }

    SWITCH_OP(head_arg, unify_head_arg_lab,
              {PUSHTRAIL(head_arg); FOLLOW(head_arg) = unnumberVarTermOpt(cl_head_arg);},

              {if (head_arg != cl_head_arg) return BP_FALSE;},

              {if (ISLIST(cl_head_arg)) {
                      head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(head_arg);
                      cl_head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl_head_arg);
                      GCQueueAdd(FOLLOW(head_ptr+1), FOLLOW(cl_head_ptr+1));
                      GCQueueAdd(FOLLOW(head_ptr), FOLLOW(cl_head_ptr));
                  } else {return BP_FALSE;}},

              {if (ISSTRUCT(cl_head_arg)) {
                      head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(head_arg);
                      cl_head_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl_head_arg);
                      if (FOLLOW(head_ptr) != FOLLOW(cl_head_ptr)) return BP_FALSE;
                      arity = GET_ARITY((SYM_REC_PTR)FOLLOW(head_ptr));
                      for (i = arity; i > 0; i--) {
                          GCQueueAdd(FOLLOW(head_ptr+i), FOLLOW(cl_head_ptr+i));
                      }
                  } else {return BP_FALSE;}},

              {if (!unify(head_arg, unnumberVarTermOpt(cl_head_arg))) return BP_FALSE;});

    if (gcQueueCount > 0) goto loop;

    //  printf("heap_top=%x local_top=%x global_unnumbervar_max=%d\n",heap_top,local_top,global_unnumbervar_max);
    return BP_TRUE;
}

