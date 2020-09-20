/********************************************************************
 *   File   : table.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: Primitives on table area

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include <stdlib.h>
#include "basic.h"
#include "term.h"
#include "bapi.h"
#include "frame.h"
#include "inst.h"

extern BPLONG table_allocate_code;

#define AllocateCellFromTableArea(ptr) {                                \
        if (table_free_cells_ptr != NULL) {                             \
            ptr = table_free_cells_ptr;                                 \
            table_free_cells_ptr = (BPLONG_PTR)FOLLOW(table_free_cells_ptr+1); \
        } else {                                                        \
            ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, ptr, 2);    \
        }                                                               \
    }

extern FILE *curr_out;

static BPLONG_PTR table_free_cells_ptr = NULL;

/*
  typedef struct {
  BPLONG_PTR low_addr,up_addr,top;
  GTERMS_HTABLE_PTR gterms_htable_ptr;
  } NUMBERED_TERM_AREA_RECORD;
*/
NUMBERED_TERM_AREA_RECORD ta_record;  /* table area record that holds the pointers */
NUMBERED_TERM_AREA_RECORD_PTR ta_record_ptr;

BPLONG_PTR picat_table_maps[NUM_PICAT_TABLE_MAPS];
BPLONG picat_table_map_ids[NUM_PICAT_TABLE_MAPS];

/* used for hash-consing for the table area */
/*
  typedef struct {
  BPLONG_PTR htable; 
  BPLONG size;     
  BPLONG count;    
  } GTERMS_HTABLE;
*/
GTERMS_HTABLE ta_gterms_htable;
GTERMS_HTABLE_PTR ta_gterms_htable_ptr;

/* this is called only once, in "init.c" */
void init_table_area() {
    int i, success;
    ta_gterms_htable_ptr = &ta_gterms_htable;
    ta_record_ptr = &ta_record;
    ta_record_ptr->low_addr = NULL;
    ta_record_ptr->gterms_htable_ptr = ta_gterms_htable_ptr;
    ta_record_ptr->num_expansions = 0;

    subgoalTable = (BPLONG_PTR)malloc(subgoalTableBucketSize*sizeof(BPLONG));
    if (subgoalTable == NULL) {
        myquit(OUT_OF_MEMORY, "tb");
    }
    for (i = 0; i < subgoalTableBucketSize; i++) {
        FOLLOW(subgoalTable+i) = (BPLONG)NULL;
    }
    /* allocate the first block */
    ADD_NEW_NUMBERED_TERM_AREA_BLOCK(ta_record_ptr, success);
    if (!success) {
        myquit(OUT_OF_MEMORY, "tb");
    }
    allocate_gterms_htable(ta_gterms_htable_ptr, 7919);
    table_free_cells_ptr = NULL;
}

BPLONG table_area_size() {
    BPLONG_PTR block_low_addr, subgoal_entry, answerTable;
    BPLONG size, i;

    //printf("table_area_size \n");
    size = 0;
    block_low_addr = ta_record_ptr->low_addr;
    while (block_low_addr != NULL) {
        size += NUMBERED_TERM_BLOCK_SIZE;
        block_low_addr = (BPLONG_PTR)FOLLOW(block_low_addr);
    }
    size += subgoalTableBucketSize;
    for (i = 0; i < subgoalTableBucketSize; i++) {
        subgoal_entry = (BPLONG_PTR)subgoalTable[i];
        while (subgoal_entry != NULL) {
            answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
            if (answerTable != NULL && ((BPLONG)answerTable & 0x1) == 0) {  /* answer table exists only when there are two or more answers */
                size += ANSWERTABLE_RECORD_SIZE+ANSWERTABLE_BUCKET_SIZE(answerTable);
            }
            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
    }
    size += gterms_htable_num_of_occupied_slots(ta_record_ptr->gterms_htable_ptr);

    size += table_maps_buckets_size();

    return size;
}

BPLONG table_area_notin_use() {
    if (ta_record_ptr->low_addr != NULL) {
        return (ta_record_ptr->up_addr - ta_record_ptr->top + 1);
    } else
        return 0;
}

int table_area_num_expansions() {
    return ta_record_ptr->num_expansions;
}

int c_INITIALIZE_TABLE() {
    BPLONG_PTR low_addr, prev_low_addr;
    void init_picat_table_maps();

    bp_exception = (BPLONG)NULL;
    in_critical_region = 0;

#ifdef NOTABLE
#else
    init_subgoal_table();
    /* release the table blocks except the first one */
    low_addr = ta_record_ptr->low_addr;
    prev_low_addr = (BPLONG_PTR)FOLLOW(low_addr);
    while (prev_low_addr != NULL) {
        free(low_addr);
        low_addr = prev_low_addr;
        prev_low_addr = (BPLONG_PTR)FOLLOW(low_addr);
    }
    ta_record_ptr->low_addr = low_addr;
    ta_record_ptr->top = ta_record_ptr->low_addr+1;
    ta_record_ptr->up_addr = ta_record_ptr->low_addr+NUMBERED_TERM_BLOCK_SIZE;
    ta_record_ptr->num_expansions = 0;

    init_gterms_htable(ta_gterms_htable_ptr);
    table_free_cells_ptr = NULL;

    init_picat_table_maps();
#endif
    return BP_TRUE;
}

void subgoal_table_statistics(int *nSubgoals, int *maxGTCollisions, float *aveGTCollisions,
                              int *nAnswers, int *maxATCollisions, float *aveATCollisions,
                              int *nTerms, int *maxTTCollisions, float *aveTTCollisions) {
    BPLONG_PTR subgoal_entry, answerTable, bucket_ptr;
    int i, j, cSubgoals, cAnswers, totalChainedAnswers, totalGTChains, totalATChains, totalTTChains, maxGTChainLen, maxATChainLen, maxTTChainLen;

    cSubgoals = cAnswers = totalChainedAnswers = totalGTChains = totalATChains = totalTTChains = maxGTChainLen = maxATChainLen = maxTTChainLen = 0;
    for (i = 0; i < subgoalTableBucketSize; i++) {
        int gtChainLen = 0;
        subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
        if (subgoal_entry != NULL) totalGTChains++;
        while (subgoal_entry != NULL) {
            cSubgoals++;
            gtChainLen++;
            answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
            if (answerTable != NULL) {
                if (((BPLONG)answerTable & 0x1) == 0) {  /* answer table exists only when there are two or more answers */
                    BPLONG_PTR answer;
                    cAnswers += ANSWERTABLE_COUNT(answerTable);
                    totalChainedAnswers += ANSWERTABLE_COUNT(answerTable);
                    bucket_ptr = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answerTable);
                    for (j = 0; j < ANSWERTABLE_BUCKET_SIZE(answerTable); j++) {
                        int chainLen = 0;
                        answer = (BPLONG_PTR)FOLLOW(bucket_ptr+j);
                        if (answer != NULL) totalATChains++;
                        while (answer != NULL) {
                            chainLen++;
                            answer = (BPLONG_PTR)ANSWER_NEXT_IN_CHAIN(answer);
                        }
                        if (maxATChainLen < chainLen)
                            maxATChainLen = chainLen;
                    }
                } else {
                    cAnswers++;
                }
            }
            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
        if (maxGTChainLen < gtChainLen) maxGTChainLen = gtChainLen;
    }
    *nSubgoals = cSubgoals;
    *maxGTCollisions = maxGTChainLen;
    *aveGTCollisions = (float)cSubgoals/totalGTChains;

    *nAnswers = cAnswers;
    *maxATCollisions = maxATChainLen;
    if (totalATChains != 0)
        *aveATCollisions = (float)totalChainedAnswers/totalATChains;
    else
        *aveATCollisions = 0.0;

    gterms_table_statistics(ta_gterms_htable_ptr, nTerms, maxTTCollisions, aveTTCollisions);
}

void init_subgoal_table() {
    BPLONG i;
    BPLONG_PTR subgoal_entry, answerTable, bucket_ptr;

    for (i = 0; i < subgoalTableBucketSize; i++) {
        subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
        while (subgoal_entry != NULL) {
            answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
            if (answerTable != NULL && ((BPLONG)answerTable & 0x1) == 0) {  /* answer table exists only when there are two or more answers */
                bucket_ptr = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answerTable);
                if (bucket_ptr != NULL) {
                    free(bucket_ptr);
                }
                free(answerTable);
            }
            FOLLOW(subgoalTable+i) = (BPLONG)NULL;
            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
    }
}

/*
  both t1 and t2 are numbered terms in the table area.
*/
int identicalTabledTerms(BPLONG t1, BPLONG t2) {
    BPLONG i, arity, op1, op2;

beginning:
    if (t1 == t2) return 1;
    switch(TAG(t1)) {
    case REF:
    case ATM: return 0;
    case LST:
        if (IsNumberedVar(t1) || IsNumberedVar(t2)) return 0;
        UNTAG_ADDR(t1); UNTAG_ADDR(t2);
        if (FOLLOW((BPLONG_PTR)t1-2) != FOLLOW((BPLONG_PTR)t2-2)) return 0;
        op1 = *(BPLONG_PTR)t1;
        op2 = *(BPLONG_PTR)t2;
        if (op1 != op2 && !identicalTabledTerms(op1, op2)) return 0;
        t1 = *((BPLONG_PTR)t1+1);
        t2 = *((BPLONG_PTR)t2+1);
        goto beginning;
    case STR:
        if (!ISSTRUCT(t2)) return 0;
        UNTAG_ADDR(t1); UNTAG_ADDR(t2);
        if (FOLLOW(t1) != FOLLOW(t2)) return 0;
        if (FOLLOW((BPLONG_PTR)t1-2) != FOLLOW((BPLONG_PTR)t2-2)) return 0;
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(t1));
        for (i = 1; i < arity; i++) {
            op1 = *((BPLONG_PTR)t1+i);
            op2 = *((BPLONG_PTR)t2+i);
            if (op1 != op2 && !identicalTabledTerms(op1, op2)) return 0;
        }
        t1 = *((BPLONG_PTR)t1+arity);
        t2 = *((BPLONG_PTR)t2+arity);
        goto beginning;
    }
    return 0;
}

void match_term_tabledTerm(BPLONG t1, BPLONG t2) {
    BPLONG_PTR top;
    BPLONG i, arity;

lab_match_term_tabledTerm:
    switch (TAG(t1)) {
    case REF:
        NDEREF(t1, lab_match_term_tabledTerm);
        FOLLOW(t1) = unnumberVarTabledTerm(t2);
        PUSHTRAIL(t1);
        return;
    case ATM:
        return;
    case LST:
        if (t1 == t2) return;
        UNTAG_ADDR(t1);
        UNTAG_ADDR(t2);
        match_term_tabledTerm(FOLLOW(t1), FOLLOW(t2));
        t1 = FOLLOW((BPLONG_PTR)t1+1);
        t2 = FOLLOW((BPLONG_PTR)t2+1);
        goto lab_match_term_tabledTerm;
    case STR:
        if (t1 == t2) return;
        if (t1 < 0) {
            unify(t1, unnumberVarTabledTerm(t2));
            return;
        }
        UNTAG_ADDR(t1);
        UNTAG_ADDR(t2);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(t1));
        for (i = 1; i < arity; i++) {
            match_term_tabledTerm(FOLLOW((BPLONG_PTR)t1+i), FOLLOW((BPLONG_PTR)t2+i));
        }
        t1 = FOLLOW((BPLONG_PTR)t1+arity);
        t2 = FOLLOW((BPLONG_PTR)t2+arity);
        goto lab_match_term_tabledTerm;
    }
}

/* make sure there is enough space on the heap befor calling this function */
BPLONG unnumberVarTabledTerm(BPLONG term) {
    BPLONG_PTR ptr, term_ptr;
    BPLONG varNo;
    BPLONG i, arity;
    SYM_REC_PTR sym_ptr;

    switch (TAG(term)) {
    case REF:  /* impossible */

    case ATM: return term;

    case LST:
        if (IsNumberedVar(term)) {
            varNo = INTVAL(term);
            if (varNo > global_unnumbervar_max) {
                global_unnumbervar_max = varNo;
                global_unnumbervar_watermark = global_unnumbervar_ptr-global_unnumbervar_max;
                FOLLOW(global_unnumbervar_ptr-varNo) = (BPLONG)heap_top;
                NEW_HEAP_FREE;
                return FOLLOW(global_unnumbervar_ptr-varNo);
            } else {
                return FOLLOW(global_unnumbervar_ptr-varNo);
            }
        } else {
            term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            if (FOLLOW(term_ptr-2) & TOP_BIT) {  /* is ground */
                return term;
            }
            ptr = heap_top; heap_top += 2;

            if (global_unnumbervar_watermark-heap_top <= LARGE_MARGIN) {
                myquit(STACK_OVERFLOW, "uv");
            }
            FOLLOW(ptr) = unnumberVarTabledTerm(FOLLOW(term_ptr));
            FOLLOW(ptr+1) = unnumberVarTabledTerm(FOLLOW(term_ptr+1));
            return ADDTAG(ptr, LST);
        }

    case STR:
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        if (FOLLOW(term_ptr-2) & TOP_BIT) {  /* is ground */
            return term;
        }
        sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
        arity = GET_ARITY(sym_ptr);
        ptr = heap_top; heap_top += arity+1;

        if (global_unnumbervar_watermark-heap_top <= LARGE_MARGIN) {
            myquit(STACK_OVERFLOW, "uv");
        }

        FOLLOW(ptr) = (BPLONG)sym_ptr;
        for (i = 1; i <= arity; i++) {
            FOLLOW(ptr+i) = unnumberVarTabledTerm(*(term_ptr + i));
        }
        return ADDTAG(ptr, STR);
    default:
        return(0);  /* impossible to come here */
    }
}

/******************* SUBGOAL TABLE ********************/
void expandSubgoalTable() {
    BPLONG new_htable_size, old_htable_size, i, index;
    BPLONG_PTR new_htable, old_htable, next_subgoal_entry;

    old_htable_size = subgoalTableBucketSize;
    old_htable = subgoalTable;
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

    new_htable = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_htable_size);
    if (new_htable == NULL) return;  /* stop expanding */
    for (i = 0; i < new_htable_size; i++) {
        new_htable[i] = (BPLONG)NULL;
    }
    for (i = 0; i < old_htable_size; i++) {
        BPLONG_PTR subgoal_entry = (BPLONG_PTR)old_htable[i];
        while (subgoal_entry != NULL) {
            next_subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
            index = hashval_of_tabled_subgoal(subgoal_entry) % new_htable_size;
            GT_NEXT(subgoal_entry) = new_htable[index];
            new_htable[index] = (BPLONG)subgoal_entry;
            subgoal_entry = next_subgoal_entry;
        }
    }
    free(old_htable);
    subgoalTableBucketSize = new_htable_size;
    subgoalTable = new_htable;
}

BPLONG_PTR lookupSubgoalTable(BPLONG_PTR stack_arg_ptr, int arity, SYM_REC_PTR sym_ptr, int mode_bits, int nt_last_arg) {
    BPLONG_PTR entryPtrPtr0, entryPtr, thisEntryPtr;
    BPLONG_PTR subgoal_arg_ptr, this_subgoal_arg_ptr;
    BPLONG i, arity1;
    BPLONG hcode0, hcode, subgoal_record_size;
    BPLONG_PTR trail_top0, old_table_top;
    BPLONG initial_diff0;
    BPULONG tmp_mode_bits;
    //    void printSubgoalTableEntry();

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;

    /* before numbering the tabled call */
    PREPARE_NUMBER_TERM(0);
    subgoal_record_size = arity+GT_RECORD_SIZE;
    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, thisEntryPtr, subgoal_record_size);
    if (thisEntryPtr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return (BPLONG_PTR)BP_ERROR;
    }

    old_table_top = thisEntryPtr;

    this_subgoal_arg_ptr = GT_ARG_ADDR(thisEntryPtr);
    hcode0 = ((BPLONG)sym_ptr & HASH_BITS) >> 2;
    if (numberVarCopySubgoalArgsToTableArea(stack_arg_ptr, this_subgoal_arg_ptr, arity, hcode0, &hcode) == BP_ERROR)
        return (BPLONG_PTR)BP_ERROR;

    if (mode_bits != 0) {  /* check mode */
        tmp_mode_bits = mode_bits;
        for (i = 0; i < arity; i++) {
            if ((tmp_mode_bits & 0x1L) == 1) {
                BPLONG t1;
                t1 = FOLLOW(this_subgoal_arg_ptr+i);
                if (!IsNumberedVar(t1)) {
                    bp_exception = output_mode_error;
                    return (BPLONG_PTR)BP_ERROR;
                }
            }
            tmp_mode_bits = tmp_mode_bits >> 1;
        }
    }

    //  printf("lookup "); write_term(*(stack_arg_ptr));        printf("hcode=%x\n",hcode);

    entryPtrPtr0 = subgoalTable + (hcode % subgoalTableBucketSize);
    entryPtr = (BPLONG_PTR)FOLLOW(entryPtrPtr0);

    arity1 = arity-1;
    while (entryPtr != NULL) {  /* lookup */
        if ((SYM_REC_PTR)GT_SYM(entryPtr) != sym_ptr) goto lab_fail1;
        subgoal_arg_ptr = GT_ARG_ADDR(entryPtr);
        for (i = 0; i < arity1; i++) {
            BPLONG t1, t2;
            t1 = FOLLOW(this_subgoal_arg_ptr+i);
            t2 = FOLLOW(subgoal_arg_ptr+i);
            if (t1 != t2 && !identicalTabledTerms(t1, t2)) goto lab_fail1;
        }
        if (nt_last_arg == 0) {
            BPLONG t1, t2;
            t1 = FOLLOW(this_subgoal_arg_ptr+arity1);
            t2 = FOLLOW(subgoal_arg_ptr+arity1);
            if (t1 != t2 && !identicalTabledTerms(t1, t2)) goto lab_fail1;
        }
        thisEntryPtr = entryPtr;
        if (ta_record_ptr->top == old_table_top+subgoal_record_size) {
            ta_record_ptr->top = old_table_top;
        }
        goto lookup_end;
    lab_fail1:
        entryPtr = (BPLONG_PTR)GT_NEXT(entryPtr);
    }
    /* not found, register the subgoal now */
    InitializeSubgoalTableEntry(thisEntryPtr, sym_ptr);
    //  SET_SUBGOAL_ANS_REVISED(entryPtr);
    GT_NEXT(thisEntryPtr) = FOLLOW(entryPtrPtr0);
    FOLLOW(entryPtrPtr0) = (BPLONG)thisEntryPtr;
    subgoalTableEntriesCount++;

    //    printSubgoalTableEntry(thisEntryPtr);
    if (2*subgoalTableEntriesCount > subgoalTableBucketSize) {
        expandSubgoalTable();
    }

lookup_end:
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    //  printf("<=lookup\n");
    return thisEntryPtr;
}

int numberVarCopySubgoalArgsToTableArea(BPLONG_PTR stack_arg_ptr, BPLONG_PTR table_arg_ptr, int arity, BPLONG hcode0, BPLONG_PTR hcode_ptr) {
    BPLONG i;
    BPLONG term;
    BPLONG hcode_sum, this_hcode;

    hcode_sum = hcode0;
    if (arity == 0) {
        *hcode_ptr = hcode_sum;
        return BP_TRUE;
    }

    term = FOLLOW(stack_arg_ptr);
    if (TAG(term) == ATM) {
        this_hcode = ((term & HASH_BITS) >> 2);
        FOLLOW(table_arg_ptr) = term;
    } else {
        BPLONG term_cp, this_ground_flag;
        this_ground_flag = TOP_BIT;
        term_cp = numberVarCopyToTableArea(ta_record_ptr, term, &this_hcode, &this_ground_flag);
        if (term_cp == BP_ERROR) return BP_ERROR;
        FOLLOW(table_arg_ptr) = term_cp;
        if (this_ground_flag == TOP_BIT) FOLLOW(stack_arg_ptr) = term_cp;
    }


    hcode_sum += this_hcode;
    //  printf("lookup(cp) hcode1=%x\n",hcode_sum);
    for (i = 1; i < arity; i++) {

        term = FOLLOW(stack_arg_ptr-i);
        if (TAG(term) == ATM) {
            this_hcode = ((term & HASH_BITS) >> 2);
            FOLLOW(table_arg_ptr+i) = term;
        } else {
            BPLONG term_cp, this_ground_flag;
            this_ground_flag = TOP_BIT;
            term_cp = numberVarCopyToTableArea(ta_record_ptr, term, &this_hcode, &this_ground_flag);
            if (term_cp == BP_ERROR) return BP_ERROR;
            FOLLOW(table_arg_ptr+i) = term_cp;
            if (this_ground_flag == TOP_BIT) FOLLOW(stack_arg_ptr-i) = term_cp;
        }

        if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode_sum);
    }
    *hcode_ptr = (hcode_sum & HASH_BITS);
    return BP_TRUE;
}

int numberVarCopyAnswerArgsToTableArea(BPLONG_PTR stack_arg_ptr, BPLONG_PTR table_arg_ptr, int arity, BPLONG_PTR hcode_ptr) {
    BPLONG i;
    BPLONG term, term_cp;
    BPLONG hcode_sum, this_hcode, this_ground_flag;

    if (arity == 0) {
        *hcode_ptr = 0;
        return BP_TRUE;
    }

    term = FOLLOW(stack_arg_ptr);
    if (TAG(term) == ATM) {
        hcode_sum = ((term & HASH_BITS) >> 2);
        FOLLOW(table_arg_ptr) = term;
    } else {
        term_cp = numberVarCopyToTableArea(ta_record_ptr, term, &hcode_sum, &this_ground_flag);
        if (term_cp == BP_ERROR) return BP_ERROR;
        FOLLOW(table_arg_ptr) = term_cp;
    }

    for (i = 1; i < arity; i++) {

        term = FOLLOW(stack_arg_ptr-i);
        if (TAG(term) == ATM) {
            this_hcode = ((term & HASH_BITS) >> 2);
            FOLLOW(table_arg_ptr+i) = term;
        } else {
            term_cp = numberVarCopyToTableArea(ta_record_ptr, term, &this_hcode, &this_ground_flag);
            if (term_cp == BP_ERROR) return BP_ERROR;
            FOLLOW(table_arg_ptr+i) = term_cp;
        }

        if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode_sum);
    }
    *hcode_ptr = (hcode_sum & HASH_BITS);
    return BP_TRUE;
}

BPLONG hashval_of_tabled_subgoal(BPLONG_PTR subgoal_entry) {
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR table_arg_ptr;
    int arity, i;
    BPLONG hcode_sum, this_hcode;

    sym_ptr = (SYM_REC_PTR)GT_SYM(subgoal_entry);
    arity = GET_ARITY(sym_ptr);
    table_arg_ptr = GT_ARG_ADDR(subgoal_entry);

    hcode_sum = ((BPLONG)sym_ptr & HASH_BITS) >> 2;
    if (arity == 0) return hcode_sum;

    hcode_sum += hashval_of_numbered_term(FOLLOW(table_arg_ptr));
    for (i = 1; i < arity; i++) {
        this_hcode = hashval_of_numbered_term(FOLLOW(table_arg_ptr+i));
        if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode_sum);
    }
    return (hcode_sum & HASH_BITS);
}

BPLONG hashval_of_tabled_answer(BPLONG_PTR answer, int arity) {
    BPLONG_PTR table_arg_ptr;
    int i;
    BPLONG hcode_sum, this_hcode;

    if (arity == 0) return 0;
    table_arg_ptr = ANSWER_ARG_ADDR(answer);
    hcode_sum = hashval_of_numbered_term(FOLLOW(table_arg_ptr));
    for (i = 1; i < arity; i++) {
        this_hcode = hashval_of_numbered_term(FOLLOW(table_arg_ptr+i));
        if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode_sum);
    }
    return (hcode_sum & HASH_BITS);
}

BPLONG hashval_of_numbered_term(BPLONG term) {
    BPLONG_PTR term_ptr;

    if (TAG(term) == ATM) {
        return ((term & HASH_BITS) >> 2);
    }
    if (IsNumberedVar(term)) {
        return 0;
    }
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    return (FOLLOW(term_ptr-2) & HASH_BITS);
}

int isGroundNumberedTerm(BPLONG term) {
    BPLONG_PTR term_ptr;

    if (TAG(term) == ATM) {
        return 1;
    }
    if (IsNumberedVar(term)) {
        return 0;
    }
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    return (FOLLOW(term_ptr-2) & TOP_BIT);
}

/******************************************************************************/
void init_gterms_htable(GTERMS_HTABLE_PTR gterms_htable_ptr) {
    int i, size;
    BPLONG_PTR htable;

    size = gterms_htable_ptr->size;
    htable = gterms_htable_ptr->htable;
    for (i = 0; i < size; i++) {
        FOLLOW(htable+i) = (BPLONG)NULL;
    }
    gterms_htable_ptr->count = 0;
}

int identical_numbered_gterms(BPLONG op1, BPLONG op2) {
lab_start:
    if (op1 == op2) return 1;
    if (ISLIST(op1) && ISLIST(op2)) {
        UNTAG_ADDR(op1);
        UNTAG_ADDR(op2);
        if (FOLLOW((BPLONG_PTR)op1-2) != FOLLOW((BPLONG_PTR)op2-2)) return 0;  /* compare hashcode */
        if (!identical_numbered_gterms(FOLLOW((BPLONG_PTR)op1), FOLLOW((BPLONG_PTR)op2))) return 0;
        op1 = FOLLOW((BPLONG_PTR)op1+1);
        op2 = FOLLOW((BPLONG_PTR)op2+1);
        goto lab_start;
    }
    if (ISSTRUCT(op1) && ISSTRUCT(op2)) {
        BPLONG arity, i;

        UNTAG_ADDR(op1);
        UNTAG_ADDR(op2);
        if (FOLLOW(op1) != FOLLOW(op2)) return 0;
        if (FOLLOW((BPLONG_PTR)op1-2) != FOLLOW((BPLONG_PTR)op2-2)) return 0;  /* compare hashcode */
        arity = GET_ARITY((SYM_REC_PTR)(FOLLOW(op1)));
        for (i = 1; i < arity; i++)
            if (!identical_numbered_gterms(*((BPLONG_PTR)op1 + i), *((BPLONG_PTR)op2 + i))) return 0;
        op1 = FOLLOW((BPLONG_PTR)op1 + arity);
        op2 = FOLLOW((BPLONG_PTR)op2 + arity);
        goto lab_start;
    }
    return 0;
}

void gterms_table_statistics(GTERMS_HTABLE_PTR gterms_htable_ptr, int *nTerms, int *maxTTCollisions, float *aveTTCollisions) {
    BPLONG_PTR htable;
    int i, size, cTerms, maxChainLen, totalChains;

    cTerms = maxChainLen = totalChains = 0;
    size = gterms_htable_ptr->size;
    htable = gterms_htable_ptr->htable;
    for (i = 0; i < size; i++) {
        int chainLen;
        BPLONG term;

        chainLen = 0;
        term = htable[i];
        if (term != (BPLONG)NULL) totalChains++;
        while (term != (BPLONG)NULL) {
            BPLONG_PTR term_ptr;
            term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            chainLen++;
            cTerms++;
            term = FOLLOW(term_ptr-1);
        }
        if (maxChainLen < chainLen) maxChainLen = chainLen;
    }
    *nTerms = cTerms;
    *maxTTCollisions = maxChainLen;
    if (cTerms != 0)
        *aveTTCollisions = (float)cTerms/totalChains;
    else
        *aveTTCollisions = 0;
}

BPLONG gterms_htable_num_of_occupied_slots(GTERMS_HTABLE_PTR gterms_htable_ptr) {
    return gterms_htable_ptr->size;
    /*
      BPLONG i,size, count;
      count = 0;
      size = gterms_htable_ptr->size;
      htable = gterms_htable_ptr->htable;
      for (i=0;i<size;i++){
      if (htable[i] != (BPLONG)NULL) count++;
      }
      return count;
    */
}

void allocate_gterms_htable(GTERMS_HTABLE_PTR gterms_htable_ptr, int size) {
    BPLONG_PTR htable;

    htable = malloc(size*sizeof(BPLONG));
    if (htable == NULL) {
        myquit(OUT_OF_MEMORY, "fa");
    }
    gterms_htable_ptr->size = size;
    gterms_htable_ptr->htable = htable;
    init_gterms_htable(gterms_htable_ptr);
}

/* term is either a ground list or a ground structure,
   let ptr points to the term. The location (ptr-1) is used for hash-chaining
   and (ptr-2) is used to hold hash code. If the top-bit of a stored hash code 
   is 1, then the term is ground.
*/
BPLONG register_gterms_htable(GTERMS_HTABLE_PTR gterms_htable_ptr, BPLONG term, BPLONG hcode) {
    BPLONG term_in_table;
    BPLONG_PTR term_ptr, slot_addr;
    BPLONG tag_hcode;

    tag_hcode = (TOP_BIT | hcode);

    slot_addr = gterms_htable_ptr->htable + hcode%gterms_htable_ptr->size;
    term_in_table = FOLLOW(slot_addr);
    while (term_in_table != (BPLONG)NULL) {
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term_in_table);
        if (FOLLOW(term_ptr-2) == tag_hcode && identical_numbered_gterms(term, term_in_table)) {
            // printf("found"); write_term(term);printf("\n");
            return term_in_table;
        }
        term_in_table = FOLLOW(term_ptr-1);
    }
    /* not found, insert */
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    FOLLOW(term_ptr-1) = FOLLOW(slot_addr);
    FOLLOW(slot_addr) = term;
    gterms_htable_ptr->count++;
    if (2*gterms_htable_ptr->count > gterms_htable_ptr->size) {
        expand_gterms_htable(gterms_htable_ptr);
    }
    return term;
}

void expand_gterms_htable(GTERMS_HTABLE_PTR gterms_htable_ptr) {
    BPLONG new_htable_size, old_htable_size, i;
    BPLONG_PTR new_htable, old_htable;

    old_htable_size = gterms_htable_ptr->size;
    old_htable = gterms_htable_ptr->htable;
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

    //  printf("faa expand %d\n",new_htable_size);

    new_htable = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_htable_size);
    if (new_htable == NULL) return;  /* stop expanding */
    for (i = 0; i < new_htable_size; i++) {
        new_htable[i] = (BPLONG)NULL;
    }
    for (i = 0; i < old_htable_size; i++) {
        BPLONG term;
        term = old_htable[i];
        while (term != (BPLONG)NULL) {
            BPLONG_PTR term_ptr;
            BPLONG next_term, bucket_no;

            term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            next_term = FOLLOW(term_ptr-1);
            bucket_no = (FOLLOW(term_ptr-2) & HASH_BITS) % new_htable_size;  /* insert this term into the new table */
            FOLLOW(term_ptr-1) = new_htable[bucket_no];
            new_htable[bucket_no] = term;

            term = next_term;
        }
    }
    free(old_htable);
    gterms_htable_ptr->size = new_htable_size;
    gterms_htable_ptr->htable = new_htable;
}

/*
  Copy term (numbered) to the table area and computes its hash code. The copy (let its address be ptr)
  has two extra cells preceeding it if it is compound, one (ptr-2) storing the hash code and the 
  other (ptr-1) being used to connect to the next term on the hash chain in gterms_htable. This function 
  must be changed accordingly whenever bp_hashval (in "mic.c") is changed.
*/
BPLONG numberVarCopyToTableArea(NUMBERED_TERM_AREA_RECORD_PTR area_record_ptr, BPLONG term, BPLONG_PTR hcode_ptr, BPLONG_PTR ground_flag_ptr)
{
    BPLONG_PTR term_ptr, dest_ptr;
    BPLONG_PTR top;
    BPLONG term_cp, term_cp1, this_hcode, this_ground_flag;
    BPLONG i, arity, size, hcode_sum;
    SYM_REC_PTR sym_ptr;

l_number_var_copy_faa:
    switch(TAG(term)) {
    case REF:
        NDEREF(term, l_number_var_copy_faa);
        ASSIGN_TRAIL_VALUE(term, NumberVar(global_var_num));
        global_var_num++;
        *hcode_ptr = 0;
        *ground_flag_ptr = 0;
        return FOLLOW(term);

    case ATM:
        *hcode_ptr = ((term & HASH_BITS) >> 2);
        return term;

    case LST:
        if (IsNumberedVar(term)) {
            *hcode_ptr = 0;
            *ground_flag_ptr = 0;
            return term;
        } else {
            return numberVarCopyListToTableArea(area_record_ptr, term, hcode_ptr, ground_flag_ptr);
        }

    case STR:
        this_ground_flag = TOP_BIT;
        if (term > 0) {  /* not susp_var */
            term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
            if (!IS_HEAP_REFERENCE(term_ptr)) {  /* term_ptr may point to a ground term in table area,*/
                *hcode_ptr = (FOLLOW(term_ptr-2) & HASH_BITS);
                return term;
            }
            sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
            hcode_sum = (((BPLONG)sym_ptr & HASH_BITS) >> 2);
            arity = GET_ARITY(sym_ptr);
            size = arity+3;
            ALLOCATE_FROM_NUMBERED_TERM_AREA(area_record_ptr, dest_ptr, size);
            if (dest_ptr == NULL) {
                bp_exception = et_OUT_OF_MEMORY;
                return BP_ERROR;
            }
            dest_ptr += 2;
            FOLLOW(dest_ptr) = FOLLOW(term_ptr);

            /* copy first argument */
            term = FOLLOW(term_ptr+1);
            if (TAG(term) == ATM) {
                this_hcode = ((term & HASH_BITS) >> 2);
                term_cp = term;
            } else {
                term_cp = numberVarCopyToTableArea(area_record_ptr, FOLLOW(term_ptr+1), &this_hcode, &this_ground_flag);
                if (term_cp == BP_ERROR) return BP_ERROR;
            }
            FOLLOW(dest_ptr+1) = term_cp;
            hcode_sum += this_hcode;
            for (i = 2; i <= arity; i++) {
                term = FOLLOW(term_ptr+i);
                if (TAG(term) == ATM) {
                    this_hcode = ((term & HASH_BITS) >> 2);
                    term_cp = term;
                } else {
                    term_cp = numberVarCopyToTableArea(area_record_ptr, FOLLOW(term_ptr+i), &this_hcode, &this_ground_flag);
                    if (term_cp == BP_ERROR) return BP_ERROR;
                }
                FOLLOW(dest_ptr+i) = term_cp;
                if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode_sum);
            }
            hcode_sum = (hcode_sum & HASH_BITS);
            *hcode_ptr = hcode_sum;
            FOLLOW(dest_ptr-2) = (this_ground_flag | hcode_sum);  /* memorize the hash code here */
            term_cp = ADDTAG(dest_ptr, STR);
            if (this_ground_flag == 0) {
                *ground_flag_ptr = 0;
                return term_cp;
            }
            term_cp1 = register_gterms_htable(area_record_ptr->gterms_htable_ptr, term_cp, hcode_sum);
            if (term_cp1 != term_cp) {
                dest_ptr -= 2;
                if (area_record_ptr->top == dest_ptr+size) {
                    area_record_ptr->top = dest_ptr;  /* the copy is not needed anymore */
                }
            }
            return term_cp1;
        } else {  /* SUSP var */
            dest_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
            PUSHTRAILC_ATOMIC(dest_ptr, term);
            FOLLOW(dest_ptr) = NumberVar(global_var_num);
            global_var_num++;
            *hcode_ptr = 0;
            *ground_flag_ptr = 0;
            return FOLLOW(dest_ptr);
        }
    }
}

/*
  Iteratively copy a list (to avoid native stack overflow). In the first pass, pointers are reversed, and 
  in the second pass, pointers are reversed back while hash code is computed and the term is copied.
*/
BPLONG numberVarCopyListToTableArea(NUMBERED_TERM_AREA_RECORD_PTR area_record_ptr, BPLONG term, BPLONG_PTR hcode_ptr, BPLONG_PTR ground_flag_ptr) {
    BPLONG prev_term, car, cdr, car_cp, cdr_cp, term_cp, term_cp1, this_hcode, hcode_sum, tmp, this_ground_flag;
    BPLONG_PTR term_ptr, dest_ptr;

    prev_term = nil_sym;
    this_ground_flag = TOP_BIT;

    //  printf("copy_list "); write_term(term); printf("\n");

lab_reverse:  /* ugly!! but has to consider the case when the tail is a free variable, see also bp_hashval_list() in "mic.c" */
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    if (!IS_HEAP_REFERENCE(term_ptr)) {  /* must be a ground term in the table area */
        hcode_sum = (FOLLOW(term_ptr-2) & HASH_BITS);
        if (prev_term == nil_sym) {
            FOLLOW(hcode_ptr) = hcode_sum;
            return term;
        }
        /* move one node backward */
        tmp = prev_term;
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tmp);
        prev_term = FOLLOW(term_ptr+1);
        FOLLOW(term_ptr+1) = term;
        cdr = cdr_cp = term;
        term = tmp;
        goto lab_reverse_back;
    }

    car = FOLLOW(term_ptr);
    if (TAG(car) != ATM) numberVarTermOpt(car);  /* number vars first, don't copy now */
    cdr = FOLLOW(term_ptr+1);
lab_test_cdr:
    if (ISLIST(cdr)) {
        FOLLOW(term_ptr+1) = prev_term;
        prev_term = term;
        term = cdr;
        goto lab_reverse;
    } else if (ISREF(cdr) && !ISFREE(cdr)) {  /* shorten the reference chain */
        BPLONG_PTR tmp_ptr = term_ptr+1;
        PUSHTRAIL_H_NONATOMIC(tmp_ptr, cdr);
        FOLLOW(tmp_ptr) = FOLLOW(cdr);
        cdr = FOLLOW(cdr);
        goto lab_test_cdr;
    }

    if (cdr == nil_sym) {
        hcode_sum = ((cdr & HASH_BITS) >> 2);
        cdr_cp = cdr;
    } else {
        cdr_cp = numberVarCopyToTableArea(area_record_ptr, cdr, &hcode_sum, &this_ground_flag);
    }

    /* the original list has been reversed except for the last cons,
       now reverse it back while computing hash code and copying it.
    */
    //  printf("=>lab_reverse_back\n");
lab_reverse_back:
    ALLOCATE_FROM_NUMBERED_TERM_AREA(area_record_ptr, dest_ptr, 4);
    if (dest_ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    dest_ptr += 2;
    car = FOLLOW(term_ptr);
    if (TAG(car) == ATM) {
        this_hcode = ((car & HASH_BITS) >> 2);
        car_cp = car;
    } else {
        car_cp = numberVarCopyToTableArea(area_record_ptr, car, &this_hcode, &this_ground_flag);
        if (car_cp == BP_ERROR) return BP_ERROR;
    }
    if (this_hcode != 0) {
        hcode_sum = this_hcode + hcode_sum*MULTIPLIER+1;
        hcode_sum = hcode_sum & HASH_BITS;
    }

    FOLLOW(dest_ptr) = car_cp;
    FOLLOW(dest_ptr+1) = cdr_cp;
    term_cp = ADDTAG(dest_ptr, LST);
    FOLLOW(dest_ptr-2) = (this_ground_flag | hcode_sum);  /* memorize the hash code here */
    if (this_ground_flag != 0) {
        term_cp1 = register_gterms_htable(area_record_ptr->gterms_htable_ptr, term_cp, hcode_sum);
        if (term_cp1 != term_cp) {
            term_cp = term_cp1;
            dest_ptr -= 2;
            if (area_record_ptr->top == dest_ptr+4) {
                area_record_ptr->top = dest_ptr;
            }
        }
    }
    cdr = term;
    term = prev_term;
    if (ISLIST(term)) {
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        prev_term = FOLLOW(term_ptr+1);
        FOLLOW(term_ptr+1) = cdr;
        cdr_cp = term_cp;
        goto lab_reverse_back;
    }

    FOLLOW(hcode_ptr) = hcode_sum;
    if (this_ground_flag == 0) {
        *ground_flag_ptr = 0;
    }
    return term_cp;
}


/******************************************************************************/
void propagate_scc_root(BPLONG_PTR fp, BPLONG_PTR subgoal_entry, BPLONG_PTR scc_root, BPLONG_PTR scc_root_ar) {
    BPLONG_PTR subgoal_entry0, scc_root_ar0, scc_root0, list_ptr;

    //  printf("fp = %lx, scc_root = %lx, scc_root_ar = %lx\n", fp, scc_root, scc_root_ar);

    while (!IS_TABLE_FRAME(fp)) {
        if (fp >= scc_root_ar) return;
        fp = (BPLONG_PTR)AR_AR(fp);  /* get the nearest tabled ancestor */
    }
    subgoal_entry0 = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(fp);
    if (SUBGOAL_ANS_IS_REVISED(subgoal_entry)) SET_SUBGOAL_ANS_REVISED(subgoal_entry0);
    scc_root0 = (BPLONG_PTR)GT_SCC_ROOT(subgoal_entry0);

    //  printf("PROP FROM "); print_subgoal_entry_only(subgoal_entry);printf(" TO "); print_subgoal_entry_only(subgoal_entry0); printf("\n");

    if (scc_root == scc_root0) return;
    scc_root_ar0 = (BPLONG_PTR)GT_TOP_AR(scc_root0);

    if (scc_root_ar > scc_root_ar0) {  /* add subgoal_entry0 into scc_root */
        AllocateCellFromTableArea(list_ptr);
        if (list_ptr == NULL) {
            myquit(OUT_OF_MEMORY, "ta");
        }

        FOLLOW(list_ptr) = (BPLONG)subgoal_entry0;
        FOLLOW(list_ptr+1) = GT_SCC_ELMS(scc_root);
        GT_SCC_ELMS(scc_root) = (BPLONG)list_ptr;
        GT_SCC_ROOT(subgoal_entry0) = (BPLONG)scc_root;
    }
}

/* initialize the subgoal and all of its dependents */
void initialize_scc_elms(BPLONG_PTR subgoal_entry) {
    BPLONG_PTR ptr, entry;

    if (GT_STATE(subgoal_entry) == 0) return;  /* initialized already */
    GT_STATE(subgoal_entry) = 0;

    ptr = (BPLONG_PTR)GT_SCC_ELMS(subgoal_entry);
    while (ptr != NULL) {
        entry = (BPLONG_PTR)FOLLOW(ptr);
        initialize_scc_elms(entry);
        ptr = (BPLONG_PTR)FOLLOW(ptr+1);
    }
}

void complete_scc_elms(BPLONG_PTR subgoal_entry) {
    BPLONG_PTR ptr, entry, tmp_ptr;

    if (GT_TOP_AR(subgoal_entry) == SUBGOAL_COMPLETE) return;  /* set already */
    GT_TOP_AR(subgoal_entry) = SUBGOAL_COMPLETE;
    /* fprintf(curr_out,"COMPLETE:");print_subgoal_entry_only(subgoal_entry);printf("\n"); */
    ptr = (BPLONG_PTR)GT_SCC_ELMS(subgoal_entry);
    while (ptr != NULL) {
        entry = (BPLONG_PTR)FOLLOW(ptr);
        // printf("   "); print_subgoal_entry_only(entry);
        complete_scc_elms(entry);
        tmp_ptr = (BPLONG_PTR)FOLLOW(ptr+1);
        TABLE_FREE_CELL(ptr);
        ptr = tmp_ptr;
    }
}

void reset_temp_complete_scc_elms(BPLONG_PTR subgoal_entry) {
    BPLONG_PTR ptr, entry, tmp_ptr;

    ptr = (BPLONG_PTR)GT_SCC_ELMS(subgoal_entry);
    while (ptr != NULL) {
        entry = (BPLONG_PTR)FOLLOW(ptr);
        if (entry != NULL && GT_TOP_AR(entry) == SUBGOAL_TEMP_COMPLETE) {
            GT_TOP_AR(entry) = (BPLONG)NULL;
        }
        ptr = (BPLONG_PTR)FOLLOW(ptr+1);
    }
}

/********************** ANSWER TABLE *********************/
BPLONG_PTR addFirstTableAnswer(BPLONG_PTR stack_arg_ptr, int arity) {
    BPLONG_PTR answer;
    BPLONG hcode;
    int size = arity+2;  /* size of an answer record */

    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, answer, size);
    if (answer == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return (BPLONG_PTR)BP_ERROR;
    }

    ANSWER_NEXT_IN_TABLE(answer) = (BPLONG)NULL;
    ANSWER_NEXT_IN_CHAIN(answer) = (BPLONG)NULL;

    PREPARE_NUMBER_TERM(0);
    if (numberVarCopyAnswerArgsToTableArea(stack_arg_ptr, ANSWER_ARG_ADDR(answer), arity, &hcode) == BP_ERROR)
        return (BPLONG_PTR)BP_ERROR;
    return answer;
}

BPLONG_PTR allocateAnswerTable(BPLONG_PTR first_answer, int arity) {
    BPLONG i, index;
    BPLONG_PTR answer_table, bucket_ptr;

    answer_table = (BPLONG_PTR)malloc(ANSWERTABLE_RECORD_SIZE*sizeof(BPLONG));
    if (answer_table == NULL) {
        //        myquit(OUT_OF_MEMORY,"at");
        bp_exception = et_OUT_OF_MEMORY;
        return (BPLONG_PTR)BP_ERROR;
    }
    ANSWERTABLE_FIRST(answer_table) = (BPLONG)first_answer;
    ANSWERTABLE_LAST(answer_table) = (BPLONG)first_answer;
    ANSWERTABLE_BUCKET_SIZE(answer_table) = InitAnswerTableBucketSize;
    ANSWERTABLE_COUNT(answer_table) = 1;
    bucket_ptr = (BPLONG_PTR)malloc(InitAnswerTableBucketSize*sizeof(BPLONG));
    if (bucket_ptr == NULL) {
        // myquit(OUT_OF_MEMORY,"at");
        bp_exception = et_OUT_OF_MEMORY;
        return (BPLONG_PTR)BP_ERROR;
    }
    ANSWERTABLE_BUCKET_PTR(answer_table) = (BPLONG)bucket_ptr;

    for (i = 0; i < InitAnswerTableBucketSize; i++) {
        FOLLOW(bucket_ptr+i) = (BPLONG)NULL;
    }

    index = hashval_of_tabled_answer(first_answer, arity)%InitAnswerTableBucketSize;
    FOLLOW(bucket_ptr+index) = (BPLONG)first_answer;
    /*
      {
      int i;
      for(i=0;i<5;i++)
      printf("%d (%d)\n",i,FOLLOW(answer_table+i));
      }
    */
    return answer_table;
}


int addTableAnswer(BPLONG_PTR stack_arg_ptr, int arity, BPLONG_PTR subgoal_entry) {
    BPLONG_PTR answer_table, this_answer, answer, last_answer, bucket_ptr, this_table_arg_ptr, table_arg_ptr, entryPtr;
    BPLONG_PTR trail_top0, old_table_top;
    BPLONG i, answer_record_size, bucket_size, hcode;

    answer_table = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
    //  initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    PREPARE_NUMBER_TERM(0);

    bucket_ptr = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answer_table);
    bucket_size = ANSWERTABLE_BUCKET_SIZE(answer_table);

    answer_record_size = arity+2;
    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, this_answer, answer_record_size);
    if (this_answer == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }

    old_table_top = this_answer;
    this_table_arg_ptr = ANSWER_ARG_ADDR(this_answer);

    if (numberVarCopyAnswerArgsToTableArea(stack_arg_ptr, this_table_arg_ptr, arity, &hcode) == BP_ERROR)
        return BP_ERROR;

    entryPtr = bucket_ptr+hcode%bucket_size;
    answer = (BPLONG_PTR)FOLLOW(entryPtr);
    while (answer != NULL) {
        table_arg_ptr = ANSWER_ARG_ADDR(answer);
        for (i = 0; i < arity; i++) {
            BPLONG t1, t2;
            t1 = FOLLOW(this_table_arg_ptr+i);
            t2 = FOLLOW(table_arg_ptr+i);
            if (t1 != t2 && !identicalTabledTerms(t1, t2)) goto lab_fail;
        }
        //    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
        //    UNDO_TRAILING; /* variants */
        if (ta_record_ptr->top == old_table_top+answer_record_size) {
            ta_record_ptr->top = old_table_top;
        }
        return BP_FALSE;  /* the producer shoud fail */
    lab_fail:
        answer = (BPLONG_PTR)ANSWER_NEXT_IN_CHAIN(answer);
    }
    //  trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    //  UNDO_TRAILING;
    /*
      printf("produce %s\n",GET_NAME((SYM_REC_PTR)GT_SYM(subgoal_entry)));
      printAnswer(ANSWER_ARG_ADDR(answer),arity);
    */
    ANSWER_NEXT_IN_CHAIN(this_answer) = FOLLOW(entryPtr);
    FOLLOW(entryPtr) = (BPLONG)this_answer;
    ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)NULL;
    last_answer = (BPLONG_PTR)ANSWERTABLE_LAST(answer_table);
    ANSWER_NEXT_IN_TABLE(last_answer) = (BPLONG)this_answer;
    ANSWERTABLE_LAST(answer_table) = (BPLONG)this_answer;
    ANSWERTABLE_COUNT(answer_table) = ANSWERTABLE_COUNT(answer_table)+1;

    if (2*ANSWERTABLE_COUNT(answer_table) > ANSWERTABLE_BUCKET_SIZE(answer_table)) {
        expandAnswerTable(answer_table, arity);
    }

    SET_SUBGOAL_ANS_REVISED(subgoal_entry);
    return BP_TRUE;
}

void expandAnswerTable(BPLONG_PTR answer_table, int arity) {
    BPLONG new_htable_size, old_htable_size, i, index;
    BPLONG_PTR new_htable, old_htable;

    old_htable_size = ANSWERTABLE_BUCKET_SIZE(answer_table);
    old_htable = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answer_table);
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

    new_htable = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_htable_size);
    if (new_htable == NULL) return;  /* stop expanding */
    for (i = 0; i < new_htable_size; i++) {
        new_htable[i] = (BPLONG)NULL;
    }
    for (i = 0; i < old_htable_size; i++) {
        BPLONG_PTR answer, next_answer;
        answer = (BPLONG_PTR)old_htable[i];
        while (answer != NULL) {
            next_answer = (BPLONG_PTR)ANSWER_NEXT_IN_CHAIN(answer);
            index = hashval_of_tabled_answer(answer, arity) % new_htable_size;
            ANSWER_NEXT_IN_CHAIN(answer) = new_htable[index];
            new_htable[index] = (BPLONG)answer;
            answer = next_answer;
        }
    }
    free(old_htable);
    ANSWERTABLE_BUCKET_SIZE(answer_table) = new_htable_size;
    ANSWERTABLE_BUCKET_PTR(answer_table) = (BPLONG)new_htable;
}


/* This function adds an answer into the answer table.
   Preconditions: 
   (1) one of the argument modes is min or max.
   (2) a hash table (answer table) has been allocated.
   Postconditions:
   (1) the answer is discarded if a variant exists already
   (2) the answer is discarded if the cardinality limit has been reached and the answer is not better than any
   (3) the answer replaces the current worst one if the cardinality limit has been reached and the answer is better than some
   (4) the answer is inserted into the ordered list of answers if the cardinality limit has not been reached
*/
int addTableOptimalAnswer(BPLONG_PTR stack_arg_ptr, int arity, BPLONG_PTR subgoal_entry, int opt_arg_index, int maximize, int table_card) {
    BPLONG_PTR answer_table, this_answer, answer, last_answer, bucket_ptr, this_table_arg_ptr, table_arg_ptr, entryPtr;
    BPLONG_PTR old_table_top;
    BPLONG bucket_size, hcode;
    int i, answer_record_size;

    answer_table = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
    //  initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    PREPARE_NUMBER_TERM(0);

    bucket_ptr = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answer_table);
    bucket_size = ANSWERTABLE_BUCKET_SIZE(answer_table);

    answer_record_size = arity+2;
    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, this_answer, answer_record_size);
    if (this_answer == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }

    old_table_top = this_answer;
    this_table_arg_ptr = ANSWER_ARG_ADDR(this_answer);

    if (numberVarCopyAnswerArgsToTableArea(stack_arg_ptr, this_table_arg_ptr, arity, &hcode) == BP_ERROR)
        return BP_ERROR;

    entryPtr = bucket_ptr+hcode%bucket_size;
    answer = (BPLONG_PTR)FOLLOW(entryPtr);
    while (answer != NULL) {
        table_arg_ptr = ANSWER_ARG_ADDR(answer);
        for (i = 0; i < arity; i++) {
            BPLONG t1, t2;
            t1 = FOLLOW(this_table_arg_ptr+i);
            t2 = FOLLOW(table_arg_ptr+i);
            if (t1 != t2 && !identicalTabledTerms(t1, t2)) goto lab_fail;
        }
        //    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
        //    UNDO_TRAILING; /* variants */
        if (ta_record_ptr->top == old_table_top+answer_record_size) {
            ta_record_ptr->top = old_table_top;  /* deallocate the space */
        }
        return BP_FALSE;  /* the producer shoud fail */
    lab_fail:
        answer = (BPLONG_PTR)ANSWER_NEXT_IN_CHAIN(answer);
    }
    //  trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    //  UNDO_TRAILING;

    /* now add the new answer if the cardinality limit allows */
    last_answer = (BPLONG_PTR)ANSWERTABLE_LAST(answer_table);
    if (table_card == ANSWERTABLE_COUNT(answer_table)) {  /* replace */
        if (maximize == 0 && isSmallerTabledAnswer(this_answer, last_answer, opt_arg_index) ||
            maximize == 1 && isBiggerTabledAnswer(this_answer, last_answer, opt_arg_index)) {
            removeAnswerFromAnswerTable(answer_table, last_answer, arity);
            copyTabledAnswerArgs(this_answer, last_answer, arity);
            if (ta_record_ptr->top == old_table_top+answer_record_size) {  /* reuse the last answer, so deallocate the space */
                ta_record_ptr->top = old_table_top;
            }
            this_answer = last_answer;
            ANSWER_NEXT_IN_CHAIN(this_answer) = FOLLOW(entryPtr);
            FOLLOW(entryPtr) = (BPLONG)this_answer;
            if (maximize == 1)
                repositionLastTabledAnswerMax(answer_table, last_answer, opt_arg_index);
            else
                repositionLastTabledAnswerMin(answer_table, last_answer, opt_arg_index);
            SET_SUBGOAL_ANS_REVISED(subgoal_entry);
            return BP_TRUE;
        } else {  /* this answer is not better, so discard it */
            if (ta_record_ptr->top == old_table_top+answer_record_size) {  /* reuse the last answer, so deallocate the space */
                ta_record_ptr->top = old_table_top;
            }
            return BP_FALSE;
        }
    } else {  /* insert */
        ANSWER_NEXT_IN_CHAIN(this_answer) = FOLLOW(entryPtr);
        FOLLOW(entryPtr) = (BPLONG)this_answer;
        if (maximize == 1)
            insertTabledAnswerMax(answer_table, this_answer, opt_arg_index);
        else
            insertTabledAnswerMin(answer_table, this_answer, opt_arg_index);
        ANSWERTABLE_COUNT(answer_table) = ANSWERTABLE_COUNT(answer_table)+1;
        if (ANSWERTABLE_COUNT(answer_table) > ANSWERTABLE_BUCKET_SIZE(answer_table)) {
            expandAnswerTable(answer_table, arity);
        }

        SET_SUBGOAL_ANS_REVISED(subgoal_entry);
        return BP_TRUE;
    }
}


/* check if ans1 is bigger than ans2 in terms of the argument with max mode */
int isBiggerTabledAnswer(BPLONG_PTR ans1, BPLONG_PTR ans2, int opt_arg_index) {
    BPLONG_PTR table_arg_ptr1, table_arg_ptr2;
    BPLONG t1, t2;

    table_arg_ptr1 = ANSWER_ARG_ADDR(ans1);
    table_arg_ptr2 = ANSWER_ARG_ADDR(ans2);
    t1 = FOLLOW(table_arg_ptr1+opt_arg_index);
    t2 = FOLLOW(table_arg_ptr2+opt_arg_index);
    return (TABLE_ANS_IS_GT(t1, t2));
}

/* check if ans1 is smaller than ans2 in terms of the argument with min mode */
int isSmallerTabledAnswer(BPLONG_PTR ans1, BPLONG_PTR ans2, int opt_arg_index) {
    BPLONG_PTR table_arg_ptr1, table_arg_ptr2;
    BPLONG t1, t2;

    table_arg_ptr1 = ANSWER_ARG_ADDR(ans1);
    table_arg_ptr2 = ANSWER_ARG_ADDR(ans2);
    t1 = FOLLOW(table_arg_ptr1+opt_arg_index);
    t2 = FOLLOW(table_arg_ptr2+opt_arg_index);
    return (TABLE_ANS_IS_LT(t1, t2));
}

/* remove this_answer from the hash chain */
void removeAnswerFromAnswerTable(BPLONG_PTR answer_table, BPLONG_PTR this_answer, int arity) {
    BPLONG_PTR answer_ptr, bucket_ptr, answer;
    BPLONG bucket_size;

    bucket_ptr = (BPLONG_PTR)ANSWERTABLE_BUCKET_PTR(answer_table);
    bucket_size = ANSWERTABLE_BUCKET_SIZE(answer_table);
    answer_ptr = bucket_ptr+hashval_of_tabled_answer(this_answer, arity)%bucket_size;
    answer = (BPLONG_PTR)FOLLOW(answer_ptr);
    while (answer != this_answer) {
        answer_ptr = ANSWER_NEXT_IN_CHAIN_ADDR(answer);
        answer = (BPLONG_PTR)ANSWER_NEXT_IN_CHAIN(answer);
    }
    FOLLOW(answer_ptr) = ANSWER_NEXT_IN_CHAIN(answer);
}

void copyTabledAnswerArgs(BPLONG_PTR src_ans, BPLONG_PTR des_ans, int arity) {
    BPLONG_PTR src_arg_ptr, des_arg_ptr;
    int i;

    src_arg_ptr = ANSWER_ARG_ADDR(src_ans);
    des_arg_ptr = ANSWER_ARG_ADDR(des_ans);
    for (i = 0; i < arity; i++) {
        FOLLOW(des_arg_ptr+i) = FOLLOW(src_arg_ptr+i);
    }
}

/* The last answer has been replaced with a better answer now. It needs to be repositioned so that
   the answers remain sorted from the best to the worst */
void repositionLastTabledAnswerMin(BPLONG_PTR answer_table, BPLONG_PTR last_answer, int opt_arg_index) {
    BPLONG_PTR prev_answer, answer;

    answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answer_table);
    if (isSmallerTabledAnswer(last_answer, answer, opt_arg_index)) {  /* place in the front, first can't be last */
        ANSWERTABLE_FIRST(answer_table) = (BPLONG)last_answer;
        ANSWER_NEXT_IN_TABLE(last_answer) = (BPLONG)answer;
        resetLastTabledAnswer(answer_table, answer, last_answer);
    } else {
        do {
            prev_answer = answer;
            answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
        } while (answer != last_answer && !isSmallerTabledAnswer(last_answer, answer, opt_arg_index));

        if (answer != last_answer) {
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)last_answer;
            ANSWER_NEXT_IN_TABLE(last_answer) = (BPLONG)answer;
            resetLastTabledAnswer(answer_table, answer, last_answer);
        }
    }
}

void repositionLastTabledAnswerMax(BPLONG_PTR answer_table, BPLONG_PTR last_answer, int opt_arg_index) {
    BPLONG_PTR prev_answer, answer;

    answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answer_table);
    if (isBiggerTabledAnswer(last_answer, answer, opt_arg_index)) {  /* place in the front, first can't be last since (cardinality_limit >= 2)*/
        ANSWERTABLE_FIRST(answer_table) = (BPLONG)last_answer;
        ANSWER_NEXT_IN_TABLE(last_answer) = (BPLONG)answer;
        resetLastTabledAnswer(answer_table, answer, last_answer);
    } else {
        do {
            prev_answer = answer;
            answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
        } while (answer != last_answer && !isBiggerTabledAnswer(last_answer, answer, opt_arg_index));

        if (answer != last_answer) {
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)last_answer;
            ANSWER_NEXT_IN_TABLE(last_answer) = (BPLONG)answer;
            resetLastTabledAnswer(answer_table, answer, last_answer);
        }
    }
}

void resetLastTabledAnswer(BPLONG_PTR answer_table, BPLONG_PTR answer, BPLONG_PTR last_answer) {
    BPLONG_PTR prev_answer;

    do {
        prev_answer = answer;
        answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(prev_answer);
    } while (answer != last_answer);
    ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)NULL;
    ANSWERTABLE_LAST(answer_table) = (BPLONG)prev_answer;
}

void insertTabledAnswerMin(BPLONG_PTR answer_table, BPLONG_PTR this_answer, int opt_arg_index) {
    BPLONG_PTR prev_answer, answer;

    void printAnswer();
    answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answer_table);
    if (isSmallerTabledAnswer(this_answer, answer, opt_arg_index)) {  /* place in the front */
        ANSWERTABLE_FIRST(answer_table) = (BPLONG)this_answer;
        ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)answer;
    } else {
        do {
            prev_answer = answer;
            answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
        } while (answer != NULL && !isSmallerTabledAnswer(this_answer, answer, opt_arg_index));
        if (answer == NULL) {  /* insert as last */
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)this_answer;
            ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)NULL;
            ANSWERTABLE_LAST(answer_table) = (BPLONG)this_answer;
        } else {
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)this_answer;
            ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)answer;
        }
    }
}

void insertTabledAnswerMax(BPLONG_PTR answer_table, BPLONG_PTR this_answer, int opt_arg_index) {
    BPLONG_PTR prev_answer, answer;

    answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answer_table);
    if (isBiggerTabledAnswer(this_answer, answer, opt_arg_index)) {  /* place in the front */
        ANSWERTABLE_FIRST(answer_table) = (BPLONG)this_answer;
        ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)answer;
    } else {
        do {
            prev_answer = answer;
            answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
        } while (answer != NULL && !isBiggerTabledAnswer(this_answer, answer, opt_arg_index));
        if (answer == NULL) {  /* insert as last */
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)this_answer;
            ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)NULL;
            ANSWERTABLE_LAST(answer_table) = (BPLONG)this_answer;
        } else {
            ANSWER_NEXT_IN_TABLE(prev_answer) = (BPLONG)this_answer;
            ANSWER_NEXT_IN_TABLE(this_answer) = (BPLONG)answer;
        }
    }
}


/***************************************************************/
int c_VARIANT() {
    BPLONG op1, op2;

    op1 = ARG(1, 2);
    op2 = ARG(2, 2);
    return b_VARIANT_cc(op1, op2);
}

int b_VARIANT_cc(BPLONG op1, BPLONG op2) {
    int i;
    BPLONG maxVarNo;
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;

    DEREF(op1); DEREF(op2);

    if (TAG(op1) == ATM || TAG(op2) == ATM)
        return op1 == op2;

    maxVarNo = -1;

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;

    PREPARE_NUMBER_TERM(0);
    numberVarTermOpt(op1);

    PREPARE_NUMBER_TERM(0);
    numberVarTermOpt(op2);

    i = unifyNumberedTerms(op1, op2);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;

    return i != 0;
}

/*
  t2 is a numbered term, which needs to be dereferenced
*/
int term_subsume_numberedterm(BPLONG t1, BPLONG t2) {
    BPLONG_PTR top, ptr;
    BPLONG i, arity;

beginning:
    DEREF_NONVAR(t2);
    SWITCH_OP_NOSUSP(t1,
                     start,
                     {NDEREF(t1, start);
                         ASSIGN_TRAIL_VALUE(t1, t2);
                         return 1;
                     },
                     {return (t1 == t2);},
                     {if (IsNumberedVar(t1)) return (t1 == t2);
                         if (!ISLIST(t2)) return 0;
                         UNTAG_ADDR(t1); UNTAG_ADDR(t2);
                         if (!term_subsume_numberedterm(*(BPLONG_PTR)t1, *(BPLONG_PTR)t2)) return 0;
                         t1 = *((BPLONG_PTR)t1+1);
                         t2 = *((BPLONG_PTR)t2+1);
                         goto beginning;
                     },
                     {if (t1 < 0) {
                             return 0;
                         }
                         if (!ISSTRUCT(t2)) return 0;
                         UNTAG_ADDR(t1); UNTAG_ADDR(t2);
                         if (FOLLOW(t1) != FOLLOW(t2)) return 0;
                         arity = GET_ARITY((SYM_REC_PTR)FOLLOW(t1));
                         for (i = 1; i <= arity; i++) {
                             if (!term_subsume_numberedterm(*((BPLONG_PTR)t1+i), *((BPLONG_PTR)t2+i))) return 0;
                         }
                         return 1;
                     });
}

int term_subsume_term(BPLONG op1, BPLONG op2) {
    int i;
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;

    PREPARE_NUMBER_TERM(0);
    numberVarTermOpt(op2);

    i = term_subsume_numberedterm(op1, op2);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;

    return i != 0;
}

int c_table_reset_subgoal_ar() {
    BPLONG_PTR sp;
    BPLONG_PTR subgoal_entry;

    sp = (BPLONG_PTR)AR_B(arreg);
    while (AR_AR(sp) != (BPLONG)sp) {
        if (IS_TABLE_FRAME(sp)) {
            subgoal_entry = (BPLONG_PTR)GET_AR_SUBGOAL_TABLE(sp);
            if (!SUBGOAL_IS_COMPLETE(subgoal_entry)) {
                GT_TOP_AR(subgoal_entry) = (BPLONG)NULL;
            }
        }
        sp = (BPLONG_PTR)AR_B(sp);
    }
    return BP_TRUE;
}

int c_SUBGOAL_TABLE_SIZE() {
    BPLONG gt_bucket_size;
    BPLONG_PTR top;

    gt_bucket_size = ARG(1, 1); DEREF(gt_bucket_size);

    return unify(gt_bucket_size, MAKEINT(subgoalTableBucketSize));
}

BPLONG_PTR lookupSubgoalTableNoCopy(BPLONG_PTR stack_arg_ptr, int arity, SYM_REC_PTR sym_ptr) {
    BPLONG_PTR entryPtrPtr0, entryPtr, thisEntryPtr;
    BPLONG_PTR subgoal_arg_ptr;
    BPLONG term;
    BPLONG i;
    BPLONG hcode, this_hcode;
    BPLONG_PTR trail_top0;
    BPLONG initial_diff0;

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;

    /* before numbering the tabled call */
    PREPARE_NUMBER_TERM(0);
    hcode = ((BPLONG)sym_ptr & HASH_BITS) >> 2;
    if (arity != 0) {
        term = FOLLOW(stack_arg_ptr);
        numberVarTermOpt(term);
        hcode += bp_hashval(term);
        //      printf("lookup(ncp) hcode1=%x\n",hcode);

        for (i = 1; i < arity; i++) {
            term = FOLLOW(stack_arg_ptr-i);
            numberVarTermOpt(term);
            this_hcode = bp_hashval(term);
            if (this_hcode != 0) hcode = MurmurHash3_x86_32_uint32((UW32)this_hcode, (UW32)hcode);
        }
        hcode = (hcode & HASH_BITS);
    }

    //  printf("lookup(ncp) hcode=%x\n",hcode);

    entryPtrPtr0 = subgoalTable + (hcode % subgoalTableBucketSize);
    entryPtr = (BPLONG_PTR)FOLLOW(entryPtrPtr0);

    while (entryPtr != NULL) {  /* lookup */
        if ((SYM_REC_PTR)GT_SYM(entryPtr) != sym_ptr) goto lab_fail1;
        subgoal_arg_ptr = GT_ARG_ADDR(entryPtr);
        for (i = 0; i < arity; i++) {
            BPLONG t1, t2;
            t1 = FOLLOW(stack_arg_ptr-i);
            t2 = FOLLOW(subgoal_arg_ptr+i);
            if (t1 != t2 && !unifyNumberedTerms(t1, t2)) goto lab_fail1;
        }
        thisEntryPtr = entryPtr;
        goto lookup_end;
    lab_fail1:
        entryPtr = (BPLONG_PTR)GT_NEXT(entryPtr);
    }
    thisEntryPtr = NULL;
lookup_end:
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    return thisEntryPtr;
}

int c_TABLE_GET_ONE_ANSWER() {
    BPLONG Call, t1, t2;
    BPLONG_PTR top, subgoal_entry, call_arg_ptr, ans_arg_ptr, stack_arg_ptr, answerTable, answer;
    SYM_REC_PTR sym_ptr;
    BPLONG i, arity;

    Call = ARG(1, 1); DEREF(Call);
    if (ISATOM(Call) || ISSTRUCT(Call)) {
        sym_ptr = GET_SYM_REC(Call);
        arity = GET_ARITY(sym_ptr);
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }

    stack_arg_ptr = local_top;  /* copy arguments */
    call_arg_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Call)+1;
    for (i = 0; i < arity; i++) {
        FOLLOW(stack_arg_ptr-i) = FOLLOW(call_arg_ptr+i);
    }

    subgoal_entry = lookupSubgoalTableNoCopy(stack_arg_ptr, arity, sym_ptr);
    if ((BPLONG)subgoal_entry == BP_ERROR) {
        return BP_ERROR;
    } else if (subgoal_entry == NULL) {
        return BP_FALSE;
    }

    answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
    if (answerTable == NULL) return BP_FALSE;

    if ((BPLONG)answerTable & 0x1)
        answer = (BPLONG_PTR)UNTAGGED_ADDR(answerTable);
    else
        answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answerTable);

    PREPARE_UNNUMBER_TERM(local_top);
    ans_arg_ptr = ANSWER_ARG_ADDR(answer);
    for (i = 0; i < arity; i++) {
        t1 = FOLLOW(call_arg_ptr+i);
        t2 = FOLLOW(ans_arg_ptr+i);
        match_term_tabledTerm(t1, t2);
    }
    return BP_TRUE;
}

/******* fectch all answers for a call ***/
int c_TABLE_GET_ALL_ANSWERS() {
    BPLONG Call, Answers;
    BPLONG list, list0, ans;
    BPLONG_PTR subgoal_entry, answerTable, answer;
    SYM_REC_PTR sym_ptr;
    BPLONG i;

    Call = ARG(1, 2);
    Answers = ARG(2, 2);
    list = (BPLONG)heap_top; list0 = list; heap_top++;
    for (i = 0; i < subgoalTableBucketSize; i++) {
        subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
        while (subgoal_entry != NULL) {
            if (table_subsume(Call, (SYM_REC_PTR)GT_SYM(subgoal_entry), GT_ARG_ADDR(subgoal_entry))) {
                sym_ptr = (SYM_REC_PTR)GT_SYM(subgoal_entry);
                answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry);
                if (answerTable != NULL) {
                    if ((BPLONG)answerTable & 0x1) {
                        answer = (BPLONG_PTR)UNTAGGED_ADDR(answerTable);
                        ANSWER_NEXT_IN_TABLE(answer) = (BPLONG)NULL;
                    } else {
                        answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answerTable);
                    }
                    do {
                        ans = answer_table_entry_2_struct(sym_ptr, ANSWER_ARG_ADDR(answer));
                        FOLLOW(list) = ADDTAG(heap_top, LST);
                        FOLLOW(heap_top++) = ans;
                        list = (BPLONG)heap_top++;

                        LOCAL_OVERFLOW_CHECK("table");

                        answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
                    } while (answer != NULL);
                }
            }
            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
    }
    FOLLOW(list) = nil_sym;
    return unify(Answers, list0);
}

int table_subsume(BPLONG Call, SYM_REC_PTR sym_ptr, BPLONG_PTR arg_ptr) {
    BPLONG subgoal;

    subgoal = answer_table_entry_2_struct(sym_ptr, arg_ptr);
    return term_subsume_term(Call, subgoal);
}

/* convert an answer record to a Prolog structure
   ptr0 points to a sequence of arguments
*/
BPLONG answer_table_entry_2_struct(SYM_REC_PTR sym_ptr, BPLONG_PTR ptr0) {
    BPLONG ans, op;
    BPLONG_PTR call_ptr;
    BPLONG i, arity;

    arity = GET_ARITY(sym_ptr);
    ans = ADDTAG(heap_top, STR);
    FOLLOW(heap_top++) = (BPLONG)sym_ptr;
    call_ptr = heap_top; heap_top += arity;  /* slots to be filled */

    LOCAL_OVERFLOW_CHECK("table");

    PREPARE_UNNUMBER_TERM(local_top);
    for (i = 0; i < arity; i++) {
        op = FOLLOW(ptr0+i);
        FOLLOW(call_ptr+i) = unnumberVarTabledTerm(op);
    }
    return ans;
}

/*****************************************************************/
/* The table modes and cardinality are stored as operands of a table_mode instruction

   name/arity:
   table_allocate Arity,Size,Sym,MaxS
   table_mode ModeBits, OptArg,Card 
*/
int c_table_cardinality_limit() {
    BPLONG name, arity, card;
    BPLONG_PTR ep;
    SYM_REC_PTR sym_ptr;

    name = ARG(1, 3);
    arity = ARG(2, 3);
    card = ARG(3, 3); DEREF(card);

    GET_GLOBAL_SYM(name, arity, sym_ptr);

    ep = (BPLONG_PTR)GET_EP(sym_ptr);
    if (GET_ETYPE(sym_ptr) != T_PRED) {
        bp_exception = illegal_arguments;
        return -1;
    }

    if (FOLLOW(ep) != table_allocate_code) {
        bp_exception = illegal_arguments;
        return -1;
    }
    if (ISREF(card)) {
        BPLONG cur_limit = FOLLOW(ep+8);
        if (cur_limit == 0) {
            cur_limit = BP_MAXINT_1W;
        }
        return unify(card, MAKEINT(cur_limit));
    } if (ISINT(card)) {
        FOLLOW(ep+8) = INTVAL(card);
        return BP_TRUE;
    }
}

/* set the cardinality limit of all tabled predicates (except those with
   unlimited cardinality) to be the given one. */
int c_set_all_table_cardinality_limit() {
    BPLONG card;
    BPLONG i;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR ep;

    card = ARG(1, 1); DEREF(card); card = INTVAL(card);

    for (i = 0; i < BUCKET_CHAIN; ++i) {
        sym_ptr = hash_table[i];
        while (sym_ptr != NULL) {
            if (GET_ETYPE(sym_ptr) == T_PRED) {
                ep = (BPLONG_PTR)GET_EP(sym_ptr);
                if (FOLLOW(ep) == table_allocate_code) {
                    FOLLOW(ep+8) = card;
                }
            }
            sym_ptr = GET_NEXT(sym_ptr);
        }
    }
    return BP_TRUE;
}

int table_statistics() {
    BPLONG i, count, subgoal_count, total_ans_count, max_ans_count, zero_ans_count, total_ans_access_count, max_ans_access_count, total_its_count, max_its_count, scc_nodes_count;
    BPLONG_PTR subgoal_entry, ptr;
    subgoal_count = 0;
    total_ans_access_count = 0;
    max_ans_access_count = 0;
    total_its_count = 0;
    max_its_count = 0;
    total_ans_count = 0;
    max_ans_count = 0;
    zero_ans_count = 0;
    scc_nodes_count = 0;

    for (i = 0; i < subgoalTableBucketSize; i++) {
        count = 0;
        subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
        while (subgoal_entry != NULL) {
            count++;
            if (GT_ANSWER_TABLE(subgoal_entry) != (BPLONG)NULL) {
                /*      fprintf(curr_out,"(%d)",ANSWERTABLE_COUNT((BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry))); */
                total_ans_count += ANSWERTABLE_COUNT((BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry));
                if (ANSWERTABLE_COUNT((BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry)) > max_ans_count) max_ans_count = ANSWERTABLE_COUNT((BPLONG_PTR)GT_ANSWER_TABLE(subgoal_entry));
            } else {
                zero_ans_count++;
            }

            ptr = (BPLONG_PTR)GT_SCC_ELMS(subgoal_entry);
            while (ptr != NULL) {
                scc_nodes_count++;
                ptr = (BPLONG_PTR)FOLLOW(ptr+1);
            }

            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
        subgoal_count += count;
        /*    if (count != 0) fprintf(curr_out,"socket#%d(chainsize%d)\n",i,count); */
    }
    fprintf(curr_out, "number_of_subgoals=" BPLONG_FMT_STR "\t\t\n", subgoal_count);
    fprintf(curr_out, "max_number_of_answers=" BPLONG_FMT_STR "\t\t\n", max_ans_count);
    fprintf(curr_out, "number_of_zero_answer_subgoals=" BPLONG_FMT_STR "\t\t\n", zero_ans_count);
    fprintf(curr_out, "average_number_of_answers=%.2f\t\t\n", (float)total_ans_count/subgoal_count);
    fprintf(curr_out, "max_iterations=" BPLONG_FMT_STR "\t\t\n", max_its_count);
    fprintf(curr_out, "average_iterations=%.2f\t\t\n", (float)total_its_count/(float)subgoal_count);
    fprintf(curr_out, "number_of_scc_nodes=" BPLONG_FMT_STR "\n", scc_nodes_count);
    return 1;
}

/* Returns the current plan that transforms the initial state
   to the current state, and the current resource amount. 
   These two values are available at the latest call
   '_$plan'(S,iplan(Limit,Plan,PlanLen)).
*/
int b_PLANNER_CURR_RPC_fff(BPLONG Amount, BPLONG Plan, BPLONG Cost) {
    BPLONG_PTR f0, f;
    f0 = arreg;
    f = (BPLONG_PTR)AR_AR(f0);
    while (f != f0) {
        if (IS_TABLE_FRAME(f)) {
            BPLONG_PTR subgoal_entry = (BPLONG_PTR)AR_SUBGOAL_TABLE(f);
            SYM_REC_PTR sym_ptr = (SYM_REC_PTR)GT_SYM(subgoal_entry);
            if (sym_ptr == plan_psc) {  /* a frame of '_$plan'/2 */
                BPLONG iplan_struct;
                BPLONG_PTR arg_ptr;
                iplan_struct = FOLLOW(f+1);  /* the second argument of '_$plan' */
                DEREF(iplan_struct);
                arg_ptr = (BPLONG_PTR)UNTAGGED_ADDR(iplan_struct);
                ASSIGN_f_atom(Amount, FOLLOW(arg_ptr+1));
                ASSIGN_v_heap_term(Plan, FOLLOW(arg_ptr+2));
                ASSIGN_v_heap_term(Cost, FOLLOW(arg_ptr+3));
                return BP_TRUE;
            }
        }
        f0 = f;
        f = (BPLONG_PTR)AR_AR(f);
    }
    ASSIGN_f_atom(Amount, MAKEINT(BP_MAXINT_1W));
    ASSIGN_f_atom(Plan, nil_sym);
    ASSIGN_f_atom(Cost, BP_ZERO);
    return BP_TRUE;
}

/* check if a call '_$planner'(S,_) has been tabled */
int b_IS_PLANNER_STATE_c(BPLONG state) {
    BPLONG_PTR stack_arg_ptr;

    stack_arg_ptr = local_top;
    FOLLOW(stack_arg_ptr) = state;
    FOLLOW(stack_arg_ptr-1) = (BPLONG)(stack_arg_ptr-1);  /* free var */
    return (lookupSubgoalTableNoCopy(stack_arg_ptr, 2, thashtable_psc) != NULL) ? BP_TRUE : BP_FALSE;
}

int b_PLANNER_UPDATE_EXPLORED_DEPTH_c(BPLONG depth) {
    BPLONG cur_depth = (BPLONG)GET_EP(planner_explored_depth_psc);

    DEREF_NONVAR(depth);
    if (INTVAL(depth) < INTVAL(cur_depth))
        GET_EP(planner_explored_depth_psc) = (int (*)(void))depth;
    return BP_TRUE;
}

/* retrieve ta_record_ptr->top */
int c_TA_TOP_f() {
    BPLONG op = ARG(1, 1);
    ASSIGN_f_atom(op, ADDTAG((BPLONG)ta_record_ptr->top, INT_TAG));
    return BP_TRUE;
}

void reset_temp_complete_subgoal_entries() {
    BPLONG i;
    BPLONG_PTR subgoal_entry, ptr;

    for (i = 0; i < subgoalTableBucketSize; i++) {
        subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
        while (subgoal_entry != NULL) {
            if (GT_TOP_AR(subgoal_entry) == SUBGOAL_TEMP_COMPLETE) {
                printf("TEMP_COMPLETE " BPULONG_FMT_STR "\n", subgoal_entry);
                ptr = (BPLONG_PTR)GT_SCC_ROOT(subgoal_entry);
                printf("SCC_ROOT = " BPULONG_FMT_STR "\n", ptr);
                printf("SCC_ROOT = " BPULONG_FMT_STR "\n", GT_TOP_AR(ptr));
            }
            //                GT_TOP_AR(subgoal_entry) = (BPLONG)NULL;
            subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
        }
    }
}

/*
  void printTable(){
  void printSubgoalTableEntry(BPLONG_PTR);
  
  BPLONG i,count,subgoal_count,total_ans_count,max_ans_count,zero_ans_count,total_ans_access_count,max_ans_access_count,total_its_count,max_its_count,scc_nodes_count;
  BPLONG_PTR subgoal_entry,ptr;
  subgoal_count = 0;
  total_ans_access_count=0;
  max_ans_access_count=0;
  total_its_count=0;
  max_its_count=0;
  total_ans_count=0;
  max_ans_count=0;
  zero_ans_count=0;
  scc_nodes_count = 0;
  
  for (i=0;i<subgoalTableBucketSize;i++){
  subgoal_entry = (BPLONG_PTR)FOLLOW(subgoalTable+i);
  while (subgoal_entry != NULL) {
  printSubgoalTableEntry(subgoal_entry);
  subgoal_entry = (BPLONG_PTR)GT_NEXT(subgoal_entry);
  }
  }
  }

  void printSubgoalTableEntry(ptr)
  BPLONG_PTR ptr;
  {
  BPLONG arity;
  BPLONG_PTR answer,answerTable;
  SYM_REC_PTR sym_ptr;
  void printAnswer();

  sym_ptr = (SYM_REC_PTR)GT_SYM(ptr);
  arity = GET_ARITY(sym_ptr);

  
  fprintf(curr_out,"%x %s(",ptr, GET_NAME(sym_ptr));
  //  printAnswer(GT_ARG_ADDR(ptr),arity);
  fprintf(curr_out,"(%x)\n",GT_TOP_AR(ptr));
  answerTable = (BPLONG_PTR)GT_ANSWER_TABLE(ptr);
  if (answerTable==NULL) return;
  answer = (BPLONG_PTR)ANSWERTABLE_FIRST(answerTable);
  fprintf(curr_out,"OLD\n");
  answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer);
  while (answer!=NULL){
  fprintf(curr_out,"%x     (",answer);printAnswer(ANSWER_ARG_ADDR(answer),arity);
  answer = (BPLONG_PTR)ANSWER_NEXT_IN_TABLE(answer); 
  }
  }

  void printAnswer(ptr,arity)
  BPLONG_PTR ptr;
  BPLONG arity;
  {
  BPLONG i;
  BPLONG term;
  void myWriteTerm();

  for (i=0;i<arity;i++){
  term = FOLLOW(ptr+i);
  myWriteTerm(term);
  if (i!=arity-1) fprintf(curr_out,",");
  }
  fprintf(curr_out,"). ");
  }

  void myWriteTerm(term)
  BPLONG term;
  {
  BPLONG_PTR top;
  DEREF(term);

  if (ISREF(term)) fprintf(curr_out,"_%x",term); 
  else if (TAG(term)==ATM) write_term(term);
  else if (TAG(term)==LST) {
  if (IsNumberedVar(term)) fprintf(curr_out,"n%x ",term);
  else {
  fprintf(curr_out,"[");
  myWriteList(term);
  fprintf(curr_out,"]");
  }
  } else {
  SYM_REC_PTR sym_ptr;
  BPLONG arity,i;
  UNTAG_ADDR(term);
  sym_ptr = (SYM_REC_PTR)FOLLOW(term);
  bp_write_pname(GET_NAME(sym_ptr));
  if (GET_ARITY(sym_ptr)>0) {
  fprintf(curr_out,"(");
  arity = GET_ARITY(sym_ptr);
  for (i=1;i<=arity;i++) {
  myWriteTerm(*((BPLONG_PTR)term+i));
  if (i<arity)
  fprintf(curr_out,",");
  }
  fprintf(curr_out,")");
  }
  }
  }

  void myWriteList(term)
  BPLONG term;
  {
  BPLONG car,cdr;
  BPLONG_PTR top;

  UNTAG_ADDR(term);
  car = FOLLOW((BPLONG_PTR)term);
  cdr = FOLLOW((BPLONG_PTR)term+1);DEREF(cdr);
  myWriteTerm(car);
  if (ISLIST(cdr)){
  fprintf(curr_out,",");
  myWriteList(cdr);
  }
  }

*/

/*  defined in basic.h, also see global_maps in assert.c

    #define NUM_PICAT_TABLE_MAPS 97

    typedef struct {
    BPLONG key;
    BPLONG val;
    BPLONG_PTR next;
    } KEY_VAL_PAIR;

    typedef KEY_VAL_PAIR *KEY_VAL_PAIR_PTR;

    typedef struct {
    BPLONG size;
    BPLONG count;
    BPLONG_PTR htable;
    } MAP_RECORD;

    typedef MAP_RECORD *MAP_RECORD_PTR;
*/

void init_picat_table_maps() {
    BPLONG i;

    for (i = 0; i < NUM_PICAT_TABLE_MAPS; i++) {
        picat_table_maps[i] = NULL;
    }
}

/* Return the number of the map with map_id. If no map with the id was found,
   then create a new map and register it into table_maps. Linear prob is used 
   to look for the map with map_id.

   Each entry in table_maps is a pointer to a MAP_RECORD, which stores the 
   information about the map, including the size of the bucket table, the number 
   of key-value pairs (count), and a pointer to the bucket table (htable).
*/
int b_GET_PICAT_TABLE_MAP_cf(BPLONG map_id, BPLONG map_num) {
    BPLONG slot_i0, slot_i, i, map_id_cp, this_hcode, this_ground_flag;
    BPLONG_PTR tmp_ptr;
    MAP_RECORD_PTR map_ptr;

    PREPARE_NUMBER_TERM(0);
    this_ground_flag = TOP_BIT;
    map_id_cp = numberVarCopyToTableArea(ta_record_ptr, map_id, &this_hcode, &this_ground_flag);

    if (map_id_cp == BP_ERROR) return BP_ERROR;
    if (this_ground_flag == 0) {
        bp_exception = ground_expected;
        return BP_ERROR;
    }
    slot_i0 = slot_i = (this_hcode % NUM_PICAT_TABLE_MAPS);

    // linear prob
    while ((BPLONG_PTR)picat_table_maps[slot_i] != NULL) {
        if (picat_table_map_ids[slot_i] == map_id_cp) {
            return unify(map_num, MAKEINT(slot_i));
        }
        slot_i++;
        if (slot_i == NUM_PICAT_TABLE_MAPS) slot_i = 0;
        if (slot_i == slot_i0) quit("TABLE MAPS OVERFLOW");
    }
    // Come here if map_id was not found. Register a map in slot i
    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, tmp_ptr, sizeof(MAP_RECORD));
    if (tmp_ptr == NULL) myquit(OUT_OF_MEMORY, "table_maps");
    map_ptr = (MAP_RECORD_PTR)tmp_ptr;
    map_ptr->count = 0;
    map_ptr->size = 7;  // initial size
    tmp_ptr = (BPLONG_PTR)malloc(7*sizeof(BPLONG_PTR));
    map_ptr->htable = tmp_ptr;
    for (i = 0; i < 7; i++)
        FOLLOW(tmp_ptr+i) = (BPLONG)NULL;

    picat_table_maps[slot_i] = (BPLONG_PTR)map_ptr;
    picat_table_map_ids[slot_i] = map_id_cp;

    return unify(map_num, MAKEINT(slot_i));
}

BPLONG table_maps_buckets_size() {
    BPLONG i, size;
    MAP_RECORD_PTR map_ptr;

    size = NUM_PICAT_TABLE_MAPS;

    for (i = 0; i < NUM_PICAT_TABLE_MAPS; i++) {
        map_ptr = (MAP_RECORD_PTR)picat_table_maps[i];
        if (map_ptr != NULL) {
            size += map_ptr->size;
        }
    }
    return size;
}

void expand_picat_table_map(MAP_RECORD_PTR mr_ptr) {
    BPLONG new_htable_size, old_htable_size, i;
    BPLONG_PTR new_htable, old_htable;
    KEY_VAL_PAIR_PTR kvp_ptr, next_kvp_ptr;

    old_htable_size = mr_ptr->size;
    old_htable = mr_ptr->htable;
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

    new_htable = (BPLONG_PTR)malloc(sizeof(BPLONG_PTR)*new_htable_size);

    if (new_htable == NULL) return;  /* stop expanding */
    for (i = 0; i < new_htable_size; i++) {
        new_htable[i] = (BPLONG)NULL;
    }
    for (i = 0; i < old_htable_size; i++) {
        kvp_ptr = (KEY_VAL_PAIR_PTR)old_htable[i];
        while (kvp_ptr != NULL) {
            BPLONG_PTR new_kvp_ptr_ptr;

            next_kvp_ptr = (KEY_VAL_PAIR_PTR)(kvp_ptr->next);
            new_kvp_ptr_ptr = (BPLONG_PTR)(new_htable + bp_hashval(kvp_ptr->key)%new_htable_size);
            kvp_ptr->next = (BPLONG_PTR)FOLLOW(new_kvp_ptr_ptr);
            FOLLOW(new_kvp_ptr_ptr) = (BPLONG)kvp_ptr;
            kvp_ptr = next_kvp_ptr;
        }
    }
    free(old_htable);
    mr_ptr->size = new_htable_size;
    mr_ptr->htable = new_htable;
}

int b_PICAT_TABLE_MAP_PUT_ccc(BPLONG map_num, BPLONG key, BPLONG val) {
    BPLONG i, key_cp, val_cp, this_hcode, this_ground_flag, dummy_hcode, dummy_ground_flag;
    BPLONG_PTR trail_top0, tmp_ptr, kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;
    BPLONG initial_diff0;

    DEREF(key);
    if (ISREF(key)) {
        bp_exception = nonvariable_expected;
        return BP_ERROR;
    }
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);

    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    PREPARE_NUMBER_TERM(0);
    key_cp = numberVarCopyToTableArea(ta_record_ptr, key, &this_hcode, &this_ground_flag);
    if (key_cp == BP_ERROR) return BP_ERROR;

    val_cp = numberVarCopyToTableArea(ta_record_ptr, val, &dummy_hcode, &dummy_ground_flag);
    if (val_cp == BP_ERROR) return BP_ERROR;

    kvp_ptr_ptr = (BPLONG_PTR)(mr_ptr->htable + (this_hcode % mr_ptr->size));
    kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

    while (kvp_ptr != NULL) {  /* lookup */
        if (kvp_ptr->key != key_cp && !key_identical(kvp_ptr->key, key_cp)) {
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        } else {
            kvp_ptr->val = val_cp;  /* update */
            goto lookup_end;
        }
    }
    // come here if lookup failed
    ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr, tmp_ptr, sizeof(KEY_VAL_PAIR));
    if (tmp_ptr == NULL) myquit(OUT_OF_MEMORY, "table_maps");
    kvp_ptr = (KEY_VAL_PAIR_PTR)tmp_ptr;
    kvp_ptr->key = key_cp;
    kvp_ptr->val = val_cp;
    kvp_ptr->next = (BPLONG_PTR)FOLLOW(kvp_ptr_ptr);
    FOLLOW(kvp_ptr_ptr) = (BPLONG)kvp_ptr;

    mr_ptr->count++;
    if (2*mr_ptr->count > mr_ptr->size)
        expand_picat_table_map(mr_ptr);

lookup_end:
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    return BP_TRUE;
}

int b_PICAT_TABLE_MAP_GET_ccf(BPLONG map_num, BPLONG key, BPLONG val) {
    BPLONG this_hcode;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    DEREF(key);
    if (ISREF(key)) {
        bp_exception = nonvariable_expected;
        return BP_ERROR;
    }

    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

    this_hcode = bp_hashval(key);

    kvp_ptr_ptr = (BPLONG_PTR)(mr_ptr->htable + (this_hcode % mr_ptr->size));
    kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

    while (kvp_ptr != NULL) {  /* lookup */
        if (kvp_ptr->key != key && !key_identical(kvp_ptr->key, key)) {
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        } else {
            PREPARE_UNNUMBER_TERM(local_top);
            return unify(val, unnumberVarTabledTerm(kvp_ptr->val));
        }
    }
    return BP_FALSE;
}

int b_PICAT_TABLE_MAP_SIZE_cf(BPLONG map_num, BPLONG size) {
    MAP_RECORD_PTR mr_ptr;

    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];
    return unify(size, MAKEINT(mr_ptr->count));
}

int b_PICAT_TABLE_MAP_CLEAR_c(BPLONG map_num) {
    int i;
    MAP_RECORD_PTR mr_ptr;
    BPLONG_PTR htable;

    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];
    mr_ptr-> count = 0;
    htable = mr_ptr->htable;
    for (i = 0; i < mr_ptr->size; i++) {
        FOLLOW(htable+i) = (BPLONG)NULL;
    }
    return BP_TRUE;
}

int b_PICAT_TABLE_MAP_KEYS_cf(BPLONG map_num, BPLONG keys) {
    BPLONG i, lst, key;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            key = unnumberVarTabledTerm(kvp_ptr->key);
            FOLLOW(heap_top) = key;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("table");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, keys);
}

int b_PICAT_TABLE_MAP_VALS_cf(BPLONG map_num, BPLONG vals) {
    BPLONG i, lst, val;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            val = unnumberVarTabledTerm(kvp_ptr->val);
            FOLLOW(heap_top) = val;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("table");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, vals);
}

int b_PICAT_TABLE_MAP_LIST_cf(BPLONG map_num, BPLONG pairs) {
    BPLONG i, lst, key, val, pair;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);

    mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            key = unnumberVarTabledTerm(kvp_ptr->key);
            PREPARE_UNNUMBER_TERM(local_top);
            val = unnumberVarTabledTerm(kvp_ptr->val);
            pair = ADDTAG(heap_top, STR);
            FOLLOW(heap_top++) = (BPLONG)equal_psc;
            FOLLOW(heap_top++) = key;
            FOLLOW(heap_top++) = val;
            FOLLOW(heap_top) = pair;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("table");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, pairs);
}
