/********************************************************************
 *   File   : gcStack.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024
 *   Purpose: stack garbage collector

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>
#include <time.h>
#include "bprolog.h"
#include "frame.h"
#include "event.h"
#include "dynamic.h"
#include "gc.h"
#include "inst.h"

BPLONG_PTR copy_local_top;  /* stack in gc area */
BPLONG_PTR b_top;  /* top of the latest choice point */
BPLONG_PTR marked_susp_var_ptr;
int n_marked_susp_vars;

#define SP_AFTER_GC(sp) (b_top-((BPULONG)copy_area_high-(BPULONG)sp)/sizeof(BPLONG))
#define POINTER_TO_COPY_AREA(p) ((BPLONG_PTR)p >= copy_area_low && (BPLONG_PTR)p <= copy_area_high)

BYTE copy_area_allocated = 0;
BPLONG no_gcs = 0;
BPLONG gc_time = 0;

BPLONG_PTR max_heap_mark;

int number_of_gcs() {
    BPLONG n = ARG(1, 1);
    return unify(n, MAKEINT(no_gcs));
}

int garbage_collector()
{
    extern BPLONG cputime();
    BPLONG msec0;

    msec0 = cputime();

    if (toam_signal_vec != 0 || in_critical_region != 0) return BP_TRUE;

    gc_is_working = 1;

    //  printf("==>GC local_top=%x heap_top=%x b(%x)->h =%x, hbreg=%x\n",local_top,heap_top,breg,AR_H(breg),hbreg);
    // show_ar_chain_upto_b(arreg);
    //  check_susp_frames_reep("=>GC");
    /*
      show_ar_chain_upto_b(arreg);
    */
    /* show_ar_chain(arreg); */
    gc_initialize_ar_chain();

    if (sfreg < breg)
        if (gc_globalize_sf_chain() == BP_ERROR) return BP_ERROR;
    no_gcs++;
    /* printf("=>packTrail\n"); */
    packEntireTrail();
    /* printf("=>eliDup\n"); */
    if (eliminateDuplicatedTrailInTopSegment() == BP_ERROR) return BP_ERROR;
    /* printf("=>stack\n"); */
    if (gcStack() == BP_ERROR) return BP_ERROR;
    /* printf("=>heap\n"); */
    if (gcHeap() == BP_ERROR) return BP_ERROR;

    gc_is_working = 0;
    //  check_susp_frames_reep("<=GC");
    //  printf("<==GC local_top=%x heap_top=%x\n",local_top,heap_top);
    //  show_ar_chain_upto_b(arreg);
    //  show_ar_chain(arreg);
    gc_time += (cputime()-msec0);
    return BP_TRUE;
}

void gc_initialize_ar_chain() {
    BPLONG_PTR cp, f;
    int n, b_initialized;

    b_initialized = 0;

    NO_RESERVED_SLOTS(arreg, n);
    f = arreg;
    for (; ; ) {
        if (f <= breg) {
            if (f == breg) b_initialized = 1;
            initialize_frame(f, n);
        }
        if (f == (BPLONG_PTR)AR_AR(f)) break;
        cp = (BPLONG_PTR)AR_CPS(f);
        n = *(cp-1);
        f = (BPLONG_PTR)AR_AR(f);
    }

    if (b_initialized == 0) {
        cp = (BPLONG_PTR)AR_CPF(breg);
        n = *(cp-1);
        initialize_frame(breg, n);
    }
}

/*
  Before dead suspension frames are garbage collected,  gc_globalize_sf_chain() 
  globalizes local variables in them so that no other stack slots reference them.
*/
int gc_globalize_sf_chain() {
    BPLONG_PTR f;
    BPLONG mask_size;

    mask_size = ((BPULONG)breg-(BPULONG)local_top)/NBITS_IN_LONG+2;  /* masking bits */
    if (allocateMaskArea(mask_size) == BP_ERROR) return BP_ERROR;

    mark_stack_references_ar_chain();
    mark_stack_references_sf_chain();

    f = sfreg;
    while (f < breg) {
        if (FRAME_IS_DEAD(f)) {
            globalize_stack_vars_in_frame(f, SUSP_FRAME_SIZE);
        }
        f = (BPLONG_PTR)AR_PREV(f);
    }
    return BP_TRUE;
}

void mark_stack_references_ar_chain() {
    BPLONG_PTR f;
    f = arreg;
    while (f != (BPLONG_PTR)AR_AR(f)) {
        if (f < breg) {
            mark_stack_references_frame(f);
        }
        f = (BPLONG_PTR)AR_AR(f);
    }
}

void mark_stack_references_sf_chain() {
    BPLONG_PTR f;
    f = sfreg;
    while (f < breg) {
        if (!FRAME_IS_DEAD(f)) {
            mark_stack_references_frame(f);
        }
        f = (BPLONG_PTR)AR_PREV(f);
    }
}

void mark_stack_references_frame(BPLONG_PTR f)
{
    BPLONG size, term;
    BPLONG_PTR sp, top;

    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    while (sp > f) {
        term = FOLLOW(sp);
        if (ISREF(term) && (BPLONG_PTR)term > local_top && (BPLONG_PTR)term < breg) {
            GCSetMaskBit(term, local_top);
        }
        sp--;
    }

    NO_RESERVED_SLOTS(f, size);
    sp = f-size;
    top = (BPLONG_PTR)AR_TOP(f);
    while (sp > top) {
        term = FOLLOW(sp);
        if (ISREF(term) && (BPLONG_PTR)term > local_top && (BPLONG_PTR)term < breg) {
            GCSetMaskBit(term, local_top);
        }
        sp--;
    }
}

void globalize_stack_vars_in_frame(BPLONG_PTR f, BPLONG size)
{
    BPLONG_PTR sp, top;

    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    while (sp > f) {
        if (FOLLOW(sp) == (BPLONG)sp && gcIsMarked(sp, local_top) == 1) {
            FOLLOW(heap_top) = FOLLOW(sp) = (BPLONG)heap_top;
            heap_top++;
            if (heap_top >= local_top) myquit(STACK_OVERFLOW, "gc");
        }
        sp--;
    }

    sp = f-size;
    top = (BPLONG_PTR)AR_TOP(f);
    while (sp > top) {
        if (FOLLOW(sp) == (BPLONG)sp && gcIsMarked(sp, local_top) == 1) {
            FOLLOW(heap_top) = FOLLOW(sp) = (BPLONG)heap_top;
            heap_top++;
            if (heap_top >= local_top) myquit(STACK_OVERFLOW, "gc");
        }
        sp--;
    }
}

int allocateCopyArea(BPLONG size)
{
    if (copy_area_allocated == 0) {
        size = (size > 1000000 ? size : 1000000);
        BP_MALLOC(copy_area_low, size);
        if (copy_area_low == NULL) return BP_ERROR;
        copy_area_high = copy_area_low+size-1;
        /*  printf("new block allocated %x %x words\n",copy_area_low,copy_area_high); */
        copy_area_allocated = 1;
    } else {  /* reuse already allocated copy area */
        if (size <= (BPLONG)((BPULONG)copy_area_high-(BPULONG)copy_area_low)/sizeof(BPLONG)) {
            return BP_TRUE;
        } else {
            /*      BPLONG doubleSize = 2*(((BPULONG)copy_area_high-(BPULONG)copy_area_low)/sizeof(BPLONG)); */
            free(copy_area_low);
            /* size = size>doubleSize ? size : doubleSize; */
            BP_MALLOC(copy_area_low, size);
            if (copy_area_low == NULL) return BP_ERROR;

            copy_area_high = copy_area_low+size-1;
        }
    }
    return BP_TRUE;
}

int gcStack()
{
    BPLONG size, mask_size;

    /* preparation */
    b_top = (BPLONG_PTR)AR_TOP(breg);
    mask_size = ((BPULONG)heap_top-(BPULONG)stack_low_addr)/NBITS_IN_LONG+2;  /* masking bits */

    if (allocateMaskArea(mask_size) == BP_ERROR) return BP_ERROR;

    size = ((BPULONG)UNTAGGED_ADDR(AR_BTM(breg)) - (BPULONG)local_top)/sizeof(BPLONG);
    if (allocateCopyArea(size) == BP_ERROR) return BP_ERROR;

    copy_local_top = copy_area_high;

    gcInitDynamicArray();
    if (sfreg < breg) {
        gcStackMarkSuspVars();
        gcTrailMarkSuspVars();
    }
    marked_susp_var_ptr = gcDynamicArray;
    n_marked_susp_vars = gcDynamicArrayCount;

    /* gcStackMarkTriggeredCs(); */

    gcQueueInit;  /* store all the suspension frames to be moved out */
    sfreg = gcMoveAliveFramesOutSf();
    arreg = gcMoveAliveFramesOutAr();
    gcResetSuspVars();

    /*  gcDisposeDynamicArray(); */
    /* gcResetTriggeredCs(); */
    copyStackBack();
    return BP_TRUE;
}

void copyStackBack() {
    BPLONG size = ((BPULONG)copy_area_high-(BPULONG)copy_local_top)/sizeof(BPLONG);
    local_top = b_top-size;
    my_memcpy_btm_up(local_top+1, copy_local_top+1, size);
}

/* move a frame slot into temp area */
void gcMoveFrameSlotOut(BPLONG term, BPLONG_PTR des)
{

start:
    /*  printf("move frame term %x des=%x\n",term,SP_AFTER_GC(des));  */
    /*
      if (TAG(term)!=ATM && (UNTAGGED_ADDR(term)>=heap_top && (UNTAGGED_ADDR(term)<=local_top))){
      printf("wrong ref=%x heap_top=%x local_top=%x\n",term,heap_top,local_top);
      bookkeep_inst_print();
      quit("dangling reference\n");
      }
    */
    if (ISREF(term)) {
        if ((BPULONG)term <= (BPULONG)b_top && (BPULONG)term > (BPULONG)heap_top) {  /* stack ref below breg */
            if (!ISFREE(term)) {
                /* if (cycle(term)){printf("term=%x\n",term); quit("cycle found");} */
                term = FOLLOW(term);  /* dereference */
                goto start;
            } else {
                FOLLOW(des) = (BPLONG)SP_AFTER_GC(des);
                FOLLOW(term) = (BPLONG)des;
                /*
                  FOLLOW(des) = FOLLOW(term) = (BPLONG)heap_top; FOLLOW(heap_top) = (BPLONG)heap_top; heap_top++;  globalize it 
                  if (heap_top>=local_top) quit("Stack overflow during GC\n");
                */
                return;
            }
        } else if (POINTER_TO_COPY_AREA(term)) {
            FOLLOW(des) = (BPLONG)SP_AFTER_GC(term);
            return;
        } else {
            FOLLOW(des) = term;
            return;
        }
    }
    /*  printf(" out: ");write_term(term);printf("\n"); */
    if (IS_SUSP_VAR(term)) {
        FOLLOW(des) = UNTAGGED_TOPON_ADDR(term);
    } else {
        FOLLOW(des) = term;
    }
}

/* move src_f to the copy area */
BPLONG_PTR gcMoveFrameOut(BPLONG_PTR src_f, BPLONG nReservedSlots)
{
    BPLONG_PTR src_sp, des_sp, des_f, top;
    BPLONG frame_type;

    /*  printf("moveFrameOut %x => %x (%x,%x)\n",src_f,SP_AFTER_GC(copy_local_top),UNTAGGED_ADDR(AR_BTM(src_f)),AR_TOP(src_f)); */
    des_sp = copy_local_top;
    frame_type = (AR_BTM(src_f) & TAG_MASK);
    src_sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(src_f));
    while (src_sp > src_f) {
        gcMoveFrameSlotOut(FOLLOW(src_sp), des_sp);
        src_sp--; des_sp--;
    }

    des_f = des_sp;

    /*   printf("move frame from %x to %x\n", src_f, SP_AFTER_GC(des_sp)); */
    AR_B(des_f) = AR_B(src_f);  /* if it is a FLAT_B_FRAME */

    if (nReservedSlots >= NONDET_FRAME_SIZE) {
        AR_SF(des_f) = AR_SF(src_f);
        AR_H(des_f) = AR_H(src_f);
        AR_T(des_f) = AR_T(src_f);
        AR_CPF(des_f) = AR_CPF(src_f);
    }

    src_sp = src_f-nReservedSlots;
    des_sp = des_f-nReservedSlots;
    top = (BPLONG_PTR)AR_TOP(src_f);
    while (src_sp > top) {
        gcMoveFrameSlotOut(FOLLOW(src_sp), des_sp);
        src_sp--; des_sp--;
    }

    AR_BTM(des_f) = ADDTAG((BPLONG)SP_AFTER_GC(copy_local_top), frame_type);
    AR_TOP(des_f) = (BPLONG)(SP_AFTER_GC(des_sp));
    copy_local_top = des_sp;
    return des_f;
}

/* move the suspension frames chain sfreg into the temp area */
BPLONG_PTR gcMoveAliveFramesOutSf()
{
    BPLONG_PTR f, f1, new_sf;
    BPLONG tmp;

    f = sfreg;
    while (f < breg) {
        GCQueueAdd(f, 0);
        f = (BPLONG_PTR)AR_PREV(f);
    }
    /* f be the latest susp frame older than B */
    new_sf = f;
    while (gcQueueCount > 0) {
        GCQueueGetFromRear(f, tmp);
        if (!FRAME_IS_DEAD(f)) {
            f1 = gcMoveFrameOut(f, SUSP_FRAME_SIZE);
            AR_REEP(f1) = AR_REEP(f);
            AR_STATUS(f1) = AR_STATUS(f);
            /*      AR_PRIORITY(f1) = AR_PRIORITY(f); */
            AR_OUT(f1) = AR_OUT(f);
            AR_PREV(f1) = (BPLONG)new_sf;
            AR_PREV(f) = (BPLONG)f1;  /* remember the place where f was moved to */
            new_sf = SP_AFTER_GC(f1);
        } else {
            /*   printf("DEAD (%x)",f);display_constraint(f);  */
        }
    }
    return new_sf;
}

/* reverse AR chain */
BPLONG_PTR gcReverseArChain(BPLONG_PTR f, BPLONG_PTR prev)
{
    BPLONG_PTR tmp;

    while (f != prev) {
        tmp = (BPLONG_PTR)AR_AR(prev);
        AR_AR(prev) = (BPLONG)f;
        f = prev;  /* return gcReverseArChain(prev,tmp); */
        prev = tmp;
    }
    return f;
}

/*
  Move out frames on the AR chain that are younger than breg.
  prev -> f -> ....
  f1 refers to the frame originally pointed to by f after the fame is moved out to 
  the copy area, new_prev refers to the frame of f after it is moved back from the 
  copy area.
*/

BPLONG_PTR gcMoveAliveFramesOutReversedAr(BPLONG_PTR prev, BPLONG_PTR f)
{
    BPLONG_PTR f1, new_f, new_prev;
    BPLONG no;

start:
    new_f = (BPLONG_PTR)AR_AR(f);
    if (f >= breg) {
        f1 = f;
        new_prev = f;
    } else if (!IS_SUSP_FRAME(f)) {
        NO_RESERVED_SLOTS(f, no);
        f1 = gcMoveFrameOut(f, no);
        AR_CPS(f1) = AR_CPS(f);
        new_prev = SP_AFTER_GC(f1);
    } else if (FRAME_IS_DEAD(f) || FRAME_IS_START(f) || FRAME_IS_CLONE(f)) {  /* impossible to be start ?*/
        f1 = gcMoveFrameOut(f, SUSP_FRAME_SIZE);
        AR_CPS(f1) = AR_CPS(f);
        AR_REEP(f1) = AR_REEP(f);
        AR_STATUS(f1) = AR_STATUS(f);
        /* AR_PRIORITY(f1) = AR_PRIORITY(f); */
        AR_OUT(f1) = AR_OUT(f);
        new_prev = SP_AFTER_GC(f1);
    } else {  /* susp frame has been moved out already */
        f1 = (BPLONG_PTR)AR_PREV(f);
        new_prev = SP_AFTER_GC(f1);
        AR_CPS(f1) = AR_CPS(f);
    }
    AR_AR(f1) = (BPLONG)prev;
    if (f == arreg) return new_prev;
    prev = new_prev;  /* return gcMoveAliveFramesOutReversedAr(new_prev,new_f); */
    f = new_f;
    goto start;
}

/* move the activation frames chain arreg into the temp area */
BPLONG_PTR gcMoveAliveFramesOutAr()
{
    BPLONG_PTR f, next;

    f = gcReverseArChain(arreg, (BPLONG_PTR)AR_AR(arreg));
    next = (BPLONG_PTR)AR_AR(f);
    AR_AR(f) = (BPLONG)f;  /* head */
    return gcMoveAliveFramesOutReversedAr(f, next);
}

/**************************************************************
  Reset cs lists (offsets of frame pointers) on suspension variables. Before garbage-collecting
  the stack, gcStackMarkSuspVars collects all those suspension variables into a dynamic array that 
  contain cs list elements younger than B. Before moving stack frames back to the control stack, 
  gcResetSuspVars adjust the frame pointers on the suspension variables. No frame pointers can be 
  reset more than once. To ensure that, we mark all the frame pointers before resetting them.

  gcStackMarkSuspVars scan the suspension frames younger than B for suspension variables that  
  may have suspension frames younger than B attached to them. 

  gcTrailMarkSuspVars scans the trailed items in the top segment of the trail stack for old suspension 
  variables that may have suspension frames younger than B attached to them. 

  Since no gc is invoked when there are events raised, it is unnecessary to collect 
  suspension variables in triggered constraints. 
******************************************************************/
void gcStackMarkSuspVars() {
    BPLONG_PTR f, sp;

    f = sfreg;
    while (f < breg) {
        sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
        while (sp > f) {
            gcMarkSuspVarsTerm(FOLLOW(sp));
            sp--;
        }
        /*
          sp = f-SUSP_FRAME_SIZE;
          top = (BPLONG_PTR)AR_TOP(f);
          while (sp>top){
          gcMarkSuspVarsTerm(FOLLOW(sp));
          sp--;
          }
        */
        f = (BPLONG_PTR)AR_PREV(f);
    }
}

void gcTrailMarkSuspVars() {
    BPLONG_PTR curr_t, addr, last_t;

    curr_t = trail_top+1;
    last_t = (BPLONG_PTR)AR_T(breg);
    while (curr_t < last_t) {
        addr = (BPLONG_PTR)FOLLOW(curr_t);
        if (TAG(addr) == TRAIL_VAR) {
            gcMarkSuspVarsTerm(FOLLOW(addr));
        } else if (TAG(addr) == TRAIL_VAL_NONATOMIC) {
            addr = (BPLONG_PTR)UNTAGGED3((BPLONG)addr);
            /*      printf("addr = %x FOLLOW(addr)=%x\n",addr,FOLLOW(addr)); */
            gcMarkSuspVarsTerm(FOLLOW(addr));
        }
        curr_t += 2;
    }
}

void gcMarkSuspVarsTerm(BPLONG term)
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR ptr;
    BPLONG_PTR top;

cont:
    DEREF(term);
    if (IS_SUSP_VAR(term)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
        if (gcIsMarked(dv_ptr, stack_low_addr) == 0) {
            gcSetMask(dv_ptr, 1, stack_low_addr);
            gcAddDynamicArray((BPLONG)dv_ptr);
            gcMarkSuspVarsCs(DV_ins_cs(dv_ptr));
            gcMarkSuspVarsCs(DV_minmax_cs(dv_ptr));
            gcMarkSuspVarsCs(DV_dom_cs(dv_ptr));
            gcMarkSuspVarsCs(DV_outer_dom_cs(dv_ptr));
            gcMarkSuspVarsTerm(DV_attached(dv_ptr));
        }
    } else if (ISLIST(term)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        if (IS_HEAP_REFERENCE(ptr) && gcIsMarked(ptr, stack_low_addr) == 0) {
            gcSetMask(ptr, 2, stack_low_addr);
            gcMarkSuspVarsTerm(FOLLOW(ptr));
            term = FOLLOW(ptr+1);
            goto cont;
        }
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        if (IS_HEAP_REFERENCE(ptr) && gcIsMarked(ptr, stack_low_addr) == 0) {
            arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
            gcSetMask(ptr, arity+1, stack_low_addr);
            for (i = 1; i < arity; i++) {
                gcMarkSuspVarsTerm(FOLLOW(ptr+i));
            }
            term = FOLLOW(ptr+arity);
            goto cont;
        }
    }
}

void gcMarkSuspVarsCs(BPLONG cs)
{
    BPLONG_PTR sf, ptr;
    BPLONG constr;
    while (ISLIST(cs)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs);  /* untag LST */
        constr = FOLLOW(ptr);  /* car */
        sf = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(constr));
        if (sf < breg && ISINT(constr)) {
            FOLLOW(ptr) = ADDTAG(((BPULONG)stack_up_addr-(BPULONG)sf), ATM);  /* mark it by turning the tag from INT to ATM */
        }
        cs = FOLLOW(ptr+1);  /* cdr */
    }
}

void gcResetSuspVars() {
    BPLONG_PTR dv_ptr;
    int i;

    for (i = 0; i < n_marked_susp_vars; i++) {
        dv_ptr = (BPLONG_PTR)FOLLOW(marked_susp_var_ptr+i);
        gcResetCs(A_DV_ins_cs(dv_ptr), DV_ins_cs(dv_ptr));
        gcResetCs(A_DV_minmax_cs(dv_ptr), DV_minmax_cs(dv_ptr));
        gcResetCs(A_DV_dom_cs(dv_ptr), DV_dom_cs(dv_ptr));
        gcResetCs(A_DV_outer_dom_cs(dv_ptr), DV_outer_dom_cs(dv_ptr));
    }
}

void gcResetTriggeredCs()
{
    BPLONG i;

    for (i = 1; i <= trigger_no; i++) {
        gcResetCs((BPLONG_PTR)&triggeredCs[i], (BPLONG)triggeredCs[i]);
    }
}

void gcResetCs(BPLONG_PTR addr, BPLONG list)
{
    BPLONG_PTR sf, des_sf, ptr;
    BPLONG constr;

start:
    if (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        constr = FOLLOW(ptr);  /* car */
        sf = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(constr));  /* frame pointer */
        if (sf < breg && ISATOM(constr)) {
            if (FRAME_IS_DEAD(sf)) {  /* the frame has been collected as garbage */
                list = FOLLOW(ptr+1);
                goto start;
            }
            des_sf = (BPLONG_PTR)AR_PREV(sf);  /* it was moved here */
            FOLLOW(ptr) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)SP_AFTER_GC(des_sf), INT_TAG);
        }
        FOLLOW(addr) = list;
        addr = ptr+1;
        list = FOLLOW(addr);
        goto start;
    }
    FOLLOW(addr) = nil_sym;
}

int b_ENABLE_GC() {
    bp_gc = old_bp_gc;
    return 1;
}

int b_DISABLE_GC() {
    old_bp_gc = bp_gc;
    bp_gc = 0;
    return 1;
}

int c_GET_GC_TIME() {
    BPLONG Value = ARG(1, 1);
    return unify(Value, MAKEINT(gc_time));
}
