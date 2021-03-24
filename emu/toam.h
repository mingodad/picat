/********************************************************************
 *   File   : toam.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <signal.h>
#include "bapi.h"
#include "event.h"
#include "clpfd.h"
#include "gc.h"
/*
  #define ToamProfile 
*/
extern BPLONG_PTR expand_trail();
extern BPLONG next_alive_susp_call();

void (*user_signal_action[NSIG])(int, void *);
void *user_signal_data[NSIG];

#define P p__reg
#define B breg
#define HB hbreg
#define AR ar__reg
#define LOCAL_TOP local_top_reg
#define T trail_top
#define H heap_top
#define SF sfreg

#define SAVE_AR arreg = AR
#define SAVE_TOP local_top = LOCAL_TOP
#define RESTORE_AR AR = arreg
#define RESTORE_TOP LOCAL_TOP = local_top

extern BPLONG no_gcs;

#define Operand0 *P
#define Operand(offset) *(P+offset)
#define NextOperandLiteral (*P++)
#define NextOperandConstant (*P++)
#define SkipDummyOperand P++;
#define NextOperandY (AR + *P++)
#define NextOperandYC *(AR + *P++)
#define NextOperandSym (*P++)
#define NextOperandAddr (*P++)

#define YU 01
#define YV 00

#define IS_YU_OPERAND(op) TAG(op) == YU
#define IS_YV_OPERAND(op) TAG(op) == YV

#define YC(varno) FOLLOW(AR + (varno))
#define Y(varno) (AR + (varno))

#define AR_IS_TOP(ar) AR_TOP(ar) == (BPLONG)LOCAL_TOP

#define UNDOTRAIL {                             \
        BPLONG_PTR top;                         \
        top = (BPLONG_PTR)AR_T(AR);             \
        while (T != top) {                      \
            BPLONG op1;                         \
            op1 = UNTAGGED3(FOLLOW(++T));       \
            *(BPLONG_PTR)op1 = FOLLOW(++T);     \
        }                                       \
    }

#define FETCH_ARGS(arity)                                       \
    do {                                                        \
        op1 = *P++; if (op1 != 0) YC(op1) = FOLLOW(sreg);       \
        sreg++;                                                 \
        arity--;                                                \
    } while (arity > 0)

#define PASS_U_ARGS(arity)                      \
    while (arity > 0) {                         \
        *LOCAL_TOP-- = YC(*P++);                \
        arity--;                                \
    }

#define PASS_Z_ARGS(arity) {                    \
        while (arity > 0) {                     \
            op1 = *P++;                         \
            PASS_ARG_Z(op1);                    \
            arity--;                            \
        }                                       \
    }

#define PASS_ARG_Z(op) {                        \
        if (IS_YU_OPERAND(op)) {                \
            *LOCAL_TOP-- = YC(op >> 2);         \
        } else if (IS_YV_OPERAND(op)) {         \
            if (op == 0) {                      \
                *LOCAL_TOP = (BPLONG)LOCAL_TOP; \
                LOCAL_TOP--;                    \
            } else {                            \
                op = (BPLONG)Y(op >> 2);        \
                *LOCAL_TOP-- = FOLLOW(op) = op; \
            }                                   \
        } else {                                \
            *LOCAL_TOP-- = op;                  \
        }                                       \
    }

#define OP_ZC(op)                               \
    if (IS_YU_OPERAND(op)) {                    \
        op = YC(op >> 2);                       \
    } else if (IS_YV_OPERAND(op)) {             \
        if (op != 0) {                          \
            op = (BPLONG)Y(op >> 2);            \
            FOLLOW(op) = op;                    \
        } else {                                \
            op = (BPLONG)H;                     \
            NEW_HEAP_FREE;                      \
        }                                       \
    }


#define OP_ZC_DEREF(op)                         \
    if (IS_YU_OPERAND(op)) {                    \
        op = YC(op >> 2); DEREF(op);            \
    } else if (IS_YV_OPERAND(op)) {             \
        if (op != 0) {                          \
            op = (BPLONG)Y(op >> 2);            \
            FOLLOW(op) = op;                    \
        } else {                                \
            op = (BPLONG)H;                     \
            NEW_HEAP_FREE;                      \
        }                                       \
    }

#define OP_NOVY_DEREF(op)                       \
    if (IS_YU_OPERAND(op)) {                    \
        op = YC(op >> 2); DEREF(op);            \
    }

#define OP_NOVY_DEREF_NONVAR(op)                \
    if (IS_YU_OPERAND(op)) {                    \
        op = YC(op >> 2); DEREF_NONVAR(op);     \
    }

/* z1,z2,..,zn,tail */
#define BUILD_REST_LIST_Zs(n, lab1, lab2, lab3) {       \
        op3 = *P++; BUILD_ARG_Z(op3, lab1); H++;        \
        n--;                                            \
        while (n > 0) {                                 \
            FOLLOW(H) = ADDTAG(H+1, LST); H++;          \
            n--;                                        \
            op3 = *P++; BUILD_ARG_Z(op3, lab2); H++;    \
        };                                              \
        op3 = *P++; BUILD_ARG_Z(op3, lab3);             \
        H++;                                            \
    }

#define BUILD_REST_COMP_LIST_Zs(n, lab1, lab2) {        \
        op3 = *P++; BUILD_ARG_Z(op3, lab1); H++;        \
        n--;                                            \
        while (n > 0) {                                 \
            FOLLOW(H) = ADDTAG(H+1, LST); H++;          \
            n--;                                        \
            op3 = *P++; BUILD_ARG_Z(op3, lab2); H++;    \
        };                                              \
        FOLLOW(H++) = nil_sym;                          \
    }

#define BUILD_Z(op, lab) {op = *P++; BUILD_ARG_Z(op, lab); H++;}
#define BUILD_ARG_Z(op, lab) {                  \
        if (IS_YU_OPERAND(op)) {                \
            op = YC(op >> 2);                   \
            BUILD_ARG_U_VALUE(op, lab);         \
        } else if (IS_YV_OPERAND(op)) {         \
            FOLLOW(H) = (BPLONG)H;              \
            if (op != 0) {                      \
                op = (BPLONG)Y(op >> 2);        \
                PUSHTRAIL_s(op);                \
                FOLLOW(op) = (BPLONG)H;         \
            }                                   \
        } else {                                \
            FOLLOW(H) = op;                     \
        }                                       \
    }

#define BUILD_Z0(op, lab) {op = *P++; BUILD_ARG_Z0(op, lab); H++;}
#define BUILD_ARG_Z0(op, lab) {                 \
        if (IS_YU_OPERAND(op)) {                \
            op = YC(op >> 2);                   \
            BUILD_ARG_U_VALUE(op, lab);         \
        } else if (IS_YV_OPERAND(op)) {         \
            FOLLOW(H) = (BPLONG)H;              \
            if (op != 0) {                      \
                YC(op >> 2) = (BPLONG)H;        \
            }                                   \
        } else {                                \
            FOLLOW(H) = op;                     \
        }                                       \
    }

#define BUILD_V0 {YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;}

#define BUILD_W {FOLLOW(H) = (BPLONG)H; H++;}

#define BUILD_ARG_V(op) {                       \
        op = (BPLONG)Y(op);                     \
        PUSHTRAIL_s(op);                        \
        FOLLOW(op) = FOLLOW(H) = (BPLONG)H;     \
    }

#define BUILD_U(lab) {                                          \
        op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, lab); H++;       \
    }
#define BUILD_ARG_U_VALUE(op, lab) {                                    \
        SWITCH_OP_STACK_VAR(op, lab,                                    \
                            {PUSHTRAIL_s(op);                           \
                                FOLLOW(op) = FOLLOW(H) = (BPLONG)H;},   \
                            {FOLLOW(H) = op;});                         \
    }

#define UNIFY_ARG_Z(arg_ptr, op1, lab) {                                \
        if (IS_YV_OPERAND(op1)) {                                       \
            if (op1 != 0) {                                             \
                op1 = (BPLONG)Y(op1 >> 2);                              \
                PUSHTRAIL_s(op1);                                       \
                FOLLOW(op1) = FOLLOW(arg_ptr);                          \
            }                                                           \
        } else if (IS_YU_OPERAND(op1)) {                                \
            if (!unify(YC(op1 >> 2), FOLLOW(arg_ptr))) BACKTRACK;       \
        } else {                                                        \
            op2 = FOLLOW(arg_ptr);                                      \
            UNIFY_ARG_CONST(op2, op1, lab);                             \
        }                                                               \
    }

#define UNIFY_ARG_Z0(arg_ptr, op1, lab) {                               \
        if (IS_YV_OPERAND(op1)) {                                       \
            if (op1 != 0) {                                             \
                YC(op1 >> 2) = FOLLOW(arg_ptr);                         \
            }                                                           \
        } else if (IS_YU_OPERAND(op1)) {                                \
            if (!unify(YC(op1 >> 2), FOLLOW(arg_ptr))) BACKTRACK;       \
        } else {                                                        \
            op2 = FOLLOW(arg_ptr);                                      \
            UNIFY_ARG_CONST(op2, op1, lab);                             \
        }                                                               \
    }

#define UNIFY_ARG_V(arg_ptr, op1) {             \
        op1 = (BPLONG)Y(op1);                   \
        PUSHTRAIL_s(op1);                       \
        FOLLOW(op1) = FOLLOW(arg_ptr);          \
    }

#define UNIFY_ARG_V0(arg_ptr, op1) {            \
        YC(op1) = FOLLOW(arg_ptr);              \
    }

#define UNIFY_LAST_ARG_Z(arg_ptr, op1, lab1, lab2, lab3) {      \
        if (IS_YV_OPERAND(op1)) {                               \
            if (op1 != 0) {                                     \
                op1 = (BPLONG)Y(op1 >> 2);                      \
                PUSHTRAIL_s(op1);                               \
                FOLLOW(op1) = FOLLOW(arg_ptr);                  \
            }                                                   \
        } else if (IS_YU_OPERAND(op1)) {                        \
            op2 = YC(op1 >> 2);                                 \
            op1 = FOLLOW(arg_ptr);                              \
            SWITCH_OP(op1, lab1,                                \
                      {goto unify_nsv_d;},                      \
                      {UNIFY_D_CONST(op2, op1, lab2);},         \
                      {goto unify_lst_d;},                      \
                      {goto unify_str_d;},                      \
                      {goto unify_susp_d;});                    \
        } else {                                                \
            op2 = FOLLOW(arg_ptr);                              \
            UNIFY_ARG_CONST(op2, op1, lab3);                    \
        }                                                       \
    }

#define UNIFY_LAST_ARG_Z0(arg_ptr, op1, lab1, lab2, lab3) {     \
        if (IS_YV_OPERAND(op1)) {                               \
            if (op1 != 0) {                                     \
                YC(op1 >> 2) = FOLLOW(arg_ptr);                 \
            }                                                   \
        } else if (IS_YU_OPERAND(op1)) {                        \
            op2 = YC(op1 >> 2);                                 \
            op1 = FOLLOW(arg_ptr);                              \
            SWITCH_OP(op1, lab1,                                \
                      {goto unify_nsv_d;},                      \
                      {UNIFY_D_CONST(op2, op1, lab2);},         \
                      {goto unify_lst_d;},                      \
                      {goto unify_str_d;},                      \
                      {goto unify_susp_d;});                    \
        } else {                                                \
            op2 = FOLLOW(arg_ptr);                              \
            UNIFY_ARG_CONST(op2, op1, lab3);                    \
        }                                                       \
    }

#define UNIFY_Z_CONST(op, value, lab)           \
    if (IS_YU_OPERAND(op)) {                    \
        op = YC(op >> 2);                       \
        UNIFY_D_CONST(op, value, lab);          \
    } else if (IS_YV_OPERAND(op)) {             \
        if (op != 0) {                          \
            YC(op >> 2) = value;                \
        }                                       \
    } else if (op != value) BACKTRACK;


#define UNIFY_D_CONST(op, value, lab) {                         \
        SWITCH_OP_VAR(op, lab,                                  \
                      {FOLLOW(op) = value;                      \
                          PUSHTRAIL(op);},                      \
                      {if (!unify(op, value)) BACKTRACK;},      \
                      {if (op != value) BACKTRACK;});           \
    }

#define UNIFY_ARG_CONST(op, value, lab) {                       \
        SWITCH_OP_VAR(op, lab,                                  \
                      {FOLLOW(op) = value;                      \
                          PUSHTRAIL_h(op);},                    \
                      {if (!unify(op, value)) BACKTRACK;},      \
                      {if (op != value) BACKTRACK;});           \
    }

#define GLOBALIZE_WHEN_NEEDED(op, lab) {        \
        SWITCH_OP_STACK_VAR(op, lab,            \
                            {GLOBALIZE(op);},   \
                            {});}

#define GLOBALIZE(op) {                         \
        PUSHTRAIL_s(op);                        \
        FOLLOW(op) = (BPLONG)H;                 \
        op = (BPLONG)H;                         \
        NEW_HEAP_FREE;}

/* copy out the slots that are to be used by the tail call */
#define COPY_TAIL_ZARGS_OUT(layout, tmp_ar)                             \
    do {                                                                \
        i = (layout & 0xf); layout = (BPULONG)layout >> 4;              \
        op1 = FOLLOW(P+i);                                              \
        if (IS_YU_OPERAND(op1)) {                                       \
            op1 >>= 2;                                                  \
            top = Y(op1); op2 = FOLLOW(top);                            \
            if ((BPLONG_PTR)op2 == top) {FOLLOW(top) = op2 = (BPLONG)H; NEW_HEAP_FREE;}; \
            FOLLOW(tmp_ar+op1) = op2;                                   \
        };                                                              \
    } while (layout != 0);

#define COPY_TAIL_YARGS_OUT(layout, tmp_ar)                             \
    do {                                                                \
        i = (layout & 0xf); layout = (BPULONG)layout >> 4;              \
        op1 = (BPLONG)FOLLOW(P+i);                                      \
        top = Y(op1); op2 = FOLLOW(top);                                \
        if ((BPLONG_PTR)op2 == top) {FOLLOW(top) = op2 = (BPLONG)H; NEW_HEAP_FREE;}; \
        FOLLOW(tmp_ar+op1) = op2;                                       \
    } while (layout != 0)


#define REARRANGE_TAIL_ZARGS(ar_btm_ptr, layout, tmp_ar)        \
    do {                                                        \
        i = (layout & 0xf); layout = (BPULONG)layout >> 4;      \
        op1 = FOLLOW(P+i);                                      \
        if (IS_YU_OPERAND(op1)) {                               \
            op1 >>= 2;                                          \
            op1 = FOLLOW(tmp_ar+op1);                           \
        } else if (IS_YV_OPERAND(op1)) {                        \
            if (op1 != 0) {                                     \
                op1 >>= 2;                                      \
                FOLLOW(tmp_ar+op1) = (BPLONG)H;                 \
            };                                                  \
            op1 = (BPLONG)H;                                    \
            NEW_HEAP_FREE;                                      \
        }                                                       \
        FOLLOW(ar_btm_ptr-i) = op1;                             \
    } while (layout != 0)

#define REARRANGE_TAIL_YARGS(ar_btm_ptr, layout, tmp_ar)        \
    do {                                                        \
        i = (layout & 0xf); layout = (BPULONG)layout >> 4;      \
        op1 = FOLLOW(P+i);                                      \
        FOLLOW(ar_btm_ptr-i) = FOLLOW(tmp_ar+op1);              \
    } while (layout != 0)

#define ASSIGN_in_toam(op, value, TrailCode)    \
    FOLLOW(op) = value;                         \
    TrailCode;

#define GRA(i, arity) *(AR+arity-i+1)

#ifdef GCC
#define CONTCASE goto **(void **)P++
#else
#define CONTCASE goto contcase
#endif

#define BACKTRACK goto lab_fail
#define BACKTRACK0 goto lab_fail0

#define toam_LOCAL_OVERFLOW_CHECK_SMALL_MARGIN(num)     \
    if (LOCAL_TOP - H <= LARGE_MARGIN) {                \
        printf("(in place %d) ", num);                  \
        SAVE_AR;                                        \
        myquit(STACK_OVERFLOW, "toam");}

#define toam_LOCAL_OVERFLOW_CHECK(num)          \
    if (LOCAL_TOP - H <= LARGE_MARGIN) {        \
        printf("(in place %d) ", num);          \
        SAVE_AR;                                \
        myquit(STACK_OVERFLOW, "toam");}

#define RAISE_ISO_EXCEPTION(exc, name, arity) {                         \
        bp_exception = (exc == (BPLONG)NULL) ? unknown_exception : exc; \
        error_goal_name = name;                                         \
        error_goal_arity = arity;                                       \
        goto catch_exception;                                           \
    }

#define ASSIGN_ENCODEFLOAT(op1, value)          \
    PUSHTRAIL_s(op1);                           \
    FOLLOW(op1) = encodefloat1(value);          \

#define FORK AR_CPF(AR) = *P++

#define SET_FORK AR_CPF(AR) = tmp_op

#define SET_FORK1 AR_CPF(AR) = *(P+1)

/* handle simple cases before calling bp_call_term_catch(term)
   if term == true, do nothing
   if term == dec_ref then frame must be (1,dec_ref,PredPtr,_,Cleanup,Head,Body), call b_DEC_PRED_REF_COUNTc(PredPtr)
   if term == dec_retr then frame must be (1,dec_ref,PredPtr,_,Cleanup,Head,Body), call b_DEC_PRED_RETR_COUNT_c(PredPtr)
   
   GC is disenabled before bp_call_term_catche since the number of variables that need to be initialized in the 
   current frame is unkonwn after cut.

   printf("<=bp_call_term breg=%x local_top=%x ar=%x",B,LOCAL_TOP,AR); write_term(term); printf("\n");
*/
#define TOAM_CALL_TERM(term, btm_ptr) {                                 \
        if (term == true_atom) {}                                       \
        else if (term == dec_ref_atom) {                                \
            b_DEC_PRED_REF_COUNT_c(FOLLOW(btm_ptr-2));                  \
        } else if (term == dec_retr_atom) {                             \
            b_DEC_PRED_RETR_COUNT_c(FOLLOW(btm_ptr-2));                 \
        } else {                                                        \
            int old_bp_gc = bp_gc;                                      \
            SAVE_AR; SAVE_TOP;                                          \
            bp_gc = 0;                                                  \
            if (bp_call_term_catch(term) == BP_ERROR) {                 \
                if (bp_exception == (BPLONG)NULL) bp_exception = unknown_exception; \
                goto forward_exception_as_is;                           \
            }                                                           \
            bp_gc = old_bp_gc;                                          \
        }                                                               \
    }

/* Two things are done in ROLL_CHOICE_POINTS
   1. If the choice point is an incomplete tabled frame, set the AR field of the subgoal table entry to NULL;
   2. If the choice point is a call_cleanup frame (Flag,Cleanup,Call,Exception,Recovery,...), call Cleanup
*/
#define ROLL_CHOICE_POINTS(to_b) {                                      \
        BPLONG_PTR b, btm_ptr, gtable;                                  \
        if (B < to_b) {                                                 \
            b = B;                                                      \
            do {                                                        \
                if (IS_CATCHER_OR_TABLE_FRAME(b)) {                     \
                    if (IS_TABLE_FRAME(b)) {                            \
                        gtable = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(b);   \
                        if (gtable != NULL && GT_TOP_AR(gtable) != SUBGOAL_COMPLETE) { \
                            GT_TOP_AR(gtable) = (BPLONG)NULL;           \
                        }                                               \
                    } else {                                            \
                        BPLONG cleanup;                                 \
                        btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(b)); \
                        cleanup = FOLLOW(btm_ptr-1);                    \
                        TOAM_CALL_TERM(cleanup, btm_ptr);               \
                    }                                                   \
                }                                                       \
                b = (BPLONG_PTR)AR_B(b);                                \
            } while (b < to_b);                                         \
        }                                                               \
    }

#define CUT                                             \
    sreg = (BPLONG_PTR)AR_B(AR);                        \
    ROLL_CHOICE_POINTS(sreg);                           \
    B = (BPLONG_PTR)AR_B(AR);                           \
    HB = (BPLONG_PTR)AR_H(B);                           \
    if (SF > AR) LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);    \


#define CUT0                                    \
    B = (BPLONG_PTR)AR_B(AR);                   \
    HB = (BPLONG_PTR)AR_H(B);                   \

#define UNIFY_NIL_Y(op1, lab, VarCode, SucCode, Fail)                   \
    SWITCH_OP_NIL(op1, lab,                                             \
                  {FOLLOW(op1) = nil_sym;                               \
                      VarCode;                                          \
                      CONTCASE;},                                       \
                  {SucCode; CONTCASE;},                                 \
                  {SucCode; op2 = nil_sym; goto bind_suspvar_value;});  \
    Fail

#define UNIFY_CONSTANT_12(op1, op2, lab, VarCode, SucCode, Fail)        \
    SWITCH_OP_ATM(op1, lab,                                             \
                  {FOLLOW(op1) = op2;                                   \
                      VarCode;                                          \
                      CONTCASE;},                                       \
                  {if (op1 == op2) { SucCode; CONTCASE;}},              \
                  {SucCode; goto bind_suspvar_value;});                 \
    Fail

#define UNIFY_CONSTANT_21(op1, op2, lab, VarCode, SucCode, Fail)        \
    SWITCH_OP_ATM(op1, lab,                                             \
                  {FOLLOW(op1) = op2;                                   \
                      VarCode;                                          \
                      CONTCASE;},                                       \
                  {if (op1 == op2) {SucCode; CONTCASE;}},               \
                  {SucCode; goto bind_value_suspvar;});                 \
    Fail

#define UNIFY_CONSTANT_21_CONT(op1, op2, lab, VarCode, SucCode, Fail, CONT) \
    SWITCH_OP_ATM(op1, lab,                                             \
                  {FOLLOW(op1) = op2;                                   \
                      VarCode;                                          \
                      CONT;},                                           \
                  {if (op1 == op2) {SucCode; CONT;}},                   \
                  {SucCode; goto bind_value_suspvar;});                 \
    Fail

#define UNIFY_CONSTANT_21_NOSTACK_VAR(op2, op1, lab, Fail)      \
    SWITCH_OP_ATM(op2, lab,                                     \
                  {FOLLOW(op2) = op1;                           \
                      PUSHTRAIL_h(op2);                         \
                      CONTCASE;},                               \
                  {if (op1 == op2) CONTCASE;},                  \
                  {goto bind_value_suspvar;});                  \
    Fail

#define RETURN_DET                                                      \
    P = (BPLONG_PTR)AR_CPS(AR);                                         \
    if (AR_IS_TOP(AR)) LOCAL_TOP = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR)); \
    AR = (BPLONG_PTR)AR_AR(AR);

#define RETURN_NONDET                           \
    P = (BPLONG_PTR)AR_CPS(AR);                 \
    AR = (BPLONG_PTR)AR_AR(AR);

#define BIND_SUSPVAR_VALUE(op1, op2) {          \
        BIND_SUSPVAR_VALUE_AUX(op1, op2);       \
        CONTCASE;}

#define BIND_SUSPVAR_VALUE_AUX(op1, op2) {              \
        top = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);     \
        if (!IS_UN_DOMAIN(top)) {                       \
            if (!ISINT(op2)) BACKTRACK;                 \
            op3 = INTVAL(op2);                          \
            if (!dm_true(top, op3)) BACKTRACK;          \
            INSERT_TRIGGER_dvar_ins(top);               \
        } else {                                        \
            INSERT_TRIGGER_var_ins(top);}               \
        PUSHTRAIL_H_NONATOMIC(top, op1);                \
        FOLLOW(top) = op2;}

#define BIND_SUSPVAR_NONINT(op1, op2) {                 \
        top = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);     \
        if (!IS_UN_DOMAIN(top)) {                       \
            BACKTRACK;                                  \
        }                                               \
        INSERT_TRIGGER_var_ins(top);                    \
        PUSHTRAIL_H_NONATOMIC(top, op1);                \
        FOLLOW(top) = op2;}


#define BIND_DVAR_VALUE(op1, op2) {                     \
        PUSHTRAIL_H_NONATOMIC(op1, FOLLOW(op1));        \
        FOLLOW(op1) = op2;                              \
        INSERT_TRIGGER_dvar_ins(op1); }

#define CATCH_INTERRUPT                         \
    if (toam_signal_vec & INTERRUPT) {          \
        bp_exception = interrupt_sym;           \
        goto interrupt_handler;                 \
    }

#define TOAM_KILL_SUSP_FRAME {                  \
        AR_STATUS(AR) = SUSP_EXIT;              \
    }

#define CATCH_WAKE_EVENT if (toam_signal_vec != 0) {                    \
        if (toam_signal_vec & INTERRUPT) {bp_exception = interrupt_sym; goto interrupt_handler;} \
        if (toam_signal_vec & EVENT_POOL_NONEMPTY) post_event_pool();   \
        if (trigger_no != 0) goto trigger_on_handler;                   \
    }

#define CONNECT_SUSP_FRAME {                    \
        AR_PREV(AR) = (BPLONG)SF;               \
        SF = AR;                                \
    }

/* frame has been added to the active chain. make a copy of it ontop of the stack */
#define CLONE_FRAME(frame)                                              \
    BPLONG_PTR sp, sp_c, frame_c;                                       \
    toam_LOCAL_OVERFLOW_CHECK_SMALL_MARGIN(6);                          \
    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(frame)); sp_c = LOCAL_TOP;    \
    while (sp > frame) {                                                \
        FOLLOW(sp_c--) = FOLLOW(sp--);                                  \
    }                                                                   \
    frame_c = sp_c;                                                     \
    sp = frame-SUSP_FRAME_SIZE; sp_c -= SUSP_FRAME_SIZE;                \
    top = (BPLONG_PTR)AR_TOP(frame);                                    \
    while(sp > top) {                                                   \
        FOLLOW(sp_c--) = FOLLOW(sp--);                                  \
    }                                                                   \
    AR_REEP(frame_c) = AR_REEP(frame);                                  \
    AR_BTM(frame_c) = ADDTAG((BPLONG)LOCAL_TOP, SUSP_FRAME_TAG);        \
    AR_TOP(frame_c) = (BPLONG)sp_c;                                     \
    AR_STATUS(frame_c) = SUSP_CLONE;                                    \
    LOCAL_TOP = sp_c;

#define CONNECT_WOKEN_FRAME(frame) {            \
        AR_AR(frame) = (BPLONG)AR;              \
        AR_CPS(frame) = (BPLONG)P;              \
        AR = frame;                             \
    }

#define CONNECT_WOKEN_FRAME_ins(frame)                                  \
    if (FRAME_IS_SLEEP(frame)) {                                        \
        if (frame > B && AR_STATUS_STAMP(frame) != ((BPULONG)stack_up_addr-(BPULONG)B)) { \
            BPLONG tmp_op;                                              \
            tmp_op = (BPLONG)AR_STATUS_ADDR(frame);                     \
            PUSHTRAILC_ATOMIC(tmp_op, FOLLOW(tmp_op));                  \
            AR_STATUS(frame) = ((BPULONG)stack_up_addr-(BPULONG)B) | SUSP_RUN; \
        } else {                                                        \
            AR_STATUS(frame) = AR_STATUS_STAMP(frame) | SUSP_RUN;       \
        }                                                               \
        CONNECT_WOKEN_FRAME(frame);                                     \
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));                            \
    } else if (AR_STATUS(frame) == SUSP_EXIT) {                         \
    } else {                                                            \
        AR_STATUS(frame) = AR_STATUS_STAMP(frame) | SUSP_REENT;         \
    }                                                                   \

#define CONNECT_WOKEN_FRAME_min_max(frame)                              \
    if (FRAME_IS_SLEEP(frame)) {                                        \
        if (frame > B && AR_STATUS_STAMP(frame) != ((BPULONG)stack_up_addr-(BPULONG)B)) { \
            BPLONG tmp_op;                                              \
            tmp_op = (BPLONG)AR_STATUS_ADDR(frame);                     \
            PUSHTRAILC_ATOMIC(tmp_op, FOLLOW(tmp_op));                  \
            AR_STATUS(frame) = ((BPULONG)stack_up_addr-(BPULONG)B) | SUSP_RUN; \
        } else {                                                        \
            AR_STATUS(frame) = AR_STATUS_STAMP(frame) | SUSP_RUN;       \
        }                                                               \
        CONNECT_WOKEN_FRAME(frame);                                     \
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));                            \
    }

#define CONNECT_WOKEN_FRAME_dom(frame, elm)                             \
    if (FRAME_IS_SLEEP(frame)) {                                        \
        if (frame > B && AR_STATUS_STAMP(frame) != ((BPULONG)stack_up_addr-(BPULONG)B)) { \
            BPLONG tmp_op;                                              \
            tmp_op = (BPLONG)AR_STATUS_ADDR(frame);                     \
            PUSHTRAILC_ATOMIC(tmp_op, FOLLOW(tmp_op));                  \
            AR_STATUS(frame) = ((BPULONG)stack_up_addr-(BPULONG)B) | SUSP_RUN; \
        } else {                                                        \
            AR_STATUS(frame) = AR_STATUS_STAMP(frame) | SUSP_RUN;       \
        }                                                               \
        AR_OUT(frame) = elm;                                            \
        CONNECT_WOKEN_FRAME(frame);                                     \
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));                            \
    } else if (FRAME_IS_RUN(frame)) {                                   \
        CLONE_FRAME(frame);                                             \
        AR_OUT(frame_c) = elm;                                          \
        CONNECT_WOKEN_FRAME(frame_c);                                   \
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));                            \
    }



#define CONNECT_WOKEN_FRAME_cg(frame, elm)                              \
    if (FRAME_IS_SLEEP(frame)) {                                        \
        if (frame > B && AR_STATUS_STAMP(frame) != ((BPULONG)stack_up_addr-(BPULONG)B)) { \
            BPLONG tmp_op;                                              \
            tmp_op = (BPLONG)AR_STATUS_ADDR(frame);                     \
            PUSHTRAILC_ATOMIC(tmp_op, FOLLOW(tmp_op));                  \
            AR_STATUS(frame) = ((BPULONG)stack_up_addr-(BPULONG)B) | SUSP_RUN; \
        } else {                                                        \
            AR_STATUS(frame) = AR_STATUS_STAMP(frame) | SUSP_RUN;       \
        }                                                               \
        AR_OUT(frame) = elm;                                            \
        CONNECT_WOKEN_FRAME(frame);                                     \
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));                            \
    }

#define MIN_AVAIL_WORDS 1000000

#define RESET_WATER_MARKS {                                     \
        available_sh_space = (LOCAL_TOP-H)/gc_threshold;        \
        stack_water_mark = LOCAL_TOP-available_sh_space;        \
        heap_water_mark = H+available_sh_space;                 \
    }

#define EXPAND_STACK(margin) if (LOCAL_TOP - H <= margin) {     \
        if (toam_signal_vec == 0 && in_critical_region == 0) {  \
            gc_is_working = 1;                                  \
            if (expand_local_global_stacks(0) == BP_ERROR) {    \
                fprintf(stderr, "%% error: OUT OF MEMORY\n");   \
                bp_exception = et_OUT_OF_MEMORY_STACK;          \
                goto interrupt_handler;                         \
            }                                                   \
            RESTORE_AR; RESTORE_TOP;                            \
            gc_is_working = 0;                                  \
        }                                                       \
    }

#define INVOKE_GC_UNCOND {                                      \
    if (bp_gc) {                                                \
        SAVE_AR; SAVE_TOP;                                      \
        if (garbage_collector() == BP_ERROR) {                  \
            bp_exception = et_OUT_OF_MEMORY_STACK;              \
            fprintf(stderr, "%% error: OUT OF MEMORY\n");       \
            goto interrupt_handler;                             \
        }                                                       \
        RESTORE_AR; RESTORE_TOP;                                \
        EXPAND_STACK(LARGE_MARGIN);                             \
    }                                                           \
        toam_LOCAL_OVERFLOW_CHECK(7);                           \

/*
  #define INVOKE_GC INVOKE_GC_UNCOND 
*/

#define INVOKE_GC {                                                     \
        if (LOCAL_TOP - H <= MIN_AVAIL_WORDS) {                         \
            if (bp_gc) {                                                \
                SAVE_AR; SAVE_TOP;                                      \
                if (garbage_collector() == BP_ERROR) {                  \
                    bp_exception = et_OUT_OF_MEMORY_STACK;              \
                    fprintf(stderr, "%% error: OUT OF MEMORY\n");       \
                    goto interrupt_handler;                             \
                }                                                       \
                RESTORE_AR; RESTORE_TOP;                                \
                EXPAND_STACK(MIN_AVAIL_WORDS+LARGE_MARGIN);             \
                RESET_WATER_MARKS;                                      \
            }                                                           \
            toam_LOCAL_OVERFLOW_CHECK(8);                               \
        }                                                               \
    }

/*
  #define INVOKE_GC_NONDET INVOKE_GC_UNCOND     
*/

/*
  #define INVOKE_GC_NONDET {                                                                                      \
  if (LOCAL_TOP - H > MIN_AVAIL_WORDS && H < heap_water_mark && LOCAL_TOP > stack_water_mark){ \
  } else {                                                                                                                        \
  if (bp_gc && ((gc_b != B) || LOCAL_TOP - H <= MIN_AVAIL_WORDS)){  \
  SAVE_AR;SAVE_TOP;                                                                                               \
  gc_b = B;                                                                                                               \
  if (garbage_collector()==BP_ERROR){                                                             \
  bp_exception = et_OUT_OF_MEMORY_STACK;                                                        \
  fprintf(stderr,"%% error: OUT OF MEMORY\n");                                  \
  goto interrupt_handler;                                                                               \
  }                                                                                                                               \
  RESTORE_AR;RESTORE_TOP;                                                                                 \
  EXPAND_STACK(MIN_AVAIL_WORDS+LARGE_MARGIN);                                             \
  RESET_WATER_MARKS;                                                                                              \
  }                                                                                                                                 \
  toam_LOCAL_OVERFLOW_CHECK(5);                                                                         \
  }                                                                                                                                       \
  }
*/
#define INVOKE_GC_NONDET {                                              \
        if (bp_gc && ((LOCAL_TOP - H <= MIN_AVAIL_WORDS) || (H >= heap_water_mark) || (LOCAL_TOP < stack_water_mark))) { \
            SAVE_AR; SAVE_TOP;                                          \
            if (garbage_collector() == BP_ERROR) {                      \
                bp_exception = et_OUT_OF_MEMORY_STACK;                  \
                fprintf(stderr, "%% error: OUT OF MEMORY\n");           \
                goto interrupt_handler;                                 \
            }                                                           \
            RESTORE_AR; RESTORE_TOP;                                    \
            EXPAND_STACK(MIN_AVAIL_WORDS+LARGE_MARGIN);                 \
            RESET_WATER_MARKS;                                          \
        }                                                               \
        toam_LOCAL_OVERFLOW_CHECK(5);                                   \
    }

#define REAL_INITIALIZE_STACK_VARS(N) {         \
        top = AR-N;                             \
        while (top > LOCAL_TOP) {               \
            FOLLOW(top) = (BPLONG)top;          \
            top--;                              \
        }                                       \
    }

#define INITIALIZE_STACK_VARS(N)

#define CALL_PROPAGATE_SCC_ROOT {                                       \
        BPLONG_PTR scc_root, scc_root_ar;                               \
        scc_root = (BPLONG_PTR)GT_SCC_ROOT(subgoal_entry);              \
        scc_root_ar = (BPLONG_PTR)GT_TOP_AR(scc_root);                  \
        if ((BPLONG)scc_root_ar != SUBGOAL_COMPLETE) {                  \
            while ((BPLONG)scc_root_ar == SUBGOAL_TEMP_COMPLETE) {      \
                scc_root = (BPLONG_PTR)GT_SCC_ROOT(scc_root);           \
                scc_root_ar = (BPLONG_PTR)GT_TOP_AR(scc_root);          \
            }                                                           \
            propagate_scc_root((BPLONG_PTR)AR_AR(AR), subgoal_entry, scc_root, scc_root_ar); \
        }                                                               \
    }

#define RESET_SUBGOAL_AR(f) {                                           \
        if (IS_TABLE_FRAME(f)) {                                        \
            BPLONG_PTR subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(f); \
            if (subgoal_entry != NULL && GT_TOP_AR(subgoal_entry) != SUBGOAL_COMPLETE) { \
                reset_temp_complete_scc_elms(subgoal_entry);            \
                GT_TOP_AR(subgoal_entry) = (BPLONG)NULL;                \
            }                                                           \
        }                                                               \
    }




