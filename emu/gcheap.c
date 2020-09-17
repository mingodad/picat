/********************************************************************
 *   File   : gcHeap.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: heap garbage collector

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

/*
  #define DEBUG_GC  
  #define CONSOLE
*/
#include "bprolog.h"
#include "frame.h"
#include "event.h"
#include "gc.h"

#define ADDR_AFTER_GC(addr) (hbreg+((BPULONG)addr-(BPULONG)copy_area_low)/sizeof(BPLONG))
#define GC_IS_MOVED(ptr) ((BPULONG)FOLLOW(ptr) >= (BPULONG)copy_area_low && (BPULONG)FOLLOW(ptr) < (BPULONG)copy_h)

extern BPLONG_PTR copy_area_low;
extern BPLONG_PTR copy_area_high;

BPLONG_PTR copy_h;

/**************************************************************
  Move alive heap cells (between hbreg and heap_top) accessible from 
  stack frames and the trail to the temp area. Postpone moving 
  free variables until all structures and lists are  copied.
*************************************************************/
int gcHeap() {
    BPLONG size;

    size = ((BPULONG)heap_top-(BPULONG)hbreg)/sizeof(BPLONG)+2;
    if (allocateCopyArea(size) == BP_ERROR) return BP_ERROR;

    gcInitDynamicArray();
    copy_h = copy_area_low;

    gcRescueBFrame(breg);
    gcRescueArFrames(arreg);
    gcRescueSfFrames(sfreg);
    /*  gcRescueTriggeredCs(); */
    gcRescueTrail();
    gcRescueFreeVars();

    /*  gcDisposeDynamicArray(); */

    copyHeapBack();
    return BP_TRUE;
}

/* An item may be doubly trailed even if it is TRAIL_VAR. */

int eliminateDuplicatedTrailInTopSegment() {
    BPLONG trail_top_region_size;
    trail_top_region_size = ((BPULONG)AR_T(breg)-(BPULONG)trail_top)/sizeof(BPLONG);

    /* printf("trail_top_region_size=%d\n",trail_top_region_size); */
    if (trail_top_region_size < 200) {
        eliminateDuplicatedTrailInTopSegmentSquare();
        return BP_TRUE;
    } else {
        return eliminateDuplicatedTrailInTopSegmentLinear();
    }
}

/*
  Eliminate duplicated trail items in the top segment before GC 
  O(n^2) may be too slow
*/
void eliminateDuplicatedTrailInTopSegmentSquare() {
    BPLONG op;
    BPLONG_PTR trail_top0, untaggedAddr, curr_t;

    trail_top0 = trail_top;  /* original trail top */
    curr_t = trail_top = (BPLONG_PTR)AR_T(breg);

    while (curr_t > trail_top0) {
        op = FOLLOW(curr_t-1);
        untaggedAddr = (BPLONG_PTR)UNTAGGED3(op);
        if (TAG(op) == TRAIL_VAR && untaggedAddr < (BPLONG_PTR)AR_TOP(sfreg)) {
            /* can't be trailed twice */
        } else if (alreadyTrailed(untaggedAddr, (BPLONG_PTR)AR_T(breg), curr_t+1)) {
            curr_t -= 2;
            continue;
        }
        *trail_top-- = FOLLOW(curr_t);
        *trail_top-- = (BPLONG)op;
        curr_t -= 2;
    }
}

/* linear-time algorithm
 */
int eliminateDuplicatedTrailInTopSegmentLinear() {
    BPLONG op, mask_size;
    BPLONG_PTR trail_top0, untaggedAddr, curr_t;
    BPLONG_PTR last_checked_addr = 0;

    trail_top0 = trail_top;  /* original trail top */
    curr_t = trail_top = (BPLONG_PTR)AR_T(breg);
    mask_size = ((BPULONG)AR_H(breg)-(BPULONG)stack_low_addr)/NBITS_IN_LONG+2;  /* masking bits */
    /* printf("mask_size=%d\n",mask_size); */

    if (allocateMaskArea(mask_size) == BP_ERROR) return BP_ERROR;  /* mask bits*/

    while (curr_t > trail_top0) {
        op = FOLLOW(curr_t-1);
        untaggedAddr = (BPLONG_PTR)UNTAGGED3(op);
        if (IS_HEAP_REFERENCE(untaggedAddr)) {
            if (gcIsMarked(untaggedAddr, stack_low_addr) == 1) {
                curr_t -= 2;
                continue;
            } else {
                GCSetMaskBit(untaggedAddr, stack_low_addr);
            }
        } else if (TAG(op) == TRAIL_VAL_ATOMIC || (TAG(op) == TRAIL_VAR && untaggedAddr < (BPLONG_PTR)AR_TOP(sfreg))) {
            /* doesn't hurt to retain it */
        } else if (last_checked_addr == untaggedAddr) {
            curr_t -= 2;
            continue;
        } else {
            /* printf("addr=%x stack_low_addr=%x heap_top=%x local_top=%x\n",op,stack_low_addr,heap_top,local_top); */
            last_checked_addr = untaggedAddr;
            if (alreadyTrailed(untaggedAddr, (BPLONG_PTR)AR_T(breg), curr_t+1)) {
                curr_t -= 2;
                continue;
            }
        }

        /* preserve it */
        *trail_top-- = FOLLOW(curr_t);
        *trail_top-- = (BPLONG)op;
        curr_t -= 2;
    }
    return BP_TRUE;
}

/*
  For each trail segment of a choice point, if the trailed address is not older 
  than the choice point, then remove it
*/
void packEntireTrail() {
    BPLONG_PTR f, f1, tmp_trail_top, curr_t, untaggedAddr;
    BPLONG op;

    gcQueueInit;  /* store all the choice point frames */
    f = breg; f1 = (BPLONG_PTR)AR_B(f);
    do {
        GCQueueAdd(f, (BPLONG)trail_top);
        trail_top = (BPLONG_PTR)AR_T(f);
        f = f1; f1 = (BPLONG_PTR)AR_B(f);
    } while (f != f1);

    trail_top = trail_up_addr;

    while (gcQueueCount > 0) {
        BPLONG cont;
        GCQueueGetFromRear(f, cont);
        tmp_trail_top = (BPLONG_PTR)cont;  /* items between f->T and tmp_trail_top form a segment for f */
        curr_t = (BPLONG_PTR)AR_T(f);
        AR_T(f) = (BPLONG)trail_top;  /* new f->T after packing */
        while (curr_t > tmp_trail_top) {
            op = FOLLOW(curr_t-1);
            untaggedAddr = (BPLONG_PTR)UNTAGGED3(op);
            if (untaggedAddr < f && untaggedAddr >= (BPLONG_PTR)AR_H(f)) {
                /* an item that should have been removed after cut */
            } else {  /* preserve it */
                *trail_top-- = FOLLOW(curr_t);
                *trail_top-- = (BPLONG)op;
            }
            curr_t -= 2;
        }
    }
}

/* check whether addr is already trailed */
int alreadyTrailed(addr, from_ptr, to_ptr)
    BPLONG_PTR addr, from_ptr, to_ptr;
{
    while (from_ptr > to_ptr) {
        if ((BPLONG_PTR)UNTAGGED3(FOLLOW(to_ptr)) == addr) return 1;
        to_ptr += 2;
    }
    return 0;
}

void copyHeapBack() {

    BPLONG size = ((BPULONG)copy_h-(BPULONG)copy_area_low)/sizeof(BPLONG);
    my_memcpy_btm_up(hbreg, copy_area_low, size);
    heap_top = hbreg+size;
}

void gcRescueFreeVars() {
    BPLONG_PTR ptr;
    BPLONG term;
    BPLONG_PTR addr;
    int count = gcDynamicArrayCount;
    ptr = gcDynamicArray;
    while (count > 0) {
        term = FOLLOW(ptr++);
        addr = (BPLONG_PTR)FOLLOW(ptr++);
        gcRescueFreeVar(addr, term);
        count -= 2;
    }
}

void gcRescueBFrame(ar)
    BPLONG_PTR ar;
{
    BPLONG no;
    BPLONG_PTR ptr, top;
    NO_RESERVED_SLOTS(ar, no);
    ptr = ar-no;
    top = (BPLONG_PTR)AR_TOP(ar);
    while (ptr > top) {
        gcRescueTerm(ptr, FOLLOW(ptr));
        ptr--;
    }
}

void gcRescueArFrames(ar)
    BPLONG_PTR ar;
{
    BPLONG no;

    /* because the chain is not chrononogical, some frames that are younger than B can only be
       reached through frames that are old the B */
    while ((BPLONG)ar != AR_AR(ar)) {
        if (ar < breg) {
            if (!IS_SUSP_FRAME(ar)) {
                NO_RESERVED_SLOTS(ar, no);
                gcRescueFrame(ar, no);
            } else if (FRAME_IS_DEAD(ar) || FRAME_IS_START(ar) || FRAME_IS_CLONE(ar)) {
                gcRescueFrame(ar, SUSP_FRAME_SIZE);
                gcRescueTerm(AR_OUT_ADDR(ar), AR_OUT(ar));
            }
        }
        ar = (BPLONG_PTR)AR_AR(ar);
    }
}

void gcRescueSfFrames(sf)
    BPLONG_PTR sf;
{

    while (sf < breg) {
        if (!FRAME_IS_DEAD(sf)) {
            gcRescueFrame(sf, SUSP_FRAME_SIZE);
            gcRescueTerm(AR_OUT_ADDR(sf), AR_OUT(sf));
        }
        sf = (BPLONG_PTR)AR_PREV(sf);
    }
}

void gcRescueFrame(f, noReservedSlots)
    BPLONG_PTR f;
    BPLONG noReservedSlots;
{
    BPLONG_PTR sp, top;

    /* move out arguments */
    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    while (sp > f) {
        gcRescueTerm(sp, FOLLOW(sp));
        sp--;
    }

    /* move out local variables */
    sp = f-noReservedSlots;
    top = (BPLONG_PTR)AR_TOP(f);
    while (sp > top) {
        gcRescueTerm(sp, FOLLOW(sp));
        sp--;
    }
}

void gcRescueTrail() {
    BPLONG_PTR curr_t, addr, last_t;

    curr_t = trail_top+1;
    last_t = (BPLONG_PTR)AR_T(breg);
    while (curr_t < last_t) {
        addr = (BPLONG_PTR)FOLLOW(curr_t);
        if (TAG(addr) == TRAIL_VAR) {
            /* printf("v(%x (%x))\n",addr,FOLLOW(addr)); */
            gcRescueTerm(addr, FOLLOW(addr));
        } else if (TAG(addr) == TRAIL_VAL_NONATOMIC) {
            addr = (BPLONG_PTR)UNTAGGED3((BPLONG)addr);
            /* printf("n(%x (%x))\n",addr,FOLLOW(addr)); */
            gcRescueTerm(addr, FOLLOW(addr));
        } else if (TAG(addr) == TRAIL_BIT_VECTOR) {
            addr = (BPLONG_PTR)UNTAGGED3((BPLONG)addr);
            /* printf("b(%x (%x))\n",addr,FOLLOW(addr)); */
            FOLLOW(addr) = gcRescueBitVector((BPLONG_PTR)FOLLOW(addr));
        }
        curr_t += 2;
    }
}

void gcRescueFreeVar(addr, term)
    BPLONG_PTR addr;
    BPLONG term;
{
    if (GC_IS_MOVED(term)) {
        FOLLOW(addr) = (BPLONG)ADDR_AFTER_GC(FOLLOW(term));
        return;
    } else {
        BPLONG tmp = (BPLONG)ADDR_AFTER_GC(copy_h);
        FOLLOW(copy_h) = tmp;
        FOLLOW(addr) = tmp;
        FOLLOW(term) = (BPLONG)copy_h; copy_h++;
        return;
    }
}

void gcRescueTriggeredCs() {
    BPLONG i;

    for (i = 1; i <= trigger_no; i++) {
        gcRescueTerm((BPLONG_PTR)&triggeredCs[i], (BPLONG)triggeredCs[i]);
    }
}

BPLONG gcRescueBitVector(bv_ptr)
    BPLONG_PTR bv_ptr;
{
    BPLONG_PTR ptr;
    BPLONG from, to;
    BPLONG i;
    BPLONG_PTR des_ptr = copy_h;

    from = BV_low_val(bv_ptr); FOLLOW(copy_h++) = from;
    to = BV_up_val(bv_ptr); FOLLOW(copy_h++) = to;
    ptr = BV_base_ptr(bv_ptr);

    for (i = from; i <= to; i += NBITS_IN_LONG) {
        FOLLOW(copy_h) = FOLLOW(ptr);
        copy_h++; ptr++;
    }
    return (BPLONG)ADDR_AFTER_GC(des_ptr);
}

void gcRescueTerm(addr, term)
    BPLONG_PTR addr;
    BPLONG term;
{
    BPLONG_PTR ptr, des_ptr;
    BPLONG arity, i;

    if (TAG(term) == ATM) {  /* int or atom */
        FOLLOW(addr) = term;
        return;
    }
    gcQueueInit;
    GCQueueAdd(addr, term);

loop:
    while (gcQueueCount > 0) {
        GCQueueGet(addr, term);
        /*    printf("rescueTerm %x %x\n",addr,term); */
    start:
        while (ISREF(term)) {
            if (INSIDE_HEAP_TOP_SEGMENT((BPLONG_PTR)term)) {
                if (GC_IS_MOVED(term)) {
                    FOLLOW(addr) = (BPLONG)ADDR_AFTER_GC(FOLLOW(term));
                    goto loop;
                } else if (!ISFREE(term)) {
                    term = FOLLOW(term);
                    goto start;
                    /*
                      FOLLOW(addr) = (BPLONG)ADDR_AFTER_GC(copy_h);
                      addr = copy_h; tmp = FOLLOW(term);
                      FOLLOW(term) = (BPLONG)copy_h; copy_h++;
                      term = tmp;
                      goto start;
                    */
                } else {  /* free noninternal var, postpone copying it */
                    gcAddDynamicArray(term);
                    gcAddDynamicArray((BPLONG)addr);
                    goto loop;
                }
            } else {
                FOLLOW(addr) = term;
                goto loop;
            }
        }
        if (TAG(term) == ATM) {
            FOLLOW(addr) = term;  /* need to reset it because dereference */
            goto loop;
        } else if (ISLIST(term)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            if (INSIDE_HEAP_TOP_SEGMENT(ptr)) {
                if (GC_IS_MOVED(ptr)) {
                    FOLLOW(addr) = ADDTAG(ADDR_AFTER_GC(FOLLOW(ptr)), LST);
                    goto loop;
                } else {
                    BPLONG tmp;
                    BPLONG_PTR des_ptr;
                    des_ptr = copy_h;
                    copy_h += 2;
                    tmp = FOLLOW(ptr);
                    if (TAG(tmp) == ATM) {
                        FOLLOW(des_ptr) = tmp;
                    } else {
                        GCQueueAdd(des_ptr, tmp);
                    }
                    FOLLOW(ptr) = (BPLONG)des_ptr;
                    tmp = FOLLOW(ptr+1);
                    if (TAG(tmp) == ATM) {
                        FOLLOW(des_ptr+1) = tmp;
                    } else {
                        GCQueueAdd(des_ptr+1, tmp);
                    }
                    FOLLOW(ptr+1) = (BPLONG)(des_ptr+1);
                    FOLLOW(addr) = ADDTAG(ADDR_AFTER_GC(des_ptr), LST);
                    goto loop;
                }
            } else {
                FOLLOW(addr) = term;
                goto loop;
            }
        } else if (ISSTRUCT(term)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            if (INSIDE_HEAP_TOP_SEGMENT(ptr)) {
                if (GC_IS_MOVED(ptr)) {
                    FOLLOW(addr) = ADDTAG(ADDR_AFTER_GC(FOLLOW(ptr)), STR);
                    goto loop;
                } else {
                    /*
                      printf("move struct %x sym_ptr=%x\n",term,FOLLOW(ptr)); 
                      if (FOLLOW(ptr)>parea_up_addr || FOLLOW(ptr)<parea_low_addr){
                      quit("STRANGE\n");
                      }
                    */
                    arity = GET_ARITY((SYM_REC_PTR)(FOLLOW(ptr)));
                    des_ptr = copy_h; copy_h += (arity+1);
                    FOLLOW(des_ptr) = FOLLOW(ptr); FOLLOW(ptr) = (BPLONG)des_ptr;
                    /* printf("sym_ptr=%x after\n",FOLLOW(ptr)); */
                    for (i = 1; i <= arity; i++) {
                        BPLONG tmp;
                        tmp = FOLLOW(ptr+i);
                        if (TAG(tmp) == ATM) {
                            FOLLOW(des_ptr+i) = tmp;
                        } else {
                            GCQueueAdd(des_ptr+i, FOLLOW(ptr+i));
                        }
                        FOLLOW(ptr+i) = (BPLONG)(des_ptr+i);
                    }
                    FOLLOW(addr) = ADDTAG(ADDR_AFTER_GC(des_ptr), STR);
                    goto loop;
                }
            } else {
                FOLLOW(addr) = term;
                goto loop;
            }
        } else {  /* susp var */
            ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
            if (INSIDE_HEAP_TOP_SEGMENT(ptr)) {
                if (GC_IS_MOVED(ptr)) {
                    FOLLOW(addr) = (BPLONG)ADDR_AFTER_GC(FOLLOW(ptr));
                    goto loop;
                }
                des_ptr = copy_h; copy_h += SIZE_OF_DV;
                FOLLOW(ptr) = (BPLONG)des_ptr;
                DV_var(des_ptr) = ADDTAG(ADDR_AFTER_GC(des_ptr), SUSP);
                DV_type(des_ptr) = DV_type(ptr);
                GCQueueAdd(A_DV_attached(des_ptr), DV_attached(ptr));
                DV_size(des_ptr) = DV_size(ptr);
                DV_first(des_ptr) = DV_first(ptr);
                DV_last(des_ptr) = DV_last(ptr);
                GCQueueAdd(A_DV_ins_cs(des_ptr), DV_ins_cs(ptr));
                GCQueueAdd(A_DV_minmax_cs(des_ptr), DV_minmax_cs(ptr));
                GCQueueAdd(A_DV_dom_cs(des_ptr), DV_dom_cs(ptr));
                GCQueueAdd(A_DV_outer_dom_cs(des_ptr), DV_outer_dom_cs(ptr));
                if (IS_BV_DOMAIN(ptr)) {
                    DV_bit_vector_ptr(des_ptr) = gcRescueBitVector((BPLONG_PTR)DV_bit_vector_ptr(ptr));
                }
                FOLLOW(addr) = (BPLONG)ADDR_AFTER_GC(des_ptr);
                goto loop;
            } else {
                FOLLOW(addr) = (BPLONG)ptr;
                goto loop;
            }
        }
    }
}

