/********************************************************************
 *   File   : expand.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024
 *   Purpose: expand data areas (program area, trail stack, table space, and global/local stacks)

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>
#include "bprolog.h"
#include "frame.h"
#include "event.h"
#include "gc.h"
extern BPLONG gc_time;
/*
  #define DEBUG_EXPAND
*/
#define VALIDATE_HEAP_PTR(ptr)
#define VALIDATE_STACK_PTR(ptr)

/*
  #define VALIDATE_HEAP_PTR(ptr) if (ptr<stack_low_addr || ptr >= heap_top){printf("ptr=%x\n",ptr); quit("STRANGE HEAP PTR\n");}

  #define VALIDATE_STACK_PTR(ptr) if (ptr<local_top || ptr > stack_up_addr){printf("ptr=%x\n",ptr); quit("STRANGE STACK PTR\n");}
*/

/***********************************************************************************
   expand program area, defined in "bapi.h"
   1. attach a new block to the existing block if the current block runs out of space.
   2. update global variables (parea_low_addr,parea_up_addr) and predicates (statistics).
***********************************************************************************/

/***********************************************************************************
 expand the trail stack, and return the new trail_top 
   1. the trail stack is expanded after the available space is below a threshold  (trail_water_mark)
   2. reset B->T for each choice point frame B
   3. copy the contents in verbtim to the new trail stack.
   4. free the old trail stack.
***********************************************************************************/
BPLONG_PTR expand_trail(BPLONG_PTR trail_top, BPLONG_PTR breg)
{
    BPLONG_PTR new_trail_low_addr, new_trail_up_addr, my_breg, top;
    BPLONG diff, new_trail_size;
    BPLONG msec0;

    msec0 = cputime();

    /*  printf("EXPAND_TRAIL %d\n",trail_size); */

    new_trail_size = 2*trail_size;

    new_trail_low_addr = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_trail_size);
    if (new_trail_low_addr == NULL) {
        new_trail_size = trail_size+1000000L;
        new_trail_low_addr = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_trail_size);
        if (new_trail_low_addr == NULL) {
            myquit(OUT_OF_MEMORY, "te");
        }
    }
    num_trail_expansions++;

    new_trail_up_addr = new_trail_low_addr + new_trail_size -1;

    my_breg = breg;
    for (; ; ) {
        top = (BPLONG_PTR)AR_T(my_breg);
        diff = (BPULONG)trail_up_addr-(BPULONG)top;
        AR_T(my_breg) = (BPULONG)new_trail_up_addr-diff;
        if (AR_B(my_breg) == (BPLONG)my_breg) break;
        my_breg = (BPLONG_PTR)AR_B(my_breg);
    }

    my_memcpy_top_down(new_trail_up_addr, trail_up_addr, trail_size);
    diff = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    trail_up_addr = new_trail_up_addr;
    trail_size = new_trail_size;
    free(trail_low_addr);
    trail_low_addr = new_trail_low_addr;
    trail_water_mark = trail_low_addr+LARGE_MARGIN;
    trail_water_mark0 = trail_low_addr+2;
    gc_time += (cputime()-msec0);
    return (BPLONG_PTR)((BPULONG)new_trail_up_addr-diff);
}

void my_memcpy_top_down(BPLONG_PTR des, BPLONG_PTR src, BPLONG size)
{
    while (size > 0) {
        FOLLOW(des) = FOLLOW(src);
        des--; src--;
        size--;
    }
}

void my_memcpy_btm_up(BPLONG_PTR des, BPLONG_PTR src, BPLONG size)
{
    while (size > 0) {
        FOLLOW(des) = FOLLOW(src);
        des++; src++;
        size--;
    }
}

/***********************************************************************************/
void unmark_ar_chain(BPLONG_PTR f)
{
    while (FRAME_IS_MARKED(f)) {
        UNMARK_FRAME(f);
        f = (BPLONG_PTR)AR_AR(f);
    }
}

void unmark_b_chain(BPLONG_PTR b)
{
    for (; ; ) {
        unmark_ar_chain(b);
        if (b == (BPLONG_PTR)AR_B(b)) return;
        b = (BPLONG_PTR)AR_B(b);
    }
}

void unmark_sf_chain(BPLONG_PTR sf)
{
    for (; ; ) {
        UNMARK_FRAME(sf);
        if (sf == (BPLONG_PTR)AR_PREV(sf)) return;
        sf = (BPLONG_PTR)AR_PREV(sf);
    }
}

/***********************************************************************************
  1. a new and larger stack-heap area is allocated after the available space 
     in the current area is below certain threshold after GC and there is no 
     event to be handled.
  2. copy stack frames and the heap to the new area.
  3. nullify untagged cells (attributes of suspension variables) in all reachable terms
     from stack frames and trailed cells.
  3'. adjust pointers to the stack in the table area
  4. reset the pointers in the copied stack and heap to point to the new area.
  5. reset global registers (local_top,heap_top, breg, hbreg, arreg, and sfreg)
***********************************************************************************/
int expand_local_global_stacks(BPLONG preferred_size) {
    BPLONG_PTR new_stack_low_addr, new_stack_up_addr;
    BPLONG new_stack_size, diff_h, diff_s;
    BPLONG msec0, maxs;


    /* precondition (toam_signal_vec == 0) so no need to consider TriggeredCs[..] */

    // printf("=>expand stack\n");
    //  track_stack_ref_on_heap();

    /*
      write_regs();
      printf("\n+++++++++++++++++++++++++++\n");
      show_stack_image();
    */
    msec0 = cputime();
#ifdef DEBUG_EXPAND
    new_stack_size = stack_size+1004;
#else
    if (preferred_size == 0)
        new_stack_size = 2*stack_size;
    else
        new_stack_size = preferred_size;
#endif

    /*  if (new_stack_size>stack_size_limit) return;  */

    BP_MALLOC(new_stack_low_addr, new_stack_size);
    if (new_stack_low_addr == NULL) {
        if (preferred_size != 0) return BP_ERROR;
        new_stack_size = stack_size+1000000L;
        BP_MALLOC(new_stack_low_addr, new_stack_size);
        if (new_stack_low_addr == NULL) {
            return BP_ERROR;
        }
    }

    num_stack_expansions++;
    // printf("expanding the stack...");
    NO_RESERVED_SLOTS(arreg, maxs);
    expand_initialize_frames(maxs);

    new_stack_up_addr = new_stack_low_addr + new_stack_size -1;
    diff_h = (BPULONG)new_stack_low_addr-(BPULONG)stack_low_addr;  /* add this to heap pointers */
    diff_s = (BPULONG)new_stack_up_addr-(BPULONG)stack_up_addr;  /* add this to stack pointers */

    my_memcpy_btm_up(new_stack_low_addr, stack_low_addr, ((BPULONG)heap_top-(BPULONG)stack_low_addr)/sizeof(BPLONG));
    my_memcpy_btm_up((BPLONG_PTR)((BPULONG)local_top+diff_s)+1, local_top+1, ((BPULONG)stack_up_addr-(BPULONG)local_top)/sizeof(BPLONG));

    if (expandStackNullifyUntaggedCells(diff_s) == BP_ERROR) return BP_ERROR;
    /* reset pointers */
    expandStackResetPointers(diff_h, diff_s);

    free(stack_low_addr);
    stack_low_addr = new_stack_low_addr;
    stack_up_addr = new_stack_up_addr;
    stack_size = new_stack_size;
    arreg = (BPLONG_PTR)((BPULONG)arreg+diff_s);
    local_top = (BPLONG_PTR)((BPULONG)local_top+diff_s);
    breg = (BPLONG_PTR)((BPULONG)breg+diff_s);
    gc_b = (BPLONG_PTR)((BPULONG)gc_b+diff_s);
    breg0 = (BPLONG_PTR)((BPULONG)breg0+diff_s);
    heap_top = (BPLONG_PTR)((BPULONG)heap_top+diff_h);
    sfreg = (BPLONG_PTR)((BPULONG)sfreg+diff_s);
    hbreg = (BPLONG_PTR)((BPULONG)hbreg+diff_h);
    // printf(" done\n");

    /*
      track_stack_ref_on_heap();
      printf("<=expand_stack\n");
      printf("\n******************************\n");
      show_stack_image(); 
      write_regs();
    */
    gc_time += (cputime()-msec0);
    return BP_TRUE;
}

/* initialize all frames before expansion. */
void expand_initialize_frames(BPLONG maxs)
{
    BPLONG_PTR b, cp;

    expand_initialize_ar_chain(arreg, maxs);

    b = breg;
    for (; ; ) {
        if (!FRAME_IS_MARKED(b)) {
            cp = (BPLONG_PTR)AR_CPF(b);
            maxs = *(cp-1);  /* maxS is always accessible through CPF */
            expand_initialize_ar_chain(b, maxs);
        }
        if (b == (BPLONG_PTR)AR_B(b)) break;
        b = (BPLONG_PTR)AR_B(b);
    }

    /* reset AR of initialized frames */
    unmark_ar_chain(arreg);
    unmark_b_chain(breg);
}

void expand_initialize_ar_chain(BPLONG_PTR f, BPLONG maxs)
{
    BPLONG_PTR cp, prev_f;

    while (!FRAME_IS_MARKED(f)) {
        prev_f = (BPLONG_PTR)AR_AR(f);
        initialize_frame(f, maxs);
        MARK_FRAME(f);
        cp = (BPLONG_PTR)AR_CPS(f);
        maxs = *(cp-1);
        f = prev_f;
    }
}

void initialize_frame(BPLONG_PTR f, BPLONG maxs)
{
    BPLONG_PTR ptr, top;
    if (IS_SUSP_FRAME(f)) return;  /* done already */
    ptr = f-maxs;
    top = (BPLONG_PTR)AR_TOP(f);
    while (ptr > top) {
        FOLLOW(ptr) = (BPLONG)ptr;
        ptr--;
    }
}


/***********************************************************/
int expandStackNullifyUntaggedCells(BPLONG diff_s)
{
    BPLONG mask_size;

    mask_size = (heap_top-stack_low_addr)/NBITS_IN_LONG+2;  /* masking bits */
    if (allocateMaskArea(mask_size) == BP_ERROR) return BP_ERROR;  /* allocate and initialize copy area */

    expandStackNullifyUntaggedCellsArFrames(arreg, diff_s);
    expandStackNullifyUntaggedCellsBFrames(breg, diff_s);
    expandStackNullifyUntaggedCellsSfFrames(sfreg, diff_s);
    expandStackNullifyUntaggedCellsTrail();
    return BP_TRUE;
}

void expandStackNullifyUntaggedCellsBFrames(BPLONG_PTR breg, BPLONG diff_s)
{
    BPLONG_PTR breg1;

    for (; ; ) {
        expandStackNullifyUntaggedCellsArFrames(breg, diff_s);  /* active chain from this choice point */
        breg1 = (BPLONG_PTR)AR_B(breg);
        if (breg1 == breg) return;
        breg = breg1;
    }
}

/* reset pointers in terms reachable from suspension frames. */
void expandStackNullifyUntaggedCellsSfFrames(BPLONG_PTR sf, BPLONG diff_s)
{
    BPLONG_PTR sf1;
    for (; ; ) {
        expandStackNullifyUntaggedCellsFrame(sf, diff_s);
        sf1 = (BPLONG_PTR)AR_PREV(sf);
        if (sf == sf1) return;
        sf = sf1;
    }
}

/* reset pointers in slots and terms reachable from activation frames. */
void expandStackNullifyUntaggedCellsArFrames(BPLONG_PTR ar, BPLONG diff_s)
{
    BPLONG_PTR ar1;

    for (; ; ) {
        if (AR_BTM(ar) == 0) return;  /* nullified already */
        expandStackNullifyUntaggedCellsFrame(ar, diff_s);
        ar1 = (BPLONG_PTR)AR_AR(ar);
        if (ar == ar1) return;
        ar = ar1;
    }
}

void expandStackNullifyUntaggedCellsFrame(BPLONG_PTR ar, BPLONG diff_s)
{
    BPLONG nslots;

    VALIDATE_STACK_PTR(ar);
    if (AR_BTM(ar) == 0) return;  /* nullified already */
    AR_CPS(ar) = 0;
    if (IS_NONDET_FRAME(ar) || IS_TABLE_FRAME(ar)) {
        AR_CPF(ar) = 0;
        if (IS_TABLE_FRAME(ar)) {
            BPLONG_PTR subgoal_entry, master_ar;
            subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(ar);
            if (subgoal_entry != (BPLONG_PTR)NULL) {  /* reset pointers to the stack in the table area */
                master_ar = (BPLONG_PTR)GT_TOP_AR(subgoal_entry);
                if (master_ar > local_top && master_ar <= stack_up_addr) {
                    GT_TOP_AR(subgoal_entry) = (BPULONG)master_ar + diff_s;
                }
            }
            AR_SUBGOAL_TABLE(ar) = 0;
            AR_CURR_ANSWER(ar) = 0;
            AR_TABLE_NEW_BITS(ar) = 0;  /* nullify it, so its content is copied in verbatim */
        }
    } else if (IS_SUSP_FRAME(ar)) {
        AR_STATUS(ar) = 0;
    }
    NO_RESERVED_SLOTS(ar, nslots);
    expandStackNullifyUntaggedCellsFrameSlots(ar, nslots, diff_s);
}

void expandStackNullifyUntaggedCellsFrameSlots(BPLONG_PTR f, BPLONG nslots, BPLONG diff_s)
{
    BPLONG_PTR sp, top;

    /* arguments */
    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    VALIDATE_STACK_PTR(sp);
    while (sp > f) {
        expandStackNullifyUntaggedCellsTerm(FOLLOW(sp));
        sp--;
    }

    /* local variables */
    sp = f-nslots;
    top = (BPLONG_PTR)AR_TOP(f);
    while (sp > top) {
        expandStackNullifyUntaggedCellsTerm(FOLLOW(sp));
        sp--;
    }
    /* borrow sp */
    sp = (BPLONG_PTR)((BPULONG)f+diff_s);  /* this frame will be moved here, reset AR_BTM(f) now */
    AR_BTM(sp) = ADDTAG(((BPULONG)UNTAGGED_ADDR(AR_BTM(f))+diff_s), (AR_BTM(f) & TAG_MASK));
    AR_BTM(f) = 0;
}

void expandStackNullifyUntaggedCellsTrail()
{
    BPLONG_PTR curr_t, addr;
    BPLONG op;

    curr_t = trail_top+1;
    while (curr_t < trail_up_addr) {
        op = FOLLOW(curr_t);
        addr = (BPLONG_PTR)UNTAGGED3(op);
        if (TAG(op) == TRAIL_VAR) {
            expandStackNullifyUntaggedCellsTerm(FOLLOW(addr));
        } else if (TAG(op) == TRAIL_VAL_NONATOMIC) {
            expandStackNullifyUntaggedCellsTerm(FOLLOW(addr));
            expandStackNullifyUntaggedCellsTerm(FOLLOW(curr_t+1));
        }
        curr_t += 2;
    }
}

void expandStackNullifyUntaggedCellsTerm(BPLONG term)
{
    BPLONG_PTR ptr;
    BPLONG arity, i, tmp;

    if (TAG(term) == ATM) return;  /* not a pointer to stack or heap */
    gcQueueInit;
    GCQueueAddTerm(term);

loop:
    while (gcQueueCount > 0) {
        GCQueueGetTerm(term);
    start:
        if (ISREF(term)) {
            if (IS_STACK_OR_HEAP_REFERENCE(term)) {
                if (!ISFREE(term)) {
                    term = FOLLOW(term);  /* deref */
                    goto start;
                };
            }
            goto loop;
        }
        if (TAG(term) == ATM) {
            goto loop;
        } else if (ISLIST(term)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            VALIDATE_HEAP_PTR(ptr);
            if (IS_HEAP_REFERENCE(ptr) && gcIsMarked(ptr, stack_low_addr) == 0) {
                gcSetMask(ptr, 2, stack_low_addr);
                tmp = FOLLOW(ptr);
                if (TAG(tmp) != ATM) {GCQueueAddTerm(tmp);}
                tmp = FOLLOW(ptr+1);
                if (TAG(tmp) != ATM) {GCQueueAddTerm(tmp);}
                goto loop;
            }
            goto loop;  /* not a heap pointer or marked already */
        } else if (ISSTRUCT(term)) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            VALIDATE_HEAP_PTR(ptr);
            if (IS_HEAP_REFERENCE(ptr) && gcIsMarked(ptr, stack_low_addr) == 0) {
                arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
                gcSetMask(ptr, arity+1, stack_low_addr);
                for (i = 1; i <= arity; i++) {
                    tmp = FOLLOW(ptr+i);
                    if (TAG(tmp) != ATM) {GCQueueAddTerm(tmp);}
                }
                goto loop;
            }
            goto loop;  /* not a heap pointer or marked already */
        } else {  /* susp var */
            ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);  /* TAG on data */
            VALIDATE_HEAP_PTR(ptr);
            if (gcIsMarked(ptr, stack_low_addr) == 0) {
                gcSetMask(ptr, 1, stack_low_addr);
                DV_first(ptr) = 0;
                DV_last(ptr) = 0;
                DV_size(ptr) = 0;
                GCQueueAddTerm(DV_attached(ptr));
                if (IS_BV_DOMAIN(ptr)) {
                    BPLONG_PTR bv_ptr;
                    BPLONG i, from, to;
                    bv_ptr = (BPLONG_PTR)DV_bit_vector_ptr(ptr);
                    from = BV_low_val(bv_ptr); BV_low_val(bv_ptr) = 0;
                    to = BV_up_val(bv_ptr); BV_up_val(bv_ptr) = 0;
                    ptr = BV_base_ptr(bv_ptr);
                    for (i = from; i <= to; i += NBITS_IN_LONG) {
                        FOLLOW(ptr++) = 0;
                    }
                }
            }
            goto loop;
        }
    }
}

void expandStackResetPointers(BPLONG diff_h, BPLONG diff_s)
{
    expandStackResetHeap(diff_h);
    expandStackResetStack(diff_h, diff_s);
    expandStackResetTrail(diff_h, diff_s);
}

BPLONG_PTR expandStackResetAddr(BPLONG_PTR addr, BPLONG diff_h, BPLONG diff_s)
{
    if (IS_HEAP_REFERENCE(addr)) {
        return (BPLONG_PTR)((BPULONG)addr+diff_h);
    } else if (IS_STACK_REFERENCE(addr)) {
        return (BPLONG_PTR)((BPULONG)addr+diff_s);
    } else
        return addr;
}

void expandStackResetHeap(BPLONG diff_h)
{
    BPLONG_PTR ptr, new_ptr, addr;
    BPLONG op;

    for (ptr = stack_low_addr; ptr < heap_top; ptr++) {
        op = FOLLOW(ptr);
        if (op != 0 && TAG(op) != ATM) {
            addr = (BPLONG_PTR)UNTAGGED_ADDR(op);
            if (IS_HEAP_REFERENCE(addr)) {
                new_ptr = (BPLONG_PTR)((BPULONG)ptr+diff_h);
                FOLLOW(new_ptr) = ADDTAG(((BPULONG)addr+diff_h), (op & TAG_MASK));
            }
            /* else if (IS_STACK_REFERENCE(addr)){
               printf("HEAP CELL POINTS TO STACK %x(%x)\n",ptr,addr);
               quit("STRANGE");
               }
            */
        }
    }
}

void expandStackResetStack(BPLONG diff_h, BPLONG diff_s)
{
    BPLONG_PTR ptr, new_ptr, addr;
    BPLONG op;

    for (ptr = stack_up_addr; ptr > local_top; ptr--) {
        op = FOLLOW(ptr);
        if (op != 0 && TAG(op) != ATM) {
            addr = (BPLONG_PTR)UNTAGGED_ADDR(op);
            new_ptr = (BPLONG_PTR)((BPULONG)ptr+diff_s);
            FOLLOW(new_ptr) = ADDTAG((BPLONG)expandStackResetAddr(addr, diff_h, diff_s), (op & TAG_MASK));
        }
    }
}

void expandStackResetTrail(BPLONG diff_h, BPLONG diff_s)
{
    BPLONG_PTR curr_t, addr;
    BPLONG op, addr_tag;

    curr_t = trail_top+1;
    while (curr_t < trail_up_addr) {
        op = FOLLOW(curr_t);
        addr = (BPLONG_PTR)UNTAGGED3(op);
        addr_tag = TAG(op);
        FOLLOW(curr_t) = ADDTAG3((BPLONG)expandStackResetAddr(addr, diff_h, diff_s), addr_tag);
        if (addr_tag == TRAIL_VAR || addr_tag == TRAIL_VAL_NONATOMIC) {
            op = FOLLOW(curr_t+1);
            if (TAG(op) != ATM) {
                addr = (BPLONG_PTR)UNTAGGED_ADDR(op);
                FOLLOW(curr_t+1) = ADDTAG((BPLONG)expandStackResetAddr(addr, diff_h, diff_s), (op & TAG_MASK));
            }
        }
        curr_t += 2;
    }
}

/************************************************************************
Utilities for tracing bugs
************************************************************************/
/*
  show_susp_frames(){

  BPLONG_PTR fp = sfreg;
  while (fp!=(BPLONG_PTR)AR_PREV(fp)){
  if (FRAME_IS_DEAD(fp)){
  fprintf(curr_out,"(*%d)",((BPULONG)fp>>2) & MASK_LOW16);
  } else if (FRAME_IS_SLEEP(fp)){
  fprintf(curr_out,"(+%d)",((BPULONG)fp>>2) & MASK_LOW16);
  } else if (FRAME_IS_REENT(fp)){
  fprintf(curr_out,"(-%d)",((BPULONG)fp>>2) & MASK_LOW16);
  } else {
  fprintf(curr_out,"(%d)",((BPULONG)fp>>2) & MASK_LOW16);
  }
  fp = (BPLONG_PTR)AR_PREV(fp);
  }
  fprintf(curr_out,"\n");                       
  return BP_TRUE;
  }
  check_susp_frames_reep(char *mess){
  BPLONG_PTR fp = sfreg;
  while (fp!=(BPLONG_PTR)AR_AR(fp)){
  if (AR_REEP(fp)<(BPLONG)parea_low_addr){
  printf("%s STRANGE  AR_REEP(%lx)=%lx \n",mess,fp,AR_REEP(fp));
  }
  if ((BPLONG_PTR)(UNTAGGED_ADDR(AR_BTM(fp)))-fp<0) {
  printf("%s STRANGE  AR_BTM(%lx)=%lx \n",mess,fp,AR_BTM(fp));
  }
  fp = (BPLONG_PTR)AR_PREV(fp);
  }
  }

  check_susp_frames(){
  
  BPLONG_PTR fp = sfreg;
  while (fp!=(BPLONG_PTR)AR_AR(fp)){
  BPLONG_PTR sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(fp));
  while (sp>fp){
  BPLONG op = FOLLOW(sp); 
  DEREF(op); 
  if (ISREF(op) && ISFREE(op)){
  show_frame(fp);
  printf("Corrupted frame \n");exit(0);}
  sp--;
  }
  fp = (BPLONG_PTR)AR_PREV(fp);
  }
  }

  trackTopoffAR(arreg,inst)
  BPLONG_PTR arreg;
  BPLONG inst;
  {
  BPLONG_PTR arreg0;
  BPLONG prev_inst;
  do {
  if ((TOP_BIT_MASK & (BPULONG)arreg)!=TOP_BIT_MASK){
  printf("previous inst=%d cur inst=%d\n",prev_inst,inst);
  }
  arreg = (BPLONG_PTR)FOLLOW(arreg);
  } while (arreg!=(BPLONG_PTR)FOLLOW(arreg));
  prev_inst = inst;
  }      

  trackTopoffHeapRef(inst)
  BPLONG  inst;
  {
  BPLONG prev_inst;
  BPLONG_PTR ptr = stack_low_addr;
  while (ptr<heap_top){
  if (ISREF(FOLLOW(ptr)) && ((TOP_BIT_MASK & FOLLOW(ptr))!=TOP_BIT_MASK)){
  printf("previous inst=%d cur inst=%d\n",prev_inst,inst);
  }
  ptr++;
  }
  prev_inst = inst;
  }

  show_cs_list(cs_list)
  BPLONG cs_list;
  {
  BPLONG_PTR ptr,constr_ar;
  while (ISLIST(cs_list)){
  ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs_list);
  constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(FOLLOW(ptr)));
  if (AR_STATUS(constr_ar)==SUSP_EXIT){
  printf("*");
  }
  write_term(FOLLOW(ptr)); printf(" ");
  cs_list = LIST_NEXT(ptr);
  }
  }      

  show_dead_sf_frame(){
  BPLONG_PTR ptr = sfreg;
  while (ptr!=(BPLONG_PTR)AR_AR(ptr)){
  if (FRAME_IS_DEAD(ptr)){
  if (ptr<breg) printf("*"); else printf("+");
  } else printf("-");
  ptr = (BPLONG_PTR)AR_PREV(ptr);
  }
  }

  traceBrokenTrail(src)
  char *src;
  {
  BPLONG_PTR curr_t,addr;
  BPLONG op;

  curr_t = trail_top+1;
  while (curr_t < trail_up_addr){
  op = FOLLOW(curr_t);
  addr = (BPLONG_PTR)UNTAGGED3(op);
  if (addr<stack_low_addr || addr>stack_up_addr){
  printf("%s addr=%x \n",src,op);
  quit("STRANGE trailed item\n");
  }
  curr_t += 2;
  }
  }

  dereferencedToZero(term)
  BPLONG term;
  {
  if (ISREF(term)){
  if (term==0) return 1;
  if (ISFREE(term)) return 0;
  term = FOLLOW(term);
  }
  return 0;
  }
    
  show_heap_image(){
  BPLONG_PTR ptr;
  printf("\nHEAP IMAGE %x %x \n",stack_low_addr,heap_top); 
  show_mem_image(stack_low_addr,heap_top-1);
  }

  show_mem_image(low,up)
  BPLONG low,up;
  {
  BPLONG_PTR ptr;
  BPLONG lpart;
  int count = 0;
  for (ptr=up;ptr>=low;ptr--){
  lpart = ((BPLONG)ptr & 0xffff);
  printf("%4x(%8x)",lpart,*ptr);
  count++;
  if (count%5==0) printf("\n");
  }
  }

  show_heap_susp_vars(){
  BPLONG_PTR ptr,dv_ptr;

  printf("stack_low_addr=%x \n",stack_low_addr); 
  ptr=stack_low_addr;
  while (ptr<heap_top){
  if (IS_SUSP_VAR(FOLLOW(ptr))){
  dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(FOLLOW(ptr));
  if (UNTAGGED_ADDR(FOLLOW(dv_ptr))==(BPLONG)dv_ptr){
  print_susp_var(dv_ptr);
  }
  }
  ptr++;
  }
  }

  write_regs(){
  printf("local_top=%x heap_top=%x arreg=%x breg=%x \n",local_top,heap_top,arreg,breg);
  printf("trail_top=%x trail_up_addr=%x parea_low_addr=%x hbreg=%x\n",trail_top,trail_up_addr,parea_low_addr,hbreg);
  printf("sfreg=%x\n",sfreg);
  }

  print_susp_var(dv_ptr)
  BPLONG_PTR dv_ptr;
  {
  printf("SUSP_VAR: %x type=%x first=%x last=%x size=%x\n",dv_ptr,DV_type(dv_ptr),DV_first(dv_ptr),DV_last(dv_ptr),DV_size(dv_ptr));
  printf("  attached: "); write_term(DV_attached(dv_ptr));printf("\n"); 
  printf("  ins_cs  : "); write_term(DV_ins_cs(dv_ptr));printf("\n"); 
  printf("  bound_cs: "); write_term(DV_minmax_cs(dv_ptr));printf("\n"); 
  printf("  dom_cs  : "); write_term(DV_dom_cs(dv_ptr));printf("\n"); 
  printf("  dom1_cs : "); write_term(DV_outer_dom_cs(dv_ptr));printf("\n"); 
  }


  show_image(){
  printf("breg=%x\n",breg);
  //  show_ar_chain(arreg); 
  show_sf_chain(sfreg); 

  show_stack_image();
  show_heap_image();
  show_trail_image();

  return BP_TRUE;
  }

  show_trail_image(){
  BPLONG_PTR curr_t,addr;
  
  printf("\nTRAIL (breg=%x heap_top=%x hbreg=%x\n",breg,heap_top,hbreg);
  curr_t = trail_top+1;
  while (curr_t < (BPLONG_PTR)trail_up_addr){ 
  addr = (BPLONG_PTR)UNTAGGED3(FOLLOW(curr_t));
  printf("%x:: %x (%x)\n",curr_t,FOLLOW(curr_t),FOLLOW(addr));
  curr_t += 2;
  }
  }

  show_top_trail_image(){
  BPLONG_PTR curr_t;
  int count = 0;
  
  printf("TRAILTRAILTRAILTRAILTRAILTRAILTRAIL\n");
  curr_t = trail_top+1;
  while (curr_t < (BPLONG_PTR)AR_T(breg)){
  printf("%x (%x) ",FOLLOW(curr_t),FOLLOW(curr_t+1));
  count++;
  if (count%4==0) printf("\n");
  curr_t += 2;
  }
  }

  show_stack_image(){
  BPLONG_PTR sp;

  printf("\nSTACK %x %x\n",local_top+1,stack_up_addr);
  show_mem_image(local_top+1,stack_up_addr);
  }

  print_frame_chains(arreg,breg,sfreg)
  BPLONG_PTR arreg,breg,sfreg;
  {
  BPLONG_PTR f;

  f =arreg;
  for (;;){
  print_frame_bound(f);
  if ((BPLONG)f==AR_AR(f)) break;
  f = (BPLONG_PTR)AR_AR(f);
  }
  f = breg;
  for (;;){
  print_frame_bound(f);
  if ((BPLONG)f==AR_B(f)) break;
  f = (BPLONG_PTR)AR_B(f);
  }
  f = sfreg;
  for (;;){
  print_frame_bound(f);
  if ((BPLONG)f==AR_PREV(f)) break;
  f = (BPLONG_PTR)AR_PREV(f);
  }
  }

  print_frame_bound(f)
  BPLONG_PTR f;
  {
  printf("frame %x (%x %x)\n",f,UNTAGGED_ADDR(AR_BTM(f)),AR_TOP(f));
  }

  show_stack_image1(low,up)
  BPLONG_PTR low,up;
  {
  BPLONG_PTR sp = low;
  while (sp <= up){
  if (sp+4<up){
  printf("%8x(%8x) %8x(%8x) %8x(%8x) %8x(%8x)\n",sp,FOLLOW(sp),sp+1,FOLLOW(sp+1),sp+2,FOLLOW(sp+2),sp+3,FOLLOW(sp+3));
  sp += 4;
  } else {
  printf("%x(%x) \n",sp,FOLLOW(sp));
  sp++;
  }
  }
  }


  show_b_chain()
  {
  BPLONG_PTR ptr;

  ptr = (BPLONG_PTR)breg;
  while ((BPLONG)ptr!=FOLLOW(ptr)){
  printf("(%x)->SF=%x\n",ptr,AR_SF(ptr));
  ptr = (BPLONG_PTR)AR_B(ptr);
  }
  }

  track_broken_sf_slot()
  {
  BPLONG_PTR ptr;

  ptr = (BPLONG_PTR)breg;
  while ((BPLONG)ptr!=FOLLOW(ptr)){
  if ((BPLONG_PTR)AR_SF(ptr)<ptr){
  printf("Strange sf (%x)->SF=%x\n",ptr,AR_SF(ptr));
  #ifdef TRACE_INSTS
  { 
  int i;
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  }
  #endif
  quit("Strange");
  }
  ptr = (BPLONG_PTR)AR_B(ptr);
  }
  }


  show_ar_chain_upto_b(ar)
  BPLONG_PTR ar;
  {
  if (ar<=breg){
  show_ar_chain_upto_b(AR_AR(ar));
  show_frame(ar);
  }
  }

  show_ar_chain(ar)
  BPLONG_PTR ar;
  {
  if ((BPLONG)ar==FOLLOW(ar))
  show_frame(ar);
  else {
  show_ar_chain(AR_AR(ar));
  show_frame(ar);
  }
  }

  track_broken_event_slot()
  {
  BPLONG_PTR sf;

  sf = sfreg;
  if (sf!=(BPLONG_PTR)AR_PREV(sf)){
  BPLONG op = AR_OUT(sf);
  DEREF(op);
  if (op==0){
  printf("AR_OUT=0\n");
  quit("STRANGE");
  }
  }
  }

  show_sf_chain(sf)
  BPLONG_PTR sf;
  {
  BPLONG_PTR sf1;

  if (sf!=(BPLONG_PTR)AR_PREV(sf))
  show_frame(sf);
  else {
  show_sf_chain(AR_PREV(sf));
  show_frame(sf);
  }
  }

  show_tabled_subgoal(ar,sym)
  BPLONG_PTR ar;
  SYM_REC_PTR sym;
  {
  BPLONG_PTR sp;
  
  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(ar));
  if (sp==ar){
  fprintf(curr_out,"%s.\n",GET_NAME(sym));
  return;
  }
  fprintf(curr_out,"%s(",GET_NAME(sym));
  while (sp>ar){
  write_term(FOLLOW(sp));
  if (sp!=ar+1)fprintf(curr_out,",");
  sp--;
  }
  fprintf(curr_out,").\n");
  }

  show_frame_args(ar)
  BPLONG_PTR ar;
  {
  BPLONG_PTR sp;
  BPLONG no,op;
  
  NO_RESERVED_SLOTS(ar,no);
  
  printf("*******%x*******(%d,%d)\n",ar,(BPLONG_PTR)(UNTAGGED_ADDR(AR_BTM(ar)))-ar,no);

  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(ar));
  while (sp>ar){
  printf("%x %x ",sp,FOLLOW(sp)); op = FOLLOW(sp); DEREF(op);
  if (ISREF(op) && op>(BPULONG)local_top){ printf("(_s%x)",op); } else{ write_term(op);}; printf("\n");
  sp--;
  }
  printf("LOCAL VARS\n");
  sp = ar-no;
  while (sp>(BPLONG_PTR)AR_TOP(ar)){
  printf("%x %x \n",sp,FOLLOW(sp));
  sp--;
  }
  }

  show_frame(ar)
  BPLONG_PTR ar;
  {
  BPLONG_PTR sp;
  BPLONG no,op;
  
  NO_RESERVED_SLOTS(ar,no);
  
  printf("*******%x*******(%d,%d)\n",ar,(BPLONG_PTR)(UNTAGGED_ADDR(AR_BTM(ar)))-ar,no);
  if ((BPLONG_PTR)(UNTAGGED_ADDR(AR_BTM(ar)))-ar<0){
  printf("STATUS=%lx\n",AR_STATUS(ar));
  quit("NEGATIVE HUHUHUGE FRAME\n");
  }

  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(ar));
  while (sp>ar){
  printf("%x %x ",sp,FOLLOW(sp)); op = FOLLOW(sp); DEREF(op);
  if (ISREF(op) && op>(BPULONG)local_top){ printf("(_s%x)",op); } else{ write_term(op);}; printf("\n");
  sp--;
  }

  printf("%x (%x)  PAR \n",ar,FOLLOW(ar));
  printf("%x (%x)  CPS \n",ar-1,FOLLOW(ar-1));
  printf("%x (%x)  TOP \n",ar-2,FOLLOW(ar-2));
  printf("%x (%x)  BTM \n",ar-3,FOLLOW(ar-3));
  if (no==SUSP_FRAME_SIZE){
  printf("%x (%x)  REEP \n",ar-4,FOLLOW(ar-4));
  printf("%x (%x)  PREV \n",ar-5,FOLLOW(ar-5));
  printf("%x (%x)  STATUS \n",ar-6,FOLLOW(ar-6));
  printf("%x (%x)  OUT \n",ar-7,FOLLOW(ar-7));
  }
    
  if (no>=NONDET_FRAME_SIZE){
  printf("%x (%x)  B \n",ar-4,FOLLOW(ar-4));
  printf("%x (%x)  CPF \n",ar-5,FOLLOW(ar-5));
  printf("%x (%x)  H \n",ar-6,FOLLOW(ar-6));
  printf("%x (%x)  T \n",ar-7,FOLLOW(ar-7));
  printf("%x (%x)  SF \n",ar-8,FOLLOW(ar-8));
  }

  sp = ar-no;
  while ((BPULONG)sp>AR_TOP(ar)){
  printf("%x %x \n",sp,FOLLOW(sp)); 
  //op = FOLLOW(sp); DEREF(op);
  //if (ISREF(op) && op>(BPULONG)local_top){ printf("(_s%x)",op); } else{ write_term(op);}; printf("\n");
  sp--;
  }
  printf("end end *******%x*******(%d,%d) end end\n\n",ar,(BPLONG_PTR)(UNTAGGED_ADDR(AR_BTM(ar)))-ar,no);
  }

  check_broken_ar(ar)
  BPLONG_PTR ar;
  {
  while (ar!=(BPLONG_PTR)AR_AR(ar)){
  ar = (BPLONG_PTR)AR_AR(ar);
  if (breg!=ar && breg>(BPLONG_PTR)AR_TOP(ar) && breg<(BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(ar))){
  printf("Strange ar %x (sfreg=%x breg=%x)\n",ar,sfreg,breg);
  #ifdef TRACE_INSTS
  { 
  int i;
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  }
  #endif
  quit("Strange");
  }
  }
  }

  hunt_broken_hb(where)
  char *where;
  {
  BPLONG_PTR sp = breg;
  while (sp!=(BPLONG_PTR)AR_B(sp)){
  if ((BPULONG)AR_H(sp)<(BPULONG)stack_low_addr || (BPULONG)AR_H(sp)>(BPULONG)stack_up_addr){
  printf("%s breg=%x breg->H=%x\n",where,sp,AR_H(sp));
  quit("STRANGE hbreg");
  }
  sp = (BPLONG_PTR)AR_B(sp);
  }
  }

  hunt_for_corrupted_sf_chain(){
  BPLONG_PTR ptr = sfreg;
  
  while (ptr!=(BPLONG_PTR)AR_PREV(ptr)){
  if ((BPULONG)AR_PREV(ptr)>(BPULONG)stack_up_addr || (BPULONG)AR_PREV(ptr)<(BPULONG)stack_low_addr){
  printf("found corrupted sf chain AR_PREV(%x) = %x\n",ptr,AR_PREV(ptr));
  printf("trail_top=%x,trail_up_addr=%x stack_low_addr=%x stack_up_addr=%x\n",trail_top,trail_up_addr,stack_low_addr,stack_up_addr);      
  quit("STRANGE\n");
  }
  ptr = (BPLONG_PTR)AR_PREV(ptr);
  }
  }

  hunt_for_corrupted_susp_var(){
  BPLONG_PTR addr;
  BPLONG_PTR ptr = stack_low_addr;

  while (ptr<heap_top){
  if (FOLLOW(ptr) ==0xcdcdcdcd){
  printf("found corrupted cell in heap %x (%x)\n",ptr,FOLLOW(ptr));
  show_heap_image();
  quit("STRANGE\n");
  }
  ptr++;
  }
  ptr = stack_up_addr;
  while (ptr>local_top){
  if (FOLLOW(ptr) ==0xcdcdcdcd){
  printf("found corrupted cell in stack %x (%x)\n",ptr,FOLLOW(ptr));
  show_stack_image();
  quit("STRANGE\n");
  }
  ptr--;
  }
  }

  check_stack(ar,breg)
  BPLONG_PTR ar,breg;
  {
  arreg = ar;
  check_ar_chain(ar);
  check_b_chain(breg); 
  }

  check_ar_chain(ar)
  BPLONG_PTR ar;
  {
  BPLONG_PTR ar0=NULL;
  while (ar!=ar0){
  check_frame(ar);
  ar0 = ar;
  ar = (BPLONG_PTR)FOLLOW(ar);
  };
  }

  check_b_chain(b)
  BPLONG_PTR b;
  {
  while(b!=(BPLONG_PTR)AR_B(b)){
  check_ar_chain(b);
  b = (BPLONG_PTR)AR_B(b);
  }
  }

  check_frame(f)
  BPLONG_PTR f;
  {
  int nslots;
  BPLONG_PTR sp,top;

  if (f>stack_up_addr || f<stack_low_addr){
  printf("wong frame pointer %x\n",f);
  }
  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
  while (sp>f){
  check_term(sp,FOLLOW(sp));
  sp--;
  }

  NO_RESERVED_SLOTS(f,nslots);
  sp = f-nslots;
  top = (BPLONG_PTR)AR_TOP(f);
  while (sp>top){
  check_term(sp,FOLLOW(sp));
  sp--;
  }
  }

  check_term(ptr,term)
  BPLONG_PTR ptr;
  BPLONG term;
  {
  int x = 0;
  printf("check_term %x %x\n",ptr,term);
  if (ISREF(term)){
  if ((TOP_BIT_MASK & term)!=TOP_BIT_MASK){
  printf("ptr=%x term=%x (arreg=%x breg=%x)\n",ptr,term,arreg,breg);
  FOLLOW(x) = 0;
  quit("STRANGE ");
  }
  if (ISFREE(term)) return;
  check_term(term,FOLLOW(term));
  return;
  } else if (ISLIST(term)){
  BPLONG_PTR ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
  check_term(ptr,FOLLOW(ptr));
  check_term(ptr+1,FOLLOW(ptr+1));
  return;
  } else if (ISSTRUCT(term)){
  BPLONG_PTR ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
  SYM_REC_PTR sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
  int arity = GET_ARITY(sym_ptr);
  while (arity>=1){
  check_term(ptr+arity,FOLLOW(ptr+arity));
  arity--;
  }
  return;
  }
  if (ISATOM(term) && (BPLONG_PTR)UNTAGGED_ADDR(term)<parea_low_addr){
  printf("ptr=%x term=%x UNTAGGED_ADDR(term)=%x (parea_low_addr=%x)\n",ptr,term,UNTAGGED_ADDR(term),parea_low_addr);
  FOLLOW(x) = 0;
  quit("STRANGE");
  }
  }

  stack_var(term)
  BPLONG term;
  {
  return ((BPULONG)UNTAGGED_ADDR(term)<(BPULONG)stack_up_addr && (BPULONG)UNTAGGED_ADDR(term)>(BPULONG)local_top);
  }

  heap_var(term)
  BPLONG term;
  {
  return ((BPULONG)UNTAGGED_ADDR(term)>=(BPULONG)stack_low_addr && (BPULONG)UNTAGGED_ADDR(term)<(BPULONG)heap_top);
  }

  trackUntrailed(ar)
  BPLONG_PTR ar;
  {
  arreg = ar;
  trackUntrailedCellsAR(arreg);
  trackUntrailedCellsSF(sfreg);
  trackUntrailedCellsB(breg);
  }

  trackUntrailedCellsAR(sp)
  BPLONG_PTR sp;
  {
  while (FOLLOW(sp)!=(BPLONG)sp){
  trackUntrailedCellsFrame(sp);
  sp = (BPLONG_PTR)AR_AR(sp);
  }
  }

  trackUntrailedCellsSF(sp)
  BPLONG_PTR sp;
  {
  
  while (FOLLOW(sp)!=(BPLONG)sp){
  trackUntrailedCellsFrame(sp);
  sp = (BPLONG_PTR)AR_PREV(sp);
  }
  }
    

  trackUntrailedCellsB(sp)
  BPLONG_PTR sp;
  {
  
  while ((BPLONG_PTR)FOLLOW(sp)!=sp){
  trackUntrailedCellsAR(sp);
  sp = (BPLONG_PTR)AR_B(sp);
  }
  }

  trackUntrailedCellsFrame(fp)
  BPLONG_PTR fp;
  {
  BPLONG noReservedSlots;
  BPLONG_PTR sp;

  NO_RESERVED_SLOTS(fp,noReservedSlots);
  
  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(fp));
  while (sp>fp){
  trackUntrailedCell(sp,25);
  sp--;
  }

  sp = fp-noReservedSlots;
  top = (BPLONG_PTR)AR_TOP(fp);
  while (sp>top){
  trackUntrailedCell(sp,25);
  sp--;
  }
  }

  trackUntrailedCell(addr,level)
  BPLONG_PTR addr;
  BPLONG level;
  {
  BPLONG term = FOLLOW(addr);
  if (level==0) return; 
  start:
  if (TAG(term)==REF){
  if (old_than_b(addr) && INSIDE_HEAP_TOP_SEGMENT(term) && not_trailed(addr)) {printf("Strange untrailed cell (%x %x) breg=%x hbreg=%x stack_low=%x stack_up=%x\n",addr,term,breg,hbreg,stack_low_addr,stack_up_addr);
  show_trail_image();
  exit(0);}
  if (ISFREE(term)) return;
  addr = term;
  term = (BPLONG_PTR)FOLLOW(term);
  goto start;
  } else if (TAG(term)==ATM) return;
  else if (ISLIST(term)){
  UNTAG_ADDR(term);
  if (!heap_term(term)) return;
  if (old_than_b(addr) && INSIDE_HEAP_TOP_SEGMENT(term) && not_trailed(addr)) {printf("Strange untrailed cell (%x %x) breg=%x hbreg=%x stack_low=%x stack_up=%x\n",addr,term,breg,hbreg,stack_low_addr,stack_up_addr);
  show_trail_image();
  exit(0);}
  trackUntrailedCell((BPLONG_PTR)term,level-1);
  trackUntrailedCell((BPLONG_PTR)term+1,level);
  } else if (ISSTRUCT(term)){
  BPLONG i,arity;
  arity = GET_STR_SYM_ARITY(term);
  UNTAG_ADDR(term);
  if (!heap_term(term)) return;
  if (old_than_b(addr) && INSIDE_HEAP_TOP_SEGMENT(term) && not_trailed(addr)) {printf("Strange untrailed cell (%x %x) breg=%x hbreg=%x stack_low=%x stack_up=%x\n",addr,term,breg,hbreg,stack_low_addr,stack_up_addr);
  show_trail_image();
  exit(0);}
  for (i=1;i<=arity;i++){
  trackUntrailedCell((BPLONG_PTR)term+i,level-1);
  }
  } else if (IS_SUSP_VAR(term)){
  BPLONG_PTR sv_ptr;
  sv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
  if (old_than_b(addr) && INSIDE_HEAP_TOP_SEGMENT(sv_ptr) && not_trailed(addr)) {printf("Strange untrailed cell (%x %x) breg=%x hbreg=%x stack_low=%x stack_up=%x\n",addr,sv_ptr,breg,hbreg,stack_low_addr,stack_up_addr);
  show_trail_image();
  exit(0);}
  trackUntrailedCell(A_DV_ins_cs(sv_ptr),5);
  trackUntrailedCell(A_DV_minmax_cs(sv_ptr),5);
  trackUntrailedCell(A_DV_dom_cs(sv_ptr),5);
  trackUntrailedCell(A_DV_outer_dom_cs(sv_ptr),5);
  }
  }

  old_than_b(addr)
  BPLONG_PTR addr;
  {
  return ((addr>breg && addr< stack_up_addr) || (addr<hbreg && addr>stack_low_addr));
  }

  not_trailed(addr)
  BPLONG_PTR addr;
  {
  return (!alreadyTrailed(addr,AR_T(breg),trail_top+1));
  }

  heap_term(addr)
  BPLONG_PTR addr;
  {
  return (addr>stack_low_addr && addr< heap_top);
  }

  is_descendent(c,p)
  BPLONG_PTR c,p;
  {
  while (c<=p){
  if (c==p){
  return 1;
  }
  c = (BPLONG_PTR) AR_AR(c);
  }
  return 0;
  }

  display_constraint_list(BPLONG list){
  BPLONG_PTR top,ptr;
  BPLONG_PTR f;

  DEREF(list);

  if (list==nil_sym) return;
  while (list!=nil_sym){
  ptr = UNTAGGED_ADDR(list);
  f = (BPLONG_PTR)UNTAGGED_ADDR(FOLLOW(ptr));
  display_constraint(f);
  printf("AR_OUT=");write_term(AR_OUT(f));printf("\n");
  list = FOLLOW(ptr+1);
  }
  printf("\n");
  }


  printReferencedToZeroChain(ptr)
  BPLONG_PTR ptr;
  {
  while (ptr!=0){
  printf("%x -> ",ptr);
  ptr = (BPLONG_PTR)FOLLOW(ptr);
  }
  quit("\nSTRANGE REFERENCE CHAIN\n");
  }

  check_topunset_ar_subgoal(){
  BPLONG_PTR ptr,subgoal_entry;
  int i;

  for (ptr=breg;(BPLONG_PTR)AR_B(ptr)!=ptr;ptr=(BPLONG_PTR)AR_B(ptr)){
  check_topunset_ar_subgoal_chain(ptr);
  }
  
  for (i=0;i<subgoalTableBucketSize;i++){
  subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
  while (subgoal_entry!=NULL){
  subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
  }
  }
  }

  check_topunset_ar_subgoal_chain(ar)
  BPLONG_PTR ar;
  {
  BPLONG_PTR ptr,subgoal_entry;
  
  while (ar!=(BPLONG_PTR)AR_AR(ar)){
  if (IS_TABLE_FRAME(ar)){
  subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(ar);
  if ((subgoal_entry!=NULL) && (((BPLONG)subgoal_entry & TOP_BIT)!=TOP_BIT)){
  printf("subgoal_entry=%x\n",subgoal_entry);
  quit("STRANGE");
  }
  }
  ar = (BPLONG_PTR)AR_AR(ar);
  }
  }



  showSuspVars(loc)
  char *loc;
  {
  BPLONG_PTR f,sp,top;
  BPLONG_PTR p,mask_end;
  BPLONG mask_size;
  
  mask_size = ((BPULONG)heap_top-(BPULONG)stack_low_addr)/NBITS_IN_LONG+2; 
  allocateMaskArea(mask_size);

  f = sfreg;
  while (f<breg){
  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
  while (sp > f){
  showSuspVarsTerm(FOLLOW(sp));
  sp--;
  }
  f = (BPLONG_PTR)AR_PREV(f);
  }
  }

  showSuspVarsTerm(term)
  BPLONG term;
  {
  BPLONG_PTR dv_ptr;
  BPLONG_PTR ptr;
  BPLONG_PTR top;

  cont:
  DEREF(term);
  if (IS_SUSP_VAR(term)){
  dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
  if (gcIsMarked(dv_ptr,stack_low_addr)==0){
  gcSetMask(dv_ptr,1,stack_low_addr);
  showSuspVar(dv_ptr);
  }
  } else if (ISLIST(term)){
  ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
  if (gcIsMarked(ptr,stack_low_addr)==0){
  gcSetMask(ptr,2,stack_low_addr);
  showSuspVarsTerm(FOLLOW(ptr));
  term = FOLLOW(ptr+1);
  goto cont;
  }
  } else if (ISSTRUCT(term)){ 
  BPLONG i,arity;
  ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
  if (gcIsMarked(ptr,stack_low_addr)==0){
  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
  gcSetMask(ptr,arity+1,stack_low_addr);
  for (i=1;i<arity;i++){
  showSuspVarsTerm(FOLLOW(ptr+i));
  }
  term = FOLLOW(ptr+arity);
  goto cont;
  }
  }
  }


  showSuspVar(dv_ptr)
  BPLONG_PTR dv_ptr;
  {
  BPLONG_PTR ptr;
  BPLONG_PTR head,tail;

  printf("susp_var: %x",dv_ptr);
  printf("susp_ins: ");
  showSuspList(DV_ins_cs(dv_ptr));
  printf("susp_dom: ");
  showSuspList(DV_dom_cs(dv_ptr));
  printf("\n");
  }

  showSuspList(list)
  BPLONG list;
  {
  list = next_alive_susp_call(list,breg);
  while (ISLIST(list)){
  write_term(bp_get_car(list));printf(",");
  list = next_alive_susp_call(bp_get_cdr(list),breg);
  }
  }

  checkTrail(){
  BPLONG op;
  BPLONG_PTR trail_top0,untaggedAddr,curr_t;

  curr_t  = (BPLONG_PTR)AR_T(breg);

  while (curr_t>trail_top){
  op = FOLLOW(curr_t-1);
  untaggedAddr = (BPLONG_PTR)UNTAGGED3(op);
  if ((TAG(op)!=TRAIL_VAR) && alreadyTrailed(untaggedAddr,AR_T(breg),curr_t+1)){
  show_top_trail_image();
  printf("addr=%x\n",untaggedAddr);
  quit("Strange\n");
  } 
  curr_t -= 2;
  }
  }

  
  check_corrupted_sf(src)
  char *src;
  {
  BPLONG_PTR fp =sfreg;

  while (fp!=(BPLONG_PTR)AR_PREV(fp)){
  fp=(BPLONG_PTR)AR_PREV(fp);
  }
  if (fp<breg){
  printf("corrupted sf chain %s\n",src);
  return 1;
  }
  return 0;
  }

  trackBrokenList(){
  BPLONG_PTR curr_t,addr,last_t;

  curr_t = trail_top+1;
  last_t = (BPLONG_PTR)AR_T(breg);
  while (curr_t < last_t){
  addr = (BPLONG_PTR)FOLLOW(curr_t); 
  if (TAG(addr)==TRAIL_VAR){
  write_term(FOLLOW(addr)); printf("\n");
  }
  curr_t += 2;
  }
  }

  trackBrokenStr()
  {
  BPLONG_PTR curr_t,addr,last_t,ptr;
  BPLONG op;

  printf("=>trackBrokenStr\n"); 
  show_trail_image(); 
  
  curr_t = trail_top+1;
  last_t = (BPLONG_PTR)AR_T(breg);
  while (curr_t < last_t){
  addr = (BPLONG_PTR)FOLLOW(curr_t); 
  if (TAG(addr)==TRAIL_VAR || TAG(addr)==TRAIL_VAL_NONATOMIC){
  op = FOLLOW(UNTAGGED3(addr));
  if (ISSTRUCT(op)){
  ptr = (BPLONG_PTR)FOLLOW(UNTAGGED_ADDR(op));
  if (ptr<parea_low_addr || ptr > parea_up_addr){
  printf("Broken structure op=%x sym_ptr=%x parea_low=%x parea_up=%x\n",op,ptr,parea_low_addr,parea_up_addr);
  quit("Strange");
  }
  }      
  }
  curr_t += 2;
  }
  printf("<=trackBrokenStr\n");
  }


  trackDoubleTrail()
  {
  BPLONG_PTR curr_t,addr,last_t,ptr;
  BPLONG op,i;

  packEntireTrail();

  curr_t = trail_top+1;
  last_t = (BPLONG_PTR)AR_T(breg);
  while (curr_t < last_t){
  addr = (BPLONG_PTR)FOLLOW(curr_t); 
  if (TAG(addr)==TRAIL_VAR){
  ptr = curr_t+2;
  while (ptr<trail_up_addr){
  if (FOLLOW(ptr)==addr){
  printf("DOUBLE TRAIL %x (%x)\n",addr,FOLLOW(addr));
  show_trail_image();
  #ifdef TRACE_INSTS
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  #endif
  quit("STRANGE\n");
  }
  ptr += 2;
  }
  }
  curr_t += 2;
  }
  }

  trackRetrailed()
  {
  BPLONG_PTR curr_t,addr,last_t,ptr;
  BPLONG op,i;

  curr_t = trail_top+1;
  last_t = (BPLONG_PTR)AR_T(breg);
  while (curr_t < trail_up_addr){
  addr = (BPLONG_PTR)FOLLOW(curr_t); 
  if (TAG(addr)==TRAIL_VAR && FOLLOW(addr)==addr){
  printf("RE TRAIL %x (%x)\n",addr,FOLLOW(addr));
  show_trail_image();
  #ifdef TRACE_INSTS
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  #endif
  quit("STRANGE\n");
  }
  curr_t += 2;
  }
  }

  checkDoubleTrail(addr,b_t,curr_t,where)
  BPLONG_PTR b_t,addr,curr_t;
  BPLONG where;
  {
  if (TAG(addr)==TRAIL_VAR && alreadyTrailed(UNTAGGED3(addr),b_t,curr_t)){
  printf("Variable doublly trailed Strange %d\n",where);
  exit(1);
  }
  }


  print_ref_chain(term)
  BPLONG term;
  {
  printf("=>%x",term);
  if (ISREF(term)){
  if (ISFREE(term)){printf("*"); return;}
  print_ref_chain(FOLLOW(term));
  }
  }

  trackTopOnRef(ar)
  BPLONG_PTR ar;
  {
  printf("=>AR\n");
  trackTopOnRefCellsAR(ar);
  printf("=>SF\n");
  trackTopOnRefCellsSF(sfreg);
  printf("=>BR\n");
  trackTopOnRefCellsB(breg);
  }

  trackTopOnRefCellsAR(sp)
  BPLONG_PTR sp;
  {
  while (FOLLOW(sp)!=(BPLONG)sp){
  trackTopOnRefCellsFrame(sp);
  sp = (BPLONG_PTR)AR_AR(sp);
  }
  }

  trackTopOnRefCellsSF(sp)
  BPLONG_PTR sp;
  {
  
  while (FOLLOW(sp)!=(BPLONG)sp){
  trackTopOnRefCellsFrame(sp);
  sp = (BPLONG_PTR)AR_PREV(sp);
  if (sp>stack_up_addr) quit("Broken SF Chain\n");
  }
  }
    

  trackTopOnRefCellsB(sp)
  BPLONG_PTR sp;
  {
  
  while ((BPLONG_PTR)FOLLOW(sp)!=sp){
  trackTopOnRefCellsAR(sp);
  sp = (BPLONG_PTR)AR_B(sp);
  }
  }

  trackTopOnRefCellsFrame(fp)
  BPLONG_PTR fp;
  {
  BPLONG noReservedSlots;
  BPLONG_PTR sp;

  NO_RESERVED_SLOTS(fp,noReservedSlots);
  
  sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(fp));
  while (sp>fp){
  trackTopOnRefCell(sp,25);
  sp--;
  }

  sp = fp-noReservedSlots;
  top = (BPLONG_PTR)AR_TOP(fp);
  while (sp>top){
  trackTopOnRefCell(sp,25);
  sp--;
  }
  }

  trackTopOnRefCell(addr,level)
  BPLONG_PTR addr;
  int level;
  {
  BPLONG op = FOLLOW(addr);
  if (ISREF(op) && ((TOP_BIT_MASK & op)==TOP_BIT_MASK)){
  printf("Strange cell %x(%x) stack_low_addr=%x stack_up_addr=%x\n",addr,op,stack_low_addr,stack_up_addr);
  quit("Strange");
  }
  }



  track_stack_ref_on_heap(){
  BPLONG_PTR ptr,addr;
  BPLONG op;
  int i;
  for (ptr=stack_low_addr;ptr<heap_top;ptr++){
  op = FOLLOW(ptr);
  if (op!=0 && TAG(op)!=ATM){
  addr = (BPLONG_PTR)UNTAGGED_ADDR(op);
  if (IS_STACK_REFERENCE(addr)){
  printf("HEAP CELL POINTS TO STACK %x(%x) stack_low_addr=%x stack_up_addr=%x\n",ptr,addr,stack_low_addr,stack_up_addr);
  #ifdef TRACE_INSTS
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  #endif
  quit("STRANGE");
  }
  }
  }
  }

  dump_stack(signo)
  int signo;
  {
  printf("SSSSSSSSSSSSSSSSSSSSSSSSSSS\n");
  show_stack_image1(local_top+1,AR_TOP(breg)); 
  printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
  show_heap_image();
  }


  print_suspvars(){
  BPLONG op;
  BPLONG_PTR ptr = stack_low_addr;

  while (ptr<heap_top){
  op = FOLLOW(ptr);
  if (IS_SUSP_VAR(op) && UNTAGGED_ADDR(op) == (BPLONG)ptr){
  print_suspvar(ptr);
  }
  ptr++;
  }
  }

  print_suspvar(dv_ptr)
  BPLONG_PTR dv_ptr;
  {
  print_cs_lst("ins",DV_ins_cs(dv_ptr));
  print_cs_lst("bod",DV_minmax_cs(dv_ptr));
  print_cs_lst("dom",DV_dom_cs(dv_ptr));
  print_cs_lst("any",DV_outer_dom_cs(dv_ptr));
  }

  print_cs_lst(src,cs)
  char *src;
  BPLONG cs;
  {
  BPLONG constr;
  BPLONG_PTR sf,ptr;
  if (ISLIST(cs)){
  printf("(%s)",src);
  do {
  ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs); 
  constr = FOLLOW(ptr);
  sf = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(constr));
  printf("[%d]",INTVAL(constr));
  if (!IS_SUSP_FRAME(sf)) quit("Strange");
  cs = FOLLOW(ptr+1); 
  } while (ISLIST(cs));
  printf("\n");
  }
  }

  check_broken_sf_chain(){
  BPLONG_PTR ptr = sfreg;

  while (ptr!=(BPLONG_PTR)AR_PREV(ptr)){
  if (ptr<local_top || (breg>(BPLONG_PTR)AR_TOP(ptr) && breg<(BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(ptr)))){
  printf("Strange sf %x (sfreg=%x breg=%x)\n",ptr,sfreg,breg);
  #ifdef TRACE_INSTS
  { 
  int i;
  for (i=9;i>=0;i--){
  printf("%d: %s\n",i,inst_name[exec_trace[i]]);
  }
  }
  #endif
  quit("Strange");
  }
  ptr = AR_PREV(ptr);
  }
  }

  addExecTrace(inst)
  int inst;
  {
  int i;
  #ifdef TRACE_INSTS
  for (i=9;i>0;i--){
  exec_trace[i] = exec_trace[i-1];
  }
  exec_trace[0] = inst;
  #endif
  }


  cycle(term)
  BPLONG term;
  {
  BPLONG term1;
  
  if (!ISREF(term))
  return 0;
  term1 = FOLLOW(term);
  if (!ISREF(term1) || term==term1) return 0;
  return cycle2(term,term1);
  }

  cycle2(term,term1)
  BPLONG term,term1;
  {
  BPLONG term2;

  term2 = FOLLOW(term1);
  if (!ISREF(term2) || term1==term2) return 0;
  if (term == term2){printf("CYCLE FOUND\n"); return 1;}
  return cycle2(term,term2) || cycle2(term1,term2);
  }

  in_ref_chain(term,ptr)
  BPLONG term;
  BPLONG_PTR ptr;
  {
  if (!ISREF(term)) return 0;
  if (term==ptr) return 1;
  if (ISFREE(term)) return 0;
  printf("check chain %x %x\n",term,ptr);
  return in_ref_chain(FOLLOW(term),ptr);
  }

  trace_invalid_ar_slot(f,src)
  BPLONG_PTR f;
  char *src;
  {
  BPLONG_PTR sp;
  BPLONG n;
  
  while (f<breg){
  NO_RESERVED_SLOTS(f,n);
  printf("tracing f=%x n=%d breg=%x %s\n",f,n,breg,src);
  sp = f-n;
  top = (BPLONG_PTR)AR_TOP(f);
  while (sp>top){
  if (FOLLOW(sp)==(BPLONG)f){
  printf("arreg=%x\n",arreg);
  printf("ar=%x AR_TOP=%x n=%d sp=%x %s ",f,AR_TOP(f),n,sp,src); quit("FOUND STRANGE\n");}
  sp--;
  }
  f = (BPLONG_PTR)AR_AR(f);
  }
  }

  track_broken_cps()
  {
  BPLONG_PTR cp,f;

  f = arreg;
  for (;;){
  if (f == (BPLONG_PTR)AR_AR(f)) break;
  cp = (BPLONG_PTR)AR_CPS(f);

  if (!(cp>parea_low_addr && cp<parea_up_addr)){
  printf("Strange CPS AR=%x AR_CPS=%x\n",f,cp);
  quit("Strage");
  }

  f = (BPLONG_PTR)AR_AR(f);
  } 
  }

*/
