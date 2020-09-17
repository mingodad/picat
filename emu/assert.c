/********************************************************************
 *   File   : assert.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: assert global database

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "bprolog.h"
#include "dynamic.h"

#define MAX_FREE_RECORD_SIZE 64

BPLONG dyn_hashtable_size = 3;

BPLONG_PTR free_record[MAX_FREE_RECORD_SIZE+1];
BPLONG free_record_count[MAX_FREE_RECORD_SIZE+1];


#define RELEASE_FREE_RECORD(record_ptr, record_size) {                  \
        if (record_size <= MAX_FREE_RECORD_SIZE) {                      \
            FOLLOW(record_ptr) = (BPLONG)free_record[record_size];      \
            free_record[record_size] = record_ptr;                      \
            free_record_count[record_size]++;                           \
        }                                                               \
        }

#define RELEASE_FREE_CLAUSE_RECORD(clause_record_ptr) {                 \
        FOLLOW((BPLONG_PTR)clause_record_ptr) = (BPLONG)free_record[5]; \
        free_record[5] = (BPLONG_PTR)clause_record_ptr;                 \
        free_record_count[5]++;                                         \
    }                                                                   \

/* a clause record has the form struct(ClRef,Head,Body,Birth,Death) */
#define ALLOCATE_CLAUSE_RECORD(clause_record_ptr) {             \
        BPLONG_PTR tmp_ptr;                                     \
        if (free_record_count[5] > 0) {                         \
            tmp_ptr = (BPLONG_PTR)free_record[5];               \
            free_record[5] = (BPLONG_PTR)FOLLOW(tmp_ptr);       \
            free_record_count[5]--;                             \
        } else {                                                \
            ALLOCATE_FROM_PAREA(tmp_ptr, 5);                    \
        }                                                       \
        clause_record_ptr = (InterpretedClausePtr)tmp_ptr;      \
        }

#define ALLOCATE_RECORD_IN_ASSERT(record_ptr, record_size) {            \
        if (record_size <= MAX_FREE_RECORD_SIZE && free_record_count[record_size] > 0) { \
            record_ptr = free_record[record_size];                      \
            free_record[record_size] = (BPLONG_PTR)FOLLOW(record_ptr);  \
            free_record_count[record_size]--;                           \
        } else {                                                        \
            ALLOCATE_FROM_PAREA(record_ptr, record_size);               \
        }                                                               \
        }

/* the EP of the predicate symbol refers to a term $addr(pred_ptr) */
#define INTERPRETED_PRED_PTR(pred) (InterpretedPredPtr)UNTAGGED_ADDR(FOLLOW((BPLONG_PTR)UNTAGGED_ADDR(pred)+1))
#define IS_INTERPRETED_PRED(pred) (ISSTRUCT(pred) && GET_STR_SYM_REC(pred) == c_object_ref_sym)

BPLONG_PTR picat_global_maps[NUM_PICAT_GLOBAL_MAPS];
BPLONG picat_global_map_ids[NUM_PICAT_GLOBAL_MAPS];

/***************************************************************************/
/* actually slots indexed 0 and 1 are never used */
void initialize_free_records() {
    int i;
    for (i = 0; i <= MAX_FREE_RECORD_SIZE; i++) {
        free_record[i] = NULL;
        free_record_count[i] = 0;
    }
}

BPLONG total_free_records_size() {
    int i;
    BPLONG total = 0;
    for (i = 0; i <= MAX_FREE_RECORD_SIZE; i++) {
        total += i*free_record_count[i];
    }
    return total*sizeof(BPLONG);
}

int c_print_pred_ref_count() {
    BPLONG Head, No;
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    InterpretedPredPtr pred_ptr;
    BPLONG pred;

    Head = ARG(1, 2);
    No = ARG(2, 2);

    DEREF(Head); DEREF(No);
    sym_ptr = GET_SYM_REC(Head);
    if (GET_ETYPE(sym_ptr) == T_DYNA) {
        pred = (BPLONG)GET_EP(sym_ptr);
        pred_ptr = INTERPRETED_PRED_PTR(pred);
        printf("ref_count=%d (after %d)\n", (int)pred_ptr->ref_count, (int)INTVAL(No));
    }
    return BP_TRUE;
}

int c_set_dyn_hashtable_size() {
    BPLONG_PTR top;
    BPLONG size = ARG(1, 1);
    DEREF(size);
    dyn_hashtable_size = INTVAL(size);

    return BP_TRUE;
}

int b_INC_PRED_REF_COUNT_c(PredPtr)
    BPLONG PredPtr;
{
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;

    DEREF(PredPtr);
    pred_ptr = (InterpretedPredPtr)UNTAGGED_ADDR(PredPtr);
    pred_ptr->ref_count++;
    return BP_TRUE;
}

int b_DEC_PRED_REF_COUNT_c(PredPtr)
    BPLONG PredPtr;
{
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;

    DEREF(PredPtr);
    pred_ptr = (InterpretedPredPtr)UNTAGGED_ADDR(PredPtr);
    pred_ptr->ref_count--;
    // printf("dec_ref (%x) ref_count = %d retr_count=%d\n",pred_ptr,pred_ptr->ref_count,pred_ptr->retr_count);
    return BP_TRUE;
}

int b_INC_PRED_RETR_COUNT_c(PredPtr)
    BPLONG PredPtr;
{
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;

    DEREF(PredPtr);
    pred_ptr = (InterpretedPredPtr)UNTAGGED_ADDR(PredPtr);
    pred_ptr->retr_count++;
    return BP_TRUE;
}

int b_DEC_PRED_RETR_COUNT_c(PredPtr)
    BPLONG PredPtr;
{
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;

    DEREF(PredPtr);
    pred_ptr = (InterpretedPredPtr)UNTAGGED_ADDR(PredPtr);
    pred_ptr->retr_count--;
    // printf("dec_retr (%x) ref_count = %d retr_count=%d\n",pred_ptr,pred_ptr->ref_count,pred_ptr->retr_count);
    return BP_TRUE;
}

/***************************************************************************/
int b_ABOLISH_cc(f, n)
    BPLONG f, n;
{
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;
    BPLONG pred;

    DEREF(f); DEREF(n);
    //  printf("ABOLISHING ");write_term(f);write_term(n);printf("\n");
    GET_GLOBAL_SYM(f, n, sym_ptr);
    if (GET_ETYPE(sym_ptr) != T_DYNA && GET_ETYPE(sym_ptr) != T_INTP) {
        BPLONG goal;
        if (GET_ETYPE(sym_ptr) == T_ORDI) return BP_TRUE;
        goal = c_error_src(GET_NAME(sym_ptr), INTVAL(n));
        bp_exception = c_permission_error(et_MODIFY, et_STATIC_PROCEDURE, goal);
        return BP_ERROR;
    }
    pred = (BPLONG)GET_EP(sym_ptr);
    if (!IS_INTERPRETED_PRED(pred)) return BP_TRUE;
    pred_ptr = INTERPRETED_PRED_PTR(pred);
    abolish_pred(pred_ptr);
    GET_ETYPE(sym_ptr) = T_ORDI;
    //  printf("<=ABOLISH\n");
    return BP_TRUE;
}

void abolish_pred(pred_ptr)
    InterpretedPredPtr pred_ptr;
{
    BPLONG_PTR hashtable, cell_ptr;
    InterpretedClausePtr clause_record_ptr;
    BPLONG list;
    int i;
    InterpretedPredBucketPtr bucket_ptr;

    pred_ptr->cl_count = 0;
    hashtable = pred_ptr->hashtable;
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+pred_ptr->bucket_size);
    list = bucket_ptr->list;
    while (ISLIST(list)) {
        cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        list = FOLLOW(cell_ptr+1);
        clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(FOLLOW(cell_ptr));
        if (pred_ptr->ref_count == 0 && pred_ptr->retr_count == 0) {
            release_clause_record_space(clause_record_ptr);
            RELEASE_FREE_RECORD(cell_ptr, 3);  /* a list cell has three fields: clause_record, next, and prev */
        }
    }
    bucket_ptr->tail = NULL;
    bucket_ptr->list = nil_sym;

    for (i = 0; i < pred_ptr->bucket_size; i++) {
        bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
        list = bucket_ptr->list;
        while (ISLIST(list)) {
            cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
            list = FOLLOW(cell_ptr+1);
            if (pred_ptr->ref_count == 0 && pred_ptr->retr_count == 0) {
                RELEASE_FREE_RECORD(cell_ptr, 3);  /* a list cell has three fields: clause_record, next, and prev */
            }
        }
        bucket_ptr->tail = NULL;
        bucket_ptr->list = nil_sym;
    }
}

/* a clause record takes form struct(CellRef,Head,Body,Birth,Death) where CellRef refers to the cell that
   wrapes this record on the last chain of the predicate
*/
void release_clause_record_space(InterpretedClausePtr clause_record_ptr) {
    release_term_space(clause_record_ptr->head);
    release_term_space(clause_record_ptr->body);
    RELEASE_FREE_CLAUSE_RECORD(clause_record_ptr);
}

/* free the space taken by a compound term for future reuse */
void release_term_space(BPLONG term) {
    BPLONG_PTR ptr;
start:
    if (ISLIST(term)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        release_term_space(FOLLOW(ptr));
        term = FOLLOW(ptr+1);
        RELEASE_FREE_RECORD(ptr, 2);
        goto start;
    } else if (ISSTRUCT(term)) {
        SYM_REC_PTR sym_ptr;
        int arity, i;

        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
        arity = GET_ARITY(sym_ptr);
        for (i = 1; i < arity; i++) {
            release_term_space(FOLLOW(ptr+i));
        }
        term = FOLLOW(ptr+arity);
        RELEASE_FREE_RECORD(ptr, arity+1);
        goto start;
    }
}

/***************************************************************************/
int b_REMOVE_CLAUSE_c(clause_record)
    BPLONG clause_record;
{
    BPLONG_PTR top;
    InterpretedClausePtr clause_record_ptr;
    BPLONG Head;
    SYM_REC_PTR sym_ptr;
    InterpretedPredPtr pred_ptr;
    BPLONG pred;

    DEREF(clause_record);
    clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(clause_record);
    if (clause_record_ptr->death_time_stamp != BP_MAXINT_1W) return BP_TRUE;  /* has been removed already */
    Head = clause_record_ptr->head;  /* struct(CellRef,Head,Body,Birth,Death) */
    sym_ptr = GET_SYM_REC(Head);

    pred = (BPLONG)GET_EP(sym_ptr);
    pred_ptr = INTERPRETED_PRED_PTR(pred);

    pred_ptr->cl_count--;
    if (pred_ptr->ref_count != 0 || pred_ptr->retr_count > 1) {
        pred_ptr->time_stamp++;
        clause_record_ptr->death_time_stamp = pred_ptr->time_stamp;
    } else {
        locate_and_free_clause_record(pred_ptr, clause_record);
    }
    return BP_TRUE;
}

void locate_and_free_clause_record(pred_ptr, clause_record)
    InterpretedPredPtr pred_ptr;
    BPLONG clause_record;
{
    int hashval, i;
    BPLONG_PTR cell_ptr, hashtable;
    InterpretedClausePtr clause_record_ptr;
    InterpretedPredBucketPtr bucket_ptr;

    clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(clause_record);
    hashtable = pred_ptr->hashtable;
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+pred_ptr->bucket_size);
    cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(clause_record_ptr->cl_ref);  /* clause_record_ptr points to struct(CellRef,Head,Body,Birth,Death) */
    disconnect_cell_of_removed_clause(bucket_ptr, cell_ptr);

    hashval = hashval_in_assert(clause_record_ptr);
    if (hashval == 0) { /* consider buckets 0-size-1 */
        for (i = 0; i < pred_ptr->bucket_size; i++) {
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
            free_cell_of_removed_in_bucket(pred_ptr, bucket_ptr, clause_record);
        }
    } else {
        bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+hashval%pred_ptr->bucket_size);
        free_cell_of_removed_in_bucket(pred_ptr, bucket_ptr, clause_record);
    }

    if (pred_ptr->ref_count == 0 && pred_ptr->retr_count <= 1) {  /* this pred is being accessed only by this retract call */
        release_clause_record_space(clause_record_ptr);
        RELEASE_FREE_RECORD(cell_ptr, 3);
    }
}

/* locate the wrapper cell of the clause record and then disconnect it */
void free_cell_of_removed_in_bucket(pred_ptr, bucket_ptr, removed_clause_record)
    InterpretedPredPtr pred_ptr;
    InterpretedPredBucketPtr bucket_ptr;
    BPLONG removed_clause_record;
{
    BPLONG clause_record, list;
    BPLONG_PTR cell_ptr;

    list = bucket_ptr->list;
    while (ISLIST(list)) {
        cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        clause_record = FOLLOW(cell_ptr);
        if (clause_record == removed_clause_record) {
            disconnect_cell_of_removed_clause(bucket_ptr, cell_ptr);
            if (pred_ptr->ref_count == 0 && pred_ptr->retr_count <= 1) {
                RELEASE_FREE_RECORD(cell_ptr, 3);
            }
            return;
        }
        list = FOLLOW(cell_ptr+1);
    }
}

/* disconnect the wrapper cell of the clause record from the chain */
void disconnect_cell_of_removed_clause(bucket_ptr, cell_ptr)
    InterpretedPredBucketPtr bucket_ptr;
    BPLONG_PTR cell_ptr;
{
    BPLONG list;
    BPLONG_PTR prev_ptr, ptr;
    if ((BPLONG_PTR)UNTAGGED_ADDR(bucket_ptr->list) == cell_ptr) {  /* first in the chain */
        list = FOLLOW(cell_ptr+1);  /* next */
        bucket_ptr->list = list;
        if (!ISLIST(list)) {
            bucket_ptr->tail = NULL;
        } else {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
            FOLLOW(ptr+2) = (BPLONG)NULL;  /* set prev to NULL */
        }
    } else if (bucket_ptr->tail == cell_ptr) {  /* last in the chain */
        ptr = (BPLONG_PTR)FOLLOW(cell_ptr+2);  /* prev */
        FOLLOW(cell_ptr+2) = (BPLONG)NULL;  /* to indicate that the cell has been removed */
        bucket_ptr->tail = ptr;
        if (ptr != NULL) {
            FOLLOW(ptr+1) = nil_sym;
        } else {
            bucket_ptr->list = nil_sym;
        }
    } else {
        prev_ptr = (BPLONG_PTR)FOLLOW(cell_ptr+2);
        if (prev_ptr == NULL) return;  /* doesn't exist anymore */
        FOLLOW(prev_ptr+1) = FOLLOW(cell_ptr+1);  /* next */
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(FOLLOW(cell_ptr+1));  /* next cell */
        FOLLOW(ptr+2) = (BPLONG)prev_ptr;
        FOLLOW(cell_ptr+2) = (BPLONG)NULL;
    }
}

/** top level, called by Prolog **/
int c_initialize_interpreted_pred()
{
    SYM_REC_PTR sym_ptr;
    BPLONG f, n, type, size;

    f = ARG(1, 4);
    n = ARG(2, 4);
    type = ARG(3, 4); DEREF_NONVAR(type); type = INTVAL(type);
    size = ARG(4, 4); DEREF_NONVAR(size); size = INTVAL(size);
    size = bp_hsize(size);  /* get the next prime number */

    GET_GLOBAL_SYM(f, n, sym_ptr);
    return (initialize_interpreted_pred(sym_ptr, type, size) == NULL) ? BP_ERROR : BP_TRUE;
}

/***************************************************************************/
int b_ASSERTA_cc(BPLONG Head, BPLONG Body)
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    InterpretedPredPtr pred_ptr;
    BPLONG res, pred;

    //  printf("asserta "); write_term(Head); printf(":-");write_term(Body); printf("\n");
    DEREF(Head);
    sym_ptr = GET_SYM_REC(Head);
    pred = (BPLONG)GET_EP(sym_ptr);
    if (IS_INTERPRETED_PRED(pred)) {
        if (GET_ETYPE(sym_ptr) == T_ORDI) {
            GET_ETYPE(sym_ptr) = T_DYNA;
        }
        pred_ptr = INTERPRETED_PRED_PTR(pred);
        if (pred_ptr->cl_count > pred_ptr->bucket_size) {
            if (rehash_interpreted_pred(pred_ptr) == BP_ERROR) return BP_ERROR;
        }
    } else {
        pred_ptr = initialize_interpreted_pred(sym_ptr, T_DYNA, dyn_hashtable_size);
        if (pred_ptr == NULL) return BP_ERROR;
    }
    if (pred_ptr->ref_count != 0 || pred_ptr->retr_count != 0) {
        pred_ptr->time_stamp++;
    }
    res = asserta_interpreted_pred(pred_ptr, Head, Body);
    if (res == BP_ERROR) return res;
    return BP_TRUE;
}

int b_ASSERTZ_cc(BPLONG Head, BPLONG Body)
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    InterpretedPredPtr pred_ptr;
    BPLONG res, pred;

    //    printf("assertz "); write_term(Head); printf(":-");write_term(Body); printf("\n");

    DEREF(Head);
    sym_ptr = GET_SYM_REC(Head);
    pred = (BPLONG)GET_EP(sym_ptr);
    if (IS_INTERPRETED_PRED(pred)) {
        if (GET_ETYPE(sym_ptr) == T_ORDI) {
            GET_ETYPE(sym_ptr) = T_DYNA;
        }
        pred_ptr = INTERPRETED_PRED_PTR(pred);
        if (pred_ptr->cl_count > pred_ptr->bucket_size)
            if (rehash_interpreted_pred(pred_ptr) == BP_ERROR) return BP_ERROR;
    } else {
        pred_ptr = initialize_interpreted_pred(sym_ptr, T_DYNA, dyn_hashtable_size);
        if (pred_ptr == NULL) return BP_ERROR;
    }
    if (pred_ptr->ref_count != 0 || pred_ptr->retr_count != 0) {
        pred_ptr->time_stamp++;
    }
    res = assertz_interpreted_pred(pred_ptr, Head, Body);
    if (res == BP_ERROR) return res;
    return BP_TRUE;
}

int b_DYN_PRED_CLAUSE_COUNT_cf(BPLONG Head, BPLONG Count) {
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    InterpretedPredPtr pred_ptr;
    BPLONG pred, cl_count;

    //    printf("assertz "); write_term(Head); printf(":-");write_term(Body); printf("\n");

    DEREF(Head);
    sym_ptr = GET_SYM_REC(Head);
    pred = (BPLONG)GET_EP(sym_ptr);
    if (IS_INTERPRETED_PRED(pred)) {
        if (GET_ETYPE(sym_ptr) == T_ORDI) {
            cl_count = 0;
        } else {
            pred_ptr = INTERPRETED_PRED_PTR(pred);
            cl_count = pred_ptr->cl_count;
        }
    } else {
        cl_count = 0;
    }
    ASSIGN_f_atom(Count, MAKEINT(cl_count));
    return BP_TRUE;
}

InterpretedPredBucketPtr new_interpreted_bucket() {
    BPLONG_PTR ptr;
    InterpretedPredBucketPtr bucket_ptr;

    ALLOCATE_RECORD_IN_ASSERT(ptr, 2);
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return NULL;
    }
    bucket_ptr = (InterpretedPredBucketPtr)ptr;
    bucket_ptr->tail = NULL;
    bucket_ptr->list = nil_sym;
    return bucket_ptr;
}

BPLONG_PTR new_interpreted_pred_hashtable(size)
    int size;
{
    BPLONG_PTR hashtable;
    int i;

    hashtable = (BPLONG_PTR)malloc(sizeof(BPLONG)*(size+1));
    if (hashtable == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return NULL;
    }
    for (i = 0; i <= size; i++) {
        FOLLOW(hashtable+i) = (BPLONG)new_interpreted_bucket();
        if (FOLLOW(hashtable+i) == (BPLONG)NULL) {
            return NULL;
        }
    }
    return hashtable;
}


InterpretedPredPtr new_interpreted_pred_record(size)
    BPLONG size;
{
    BPLONG_PTR ptr;
    InterpretedPredPtr pred_ptr;

    ALLOCATE_FROM_PAREA(ptr, sizeof(InterpretedPred)/sizeof(BPLONG));
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return NULL;
    }
    pred_ptr = (InterpretedPredPtr)ptr;
    pred_ptr->ref_count = 0;
    pred_ptr->retr_count = 0;
    pred_ptr->cl_count = 0;
    pred_ptr->time_stamp = 0;
    pred_ptr->bucket_size = size;
    pred_ptr->hashtable = new_interpreted_pred_hashtable(size);
    if (pred_ptr->hashtable == NULL) return NULL;
    return pred_ptr;
}

InterpretedPredPtr initialize_interpreted_pred(sym_ptr, type, size)
    SYM_REC_PTR sym_ptr;
BPLONG type, size;
{
    InterpretedPredPtr pred_ptr;
    BPLONG_PTR ptr;
    BPLONG pred;

    GET_ETYPE(sym_ptr) = (BYTE)type;
    pred = (BPLONG)GET_EP(sym_ptr);
    if (IS_INTERPRETED_PRED(pred)) {
        pred_ptr = INTERPRETED_PRED_PTR(pred);
        abolish_pred(pred_ptr);
    } else {
        pred_ptr = new_interpreted_pred_record(size);
        if (pred_ptr == NULL) return NULL;
        ALLOCATE_RECORD_IN_ASSERT(ptr, 2);
        if (ptr == NULL) {
            bp_exception = et_OUT_OF_MEMORY;
            return NULL;
        }
        GET_EP(sym_ptr) = (int (*)(void))ADDTAG(ptr, STR);
        FOLLOW(ptr++) = (BPLONG)c_object_ref_sym;
        FOLLOW(ptr) = ADDTAG((BPLONG)pred_ptr, INT_TAG);
    }
    return pred_ptr;
}


/*
  extend the hashtable when the number of clauses exceeds 2*(the hashtable size), 
  reusing the old hashtable as possible:

  1. Move all the clauses list to the new hashtable as it is.
  2. Move all bucket records to the new hashtable. add (newsize-oldsize) bucket records.
  3. Move the last chain (hashtable+oldsize) to the new hashtable. For each clause whose hash value is 0,
  add a list cell to each of the newly added chains (oldsize+1,oldsize+2,...,newsize)
  4. Move the other chains to the new hashtable.
  5. The list structures are kept the same. Let (C1,C2) be a list. If C1 is deleted, 
  C2 is the next clause after C1 after copy.
*/
int rehash_interpreted_pred(pred_ptr)
    InterpretedPredPtr pred_ptr;
{
    InterpretedPredBucketPtr bucket_ptr;
    BPLONG old_size, new_size;
    BPLONG_PTR old_hashtable, new_hashtable, cell_ptr;
    InterpretedClausePtr clause_record_ptr;
    BPLONG i, hashval, clause_record;
    BPLONG list;

    /*  printf("=>REHASH \n");  */
    old_size = pred_ptr->bucket_size;
    new_size = 2*old_size;  /* no predicate can contain 2^sizeof(long) clauses, so no overflow is checked */
    new_size = bp_hsize(new_size);
    old_hashtable = pred_ptr->hashtable;

    /* allocate and initialize a new hashtable */
    new_hashtable = (BPLONG_PTR)malloc(sizeof(BPLONG)*(new_size+1));
    if (new_hashtable == NULL) return BP_TRUE;  /* stop rehashing*/
    pred_ptr->hashtable = new_hashtable;
    pred_ptr->bucket_size = new_size;

    FOLLOW(new_hashtable+new_size) = FOLLOW(old_hashtable+old_size);  /* the last chain is the same */
    /* release cells in the old table except the last chain*/
    for (i = 0; i < old_size; i++) {
        bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(old_hashtable+i);
        list = bucket_ptr->list;
        while (ISLIST(list)) {
            cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
            list = FOLLOW(cell_ptr+1);
            RELEASE_FREE_RECORD(cell_ptr, 3);
        }
        bucket_ptr->tail = NULL;
        bucket_ptr->list = nil_sym;
        FOLLOW(new_hashtable+i) = (BPLONG)bucket_ptr;
    }
    for (i = old_size; i < new_size; i++) {
        FOLLOW(new_hashtable+i) = (BPLONG)new_interpreted_bucket();
        if (FOLLOW(new_hashtable+i) == (BPLONG)NULL) {
            return BP_ERROR;
        }
    }

    /* re-assert all the clauses on the chain (old_size) */
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(new_hashtable+new_size);
    list = bucket_ptr->list;
    while (ISLIST(list)) {
        cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        list = FOLLOW(cell_ptr+1);
        clause_record = FOLLOW(cell_ptr);
        clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(clause_record);
        hashval = hashval_in_assert(clause_record_ptr);
        if (hashval == 0) {  /* a nondiscriminating clause has hashval 0 */
            for (i = 0; i < new_size; i++) {
                bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(new_hashtable+i);
                if (assertz_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
            }
        } else {
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(new_hashtable+(hashval%new_size));
            if (assertz_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
        }
    }
    free(old_hashtable);
    return BP_TRUE;
}


/* Recall that a clause record is a Prolog structure in the form struct(CellRef,Head,Body,Birth,Death)
   where CellRef is used to reference the enclosing d-list cell */
BPLONG create_clause_record(pred_ptr, head, body)
    InterpretedPredPtr pred_ptr;
BPLONG head, body;
{
    BPLONG clause_record, tmp;
    InterpretedClausePtr clause_record_ptr;
    BPLONG varno = 1;

    ALLOCATE_CLAUSE_RECORD(clause_record_ptr);
    if (clause_record_ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    clause_record = ADDTAG((BPLONG)clause_record_ptr, STR);
    tmp = numberVarCopyToParea(head, &varno);
    if (tmp == BP_ERROR) return BP_ERROR;
    clause_record_ptr->head = tmp;
    tmp = numberVarCopyToParea(body, &varno);
    if (tmp == BP_ERROR) return BP_ERROR;
    clause_record_ptr->body = tmp;
    clause_record_ptr->birth_time_stamp = pred_ptr->time_stamp;
    clause_record_ptr->death_time_stamp = BP_MAXINT_1W;
    return clause_record;
}

BPLONG_PTR asserta_clause_record(bucket_ptr, clause_record)
    InterpretedPredBucketPtr bucket_ptr;
    BPLONG clause_record;
{
    BPLONG_PTR cell_ptr, ptr;

    /*  assert_print_cls(bucket_ptr->list); */
    ALLOCATE_RECORD_IN_ASSERT(ptr, 3);
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return NULL;
    }
    if (bucket_ptr->list == nil_sym) {
        FOLLOW(ptr) = clause_record;  /* car */
        FOLLOW(ptr+1) = nil_sym;  /* next */
        FOLLOW(ptr+2) = (BPLONG)NULL;  /* prev */
        bucket_ptr->tail = ptr;
    } else {
        FOLLOW(ptr) = clause_record;  /* car */
        FOLLOW(ptr+1) = bucket_ptr->list;  /* next */
        FOLLOW(ptr+2) = (BPLONG)NULL;  /* prev */
        cell_ptr = (BPLONG_PTR)UNTAGGED_ADDR(bucket_ptr->list);
        FOLLOW(cell_ptr+2) = (BPLONG)ptr;
    }
    bucket_ptr->list = ADDTAG(ptr, LST);
    return ptr;
}

int asserta_interpreted_pred(pred_ptr, head, body)
    InterpretedPredPtr pred_ptr;
BPLONG head, body;
{
    BPLONG clause_record;
    BPLONG_PTR hashtable, cell_ptr;
    InterpretedPredBucketPtr bucket_ptr;
    int i, hashval;
    InterpretedClausePtr clause_record_ptr;

    hashtable = pred_ptr->hashtable;
    pred_ptr->cl_count++;
    clause_record = create_clause_record(pred_ptr, head, body);
    if (clause_record == BP_ERROR) return BP_ERROR;
    clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(clause_record);

    hashval = hashval_in_assert(clause_record_ptr);
    if (hashval == 0) {  /* no arg or first arg is var */
        for (i = 0; i < pred_ptr->bucket_size; i++) {
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
            if (asserta_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
        }
    } else {
        if (pred_ptr->bucket_size != 0) {  /* how can pred_ptr->bucket_size be 0? */
            i = hashval%pred_ptr->bucket_size;
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
            if (asserta_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
        }
    }
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+pred_ptr->bucket_size);
    cell_ptr = asserta_clause_record(bucket_ptr, clause_record);
    if (cell_ptr == NULL) return BP_ERROR;
    clause_record_ptr->cl_ref = ADDTAG((BPLONG)cell_ptr, INT_TAG);  /* struct(CellRef,Head,Body,Birth,Death) */
    return BP_TRUE;
}

BPLONG_PTR assertz_clause_record(bucket_ptr, clause_record)
    InterpretedPredBucketPtr bucket_ptr;
    BPLONG clause_record;
{
    BPLONG_PTR cell_ptr, ptr;

    /*  assert_print_cls(bucket_ptr->list); */
    ALLOCATE_RECORD_IN_ASSERT(ptr, 3);
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return NULL;
    }
    if (bucket_ptr->list == nil_sym) {
        FOLLOW(ptr) = clause_record;  /* car */
        FOLLOW(ptr+1) = nil_sym;  /* cdr */
        FOLLOW(ptr+2) = (BPLONG)NULL;  /* prev */
        bucket_ptr->tail = ptr;
        bucket_ptr->list = ADDTAG(ptr, LST);
    } else {
        cell_ptr = (BPLONG_PTR)bucket_ptr->tail;
        FOLLOW(ptr) = clause_record;  /* car */
        FOLLOW(ptr+1) = nil_sym;  /* cdr */
        FOLLOW(ptr+2) = (BPLONG)cell_ptr;  /* prev */
        FOLLOW(cell_ptr+1) = ADDTAG((BPLONG)ptr, LST);
        bucket_ptr->tail = ptr;
    }
    return ptr;
}

int assertz_interpreted_pred(pred_ptr, head, body)
    InterpretedPredPtr pred_ptr;
BPLONG head, body;
{
    BPLONG clause_record;
    BPLONG_PTR hashtable;
    InterpretedPredBucketPtr bucket_ptr;
    int i, hashval;
    BPLONG_PTR cell_ptr;
    InterpretedClausePtr clause_record_ptr;

    //  printf("=>assertz_interpreted_pred pred_ptr=%x",pred_ptr); write_term(head); write_term(body); printf("\n");

    hashtable = pred_ptr->hashtable;
    pred_ptr->cl_count++;
    clause_record = create_clause_record(pred_ptr, head, body);
    if (clause_record == BP_ERROR) return BP_ERROR;

    clause_record_ptr = (InterpretedClausePtr)UNTAGGED_ADDR(clause_record);
    hashval = hashval_in_assert(clause_record_ptr);
    if (hashval == 0) {  /* no arg or first arg is var */
        for (i = 0; i < pred_ptr->bucket_size; i++) {
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
            if (assertz_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
        }
    } else {
        if (pred_ptr->bucket_size != 0) {
            i = hashval%pred_ptr->bucket_size;
            bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+i);
            if (assertz_clause_record(bucket_ptr, clause_record) == NULL) return BP_ERROR;
        }
    }
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(hashtable+pred_ptr->bucket_size);
    cell_ptr = assertz_clause_record(bucket_ptr, clause_record);
    if (cell_ptr == NULL) return BP_ERROR;
    clause_record_ptr->cl_ref = ADDTAG((BPLONG)cell_ptr, INT_TAG);  /* struct(CellRef,Head,Body,Birth,Death) */
    return BP_TRUE;
}

BPLONG hashval_in_assert(clause_record_ptr)
    InterpretedClausePtr clause_record_ptr;
{
    BPLONG head, arg1, hashcode;

    head = clause_record_ptr->head;
    if (ISATOM(head)) return 0;
    arg1 = FOLLOW((BPLONG_PTR)UNTAGGED_ADDR(head)+1);  /* arg(1,Head,Arg1) */

    BP_HASH_CODE1(arg1, hashcode, lab);
    return hashcode;
}

int b_ASSERTABLE_c(Head)
    BPLONG Head;
{
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;

    DEREF(Head);
    if (ISSTRUCT(Head)) {
        sym_ptr = (SYM_REC_PTR)FOLLOW(UNTAGGED_ADDR(Head));
        if (sym_ptr == bigint_psc || sym_ptr == float_psc)
            return BP_FALSE;
    } else if (ISATOM(Head))
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(Head);
    else return BP_FALSE;
    return (GET_ETYPE(sym_ptr) == T_ORDI || GET_ETYPE(sym_ptr) == T_DYNA) ? BP_TRUE : BP_FALSE;
}

int b_RETRACTABLE_c(Head)
    BPLONG Head;
{
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;

    DEREF(Head);
    if (ISSTRUCT(Head))
        sym_ptr = (SYM_REC_PTR)FOLLOW(UNTAGGED_ADDR(Head));
    else if (ISATOM(Head))
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(Head);
    else return BP_FALSE;
    return (GET_ETYPE(sym_ptr) == T_ORDI || GET_ETYPE(sym_ptr) == T_DYNA) ? BP_TRUE : BP_FALSE;
}


/***************************************************************************/
int b_GET_PRED_PTR_cff(Head, PredPtr, IsDynamic)
    BPLONG Head, PredPtr, IsDynamic;
{
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;
    BPLONG pred, is_dynamic;

    DEREF(Head);
    if (ISSTRUCT(Head)) {
        sym_ptr = GET_STR_SYM_REC(Head);
    } else if (ISATOM(Head)) {
        sym_ptr = GET_ATM_SYM_REC(Head);
    } else {
        if (ISREF(Head)) {
            bp_exception = et_INSTANTIATION_ERROR;
        } else {
            bp_exception = c_type_error(et_CALLABLE, Head);
        }
        return BP_ERROR;
    }

    if (GET_ETYPE(sym_ptr) == T_DYNA) {
#ifdef NO_ISO_ASSERT
        is_dynamic = BP_MONE;
#else
        is_dynamic = BP_ONE;
#endif
    } else if (GET_ETYPE(sym_ptr) == T_INTP) {
        is_dynamic = BP_ZERO;
    } else if (GET_ETYPE(sym_ptr) == T_ORDI) {
        return BP_FALSE;
    } else {
        bp_exception = c_permission_error(et_ACCESS, et_PRIVATE_PROCEDURE, Head);
        return BP_ERROR;
    }

    pred = (BPLONG)GET_EP(sym_ptr);
    if (IS_INTERPRETED_PRED(pred)) {
        pred_ptr = INTERPRETED_PRED_PTR(pred);
        ASSIGN_f_atom(PredPtr, ADDTAG((BPLONG)pred_ptr, INT_TAG));
        ASSIGN_f_atom(IsDynamic, is_dynamic);
        return BP_TRUE;
    }
    return BP_FALSE;
}


/* The predicate of Head is known to be either T_DYNA or T_INT */
int b_GET_CLAUSES_cfff(Head, Clauses, Key, TimeStamp)
    BPLONG Clauses, Head, Key, TimeStamp;
{
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;
    InterpretedPredPtr pred_ptr;
    BPLONG pred, arg1, arg1_key;
    InterpretedPredBucketPtr bucket_ptr;
    int i;

    DEREF(Head);
    sym_ptr = GET_SYM_REC(Head);
    pred = (BPLONG)GET_EP(sym_ptr);
    pred_ptr = INTERPRETED_PRED_PTR(pred);
    if (pred_ptr->bucket_size != 0 && ISSTRUCT(Head)) {
        arg1 = FOLLOW((BPLONG_PTR)UNTAGGED_ADDR(Head)+1);  /* arg(1,Head,Arg1) */
        BP_HASH_KEY1_CODE1(arg1, arg1_key, i, lab1);
        ASSIGN_f_atom(Key, arg1_key);
        if (i == 0) i = pred_ptr->bucket_size; else i = i%pred_ptr->bucket_size;
    } else {
        i = pred_ptr->bucket_size;
        ASSIGN_f_atom(Key, BP_ZERO);
    }
    bucket_ptr = (InterpretedPredBucketPtr)FOLLOW(pred_ptr->hashtable+i);
    ASSIGN_f_atom(TimeStamp, MAKEINT(pred_ptr->time_stamp));
    ASSIGN_sv_heap_term(Clauses, bucket_ptr->list);
    return BP_TRUE;
}


void Cboot_assert() {
    insert_cpred("c_initialize_interpred", 4, c_initialize_interpreted_pred);
    insert_cpred("c_set_dyn_hashtable_size", 1, c_set_dyn_hashtable_size);
    insert_cpred("c_print_pred_ref_count", 2, c_print_pred_ref_count);
}

/***************************************************************************/
BPLONG numberVarCopyToParea(term, varno)
    BPLONG term;
    BPLONG *varno;
{
    BPLONG_PTR term_ptr, ptr;
    BPLONG_PTR top;

    DEREF(term);
    if (TAG(term) == ATM || IsNumberedVar(term))
        return term;
    else if (ISREF(term)) {
        ASSIGN_TRAIL_VALUE(term, NumberVar(*varno));
        *varno = *varno+1;
        return FOLLOW(term);
    } else if (ISLIST(term)) {
        return numberVarCopyListToParea(term, varno);
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity, size;
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        if (FOLLOW(term_ptr) == (BPLONG)comma_psc) {
            return numberVarCopyCommaToParea(term, varno);
        }
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(term_ptr));
        size = arity+1;
        ALLOCATE_RECORD_IN_ASSERT(ptr, size);
        if (ptr == NULL) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        FOLLOW(ptr) = FOLLOW(term_ptr);
        for (i = 1; i <= arity; i++) {
            BPLONG tmp = numberVarCopyToParea(FOLLOW(term_ptr+i), varno);
            if (tmp == BP_ERROR) return BP_ERROR;
            FOLLOW(ptr+i) = tmp;
        }
        return ADDTAG(ptr, STR);
    } else {
        ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
        PUSHTRAILC_ATOMIC(ptr, term);
        FOLLOW(ptr) = NumberVar(*varno);
        *varno = *varno+1;
        return FOLLOW(ptr);
    }
}

/* term in the form of [a,b,...] */
BPLONG numberVarCopyListToParea(term, varno)
    BPLONG term;
    BPLONG *varno;
{
    BPLONG_PTR ret_term_ptr, ptr, top;
    BPLONG ret_term, tmp;

    ret_term_ptr = &ret_term;
    while (ISLIST(term)) {
        UNTAG_ADDR(term);
        ALLOCATE_RECORD_IN_ASSERT(ptr, 2);
        if (ptr == NULL) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        FOLLOW(ret_term_ptr) = ADDTAG(ptr, LST);
        tmp = numberVarCopyToParea(FOLLOW(term), varno);
        if (tmp == BP_ERROR) return BP_ERROR;
        FOLLOW(ptr) = tmp;
        ret_term_ptr = ptr+1;
        term = FOLLOW((BPLONG_PTR)term+1);
        DEREF(term);
    }
    tmp = numberVarCopyToParea(term, varno);
    if (tmp == BP_ERROR) return BP_ERROR;
    FOLLOW(ret_term_ptr) = tmp;
    return ret_term;
}

/* term in the form of (a,b,...) */
BPLONG numberVarCopyCommaToParea(term, varno)
    BPLONG term;
    BPLONG *varno;
{
    BPLONG_PTR term_ptr, ptr, ret_term_ptr, top;
    BPLONG ret_term, tmp;

    ret_term_ptr = &ret_term;
    while (ISSTRUCT(term)) {
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        if (FOLLOW(term_ptr) != (BPLONG)comma_psc) break;
        ALLOCATE_RECORD_IN_ASSERT(ptr, 3);
        if (ptr == NULL) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        FOLLOW(ret_term_ptr) = ADDTAG(ptr, STR);
        FOLLOW(ptr) = (BPLONG)comma_psc;
        tmp = numberVarCopyToParea(FOLLOW(term_ptr+1), varno);
        if (tmp == BP_ERROR) return BP_ERROR;
        FOLLOW(ptr+1) = tmp;
        ret_term_ptr = ptr+2;
        term = FOLLOW(term_ptr+2);
        DEREF(term);
    }
    tmp = numberVarCopyToParea(term, varno);
    if (tmp == BP_ERROR) return BP_ERROR;
    FOLLOW(ret_term_ptr) = tmp;
    return ret_term;
}

/*  defined basic.h, also see table_maps in table.c

    There is a big difference between a global map and a table map:
    a global map hashes a key-value pair by using a hash code generated 
    from the main functor of the key, while a table map hashes a key-value 
    pair by using a hash code generated from the entire key.

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

void init_picat_global_maps() {
    BPLONG i;

    for (i = 0; i < NUM_PICAT_GLOBAL_MAPS; i++) {
        picat_global_maps[i] = NULL;
    }
}

/* Return the number of the map with map_id. If no map with the id was found,
   then create a new map and register it into global_maps. Linear prob is used 
   to look for the map with map_id.

   Each entry in global_maps is a pointer to a MAP_RECORD, which stores the 
   information about the map, including the size of the bucket table, the number 
   of key-value pairs (count), and a pointer to the bucket table (htable).
*/
int b_GET_PICAT_GLOBAL_MAP_cf(BPLONG map_id, BPLONG map_num) {
    BPLONG slot_i0, slot_i, i, map_id_cp, this_hcode, varno;
    BPLONG_PTR tmp_ptr;
    MAP_RECORD_PTR map_ptr;

    this_hcode = bp_hashval(map_id);
    slot_i0 = slot_i = (this_hcode % NUM_PICAT_GLOBAL_MAPS);

    // linear prob
    while ((BPLONG_PTR)picat_global_maps[slot_i] != NULL) {
        if (key_identical(picat_global_map_ids[slot_i], map_id)) {
            return unify(map_num, MAKEINT(slot_i));
        }
        slot_i++;
        if (slot_i == NUM_PICAT_GLOBAL_MAPS) slot_i = 0;
        if (slot_i == slot_i0) quit("GLOBAL MAPS OVERFLOW");
    }
    // Come here if map_id was not found. Register a map in slot i
    varno = 0;
    map_id_cp = numberVarCopyToParea(map_id, &varno);
    if (map_id_cp == BP_ERROR) return BP_ERROR;
    if (varno != 0) {
        bp_exception = ground_expected;
        return BP_ERROR;
    }

    ALLOCATE_FROM_PAREA(tmp_ptr, sizeof(MAP_RECORD)/sizeof(BPLONG));
    if (tmp_ptr == NULL) myquit(OUT_OF_MEMORY, "global_maps");
    map_ptr = (MAP_RECORD_PTR)tmp_ptr;
    map_ptr->count = 0;
    map_ptr->size = 7;  // initial size
    tmp_ptr = (BPLONG_PTR)malloc(7*sizeof(BPLONG_PTR));
    map_ptr->htable = tmp_ptr;
    for (i = 0; i < 7; i++)
        FOLLOW(tmp_ptr+i) = (BPLONG)NULL;

    picat_global_maps[slot_i] = (BPLONG_PTR)map_ptr;

    picat_global_map_ids[slot_i] = map_id_cp;
    return unify(map_num, MAKEINT(slot_i));
}

void expand_picat_global_map(MAP_RECORD_PTR mr_ptr) {
    BPLONG new_htable_size, old_htable_size, i, key, this_hcode;
    BPLONG_PTR new_htable, old_htable;
    KEY_VAL_PAIR_PTR kvp_ptr, next_kvp_ptr;

    old_htable_size = mr_ptr->size;
    old_htable = mr_ptr->htable;
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

    //  printf("global_map expand %d\n",new_htable_size);

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
            key = kvp_ptr->key;
            BP_HASH_CODE1(key, this_hcode, lab);
            new_kvp_ptr_ptr = (BPLONG_PTR)(new_htable + this_hcode%new_htable_size);
            kvp_ptr->next = (BPLONG_PTR)FOLLOW(new_kvp_ptr_ptr);
            FOLLOW(new_kvp_ptr_ptr) = (BPLONG)kvp_ptr;
            kvp_ptr = next_kvp_ptr;
        }
    }
    free(old_htable);
    mr_ptr->size = new_htable_size;
    mr_ptr->htable = new_htable;
}

int b_PICAT_GLOBAL_MAP_PUT_ccc(BPLONG map_num, BPLONG key, BPLONG val) {
    BPLONG i, key_cp, val_cp, this_hcode, dummy_hcode, varno;
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

    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];
    BP_HASH_CODE1(key, this_hcode, lab);

    kvp_ptr_ptr = (BPLONG_PTR)(mr_ptr->htable + (this_hcode % mr_ptr->size));
    kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
    varno = 0;
    val_cp = numberVarCopyToParea(val, &varno);
    if (val_cp == BP_ERROR) return BP_ERROR;

    while (kvp_ptr != NULL) {  /* lookup */
        if (!key_identical(kvp_ptr->key, key)) {
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        } else {
            release_term_space(kvp_ptr->val);
            kvp_ptr->val = val_cp;  /* update */
            goto lookup_end;
        }
    }
    // come here if lookup failed

    varno = 0;
    key_cp = numberVarCopyToParea(key, &varno);
    if (key_cp == BP_ERROR) return BP_ERROR;

    ALLOCATE_FROM_PAREA(tmp_ptr, sizeof(KEY_VAL_PAIR)/sizeof(BPLONG));
    if (tmp_ptr == NULL) myquit(OUT_OF_MEMORY, "global_maps");
    kvp_ptr = (KEY_VAL_PAIR_PTR)tmp_ptr;
    kvp_ptr->key = key_cp;
    kvp_ptr->val = val_cp;
    kvp_ptr->next = (BPLONG_PTR)FOLLOW(kvp_ptr_ptr);
    FOLLOW(kvp_ptr_ptr) = (BPLONG)kvp_ptr;

    mr_ptr->count++;
    if (2*mr_ptr->count > mr_ptr->size)
        expand_picat_global_map(mr_ptr);

lookup_end:
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;
    return BP_TRUE;
}

int b_PICAT_GLOBAL_MAP_GET_ccf(BPLONG map_num, BPLONG key, BPLONG val) {
    BPLONG i, key_cp, val_cp, this_hcode, varno;
    BPLONG_PTR trail_top0, kvp_ptr_ptr;
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
    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];

    BP_HASH_CODE1(key, this_hcode, lab);

    kvp_ptr_ptr = (BPLONG_PTR)(mr_ptr->htable + (this_hcode % mr_ptr->size));
    kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

    while (kvp_ptr != NULL) {  /* lookup */
        if (!key_identical(key, kvp_ptr->key)) {
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        } else {
            PREPARE_UNNUMBER_TERM(local_top);
            return unify(val, unnumberVarTermOpt(kvp_ptr->val));
        }
    }
    return BP_FALSE;
}

int b_PICAT_GLOBAL_MAP_SIZE_cf(BPLONG map_num, BPLONG size) {
    MAP_RECORD_PTR mr_ptr;

    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);

    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];
    return unify(size, MAKEINT(mr_ptr->count));
}

int b_PICAT_GLOBAL_MAP_CLEAR_c(BPLONG map_num) {
    int i;
    MAP_RECORD_PTR mr_ptr;
    BPLONG_PTR htable;

    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];
    mr_ptr-> count = 0;
    htable = mr_ptr->htable;
    for (i = 0; i < mr_ptr->size; i++) {
        BPLONG term = FOLLOW(htable+i);
        if (term != (BPLONG)NULL) {
            release_term_space(term);
            FOLLOW(htable+i) = (BPLONG)NULL;
        }
    }
    return BP_TRUE;
}

int b_PICAT_GLOBAL_MAP_KEYS_cf(BPLONG map_num, BPLONG keys) {
    BPLONG i, lst, key;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            key = unnumberVarTermOpt(kvp_ptr->key);
            FOLLOW(heap_top) = key;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("global");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, keys);
}

int b_PICAT_GLOBAL_MAP_VALS_cf(BPLONG map_num, BPLONG vals) {
    BPLONG i, lst, val;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);
    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            val = unnumberVarTermOpt(kvp_ptr->val);
            FOLLOW(heap_top) = val;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("global");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, vals);
}

int b_PICAT_GLOBAL_MAP_LIST_cf(BPLONG map_num, BPLONG pairs) {
    BPLONG i, lst, key, val, pair;
    BPLONG_PTR kvp_ptr_ptr;
    MAP_RECORD_PTR mr_ptr;
    KEY_VAL_PAIR_PTR kvp_ptr;

    lst = nil_sym;
    DEREF_NONVAR(map_num);
    map_num = INTVAL(map_num);

    mr_ptr = (MAP_RECORD_PTR)picat_global_maps[map_num];

    for (i = 0; i < mr_ptr->size; i++) {
        kvp_ptr_ptr = (mr_ptr->htable+i);
        kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

        while (kvp_ptr != NULL) {  /* lookup */
            PREPARE_UNNUMBER_TERM(local_top);
            key = unnumberVarTermOpt(kvp_ptr->key);
            PREPARE_UNNUMBER_TERM(local_top);
            val = unnumberVarTermOpt(kvp_ptr->val);
            pair = ADDTAG(heap_top, STR);
            FOLLOW(heap_top++) = (BPLONG)equal_psc;
            FOLLOW(heap_top++) = key;
            FOLLOW(heap_top++) = val;
            FOLLOW(heap_top) = pair;
            FOLLOW(heap_top+1) = lst;
            lst = ADDTAG(heap_top, LST);
            heap_top += 2;
            LOCAL_OVERFLOW_CHECK("global");
            kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
        }
    }
    return unify(lst, pairs);
}


