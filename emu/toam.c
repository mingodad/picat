/********************************************************************
 *   File   : toam.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: Emulator of ATOAM

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>
#include "basicd.h"
#include "extern_decl.h"
#include "term.h"
#include "inst.h"
#include "frame.h"
#include "toam.h"
#include <signal.h>
/* #include <setjmp.h> */

#define TRACE_BUILTIN
/*
  #define DEBUG_CALL 
  #define DEBUG_INST
  #define TRACE_BUILTIN
  #define TRACE_TOAM
  #define DEBUG_DELAY
  #define DEBUG_INST
*/

extern Builtins builtins[];  //branch
extern BPLONG_PTR *asp_rel_mins, *asp_rel_sizes;
extern char *eventNoNameTable[];  //branch

BPLONG interrupt_sym;  /* the atom 'interrupt' */
SYM_REC_PTR ball_psc;  /* the global var '$ball' */
BPLONG_PTR sreg = NULL;  /* current build or unify field */
BYTE bprolog_initialized = 0;
/* jmp_buf restart_env; */

BPLONG gc_threshold = 50;
BPLONG last_tabled_call_code = last_tabled_call;
BPLONG table_allocate_code = table_allocate;

#ifdef GCC
void **jmp_table;
#endif

void exception_handler(signo)
    int signo;
{
    switch (signo) {
#ifdef BPSOLVER
    case SIGXCPU:
    case SIGSEGV:
        fprintf(stderr, "%% unhandled signal %d\n", signo);
        //    fprintf(stdout,"%% UNKNOWN\n");
        exit(1);
#endif

    case SIGINT:
        /*    longjmp(restart_env,-1); */
        toam_signal_vec |= INTERRUPT;
        if (signal(SIGINT, exception_handler) == SIG_ERR)
            printf("can't catch SIGINT\n");
        break;

    default:
        if (user_signal_action[signo])  //branch
        {  //branch
            toam_signal_vec |= (INTERRUPT | USER_INTERRUPT);  //branch
            user_signal = signo;  //branch
        }  //branch
        else  //branch
        {  //branch
            printf("signal not handled: signo=%d\n", signo);
            exit(2);
        }  //branch
        break;
    }
}

void init_signals() {
    int i;  //branch
    toam_signal_vec = 0;

    for (i = 0; i < NSIG; i++)  //branch
    {  //branch
        user_signal_action[i] = NULL;  //branch
    }  //branch
    if (signal(SIGINT, exception_handler) == SIG_ERR)
        printf("can't catch SIGINT\n");
#ifdef BPSOLVER
    if (signal(SIGXCPU, exception_handler) == SIG_ERR)
        printf("can't catch SIGINT\n");
    if (signal(SIGSEGV, exception_handler) == SIG_ERR)
        printf("can't catch SIGINT\n");
#endif
}

int initialize_bprolog(argc, argv)
    int argc;
    char *argv[];
{
    BPLONG_PTR inst_begin0;
    if (bprolog_initialized == 1) return BP_TRUE;
    init_toam(argc, argv);  /* first scan of the command line arguments */
    file_init();

#ifdef GCC
    toam(NULL, NULL, NULL);
#endif

    init_builtins();
    init_signals();
    Cboot();
    bprolog_initialized = 1;
    load_byte_code_from_c_array();

    if (init_loading(argc, argv) == BP_ERROR) {
        bp_exception = bp_initialization_error;
        return BP_ERROR;
    }

    inst_begin0 = inst_begin;
    bp_call_term(ADDTAG(insert_sym("initialize_bp", 13, 0), ATM));
    inst_begin = inst_begin0;
    return BP_TRUE;
}

int toam(P, AR, LOCAL_TOP)
    register BPLONG_PTR P, AR, LOCAL_TOP;
{
    register BPLONG op1;
    register BPLONG_PTR top, sreg = NULL;
    register BPLONG op2, op3;
    register SYM_REC_PTR sym_ptr = NULL;
    register BPLONG arity, i;
    register BPLONG_PTR dv_ptr;
    BYTE table_flag;
    BPLONG_PTR ep;

    BPLONG head_arity;
    char *error_goal_name;
    BPLONG error_goal_arity;
    BPLONG first, last, available_sh_space;

    BPLONG_PTR subgoal_entry;
    BPLONG_PTR stack_water_mark, heap_water_mark;
    BPLONG_PTR constr_ar;

#include "jmp_table.c"
#ifdef GCC
    if (P == NULL) {  /* first time executed */
        jmp_table = local_jmp_table;
        FOLLOW(addr_halt) = (BPLONG)local_jmp_table[halt];
        FOLLOW(addr_halt0) = (BPLONG)local_jmp_table[halt0];
        FOLLOW(addr_fail) = (BPLONG)local_jmp_table[fail];
        FOLLOW(addr_table_consume) = (BPLONG)local_jmp_table[table_consume];
        last_tabled_call_code = (BPLONG)local_jmp_table[last_tabled_call];
        table_allocate_code = (BPLONG)local_jmp_table[table_allocate];
        return(BP_TRUE);  //branch
    }
#endif

#ifdef ToamProfile
    Cboot_profile();
#endif

    RESET_WATER_MARKS;
    gc_b = B;  /* bottom of the top segment that has been gc collected */
    /**************************************************************************/
#ifndef GCC  //branch
contcase:  /* LOCAL_TOP OF EXECUTION LOOP : Read Mode */
#endif  //branch
#ifdef DEBUG_INST
    // if (trace_toam) {
    cpreg = P;
    printf("%d p(%x) ar(%x) lt(%x) h(%x)\n", toam_signal_vec, P, AR, LOCAL_TOP, H);
    print_inst(stdout);
    // }
#endif
#ifdef TRACE_TOAM
    if (trace_toam) {
        cpreg = P;
        printf("ar(%x) lt(%x)", AR, LOCAL_TOP);
        print_inst(curr_out);
    }
#endif
#ifdef TRACE_BUILTIN
    cpreg = P;
    if (*cpreg >= builtin0 && *cpreg <= builtin4) {
        printf("ar(%x) lt(%x)", AR, LOCAL_TOP);
        print_inst(curr_out);
    }
#endif

#ifdef GCC
    goto **(void **)P++;
#endif

#ifdef ToamProfile
    execute_inst(*P);
#endif

    /*     bookkeep_inst(*P); */

    /*
      #ifdef TRACE_INSTS
      addExecTrace(*P);
      #endif
      if (toam_signal_vec!=0) printf("toam_signal_vec=%x\n",toam_signal_vec);  
    */

#include "emu_inst.h"

interrupt_handler: {
        BPLONG this_exception;
        BPLONG_PTR btm_ptr;
        BPLONG_PTR f = B;

#ifdef BPSOLVER
        fprintf(stderr, "%% unhandled interrupt event \n");
        //      fprintf(stdout,"%% UNKNOWN\n");
        exit(1);
#endif
        if (bp_exception == (BPLONG)NULL) bp_exception = unknown_exception;
        event_func.func = NULL;
        if (toam_signal_vec & USER_INTERRUPT)
        {
            (*user_signal_action[user_signal])(user_signal, user_signal_data[user_signal]);
        }
        while ((BPLONG)f != AR_B(f)) {
            RESET_SUBGOAL_AR(f);
            if (IS_CATCHER_FRAME(f)) {
                btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));  /* a catcher frame is in the form of p(Flag,Cleanup,Calll,Exception,Recovery,...) */
                this_exception = FOLLOW(btm_ptr-3);
                if (is_UNIFIABLE(bp_exception, this_exception)) {
                    goto interrupt_end_while;
                }
            }
            f = (BPLONG_PTR)AR_B(f);
        }

    interrupt_end_while:
        B = f;
        HB = (BPLONG_PTR)AR_H(B);
        if (toam_signal_vec & USER_INTERRUPT)
        {
            BPLONG_PTR parent_ar = (BPLONG_PTR)AR_AR(AR);
            toam_signal_vec = 0;
            *LOCAL_TOP-- = MAKEINT(event_func.signo);

            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)AR_CPS(parent_ar);
            AR = LOCAL_TOP;
            P = (BPLONG_PTR)GET_EP(event_func.func);
            CONTCASE;
        }
        else
        {
            toam_signal_vec = 0;
            GET_EP(ball_psc) = (int (*)(void))bp_exception;
            bp_exception = (BPLONG)NULL;
            GET_ETYPE(ball_psc) = T_DYNA;
            BACKTRACK;
        }
    }

    /*------------------------------------------------------------------*/
catch_exception:{
        BPLONG_PTR parent_ar = (BPLONG_PTR)AR_AR(AR);
        if (bp_exception == (BPLONG)NULL) bp_exception = unknown_exception;
        *LOCAL_TOP-- = bp_exception;
        bp_exception = (BPLONG)NULL;
        *LOCAL_TOP-- = c_error_src(error_goal_name, error_goal_arity);

        AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
        AR_CPS(LOCAL_TOP) = (BPLONG)AR_CPS(parent_ar);
        AR = LOCAL_TOP;
        P = (BPLONG_PTR)GET_EP(handle_exception_psc);
        in_critical_region = 1;  /* disable GC, since the number of variables needing initialization is unknown */
        CONTCASE;
    }

forward_exception_as_is:{
        BPLONG_PTR parent_ar = (BPLONG_PTR)AR_AR(AR);
        if (bp_exception == (BPLONG)NULL) bp_exception = unknown_exception;
        *LOCAL_TOP-- = bp_exception;
        bp_exception = (BPLONG)NULL;

        AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
        AR_CPS(LOCAL_TOP) = (BPLONG)AR_CPS(parent_ar);
        AR = LOCAL_TOP;
        P = (BPLONG_PTR)GET_EP(forward_exception_psc);
        in_critical_region = 1;  /* disable GC, since the number of variables needing initialization is unknown */
        CONTCASE;
    }

    /*------------------------------------------------------------------*/
trigger_on_handler:{
        BPLONG flag;
        BPLONG time_out_event_index = 0;
        /*
          if (trigger_no>2) printf("DEBUG==>handler %d \n",trigger_no);
          printf("DEBUG==>handler %d flag=%d\n",trigger_no,event_flag[trigger_no]);
          write_term(FOLLOW(triggeredCs[trigger_no]));
          printf("\n");
        */
        for (i = trigger_no; i > 0; i--) {
            flag = event_flag[i];
            op1 = FOLLOW(triggeredCs[i]);

            switch (flag) {
            case EVENT_VAR_INS:
            case EVENT_DVAR_INS:
                while (ISLIST(op1)) {
                    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);  /* borrow sreg */
                    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                    CONNECT_WOKEN_FRAME_ins(constr_ar);
                    op1 = LIST_NEXT(sreg);
                }
                break;

            case EVENT_DVAR_MINMAX:
                while (ISLIST(op1)) {
                    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                    if (constr_ar != triggering_frame[trigger_no]) {
                        CONNECT_WOKEN_FRAME_min_max(constr_ar);
                    }
                    op1 = LIST_NEXT(sreg);
                }
                break;

            case EVENT_DVAR_OUTER_DOM:
            case EVENT_DVAR_DOM:
                while (ISLIST(op1)) {
                    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                    if (TAG(event_object[i]) != ATM) {
                        op1 = (BPLONG)AR_OUT_ADDR(constr_ar);
                        PUSHTRAIL_S_NONATOMIC(op1, FOLLOW(op1));  /* trail for GC */
                    }
                    CONNECT_WOKEN_FRAME_dom(constr_ar, event_object[i]);
                    op1 = LIST_NEXT(sreg);
                }
                break;
            case TimeOut:
                if (time_out_event_index == 0) {
                    time_out_event_index = i;
                } else {
                    while (ISLIST(op1)) {
                        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                        CONNECT_WOKEN_FRAME_dom(constr_ar, dummy_event_object);
                        op1 = LIST_NEXT(sreg);
                    }
                }
                break;

            case EVENT_GENERAL:{
                BPLONG prev_alive_node_cdr;
                BPLONG_PTR prev_alive_node_ptr;

                prev_alive_node_ptr = triggeredCs[i];
                prev_alive_node_cdr = op1;
                while (ISLIST(op1)) {
                    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                    if (!FRAME_IS_DEAD(constr_ar)) {  /* compact the list */
                        if (prev_alive_node_cdr != op1) {
                            PUSHTRAIL_H_NONATOMIC(prev_alive_node_ptr, prev_alive_node_cdr);
                            FOLLOW(prev_alive_node_ptr) = op1;
                        }
                        if (TAG(event_object[i]) != ATM) {  /* neither Atom nor Integer */
                            op1 = (BPLONG)AR_OUT_ADDR(constr_ar);
                            PUSHTRAIL_S_NONATOMIC(op1, FOLLOW(op1));  /* trail for GC */
                        }
                        CONNECT_WOKEN_FRAME_dom(constr_ar, event_object[i]);
                        prev_alive_node_ptr = (sreg+1);
                        prev_alive_node_cdr = FOLLOW(prev_alive_node_ptr);
                    }
                    op1 = LIST_NEXT(sreg);
                }
                if (prev_alive_node_cdr != nil_sym) {
                    PUSHTRAIL_H_NONATOMIC(prev_alive_node_ptr, prev_alive_node_cdr);
                    FOLLOW(prev_alive_node_ptr) = nil_sym;
                }
                break;
            }

#ifdef JAVA
            case MouseClicked:
            case MousePressed:
            case MouseReleased:
            case MouseEntered:
            case MouseExited:
            case MouseMoved:
            case MouseDragged:{
                extern mouse_event_psc;
                BPLONG bp_o;
                CgMouseEventClass o = (CgMouseEventClass)event_object[i];
                bp_o = ADDTAG(H, STR);
                FOLLOW(H++) = mouse_event_psc;
                FOLLOW(H++) = MAKEINT(o->type);
                FOLLOW(H++) = MAKEINT(o->x);
                FOLLOW(H++) = MAKEINT(o->y);
                FOLLOW(H++) = MAKEINT(o->count);
                FOLLOW(H++) = MAKEINT(o->modifiers);
                event_object[i] = bp_o;
                free(o);
                goto install_handlers;
            };

            case ActionPerformed:
            case WindowClosing:
            case WindowOpened:
            case WindowIconified:
            case WindowDeiconified:
            case WindowClosed:
            case WindowActivated:
            case WindowDeactivated:
            case FocusGained:
            case FocusLost:
            case TextValueChanged:
                goto install_handlers;

            case KeyPressed:
            case KeyReleased:
            case KeyTyped:{
                extern key_event_psc;
                BPLONG bp_o;
                CgKeyEventClass o = (CgKeyEventClass)event_object[i];
                bp_o = ADDTAG(H, STR);
                FOLLOW(H++) = key_event_psc;
                FOLLOW(H++) = MAKEINT(o->type);
                FOLLOW(H++) = MAKEINT(o->code);
                FOLLOW(H++) = MAKEINT(o->ch);
                FOLLOW(H++) = MAKEINT(o->modifiers);
                event_object[i] = bp_o;
                free(o);
                goto install_handlers;
            }

            case ComponentMoved:
            case ComponentResized:
            case ComponentHidden:
            case ComponentShown:{
                extern component_event_psc;
                BPLONG bp_o;
                CgComponentEventClass o = (CgComponentEventClass)event_object[i];
                bp_o = ADDTAG(H, STR);
                FOLLOW(H++) = component_event_psc;
                FOLLOW(H++) = MAKEINT(o->type);
                FOLLOW(H++) = MAKEINT(o->x);
                FOLLOW(H++) = MAKEINT(o->y);
                FOLLOW(H++) = MAKEINT(o->width);
                FOLLOW(H++) = MAKEINT(o->height);
                event_object[i] = bp_o;
                free(o);
                goto install_handlers;
            };
            case ItemStateChanged:{
                extern item_event_psc;
                BPLONG bp_o;
                CgItemEventClass o = (CgItemEventClass)event_object[i];
                bp_o = ADDTAG(H, STR);
                FOLLOW(H++) = item_event_psc;
                FOLLOW(H++) = MAKEINT(o->type);
                FOLLOW(H++) = MAKEINT(o->index);
                FOLLOW(H++) = MAKEINT(o->change);
                event_object[i] = bp_o;
                free(o);
                goto install_handlers;
            }
            case AdjustmentValueChanged:{
                extern adjustment_event_psc;
                BPLONG bp_o;
                CgAdjustmentEventClass o = (CgAdjustmentEventClass)event_object[i];
                bp_o = ADDTAG(H, STR);
                FOLLOW(H++) = adjustment_event_psc;
                FOLLOW(H++) = MAKEINT(o->type);
                FOLLOW(H++) = MAKEINT(o->adjust_type);
                FOLLOW(H++) = MAKEINT(o->value);
                event_object[i] = bp_o;
                free(o);

                install_handlers:
                while (ISLIST(op1)) {
                    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                    if (event_handler_type(constr_ar) == flag) {
                        if (TAG(event_object[i]) != ATM) {
                            op1 = (BPLONG)AR_OUT_ADDR(constr_ar);
                            PUSHTRAIL_S_NONATOMIC(op1, FOLLOW(op1));
                        }
                        CONNECT_WOKEN_FRAME_cg(constr_ar, event_object[i]);
                    }
                    op1 = LIST_NEXT(sreg);
                }
            }
#endif
            default:
                break;
            }
        }
        if (time_out_event_index != 0) {  /* the most recent  time-event has top priority */
            op1 = FOLLOW(triggeredCs[time_out_event_index]);
            while (ISLIST(op1)) {
                sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
                CONNECT_WOKEN_FRAME_dom(constr_ar, dummy_event_object);
                op1 = LIST_NEXT(sreg);
            }
        }

        trigger_no = 0;
        toam_signal_vec &= ~TRIGGER_ON;
        toam_LOCAL_OVERFLOW_CHECK_SMALL_MARGIN(7);
        /*
          printf("<==handler %d\n",trigger_no);
          check_ar_chain(AR);
        */
        CONTCASE;
    }
}  /* end toam.c */


/**************************************************/
/*
  printEntrance(p)
  BPLONG_PTR p;
  {
  SYM_REC_PTR sym_ptr;

  while (!is_allocate(p) && p>=parea_low_addr && p<parea_up_addr) p--;
  if (p<=parea_low_addr || p>=parea_up_addr) return;
  sym_ptr = (SYM_REC_PTR)FOLLOW(p+3);
  if (GET_ETYPE(sym_ptr)==T_PRED && GET_EP(sym_ptr)>parea_low_addr && GET_EP(sym_ptr)<parea_up_addr) 
  printf("\t > %s/%d\n",GET_NAME(sym_ptr),GET_ARITY(sym_ptr));
  }
  

  is_allocate(p)
  BPLONG_PTR p;
  {
  if (*p==allocate_det ||
  *p ==allocate_det_b ||
  *p == allocate_nondet ||
  *p == allocate_susp || 
  *p == table_allocate){
  if (*(p+1)<100 && *(p+1)>=0 && *(p+2)<100 && *(p+2)>=0 &&
  (BPLONG_PTR)*(p+3)>parea_low_addr && (BPLONG_PTR)*(p+3)<parea_up_addr)
  return 1;
  }
  return 0;
  }
*/

