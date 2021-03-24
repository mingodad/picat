/********************************************************************
 *   File   : emu_inst.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021
 *   THIS FILE IS AUTOMATICALLY ASSEMBLED. 

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#ifndef GCC
switch (*P++) {
#endif
#ifndef GCC
case noop:  /* E */
#endif
lab_noop:
    CONTCASE;

#ifndef GCC
case noop1:  /* i */
#endif
lab_noop1:
    P++;
    CONTCASE;

#ifndef GCC
case allocate_det_b:  /* i,i,s,i */
#endif
lab_allocate_det_b:
    AR_B(AR) = (BPLONG)B;
    AR_BTM(AR) = ADDTAG((AR+Operand0), FLAT_B_FRAME_TAG);
    LOCAL_TOP = AR-Operand(1);
    P += 4;
    INITIALIZE_STACK_VARS(FLAT_B_FRAME_SIZE);
    AR_TOP(AR) = (BPLONG)LOCAL_TOP;
    INVOKE_GC;
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case nondet:  /* i */
#endif
lab_nondet:
    if ((BPLONG)LOCAL_TOP == AR_TOP(AR)) {  /* turn this frame into a nondet frame */
        AR_B(AR) = (BPLONG)B;
        AR_H(AR) = (BPLONG)H;
        AR_T(AR) = (BPLONG)T;
        AR_SF(AR) = (BPLONG)SF;
        B = AR;
        HB = H;
        P++;
        CONTCASE;
    } else {  /* copy the frame */
        op1 = AR-(BPLONG_PTR)AR_TOP(AR);
        op2 = *P++;  /* arity */

        for (i = op2; i > 0; i--) {  /* copy arguments */
            *LOCAL_TOP-- = FOLLOW(AR+i);
        }

        *LOCAL_TOP = FOLLOW(AR);
        *(LOCAL_TOP-1) = AR_CPS(AR);
        AR = LOCAL_TOP;
        LOCAL_TOP = AR - op1;
        AR_BTM(AR) = ADDTAG((AR+op2), NONDET_FRAME_TAG);
        AR_TOP(AR) = (BPLONG)LOCAL_TOP;
        AR_B(AR) = (BPLONG)B;
        AR_H(AR) = (BPLONG)H;
        AR_T(AR) = (BPLONG)T;
        AR_SF(AR) = (BPLONG)SF;
        toam_LOCAL_OVERFLOW_CHECK(2);
        INITIALIZE_STACK_VARS(NONDET_FRAME_SIZE);
        B = AR;
        HB = H;
        CONTCASE;
    }

#ifndef GCC
case cut_return:  /* E */
#endif
lab_cut_return:
    /* cut, return_det */
    CUT0;
    /* goto lab_return_det; */

#ifndef GCC
case return_det:  /* E */
#endif
lab_return_det:
    RETURN_DET;
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case unify_constant_return_det:  /* y,c */
#endif
lab_unify_constant_return_det:
    op1 = NextOperandYC;
    op2 = NextOperandLiteral;
    SWITCH_OP_ATM(op1, rr_unify_constant_return_det,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = op2;
                      goto lab_return_det;},
                  {if (op1 == op2) {goto lab_return_det;}},
                  {BIND_SUSPVAR_VALUE_AUX(op1, op2);
                      goto lab_return_det;});

    BACKTRACK;

#ifndef GCC
case cut_unify_value_return_det:  /* y,y */
#endif
lab_cut_unify_value_return_det:
    CUT0;

#ifndef GCC
case unify_value_return_det:  /* y,y */
#endif
lab_unify_value_return_det:
    /* followed by return_det */
    op1 = NextOperandYC;
    op2 = NextOperandYC;
    SWITCH_OP(op1, rr_unify_value_return_det,
              {SWITCH_OP_VAR(op2, unify_value_return_det_v_d,
                             {goto unify_v_v;},
                             {goto unify_v_susp;}, {});
                  ASSIGN_in_toam(op1, op2, {PUSHTRAIL(op1);});
                  goto lab_return_det;},
              {UNIFY_CONSTANT_21_CONT(op2, op1, rrr_unify_value_return_det, {PUSHTRAIL(op2);}, {}, {BACKTRACK;}, {goto lab_return_det;});},
              {goto unify_lst_d;},
              {goto unify_str_d;},
              {goto unify_susp_d;});


#ifndef GCC
case return_nondet:  /* E */
#endif
lab_return_nondet:
    RETURN_NONDET;
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case unify_constant_return_nondet:  /* y,c */
#endif
lab_unify_constant_return_nondet:
    op1 = NextOperandYC;
    op2 = NextOperandLiteral;
    SWITCH_OP_ATM(op1, rr_unify_constant_return_nondet,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = op2;
                      goto lab_return_nondet;},
                  {if (op1 == op2) {goto lab_return_nondet;}},
                  {BIND_SUSPVAR_VALUE_AUX(op1, op2);
                      goto lab_return_nondet;});

    BACKTRACK;

#ifndef GCC
case fork_unify_constant_return_nondet:  /* l,y,c */
#endif
lab_fork_unify_constant_return_nondet:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        op2 = NextOperandLiteral;
        SWITCH_OP_ATM(op1, rr_fork_unify_constant_return_nondet,
                      {SET_FORK; PUSHTRAIL(op1);
                          FOLLOW(op1) = op2;
                          goto lab_return_nondet;},
                      {if (op1 == op2) {SET_FORK; goto lab_return_nondet;}},
                      {SET_FORK;
                          BIND_SUSPVAR_VALUE_AUX(op1, op2);
                          goto lab_return_nondet;});

        P = (BPLONG_PTR)tmp_op;
        CONTCASE;
}

#ifndef GCC
case fork_unify_nil_unify_value_return_nondet:  /* l,y,y,y */
#endif
lab_fork_unify_nil_unify_value_return_nondet:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_NIL(op1, rr_fork_unify_nil_unify_value_return_nondet,
                  {SET_FORK;
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = nil_sym;
                      goto lab_unify_value_return_nondet;},
                  {SET_FORK; goto lab_unify_value_return_nondet;},
                  {SET_FORK;
                      BIND_SUSPVAR_VALUE_AUX(op1, nil_sym);
                      goto lab_unify_value_return_nondet;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case unify_value_return_nondet:  /* y,y */
#endif
lab_unify_value_return_nondet:
    op1 = NextOperandYC;
    op2 = NextOperandYC;
    SWITCH_OP(op1, rr_unify_value_return_nondet,
              {SWITCH_OP_VAR(op2, unify_value_return_nondet_v_d,
                             {goto unify_v_v;},
                             {goto unify_v_susp;}, {});
                  ASSIGN_in_toam(op1, op2, {PUSHTRAIL(op1);});
                  goto lab_return_nondet;},
              {UNIFY_CONSTANT_21_CONT(op2, op1, rrr_unify_value_return_nondet, {PUSHTRAIL(op2);}, {}, {BACKTRACK;}, {goto lab_return_nondet;});},
              {goto unify_lst_d;},
              {goto unify_str_d;},
              {goto unify_susp_d;});


#ifndef GCC
case bp_fork:  /* l */
#endif
lab_bp_fork:
    FORK;
    CONTCASE;

#ifndef GCC
case cut:  /* E */
#endif
lab_cut:
    CUT;
    CONTCASE;

#ifndef GCC
case cut0:  /* E */
#endif
lab_cut0:
    CUT0;
    CONTCASE;

#ifndef GCC
case unify_value_cut:  /* y,y */
#endif
lab_unify_value_cut:
    /* followed by cut0 */
    op1 = NextOperandYC;
    op2 = NextOperandYC;
    SWITCH_OP(op1, rr_unify_value_cut,
              {CUT0; P++; goto unify_v_d;},  /* skip cut */
              {UNIFY_CONSTANT_21(op2, op1, rrr_unify_value_cut, {CUT0; P++; PUSHTRAIL(op2);}, {CUT0; P++;}, {BACKTRACK;});},
              {goto unify_lst_d;},  /* followed by cut0 */
              {goto unify_str_d;},
              {goto unify_susp_d;});

#ifndef GCC
case fail:  /* E */
#endif
lab_fail:
#ifdef ToamProfile
    execute_inst(fail);
#endif

    trigger_no = 0;
    toam_signal_vec &= (INTERRUPT | EVENT_POOL_NONEMPTY);
    AR = B;
    H = HB;
    SF = (BPLONG_PTR)AR_SF(AR);

    top = (BPLONG_PTR)AR_T(AR);
    while (T != top) {
        POPTRAIL(T);
    }

    LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);
    RESET_WATER_MARKS;
    /*
      if (IS_TABLE_FRAME(AR)){
      INITIALIZE_STACK_VARS(TABLE_FRAME_SIZE); 
      } else {
      INITIALIZE_STACK_VARS(NONDET_FRAME_SIZE); 
      }
    */
    P = (BPLONG_PTR)AR_CPF(AR);
    CONTCASE;


#ifndef GCC
case cut_fail:  /* E */
#endif
lab_cut_fail:
    /* cut0,fail */
    /* ROLL_TABLED_FRAME(AR_B(AR)); */
rr_cut_fail:
    B = AR = (BPLONG_PTR)AR_B(AR);
    LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);
    HB = (BPLONG_PTR)AR_H(AR);
    SF = (BPLONG_PTR)AR_SF(AR);

    /* BACKTRACK0; */

#ifndef GCC
case fail0:  /* E */
#endif
lab_fail0:
#ifdef ToamProfile
    execute_inst(fail0);
#endif
    trigger_no = 0;
    toam_signal_vec &= (INTERRUPT | EVENT_POOL_NONEMPTY);
    H = (BPLONG_PTR)HB;
    RESET_WATER_MARKS;

    top = (BPLONG_PTR)AR_T(AR);
    while (T != top) {
        POPTRAIL(T);
    }

    P = (BPLONG_PTR)AR_CPF(AR);
    /*
      if (IS_TABLE_FRAME(AR)){
      INITIALIZE_STACK_VARS(TABLE_FRAME_SIZE); 
      } else {
      INITIALIZE_STACK_VARS(NONDET_FRAME_SIZE); 
      }
    */
    CONTCASE;

#ifndef GCC
case getbreg_y:  /* y */
#endif
lab_getbreg_y:
    op1 = (BPLONG)NextOperandY;
    FOLLOW(op1) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)B, INT_TAG);
    CONTCASE;

#ifndef GCC
case putbreg_y:  /* y */
#endif
lab_putbreg_y:
    op1 = NextOperandYC;
    op1 = UNTAGGED_CONT(op1);
    sreg = (BPLONG_PTR)((BPULONG)stack_up_addr - (BPULONG)op1);  /* borrow sreg here */
    if (B > sreg) CONTCASE;
    ROLL_CHOICE_POINTS(sreg);
    B = sreg;
    HB = (BPLONG_PTR)AR_H(B);
    if (SF > AR) LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);
    CONTCASE;

#ifndef GCC
case getpbreg_y:  /* y */
#endif
lab_getpbreg_y:
    op1 = (BPLONG)NextOperandY;
    if (AR == B) {
        top = (BPLONG_PTR)AR_B(AR);
        FOLLOW(op1) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)top, INT_TAG);
    } else {
        FOLLOW(op1) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)B, INT_TAG);
    }
    CONTCASE;

#ifndef GCC
case halt:  /* E */
#endif
lab_halt:
    SAVE_AR; SAVE_TOP;
    if (bp_exception == (BPLONG)NULL) return BP_TRUE; else return BP_ERROR;

#ifndef GCC
case halt0:  /* E */
#endif
lab_halt0:
    SAVE_AR; SAVE_TOP;
    curr_toam_status = TOAM_DONE;
    if (bp_exception == (BPLONG)NULL) return BP_FALSE; else return BP_ERROR;

#ifndef GCC
case call0:  /* s,i */
#endif
lab_call0:
    sym_ptr = (SYM_REC_PTR)NextOperandSym; P++;  /* skip MaxS for GC */
call_sub:  /* call a struct (psc) */
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;

call_sub_after_ar_cps:

#ifdef DEBUG_CALL
    printf("==> call_sub");
    printf("(name : %s/%d, type : %lx)\n", GET_NAME(sym_ptr), GET_ARITY(sym_ptr), GET_ETYPE(sym_ptr));
#endif

    switch (GET_ETYPE(sym_ptr)) {
    case T_ORDI :
        AR_BTM(AR) = ADDTAG(AR+GET_ARITY(sym_ptr), FLAT_FRAME_TAG);
        LOCAL_TOP = AR-FLAT_FRAME_SIZE;
        AR_TOP(AR) = (BPLONG)LOCAL_TOP;
        RAISE_ISO_EXCEPTION(c_existence_error(et_PROCEDURE, c_error_src(GET_NAME(sym_ptr), GET_ARITY(sym_ptr))), "call", 1);

    case T_DYNA :
    case T_INTP :
        /*      printf("\n*** call dynamic pred %s/%i***\n",GET_NAME(sym_ptr),GET_ARITY(sym_ptr)); */
        arity = GET_ARITY(sym_ptr);
        if (arity == 0) {  /* rearrange the frame */
            *(AR-2) = AR_CPS(AR);
            *(AR-1) = AR_AR(AR);
            *AR = ADDTAG(sym_ptr, ATM);
            AR--;
        } else {
            BPLONG tmp_op;
            tmp_op = ADDTAG(H, STR);
            NEW_HEAP_NODE((BPLONG)sym_ptr);
            for (i = 1; i <= arity; i++) {
                BUILD_VALUE(GRA(i, arity), call_sub_dyn);
            }
            *(AR+1) = tmp_op;
        }
        P = (BPLONG_PTR) GET_EP(enter_dyn_call);
        CONTCASE;

    case T_PRED :
        P = (BPLONG_PTR) GET_EP(sym_ptr);
        CONTCASE;

    case C_PRED :{
        BPLONG old_arreg_offset;

        arity = GET_ARITY(sym_ptr);
        AR_BTM(AR) = ADDTAG((AR+arity), FLAT_FRAME_TAG);
        LOCAL_TOP = AR-FLAT_FRAME_SIZE-256;  /* in case the local stack is used by the C function */
        AR_TOP(AR) = (BPLONG)LOCAL_TOP;

        SAVE_AR; SAVE_TOP;
        old_arreg_offset = (BPULONG)stack_up_addr-(BPULONG)AR;  /* in case stack expansion occurs in the C function */
        op1 = ((*GET_EP(sym_ptr))());
        AR = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)old_arreg_offset);
        RESTORE_TOP;
        if (AR != arreg) {  /* c code created frames */
            while (B < AR) {
                B = (BPLONG_PTR)AR_B(B);
                HB = (BPLONG_PTR)AR_H(B);
            }
        }

        if (op1 == BP_TRUE) {
            if (AR_IS_TOP(AR)) LOCAL_TOP = AR+arity;
            P = (BPLONG_PTR)AR_CPS(AR);
            AR = (BPLONG_PTR)AR_AR(AR);
            CONTCASE;
        } else if (op1 == BP_FALSE) {
            BACKTRACK;
        } else {
            if (!is_iso_exception(bp_exception)) {
                switch (arity) {
                case 0: break;
                case 1: bp_exception = c_builtin_error1(bp_exception, FOLLOW(AR+1)); break;
                case 2: bp_exception = c_builtin_error2(bp_exception, FOLLOW(AR+2), FOLLOW(AR+1)); break;
                case 3: bp_exception = c_builtin_error3(bp_exception, FOLLOW(AR+3), FOLLOW(AR+2), FOLLOW(AR+1)); break;
                case 4: bp_exception = c_builtin_error4(bp_exception, FOLLOW(AR+4), FOLLOW(AR+3), FOLLOW(AR+2), FOLLOW(AR+1)); break;
                default: bp_exception = c_builtin_error4(bp_exception, FOLLOW(AR+arity), FOLLOW(AR+arity-1), FOLLOW(AR+arity-2), dots_atom);
                }
            }
            RAISE_ISO_EXCEPTION(bp_exception, GET_NAME(sym_ptr), arity);
        }
    }
    }

#ifndef GCC
case call_uv_d:  /* l,y,y,i */
#endif
lab_call_uv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uv_ot];
        goto lab_call_uv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uv_nondet;
    } else {
        FOLLOW(P-1) = call_uv_ot;
        goto lab_call_uv_ot;
    }
#endif

#ifndef GCC
case call_uv_det:  /* l,y,y,i */
#endif
lab_call_uv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 2; P += 4;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case allocate_det:  /* i,i,s,i */
#endif
lab_allocate_det:
    AR_BTM(AR) = ADDTAG((AR+Operand0), FLAT_FRAME_TAG);
    LOCAL_TOP = AR-Operand(1);
    P += 4;
    AR_TOP(AR) = (BPLONG)LOCAL_TOP;
    INITIALIZE_STACK_VARS(FLAT_FRAME_SIZE);
rr_allocate_det:
    INVOKE_GC;
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case call_uv_nondet:  /* l,y,y,i */
#endif
lab_call_uv_nondet:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 2; P += 4;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_nondet;
}

#ifndef GCC
case call_uv_ot:  /* l,y,y,i */
#endif
lab_call_uv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 2; P += 4;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uuv_d:  /* l,y,y,y,i */
#endif
lab_call_uuv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uuv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uuv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uuv_ot];
        goto lab_call_uuv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uuv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uuv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuv_nondet;
    } else {
        FOLLOW(P-1) = call_uuv_ot;
        goto lab_call_uuv_ot;
    }
#endif

#ifndef GCC
case call_uuv_det:  /* l,y,y,y,i */
#endif
lab_call_uuv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uuv_nondet:  /* l,y,y,y,i */
#endif
lab_call_uuv_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uuv_ot:  /* l,y,y,y,i */
#endif
lab_call_uuv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uvv_d:  /* l,y,y,y,i */
#endif
lab_call_uvv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uvv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uvv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uvv_ot];
        goto lab_call_uvv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uvv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uvv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvv_nondet;
    } else {
        FOLLOW(P-1) = call_uvv_ot;
        goto lab_call_uvv_ot;
    }
#endif

#ifndef GCC
case call_uvv_det:  /* l,y,y,y,i */
#endif
lab_call_uvv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uvv_nondet:  /* l,y,y,y,i */
#endif
lab_call_uvv_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uvv_ot:  /* l,y,y,y,i */
#endif
lab_call_uvv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uvc_d:  /* l,y,y,c,i */
#endif
lab_call_uvc_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uvc_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvc_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uvc_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvc_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uvc_ot];
        goto lab_call_uvc_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uvc_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvc_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uvc_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvc_nondet;
    } else {
        FOLLOW(P-1) = call_uvc_ot;
        goto lab_call_uvc_ot;
    }
#endif

#ifndef GCC
case call_uvc_det:  /* l,y,y,c,i */
#endif
lab_call_uvc_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        *(LOCAL_TOP-2) = *(P+3);
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uvc_nondet:  /* l,y,y,c,i */
#endif
lab_call_uvc_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = *(P+3);
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uvc_ot:  /* l,y,y,c,i */
#endif
lab_call_uvc_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = *(P+3);
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uvu_d:  /* l,y,y,y,i */
#endif
lab_call_uvu_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uvu_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvu_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uvu_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvu_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uvu_ot];
        goto lab_call_uvu_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uvu_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvu_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uvu_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvu_nondet;
    } else {
        FOLLOW(P-1) = call_uvu_ot;
        goto lab_call_uvu_ot;
    }
#endif

#ifndef GCC
case call_uvu_det:  /* l,y,y,y,i */
#endif
lab_call_uvu_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        *(LOCAL_TOP-2) = YC(*(P+3));
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uvu_nondet:  /* l,y,y,y,i */
#endif
lab_call_uvu_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uvu_ot:  /* l,y,y,y,i */
#endif
lab_call_uvu_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uvuv_d:  /* l,y,y,y,y,i */
#endif
lab_call_uvuv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uvuv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvuv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uvuv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvuv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uvuv_ot];
        goto lab_call_uvuv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uvuv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvuv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uvuv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uvuv_nondet;
    } else {
        FOLLOW(P-1) = call_uvuv_ot;
        goto lab_call_uvuv_ot;
    }
#endif

#ifndef GCC
case call_uvuv_det:  /* l,y,y,y,y,i */
#endif
lab_call_uvuv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
        *(LOCAL_TOP-2) = YC(*(P+3));
        op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 4; P += 6;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uvuv_nondet:  /* l,y,y,y,y,i */
#endif
lab_call_uvuv_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = YC(*(P+3));
    op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uvuv_ot:  /* l,y,y,y,y,i */
#endif
lab_call_uvuv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    *(LOCAL_TOP-2) = YC(*(P+3));
    op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uuuv_d:  /* l,y,y,y,y,i */
#endif
lab_call_uuuv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uuuv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uuuv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uuuv_ot];
        goto lab_call_uuuv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uuuv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uuuv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuv_nondet;
    } else {
        FOLLOW(P-1) = call_uuuv_ot;
        goto lab_call_uuuv_ot;
    }
#endif

#ifndef GCC
case call_uuuv_det:  /* l,y,y,y,y,i */
#endif
lab_call_uuuv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 4; P += 6;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uuuv_nondet:  /* l,y,y,y,y,i */
#endif
lab_call_uuuv_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uuuv_ot:  /* l,y,y,y,y,i */
#endif
lab_call_uuuv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    op1 = (BPLONG)Y(*(P+4)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_uuuuv_d:  /* l,y,y,y,y,y,i */
#endif
lab_call_uuuuv_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_uuuuv_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuuv_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_uuuuv_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuuv_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_uuuuv_ot];
        goto lab_call_uuuuv_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_uuuuv_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuuv_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_uuuuv_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_uuuuv_nondet;
    } else {
        FOLLOW(P-1) = call_uuuuv_ot;
        goto lab_call_uuuuv_ot;
    }
#endif

#ifndef GCC
case call_uuuuv_det:  /* l,y,y,y,y,y,i */
#endif
lab_call_uuuuv_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *LOCAL_TOP = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        op1 = (BPLONG)Y(*(P+5)); *(LOCAL_TOP-4) = FOLLOW(op1) = op1;
        LOCAL_TOP -= 5; P += 7;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_uuuuv_nondet:  /* l,y,y,y,y,y,i */
#endif
lab_call_uuuuv_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    op1 = (BPLONG)Y(*(P+5)); *(LOCAL_TOP-4) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_uuuuv_ot:  /* l,y,y,y,y,y,i */
#endif
lab_call_uuuuv_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *LOCAL_TOP = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    op1 = (BPLONG)Y(*(P+5)); *(LOCAL_TOP-4) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call0_d:  /* l,i */
#endif
lab_call0_d:
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)(P+2);  /* skip MaxS for GC */
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case call_v_d:  /* l,y,i */
#endif
lab_call_v_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_v_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_v_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_v_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_v_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_v_ot];
        goto lab_call_v_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_v_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_v_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_v_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_v_nondet;
    } else {
        FOLLOW(P-1) = call_v_ot;
        goto lab_call_v_ot;
    }
#endif

#ifndef GCC
case call_v_det:  /* l,y,i */
#endif
lab_call_v_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P++;
        op1 = (BPLONG)Y(*P); P += 2; FOLLOW(op1) = op1; *LOCAL_TOP-- = op1;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_v_nondet:  /* l,y,i */
#endif
lab_call_v_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P++;
    op1 = (BPLONG)Y(*P); P += 2; FOLLOW(op1) = op1; *LOCAL_TOP-- = op1;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_v_ot:  /* l,y,i */
#endif
lab_call_v_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P++;
    op1 = (BPLONG)Y(*P); P += 2; FOLLOW(op1) = op1; *LOCAL_TOP-- = op1;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_u_d:  /* l,y,i */
#endif
lab_call_u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_u_ot];
        goto lab_call_u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_u_nondet;
    } else {
        FOLLOW(P-1) = call_u_ot;
        goto lab_call_u_ot;
    }
#endif

#ifndef GCC
case call_u_det:  /* l,y,i */
#endif
lab_call_u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P++;
        *LOCAL_TOP-- = YC(*P); P += 2;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_u_nondet:  /* l,y,i */
#endif
lab_call_u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P++;
    *LOCAL_TOP-- = YC(*P); P += 2;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_u_ot:  /* l,y,i */
#endif
lab_call_u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P++;
    *LOCAL_TOP-- = YC(*P); P += 2;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_2u_d:  /* l,y,y,i */
#endif
lab_call_2u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_2u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_2u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_2u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_2u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_2u_ot];
        goto lab_call_2u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_2u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_2u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_2u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_2u_nondet;
    } else {
        FOLLOW(P-1) = call_2u_ot;
        goto lab_call_2u_ot;
    }
#endif

#ifndef GCC
case call_2u_det:  /* l,y,y,i */
#endif
lab_call_2u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        LOCAL_TOP -= 2; P += 4;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_2u_nondet:  /* l,y,y,i */
#endif
lab_call_2u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    LOCAL_TOP -= 2; P += 4;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case allocate_nondet:  /* i,i,s,i */
#endif
lab_allocate_nondet:
    AR_BTM(AR) = ADDTAG((AR+Operand0), NONDET_FRAME_TAG);
    LOCAL_TOP = AR - Operand(1);
    P += 4;
    AR_TOP(AR) = (BPLONG)LOCAL_TOP;

    INITIALIZE_STACK_VARS(NONDET_FRAME_SIZE);
rr_allocate_nondet:
    INVOKE_GC_NONDET;
    CATCH_WAKE_EVENT;
    AR_B(AR) = (BPLONG)B;
    AR_H(AR) = (BPLONG)H;
    AR_T(AR) = (BPLONG)T;
    AR_SF(AR) = (BPLONG)SF;
    B = AR;
    HB = H;
    P += 2;  /* skip nondet */
    CONTCASE;

#ifndef GCC
case call_2u_ot:  /* l,y,y,i */
#endif
lab_call_2u_ot:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        LOCAL_TOP -= 2; P += 4;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        CONTCASE;
}

#ifndef GCC
case call_3u_d:  /* l,y,y,y,i */
#endif
lab_call_3u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_3u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_3u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_3u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_3u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_3u_ot];
        goto lab_call_3u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_3u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_3u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_3u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_3u_nondet;
    } else {
        FOLLOW(P-1) = call_3u_ot;
        goto lab_call_3u_ot;
    }
#endif


#ifndef GCC
case call_3u_det:  /* l,y,y,y,i */
#endif
lab_call_3u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_3u_nondet:  /* l,y,y,y,i */
#endif
lab_call_3u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_3u_ot:  /* l,y,y,y,i */
#endif
lab_call_3u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_4u_d:  /* l,y,y,y,y,i */
#endif
lab_call_4u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_4u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_4u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_4u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_4u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_4u_ot];
        goto lab_call_4u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_4u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_4u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_4u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_4u_nondet;
    } else {
        FOLLOW(P-1) = call_4u_ot;
        goto lab_call_4u_ot;
    }
#endif

#ifndef GCC
case call_4u_det:  /* l,y,y,y,y,i */
#endif
lab_call_4u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        LOCAL_TOP -= 4; P += 6;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_4u_nondet:  /* l,y,y,y,y,i */
#endif
lab_call_4u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_4u_ot:  /* l,y,y,y,y,i */
#endif
lab_call_4u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_5u_d:  /* l,y,y,y,y,y,i */
#endif
lab_call_5u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_5u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_5u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_5u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_5u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_5u_ot];
        goto lab_call_5u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_5u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_5u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_5u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_5u_nondet;
    } else {
        FOLLOW(P-1) = call_5u_ot;
        goto lab_call_5u_ot;
    }
#endif

#ifndef GCC
case call_5u_det:  /* l,y,y,y,y,y,i */
#endif
lab_call_5u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        LOCAL_TOP -= 5; P += 7;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_5u_nondet:  /* l,y,y,y,y,y,i */
#endif
lab_call_5u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_5u_ot:  /* l,y,y,y,y,y,i */
#endif
lab_call_5u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_6u_d:  /* l,y,y,y,y,y,y,i */
#endif
lab_call_6u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_6u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_6u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_6u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_6u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_6u_ot];
        goto lab_call_6u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_6u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_6u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_6u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_6u_nondet;
    } else {
        FOLLOW(P-1) = call_6u_ot;
        goto lab_call_6u_ot;
    }
#endif

#ifndef GCC
case call_6u_det:  /* l,y,y,y,y,y,y,i */
#endif
lab_call_6u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        *(LOCAL_TOP-5) = YC(*(P+6));
        LOCAL_TOP -= 6; P += 8;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_6u_nondet:  /* l,y,y,y,y,y,y,i */
#endif
lab_call_6u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    LOCAL_TOP -= 6; P += 8;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_6u_ot:  /* l,y,y,y,y,y,y,i */
#endif
lab_call_6u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    LOCAL_TOP -= 6; P += 8;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_7u_d:  /* l,y,y,y,y,y,y,y,i */
#endif
lab_call_7u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_7u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_7u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_7u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_7u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_7u_ot];
        goto lab_call_7u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_7u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_7u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_7u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_7u_nondet;
    } else {
        FOLLOW(P-1) = call_7u_ot;
        goto lab_call_7u_ot;
    }
#endif

#ifndef GCC
case call_7u_det:  /* l,y,y,y,y,y,y,y,i */
#endif
lab_call_7u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        *(LOCAL_TOP-5) = YC(*(P+6));
        *(LOCAL_TOP-6) = YC(*(P+7));
        LOCAL_TOP -= 7; P += 9;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_7u_nondet:  /* l,y,y,y,y,y,y,y,i */
#endif
lab_call_7u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    LOCAL_TOP -= 7; P += 9;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_7u_ot:  /* l,y,y,y,y,y,y,y,i */
#endif
lab_call_7u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    LOCAL_TOP -= 7; P += 9;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_8u_d:  /* l,y,y,y,y,y,y,y,y,i */
#endif
lab_call_8u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_8u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_8u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_8u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_8u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_8u_ot];
        goto lab_call_8u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_8u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_8u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_8u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_8u_nondet;
    } else {
        FOLLOW(P-1) = call_8u_ot;
        goto lab_call_8u_ot;
    }
#endif

#ifndef GCC
case call_8u_det:  /* l,y,y,y,y,y,y,y,y,i */
#endif
lab_call_8u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        *(LOCAL_TOP-5) = YC(*(P+6));
        *(LOCAL_TOP-6) = YC(*(P+7));
        *(LOCAL_TOP-7) = YC(*(P+8));
        LOCAL_TOP -= 8; P += 10;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_8u_nondet:  /* l,y,y,y,y,y,y,y,y,i */
#endif
lab_call_8u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    *(LOCAL_TOP-7) = YC(*(P+8));
    LOCAL_TOP -= 8; P += 10;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_8u_ot:  /* l,y,y,y,y,y,y,y,y,i */
#endif
lab_call_8u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    *(LOCAL_TOP-7) = YC(*(P+8));
    LOCAL_TOP -= 8; P += 10;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_9u_d:  /* l,y,y,y,y,y,y,y,y,y,i */
#endif
lab_call_9u_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_9u_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_9u_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_9u_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_9u_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_9u_ot];
        goto lab_call_9u_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_9u_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_9u_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_9u_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_9u_nondet;
    } else {
        FOLLOW(P-1) = call_9u_ot;
        goto lab_call_9u_ot;
    }
#endif

#ifndef GCC
case call_9u_det:  /* l,y,y,y,y,y,y,y,y,y,i */
#endif
lab_call_9u_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = YC(*(P+1));
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        *(LOCAL_TOP-5) = YC(*(P+6));
        *(LOCAL_TOP-6) = YC(*(P+7));
        *(LOCAL_TOP-7) = YC(*(P+8));
        *(LOCAL_TOP-8) = YC(*(P+9));
        LOCAL_TOP -= 9; P += 11;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_9u_nondet:  /* l,y,y,y,y,y,y,y,y,y,i */
#endif
lab_call_9u_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    *(LOCAL_TOP-7) = YC(*(P+8));
    *(LOCAL_TOP-8) = YC(*(P+9));
    LOCAL_TOP -= 9; P += 11;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_9u_ot:  /* l,y,y,y,y,y,y,y,y,y,i */
#endif
lab_call_9u_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = YC(*(P+1));
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    *(LOCAL_TOP-5) = YC(*(P+6));
    *(LOCAL_TOP-6) = YC(*(P+7));
    *(LOCAL_TOP-7) = YC(*(P+8));
    *(LOCAL_TOP-8) = YC(*(P+9));
    LOCAL_TOP -= 9; P += 11;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_cu_d:  /* l,c,y,i */
#endif
lab_call_cu_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_cu_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cu_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_cu_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cu_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_cu_ot];
        goto lab_call_cu_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_cu_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cu_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_cu_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cu_nondet;
    } else {
        FOLLOW(P-1) = call_cu_ot;
        goto lab_call_cu_ot;
    }
#endif

#ifndef GCC
case call_cu_det:  /* l,c,y,i */
#endif
lab_call_cu_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = *(P+1);
        *(LOCAL_TOP-1) = YC(*(P+2));
        LOCAL_TOP -= 2; P += 4;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_cu_nondet:  /* l,c,y,i */
#endif
lab_call_cu_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    LOCAL_TOP -= 2; P += 4;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_cu_ot:  /* l,c,y,i */
#endif
lab_call_cu_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    LOCAL_TOP -= 2; P += 4;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_cuu_d:  /* l,c,y,y,i */
#endif
lab_call_cuu_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_cuu_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuu_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_cuu_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuu_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_cuu_ot];
        goto lab_call_cuu_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_cuu_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuu_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_cuu_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuu_nondet;
    } else {
        FOLLOW(P-1) = call_cuu_ot;
        goto lab_call_cuu_ot;
    }
#endif


#ifndef GCC
case call_cuu_det:  /* l,c,y,y,i */
#endif
lab_call_cuu_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = *(P+1);
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        LOCAL_TOP -= 3; P += 5;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_cuu_nondet:  /* l,c,y,y,i */
#endif
lab_call_cuu_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_cuu_ot:  /* l,c,y,y,i */
#endif
lab_call_cuu_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    LOCAL_TOP -= 3; P += 5;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_cuuu_d:  /* l,c,y,y,y,i */
#endif
lab_call_cuuu_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_cuuu_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuu_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_cuuu_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuu_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_cuuu_ot];
        goto lab_call_cuuu_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_cuuu_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuu_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_cuuu_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuu_nondet;
    } else {
        FOLLOW(P-1) = call_cuuu_ot;
        goto lab_call_cuuu_ot;
    }
#endif

#ifndef GCC
case call_cuuu_det:  /* l,c,y,y,y,i */
#endif
lab_call_cuuu_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = *(P+1);
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        LOCAL_TOP -= 4; P += 6;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_cuuu_nondet:  /* l,c,y,y,y,i */
#endif
lab_call_cuuu_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_cuuu_ot:  /* l,c,y,y,y,i */
#endif
lab_call_cuuu_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    LOCAL_TOP -= 4; P += 6;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_cuuuu_d:  /* l,c,y,y,y,y,i */
#endif
lab_call_cuuuu_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[call_cuuuu_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuuu_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[call_cuuuu_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuuu_nondet;
    } else {
        *(void **)(P-1) = jmp_table[call_cuuuu_ot];
        goto lab_call_cuuuu_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = call_cuuuu_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuuu_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = call_cuuuu_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_call_cuuuu_nondet;
    } else {
        FOLLOW(P-1) = call_cuuuu_ot;
        goto lab_call_cuuuu_ot;
    }
#endif

#ifndef GCC
case call_cuuuu_det:  /* l,c,y,y,y,y,i */
#endif
lab_call_cuuuu_det:
{BPLONG_PTR ep;
        ep = (BPLONG_PTR)*P;
        *(LOCAL_TOP) = *(P+1);
        *(LOCAL_TOP-1) = YC(*(P+2));
        *(LOCAL_TOP-2) = YC(*(P+3));
        *(LOCAL_TOP-3) = YC(*(P+4));
        *(LOCAL_TOP-4) = YC(*(P+5));
        LOCAL_TOP -= 5; P += 7;

        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)P;
        P = ep;
        goto lab_allocate_det;
}

#ifndef GCC
case call_cuuuu_nondet:  /* l,c,y,y,y,y,i */
#endif
lab_call_cuuuu_nondet:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;
}

#ifndef GCC
case call_cuuuu_ot:  /* l,c,y,y,y,y,i */
#endif
lab_call_cuuuu_ot:
{BPLONG_PTR ep;
    ep = (BPLONG_PTR)*P;
    *(LOCAL_TOP) = *(P+1);
    *(LOCAL_TOP-1) = YC(*(P+2));
    *(LOCAL_TOP-2) = YC(*(P+3));
    *(LOCAL_TOP-3) = YC(*(P+4));
    *(LOCAL_TOP-4) = YC(*(P+5));
    LOCAL_TOP -= 5; P += 7;

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;
}

#ifndef GCC
case call_var:  /* i,zs(-1),i */
#endif
lab_call_var:
    arity = NextOperandLiteral;
    op1 = *P++; OP_ZC_DEREF(op1);
    if (ISSTRUCT(op1)) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        sym_ptr = (SYM_REC_PTR)FOLLOW(top);
        head_arity = GET_ARITY(sym_ptr);
        for (i = 1; i <= head_arity; i++) {
            *LOCAL_TOP-- = FOLLOW(top+i);
        }
    } else if (ISATOM(op1)) {
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(op1);
        head_arity = 0;
    } else if (ISLIST(op1)) {
        sym_ptr = list_psc;
        top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        *LOCAL_TOP-- = FOLLOW(top);
        *LOCAL_TOP-- = FOLLOW(top+1);
        head_arity = 2;
    } else {
        if (ISREF(op1)) {
            RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "call", 1);
        } else {
            RAISE_ISO_EXCEPTION(c_type_error(et_CALLABLE, op1), "call", 1);
        }
    }

    head_arity += arity-1;
    while (arity > 1) {
        op1 = *P++;
        PASS_ARG_Z(op1);
        arity--;
    }

    P++;  /* skip MaxS for GC */
    sym_ptr = (SYM_REC_PTR)insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), head_arity);
    goto call_sub;


#ifndef GCC
case call_var0:  /* y,i */
#endif
lab_call_var0:
    op1 = NextOperandYC; DEREF(op1); P++;  /* skip MaxS for GC */
    if (ISSTRUCT(op1)) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        sym_ptr = (SYM_REC_PTR)FOLLOW(top);
        head_arity = GET_ARITY(sym_ptr);
        for (i = 1; i <= head_arity; i++) {
            *LOCAL_TOP-- = FOLLOW(top+i);
        }
        goto call_sub;
    } else if (ISATOM(op1)) {
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(op1);
        goto call_sub;
    } else if (ISLIST(op1)) {
        sym_ptr = list_psc;
        top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        *LOCAL_TOP-- = FOLLOW(top);
        *LOCAL_TOP-- = FOLLOW(top+1);
        goto call_sub;
    } else {
        if (ISREF(op1)) {
            RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "call", 1);
        } else {
            RAISE_ISO_EXCEPTION(c_type_error(et_CALLABLE, op1), "call", 1);
        }
    }

#ifndef GCC
case jmp:  /* l */
#endif
lab_jmp:
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_eq_constant:  /* y,c,l,l */
#endif
lab_jmpn_eq_constant:
    op1 = NextOperandYC;
    op2 = NextOperandConstant;
    SWITCH_OP_ATM(op1, rr_jmpn_eq_constant,
                  {P = (BPLONG_PTR)*(P+1); CONTCASE;},
                  {if (op1 == op2) {P += 2; CONTCASE;}},
                  {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_nil:  /* y,l,l */
#endif
lab_jmpn_nil:
    op1 = NextOperandYC;
    SWITCH_OP_NIL(op1, rr_jmpn_nil,
                  {P = (BPLONG_PTR)*(P+1); CONTCASE;},
                  {P += 2; CONTCASE;},
                  {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_eq_struct:  /* y,s,l,l */
#endif
lab_jmpn_eq_struct:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_jmpn_eq_struct1,
                     {P = (BPLONG_PTR)*(P+1); CONTCASE;},
                     {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                         if ((SYM_REC_PTR)FOLLOW(sreg) == sym_ptr) {
                             P += 2;
                             sreg++;
                             CONTCASE;
                         }},
                     {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_eq_struct_fetch_v:  /* y,s,l,l,y */
#endif
lab_jmpn_eq_struct_fetch_v:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_jmpn_eq_struct_fetch_v1,
                     {P = (BPLONG_PTR)*(P+1); CONTCASE;},
                     {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                         if ((SYM_REC_PTR)FOLLOW(sreg) == sym_ptr) {
                             YC(*(P+2)) = FOLLOW(sreg+1);
                             P += 3;
                             sreg += 2;
                             CONTCASE;
                         }},
                     {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case switch_cons:  /* y,l,l,l,y,y */
#endif
lab_switch_cons:
    op1 = NextOperandYC;
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (!ISLIST(op1)) goto rr_switch_cons_nolst;
    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    op1 = *(P+3); if (op1 != 0) YC(op1) = FOLLOW(sreg);
    op1 = *(P+4); if (op1 != 0) YC(op1) = FOLLOW(sreg+1);
    P += 5;
    CONTCASE;

rr_switch_cons_nolst:
    if (ISNIL(op1)) {P = (BPLONG_PTR)*P; CONTCASE;}
    if (IS_SUSP_VAR(op1)) {P = (BPLONG_PTR)*(P+1); CONTCASE;}
    P = (BPLONG_PTR)*(P+2);
    CONTCASE;

#ifndef GCC
case switch_cons_car:  /* y,l,l,l,y */
#endif
lab_switch_cons_car:
    op2 = (BPLONG)NextOperandY;
    op1 = FOLLOW(op2);
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (!ISLIST(op1)) goto rr_switch_cons_nolst;
    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    op1 = *(P+3); if (op1 != 0) YC(op1) = FOLLOW(sreg);
    FOLLOW(op2) = FOLLOW(sreg+1);
    P += 4;
    CONTCASE;


#ifndef GCC
case switch_cons_910:  /* y,l,l,l */
#endif
lab_switch_cons_910:
    op1 = NextOperandYC;
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (!ISLIST(op1)) goto rr_switch_cons_nolst;
    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    YC(-9) = FOLLOW(sreg); YC(-10) = FOLLOW(sreg+1);
    P += 3;
    CONTCASE;

#ifndef GCC
case switch_cons_vv:  /* y,l,l,l,y,y */
#endif
lab_switch_cons_vv:
    op1 = NextOperandYC;
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (!ISLIST(op1)) goto rr_switch_cons_nolst;
    sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
    YC(*(P+3)) = FOLLOW(sreg);
    YC(*(P+4)) = FOLLOW(sreg+1);
    P += 5;
    CONTCASE;

#ifndef GCC
case jmpn_eql_uu:  /* y,y,l */
#endif
lab_jmpn_eql_uu:
    op1 = YC(*P++);
    op2 = YC(*P++);
    SWITCH_OP_INT(op1, rr_jmpn_eql_uu1, {},
                  {SWITCH_OP_INT(op2, rr_jmpn_eql_uu2, {},
                                 {if (op1 == op2) {
                                         P++;
                                         CONTCASE;
                                     } else {
                                         P = (BPLONG_PTR)*P;
                                         CONTCASE;
                                     }},
                                 {});},
                  {});

rr_jmpn_eql_notint:
    SAVE_TOP;
    op1 = equal_to(op1, op2);
    if (op1 == 1) {
        P++;
        CONTCASE;
    } else if (op1 == 0) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, "=:=", 2);
    }


#ifndef GCC
case jmpn_eql_uc:  /* y,c,l */
#endif
lab_jmpn_eql_uc:
    op1 = YC(*P++);
    op2 = *P++;
    SWITCH_OP_INT(op1, rr_jmpn_eql_uc1, {},
                  {if (op1 == op2) {
                          P++;
                          CONTCASE;
                      } else {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      }},
                  {});
    goto rr_jmpn_eql_notint;

#ifndef GCC
case jmp_eql_uu:  /* y,y,l */
#endif
lab_jmp_eql_uu:
    op1 = *(P+2);
    op1 = FOLLOW(op1);  /* the label */
#ifdef GCC
    if ((void *)op1 == jmp_table[fail]) {
        *(void **)(P-1) = jmp_table[jmp_eql_uu_fail];
        goto lab_jmp_eql_uu_fail;
    } else {
        *(void **)(P-1) = jmp_table[jmp_eql_uu_ot];
        goto lab_jmp_eql_uu_ot;
    }
#else
    if (op1 == fail) {
        FOLLOW(P-1) = jmp_eql_uu_fail;
        goto lab_jmp_eql_uu_fail;
    } else {
        FOLLOW(P-1) = jmp_eql_uu_ot;
        goto lab_jmp_eql_uu_ot;
    }
#endif

#ifndef GCC
case jmp_eql_uu_fail:  /* y,y,l */
#endif
lab_jmp_eql_uu_fail:
    op1 = YC(*P++);
    op2 = YC(*P++);
    SWITCH_OP_INT(op1, rr_jmp_eql_uu_fail1, {},
                  {SWITCH_OP_INT(op2, rr_jmp_eql_uu_fail2, {},
                                 {if (op1 == op2) {
                                         goto lab_fail;
                                     } else {
                                         P++;
                                         CONTCASE;
                                     }},
                                 {})},
                  {});

    goto rr_jmp_eql_notint;

#ifndef GCC
case jmp_eql_uu_ot:  /* y,y,l */
#endif
lab_jmp_eql_uu_ot:
    op1 = YC(*P++);
    op2 = YC(*P++);
    SWITCH_OP_INT(op1, rr_jmp_eql_uu1, {},
                  {SWITCH_OP_INT(op2, rr_jmp_eql_uu2, {},
                                 {if (op1 == op2) {
                                         P = (BPLONG_PTR)*P;
                                         CONTCASE;
                                     } else {
                                         P++;
                                         CONTCASE;
                                     }},
                                 {})},
                  {});

rr_jmp_eql_notint:
    SAVE_TOP;
    op1 = equal_to(op1, op2);
    if (op1 == 1) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else if (op1 == 0) {
        P++;
        CONTCASE;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, "=\\=", 2);
    }

#ifndef GCC
case jmp_eql_uc:  /* y,c,l */
#endif
lab_jmp_eql_uc:
    op1 = YC(*P++);
    op2 = *P++;
    SWITCH_OP_INT(op1, rr_jmp_eql_uc, {},
                  {if (op1 == op2) {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      } else {
                          P++;
                          CONTCASE;
                      }},
                  {});
    goto rr_jmp_eql_notint;

#ifndef GCC
case jmpn_gt_uu:  /* y,y,l */
#endif
lab_jmpn_gt_uu:
    op1 = YC(*P++);
    op2 = YC(*P++);
    SWITCH_OP_INT(op1, rr_jmpn_gt_uu1,
                  {},
                  {SWITCH_OP_INT(op2, rr_jmpn_gt_uu2,
                                 {},
                                 {if (INTVAL(op1) > INTVAL(op2)) {
                                         P++;
                                         CONTCASE;
                                     } else {
                                         P = (BPLONG_PTR)*P;
                                         CONTCASE;
                                     }},
                                 {})},
                  {});

rr_jmpn_gt_notint:
    SAVE_TOP;
    op1 = greater_than(op1, op2);
    if (op1 == 1) {
        P++;
        CONTCASE;
    } else if (op1 == 0) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, ">", 2);
    }

#ifndef GCC
case jmpn_gt_ui:  /* y,i,l */
#endif
lab_jmpn_gt_ui:
    op1 = YC(*P++);
    op2 = *P++;
    SWITCH_OP_INT(op1, rr_jmpn_gt_ui,
                  {},
                  {if (INTVAL(op1) > op2) {
                          P++;
                          CONTCASE;
                      } else {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      }},
                  {});
    op2 = MAKEINT(op2);
    goto rr_jmpn_gt_notint;

#ifndef GCC
case jmpn_gt_iu:  /* i,y,l */
#endif
lab_jmpn_gt_iu:
    op1 = *P++;
    op2 = YC(*P++);
    SWITCH_OP_INT(op2, rr_jmpn_gt_iu,
                  {},
                  {if (op1 > INTVAL(op2)) {
                          P++;
                          CONTCASE;
                      } else {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      }},
                  {});
    op1 = MAKEINT(op1);
    goto rr_jmpn_gt_notint;


#ifndef GCC
case jmpn_ge_uu:  /* y,y,l */
#endif
lab_jmpn_ge_uu:
    op1 = YC(*P++);
    op2 = YC(*P++);
    SWITCH_OP_INT(op1, rr_jmpn_ge_uu1, {},
                  {SWITCH_OP_INT(op2, rr_jmpn_ge_uu2, {},
                                 {if (INTVAL(op1) >= INTVAL(op2)) {
                                         P++;
                                         CONTCASE;
                                     } else {
                                         P = (BPLONG_PTR)*P;
                                         CONTCASE;
                                     }},
                                 {})},
                  {});

rr_jmpn_ge_notint:
    SAVE_TOP;
    op1 = greater_equal(op1, op2);
    if (op1 == 1) {
        P++;
        CONTCASE;
    } else if (op1 == 0) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, ">=", 2);
    }

#ifndef GCC
case jmpn_ge_ui:  /* y,i,l */
#endif
lab_jmpn_ge_ui:
    op1 = YC(*P++);
    op2 = *P++;
    SWITCH_OP_INT(op1, rr_jmpn_ge_ui,
                  {},
                  {if (INTVAL(op1) >= op2) {
                          P++;
                          CONTCASE;
                      } else {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      }},
                  {});
    op2 = MAKEINT(op2);
    goto rr_jmpn_ge_notint;

#ifndef GCC
case jmpn_ge_iu:  /* i,y,l */
#endif
lab_jmpn_ge_iu:
    op1 = *P++;
    op2 = YC(*P++); DEREF(op2);
    SWITCH_OP_INT(op2, rr_jmpn_ge_iu,
                  {},
                  {if (op1 >= INTVAL(op2)) {
                          P++;
                          CONTCASE;
                      } else {
                          P = (BPLONG_PTR)*P;
                          CONTCASE;
                      }},
                  {});
    op1 = MAKEINT(op1);
    goto rr_jmpn_ge_notint;

#ifndef GCC
case jmpn_id_uu:  /* y,y,l */
#endif
lab_jmpn_id_uu:
    op1 = YC(*P++); DEREF(op1);
    op2 = YC(*P++); DEREF(op2);
    if (op1 == op2 || (ISCOMPOUND(op1) && bp_identical(op1, op2))) {
        P++;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case jmpn_id_uc:  /* y,c,l */
#endif
lab_jmpn_id_uc:
    op1 = YC(*P++); DEREF(op1);
    op2 = *P++;
    if (op1 == op2) {
        P++;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case jmp_id_uu:  /* y,y,l */
#endif
lab_jmp_id_uu:
    op1 = YC(*P++); DEREF(op1);
    op2 = YC(*P++); DEREF(op2);
    if (op1 == op2 || (ISCOMPOUND(op1) && bp_identical(op1, op2))) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        P++;
        CONTCASE;
    }

#ifndef GCC
case jmp_id_uc:  /* y,c,l */
#endif
lab_jmp_id_uc:
    op1 = YC(*P++); DEREF(op1);
    op2 = *P++;
    if (op1 == op2) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        P++;
        CONTCASE;
    }

#ifndef GCC
case jmpn_var_y:  /* y,l */
#endif
lab_jmpn_var_y:
    op1 = *(P+1);
    op1 = FOLLOW(op1);  /* the label */
#ifdef GCC
    if ((void *)op1 == jmp_table[fail0]) {
        *(void **)(P-1) = jmp_table[jmpn_var_y_fail0];
        goto lab_jmpn_var_y_fail0;
    } else {
        *(void **)(P-1) = jmp_table[jmpn_var_y_ot];
        goto lab_jmpn_var_y_ot;
    }
#else
    if (op1 == fail0) {
        FOLLOW(P-1) = jmpn_var_y_fail0;
        goto lab_jmpn_var_y_fail0;
    } else {
        FOLLOW(P-1) = jmpn_var_y_ot;
        goto lab_jmpn_var_y_ot;
    }
#endif

#ifndef GCC
case jmpn_var_y_fail0:  /* y,l */
#endif
lab_jmpn_var_y_fail0:
    op1 = YC(*P);
    DEREF2(op1, {P += 2; CONTCASE;});
    if (IS_SUSP_VAR(op1)) {P += 2; CONTCASE;}
    goto lab_fail0;

#ifndef GCC
case jmpn_var_y_ot:  /* y,l */
#endif
lab_jmpn_var_y_ot:
    op1 = YC(*P);
    DEREF2(op1, {P += 2; CONTCASE;});
    if (IS_SUSP_VAR(op1)) {P += 2; CONTCASE;}
    P = (BPLONG_PTR)*(P+1);
    CONTCASE;

#ifndef GCC
case jmp_var_y:  /* y,l */
#endif
lab_jmp_var_y:
    op1 = YC(*P);
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (IS_SUSP_VAR(op1)) {P = (BPLONG_PTR)*(P+1); CONTCASE;}
    P += 2;
    CONTCASE;

#ifndef GCC
case jmpn_atom_y:  /* y,l */
#endif
lab_jmpn_atom_y:
    op1 = YC(*P);
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (ISATOM(op1)) {
        P += 2;
        CONTCASE;
    }
    P = (BPLONG_PTR)*(P+1);
    CONTCASE;

#ifndef GCC
case jmpn_atomic_y:  /* y,l */
#endif
lab_jmpn_atomic_y:
    op1 = *(P+1);
    op1 = FOLLOW(op1);  /* the label */
#ifdef GCC
    if ((void *)op1 == jmp_table[fail0]) {
        *(void **)(P-1) = jmp_table[jmpn_atomic_y_fail0];
        goto lab_jmpn_atomic_y_fail0;
    } else {
        *(void **)(P-1) = jmp_table[jmpn_atomic_y_ot];
        goto lab_jmpn_atomic_y_ot;
    }
#else
    if (op1 == fail0) {
        FOLLOW(P-1) = jmpn_atomic_y_fail0;
        goto lab_jmpn_atomic_y_fail0;
    } else {
        FOLLOW(P-1) = jmpn_atomic_y_ot;
        goto lab_jmpn_atomic_y_ot;
    }
#endif

#ifndef GCC
case jmpn_atomic_y_fail0:  /* y,l */
#endif
lab_jmpn_atomic_y_fail0:
    op1 = YC(*P);
    DEREF2(op1, {goto lab_fail0;});
    if (TAG(op1) == ATM || (ISSTRUCT(op1) && (IS_FLOAT_PSC(op1) || IS_BIGINT_PSC(op1)))) {
        P += 2;
        CONTCASE;
    }
    goto lab_fail0;

#ifndef GCC
case jmpn_atomic_y_ot:  /* y,l */
#endif
lab_jmpn_atomic_y_ot:
    op1 = YC(*P);
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (TAG(op1) == ATM || (ISSTRUCT(op1) && (IS_FLOAT_PSC(op1) || IS_BIGINT_PSC(op1)))) {
        P += 2;
        CONTCASE;
    }
    P = (BPLONG_PTR)*(P+1);
    CONTCASE;

#ifndef GCC
case jmpn_num_y:  /* y,l */
#endif
lab_jmpn_num_y:
    op1 = NextOperandYC;
    DEREF(op1);
    if (ISNUM(op1)) {
        P++;
        CONTCASE;
    }
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_float_y:  /* y,l */
#endif
lab_jmpn_float_y:
    op1 = NextOperandYC;
    DEREF(op1);
    if (ISFLOAT(op1)) {
        P++;
        CONTCASE;
    }
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case jmpn_int_y:  /* y,l */
#endif
lab_jmpn_int_y:
    op1 = YC(*P);
    DEREF2(op1, {P = (BPLONG_PTR)*(P+1); CONTCASE;});
    if (ISINT(op1) || IS_BIGINT(op1)) {
        P += 2;
        CONTCASE;
    }
    P = (BPLONG_PTR)*(P+1);
    CONTCASE;

#ifndef GCC
case hash:  /* y,i,i,l */
#endif
lab_hash:
    op1 = *P; op1 = YC(op1);
    SWITCH_OP(op1, nhash,
              {P = (BPLONG_PTR)*(P+3); CONTCASE;},
              {sym_ptr = NULL; P = (BPLONG_PTR)*(BPLONG_PTR *)(IHASH(op1, *(P+1))*sizeof(BPLONG) + *(P+2)); CONTCASE;},
              {sym_ptr = NULL; P = (BPLONG_PTR)*(BPLONG_PTR *)(IHASH((BPLONG)list_psc, *(P+1))*sizeof(BPLONG) + *(P+2));
                  sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                  CONTCASE;},
              {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                  sym_ptr = (SYM_REC_PTR) FOLLOW(sreg);
                  P = (BPLONG_PTR)*(BPLONG_PTR *)(IHASH((BPLONG)sym_ptr, *(P+1))*sizeof(BPLONG) + *(P+2));
                  sreg++;
                  CONTCASE;},
              {P = (BPLONG_PTR)*(P+3); CONTCASE;});

#ifndef GCC
case hash_jmpn_constant:  /* c,l */
#endif
lab_hash_jmpn_constant:
    op2 = NextOperandLiteral;
    if (op1 == op2) {
        P++;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case hash_branch_constant:  /* c,l,l */
#endif
lab_hash_branch_constant:
    /* c,l_neq,l_eq */
    op2 = Operand0;
    if (op1 == op2) {
        P = (BPLONG_PTR)*(P+2);
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*(P+1);
        CONTCASE;
    }

#ifndef GCC
case hash_jmpn_nil:  /* l */
#endif
lab_hash_jmpn_nil:
    if (ISNIL(op1)) {
        P++;
        CONTCASE;
    }
    else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case hash_jmpn_struct:  /* s,l */
#endif
lab_hash_jmpn_struct:
    op2 = NextOperandSym;
    if ((BPLONG)sym_ptr == op2) {  /* sreg and sym_ptr got value at hash */
        P++;
        CONTCASE;
    }
    P = (BPLONG_PTR)*P;
    CONTCASE;

#ifndef GCC
case hash_branch_struct:  /* s,l */
#endif
lab_hash_branch_struct:
    /* s, l_neq, l_eq */
    op2 = Operand0;
    if ((BPLONG)sym_ptr == op2) {  /* sym_ptr got value at hash */
        P = (BPLONG_PTR)*(P+2);  /* hash_jmpn_struct */
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*(P+1);
        CONTCASE;
    }

#ifndef GCC
case hash_jmpn_list0:  /* l */
#endif
lab_hash_jmpn_list0:
    if (ISLIST(op1)) {
        P++;
        CONTCASE;
    }
    P = (BPLONG_PTR)*P;
    CONTCASE;


#ifndef GCC
case hash_jmpn_list:  /* l,y,y */
#endif
lab_hash_jmpn_list:
    if (ISLIST(op1)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op1 = *(P+1); if (op1 != 0) YC(op1) = FOLLOW(sreg);
        op1 = *(P+2); if (op1 != 0) YC(op1) = FOLLOW(sreg+1);
        P += 3;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case para_uuuv:  /* y,y,y,y */
#endif
lab_para_uuuv:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    *(LOCAL_TOP-2) = YC(*(P+2));
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 4;
    CONTCASE;

#ifndef GCC
case para_uuv:  /* y,y,y */
#endif
lab_para_uuv:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 3;
    CONTCASE;

#ifndef GCC
case para_uv:  /* y,y */
#endif
lab_para_uv:
    *LOCAL_TOP = YC(*P);
    op1 = (BPLONG)Y(*(P+1)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 2; P += 2;
    CONTCASE;

#ifndef GCC
case para_uuu:  /* y,y,y */
#endif
lab_para_uuu:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    *(LOCAL_TOP-2) = YC(*(P+2));
    LOCAL_TOP -= 3; P += 3;
    CONTCASE;

#ifndef GCC
case para_uuuw:  /* y,y,y */
#endif
lab_para_uuuw:
    *LOCAL_TOP-- = NextOperandYC;

#ifndef GCC
case para_uuw:  /* y,y */
#endif
lab_para_uuw:
    *LOCAL_TOP-- = NextOperandYC;

#ifndef GCC
case para_uw:  /* y */
#endif
lab_para_uw:
    *LOCAL_TOP-- = NextOperandYC;
    *LOCAL_TOP = (BPLONG)LOCAL_TOP; LOCAL_TOP--;
    CONTCASE;

#ifndef GCC
case para_vv:  /* y,y */
#endif
lab_para_vv:
    op1 = (BPLONG)Y(*P); *LOCAL_TOP = FOLLOW(op1) = op1;
    op1 = (BPLONG)Y(*(P+1)); *(LOCAL_TOP-1) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 2; P += 2;
    CONTCASE;

#ifndef GCC
case para_cuv:  /* c,y,y */
#endif
lab_para_cuv:
    *LOCAL_TOP = *P;
    *(LOCAL_TOP-1) = YC(*(P+1));
    op1 = (BPLONG)Y(*(P+2)); *(LOCAL_TOP-2) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 3; P += 3;
    CONTCASE;

#ifndef GCC
case para_cuuv:  /* c,y,y,y */
#endif
lab_para_cuuv:
    *LOCAL_TOP = *P;
    *(LOCAL_TOP-1) = YC(*(P+1));
    *(LOCAL_TOP-2) = YC(*(P+2));
    op1 = (BPLONG)Y(*(P+3)); *(LOCAL_TOP-3) = FOLLOW(op1) = op1;
    LOCAL_TOP -= 4; P += 4;
    CONTCASE;

#ifndef GCC
case para_uuuc:  /* y,y,y,c */
#endif
lab_para_uuuc:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    *(LOCAL_TOP-2) = YC(*(P+2));
    *(LOCAL_TOP-3) = *(P+3);
    LOCAL_TOP -= 4; P += 4;
    CONTCASE;

#ifndef GCC
case para_uuc:  /* y,y,c */
#endif
lab_para_uuc:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    *(LOCAL_TOP-2) = *(P+2);
    LOCAL_TOP -= 3; P += 3;
    CONTCASE;

#ifndef GCC
case para_uu:  /* y,y */
#endif
lab_para_uu:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = YC(*(P+1));
    LOCAL_TOP -= 2; P += 2;
    CONTCASE;

#ifndef GCC
case para_uc:  /* y,c */
#endif
lab_para_uc:
    *LOCAL_TOP = YC(*P);
    *(LOCAL_TOP-1) = *(P+1);
    LOCAL_TOP -= 2; P += 2;
    CONTCASE;

#ifndef GCC
case para_u:  /* y */
#endif
lab_para_u:
    *LOCAL_TOP-- = NextOperandYC;
    CONTCASE;

#ifndef GCC
case para_v:  /* y */
#endif
lab_para_v:
    op1 = (BPLONG)NextOperandY; *LOCAL_TOP-- = FOLLOW(op1) = op1;
    CONTCASE;


#ifndef GCC
case para_w:  /* E */
#endif
lab_para_w:
    *LOCAL_TOP = (BPLONG)LOCAL_TOP; LOCAL_TOP--;
    CONTCASE;

#ifndef GCC
case para_c:  /* c */
#endif
lab_para_c:
    *LOCAL_TOP-- = *P++;
    CONTCASE;

#ifndef GCC
case para_nil:  /* E */
#endif
lab_para_nil:
    *LOCAL_TOP-- = nil_sym;
    CONTCASE;

#ifndef GCC
case fetch_4v:  /* y,y,y,y */
#endif
lab_fetch_4v:
    YC(*P) = FOLLOW(sreg);
    YC(*(P+1)) = FOLLOW(sreg+1);
    YC(*(P+2)) = FOLLOW(sreg+2);
    YC(*(P+3)) = FOLLOW(sreg+3);
    P += 4; sreg += 4;
    CONTCASE;

#ifndef GCC
case fetch_3v:  /* y,y,y */
#endif
lab_fetch_3v:
    YC(*P) = FOLLOW(sreg);
    YC(*(P+1)) = FOLLOW(sreg+1);
    YC(*(P+2)) = FOLLOW(sreg+2);
    P += 3; sreg += 3;
    CONTCASE;


#ifndef GCC
case fetch_2v:  /* y,y */
#endif
lab_fetch_2v:
    YC(*P) = FOLLOW(sreg);
    YC(*(P+1)) = FOLLOW(sreg+1);
    P += 2; sreg += 2;
    CONTCASE;

#ifndef GCC
case fetch_v:  /* y */
#endif
lab_fetch_v:
    YC(*P++) = FOLLOW(sreg++);
    CONTCASE;

#ifndef GCC
case fetch_vw:  /* y */
#endif
lab_fetch_vw:
    YC(*P++) = FOLLOW(sreg); sreg += 2;
    CONTCASE;

#ifndef GCC
case fetch_wv:  /* y */
#endif
lab_fetch_wv:
    YC(*P++) = FOLLOW(sreg+1); sreg += 2;
    CONTCASE;

#ifndef GCC
case fetch_w:  /* E */
#endif
lab_fetch_w:
    sreg++;
    CONTCASE;

#ifndef GCC
case fetch_2w:  /* E */
#endif
lab_fetch_2w:
    sreg += 2;
    CONTCASE;

#ifndef GCC
case fetch_3w:  /* E */
#endif
lab_fetch_3w:
    sreg += 3;
    CONTCASE;

#ifndef GCC
case fetch_4w:  /* E */
#endif
lab_fetch_4w:
    sreg += 4;
    CONTCASE;

#ifndef GCC
case fetch_ws:  /* i */
#endif
lab_fetch_ws:
    sreg += NextOperandLiteral;
    CONTCASE;

#ifndef GCC
case fetch_910:  /* E */
#endif
lab_fetch_910:
    YC(-9) = FOLLOW(sreg++);
    YC(-10) = FOLLOW(sreg++);
    CONTCASE;

#ifndef GCC
case fetch_45:  /* E */
#endif
lab_fetch_45:
    YC(-4) = FOLLOW(sreg++);
    YC(-5) = FOLLOW(sreg++);
    CONTCASE;

#ifndef GCC
case unify_constant:  /* y,c */
#endif
lab_unify_constant:
    op1 = NextOperandYC;
    op2 = NextOperandLiteral;
    UNIFY_CONSTANT_12(op1, op2, rr_unify_constant, {PUSHTRAIL(op1);}, {}, {BACKTRACK;});

#ifndef GCC
case fork_unify_constant:  /* l,y,c */
#endif
lab_fork_unify_constant:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        op2 = NextOperandLiteral;
        UNIFY_CONSTANT_12(op1, op2, rr_fork_unify_constant, {SET_FORK; PUSHTRAIL(op1);}, {SET_FORK;}, {P = (BPLONG_PTR)tmp_op; CONTCASE;});
}

#ifndef GCC
case unify_nil:  /* y */
#endif
lab_unify_nil:
    op1 = NextOperandYC;
    UNIFY_NIL_Y(op1, rr_unify_nil_y, {PUSHTRAIL(op1);}, {}, {BACKTRACK;});


#ifndef GCC
case fork_unify_nil:  /* l,y */
#endif
lab_fork_unify_nil:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        UNIFY_NIL_Y(op1, rr_fork_unify_nil_y, {SET_FORK; PUSHTRAIL(op1);}, {SET_FORK;}, {P = (BPLONG_PTR)tmp_op; CONTCASE;});
}

#ifndef GCC
case unify_struct:  /* y,s */
#endif
lab_unify_struct:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             CONTCASE;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case fork_unify_struct:  /* l,y,s */
#endif
lab_fork_unify_struct:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        sym_ptr = (SYM_REC_PTR)NextOperandSym;
        SWITCH_OP_STRUCT(op1, rr_fork_unify_struct,
                         {SET_FORK;
                             PUSHTRAIL(op1);
                             FOLLOW(op1) = ADDTAG(H, STR);
                             FOLLOW(H++) = (BPLONG)sym_ptr;
                             sreg = NULL;
                             CONTCASE;},
                         {UNTAG_ADDR(op1);
                             if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                                 SET_FORK;
                                 sreg = (BPLONG_PTR) op1 + 1;
                                 CONTCASE;}},
                         {SET_FORK; op2 = ADDTAG(H, STR);
                             FOLLOW(H++) = (BPLONG)sym_ptr;
                             sreg = NULL;
                             goto bind_suspvar_value;});

        P = (BPLONG_PTR)tmp_op;
        CONTCASE;
}

#ifndef GCC
case unify_struct_cut:  /* y,s */
#endif
lab_unify_struct_cut:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;

    SWITCH_OP_STRUCT(op1, rr_unify_struct_cut,
                     {CUT0;
                         PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             CUT0;
                             CONTCASE;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BIND_SUSPVAR_VALUE_AUX(op1, op2);
                         CUT0;
                         CONTCASE;});

    BACKTRACK;


#ifndef GCC
case unify_struct_arg_uv0:  /* y,s,y,y */
#endif
lab_unify_struct_arg_uv0:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_uv0,
                     {rr_unify_struct_arg_uv0_var:
                         PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, rr_unify_struct_arg_uv0_1); H++;
                         YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                         CONTCASE;},
                     { UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                         rr_unify_struct_arg_uv0_str:
                             sreg = (BPLONG_PTR) op1 + 1;
                             YC(*(P+1)) = FOLLOW(sreg+1);
                             op1 = FOLLOW(sreg); op2 = YC(*P); P += 2; sreg += 2;
                             goto rrr_unify_arg_u;}},
                     {rr_unify_struct_arg_uv0_susp:
                         op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         op3 = YC(*P++); BUILD_ARG_U_VALUE(op3, rr_unify_struct_arg_uv0_2); H++;
                         YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_2v0:  /* y,s,y,y */
#endif
lab_unify_struct_arg_2v0:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_2v0,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_V0;
                         BUILD_V0;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             YC(*P++) = FOLLOW(sreg++);
                             YC(*P++) = FOLLOW(sreg++);
                             CONTCASE;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_V0;
                         BUILD_V0;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_v0:  /* y,s,y */
#endif
lab_unify_struct_arg_v0:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_v0,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_V0;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             YC(*P++) = FOLLOW(sreg++);
                             CONTCASE;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_V0;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_2u:  /* y,s,y,y */
#endif
lab_unify_struct_arg_2u:
    /* Y A2 A1: notice arguments in reversed order */
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_2u,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         goto rr_unify_arg_2u_write;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             P++;  /* skip A2 */
                             goto rr_unify_arg_u;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         op3 = YC(*(P+1)); BUILD_ARG_U_VALUE(op3, rrr_unify_struct_arg_2u); H++;
                         op3 = YC(*P); BUILD_ARG_U_VALUE(op3, rrrr_unify_struct_arg_2u); H++;
                         P += 4;  /* 2+sizeof(unify_arg_u) */
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_u:  /* y,s,y */
#endif
lab_unify_struct_arg_u:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_u,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         goto rr_unify_arg_u_write;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             goto rr_unify_arg_u;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         op3 = YC(*P++); BUILD_ARG_U_VALUE(op3, r_unify_struct_arg_u); H++;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_c:  /* y,s,c */
#endif
lab_unify_struct_arg_c:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_c,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         FOLLOW(H++) = *P++;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             goto rr_unify_arg_c;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         FOLLOW(H++) = *P++;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_struct_arg_wc:  /* y,s,c */
#endif
lab_unify_struct_arg_wc:
    op1 = NextOperandYC;
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    SWITCH_OP_STRUCT(op1, rr_unify_struct_arg_wc,
                     {PUSHTRAIL(op1);
                         FOLLOW(op1) = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_W;
                         FOLLOW(H++) = *P++;
                         CONTCASE;},
                     {UNTAG_ADDR(op1);
                         if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                             sreg = (BPLONG_PTR) op1 + 1;
                             sreg++;
                             goto rr_unify_arg_c;}},
                     {op2 = ADDTAG(H, STR);
                         FOLLOW(H++) = (BPLONG)sym_ptr;
                         sreg = NULL;
                         BUILD_W;
                         FOLLOW(H++) = *P++;
                         goto bind_suspvar_value;});

    BACKTRACK;

#ifndef GCC
case unify_arg_c:  /* c */
#endif
lab_unify_arg_c:
    if (sreg == NULL) {  /* write mode */
        FOLLOW(H++) = *P++;
        CONTCASE;
    } else {
    rr_unify_arg_c:
        op1 = FOLLOW(sreg++);
        op2 = *P++;
        SWITCH_OP_VAR(op1, rrr_unify_arg_c,
                      {FOLLOW(op1) = op2;
                          PUSHTRAIL_h(op1);},
                      {goto bind_suspvar_value;},
                      {if (op1 != op2) BACKTRACK;});
        CONTCASE;
    }


#ifndef GCC
case unify_arg_u:  /* y */
#endif
lab_unify_arg_u:
    if (sreg == NULL) {  /* build mode */
    rr_unify_arg_u_write:
        op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, r_unify_arg_u); H++;
        CONTCASE;
    } else {
    rr_unify_arg_u:
        op1 = FOLLOW(sreg++);
        op2 = YC(*P++);
        SWITCH_OP(op1, rrr_unify_arg_u,
                  {goto unify_nsv_d;},
                  {UNIFY_CONSTANT_21(op2, op1, rrrr_unify_arg_u, {PUSHTRAIL(op2);}, {}, {BACKTRACK;});},
                  {goto unify_lst_d;},
                  {goto unify_str_d;},
                  {goto unify_susp_d;});
        CONTCASE;
    }

#ifndef GCC
case unify_arg_read_u:  /* y */
#endif
lab_unify_arg_read_u:
    op1 = FOLLOW(sreg++);
    op2 = YC(*P++);
    SWITCH_OP(op1, rrr_unify_arg_read_u,
              {goto unify_nsv_d;},
              {UNIFY_CONSTANT_21(op2, op1, rrrr_unify_arg_u, {PUSHTRAIL(op2);}, {}, {BACKTRACK;});},
              {goto unify_lst_d;},
              {goto unify_str_d;},
              {goto unify_susp_d;});
    CONTCASE;


#ifndef GCC
case unify_arg_v:  /* y */
#endif
lab_unify_arg_v:
    if (sreg == NULL) {  /* build mode */
        op1 = *P++; BUILD_ARG_V(op1); H++;
        CONTCASE;
    } else {
        op1 = *P++; UNIFY_ARG_V(sreg, op1); sreg++;
        CONTCASE;
    }


#ifndef GCC
case unify_arg_v0:  /* y */
#endif
lab_unify_arg_v0:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        CONTCASE;
    }


#ifndef GCC
case unify_arg_w:  /* E */
#endif
lab_unify_arg_w:
    if (sreg == NULL) {  /* build mode */
        FOLLOW(H) = (BPLONG)H; H++;
        CONTCASE;
    } else {
        sreg++;
        CONTCASE;
    }


#ifndef GCC
case unify_arg_struct:  /* s */
#endif
lab_unify_arg_struct:
    sym_ptr = (SYM_REC_PTR)NextOperandSym;
    if (sreg == NULL) {
        H++;
        *(H-1) = ADDTAG(H, STR);
        *H++ = (BPLONG)sym_ptr;
        CONTCASE;
    } else {
        op1 = *sreg;
        SWITCH_OP_STRUCT(op1, rr_unify_arg_struct,
                         {FOLLOW(op1) = ADDTAG(H, STR);
                             PUSHTRAIL_h(op1);
                             *H++ = (BPLONG)sym_ptr;
                             sreg = NULL;
                             CONTCASE;},
                         {UNTAG_ADDR(op1);
                             if (FOLLOW(op1) == (BPLONG)sym_ptr) {
                                 sreg = (BPLONG_PTR)op1 + 1;
                                 CONTCASE;
                             }},
                         {op2 = ADDTAG(H, STR);
                             FOLLOW(H++) = (BPLONG)sym_ptr;
                             sreg = NULL;
                             goto bind_suspvar_value;});
        BACKTRACK;
    }

#ifndef GCC
case unify_arg_list:  /* i,zs(-1)+1 */
#endif
lab_unify_arg_list:
    if (sreg == NULL) {
        goto lab_build_arg_list;
    } else {
        op1 = *sreg;
        arity = NextOperandLiteral;
        goto rr_unify_list;
    }

#ifndef GCC
case unify_arg_2c:  /* c,c */
#endif
lab_unify_arg_2c:
    if (sreg == NULL) {  /* write mode */
        FOLLOW(H++) = *P++;
        FOLLOW(H++) = *P++;
        CONTCASE;
    } else {
        op1 = FOLLOW(sreg);
        op2 = *P++;
        UNIFY_ARG_CONST(op1, op2, rrr_unify_arg_2c);
        sreg++;
        goto rr_unify_arg_c;
    }



#ifndef GCC
case unify_arg_3c:  /* c,c,c */
#endif
lab_unify_arg_3c:
    if (sreg == NULL) {  /* write mode */
        FOLLOW(H++) = *P++;
        FOLLOW(H++) = *P++;
        FOLLOW(H++) = *P++;
        CONTCASE;
    } else {
        op1 = FOLLOW(sreg);
        op2 = *P++;
        UNIFY_ARG_CONST(op1, op2, rrr_unify_arg_3c);
        sreg++;
        op1 = FOLLOW(sreg);
        op2 = *P++;
        UNIFY_ARG_CONST(op1, op2, rrrr_unify_arg_3c);
        sreg++;
        goto rr_unify_arg_c;
    }



#ifndef GCC
case unify_arg_v0u:  /* y,y */
#endif
lab_unify_arg_v0u:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        BUILD_U(r_unify_arg_vu);
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        goto rr_unify_arg_u;
    }


#ifndef GCC
case unify_arg_v0c:  /* y,c */
#endif
lab_unify_arg_v0c:
    if (sreg == NULL) {  /* write mode */
        BUILD_V0;
        FOLLOW(H++) = *P++;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        goto rr_unify_arg_c;
    }


#ifndef GCC
case unify_arg_uv0:  /* y,y */
#endif
lab_unify_arg_uv0:
    if (sreg == NULL) {  /* build mode */
        BUILD_U(r_unify_arg_uv0);
        BUILD_V0;
        CONTCASE;
    } else {
        YC(*(P+1)) = FOLLOW(sreg+1);
        op1 = FOLLOW(sreg); sreg += 2;
        op2 = YC(*P); P += 2;
        goto rrr_unify_arg_u;
    }

#ifndef GCC
case unify_arg_v0w:  /* y */
#endif
lab_unify_arg_v0w:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        BUILD_W;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++); sreg++;
        CONTCASE;
    }


#ifndef GCC
case unify_arg_wc:  /* c */
#endif
lab_unify_arg_wc:
    if (sreg == NULL) {  /* write mode */
        BUILD_W;
        FOLLOW(H++) = *P++;
        CONTCASE;
    } else {
        sreg++;
        goto rr_unify_arg_c;
    }


#ifndef GCC
case unify_arg_2v0:  /* y,y */
#endif
lab_unify_arg_2v0:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        BUILD_V0;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        CONTCASE;
    }


#ifndef GCC
case unify_arg_3v0:  /* y,y,y */
#endif
lab_unify_arg_3v0:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        BUILD_V0;
        BUILD_V0;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        CONTCASE;
    }


#ifndef GCC
case unify_arg_4v0:  /* y,y,y,y */
#endif
lab_unify_arg_4v0:
    if (sreg == NULL) {  /* build mode */
        BUILD_V0;
        BUILD_V0;
        BUILD_V0;
        BUILD_V0;
        CONTCASE;
    } else {
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        YC(*P++) = FOLLOW(sreg++);
        CONTCASE;
    }


#ifndef GCC
case unify_arg_3u:  /* y,y,y */
#endif
lab_unify_arg_3u:
    /* A3 A2 A1: notice arguments in reversed order */
    if (sreg == NULL) {  /* build mode */
        op1 = YC(*(P+2)); BUILD_ARG_U_VALUE(op1, r_unify_arg_3u); H++;
        op1 = YC(*(P+1)); BUILD_ARG_U_VALUE(op1, rr_unify_arg_3u); H++;
        op1 = YC(*P); BUILD_ARG_U_VALUE(op1, rrr_unify_arg_3u); H++;
        P += 7;  /* 3+2*sizeof(unify_arg_u) */
        CONTCASE;
    } else {
        P += 2;  /* skip A3 and A2 */
        goto rr_unify_arg_u;
    }

#ifndef GCC
case unify_arg_2u:  /* y,y */
#endif
lab_unify_arg_2u:
    /* A2 A1: notice arguments in reversed order */
    if (sreg == NULL) {  /* build mode */
    rr_unify_arg_2u_write:
        op1 = YC(*(P+1)); BUILD_ARG_U_VALUE(op1, r_unify_arg_2u); H++;
        op1 = YC(*P); BUILD_ARG_U_VALUE(op1, rr_unify_arg_2u); H++;
        P += 4;  /* 2+sizeof(unify_arg_u) */
        CONTCASE;
    } else {
        P++;  /* skip A2 */
        goto rr_unify_arg_u;
    }

#ifndef GCC
case unify_arg_2w:  /* E */
#endif
lab_unify_arg_2w:
    if (sreg == NULL) {  /* build mode */
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        CONTCASE;
    } else {
        sreg += 2;
        CONTCASE;
    }


#ifndef GCC
case unify_arg_3w:  /* E */
#endif
lab_unify_arg_3w:
    if (sreg == NULL) {  /* build mode */
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        CONTCASE;
    } else {
        sreg += 3;
        CONTCASE;
    }

#ifndef GCC
case unify_arg_4w:  /* E */
#endif
lab_unify_arg_4w:
    if (sreg == NULL) {  /* build mode */
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        CONTCASE;
    } else {
        sreg += 4;
        CONTCASE;
    }

#ifndef GCC
case unify_arg_5w:  /* E */
#endif
lab_unify_arg_5w:
    if (sreg == NULL) {  /* build mode */
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        FOLLOW(H) = (BPLONG)H; H++;
        CONTCASE;
    } else {
        sreg += 5;
        CONTCASE;
    }

#ifndef GCC
case build_arg_c:  /* c */
#endif
lab_build_arg_c:
    FOLLOW(H++) = *P++;
    CONTCASE;


#ifndef GCC
case build_arg_v:  /* y */
#endif
lab_build_arg_v:
    op1 = *P++; BUILD_ARG_V(op1); H++;
    CONTCASE;


#ifndef GCC
case build_arg_struct:  /* s */
#endif
lab_build_arg_struct:
    H++;
    *(H-1) = ADDTAG(H, STR);
    *H++ = NextOperandSym;
    CONTCASE;

#ifndef GCC
case build_arg_list:  /* i,zs(-1)+1 */
#endif
lab_build_arg_list:
    H++;
    *(H-1) = ADDTAG(H, LST);
    goto rr_build_list;


#ifndef GCC
case build_arg_2c:  /* c,c */
#endif
lab_build_arg_2c:
    FOLLOW(H++) = *P++;
    FOLLOW(H++) = *P++;
    CONTCASE;

#ifndef GCC
case build_arg_3c:  /* c,c,c */
#endif
lab_build_arg_3c:
    FOLLOW(H++) = *P++;
    FOLLOW(H++) = *P++;
    FOLLOW(H++) = *P++;
    CONTCASE;

#ifndef GCC
case build_arg_v0u:  /* y,y */
#endif
lab_build_arg_v0u:
    BUILD_V0;
    BUILD_U(r_build_arg_vu);
    CONTCASE;

#ifndef GCC
case build_arg_v0c:  /* y,c */
#endif
lab_build_arg_v0c:
    BUILD_V0;
    FOLLOW(H++) = *P++;
    CONTCASE;

#ifndef GCC
case build_arg_uv0:  /* y,y */
#endif
lab_build_arg_uv0:
    BUILD_U(r_build_arg_uv0);
    BUILD_V0;
    CONTCASE;

#ifndef GCC
case build_arg_v0w:  /* y */
#endif
lab_build_arg_v0w:
    BUILD_V0;
    BUILD_W;
    CONTCASE;

#ifndef GCC
case build_arg_wc:  /* c */
#endif
lab_build_arg_wc:
    BUILD_W;
    FOLLOW(H++) = *P++;
    CONTCASE;

#ifndef GCC
case build_arg_4v0:  /* y,y,y,y */
#endif
lab_build_arg_4v0:
    BUILD_V0;

#ifndef GCC
case build_arg_3v0:  /* y,y,y */
#endif
lab_build_arg_3v0:
    BUILD_V0;

#ifndef GCC
case build_arg_2v0:  /* y,y */
#endif
lab_build_arg_2v0:
    BUILD_V0;

#ifndef GCC
case build_arg_v0:  /* y */
#endif
lab_build_arg_v0:
    BUILD_V0;
    CONTCASE;


#ifndef GCC
case build_arg_5w:  /* E */
#endif
lab_build_arg_5w:
    FOLLOW(H) = (BPLONG)H; H++;


#ifndef GCC
case build_arg_4w:  /* E */
#endif
lab_build_arg_4w:
    FOLLOW(H) = (BPLONG)H; H++;


#ifndef GCC
case build_arg_3w:  /* E */
#endif
lab_build_arg_3w:
    FOLLOW(H) = (BPLONG)H; H++;

#ifndef GCC
case build_arg_2w:  /* E */
#endif
lab_build_arg_2w:
    FOLLOW(H) = (BPLONG)H; H++;

#ifndef GCC
case build_arg_w:  /* E */
#endif
lab_build_arg_w:
    FOLLOW(H) = (BPLONG)H; H++;
    CONTCASE;


#ifndef GCC
case build_arg_4u:  /* y,y,y,y */
#endif
lab_build_arg_4u:
    op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, r_build_arg_4u); H++;

#ifndef GCC
case build_arg_3u:  /* y,y,y */
#endif
lab_build_arg_3u:
    op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, r_build_arg_3u); H++;

#ifndef GCC
case build_arg_2u:  /* y,y */
#endif
lab_build_arg_2u:
    op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, r_build_arg_2u); H++;

#ifndef GCC
case build_arg_u:  /* y */
#endif
lab_build_arg_u:
    op1 = YC(*P++); BUILD_ARG_U_VALUE(op1, r_build_arg_u); H++;
    CONTCASE;

#ifndef GCC
case unify_list:  /* y,i,zs(-1)+1 */
#endif
lab_unify_list:
    op1 = NextOperandYC;
    arity = NextOperandLiteral;

    SWITCH_OP_LST(op1, rr_unify_list,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      BUILD_REST_LIST_Zs(arity, rrr_unify_list1, rrr_unify_list2, rrr_unify_list3);
                      CONTCASE;},
                  {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = *P++; UNIFY_ARG_Z(sreg, op1, rr_unify_list1);
                      arity--;
                      if (arity > 0) {
                          op1 = FOLLOW(sreg+1);
                          goto rr_unify_list;
                      } else {
                          op1 = *P++; UNIFY_ARG_Z(sreg+1, op1, rr_unify_list7);
                          CONTCASE;}},
                  {op2 = ADDTAG(H, LST);
                      BUILD_REST_LIST_Zs(arity, rrrr_unify_list1, rrrr_unify_list2, rrrr_unify_list3);
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_comp_list:  /* y,i,zs(-1) */
#endif
lab_unify_comp_list:
    op1 = NextOperandYC;
    arity = NextOperandLiteral;

    SWITCH_OP_LST(op1, rr_unify_comp_list,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      BUILD_REST_COMP_LIST_Zs(arity, rrr_unify_comp_list_1, rrr_unify_comp_list_2);
                      CONTCASE;},
                  {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = *P++; UNIFY_ARG_Z(sreg, op1, rr_unify_comp_list_1);
                      arity--;
                      op1 = FOLLOW(sreg+1);
                      if (arity > 0) {
                          goto rr_unify_comp_list;
                      } else {
                          goto rr_unify_nil_y;
                      }},
                  {op2 = ADDTAG(H, LST);
                      BUILD_REST_COMP_LIST_Zs(arity, rrrr_unify_comp_list_1, rrrr_unify_comp_list_2);
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons:  /* y,z,z */
#endif
lab_unify_cons:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons,
                  {rr_unify_cons_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      BUILD_Z(op1, rr_unify_cons1);
                      BUILD_Z(op1, rr_unify_cons2);
                      CONTCASE;},
                  {rr_unify_cons_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = *(P+1); UNIFY_ARG_Z(sreg+1, op1, rrrr_unify_cons1);
                      op1 = *P; P += 2; UNIFY_LAST_ARG_Z(sreg, op1, rrrr_unify_cons2, rrrr_unify_cons3, rrrr_unify_cons4);
                      CONTCASE;},
                  {BPLONG tmp_op;
                  rr_unify_cons_susp:
                      op2 = ADDTAG(H, LST);
                      BUILD_Z(tmp_op, rrr_unify_cons1);
                      BUILD_Z(tmp_op, rrr_unify_cons2);
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_w:  /* y */
#endif
lab_cut_unify_cons_w:
    CUT0;


#ifndef GCC
case unify_cons_w:  /* y */
#endif
lab_unify_cons_w:
    op3 = (BPLONG)NextOperandY;
    op1 = FOLLOW(op3);
    SWITCH_OP_LST(op1, rr_unify_cons_w,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      FOLLOW(op3) = FOLLOW(sreg+1);
                      CONTCASE;},
                  {op2 = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_u:  /* y,y */
#endif
lab_unify_cons_u:
    op3 = (BPLONG)NextOperandY;
    op1 = FOLLOW(op3);
    SWITCH_OP_LST(op1, rr_unify_cons_u,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_u_1); H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      FOLLOW(op3) = FOLLOW(sreg+1);
                      op1 = FOLLOW(sreg); op2 = NextOperandYC;
                      goto rrr_unify_arg_u;},
                  {BPLONG tmp_op;
                      op2 = ADDTAG(H, LST);
                      tmp_op = NextOperandYC;
                      BUILD_ARG_U_VALUE(tmp_op, rrr_unify_cons_u_1); H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_v0:  /* y,y */
#endif
lab_unify_cons_v0:
    op3 = (BPLONG)NextOperandY;
    op1 = FOLLOW(op3);
    SWITCH_OP_LST(op1, rr_unify_cons_v0,
                  {PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P++) = FOLLOW(sreg);
                      FOLLOW(op3) = FOLLOW(sreg+1);
                      CONTCASE;},
                  {op2 = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(op3) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_uu:  /* y,y,y */
#endif
lab_cut_unify_cons_uu:
    CUT0;

#ifndef GCC
case unify_cons_uu:  /* y,y,y */
#endif
lab_unify_cons_uu:
    /* unify_cons_uu y y1 y2
       always followed by unify_arg_u y2 */
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_uu,
                  {rr_unify_cons_uu_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_uu_1); H++;
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_uu_2); H++;
                      P += 2;  /* skip unify_arg_u */
                      CONTCASE;},
                  {rr_unify_cons_uu_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = FOLLOW(sreg++); op2 = NextOperandYC; P++;  /* skip y2 */
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_uu_susp:
                      op2 = ADDTAG(H, LST);
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_uu_1); H++;
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_uu_2); H++;
                      P += 2;  /* skip unify_arg_u */
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_uv:  /* y,y,y */
#endif
lab_cut_unify_cons_uv:
    CUT0;

#ifndef GCC
case unify_cons_uv0:  /* y,y,y */
#endif
lab_unify_cons_uv0:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_uv0,
                  {rr_unify_cons_uv0_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_uv0_1); H++;
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons_uv0_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*(P+1)) = FOLLOW(sreg+1);
                      op1 = FOLLOW(sreg); op2 = YC(*P); P += 2;
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_uv0_susp:
                      op2 = ADDTAG(H, LST);
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_uv0_1); H++;
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_uw:  /* y,y */
#endif
lab_unify_cons_uw:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_uw,
                  {rr_unify_cons_uw_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_uw_1); H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons_uw_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = FOLLOW(sreg); op2 = NextOperandYC;
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_uw_susp:
                      op2 = ADDTAG(H, LST);
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_uw_1); H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_uc:  /* y,y,c */
#endif
lab_unify_cons_uc:
    /* followed by unify_arg_c */
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_uc,
                  {rr_unify_cons_uc_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_uc_1); H++;
                      FOLLOW(H) = *P; P += 3;  /* skip unify_arg_c */
                      H++;
                      CONTCASE;},
                  {rr_unify_cons_uc_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = FOLLOW(sreg++); op2 = NextOperandYC; P++;  /* skip y2, followed by unify_arg_c */
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_uc_susp:
                      op2 = ADDTAG(H, LST);
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_uc_1); H++;
                      FOLLOW(H) = *P; P += 3;  /* skip unify_arg_c */
                      H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_vv:  /* y,y,y */
#endif
lab_cut_unify_cons_vv:
    CUT0;

#ifndef GCC
case unify_cons_v0v0:  /* y,y,y */
#endif
lab_unify_cons_v0v0:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_v0v0,
                  {rr_unify_cons_v0v0_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(*P) = FOLLOW(H) = (BPLONG)H; H++;
                      YC(*(P+1)) = FOLLOW(H) = (BPLONG)H; H++;
                      P += 2;
                      CONTCASE;},
                  {rr_unify_cons_v0v0_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P) = FOLLOW(sreg);
                      YC(*(P+1)) = FOLLOW(sreg+1);
                      P += 2;
                      CONTCASE;},
                  {rr_unify_cons_v0v0_susp:
                      op2 = ADDTAG(H, LST);
                      YC(*P) = FOLLOW(H) = (BPLONG)H; H++;
                      YC(*(P+1)) = FOLLOW(H) = (BPLONG)H; H++;
                      P += 2;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_vu:  /* y,y,y */
#endif
lab_cut_unify_cons_vu:
    CUT0;

#ifndef GCC
case unify_cons_v0u:  /* y,y,y */
#endif
lab_unify_cons_v0u:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_v0u,
                  {rr_unify_cons_v0u_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_v0u_1); H++;
                      CONTCASE;},
                  {rr_unify_cons_v0u_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P) = FOLLOW(sreg);
                      op1 = FOLLOW(sreg+1); op2 = YC(*(P+1)); P += 2;
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_v0u_susp:
                      op2 = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_v0u_1); H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_v0w:  /* y,y */
#endif
lab_unify_cons_v0w:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_v0w,
                  {rr_unify_cons_v0w_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons_v0w_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P++) = FOLLOW(sreg);
                      CONTCASE;},
                  {rr_unify_cons_v0w_susp:
                      op2 = ADDTAG(H, LST);
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_vc:  /* y,y,c */
#endif
lab_cut_unify_cons_vc:
    CUT0;

#ifndef GCC
case unify_cons_v0c:  /* y,y,c */
#endif
lab_unify_cons_v0c:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_v0c,
                  {rr_unify_cons_v0c_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(*P) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = *(P+1); H++;
                      P += 2;
                      CONTCASE;},
                  {rr_unify_cons_v0c_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P) = FOLLOW(sreg);
                      op1 = FOLLOW(sreg+1); op2 = *(P+1); P += 2;
                      goto rrr_unify_arg_c;},
                  {rr_unify_cons_v0c_susp:
                      op2 = ADDTAG(H, LST);
                      YC(*P) = FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = *(P+1); H++;
                      P += 2;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_wv0:  /* y,y */
#endif
lab_unify_cons_wv0:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_wv0,
                  {rr_unify_cons_wv0_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons_wv0_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*P++) = FOLLOW(sreg+1);
                      CONTCASE;},
                  {rr_unify_cons_wv0_susp:
                      op2 = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      YC(*P++) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_cu:  /* y,c,y */
#endif
lab_unify_cons_cu:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_cu,
                  {rr_unify_cons_cu_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = *P++; H++;
                      op1 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op1, rr_unify_cons_cu_1); H++;
                      CONTCASE;},
                  {rr_unify_cons_cu_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = FOLLOW(sreg);
                      op2 = *P++;
                      UNIFY_ARG_CONST(op1, op2, rrr_unify_cons_cu);
                      op1 = FOLLOW(sreg+1); op2 = NextOperandYC;
                      goto rrr_unify_arg_u;},
                  {rr_unify_cons_cu_susp:
                      op2 = ADDTAG(H, LST);
                      FOLLOW(H) = *P++; H++;
                      op3 = NextOperandYC;
                      BUILD_ARG_U_VALUE(op3, rrr_unify_cons_cu_2); H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case cut_unify_cons_cv:  /* y,c,y */
#endif
lab_cut_unify_cons_cv:
    CUT0;

#ifndef GCC
case unify_cons_cv0:  /* y,c,y */
#endif
lab_unify_cons_cv0:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_cv0,
                  {rr_unify_cons_cv0_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = *P; H++;
                      YC(*(P+1)) = FOLLOW(H) = (BPLONG)H; H++;
                      P += 2;
                      CONTCASE;},
                  {rr_unify_cons_cv0_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(*(P+1)) = FOLLOW(sreg+1);
                      op1 = FOLLOW(sreg); op2 = *P; P += 2;
                      goto rrr_unify_arg_c;},
                  {rr_unify_cons_cv0_susp:
                      op2 = ADDTAG(H, LST);
                      FOLLOW(H) = *P; H++;
                      YC(*(P+1)) = FOLLOW(H) = (BPLONG)H; H++;
                      P += 2;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_cc:  /* y,c,c */
#endif
lab_unify_cons_cc:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_cc,
                  {rr_unify_cons_cc_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = *P; H++;
                      FOLLOW(H) = *(P+1); H++;
                      P += 2;
                      CONTCASE;},
                  {rr_unify_cons_cc_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      op1 = FOLLOW(sreg);
                      op2 = *P;
                      UNIFY_ARG_CONST(op1, op2, rrr_unify_cons_cc);
                      op1 = FOLLOW(sreg+1); op2 = *(P+1); P += 2;
                      goto rrr_unify_arg_c;},
                  {rr_unify_cons_cc_susp:
                      op2 = ADDTAG(H, LST);
                      FOLLOW(H) = *P; H++;
                      FOLLOW(H) = *(P+1); H++;
                      P += 2;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons_ww:  /* y */
#endif
lab_unify_cons_ww:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons_ww,
                  {rr_unify_cons_ww_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons_ww_lst:
                      CONTCASE;},
                  {rr_unify_cons_ww_susp:
                      op2 = ADDTAG(H, LST);
                      FOLLOW(H) = (BPLONG)H; H++;
                      FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case unify_cons0_v910:  /* y */
#endif
lab_unify_cons0_v910:
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_unify_cons0_v910,
                  {rr_unify_cons0_v910_var:
                      PUSHTRAIL(op1);
                      FOLLOW(op1) = ADDTAG(H, LST);
                      YC(-9) = FOLLOW(H) = (BPLONG)H; H++;
                      YC(-10) = FOLLOW(H) = (BPLONG)H; H++;
                      CONTCASE;},
                  {rr_unify_cons0_v910_lst:
                      sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                      YC(-9) = FOLLOW(sreg);
                      YC(-10) = FOLLOW(sreg+1);
                      CONTCASE;},
                  {rr_unify_cons0_v910_susp:
                      op2 = ADDTAG(H, LST);
                      YC(-9) = FOLLOW(H) = (BPLONG)H; H++;
                      YC(-10) = FOLLOW(H) = (BPLONG)H; H++;
                      goto bind_suspvar_value;});

    BACKTRACK;


#ifndef GCC
case conc:  /* y,y,l */
#endif
lab_conc:
    /*
      Do special case of append fast. This instruction is followed by:
      label(lab0)
      switch_cons(Y3,lab1,lab2,lab3,Yt,Y3),
      unify_cons(Y1,u(Yt),v(Y1)) 
      tr_det_call0(lab0).
      ** iterate through elements in Y3 if Y1 is a plain variable.
      */
    /* if (!AR_IS_TOP(AR)){P++; CONTCASE;} */
{BPLONG y1, y3;
        y1 = *P++; y3 = *P++;
        op3 = YC(y3);
        DEREF2(op3, {goto r_conc;});
        P++; CONTCASE;  /* skip labNil */

r_conc:
        op2 = op3;  /* make a copy here */
        op1 = YC(y1); DEREF(op1);
        while (ISLIST(op1)) {
            sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            FOLLOW(op3) = ADDTAG(H, LST);
            FOLLOW(H++) = FOLLOW(sreg);  /* car of the previous cons */
            op3 = (BPLONG)H; H++;
            op1 = FOLLOW(sreg+1);
            DEREF(op1);
        }
        if (op2 != op3) {  /* added at least one element into arg3 */
            if (H >= LOCAL_TOP) quit("Stack overflow: conc\n");
            PUSHTRAIL(op2);
            YC(y3) = FOLLOW(op3) = op3;
            if (IS_SUSP_VAR(op1)) op1 = UNTAGGED_ADDR(op1);
            YC(y1) = op1;
        }

        if (ISNIL(op1)) {P = (BPLONG_PTR)*P; CONTCASE;}
        P++;  /* skip labNil */
        CONTCASE;
}

#ifndef GCC
case leng:  /* y,y,l */
#endif
lab_leng:
    /* compute the length of a list. Followed by
       switch_cons(Y1,LabNil,LabVar,LabFail,0,Y1),
       add1(Y2),
       tr_det_call0(Lab,MaxS)
    */
    /* if (!AR_IS_TOP(AR)){P++; CONTCASE;} */

{BPLONG y1, y2;
    y1 = *P++;
    y2 = *P++; op2 = YC(y2); DEREF(op2);
    if (!ISINT(op2)) {P++; CONTCASE;}  /* let add1 handle the error */
    op2 = INTVAL(op2);

    op1 = YC(y1); DEREF(op1);
    while (ISLIST(op1)) {
        op2++;
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op1 = FOLLOW(sreg+1);
        DEREF(op1);
    }
    if (IS_SUSP_VAR(op1)) op1 = UNTAGGED_ADDR(op1);
    YC(y1) = op1;
    YC(y2) = MAKEINT(op2);
    if (ISNIL(op1)) {P = (BPLONG_PTR)*P; CONTCASE;}
    P++;  /* skip labNil */
    CONTCASE;
}

#ifndef GCC
case memb_le:  /* l */
#endif
lab_memb_le:
    /* check member. Followed by
       switch_cons(2,LabNil,LabVar,LabFail,-9,-10),
       bp_fork(LabAlt),
       unify_value(-9,1),
       cut0,return_det,
       noop1(11),
       label(LabAlt),
       cut0,
       tr_nondet_call1_au(Lab,1,2,-10,1,11)
    */
    /* if (!AR_IS_TOP(AR)){P++;CONTCASE;} */

    op2 = YC(1); DEREF(op2);
    if (TAG(op2) != ATM) {P++; CONTCASE;}

    op1 = YC(2); DEREF(op1);
    while (ISLIST(op1)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op3 = FOLLOW(sreg); DEREF(op3);
        if (TAG(op3) != ATM || op2 == op3) {YC(2) = op1; P++; CONTCASE;}
        op1 = FOLLOW(sreg+1);
        DEREF(op1);
    }
    if (ISNIL(op1)) {YC(2) = op1; P = (BPLONG_PTR)*P; CONTCASE;}
    if (IS_SUSP_VAR(op1)) op1 = UNTAGGED_ADDR(op1);
    YC(2) = op1;
    P++;  /* skip labNil */
    CONTCASE;

#ifndef GCC
case memb_el:  /* l */
#endif
lab_memb_el:
    /* check member. Followed by
       switch_cons(1,LabNil,LabVar,LabFail,-9,-10),
       bp_fork(LabAlt),
       unify_value(2,-9),
       cut0,return_det,
       noop1(11),
       label(LabAlt),
       cut0,
       tr_nondet_call1_au(Lab,2,2,2,-10,11)
    */
    /* if (!AR_IS_TOP(AR)){P++;CONTCASE;} */

    op2 = YC(2); DEREF(op2);
    if (TAG(op2) != ATM) {P++; CONTCASE;}

    op1 = YC(1); DEREF(op1);
    while (ISLIST(op1)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op3 = FOLLOW(sreg); DEREF(op3);
        if (TAG(op3) != ATM || op2 == op3) {YC(1) = op1; P++; CONTCASE;}
        op1 = FOLLOW(sreg+1); DEREF(op1);
    }
    if (ISNIL(op1)) {YC(1) = op1; P = (BPLONG_PTR)*P; CONTCASE;}
    if (IS_SUSP_VAR(op1)) op1 = UNTAGGED_ADDR(op1);
    YC(1) = op1;
    P++;  /* skip labNil */
    CONTCASE;

#ifndef GCC
case fork_unify_cons:  /* l,y,z,z */
#endif
lab_fork_unify_cons:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        SWITCH_OP_LST(op1, rr_fork_unify_cons,
                      {SET_FORK;
                          PUSHTRAIL(op1);
                          FOLLOW(op1) = ADDTAG(H, LST);
                          BUILD_Z0(op1, rr_unify_cons0_1);
                          BUILD_Z0(op1, rr_unify_cons0_2);
                          CONTCASE;},
                      {SET_FORK;
                          sreg = (BPLONG_PTR)UNTAGGED_ADDR(op1);
                          op1 = *(P+1); UNIFY_ARG_Z0(sreg+1, op1, rrrr_unify_cons0_1);
                          op1 = *P; P += 2; UNIFY_LAST_ARG_Z0(sreg, op1, rrrr_unify_cons0_2, rrrr_unify_cons0_3, rrrr_unify_cons0_4);
                          CONTCASE;},
                      {SET_FORK;
                          op2 = ADDTAG(H, LST);
                          BUILD_Z0(tmp_op, rrr_unify_cons0_1);
                          BUILD_Z0(tmp_op, rrr_unify_cons0_2);
                          goto bind_suspvar_value;});

        P = (BPLONG_PTR)tmp_op;
        CONTCASE;
}

#ifndef GCC
case fork_unify_cons_uu:  /* l,y,y,y */
#endif
lab_fork_unify_cons_uu:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_uu,
                  {SET_FORK; goto rr_unify_cons_uu_var;},
                  {SET_FORK; goto rr_unify_cons_uu_lst;},
                  {SET_FORK; goto rr_unify_cons_uu_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_uv:  /* l,y,y,y */
#endif
lab_fork_unify_cons_uv:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_uv,
                  {SET_FORK; goto rr_unify_cons_uv0_var;},
                  {SET_FORK; goto rr_unify_cons_uv0_lst;},
                  {SET_FORK; goto rr_unify_cons_uv0_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_uc:  /* l,y,y,c */
#endif
lab_fork_unify_cons_uc:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_uc,
                  {SET_FORK; goto rr_unify_cons_uc_var;},
                  {SET_FORK; goto rr_unify_cons_uc_lst;},
                  {SET_FORK; goto rr_unify_cons_uc_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_uw:  /* l,y,y */
#endif
lab_fork_unify_cons_uw:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_uw,
                  {SET_FORK; goto rr_unify_cons_uw_var;},
                  {SET_FORK; goto rr_unify_cons_uw_lst;},
                  {SET_FORK; goto rr_unify_cons_uw_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_vu:  /* l,y,y,y */
#endif
lab_fork_unify_cons_vu:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_vu,
                  {SET_FORK; goto rr_unify_cons_v0u_var;},
                  {SET_FORK; goto rr_unify_cons_v0u_lst;},
                  {SET_FORK; goto rr_unify_cons_v0u_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_vv:  /* l,y,y,y */
#endif
lab_fork_unify_cons_vv:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_vv,
                  {SET_FORK; goto rr_unify_cons_v0v0_var;},
                  {SET_FORK; goto rr_unify_cons_v0v0_lst;},
                  {SET_FORK; goto rr_unify_cons_v0v0_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_vw:  /* l,y,y */
#endif
lab_fork_unify_cons_vw:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_vw,
                  {SET_FORK; goto rr_unify_cons_v0w_var;},
                  {SET_FORK; goto rr_unify_cons_v0w_lst;},
                  {SET_FORK; goto rr_unify_cons_v0w_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_vc:  /* l,y,y,c */
#endif
lab_fork_unify_cons_vc:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_vc,
                  {SET_FORK; goto rr_unify_cons_v0c_var;},
                  {SET_FORK; goto rr_unify_cons_v0c_lst;},
                  {SET_FORK; goto rr_unify_cons_v0c_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_cc:  /* l,y,c,c */
#endif
lab_fork_unify_cons_cc:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_cc,
                  {SET_FORK; goto rr_unify_cons_cc_var;},
                  {SET_FORK; goto rr_unify_cons_cc_lst;},
                  {SET_FORK; goto rr_unify_cons_cc_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case fork_unify_cons_v910:  /* l,y */
#endif
lab_fork_unify_cons_v910:
{BPLONG tmp_op;
    tmp_op = NextOperandAddr;
    op1 = NextOperandYC;
    SWITCH_OP_LST(op1, rr_fork_unify_cons_v910,
                  {SET_FORK; goto rr_unify_cons0_v910_var;},
                  {SET_FORK; goto rr_unify_cons0_v910_lst;},
                  {SET_FORK; goto rr_unify_cons0_v910_susp;});

    P = (BPLONG_PTR)tmp_op;
    CONTCASE;
}

#ifndef GCC
case unify_value:  /* y,y */
#endif
lab_unify_value:
    op1 = NextOperandYC;
    op2 = NextOperandYC;
    SWITCH_OP(op1, unify_d_d,
              {SWITCH_OP_VAR(op2, unify_v_d,
                             {goto unify_v_v;},
                             {goto unify_v_susp;}, {});
                  ASSIGN_in_toam(op1, op2, {PUSHTRAIL(op1);});
                  CONTCASE;},
              {UNIFY_CONSTANT_21(op2, op1, rr_unify_uy_uy, {PUSHTRAIL(op2);}, {}, {BACKTRACK;});},
              {goto unify_lst_d;},
              {goto unify_str_d;},
              {goto unify_susp_d;});

unify_v_v:
    if (op1 != op2) {  /* op1 is free, op2 is free */
        if ((BPULONG)op1 < (BPULONG)op2) {
            if ((BPULONG)op1 < (BPULONG)H) {  /* op1 not in loc stack */
                ASSIGN_in_toam(op2, op1, PUSHTRAIL(op2));
                CONTCASE;
            } else {  /* op1 points to op2 */
                ASSIGN_in_toam(op1, (BPLONG)H, {PUSHTRAIL_s(op1);});
                ASSIGN_in_toam(op2, (BPLONG)H, {PUSHTRAIL_s(op2);});
                NEW_HEAP_FREE;
                CONTCASE;
            }
        } else {  /* op1 > op2 */
            if ((BPULONG)op2 < (BPULONG)H) {
                ASSIGN_in_toam(op1, op2, {PUSHTRAIL(op1);});
                CONTCASE;
            } else {
                ASSIGN_in_toam(op1, (BPLONG)H, {PUSHTRAIL_s(op1);});
                ASSIGN_in_toam(op2, (BPLONG)H, {PUSHTRAIL_s(op2);});
                NEW_HEAP_FREE;
                CONTCASE;
            }
        }
    }
    CONTCASE;

    /*  unify_nsv_d: */
    SWITCH_OP_VAR(op2, unify_nsv_d,
                  {if (op1 != op2) {
                          if ((BPULONG)op1 < (BPULONG)op2) {
                              ASSIGN_in_toam(op2, op1, {PUSHTRAIL(op2);});
                              CONTCASE;
                          } else {  /* op1 points to op2 */
                              ASSIGN_in_toam(op1, op2, {PUSHTRAIL_h(op1);});
                              CONTCASE;
                          }
                      }
                      CONTCASE;},
                  {goto unify_v_susp;},
                  {});
    ASSIGN_in_toam(op1, op2, {PUSHTRAIL_h(op1);});
    CONTCASE;

unify_v_susp:
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
    FOLLOW(op1) = (BPLONG)dv_ptr;
    PUSHTRAIL(op1);
    CONTCASE;

    /* unify_lst_d: */
    SWITCH_OP(op2, unify_lst_d,
              {FOLLOW(op2) = op1;
                  PUSHTRAIL(op2);
                  CONTCASE;},
              {BACKTRACK;},
              {if (op1 != op2) {
                      UNTAG_ADDR(op1);
                      UNTAG_ADDR(op2);
                      if (!unify(*(BPLONG_PTR)op1, *(BPLONG_PTR)op2)) BACKTRACK;
                      op1 = *((BPLONG_PTR)op1 + 1);
                      op2 = *((BPLONG_PTR)op2 + 1);
                      SWITCH_OP(op1, unify_lst_d_1,
                                {goto unify_nsv_d;},
                                {UNIFY_CONSTANT_21_NOSTACK_VAR(op2, op1, unify_lst_d_2, {BACKTRACK;});},
                                {goto unify_lst_d;},
                                {goto unify_str_d;},
                                {goto unify_susp_d;});
                  };
                  CONTCASE;},
              {BACKTRACK;},
              {goto bind_value_suspvar;});

    /* unify_susp_d */
    SWITCH_OP_VAR(op2, unify_susp_d,
                  {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
                      PUSHTRAIL(op2);
                      FOLLOW(op2) = (BPLONG)dv_ptr;
                      CONTCASE;},
                  {goto unify_suspvars;},
                  {goto bind_suspvar_value;});

bind_suspvar_value:
    BIND_SUSPVAR_VALUE(op1, op2);

bind_value_suspvar:
    BIND_SUSPVAR_VALUE(op2, op1);

unify_suspvars:  /* both are suspending vars */
    if (op1 == op2) CONTCASE;
    if (!unify_suspvar_suspvar(op1, op2)) {
        BACKTRACK;
    }
    CONTCASE;

    /* unify_str_d: */
    SWITCH_OP_STRUCT(op2, unify_str_d,
                     {FOLLOW(op2) = op1;
                         PUSHTRAIL(op2);
                         CONTCASE;},
                     {if (op1 != op2) {
                             UNTAG_ADDR(op1);
                             UNTAG_ADDR(op2);
                             sym_ptr = (SYM_REC_PTR)FOLLOW(op1);
                             if ((BPLONG)sym_ptr != FOLLOW(op2))
                                 BACKTRACK;
                             else {
                                 arity = GET_ARITY(sym_ptr);
                                 for (i = 1; i < arity; i++) {
                                     if (!unify(*((BPLONG_PTR)op1+i), *((BPLONG_PTR)op2+i))) BACKTRACK;
                                 }
                                 op1 = *((BPLONG_PTR)op1+arity);
                                 op2 = *((BPLONG_PTR)op2+arity);
                                 SWITCH_OP(op1, unify_str_d_1,
                                           {goto unify_nsv_d;},
                                           {UNIFY_CONSTANT_21_NOSTACK_VAR(op2, op1, unify_str_d_2, {BACKTRACK;});},
                                           {goto unify_lst_d;},
                                           {goto unify_str_d;},
                                           {goto unify_susp_d;});
                             }
                         }
                         CONTCASE;},
                     {goto bind_value_suspvar;});
    BACKTRACK;




#ifndef GCC
case fork_unify_value:  /* l,y,y */
#endif
lab_fork_unify_value:
{BPLONG tmp_op;
        tmp_op = NextOperandAddr;
        op1 = NextOperandYC;
        op2 = NextOperandYC;
        SWITCH_OP(op1, fork_unify_d_d,
                  {SET_FORK; goto unify_v_d;},
                  {UNIFY_CONSTANT_21(op2, op1, rr_fork_unify_d_c, {SET_FORK; PUSHTRAIL(op2);}, {SET_FORK;}, {P = (BPLONG_PTR)tmp_op; CONTCASE;});},
                  {SET_FORK; goto unify_lst_d;},
                  {SET_FORK; goto unify_str_d;},
                  {SET_FORK; goto unify_susp_d;});
}

#ifndef GCC
case move_constant:  /* y,c */
#endif
lab_move_constant:
    op1 = (BPLONG)NextOperandY;
    op2 = NextOperandConstant;
    FOLLOW(op1) = op2;
    CONTCASE;


#ifndef GCC
case move_struct:  /* y,s */
#endif
lab_move_struct:
    op1 = (BPLONG)NextOperandY;
    PUSHTRAIL_s(op1);
    FOLLOW(op1) = ADDTAG(H, STR);
    FOLLOW(H++) = NextOperandSym;
    CONTCASE;

#ifndef GCC
case move_struct0:  /* y,s */
#endif
lab_move_struct0:
    YC(*P++) = ADDTAG(H, STR);
    FOLLOW(H++) = NextOperandSym;
    CONTCASE;

#ifndef GCC
case move_list:  /* y,i,zs(-1)+1 */
#endif
lab_move_list:
    op1 = (BPLONG)NextOperandY;
    PUSHTRAIL_s(op1);
    FOLLOW(op1) = ADDTAG(H, LST);
rr_build_list:
    arity = NextOperandLiteral;
    BUILD_REST_LIST_Zs(arity , rr_move_list1, rr_move_list2, rr_move_list3);
    CONTCASE;

#ifndef GCC
case move_comp_list:  /* y,i,zs(-1) */
#endif
lab_move_comp_list:
    op1 = (BPLONG)NextOperandY;
    arity = NextOperandLiteral;
    PUSHTRAIL_s(op1);
    FOLLOW(op1) = ADDTAG(H, LST);
    BUILD_REST_COMP_LIST_Zs(arity , rr_move_comp_list1, rr_move_comp_list2);
    CONTCASE;

#ifndef GCC
case move_comp_list1:  /* y,z */
#endif
lab_move_comp_list1:
    op1 = (BPLONG)NextOperandY;
    PUSHTRAIL_s(op1);
    FOLLOW(op1) = ADDTAG(H, LST);
    op1 = *P++; BUILD_ARG_Z(op1, rr_move_comp_list1_1); H++;
    FOLLOW(H++) = nil_sym;
    CONTCASE;

#ifndef GCC
case move_cons:  /* y,z,z */
#endif
lab_move_cons:
{BPLONG tmp_op;
        sreg = H;
        tmp_op = (BPLONG)NextOperandY;
        BUILD_Z(op1, move_cons_1);
        BUILD_Z(op1, move_cons_2);
        PUSHTRAIL_s(tmp_op);
        FOLLOW(tmp_op) = ADDTAG(sreg, LST);
        CONTCASE;
}

#ifndef GCC
case move_cons0_uv:  /* y,y,y */
#endif
lab_move_cons0_uv:
{BPLONG tmp_op;
    tmp_op = (BPLONG)NextOperandY;
    sreg = H;
    BUILD_U(rr_move_cons0_uv_1);
    BUILD_V0;
    FOLLOW(tmp_op) = ADDTAG(sreg, LST);
    CONTCASE;
}

#ifndef GCC
case move_cons0_uu:  /* y,y,y */
#endif
lab_move_cons0_uu:
{BPLONG tmp_op;
    sreg = H;
    tmp_op = (BPLONG)NextOperandY;
    BUILD_U(rr_move_cons0_uu_1);
    BUILD_U(rr_move_cons0_uu_2);
    FOLLOW(tmp_op) = ADDTAG(sreg, LST);
    CONTCASE;
}

#ifndef GCC
case move_var:  /* y */
#endif
lab_move_var:
    op1 = (BPLONG)NextOperandY;
    FOLLOW(op1) = op1;
    CONTCASE;

#ifndef GCC
case move_value:  /* y,y */
#endif
lab_move_value:
    op1 = (BPLONG)NextOperandY;
    op2 = NextOperandYC;
    SWITCH_OP_STACK_VAR(op2, rr_move_value,
                        {PUSHTRAIL_s(op2);
                            PUSHTRAIL_s(op1);
                            FOLLOW(op1) = FOLLOW(op2) = (BPLONG)H;
                            NEW_HEAP_FREE;},
                        {if (TAG(op2) != ATM) {PUSHTRAIL_s(op1);} FOLLOW(op1) = op2;});

    CONTCASE;


#ifndef GCC
case and:  /* z,z,y */
#endif
lab_and:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op1 = INTVAL(op2) & INTVAL(op1);
        if (BP_IN_1W_INT_RANGE(op1)) {
            op1 = MAKEINT(op1);
        } else {
            op1 = bp_int_to_bigint(op1);
            PUSHTRAIL_s(sreg);
        }
        FOLLOW(sreg) = op1;
        CONTCASE;
    }
    SAVE_TOP;
    op1 = bp_bitwise_and(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "/\\", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case or:  /* z,z,y */
#endif
lab_or:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op1 = INTVAL(op2) | INTVAL(op1);
        if (BP_IN_1W_INT_RANGE(op1)) {
            op1 = MAKEINT(op1);
        } else {
            op1 = bp_int_to_bigint(op1);
            PUSHTRAIL_s(sreg);
        }
        FOLLOW(sreg) = op1;
        CONTCASE;
    }
    SAVE_TOP;
    op1 = bp_bitwise_or(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "\\/", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;


#ifndef GCC
case lshiftl:  /* z,z,y */
#endif
lab_lshiftl:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;  /* borrow sreg */
    if (ISINT(op1) && ISINT(op2)) {
        op2 = INTVAL(op2); op1 = INTVAL(op1);
        if (BP_IN_TINYINT_RANGE(op1) && op2 >= 0 && op2 <= 11) {
            FOLLOW(sreg) = MAKEINT(op1 << op2);
            CONTCASE;
        } else {
            op1 = MAKEINT(op1); op2 = MAKEINT(op2);
        }
    }
    SAVE_TOP;
    op1 = bp_bitwise_shiftl(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "<<", 2);
    }
    PUSHTRAIL_s(sreg);
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case lshiftr:  /* z,z,y */
#endif
lab_lshiftr:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;  /* borrow sreg */
    if (ISINT(op1) && ISINT(op2)) {
        op1 = INTVAL(op1); op2 = INTVAL(op2);
        if (op2 >= 0 && BP_IN_28B_INT_RANGE(op1)) {
            FOLLOW(sreg) = MAKEINT(op1 >> op2);
            CONTCASE;
        } else {
            op1 = MAKEINT(op1); op2 = MAKEINT(op2);
        }
    }
    SAVE_TOP;
    op1 = bp_bitwise_shiftr(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, ">>", 2);
    }
    PUSHTRAIL_s(sreg);
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case complement:  /* z,y */
#endif
lab_complement:
    op1 = *P++; OP_NOVY_DEREF(op1);
    sreg = NextOperandY;
    if (ISINT(op1)) {
        op1 = ~INTVAL(op1);
        if (BP_IN_1W_INT_RANGE(op1)) {
            FOLLOW(sreg) = MAKEINT(op1);
        } else {
            PUSHTRAIL_s(sreg);
            FOLLOW(sreg) = bp_int_to_bigint(op1);
        }
    } else {
        SAVE_TOP;
        op1 = bp_bitwise_complement(op1);
        if (op1 == BP_ERROR) {
            RAISE_ISO_EXCEPTION(bp_exception, "\\", 1);
        }
        if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
        FOLLOW(sreg) = op1;
    }
    CONTCASE;


#ifndef GCC
case add_uuv:  /* y,y,y */
#endif
lab_add_uuv:
    op1 = YC(*P++);
    op2 = YC(*P++);
    sreg = NextOperandY;  /* borrow sreg */
    SWITCH_OP_INT(op1, rr_add_uuv1,
                  {},
                  {SWITCH_OP_INT(op2, rr_add_uuv2,
                                 {},
                                 {op1 = INTVAL(op1) + INTVAL(op2);
                                     if (BP_IN_1W_INT_RANGE(op1)) {
                                         FOLLOW(sreg) = MAKEINT(op1);
                                     } else {
                                         SAVE_TOP;
                                         PUSHTRAIL_s(sreg);
                                         FOLLOW(sreg) = bp_int_to_bigint(op1);
                                     }
                                     CONTCASE;},
                                 {})},
                  {});

external_add:
    SAVE_TOP;
    op1 = bp_math_add(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "+", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;


#ifndef GCC
case add_uiv:  /* y,i,y */
#endif
lab_add_uiv:
    op1 = YC(*P++);
    op2 = *P++;
    sreg = NextOperandY;
    SWITCH_OP_INT(op1, rr_add_uiv, {},
                  {op1 = INTVAL(op1) + op2;
                      if (BP_IN_1W_INT_RANGE(op1)) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      }
                      CONTCASE;},
                  {});
    op2 = MAKEINT(op2);
    goto external_add;

#ifndef GCC
case add_u1v:  /* y,y */
#endif
lab_add_u1v:
    op1 = YC(*P++);
    sreg = NextOperandY;
    SWITCH_OP_INT(op1, rr_add_u1v, {},
                  {op1 = INTVAL(op1) + 1;
                      if (op1 <= BP_MAXINT_1W) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      };
                      CONTCASE;},
                  {});

    op2 = BP_ONE;
    goto external_add;

#ifndef GCC
case add1:  /* y */
#endif
lab_add1:
    sreg = Y(*P++);
    op1 = FOLLOW(sreg);
    SWITCH_OP_INT(op1, rr_add1, {},
                  {op1 = INTVAL(op1) + 1;
                      if (op1 <= BP_MAXINT_1W) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      }
                      CONTCASE;},
                  {});
    op2 = BP_ONE;
    goto external_add;

#ifndef GCC
case sub_uuv:  /* y,y,y */
#endif
lab_sub_uuv:
    op1 = YC(*P++);
    op2 = YC(*P++);
    sreg = NextOperandY;
    SWITCH_OP_INT(op1, rr_sub_uuv1,
                  {},
                  {SWITCH_OP_INT(op2, rr_sub_uuv2, {},
                                 {op1 = INTVAL(op1) - INTVAL(op2);
                                     if (BP_IN_1W_INT_RANGE(op1)) {
                                         FOLLOW(sreg) = MAKEINT(op1);
                                     } else {
                                         SAVE_TOP;
                                         PUSHTRAIL_s(sreg);
                                         FOLLOW(sreg) = bp_int_to_bigint(op1);
                                     };
                                     CONTCASE;},
                                 {})},
                  {});

external_sub:
    SAVE_TOP;
    op1 = bp_math_sub(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "-", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case sub_uiv:  /* y,i,y */
#endif
lab_sub_uiv:
    op1 = YC(*P++);
    op2 = *P++;
    sreg = NextOperandY;
    SWITCH_OP_INT(op1, rr_sub_uiv, {},
                  {op1 = INTVAL(op1) - op2;
                      if (BP_IN_1W_INT_RANGE(op1)) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      };
                      CONTCASE;},
                  {});
    op2 = MAKEINT(op2);
    goto external_sub;

#ifndef GCC
case sub_u1v:  /* y,y */
#endif
lab_sub_u1v:
    op1 = YC(*P++);
    sreg = NextOperandY;
    SWITCH_OP_INT(op1, rr_sub_u1v,
                  {},
                  {op1 = INTVAL(op1)-1;
                      if (op1 >= BP_MININT_1W) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      }
                      CONTCASE;},
                  {});
    op2 = BP_ONE;
    goto external_sub;


#ifndef GCC
case sub1:  /* y */
#endif
lab_sub1:
    sreg = NextOperandY;
    op1 = FOLLOW(sreg);
    SWITCH_OP_INT(op1, rr_sub1,
                  {},
                  {op1 = INTVAL(op1)-1;
                      if (op1 >= BP_MININT_1W) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      }
                      CONTCASE;},
                  {});
    op2 = BP_ONE;
    goto external_sub;

#ifndef GCC
case sub_iuv:  /* i,y,y */
#endif
lab_sub_iuv:
    op1 = *P++;
    op2 = YC(*P++);
    sreg = NextOperandY;
    SWITCH_OP_INT(op2, rr_sub_iuv,
                  {},
                  {op1 = op1-INTVAL(op2);
                      if (BP_IN_1W_INT_RANGE(op1)) {
                          FOLLOW(sreg) = MAKEINT(op1);
                      } else {
                          SAVE_TOP;
                          PUSHTRAIL_s(sreg);
                          FOLLOW(sreg) = bp_int_to_bigint(op1);
                      };
                      CONTCASE;},
                  {});
    op1 = MAKEINT(op1);
    goto external_sub;

#ifndef GCC
case mul:  /* z,z,y */
#endif
lab_mul:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
internal_mul:
    if (ISINT(op1) && ISINT(op2)) {
        BPLONG t_op1, t_op2;
        t_op1 = INTVAL(op1);
        t_op2 = INTVAL(op2);
        if (BP_IN_28B_INT_RANGE(t_op1) && BP_IN_28B_INT_RANGE(t_op2)) {
#ifdef M64BITS
            t_op1 = t_op1*t_op2;
            FOLLOW(sreg) = MAKEINT(t_op1);
            CONTCASE;
#else
            double d1, d2;
            d1 = t_op1;
            d2 = t_op2;
            d1 = d1*d2;  /* the result d1*d2 may not fit in one 64-bit word */
            if (BP_IN_28B_INT_RANGE(d1)) {
                t_op1 = (BPLONG)d1;
                FOLLOW(sreg) = MAKEINT(t_op1);
                CONTCASE;
            }
#endif
        }
    }
external_mul:
    SAVE_TOP;
    op1 = bp_math_mul(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "*", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;


#ifndef GCC
case mul_iuv:  /* i,y,y */
#endif
lab_mul_iuv:
    op1 = *P++;
    op2 = YC(*P++); DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op2)) {
        BPLONG t_op2;
        t_op2 = INTVAL(op2);
        if (BP_IN_28B_INT_RANGE(t_op2)) {
#ifdef M64BITS
            t_op2 = op1*t_op2;
            FOLLOW(sreg) = MAKEINT(t_op2);
            CONTCASE;
#else
            double d1, d2;
            d1 = op1;
            d2 = t_op2;
            d1 = d1*d2;  /* the result d1*d2 may not fit in one 64-bit word */
            if (BP_IN_28B_INT_RANGE(d1)) {
                t_op2 = (BPLONG)d1;
                FOLLOW(sreg) = MAKEINT(t_op2);
                CONTCASE;
            }
#endif
        }
    }
    op1 = MAKEINT(op1);
    goto external_mul;



#ifndef GCC
case mul_uuv:  /* y,y,y */
#endif
lab_mul_uuv:
    op1 = YC(*P++); DEREF(op1);
    op2 = YC(*P++); DEREF(op2);
    sreg = NextOperandY;
    goto internal_mul;

#ifndef GCC
case div:  /* z,z,y */
#endif
lab_div:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 == 0) {
            RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "/", 2);
        }
        ASSIGN_ENCODEFLOAT(sreg, (double)INTVAL(op1) / (double)op2);
        CONTCASE;
    }

external_div:
    SAVE_TOP;
    op1 = bp_math_div(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "/", 2);
    }
    PUSHTRAIL_s(sreg);
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case idiv:  /* z,z,y */
#endif
lab_idiv:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 == 0) {
            RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "//", 2);
        }
        FOLLOW(sreg) = MAKEINT((BPLONG)(INTVAL(op1) / op2));
        CONTCASE;
    }
external_idiv:
    SAVE_TOP;
    op1 = bp_math_idiv(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "//", 2);
    }
    if (!ISINT(op1)) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;

#ifndef GCC
case idiv_uiv:  /* y,i,y */
#endif
lab_idiv_uiv:
    op1 = YC(*P++); DEREF(op1);
    op2 = *P++;
    sreg = NextOperandY;
    if (ISINT(op1)) {
        FOLLOW(sreg) = MAKEINT((BPLONG)(INTVAL(op1) / op2));
        CONTCASE;
    }
    op2 = MAKEINT(op2);
    goto external_idiv;


#ifndef GCC
case idiv_uuv:  /* y,y,y */
#endif
lab_idiv_uuv:
    op1 = YC(*P++); DEREF(op1);
    op2 = YC(*P++); DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 == 0) {
            RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "//", 2);
        }
        FOLLOW(sreg) = MAKEINT((BPLONG)(INTVAL(op1) / op2));
        CONTCASE;
    }
    goto external_idiv;


#ifndef GCC
case divge:  /* z,z,y */
#endif
lab_divge:
{BPLONG tmp_op;
        op1 = *P++; OP_NOVY_DEREF(op1);
        op2 = *P++; OP_NOVY_DEREF(op2);
        sreg = NextOperandY;
        if (!ISINT(op1) || !ISINT(op2)) {
            SAVE_TOP;
            op1 = bp_math_divge(op1, op2);
            if (op1 == BP_ERROR) {
                RAISE_ISO_EXCEPTION(bp_exception, "/>", 2);
            }
            PUSHTRAIL_s(sreg);
            FOLLOW(sreg) = op1;
            CONTCASE;
        }
        op1 = INTVAL(op1); op2 = INTVAL(op2);
        if (op2 == 0) {
            RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "//>", 2);
        }
        if (op2 < 0) { op1 = -op1; op2 = -op2;}
        UP_DIV(tmp_op, op1, op2);
        FOLLOW(sreg) = MAKEINT(tmp_op);
        CONTCASE;
}

#ifndef GCC
case divle:  /* z,z,y */
#endif
lab_divle:
{BPLONG tmp_op;
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (!ISINT(op1) || !ISINT(op2)) {
        SAVE_TOP;
        op1 = bp_math_divle(op1, op2);
        if (op1 == BP_ERROR) {
            RAISE_ISO_EXCEPTION(bp_exception, "/>", 2);
        }
        PUSHTRAIL_s(sreg);
        FOLLOW(sreg) = op1;
        CONTCASE;
    }
    op1 = INTVAL(op1); op2 = INTVAL(op2);
    if (op2 == 0) {
        RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "//>", 2);
    }
    if (op2 < 0) { op1 = -op1; op2 = -op2;}
    LOW_DIV(tmp_op, op1, op2);
    FOLLOW(sreg) = MAKEINT(tmp_op);
    CONTCASE;
}

#ifndef GCC
case mod:  /* z,z,y */
#endif
lab_mod:
    op1 = *P++; OP_NOVY_DEREF(op1);
    op2 = *P++; OP_NOVY_DEREF(op2);
    sreg = NextOperandY;
    if (ISINT(op1) && ISINT(op2)) {
        op2 = INTVAL(op2);
        if (op2 == 0) {
            RAISE_ISO_EXCEPTION(et_ZERO_DIVISOR, "mod", 2);
        }
        op1 = INTVAL(op1);
        /*
          FOLLOW(sreg) = MAKEINT((op2>0) ? ((op1>=0) ? op1%op2 : op2+op1%op2)
          : ((op1>=0) ? op2+op1%op2 : op1%op2));
        */
        if (op1 > 0 && op2 > 0) {
            FOLLOW(sreg) = MAKEINT(op1%op2);
            CONTCASE;
        } else if (BP_IN_28B_INT_RANGE(op1) && BP_IN_28B_INT_RANGE(op2)) {
            double f;
            f = (double)op1/(double)op2;
            op3 = (BPLONG)f;
            if (f < op3) {  /* floor */
                op3--;
            }
            FOLLOW(sreg) = MAKEINT(op1-op3*op2);
            CONTCASE;
        } else {
            op1 = MAKEINT(op1);
            op2 = MAKEINT(op2);
        }
    }
    SAVE_TOP;
    op1 = bp_math_mod(op1, op2);
    if (op1 == BP_ERROR) {
        RAISE_ISO_EXCEPTION(bp_exception, "mod", 2);
    }
    FOLLOW(sreg) = op1;
    CONTCASE;



#ifndef GCC
case picat_arg:  /* y,y,y */
#endif
lab_arg:
    op1 = YC(*P++);
arg_check_op1:
    if (!ISINT(op1)) {
        if (ISREF(op1)) {
            NDEREF(op1, arg_check_op1);
        }
        op2 = YC(*P++);
        goto error_arg;
    }

    op1 = INTVAL(op1);
    if (op1 <= 0) {
        op1 = MAKEINT(op1);
        op2 = YC(*P++);
        goto error_arg;
    }

arg_idd:
    op2 = YC(*P++);
    sreg = Y(*P++);
arg_check_op2:
    if (!ISSTRUCT(op2)) {
        if (ISREF(op2)) {
            NDEREF(op2, arg_check_op2);
        }
        if (ISLIST(op2)) {
            if (op1 > 2) BACKTRACK;
            PUSHTRAIL_s(sreg);
            FOLLOW(sreg) = FOLLOW((BPLONG_PTR)UNTAGGED_ADDR(op2) + op1 - 1);
            CONTCASE;
        }
        op1 = MAKEINT(op1);
        goto error_arg;
    }
    op2 = UNTAGGED_ADDR(op2);
    if (op1 > GET_ARITY((SYM_REC_PTR)FOLLOW(op2))) BACKTRACK;
    op1 = FOLLOW((BPLONG_PTR)op2 + op1);
    if (TAG(op1) != ATM) {PUSHTRAIL_s(sreg);}
    FOLLOW(sreg) = op1;
    CONTCASE;

error_arg:
    if (!ISINT(op1)) {
        if (ISREF(op1) || IS_SUSP_VAR(op1)) {
            bp_exception = et_INSTANTIATION_ERROR;
        } else {
            bp_exception = c_type_error(et_INTEGER, op1);
        }
    } else if (INTVAL(op1) <= 0) {
        if (INTVAL(op1) < 0) {
            bp_exception = c_domain_error(et_NOT_LESS_THAN_ZERO, op1);
        } else {
            DEREF(op2);
            if (ISSTRUCT(op2) || ISLIST(op2)) BACKTRACK;
            bp_exception = c_type_error(et_COMPOUND, op2);
        }
    } else if (ISREF(op2) || IS_SUSP_VAR(op2)) {
        bp_exception = et_INSTANTIATION_ERROR;
    } else {
        bp_exception = c_type_error(et_COMPOUND, op2);
    }
    RAISE_ISO_EXCEPTION(bp_exception, "arg", 3);

#ifndef GCC
case arg0:  /* i,y,y */
#endif
lab_arg0:
    op1 = *P++; goto arg_idd;

#ifndef GCC
case setarg:  /* y,y,z */
#endif
lab_setarg:
    op1 = YC(*P++); DEREF(op1);
    if (!ISINT(op1)) {
        if (ISREF(op1)) {
            bp_exception = et_INSTANTIATION_ERROR;
        } else {
            bp_exception = c_type_error(et_INTEGER, op1);
        }
        goto error_setarg;
    } else op1 = INTVAL(op1);

    if (op1 <= 0) {
        bp_exception = c_domain_error(et_NOT_LESS_THAN_ZERO, MAKEINT(op1));
        goto error_setarg;
    }
setarg_idd:
    op2 = YC(*P++); DEREF(op2);
    op3 = *P++; OP_ZC(op3);
    SWITCH_OP_STACK_VAR(op3, setarg3,
                        {FOLLOW(op3) = (BPLONG) H;
                            PUSHTRAIL_s(op3);
                            op3 = (BPLONG)H;
                            NEW_HEAP_FREE;},
                        {});
    if (ISSTRUCT(op2)) {
        if (op1 > GET_STR_SYM_ARITY(op2)) {
            bp_exception = out_of_range;
            goto error_setarg;
        } else {
            sreg = ((BPLONG_PTR)UNTAGGED_ADDR(op2)+op1);
            if (!IS_HEAP_REFERENCE(sreg)) {
                bp_exception = c_update_error(et_UPDATE);
                goto error_setarg;
            }
            if ((BPLONG)sreg != op3) {
                PUSHTRAIL_H_NONATOMIC(sreg, FOLLOW(sreg));
                FOLLOW(sreg) = op3;
            }
            CONTCASE;
        }
    } else if (ISLIST(op2)) {
        if (op1 > 2) {
            bp_exception = out_of_range;
            goto error_setarg;
        } else {
            sreg = ((BPLONG_PTR)UNTAGGED_ADDR(op2)+op1-1);
            if (!IS_HEAP_REFERENCE(sreg)) {
                bp_exception = c_update_error(et_UPDATE);
                goto error_setarg;
            }
            if ((BPLONG)sreg != op3) {
                PUSHTRAIL_H_NONATOMIC(sreg, FOLLOW(sreg));
                FOLLOW(sreg) = op3;
            }
            CONTCASE;
        }
    } else {
        bp_exception = et_INSTANTIATION_ERROR;
    }
error_setarg:
    RAISE_ISO_EXCEPTION(bp_exception, "setarg", 3);



#ifndef GCC
case setarg0:  /* i,y,z */
#endif
lab_setarg0:
    op1 = *P++;
    goto setarg_idd;


#ifndef GCC
case functor:  /* z,z,z */
#endif
lab_functor:
    op1 = *P++; op2 = *P++; op3 = *P++;
    if (IS_YU_OPERAND(op1)) {
        op1 = YC(op1 >> 2);
    } else if (IS_YV_OPERAND(op1)) {
        if (op1 != 0) {op1 = (BPLONG)Y(op1 >> 2); PUSHTRAIL_s(op1); goto functor_vdd;}
        CONTCASE;
    }

    SWITCH_OP(op1, functor_udd,
              {PUSHTRAIL(op1);
                  goto functor_vdd;},

              {i = MAKEINT(0);},

              {i = MAKEINT(2); op1 = period_sym;},

              {sym_ptr = GET_STR_SYM_REC(op1);
                  i = MAKEINT(GET_ARITY(sym_ptr));
                  op1 = ADDTAG(insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 0), ATM);},

              {OP_ZC(op2); OP_ZC(op3);
                  op1 = cfunctor1(op1, op2, op3);
                  if (op1 == BP_ERROR) {
                      RAISE_ISO_EXCEPTION(bp_exception, "functor", 3);} else CONTCASE;
              });
    UNIFY_Z_CONST(op2, op1, rr_functor_1);
    UNIFY_Z_CONST(op3, i, rr_functor_2);
    CONTCASE;

functor_vdd:
    OP_ZC_DEREF(op2);
    OP_ZC_DEREF(op3);

functor_vdd_aux:
    if (ISATOM(op2)) {
        if (ISINT(op3)) {
            sym_ptr = GET_ATM_SYM_REC(op2);
            op3 = INTVAL(op3);
            switch (op3) {
            case 0:
                FOLLOW(op1) = op2;
                CONTCASE;
            case 2:
                if (op2 == period_sym) {
                    FOLLOW(op1) = ADDTAG(H, LST);
                    sreg = H;
                    H += op3;
                    break;
                }
            case 1:
            case 3:
            case 4:
            case 5:
                FOLLOW(op1) = ADDTAG(H, STR);
                *H++ = (BPLONG)insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), op3);
                sreg = H;
                H += op3;
                break;
            default:
                if (op3 < 0) {
                    RAISE_ISO_EXCEPTION(c_domain_error(et_NOT_LESS_THAN_ZERO, MAKEINT(op3)), "functor", 3);
                } else if (op3 > MAX_ARITY) {  /* 2^16 */
                    RAISE_ISO_EXCEPTION(c_representation_error(et_MAX_ARITY), "functor", 3);
                }
                FOLLOW(op1) = ADDTAG(H, STR);
                *H++ = (BPLONG)insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), op3);
                sreg = H;
                H += op3;
                toam_LOCAL_OVERFLOW_CHECK_SMALL_MARGIN(1);
                break;
            }
            while (sreg < H) {
                FOLLOW(sreg) = (BPLONG)sreg; sreg++;
            }
            CONTCASE;
        } else {  /* op3 is not an integer */
            if (ISREF(op3)) {
                RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);
            } if (IS_BIGINT(op3)) {
                RAISE_ISO_EXCEPTION(c_representation_error(et_MAX_ARITY), "functor", 3);
            } else {
                RAISE_ISO_EXCEPTION(c_type_error(et_INTEGER, op3), "functor", 3);
            }
        }
    } else if (ISNUM(op2)) {
        if (ISREF(op3)) {
            RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);
        } else if (ISINT(op3)) {
            if (INTVAL(op3) == 0) {
                FOLLOW(op1) = op2;
                CONTCASE;
            } else {
                RAISE_ISO_EXCEPTION(c_type_error(et_ATOM, op2), "functor", 3);
            }
        } else {
            RAISE_ISO_EXCEPTION(c_type_error(et_INTEGER, op3), "functor", 3);
        }
    } else {
        if (ISREF(op2)) {
            RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);
        } else {
            RAISE_ISO_EXCEPTION(c_type_error(et_ATOMIC, op2), "functor", 3);
        }
    }

#ifndef GCC
case functor_uvv:  /* y,y,y */
#endif
lab_functor_uvv:
    op1 = YC(*P++);
    op2 = (BPLONG)Y(*P++);
    op3 = (BPLONG)Y(*P++);
    SWITCH_OP(op1, rr_functor_uvv,
              {RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);},
              {FOLLOW(op2) = op1; FOLLOW(op3) = MAKEINT(0);},
              {FOLLOW(op2) = period_sym; FOLLOW(op3) = MAKEINT(2);},
              {sym_ptr = GET_STR_SYM_REC(op1);
                  FOLLOW(op2) = ADDTAG(insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 0), ATM);
                  FOLLOW(op3) = MAKEINT(GET_ARITY(sym_ptr));},
              {RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);});
    CONTCASE;

#ifndef GCC
case functor_vuu:  /* y,y,y */
#endif
lab_functor_vuu:
    op1 = (BPLONG)Y(*P++); op2 = YC(*P++); op3 = YC(*P++);
    PUSHTRAIL_s(op1);
    DEREF(op2); DEREF(op3);
    goto functor_vdd_aux;



#ifndef GCC
case functor_arity:  /* y,y */
#endif
lab_functor_arity:
    op1 = YC(*P++);
    op2 = (BPLONG)Y(*P++);
    SWITCH_OP(op1, rr_functor_arity,
              {RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);},
              {FOLLOW(op2) = MAKEINT(0);},
              {FOLLOW(op2) = MAKEINT(2);},
              {sym_ptr = GET_STR_SYM_REC(op1);
                  FOLLOW(op2) = MAKEINT(GET_ARITY(sym_ptr));},
              {RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "functor", 3);});
    CONTCASE;

#ifndef GCC
case get_clause_copy:  /* y,y,y */
#endif
lab_get_clause_copy:
    op1 = NextOperandYC;
    op2 = NextOperandYC;
    op3 = NextOperandYC;
    SAVE_TOP;
    op1 = aux_copy_clause(op1, op2, op3);
    if (op1 == BP_TRUE) {
        CONTCASE;
    } if (op1 == BP_FALSE) {
        BACKTRACK;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, "clause", 3);
    }


#ifndef GCC
case garbage_collect:  /* y */
#endif
lab_garbage_collect:
    op1 = NextOperandYC;
    DEREF(op1);
    op1 = INTVAL(op1);
    if (op1 == 0 || LOCAL_TOP-H <= op1+LARGE_MARGIN) {
        SAVE_AR; SAVE_TOP;
        if (garbage_collector() == BP_ERROR) {
            bp_exception = et_OUT_OF_MEMORY_STACK;
            goto interrupt_handler;
        }
        RESTORE_AR; RESTORE_TOP;
        if (LOCAL_TOP - H < op1+LARGE_MARGIN) {
            if (op1 != 0) {op1 = stack_size+op1+LARGE_MARGIN-(LOCAL_TOP - H);}
            gc_is_working = 1;
            if (expand_local_global_stacks(op1) == BP_ERROR) {
                bp_exception = et_OUT_OF_MEMORY_STACK;
                goto interrupt_handler;
            }
            RESTORE_AR; RESTORE_TOP;
            gc_is_working = 0;
        }
        toam_LOCAL_OVERFLOW_CHECK(8);
    }
    CONTCASE;


#ifndef GCC
case catch_clean_up:  /* E */
#endif
lab_catch_clean_up:
    if (B == AR) {  /*  a catcher frame is in the form of p(Flag,Cleanup,Calll,Exception,Recovery,...) */
        B = (BPLONG_PTR)AR_B(B);
        HB = (BPLONG_PTR)AR_H(B);
        CONTCASE;
    } else {
        goto lab_return_nondet;
    }

#ifndef GCC
case throw_ball:  /* y,y */
#endif
lab_throw_ball:
    op1 = NextOperandYC;
    op2 = (BPLONG)NextOperandY;
    {
        BPLONG catch_all_flag, except;
        BPLONG_PTR btm_ptr;
        BPLONG_PTR af = AR;  /* ancestor frame */
        BPLONG_PTR b = B;
        while ((BPLONG)af != AR_AR(af)) {
            while (b <= af) {
                RESET_SUBGOAL_AR(b);
                if (IS_CATCHER_FRAME(b)) {
                    btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(b));  /* a catcher frame is in the form of p(Flag,Cleanup,Calll,Exception,Recovery,...) */
                    catch_all_flag = FOLLOW(btm_ptr);
                    except = FOLLOW(btm_ptr-3);
                    if ((catch_all_flag == BP_ONE || b == af) && is_UNIFIABLE(op1, except)) {  /* this catcher catches the exception */
                        FOLLOW(op2) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)b, INT_TAG);
                        //                                        reset_temp_complete_subgoal_entries();
                        CONTCASE;
                    }
                }
                b = (BPLONG_PTR)AR_B(b);
            }
            af = (BPLONG_PTR)AR_AR(af);
        }
        FOLLOW(op2) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)af, INT_TAG);
        //              reset_temp_complete_subgoal_entries();
        CONTCASE;
    }

#ifndef GCC
case builtin0:  /* i,l */
#endif
lab_builtin0:
    SAVE_AR; SAVE_TOP;
    EXECUTE_BUILTIN0(op1, *P++);
    if (op1 == 1) {
        P++;
        CONTCASE;
    } else if (op1 == 0) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        RAISE_ISO_EXCEPTION(bp_exception, "builtin", *(P-1));
    }


#ifndef GCC
case builtin1:  /* i,l,z */
#endif
lab_builtin1:
{BPLONG res;
        op1 = *(P+2); OP_ZC(op1);
        SAVE_AR; SAVE_TOP;
        EXECUTE_BUILTIN1(res, *P, op1);
        P += 3;
        if (res == BP_TRUE) {
            CONTCASE;
        } else if (res == BP_FALSE) {
            P = (BPLONG_PTR)*(P-2);
            CONTCASE;
        } else {
            if (!is_iso_exception(bp_exception)) {
                bp_exception = c_builtin_error1(bp_exception, op1);
            }
            RAISE_ISO_EXCEPTION(bp_exception, "builtin", *(P-3));
        }
}

#ifndef GCC
case builtin2:  /* i,l,z,z */
#endif
lab_builtin2:
{BPLONG res;
    op1 = *(P+2); OP_ZC(op1);
    op2 = *(P+3); OP_ZC(op2);
    SAVE_AR; SAVE_TOP;
    EXECUTE_BUILTIN2(res, *P, op1, op2);
    P += 4;
    if (res == BP_TRUE) {
        CONTCASE;
    } else if (res == BP_FALSE) {
        P = (BPLONG_PTR)*(P-3);
        CONTCASE;
    } else {
        if (!is_iso_exception(bp_exception)) {
            bp_exception = c_builtin_error2(bp_exception, op1, op2);
        }
        RAISE_ISO_EXCEPTION(bp_exception, "builtin", *(P-4));
    }
}

#ifndef GCC
case builtin3:  /* i,l,z,z,z */
#endif
lab_builtin3:
{BPLONG res;
    op1 = *(P+2); OP_ZC(op1);
    op2 = *(P+3); OP_ZC(op2);
    op3 = *(P+4); OP_ZC(op3);
    SAVE_AR; SAVE_TOP;
    EXECUTE_BUILTIN3(res, *P, op1, op2, op3);
    P += 5;
    if (res == BP_TRUE) {
        CONTCASE;
    } else if (res == BP_FALSE) {
        P = (BPLONG_PTR)*(P-4);
        CONTCASE;
    } else {
        if (!is_iso_exception(bp_exception)) {
            bp_exception = c_builtin_error3(bp_exception, op1, op2, op3);
        }
        RAISE_ISO_EXCEPTION(bp_exception, "builtin", *(P-5));
    }
}

#ifndef GCC
case builtin4:  /* i,l,z,z,z,z */
#endif
lab_builtin4:
{BPLONG res, op4;
    op1 = *(P+2); OP_ZC(op1);
    op2 = *(P+3); OP_ZC(op2);
    op3 = *(P+4); OP_ZC(op3);
    op4 = *(P+5); OP_ZC(op4);
    SAVE_AR; SAVE_TOP;
    EXECUTE_BUILTIN4(res, *P, op1, op2, op3, op4);
    P += 6;
    if (res == BP_TRUE) {
        CONTCASE;
    } else if (res == BP_FALSE) {
        P = (BPLONG_PTR)*(P-5);
        CONTCASE;
    } else {
        if (!is_iso_exception(bp_exception)) {
            bp_exception = c_builtin_error4(bp_exception, op1, op2, op3, op4);
        }
        RAISE_ISO_EXCEPTION(bp_exception, "builtin", *(P-6));
    }
}

#ifndef GCC
case move_vars:  /* i,i */
#endif
lab_move_vars:
    op1 = *P++;
    op2 = *P++;
    for (i = op1; i <= op2; i++) {
        top = (BPLONG_PTR)Y(i);
        FOLLOW(top) = (BPLONG)top;
    }
    CONTCASE;

#ifndef GCC
case last_call:  /* i,s,zs(s(-1)),i */
#endif
lab_last_call:
    op3 = NextOperandLiteral;  /* misplaced args */
    sym_ptr = (SYM_REC_PTR)*P;
    arity = GET_ARITY(sym_ptr);
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
        tmp_op = op3;
        sreg = LOCAL_TOP-16;  /* temporary frame, no tro call has more than 16 args */
        if (head_arity == arity) {
            COPY_TAIL_ZARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_ZARGS(top, op3, sreg);
        } else {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);

            COPY_TAIL_ZARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_ZARGS(top, op3, sreg);

            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            AR = LOCAL_TOP;
        }
        if (GET_ETYPE(sym_ptr) == T_PRED) {
            P = (BPLONG_PTR)GET_EP(sym_ptr);
            CONTCASE;
        } else goto call_sub_after_ar_cps;
    }
    P++;
    while (arity > 0) {
        op1 = *P++;
        PASS_ARG_Z(op1);
        arity--;
    }
    P++;  /* skip MaxS */
    goto call_sub;

#ifndef GCC
case last_call_var0:  /* y,i */
#endif
lab_last_call_var0:
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG_PTR parent_ar, parent_cps;

        op1 = NextOperandYC; DEREF(op1); P++;  /* skip MaxS for GC */

        parent_ar = (BPLONG_PTR)AR_AR(AR);
        parent_cps = (BPLONG_PTR)AR_CPS(AR);  /* first discard the frame */
        LOCAL_TOP = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR));

        if (ISSTRUCT(op1)) {
            top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            sym_ptr = (SYM_REC_PTR)FOLLOW(top);
            head_arity = GET_ARITY(sym_ptr);
            for (i = 1; i <= head_arity; i++) {
                *LOCAL_TOP-- = FOLLOW(top+i);
            }
        } else if (ISATOM(op1)) {
            sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(op1);
        } else if (ISLIST(op1)) {
            sym_ptr = list_psc;
            top = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            *LOCAL_TOP-- = FOLLOW(top);
            *LOCAL_TOP-- = FOLLOW(top+1);
        } else {
            if (ISREF(op1)) {
                RAISE_ISO_EXCEPTION(et_INSTANTIATION_ERROR, "call", 1);
            } else {
                RAISE_ISO_EXCEPTION(c_type_error(et_CALLABLE, op1), "call", 1);
            }
        }

        AR = LOCAL_TOP;
        AR_AR(AR) = (BPLONG)parent_ar;
        AR_CPS(AR) = (BPLONG)parent_cps;
        goto call_sub_after_ar_cps;
    } else {
        goto lab_call_var0;
    }

#ifndef GCC
case last_call0:  /* s,i */
#endif
lab_last_call0:
    sym_ptr = (SYM_REC_PTR)*P++; P++;  /* skip MaxS */
    arity = GET_ARITY(sym_ptr);
    head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        if (head_arity != arity) {
            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = AR_AR(AR);
            AR_CPS(LOCAL_TOP) = AR_CPS(AR);
            AR = LOCAL_TOP;
        }
        if (GET_ETYPE(sym_ptr) == T_PRED) {
            P = (BPLONG_PTR)GET_EP(sym_ptr);
            CONTCASE;
        } else goto call_sub_after_ar_cps;
    } else {  /* copy arity args from the current frame */
        sreg = AR+head_arity;
        for (i = 0; i < arity; i++) {
            FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
        }
        goto call_sub;
    }

#ifndef GCC
case last_call_d:  /* l,i,i,zs(-1),i */
#endif
lab_last_call_d:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
        tmp_op = op3;
        sreg = LOCAL_TOP-16;  /* temporary frame, no tro call has more than 16 args */
        if (head_arity == arity) {
            COPY_TAIL_ZARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_ZARGS(top, op3, sreg);
        } else {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);

            COPY_TAIL_ZARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_ZARGS(top, op3, sreg);

            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            AR = LOCAL_TOP;
        }

        P = ep;
        CONTCASE;
    }
    P++;
rr_call_d:
    PASS_Z_ARGS(arity);
    P++;  /* skip MaxS */
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;

#ifndef GCC
case last_call1_d:  /* l,i,i,zs(-1),i */
#endif
lab_last_call1_d:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* nogood arg index, the first arg is indexed 1 */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
        op1 = FOLLOW(P+op3);
        if (IS_YU_OPERAND(op1)) {
            top = Y(op1 >> 2); op1 = FOLLOW(top);
            if ((BPLONG_PTR)op1 == top) {FOLLOW(top) = op1 = (BPLONG)H; NEW_HEAP_FREE;};
        } else if (IS_YV_OPERAND(op1)) {
            op1 = (BPLONG)H;
            NEW_HEAP_FREE;
        }
        if (head_arity != arity) {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);
            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            FOLLOW(AR+head_arity+1-op3) = op1;
            AR = LOCAL_TOP;
        } else {
            FOLLOW(AR+arity+1-op3) = op1;
        }
        P = ep;
        CONTCASE;
    } else {
        P++;
        goto rr_call_d;
    }

#ifndef GCC
case last_call_au_d:  /* l,i,i,ys(-1),i */
#endif
lab_last_call_au_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[last_call_au_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call_au_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[last_call_au_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call_au_nondet;
    } else {
        *(void **)(P-1) = jmp_table[last_call_au_ot];
        goto lab_last_call_au_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = last_call_au_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call_au_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = last_call_au_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call_au_nondet;
    } else {
        FOLLOW(P-1) = last_call_au_ot;
        goto lab_last_call_au_ot;
    }
#endif

#ifndef GCC
case last_call_au_det:  /* l,i,i,ys(-1),i */
#endif
lab_last_call_au_det:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        head_arity = ((BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR);
        tmp_op = op3;
        sreg = LOCAL_TOP-16;  /* temporary frame, no tro call has more than 16 args */
        if (head_arity == arity) {
            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);
        } else {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);

            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);

            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            AR = LOCAL_TOP;
        }
        P = ep;
        goto lab_allocate_det;
    }
    P++;
rr_call_au_det:

    PASS_U_ARGS(arity);

    P++;  /* skip MaxS */

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_det;


#ifndef GCC
case last_call_au_nondet:  /* l,i,i,ys(-1),i */
#endif
lab_last_call_au_nondet:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        head_arity = ((BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR);
        tmp_op = op3;
        sreg = LOCAL_TOP-16;  /* temporary frame, no tro call has more than 16 args */
        if (head_arity == arity) {
            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);
        } else {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);

            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);

            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            AR = LOCAL_TOP;
        }
        P = ep;
        goto lab_allocate_nondet;
    }
    P++;
rr_call_au_nondet:

    PASS_U_ARGS(arity);

    P++;  /* skip MaxS */

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    goto lab_allocate_nondet;



#ifndef GCC
case last_call_au_ot:  /* l,i,i,ys(-1),i */
#endif
lab_last_call_au_ot:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        head_arity = ((BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR);
        tmp_op = op3;
        sreg = LOCAL_TOP-16;  /* temporary frame, no tro call has more than 16 args */
        if (head_arity == arity) {
            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);
        } else {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);

            COPY_TAIL_YARGS_OUT(tmp_op, sreg);
            top = AR+head_arity+1;
            REARRANGE_TAIL_YARGS(top, op3, sreg);

            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            AR = LOCAL_TOP;
        }
        P = ep;
        CONTCASE;
    }
    P++;
rr_call_au_d:
    PASS_U_ARGS(arity);

    P++;  /* skip MaxS */

    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = ep;
    CONTCASE;


#ifndef GCC
case last_call1_au_d:  /* l,i,i,ys(-1),i */
#endif
lab_last_call1_au_d:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* nogood arg index, the first arg is indexed 1 */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
        op1 = FOLLOW(P+op3);
        top = Y(op1); op1 = FOLLOW(top);
        if ((BPLONG_PTR)op1 == top) {FOLLOW(top) = op1 = (BPLONG)H; NEW_HEAP_FREE;};
        if (head_arity != arity) {
            BPLONG_PTR parent_ar, parent_cps;

            parent_ar = (BPLONG_PTR)AR_AR(AR);
            parent_cps = (BPLONG_PTR)AR_CPS(AR);
            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = (BPLONG)parent_ar;
            AR_CPS(LOCAL_TOP) = (BPLONG)parent_cps;
            FOLLOW(AR+head_arity+1-op3) = op1;
            AR = LOCAL_TOP;
        } else {
            FOLLOW(AR+arity+1-op3) = op1;
        }
        P = ep;
        CONTCASE;
    } else {
        P++;
        goto rr_call_au_d;
    }

#ifndef GCC
case last_call0_d:  /* l,i,i */
#endif
lab_last_call0_d:
    ep = (BPLONG_PTR)*P++;
    arity = *P++; P++;  /* skip MaxS */
    head_arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        if (head_arity != arity) {
            LOCAL_TOP = AR+(head_arity-arity);
            AR_AR(LOCAL_TOP) = AR_AR(AR);
            AR_CPS(LOCAL_TOP) = AR_CPS(AR);
            AR = LOCAL_TOP;
        }
        P = ep;
        CONTCASE;
    }  /* copy n args from the current frame */

    sreg = AR+head_arity;
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = (BPLONG_PTR)ep;
    CONTCASE;

#ifndef GCC
case last_call0_sa_d:  /* l,i */
#endif
lab_last_call0_sa_d:
    ep = (BPLONG_PTR)*P;
    op1 = *ep;
#ifdef GCC
    if ((void *)op1 == jmp_table[allocate_det]) {
        *(void **)(P-1) = jmp_table[last_call0_sa_det];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call0_sa_det;
    } else if ((void *)op1 == jmp_table[allocate_nondet]) {
        *(void **)(P-1) = jmp_table[last_call0_sa_nondet];
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call0_sa_nondet;
    } else {
        *(void **)(P-1) = jmp_table[last_call0_sa_ot];
        goto lab_last_call0_sa_ot;
    }
#else
    if (op1 == allocate_det) {
        FOLLOW(P-1) = last_call0_sa_det;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call0_sa_det;
    } else if (op1 == allocate_nondet) {
        FOLLOW(P-1) = last_call0_sa_nondet;
        FOLLOW(P) = (BPLONG)(ep+1);
        goto lab_last_call0_sa_nondet;
    } else {
        FOLLOW(P-1) = last_call0_sa_ot;
        goto lab_last_call0_sa_ot;
    }
#endif

#ifndef GCC
case last_call0_sa_det:  /* l,i */
#endif
lab_last_call0_sa_det:
    /* the current and the last call have the same arity */
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        P = (BPLONG_PTR)*P;
        goto lab_allocate_det;
    }  /* copy n args from the current frame */

    ep = (BPLONG_PTR)*P++; P++;  /* skip MaxS */
    arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    sreg = AR+arity;
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = (BPLONG_PTR)ep;
    goto lab_allocate_det;

#ifndef GCC
case last_call0_sa_nondet:  /* l,i */
#endif
lab_last_call0_sa_nondet:
    /* the current and the last call have the same arity */
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        P = (BPLONG_PTR)*P;
        goto lab_allocate_nondet;
    }  /* copy n args from the current frame */

    ep = (BPLONG_PTR)*P++; P++;  /* skip MaxS */
    arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    sreg = AR+arity;
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = (BPLONG_PTR)ep;
    goto lab_allocate_nondet;

#ifndef GCC
case last_call0_sa_ot:  /* l,i */
#endif
lab_last_call0_sa_ot:
    /* the current and the last call have the same arity */
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }  /* copy n args from the current frame */

    ep = (BPLONG_PTR)*P++; P++;  /* skip MaxS */
    arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    sreg = AR+arity;
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)P;
    P = (BPLONG_PTR)ep;
    CONTCASE;

#ifndef GCC
case tr_det_call_au:  /* l,i,i,ys(-1),i */
#endif
lab_tr_det_call_au:
    ep = (BPLONG_PTR)*P++;
    op3 = *P++;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        tmp_op = op3;
        sreg = LOCAL_TOP-arity;  /* temporary frame */
        COPY_TAIL_YARGS_OUT(tmp_op, sreg);
        top = AR+arity+1;
        REARRANGE_TAIL_YARGS(top, op3, sreg);
        P = ep;
        goto rr_allocate_det;
    }
    P++;
    while (arity > 0) {
        *LOCAL_TOP-- = NextOperandYC;
        arity--;
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)(P+1);  /* skip MaxS */
    P = (ep-5);  /* the size of allocate_det */
    CONTCASE;


#ifndef GCC
case tr_det_call1_au:  /* l,i,i,ys(-1),i */
#endif
lab_tr_det_call1_au:
    ep = (BPLONG_PTR)NextOperandAddr;
    op3 = NextOperandLiteral;  /* misplaced arg index */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        op1 = FOLLOW(P+op3);
        top = Y(op1); op1 = FOLLOW(top);
        if ((BPLONG_PTR)op1 == top) {FOLLOW(top) = op1 = (BPLONG)H; NEW_HEAP_FREE;};
        FOLLOW(AR+arity+1-op3) = op1;
        P = ep;
        goto rr_allocate_det;
    } else {
        ep -= 5;  /* the size of allocate_det */
        P++;
        goto rr_call_au_d;
    }

#ifndef GCC
case tr_det_call2_au:  /* l,i,i,i,ys(-1),i */
#endif
lab_tr_det_call2_au:
    ep = (BPLONG_PTR)*P++;
    op2 = *P++;  /* misplaced arg index */
    op3 = *P++;
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG arg1, arg2;
        sreg = AR+arity+1;
        top = Y(FOLLOW(P+op2)); arg1 = FOLLOW(top);
        if ((BPLONG_PTR)arg1 == top) {FOLLOW(top) = arg1 = (BPLONG)H; NEW_HEAP_FREE;};
        top = Y(FOLLOW(P+op3)); arg2 = FOLLOW(top);
        if ((BPLONG_PTR)arg2 == top) {FOLLOW(top) = arg2 = (BPLONG)H; NEW_HEAP_FREE;};
        FOLLOW(sreg-op2) = arg1;
        FOLLOW(sreg-op3) = arg2;
        P = ep;
        goto rr_allocate_det;
    } else {
        ep -= 5;  /* the size of allocate_det */
        P++;
        goto rr_call_au_d;
    }

#ifndef GCC
case tr_det_call0:  /* l,i */
#endif
lab_tr_det_call0:
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        P = (BPLONG_PTR)*P;
        goto rr_allocate_det;
    }

    arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    sreg = AR+arity;  /* copy n args from the current frame */
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)(P+2);
    P = (BPLONG_PTR)*P-5;  /* size of allocate_det = 5 */
    CONTCASE;  /* two possible insts: allocate_det and allocate_det_b */


#ifndef GCC
case tr_nondet_call_au:  /* l,i,i,ys(-1),i */
#endif
lab_tr_nondet_call_au:
    ep = (BPLONG_PTR)*P++;
    op3 = *P++;  /* layout bits */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG tmp_op;
        tmp_op = op3;
        sreg = LOCAL_TOP-arity;  /* temporary frame */
        COPY_TAIL_YARGS_OUT(tmp_op, sreg);
        top = AR+arity+1;
        REARRANGE_TAIL_YARGS(top, op3, sreg);
        P = ep;
        goto rr_tr_nondet_call;
    }
rr_tr_nondet_call_au_noreuse:
    P++;
    while (arity > 0) {
        *LOCAL_TOP-- = NextOperandYC;
        arity--;
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)(P+1);  /* skip MaxS */
    P = (ep-6);  /* sizes of allocate_nondet and nondet = 7 */
    goto lab_allocate_nondet;

#ifndef GCC
case tr_nondet_call1_au:  /* l,i,i,ys(-1),i */
#endif
lab_tr_nondet_call1_au:
    ep = (BPLONG_PTR)*P++;
    op3 = *P++;  /* misplaced arg index */
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        op1 = FOLLOW(P+op3);
        top = Y(op1); op1 = FOLLOW(top);
        if ((BPLONG_PTR)op1 == top) {FOLLOW(top) = op1 = (BPLONG)H; NEW_HEAP_FREE;};
        FOLLOW(AR+arity+1-op3) = op1;
        P = ep;
        goto rr_tr_nondet_call;
    } else {
        goto rr_tr_nondet_call_au_noreuse;
    }

#ifndef GCC
case tr_nondet_call2_au:  /* l,i,i,i,ys(-1),i */
#endif
lab_tr_nondet_call2_au:
    ep = (BPLONG_PTR)*P++;
    op2 = *P++;  /* misplaced arg index */
    op3 = *P++;
    arity = *P;
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        BPLONG arg1, arg2;
        sreg = AR+arity+1;
        top = Y(FOLLOW(P+op2)); arg1 = FOLLOW(top);
        if ((BPLONG_PTR)arg1 == top) {FOLLOW(top) = arg1 = (BPLONG)H; NEW_HEAP_FREE;};
        top = Y(FOLLOW(P+op3)); arg2 = FOLLOW(top);
        if ((BPLONG_PTR)arg2 == top) {FOLLOW(top) = arg2 = (BPLONG)H; NEW_HEAP_FREE;};
        FOLLOW(sreg-op2) = arg1;
        FOLLOW(sreg-op3) = arg2;
        P = ep;
        goto rr_tr_nondet_call;
    } else {
        goto rr_tr_nondet_call_au_noreuse;
    }

#ifndef GCC
case tr_nondet_call0:  /* l,i */
#endif
lab_tr_nondet_call0:
    if (AR_IS_TOP(AR)) {  /* reuse the frame */
        P = (BPLONG_PTR)*P;
    rr_tr_nondet_call:
        if (toam_signal_vec == 0) {
            INVOKE_GC_NONDET;
            AR_H(AR) = (BPLONG)H;
            HB = H;
            AR_T(AR) = (BPLONG)T;
            AR_SF(AR) = (BPLONG)SF;
            B = AR;
            CONTCASE;
        } else {
            P -= 2;  /* size of nondet */
            goto rr_allocate_nondet;
        }
    }

    arity = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(AR))-AR;
    sreg = AR+arity;  /* copy n args from the current frame */
    for (i = 0; i < arity; i++) {
        FOLLOW(LOCAL_TOP--) = FOLLOW(sreg--);
    }
    *LOCAL_TOP = (BPLONG)AR;
    AR = LOCAL_TOP;
    AR_CPS(AR) = (BPLONG)(P+2);
    P = (BPLONG_PTR)*P-6;  /* size of allocate_nondet and nondet */
    goto lab_allocate_nondet;



#ifndef GCC
case allocate_susp:  /* i,i,s,i */
#endif
lab_allocate_susp:
    op1 = *P++;
    AR_BTM(AR) = ADDTAG((AR+op1), SUSP_FRAME_TAG);
    op1 = *P++;
    P += 2;
    LOCAL_TOP = AR-op1;
    AR_STATUS(AR) = SUSP_START;
    AR_OUT(AR) = dummy_event_object;
    REAL_INITIALIZE_STACK_VARS(SUSP_FRAME_SIZE);
    AR_TOP(AR) = (BPLONG)LOCAL_TOP;
    INVOKE_GC;
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case return_commit:  /* E */
#endif
lab_return_commit:
    if (FRAME_IS_START(AR) || FRAME_IS_CLONE(AR)) {
        goto lab_return_det;
    } else {
        goto lab_return_nondet;
    }


#ifndef GCC
case return_delay:  /* E */
#endif
lab_return_delay:
    sreg = AR;
    P = (BPLONG_PTR)AR_CPS(AR);
    AR = (BPLONG_PTR)AR_AR(AR);
    if (AR_STATUS(sreg) == SUSP_EXIT);
    else if (FRAME_IS_CLONE(sreg)) {
        if (AR_IS_TOP(sreg))
            LOCAL_TOP = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(sreg));
    } else if (FRAME_IS_REENT(sreg)) {
        CONNECT_WOKEN_FRAME(sreg);
        AR_STATUS(sreg) = AR_STATUS_STAMP(sreg) | SUSP_RUN;
        P = (BPLONG_PTR)FOLLOW(AR_REEP(AR));
    } else {
        AR_STATUS(sreg) = AR_STATUS_STAMP(sreg) | SUSP_SLEEP;
    }
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case no_vars_gt:  /* i,i,l */
#endif
lab_no_vars_gt:
    arity = *P++;
    op1 = *P++;  /* limit */
    op3 = 0;  /* total no of vars */
    while (arity > 0) {
        op2 = YC(arity);
        SWITCH_OP(op2, rr_no_vars_gt,
                  {op3++; if (op3 > op1) {P++; CONTCASE;}},
                  {},
                  {if ((op3 = n_vars_gt(op3, op2, op1)) == -1L) {P++; CONTCASE;}},
                  {if ((op3 = n_vars_gt(op3, op2, op1)) == -1L) {P++; CONTCASE;}},
                  {op3++; if (op3 > op1) {P++; CONTCASE;}});

        arity--;
    }
    P = (BPLONG_PTR)*P;
    CONTCASE;


#ifndef GCC
case trigger_var_ins:  /* y */
#endif
lab_trigger_var_ins:
    op2 = NextOperandYC;
    SWITCH_OP(op2, rr_trigger_var_ins,
              {CREATE_SUSP_VAR_ins(op2, AR);},
              {},
              {SAVE_AR; trigger_vars_ins(op2);},
              {SAVE_AR; trigger_vars_ins(op2);},
              {INSERT_SUSP_CALL(op2, A_DV_ins_cs(dv_ptr), AR);});
    CONTCASE;

#ifndef GCC
case trigger_var_minmax:  /* y */
#endif
lab_trigger_var_minmax:
    op2 = NextOperandYC;
    SWITCH_OP(op2, rr_trigger_var_minmax,
              {CREATE_SUSP_VAR_minmax(op2, AR);},
              {},
              {SAVE_AR; trigger_vars_minmax(op2);},
              {SAVE_AR; trigger_vars_minmax(op2);},
              {INSERT_SUSP_CALL_NOINS(op2, A_DV_minmax_cs(dv_ptr), AR);});
    CONTCASE;



#ifndef GCC
case trigger_var_dom:  /* y */
#endif
lab_trigger_var_dom:
    op2 = NextOperandYC;
    SWITCH_OP(op2, rr_trigger_var_dom,
              {CREATE_SUSP_VAR_dom(op2, AR);},
              {},
              {SAVE_AR; trigger_vars_dom(op2);},
              {SAVE_AR; trigger_vars_dom(op2);},
              {INSERT_SUSP_CALL_NOINS(op2, A_DV_dom_cs(dv_ptr), AR);});
    CONTCASE;




#ifndef GCC
case trigger_ins_min_max:  /* i */
#endif
lab_trigger_ins_min_max:
    op1 = *P++;
    sreg = AR+2*op1+2;  /* CoesType */
    op2 = FOLLOW(sreg); DEREF_NONVAR(op2); FOLLOW(sreg) = op2;
    sreg--;  /* constant */
    op2 = FOLLOW(sreg); DEREF_NONVAR(op2); FOLLOW(sreg) = op2;
    for (i = 1; i <= op1; i++) {
        sreg--;
        op2 = FOLLOW(sreg); DEREF_NONVAR(op2); FOLLOW(sreg) = op2;
    }

    for (i = 1; i <= op1; i++) {
        op2 = FOLLOW(AR+i); DEREF_NONVAR(op2);
        if (IS_SUSP_VAR(op2)) {
            INSERT_SUSP_CALL(op2, A_DV_ins_cs(dv_ptr), AR);
            INSERT_SUSP_CALL_NOINS(op2, A_DV_minmax_cs(dv_ptr), AR);
        } else if (ISREF(op2)) {
            CREATE_SUSP_VAR_ins(op2, AR);
            INSERT_SUSP_CALL(op2, A_DV_minmax_cs(dv_ptr), AR);
        }
    }
    CONTCASE;

#ifndef GCC
case trigger_var_any_dom:  /* y */
#endif
lab_trigger_var_any_dom:
    op2 = NextOperandYC;
    SWITCH_OP(op2, rr_trigger_var_any_dom,
              {CREATE_SUSP_VAR_any_dom(op2, AR);},
              {},
              {SAVE_AR; trigger_vars_any_dom(op2);},
              {SAVE_AR; trigger_vars_any_dom(op2);},
              {INSERT_SUSP_CALL_NOINS(op2, A_DV_dom_cs(dv_ptr), AR);
                  INSERT_SUSP_CALL_NOINS(op2, A_DV_outer_dom_cs(dv_ptr), AR);});
    CONTCASE;

#ifndef GCC
case trigger_cg_event_handler:  /* i,y */
#endif
lab_trigger_cg_event_handler:
    op1 = NextOperandLiteral;  /* event no */
    op2 = NextOperandYC;
    SAVE_AR;
    op2 = register_event_source(op1, op2);
    if (op2 == BP_ERROR) {
        char *str;
        extern char *eventNoNameTable[];
        str = eventNoNameTable[op1-ActionPerformed];
        RAISE_ISO_EXCEPTION(illegal_event, str, 2);
    } else {
        goto rr_trigger_var_dom;
    }

#ifndef GCC
case fetch_event_object:  /* y */
#endif
lab_fetch_event_object:
    op2 = (BPLONG)NextOperandY;
    op3 = AR_OUT(AR);
    if (TAG(op3) != ATM) {PUSHTRAIL_s(op2);}  /* required by GC */
    FOLLOW(op2) = op3;  /* set the event object */
    CONTCASE;


#ifndef GCC
case delay:  /* l,l,s,l */
#endif
lab_delay:
    if (FRAME_IS_START(AR)) {
        CONNECT_SUSP_FRAME;
        AR_STATUS(AR) = SUSP_RUN;
        AR_REEP(AR) = (BPLONG)P;
        P += 4;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)FOLLOW((P+3));
        CONTCASE;
    }

#ifndef GCC
case end_delay:  /* E */
#endif
lab_end_delay:
    if (FRAME_IS_START(AR)) {
    } else if (!FRAME_IS_CLONE(AR)) {
        /*
          if (AR>B) { 
          op1 = (BPLONG)AR_STATUS_ADDR(AR);
          PUSHTRAILC_ATOMIC(op1,FOLLOW(op1));
          }
        */
        AR_STATUS(AR) = SUSP_EXIT;
    }
    CONTCASE;

#ifndef GCC
case jmpn_dvar_y:  /* y,l */
#endif
lab_jmpn_dvar_y:
    op1 = NextOperandYC; DEREF(op1);
    if (IS_SUSP_VAR(op1)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
        if (!IS_UN_DOMAIN(dv_ptr)) {
            P++;
            CONTCASE;
        }
    }
    P = *(BPLONG_PTR *)P;
    CONTCASE;

#ifndef GCC
case jmpn_susp_var_y:  /* y,l */
#endif
lab_jmpn_susp_var_y:
    op1 = NextOperandYC; DEREF(op1);
    if (IS_SUSP_VAR(op1)) {
        P++;
        CONTCASE;
    } else {
        P = *(BPLONG_PTR *)P;
        CONTCASE;
    }

#ifndef GCC
case interval_consistent_eq:  /* i */
#endif
lab_interval_consistent_eq:
    op1 = NextOperandLiteral;
    SAVE_AR;
    op1 = nary_interval_consistent_eq(op1);
    if (op1 == 0) {
        BACKTRACK;
    } else {
        CONTCASE;
    }

#ifndef GCC
case interval_consistent_eq_nocoe:  /* i */
#endif
lab_interval_consistent_eq_nocoe:
    printf("interval_consistent_eq_nocoe not supported\n");
    exit(1);
    /*
      op1 = NextOperandLiteral;
      SAVE_AR;
      op1 = nary_interval_consistent_nocoe(op1);
      if (op1==0){
      BACKTRACK;
      } else {
      CONTCASE;
      }
    */


#ifndef GCC
case interval_consistent_ge:  /* i */
#endif
lab_interval_consistent_ge:
    op1 = NextOperandLiteral;
    SAVE_AR;
    op1 = nary_interval_consistent_ge(op1);
    if (op1 == 0) {
        BACKTRACK;
    } else {
        goto lab_return_delay;
    }

#ifndef GCC
case call_binary_constr_eq:  /* i */
#endif
lab_call_binary_constr_eq:
{
    BPLONG c, a1, x1, a2, x2, a, x;
    BPLONG_PTR oldP;
    BPLONG n, count;
    n = *P++; count = 0; oldP = P;

#define PARA_VALUE_DVAR(val) *LOCAL_TOP-- = UNTAGGED_TOPON_ADDR(val)
#define PARA_VALUE_INT(val) *LOCAL_TOP-- = val

    c = FOLLOW(AR+2*n+1); DEREF_NONVAR(c); c = INTVAL(c);
    for (i = 1; i <= n; i++) {
        a = FOLLOW(AR+n+i); DEREF_NONVAR(a);
        x = FOLLOW(AR+i); DEREF_NONVAR(x);
        if (a != BP_ZERO) {
            if (IS_SUSP_VAR(x)) {
                count++;
                if (count == 1) {
                    a1 = a;
                    x1 = x;
                } else {
                    a2 = a;
                    x2 = x;
                }
            } else {
                c += INTVAL(a)*INTVAL(x);
            }
        }
    }
    if (count == 2) {
        /*      write_term(a1); printf(" "); write_term(a2);printf("\n"); */
        if (a1 == BP_ONE) {
            if (a2 == BP_ONE) {
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(-c));
                P = (BPLONG_PTR) GET_EP(vv_eq_c_arc);
                goto call_binary_constr;
            } else if (a2 == BP_MONE) {  /* X = Y+C */
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(v_eq_vc_arc);
                goto call_binary_constr;
            }
        } else if (a1 == BP_MONE) {
            if (a2 == BP_ONE) {  /* X = Y+C */
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(v_eq_vc_arc);
                goto call_binary_constr;
            } else if (a2 == BP_MONE) {  /* X + Y =C */
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(vv_eq_c_arc);
                goto call_binary_constr;
            }
        }
        if (INTVAL(a1) > 0) {
            if (INTVAL(a2) > 0) {
                PARA_VALUE_INT(a1);
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_INT(a2);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(-c));
                P = (BPLONG_PTR) GET_EP(uu_eq_c_arc);
            } else {
                a2 = MAKEINT(-INTVAL(a2));
                PARA_VALUE_INT(a2);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(a1);
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(u_eq_uc_arc);
            }
        } else {
            if (INTVAL(a2) > 0) {
                a1 = MAKEINT(-INTVAL(a1));
                PARA_VALUE_INT(a1);
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_INT(a2);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(u_eq_uc_arc);
            } else {
                a1 = MAKEINT(-INTVAL(a1));
                a2 = MAKEINT(-INTVAL(a2));
                PARA_VALUE_INT(a1);
                PARA_VALUE_DVAR(x1);
                PARA_VALUE_INT(a2);
                PARA_VALUE_DVAR(x2);
                PARA_VALUE_INT(MAKEINT(c));
                P = (BPLONG_PTR) GET_EP(uu_eq_c_arc);
            }
        }
    call_binary_constr:
        *LOCAL_TOP = (BPLONG)AR;
        AR = LOCAL_TOP;
        AR_CPS(AR) = (BPLONG)oldP;
        CONTCASE;
    } else if (count == 1) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(x1);
        a1 = INTVAL(a1);
        c = -c;
        op2 = c/a1;
        if (op2*a1 == c && dm_true(dv_ptr, op2)) {
            op1 = x1;
            op2 = MAKEINT(op2);
            goto bind_suspvar_value;
        } else BACKTRACK;
    } else {
        if (c != 0) BACKTRACK; else CONTCASE;
    }
}

#ifndef GCC
case domain_next_inst_yyy:  /* y,y,y */
#endif
lab_domain_next_inst_yyy:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = NextOperandYC; DEREF_NONVAR(op2); op2 = INTVAL(op2);
    /* if (!IS_SUSP_VAR(op1)) BACKTRACK; must be a dvar */
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
    if (op2 >= DV_last(dv_ptr)) BACKTRACK;
    op3 = (BPLONG)NextOperandY;
    n_backtracks++;
    op2++;
    if (IS_BV_DOMAIN(dv_ptr)) {
        BPLONG_PTR bv_ptr, w_ptr;
        BPULONG w, offset, mask;
        bv_ptr = (BPLONG_PTR)DV_bit_vector_ptr(dv_ptr);
        BV_NEXT_IN(bv_ptr, op2, w, w_ptr, offset, mask);
    }
    FOLLOW(op3) = MAKEINT(op2);
    CONTCASE;

#ifndef GCC
case domain_set_false_yy:  /* y,y */
#endif
lab_domain_set_false_yy:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = NextOperandYC; DEREF_NONVAR(op2);
    if (!IS_SUSP_VAR(op1)) {  /* be sure to be a domain var */
        if (op1 == op2) {
            BACKTRACK;
        }
        CONTCASE;
    }
    if (!ISINT(op2)) {
        CONTCASE;
    }
    op2 = INTVAL(op2);

domain_set_false2:
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
    first = DV_first(dv_ptr);
    last = DV_last(dv_ptr);

    if (op2 > first && op2 < last) {
        if (IS_IT_DOMAIN(dv_ptr)) {
            it_to_bv_with_hole(dv_ptr, op2);
        } else {
            exclude_inner_elm_bv(dv_ptr, op2);
        }
        CONTCASE;
    }
    if (op2 == first) {
        op1 = DV_size(dv_ptr);
        if (op1 == 2) {
            BIND_DVAR_VALUE(dv_ptr, MAKEINT(last));
            CONTCASE;
        }
        INSERT_TRIGGER_outer_dom(dv_ptr, MAKEINT(op2));
        op2++;
        if (IS_IT_DOMAIN(dv_ptr)) {
        } else {
            op2 = domain_next_bv(dv_ptr, op2);
        }
        UPDATE_FIRST_SIZE(dv_ptr, first, op2, op1-1);
        CONTCASE;
    }
    if (op2 == last) {
        op1 = DV_size(dv_ptr);
        if (op1 == 2) {
            BIND_DVAR_VALUE(dv_ptr, MAKEINT(first));
            CONTCASE;
        }
        INSERT_TRIGGER_outer_dom(dv_ptr, MAKEINT(op2));
        op2--;
        if (IS_IT_DOMAIN(dv_ptr)) {
        } else {
            op2 = domain_prev_bv(dv_ptr, op2);
        }
        UPDATE_LAST_SIZE(dv_ptr, last, op2, op1-1);
        CONTCASE;
    }
    CONTCASE;


#ifndef GCC
case domain_min_max_yyy:  /* y,y,y */
#endif
lab_domain_min_max_yyy:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    if (IS_SUSP_VAR(op1)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        op1 = MAKEINT(DV_first(dv_ptr));
        op2 = *P++;
        if (op2 != 0) {  /* not void */
            YC(op2) = op1;
        }
        op1 = MAKEINT(DV_last(dv_ptr));
        op2 = *P++;
        if (op2 != 0) {  /* not void */
            YC(op2) = op1;
        }
        CONTCASE;
    } else {
        op2 = *P++;
        if (op2 != 0) {
            YC(op2) = op1;
        }
        op2 = *P++;
        if (op2 != 0) {
            YC(op2) = op1;
        }
        CONTCASE;
    }

#ifndef GCC
case domain_region:  /* y,z,z */
#endif
lab_domain_region:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = *P++; OP_NOVY_DEREF_NONVAR(op2);
    op3 = *P++; OP_NOVY_DEREF_NONVAR(op3);
    if (!ISINT(op2) || !ISINT(op3)) {  /* op2 and op3 may be big integers */
        CONTCASE;
    }

    op2 = INTVAL(op2); op3 = INTVAL(op3);

start_domain_region:
    /* printf("domain_region op1=%x (%d..%d)\n",op1,op2,op3); */
    if (op2 > op3) BACKTRACK;
    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (op1 >= op2 && op1 <= op3)
            CONTCASE;
        else
            BACKTRACK;
    }
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
    first = DV_first(dv_ptr); last = DV_last(dv_ptr);
    /*     printf("domain_region <%d %d> <%d %d>\n",op2,op3,first,last); */
    if (op2 > first) {
        if (op3 < last) {  /* update both bounds */
            if (op2 == op3) {
                if (!dm_true(dv_ptr, op2)) BACKTRACK;
                BIND_DVAR_VALUE(dv_ptr, MAKEINT(op2));
                CONTCASE;
            } else if (IS_IT_DOMAIN(dv_ptr)) {
                if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                    for (i = first; i < op2; i++) {
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(i));
                    }
                    for (i = op3+1; i <= last; i++) {
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(i));
                    }
                }
                arity = op3-op2+1;
            } else {
                if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                    arity = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr, first+1, op2-1);
                    arity += CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr, op3+1, last-1);
                    arity = DV_size(dv_ptr)-arity-2;
                    if (arity > 1) {
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(first));
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(last));
                    }
                } else {
                    arity = CALL_COUNT_DOMAIN_ELMS(dv_ptr, first+1, op2-1);
                    arity += CALL_COUNT_DOMAIN_ELMS(dv_ptr, op3+1, last-1);
                    arity = DV_size(dv_ptr)-arity-2;
                }
                if (arity == 0) BACKTRACK;
                op2 = domain_next_bv(dv_ptr, op2);
                if (arity == 1) {
                    BIND_DVAR_VALUE(dv_ptr, MAKEINT(op2));
                    CONTCASE;
                }
                op3 = domain_prev_bv(dv_ptr, op3);
            }
            UPDATE_FIRST_LAST_SIZE(dv_ptr, first, op2, last, op3, arity);
            INSERT_TRIGGER_minmax(dv_ptr);
            CONTCASE;
        } else {  /* update min, op2>first, op3 >= last*/
            if (op2 >= last) {
                if (op2 > last) BACKTRACK;
                BIND_DVAR_VALUE(dv_ptr, MAKEINT(op2));
                CONTCASE;
            } else if (IS_IT_DOMAIN(dv_ptr)) {
                if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                    for (i = first; i < op2; i++) {
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(i));
                    }
                }
                arity = last-op2+1;
            } else {
                if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                    arity = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr, first+1, op2-1);
                    arity = DV_size(dv_ptr)-arity-1;
                    if (arity > 1) {
                        INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(first));
                    }
                } else {
                    arity = CALL_COUNT_DOMAIN_ELMS(dv_ptr, first+1, op2-1);
                    arity = DV_size(dv_ptr)-arity-1;
                }
                if (arity == 1) {
                    BIND_DVAR_VALUE(dv_ptr, MAKEINT(last));
                    CONTCASE;
                }
                op2 = domain_next_bv(dv_ptr, op2);
            }
            UPDATE_FIRST_SIZE(dv_ptr, first, op2, arity);
            INSERT_TRIGGER_minmax(dv_ptr);
            CONTCASE;
        }
    } else if (op3 < last) {  /* op2 <= first, update max */
        if (op3 <= first) {
            if (op3 < first) BACKTRACK;
            BIND_DVAR_VALUE(dv_ptr, MAKEINT(op3));
            CONTCASE;
        } else if (IS_IT_DOMAIN(dv_ptr)) {
            if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                for (i = op3+1; i <= last; i++) {
                    INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(i));
                }
            }
            arity = op3-first+1;
        } else {
            if (DV_outer_dom_cs(dv_ptr) != nil_sym) {
                arity = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr, op3+1, last-1);
                arity = DV_size(dv_ptr)-arity-1;
                if (arity > 1) {
                    INSERT_TRIGGER_outer_dom0(dv_ptr, MAKEINT(last));
                }
            } else {
                arity = CALL_COUNT_DOMAIN_ELMS(dv_ptr, op3+1, last-1);
                arity = DV_size(dv_ptr)-arity-1;
            }
            if (arity == 1) {
                BIND_DVAR_VALUE(dv_ptr, MAKEINT(first));
                CONTCASE;
            }
            op3 = domain_prev_bv(dv_ptr, op3);
        }
        UPDATE_LAST_SIZE(dv_ptr, last, op3, arity);
        INSERT_TRIGGER_minmax(dv_ptr);
        CONTCASE;
    } else {
        CONTCASE;  /* no action */
    }




#ifndef GCC
case domain_region_min:  /* y,z */
#endif
lab_domain_region_min:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = *P++; OP_NOVY_DEREF_NONVAR(op2);
    if (!ISINT(op2)) {
        CONTCASE;
    }
    op2 = INTVAL(op2);
    op3 = BP_MAXINT_1W;
    goto start_domain_region;


#ifndef GCC
case domain_region_max:  /* y,z */
#endif
lab_domain_region_max:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = BP_MININT_1W;
    op3 = *P++; OP_NOVY_DEREF_NONVAR(op3);
    if (!ISINT(op3)) {
        CONTCASE;
    }
    op3 = INTVAL(op3);
    goto start_domain_region;

#ifndef GCC
case v_in_cv_int:  /* E */
#endif
lab_v_in_cv_int:
{BPLONG tmp_op;
        op1 = YC(3); DEREF_NONVAR(op1);  /* X */
        tmp_op = INTVAL(YC(2));  /* no dereference necessary, done in call_binary_constr_eq */
        op2 = YC(1); DEREF_NONVAR(op2);  /* Y */

        if (IS_SUSP_VAR(op2)) {
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
            op2 = tmp_op-DV_last(dv_ptr);
            op3 = tmp_op-DV_first(dv_ptr);
            goto start_domain_region;
        } else {
            TOAM_KILL_SUSP_FRAME;
            op2 = tmp_op - INTVAL(op2);
        unify_dvar_value:
            if (ISINT(op1)) {if (INTVAL(op1) == op2) CONTCASE; else BACKTRACK;}
            top = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
            if (op2 < DV_first(top) || op2 > DV_last(top)) BACKTRACK;
            if (IS_IT_DOMAIN(top)) ;
            else if (!dm_true(top, op2)) BACKTRACK;
            INSERT_TRIGGER_dvar_ins(top);
            PUSHTRAIL_H_NONATOMIC(top, op1);
            FOLLOW(top) = MAKEINT(op2);
            CONTCASE;
        }
}

#ifndef GCC
case v_in_vc_int:  /* E */
#endif
lab_v_in_vc_int:
    /* X in Y+C */
{BPLONG tmp_op;
    op1 = YC(3); DEREF_NONVAR(op1);  /* X */
    op2 = YC(2); DEREF_NONVAR(op2);  /* Y */
    tmp_op = INTVAL(YC(1));  /* no dereference necessary, done in call_binary_constr_eq */
    if (IS_SUSP_VAR(op2)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
        op2 = DV_first(dv_ptr)+tmp_op;
        op3 = DV_last(dv_ptr)+tmp_op;
        goto start_domain_region;
    } else {
        TOAM_KILL_SUSP_FRAME;
        op2 = tmp_op+INTVAL(op2);
        goto unify_dvar_value;
    }
}

#ifndef GCC
case u_in_cu_int:  /* E */
#endif
lab_u_in_cu_int:
    /* A*X in C-B*Y */
{
    BPLONG a, b, c, tmp_op;
    a = INTVAL(YC(5));  /* no dereference necessary, done in call_binary_constr_eq */
    op1 = YC(4); DEREF_NONVAR(op1);  /* X */
    c = INTVAL(YC(3));
    b = INTVAL(YC(2));
    op2 = YC(1); DEREF_NONVAR(op2);  /* Y */

    /*     printf("u_in_cu %d %lx %d %d %lx\n",a,op1,c,b,op2);  */
    if (IS_SUSP_VAR(op2)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
        tmp_op = c-b*DV_last(dv_ptr);
        UP_DIV(op2, tmp_op, a);
        tmp_op = c-b*DV_first(dv_ptr);
        LOW_DIV(op3, tmp_op, a);
        goto start_domain_region;
    } else {
        TOAM_KILL_SUSP_FRAME;
        tmp_op = c-b*INTVAL(op2);
        op2 = tmp_op/a;
        if (a*op2 != tmp_op) BACKTRACK;
        goto unify_dvar_value;
    }
}

#ifndef GCC
case u_in_uc_int:  /* E */
#endif
lab_u_in_uc_int:
    /* A*X in B*Y+C */
{BPLONG a, b, c, tmp_op;
    a = INTVAL(YC(5));  /* no dereference necessary, done in call_binary_constr_eq */
    op1 = YC(4); DEREF_NONVAR(op1);  /* X */
    b = INTVAL(YC(3));
    op2 = YC(2); DEREF_NONVAR(op2);  /* Y */
    c = INTVAL(YC(1));

    if (IS_SUSP_VAR(op2)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
        tmp_op = b*DV_first(dv_ptr)+c;
        UP_DIV(op2, tmp_op, a);
        tmp_op = b*DV_last(dv_ptr)+c;
        LOW_DIV(op3, tmp_op, a);
        goto start_domain_region;
    } else {
        TOAM_KILL_SUSP_FRAME;
        tmp_op = b*INTVAL(op2)+c;
        op2 = tmp_op/a;
        if (a*op2 != tmp_op) BACKTRACK;
        goto unify_dvar_value;
    }
}


#ifndef GCC
case u_eq_cu_dom:  /* y */
#endif
lab_u_eq_cu_dom:
{
    /*
      u_eq_cu_dom(A,X,C,B,Y):-
      T is C-B*Ey,
      Ex is T//A,
      (A*Ex=:=T->domain_set_false(X,Ex);true).
    */
    BPLONG a, b, c;
    op1 = YC(4); DEREF_NONVAR(op1);  /* X */
    if (!IS_SUSP_VAR(op1)) {P++; CONTCASE;}
    a = INTVAL(YC(5));
    c = INTVAL(YC(3));
    b = INTVAL(YC(2));
    op3 = INTVAL(YC(*P++));
    op3 = c-b*op3;
    op2 = op3/a;
    if (a*op2 == op3) goto domain_set_false2;
    CONTCASE;
}


#ifndef GCC
case u_eq_uc_dom:  /* y */
#endif
lab_u_eq_uc_dom:
{
    /*
      u_eq_uc_dom(A,X,B,Y,C):-
      T is B*Ey+C,
      Ex is T//A,
      (A*Ex=:=T->domain_set_false(X,Ex);true).
    */
    BPLONG a, b, c;

    op1 = YC(4); DEREF_NONVAR(op1);  /* X */
    if (!IS_SUSP_VAR(op1)) {P++; CONTCASE;}
    a = INTVAL(YC(5));
    b = INTVAL(YC(3));
    c = INTVAL(YC(1));
    op3 = INTVAL(YC(*P++));
    op3 = b*op3+c;
    op2 = op3/a;
    if (a*op2 == op3) goto domain_set_false2;
    CONTCASE;
}

#ifndef GCC
case bcp:  /* i */
#endif
lab_bcp:
{
    /* perform unit propagation on a clause: (A1, ..., An, V1,...,Vn) where Ai is 1 or -1 */
    BPLONG coe, var, op4;
    op1 = *P++;  /* num of vars in the cnf */
    op4 = 0;  /* count of vars */
    for (i = op1; i > 0; i--) {  /* if any literal is 1, then kill this propagator */
        op2 = FOLLOW(AR+i); DEREF_NONVAR(op2);
        op3 = FOLLOW(AR+i+op1); DEREF_NONVAR(op3);
        if (ISINT(op2)) {
            if (op2 == op3) {  /* both 1 or both 0*/
                /*
                  if (AR>B) { 
                  op1 = (BPLONG)AR_STATUS_ADDR(AR);
                  PUSHTRAILC_ATOMIC(op1,FOLLOW(op1));
                  } 
                */
                AR_STATUS(AR) = SUSP_EXIT;
                CONTCASE;
            }
        } else {
            op4++;
            var = op2;
            coe = op3;
        }
    }
    if (op4 == 1) {  /* unit clause */
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
        BIND_DVAR_VALUE(dv_ptr, coe);
        AR_STATUS(AR) = SUSP_EXIT;
        CONTCASE;
    } else if (op4 == 0) {
        BACKTRACK;
    }
    CONTCASE;
}


#ifndef GCC
case activate_first_agent:  /* y,y */
#endif
lab_activate_first_agent:
    op1 = NextOperandYC;  /* agent frame */
    op2 = NextOperandYC; DEREF(op2);  /* event object */
    P += 2;  /* skip noop1 40 */
    constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_ADDR(op1));
    CONNECT_WOKEN_FRAME_dom(constr_ar, op2);
    CONTCASE;

#ifndef GCC
case activate_agents_conjunction:  /* y,y,y */
#endif
lab_activate_agents_conjunction:
{
    BPLONG op4;

    op1 = NextOperandYC; DEREF(op1);  /* agent list 1 */
    op2 = NextOperandYC; DEREF(op2);  /* agent list 2 */
    op3 = NextOperandYC; DEREF(op3);  /* event object */
    P += 2;  /* skip noop1 40 */
    /*
      printf("activate_agents:\n");
      write_term(op1); printf("\n");
      write_term(op2); printf("\n\n");
    */
    op4 = op1;  /* mark list 1 */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) | TOP_BIT);  /* mark it */
        op4 = FOLLOW(sreg+1);
    }
    op4 = op2;  /* intersect */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        if (AR_STATUS(constr_ar) & TOP_BIT_MASK) {  /* also in list op1 */
            AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) & ~TOP_BIT);  /* unmark this one here */
            CONNECT_WOKEN_FRAME_dom(constr_ar, op3);
        }
        op4 = FOLLOW(sreg+1);
    }
    op4 = op1;  /* unmark list 1 */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) & ~TOP_BIT);
        op4 = FOLLOW(sreg+1);
    }
    CONTCASE;
}


#ifndef GCC
case activate_agents_disjunction:  /* y,y,y */
#endif
lab_activate_agents_disjunction:
{BPLONG op4;
    op1 = NextOperandYC; DEREF(op1);  /* agent list 1 */
    op2 = NextOperandYC; DEREF(op2);  /* agent list 2 */
    op3 = NextOperandYC; DEREF(op3);  /* event object */
    P += 2;  /* skip noop1 40 */
    /*
      printf("activate_agents:\n");
      write_term(op1); printf("\n");
      write_term(op2); printf("\n\n");
    */
    op4 = op2;  /* mark list 2 */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) | TOP_BIT);  /* mark it */
        op4 = FOLLOW(sreg+1);
    }
    op4 = op1;  /* activate those agents on list1 that are not in list 2 */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        if (AR_STATUS(constr_ar) & TOP_BIT_MASK) {  /* also in list op2 */
        } else {
            CONNECT_WOKEN_FRAME_dom(constr_ar, op3);
        }
        op4 = FOLLOW(sreg+1);
    }
    op4 = op2;  /* activate and unmark list 2 */
    while (ISLIST(op4)) {
        sreg = (BPLONG_PTR)UNTAGGED_ADDR(op4);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(sreg)));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) & ~TOP_BIT);
        CONNECT_WOKEN_FRAME_dom(constr_ar, op3);
        op4 = FOLLOW(sreg+1);
    }
    CONTCASE;
}

#ifndef GCC
case table_allocate:  /* i,i,s,i */
#endif
lab_table_allocate:
    /* The table mode and cardinality are stored as operands of a table_mode instruction:
       table_allocate Arity,Size,Sym,MaxS
       table_mode CallMode,OptArg,CardNTArg
    */
{
    BPLONG_PTR master_ar, ptr;
    BPLONG answer_table, mode_bits, nt_last_arg, last_arg, limit, old_limit;

    op1 = *P++;  /* arity */
    op2 = *P++;  /* frame size */
    sym_ptr = (SYM_REC_PTR)*P++;  /* Predicate symbol */
    P++;  /* skip MaxS */

    AR_BTM(AR) = ADDTAG((AR+op1), TABLE_FRAME_TAG);
    LOCAL_TOP = AR - op2;

    CATCH_INTERRUPT;

    AR_TOP(AR) = (BPLONG)LOCAL_TOP;

    AR_B(AR) = (BPLONG)B;
    AR_H(AR) = (BPLONG)H;
    AR_T(AR) = (BPLONG)T;
    AR_SF(AR) = (BPLONG)SF;
    B = AR;
    HB = H;
    AR_CPF(AR) = (BPLONG)addr_table_consume;
    //  AR_TABLE_NEW_BITS(AR) = 0;

    INITIALIZE_STACK_VARS(TABLE_FRAME_SIZE);
    AR_SUBGOAL_TABLE(AR) = (BPLONG)NULL;
    AR_CURR_ANSWER(AR) = (BPLONG)NULL;
    INVOKE_GC_NONDET;

    SAVE_AR;
    mode_bits = *(P+1);  /* the CallMode word */
    nt_last_arg = (FOLLOW(P+3) >= (1 << 27)) ? 1 : 0;  /* the CardNTArg word */
    if (nt_last_arg == 1) {  /* ignore the last argument */
        last_arg = FOLLOW(AR+1);
        FOLLOW(AR+1) = (BPLONG)(AR+1);
    }
    subgoal_entry = lookupSubgoalTable(AR+op1, op1, sym_ptr, mode_bits, nt_last_arg);
    if ((BPLONG)subgoal_entry == BP_ERROR) goto table_error;
    if (nt_last_arg == 1) {  /* restore the last argument */
        FOLLOW(AR+1) = last_arg;
    }

    SET_AR_SUBGOAL_TABLE(AR, (BPLONG)subgoal_entry);

    answer_table = GT_ANSWER_TABLE(subgoal_entry);
    if (answer_table == (BPLONG)NULL) {
        AR_CURR_ANSWER(AR) = (BPLONG)NULL;
    } else if ((BPLONG)answer_table & 0x1) {  /* has only one answer */
        AR_CURR_ANSWER(AR) = UNTAGGED_ADDR((BPLONG)answer_table);
    } else {
        AR_CURR_ANSWER(AR) = GT_ANSWER_TABLE_FIRST0(subgoal_entry);
    }

    master_ar = (BPLONG_PTR)GT_TOP_AR(subgoal_entry);
    if ((BPLONG)master_ar == SUBGOAL_COMPLETE) {
        if (sym_ptr == plan_psc) goto check_plan_resource_limit;
        goto lab_table_consume;
    } else if (master_ar == NULL) {  /* Master */
        if (sym_ptr == plan_psc) {  /* the last arg is iplan(Limit,Plan,PlanCost) */
            DEREF(last_arg);
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(last_arg);
            limit = FOLLOW(ptr+1);
            DEREF(limit);
            ptr = GT_ARG_ADDR(subgoal_entry);
            FOLLOW(ptr+op1-1) = limit;
        }
        GT_TOP_AR(subgoal_entry) = (BPLONG)AR;
        //    AR_TABLE_NEW_BITS(AR) = 0x1L;

        SUBGOAL_START_NORMAL(subgoal_entry);
        CONTCASE;
    } else if ((BPLONG)master_ar == SUBGOAL_TEMP_COMPLETE) {  /* has been evaluated at least once overall */
        if (sym_ptr == plan_psc) goto check_plan_resource_limit;
        if (SUBGOAL_EVALUATED(subgoal_entry)) {  /* has been evaluated at least once in this round */
            goto lab_table_consume;
        }
        GT_TOP_AR(subgoal_entry) = (BPLONG)AR;

        SUBGOAL_START_ITERATION(subgoal_entry);
        CONTCASE;
    } else {  /* descendent of master_ar */
        SET_SUBGOAL_LOOPING(subgoal_entry);
        goto lab_table_consume;
    }

check_plan_resource_limit:
    DEREF(last_arg);  /* the last arg is '_$iplan'(Limit,Plan,PlanCost) */
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(last_arg);
    limit = FOLLOW(ptr+1);
    DEREF_NONVAR(limit);
    ptr = GT_ARG_ADDR(subgoal_entry);
    ptr += (op1-1);  /* pointer to the last argument of the tabled subgoal */
    old_limit = FOLLOW(ptr);

    if (INTVAL(limit) > INTVAL(old_limit)) {
        FOLLOW(ptr) = limit;
        GT_TOP_AR(subgoal_entry) = (BPLONG)AR;
        GT_SCC_ELMS(subgoal_entry) = (BPLONG)NULL;
        GT_SCC_ROOT(subgoal_entry) = (BPLONG)subgoal_entry;
        SUBGOAL_START_NORMAL(subgoal_entry);
        CONTCASE;
    } else {
        goto table_consume_fail;
    }

table_error:
    RAISE_ISO_EXCEPTION(bp_exception, GET_NAME(sym_ptr), GET_ARITY(sym_ptr));
}


#ifndef GCC
case table_produce:  /* E */
#endif
lab_table_produce:
    /* The table modes and cardinality are stored as operands of a table_mode instruction
        
       table_allocate Arity,Size,Sym,MaxS
       table_mode CallMode,OptArg,CardNTArg
         
       let p be the pointer to the beginning of the table_allocate instruction.
       the opcode of the next instruction after table_allocate is FOLLOW(p+5). 
       If the instruction is table_mode, then OptArg can be retrieved 
       as FOLLOW(p+7) and CardNTArg as FOLLOW(p+8). In CardNTArg, the 27th bit
       from bottom indicates if the last argument is nt (not-tabled) and the bits
       from 1-26 are for the cardinality limit.
    */
{
    BPLONG_PTR answer_table, answer, table_arg_ptr, stack_arg_ptr;
    BPLONG_PTR loc_p;
    BPLONG table_card, nt_last_arg, tmp;
    int opt_arg_index, maximize;

    subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(AR);
    sym_ptr = (SYM_REC_PTR)GT_SYM(subgoal_entry);
    arity = GET_ARITY(sym_ptr);
    answer_table = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
    stack_arg_ptr = AR+arity;

    loc_p = (BPLONG_PTR)GET_EP(sym_ptr);
    opt_arg_index = FOLLOW(loc_p+7);
    tmp = FOLLOW(loc_p+8);
    table_card = (tmp & ~(1 << 27));  /* the 27th bit indicates if the last argument is not-tabled */
    nt_last_arg = (table_card == tmp) ? 0 : 1;
    if (nt_last_arg == 1) {
        BPLONG_PTR ar_1 = AR+1;
        PUSHTRAILC_ATOMIC(ar_1, FOLLOW(ar_1));
        FOLLOW(ar_1) = (BPLONG)ar_1;
    }

    if (answer_table == NULL) {  /* no answer yet */
        answer = addFirstTableAnswer(stack_arg_ptr, arity);
        if ((BPLONG)answer == BP_ERROR) goto table_error;
        GT_ANSWER_TABLE(subgoal_entry) = ((BPLONG)answer | 0x1);  /* the TAG indicates that it's an answer, not an answer table */
        AR_CURR_ANSWER(AR) = (BPLONG)answer;
        SET_SUBGOAL_ANS_REVISED(subgoal_entry);
        BACKTRACK;
    }

    if ((BPLONG)answer_table & 0x1) {  /* has one answer */
        answer = (BPLONG_PTR)UNTAGGED_ADDR(answer_table);

        if (opt_arg_index == 0) {
            if (table_card == 1) {  /* the current tabled call is complete now */
                /*
                  sreg = AR;
                  ROLL_CHOICE_POINTS(sreg);
                  B = AR;
                  HB = (BPLONG_PTR)AR_H(B);                               
                  if (SF>AR) LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);  
                */
                BACKTRACK;
            }
            /* otherwise, add the first answer */
            answer_table = allocateAnswerTable(answer, arity);
            if ((BPLONG)answer_table == BP_ERROR) goto table_error;
            GT_ANSWER_TABLE(subgoal_entry) = (BPLONG)answer_table;
            if (addTableAnswer(stack_arg_ptr, arity, subgoal_entry) == BP_ERROR)
                goto table_error;
            BACKTRACK;
        } else {
            if ((opt_arg_index & 0xffffL) == 0) {  /* maximize this argument */
                opt_arg_index >>= 16;
                maximize = 1;
            } else {
                maximize = 0;
            }
            opt_arg_index--;
            if (table_card == 1) {
                table_arg_ptr = ANSWER_ARG_ADDR(answer);
                op1 = FOLLOW(stack_arg_ptr-opt_arg_index); DEREF(op1);
                op2 = FOLLOW(table_arg_ptr+opt_arg_index);
                if ((maximize == 0 && TABLE_ANS_IS_LT(op1, op2)) ||
                    (maximize == 1 && TABLE_ANS_IS_GT(op1, op2))) {
                    BPLONG hcode;
                    PREPARE_NUMBER_TERM(0);
                    if (numberVarCopyAnswerArgsToTableArea(stack_arg_ptr, ANSWER_ARG_ADDR(answer), arity, &hcode) == BP_ERROR)  /* replace the old answer */
                        goto table_error;
                    SET_SUBGOAL_ANS_REVISED(subgoal_entry);
                }
                BACKTRACK;
            } else {
                answer_table = allocateAnswerTable(answer, arity);
                if ((BPLONG)answer_table == BP_ERROR) goto table_error;
                GT_ANSWER_TABLE(subgoal_entry) = (BPLONG)answer_table;
                if (addTableOptimalAnswer(stack_arg_ptr, arity, subgoal_entry, opt_arg_index, maximize, table_card) == BP_ERROR)
                    goto table_error;
                AR_CURR_ANSWER(AR) = ANSWERTABLE_FIRST(answer_table);
            }
        }
        BACKTRACK;
    }
    /* has two or more answers */
    answer_table = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);

    if (opt_arg_index == 0) {
        if (table_card == ANSWERTABLE_COUNT(answer_table)) {BACKTRACK;}
        if (addTableAnswer(stack_arg_ptr, arity, subgoal_entry) == BP_ERROR)
            goto table_error;
    } else {
        if ((opt_arg_index & 0xffffL) == 0) {  /* maximize this argument */
            opt_arg_index >>= 16;
            maximize = 1;
        } else {
            maximize = 0;
        }
        opt_arg_index--;
        if (addTableOptimalAnswer(stack_arg_ptr, arity, subgoal_entry, opt_arg_index, maximize, table_card) == BP_ERROR)
            goto table_error;
        AR_CURR_ANSWER(AR) = ANSWERTABLE_FIRST(answer_table);
    }
    BACKTRACK;
}


#ifndef GCC
case table_consume:  /* E */
#endif
lab_table_consume:
    /* The table modes and cardinality are stored as operands of a table_mode instruction
        
       table_allocate Arity,Size,Sym,MaxS
       table_mode CallMode,OptArg,CardNTArg
    */
{
    BPLONG_PTR stack_arg_ptr, table_arg_ptr, answer, loc_p;
    int nt_last_arg;

    subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(AR);
    answer = (BPLONG_PTR)AR_CURR_ANSWER(AR);
    if (answer == NULL) goto table_consume_fail;

    sym_ptr = (SYM_REC_PTR)GT_SYM(subgoal_entry);
    arity = GET_ARITY(sym_ptr);

    AR_CURR_ANSWER(AR) = ANSWER_NEXT_IN_TABLE(answer);

    stack_arg_ptr = AR+arity;
    table_arg_ptr = ANSWER_ARG_ADDR(answer);
    loc_p = (BPLONG_PTR)GET_EP(sym_ptr);
    nt_last_arg = (FOLLOW(loc_p+8) >= (1 << 27)) ? 1 : 0;  /* the CardNTArg word */
    if (nt_last_arg == 1) arity--;  /* ignore the last argument */

    PREPARE_UNNUMBER_TERM(LOCAL_TOP);
    for (i = 0; i < arity; i++) {
        op1 = FOLLOW(stack_arg_ptr-i);
        op2 = FOLLOW(table_arg_ptr+i);
        if (op1 != op2) {
            match_term_tabledTerm(op1, op2);
        }
    }
    toam_LOCAL_OVERFLOW_CHECK_SMALL_MARGIN(10);

    if (SUBGOAL_IS_COMPLETE(subgoal_entry) && AR_CURR_ANSWER(AR) == (BPLONG)NULL) {
        CUT;
        RETURN_DET;
    } else {
        RETURN_NONDET;
    }

    CATCH_INTERRUPT;
    CONTCASE;

table_consume_fail:
    CALL_PROPAGATE_SCC_ROOT;
    goto rr_cut_fail;
}


#ifndef GCC
case table_check_completion:  /* l */
#endif
lab_table_check_completion:
    subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(AR);
    /*
      printf("check_comp %s\n",GET_NAME((SYM_REC_PTR)GT_SYM(subgoal_entry)));
      printTable();
      printf("\n");
    */
    if (AR_IS_SCC_ROOT(AR, subgoal_entry)) { /* the root of the current SCC */
        if (!SUBGOAL_IS_LOOPING(subgoal_entry) || !SUBGOAL_ANS_IS_REVISED(subgoal_entry)) {
            /*
              printf("COMPLETE "); print_subgoal_entry_only(subgoal_entry); 
              if (SUBGOAL_IS_LOOPING(subgoal_entry)) printf("LOOPING"); else printf("NOLOOPING");printf("\n");
            */
            complete_scc_elms(subgoal_entry);
            AR_CPF(AR) = (BPLONG)addr_table_consume;

            goto lab_table_consume;
        } else {  /* needs to be iterated */
            //    AR_TABLE_NEW_BITS(AR) = 0;
            initialize_scc_elms(subgoal_entry);

            SUBGOAL_START_ITERATION(subgoal_entry);
            // printf("recompute =%lx\n",subgoal_entry);  print_subgoal_entry_only(subgoal_entry);printf("\n");
            P = (BPLONG_PTR)*P;
            CONTCASE;
        }
    } else {  /* not scc root */
        if ((BPLONG_PTR)GT_SCC_ROOT(subgoal_entry) != subgoal_entry) {  /* bug fixed on Sep. 16, 2015 */
            GT_TOP_AR(subgoal_entry) = SUBGOAL_TEMP_COMPLETE;  /* has been evaluated at least once, overall */
        }
        SET_SUBGOAL_EVALUATED(subgoal_entry);
        AR_CPF(AR) = (BPLONG)addr_table_consume;

        goto lab_table_consume;
    }


#ifndef GCC
case table_neck_no_reeval:  /* E */
#endif
lab_table_neck_no_reeval:
    subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(AR);
    if (SUBGOAL_IS_ITERATION(subgoal_entry)) {
        BACKTRACK;
    }
    CONTCASE;

#ifndef GCC
case last_tabled_call:  /* E */
#endif
lab_last_tabled_call:
    CONTCASE;

#ifndef GCC
case table_cut:  /* l */
#endif
lab_table_cut:
    ROLL_CHOICE_POINTS(AR);
    B = AR;
    HB = (BPLONG_PTR)AR_H(B);
    AR_CPF(AR) = *P++;
    if (SF > AR) LOCAL_TOP = (BPLONG_PTR)AR_TOP(AR);
    CATCH_WAKE_EVENT;
    CONTCASE;

#ifndef GCC
case table_neck:  /* E */
#endif
lab_table_neck:
    /* not used */
    CONTCASE;


#ifndef GCC
case table_eager_consume:  /* E */
#endif
lab_table_eager_consume:
    printf("eager consume not supported anymore\n");
    /*
      SET_AR_EAGER_CONSUMER(AR);
      AR_CPF(AR) = (BPLONG)(P-1);
      sreg = (BPLONG_PTR)AR_CURR_ANSWER(AR);
      if (sreg==NULL) CONTCASE;
      if ((BPLONG_PTR)ANSWER_NEXT_IN_TABLE(sreg)==NULL) CONTCASE;
      goto lab_table_consume;
    */
    CONTCASE;

#ifndef GCC
case table_set_new_bit:  /* i */
#endif
lab_table_set_new_bit:
    printf("semi-naive not supported\n");
    /*
      op1 = NextOperandLiteral;
      #ifdef NO_SEMI_OPT
      #else
      op2 = AR_TABLE_NEW_BITS(AR);
      op2 = op2 & ((((unsigned int)FFFF)<<(NBITS_IN_LONG-op1)) >> (NBITS_IN_LONG-op1));
      if (table_flag==1){ 
      AR_TABLE_NEW_BITS(AR) = (op2 | (0x1L<<op1));
      } else {
      AR_TABLE_NEW_BITS(AR) = op2;
      }
      #endif
    */
    P++;
    CONTCASE;


#ifndef GCC
case jmpn_unif:  /* z,z,l */
#endif
lab_jmpn_unif:
    op1 = *P++; OP_ZC_DEREF(op1);
    op2 = *P++; OP_ZC_DEREF(op2);
    if (op1 == op2) {
        P++;
        CONTCASE;
    } else if ((TAG(op1) == ATM && (TAG(op2) == ATM || ISCOMPOUND(op2))) ||
               (TAG(op2) == ATM && ISCOMPOUND(op1))) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else if (is_UNIFIABLE(op1, op2)) {
        unify(op1, op2);
        P++;
        CONTCASE;
    } else {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    }

#ifndef GCC
case jmp_unif:  /* z,z,l */
#endif
lab_jmp_unif:
    op1 = *P++; OP_ZC(op1);
    op2 = *P++; OP_ZC(op2);

    if (is_UNIFIABLE(op1, op2)) {
        P = (BPLONG_PTR)*P;
        CONTCASE;
    } else {
        P++;
        CONTCASE;
    }

#ifndef GCC
case arg_no_chk:  /* y,y,y */
#endif
lab_arg_no_chk:
    op1 = YC(*P++); DEREF_NONVAR(op1); op1 = INTVAL(op1);
    op2 = YC(*P++); DEREF_NONVAR(op2); op2 = UNTAGGED_ADDR(op2);
    sreg = Y(*P++);
    PUSHTRAIL_s(sreg);
    FOLLOW(sreg) = FOLLOW((BPLONG_PTR)op2 + op1);
    CONTCASE;


#ifndef GCC
case setarg0_no_chk:  /* i,y,y */
#endif
lab_setarg0_no_chk:
    op1 = *P++;
    op2 = YC(*P++); DEREF_NONVAR(op2);
    op3 = YC(*P++); DEREF_NONVAR(op3);
    sreg = ((BPLONG_PTR)UNTAGGED_ADDR(op2)+op1);
    PUSHTRAIL_H_NONATOMIC(sreg, FOLLOW(sreg));
    FOLLOW(sreg) = op3;
    CONTCASE;



#ifndef GCC
case bcp1:  /* i */
#endif
lab_bcp1:
{
    /* perform unit propagation on a clause: (V1,...,Vn) */
    BPLONG var, op4;
    op1 = *P++;  /* num of vars in the cnf */
    op4 = 0;  /* count of vars */
    for (i = op1; i > 0; i--) {  /* if any literal is 1, then kill this propagator */
        op2 = FOLLOW(AR+i); DEREF_NONVAR(op2);
        if (op2 == BP_ONE) {  /* satisfied */
            /*
              if (AR>B) { 
              op1 = (BPLONG)AR_STATUS_ADDR(AR);
              PUSHTRAILC_ATOMIC(op1,FOLLOW(op1));
              } 
            */
            AR_STATUS(AR) = SUSP_EXIT;
            CONTCASE;
        } else if (op2 != BP_ZERO) {  /* it is a var */
            op4++;
            var = op2;
        }
    }
    if (op4 == 1) {  /* unit clause */
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
        BIND_DVAR_VALUE(dv_ptr, BP_ONE);
        AR_STATUS(AR) = SUSP_EXIT;
        CONTCASE;
    } else if (op4 == 0) {
        BACKTRACK;
    }
    CONTCASE;
}


#ifndef GCC
case domain_nogood_region:  /* y,z,z */
#endif
lab_domain_nogood_region:
    op1 = NextOperandYC; DEREF_NONVAR(op1);
    op2 = *P++; OP_NOVY_DEREF_NONVAR(op2);
    op3 = *P++; OP_NOVY_DEREF_NONVAR(op3);
    op2 = INTVAL(op2); op3 = INTVAL(op3);

    if (ISINT(op1)) {
        op1 = INTVAL(op1);
        if (op1 >= op2 && op1 <= op3) BACKTRACK; else CONTCASE;
    } else {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
        if (op2 <= DV_first(dv_ptr)) {
            op2 = op3+1; op3 = DV_last(dv_ptr);
            goto start_domain_region;
        } else if (op3 >= DV_last(dv_ptr)) {
            op3 = op2-1;
            op2 = DV_first(dv_ptr);
            goto start_domain_region;
        } else {
            if (IS_IT_DOMAIN(dv_ptr)) {
                SAVE_AR; SAVE_TOP;
                it_to_bv_with_interval_holes(dv_ptr, op2, op3);
            } else {
                exclude_inner_interval_bv(dv_ptr, op2, op3);
            }
            CONTCASE;
        }
    }

#ifndef GCC
case endfile:  /* l */
#endif
lab_endfile:
    P++;
    CONTCASE;


#ifndef GCC
case tabsize:  /* i */
#endif
lab_tabsize:
    CONTCASE;

#ifndef GCC
case table_mode:  /* i,i,i */
#endif
lab_table_mode:
    P += 3;
    CONTCASE;

#ifndef GCC
case asp_decode:  /* y,y,i,ys(-1) */
#endif
lab_asp_decode:
    printf("asp_decode no longer exists\n");
    exit(0);

#ifndef GCC
case asp_add_tuple:  /* y,i,ys(-1) */
#endif
lab_asp_add_tuple:
    printf("asp_add_tuple no longer exists\n");
    exit(0);


#ifndef GCC
case set_catcher_frame:  /* E */
#endif
lab_set_catcher_frame:
    AR_BTM(AR) = (AR_BTM(AR) | CATCHER_FRAME_TAG);
    CONTCASE;

#ifndef GCC
case filter_clauses:  /* y,y,y,y */
#endif
lab_filter_clauses:
    /* Cls, Key, CallTime, NCls */
    op1 = NextOperandYC;  /* Cls, no dereference needed */
    op2 = NextOperandYC;  /* Key */
    op3 = NextOperandYC;  /* CallTime */
    sreg = NextOperandY;  /* NCls */
    op3 = INTVAL(op3);
    while (ISLIST(op1)) {
        BPLONG_PTR lst_ptr;
        InterpretedClausePtr clause_record_ptr;
        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
        clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(FOLLOW(lst_ptr));
        if (clause_record_ptr->birth_time_stamp > op3 || op3 >= clause_record_ptr->death_time_stamp) {
            op1 = FOLLOW(lst_ptr+1);
            continue;  /* this clause is not in the logical view for the call */
        }
        if (op2 != BP_ZERO) {
            BPLONG cl_head, arg1, arg1_key;
            cl_head = clause_record_ptr->head;  /* struct(CellRef,Head,Body,Birth,Death) */
            arg1 = FOLLOW((BPLONG_PTR)UNTAGGED_ADDR(cl_head)+1);  /* cl_head must be a structure, o.w., Key==0 */
            BP_HASH_KEY1(arg1, arg1_key, lab_filter_clauses_1);
            if (arg1_key != BP_ZERO && op2 != arg1_key) {
                op1 = FOLLOW(lst_ptr+1);
                continue;  /* this clause has a different key */
            }
        }
        FOLLOW(sreg) = op1;
        CONTCASE;
    }
    FOLLOW(sreg) = nil_sym;
    CONTCASE;

#ifndef GCC
}
#endif
