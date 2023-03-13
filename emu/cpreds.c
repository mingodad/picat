/********************************************************************
 *   File   : cpreds.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2023
 *   Purpose: Non-inline built-ins in C

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 ********************************************************************/

#include <string.h>
#include <stdlib.h>
#include "picat.h"
#include <time.h>
/*
  extern void Cboot_libdata(void);                                                                                      //branch
  extern void Cboot_libklr(void);                                                                                               //branch
  extern void Cboot_libprofare(void);                                                                                   //branch
  extern void Cboot_libtaxtextdata(void);                                                                                       //branch
*/
extern char *string_in;


/* this function is by Steve Branch */
int bp_signal(int signo, void user_signal_handler(int, void *), void *userdata)
{
    user_signal_action[signo] = user_signal_handler;
    user_signal_data[signo] = userdata;

    if (signal(signo, exception_handler) == SIG_ERR)
    {
        printf("can't catch user signal %d\n", signo);
    }
    return(BP_TRUE);
}

/* this function is by Steve Branch */
void *bp_get_address(BPLONG t)
{
    BPULONG ulint = 0;

    DEREF(t);
    if (ISINT(t)) {
        if (t != BP_ZERO) printf("expected address, found integer\n");
        return(NULL);  //zero represented as an integer for prolog checks
    } else if (ISADDR(t)) {
        UNTAG_ADDR(t);
        ulint = INTVAL(*((BPLONG_PTR)t+1));
        ulint = ulint << (sizeof(BPLONG)*4);
        ulint = ulint | INTVAL(*((BPLONG_PTR)t+2));
        return((void *)ulint);
    } else {
        bp_exception = number_expected;
        return NULL;
    }
}

/* this function is by Steve Branch */
BPLONG bp_build_address(void *address) {
    BPLONG temp;

    if (address) {
        temp = ADDTAG(heap_top, STR);
        NEW_HEAP_NODE((BPLONG)address_psc);  /* '$address'(void *)   */
        NEW_HEAP_NODE(MAKEINT(((BPULONG)address >> (sizeof(BPLONG)*4))));
        NEW_HEAP_NODE(MAKEINT(((BPULONG)address << (sizeof(BPLONG)*4)) >> (sizeof(BPLONG)*4)));
    } else {
        temp = BP_ZERO;  // prolog code is checking for zero
    }
    return temp;
}

int identical_VAR_VAR(t1, t2)
    BPLONG t1;
    BPLONG t2;
{
    BPLONG_PTR top;
    DEREF(t1);
    DEREF(t2);
    return t1 == t2;
}

BPLONG bp_get_call_arg(i, arity)
    BPLONG i, arity;
{
    return ARG(i, arity);
}

BPLONG picat_get_call_arg(i, arity)
    BPLONG i, arity;
{
    return ARG(i, arity);
}

int bp_is_var(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISREF(t);
}

int picat_is_var(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISREF(t);
}

int picat_is_attr_var(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return IS_SUSP_VAR(t);
}

int picat_is_dvar(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (IS_SUSP_VAR(t)) {
        BPLONG_PTR dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(t);
        return !IS_UN_DOMAIN(dv_ptr);
    }
    return BP_FALSE;
}

int picat_is_bool_dvar(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (IS_SUSP_VAR(t)) {
        BPLONG_PTR dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(t);
        if (IS_UN_DOMAIN(dv_ptr)) return BP_FALSE;
        if (DV_first(dv_ptr) == 0 && DV_last(dv_ptr) == 1) {
            return BP_TRUE;
        }
    }
    return BP_FALSE;
}

int bp_is_atom(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISATOM(t);
}

int picat_is_atom(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISATOM(t);
}

int bp_is_integer(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return (ISINT(t) || IS_BIGINT(t));
}

int picat_is_integer(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return (ISINT(t) || IS_BIGINT(t));
}

int bp_is_float(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISFLOAT(t);
}

int picat_is_float(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISFLOAT(t);
}

int bp_is_nil(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISNIL(t);
}

int picat_is_nil(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISNIL(t);
}

int bp_is_list(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISLIST(t);
}

int picat_is_list(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return ISLIST(t);
}

int picat_is_string(t)
    BPLONG t;
{
    return b_IS_STRING_c(t);
}

int bp_is_structure(t)
    BPLONG t;
{
    return picat_is_structure(t);
}

int picat_is_structure(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISSTRUCT(t)) {
        BPLONG_PTR struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(t);
        SYM_REC_PTR sym_ptr = (SYM_REC_PTR)FOLLOW(struct_ptr);
        if (sym_ptr != float_psc && sym_ptr != bigint_psc)
            return BP_TRUE;
    }
    return BP_FALSE;
}

int bp_is_compound(t)
    BPLONG t;
{
    return picat_is_compound(t);
}

int picat_is_compound(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    return (bp_is_list(t) || bp_is_structure(t));
}

int picat_is_array(t)
    BPLONG t;
{
    return b_IS_ARRAY_c(t);
}

int bp_is_unifiable(t1, t2)
    BPLONG t1, t2;
{
    return is_UNIFIABLE(t1, t2);
}

int picat_is_unifiable(t1, t2)
    BPLONG t1, t2;
{
    return is_UNIFIABLE(t1, t2);
}

int bp_is_identical(t1, t2)
    BPLONG t1, t2;
{
    return is_IDENTICAL(t1, t2);
}

int picat_is_identical(t1, t2)
    BPLONG t1, t2;
{
    return is_IDENTICAL(t1, t2);
}

/**/
long bp_get_integer(t)
    BPLONG t;
{
    return picat_get_integer(t) ;
}

BPLONG picat_get_integer(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISINT(t)) {
        return INTVAL(t);
    } else if (IS_BIGINT(t)) {
        return bp_bigint_to_int(t);  /* !! may lose bits */
    } else if(ISADDR(t)) {
        printf("integer expected, found address\n");
    } else {
        bp_exception = integer_expected;
        return 0;
    }
}

double bp_get_float(t)
    BPLONG t;
{
    return picat_get_float(t);
}

double picat_get_float(t)
    BPLONG t;
{
    BPLONG_PTR top;

    DEREF(t);
    if (ISINT(t)) {
        return (double)INTVAL(t);
    } else if (ISFLOAT(t)) {
        return floatval(t);
    } else {
        bp_exception = number_expected;
        return 0.0;
    }
}

char *picat_get_name(t)
    BPLONG t;
{
    DEREF(t);
    if (ISATOM(t)) {
        return picat_get_atom_name(t);
    } else {
        return picat_get_struct_name(t);
    }
}

char *bp_get_name(t)
    BPLONG t;
{
    return picat_get_name(t);
}

char *picat_get_struct_name(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISSTRUCT(t)) {
        return GET_NAME(GET_SYM_REC(t));
    } else {
        bp_exception = structure_expected;
        return NULL;
    }
}

char *bp_get_struct_name(t)
    BPLONG t;
{
    return picat_get_struct_name(t);
}

int picat_get_struct_arity(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISSTRUCT(t)) {
        BPLONG_PTR struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(t);
        SYM_REC_PTR sym_ptr = (SYM_REC_PTR)FOLLOW(struct_ptr);
        if (sym_ptr == float_psc || sym_ptr == bigint_psc) {
            bp_exception = illegal_arguments;
            return 0;
        }
        return GET_ARITY(sym_ptr);
    } else {
        bp_exception = illegal_arguments;
        return 0;
    }
}

int bp_get_struct_arity(t)
    BPLONG t;
{
    return picat_get_struct_arity(t);
}

char *picat_get_atom_name(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISATOM(t)) {
        return GET_NAME(GET_ATM_SYM_REC(t));
    } else {
        bp_exception = atom_expected;
        return NULL;
    }
}

char *bp_get_atom_name(t)
    BPLONG t;
{
    return picat_get_atom_name(t);
}

int bp_unify(t1, t2)
    BPLONG t1, t2;
{
    return unify(t1, t2);
}

int picat_unify(t1, t2)
    BPLONG t1, t2;
{
    return unify(t1, t2);
}

BPLONG picat_get_arg(i, t)
    BPLONG i;
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISLIST(t)) {
        return *((BPLONG_PTR)UNTAGGED_ADDR(t)+i-1);
    } else if (ISSTRUCT(t)) {
        return *((BPLONG_PTR)UNTAGGED_ADDR(t)+i);
    } else {
        bp_exception = compound_expected;
        return BP_ZERO;
    }
}

BPLONG bp_get_arg(i, t)
    BPLONG i;
    BPLONG t;
{
    return picat_get_arg(i, t);
}

BPLONG picat_get_car(t)
    BPLONG t;
{

    BPLONG_PTR top;
    DEREF(t);
    if (ISLIST(t)) {
        return *((BPLONG_PTR)UNTAGGED_ADDR(t));
    } else {
        bp_exception = list_expected;
        return BP_ZERO;
    }
}

BPLONG bp_get_car(t)
    BPLONG t;
{
    return picat_get_car(t);
}

BPLONG picat_get_cdr(t)
    BPLONG t;
{
    BPLONG_PTR top;
    DEREF(t);
    if (ISLIST(t)) {
        return *((BPLONG_PTR)UNTAGGED_ADDR(t)+1);
    } else {
        bp_exception = list_expected;
        return BP_ZERO;
    }
}

BPLONG bp_get_cdr(t)
    BPLONG t;
{
    return picat_get_cdr(t);
}

/**/
BPLONG picat_build_var() {
    NEW_HEAP_FREE;
    return (BPLONG)(heap_top-1);
}

BPLONG bp_build_var() {
    return picat_build_var();
}

BPLONG picat_build_integer(i)
    BPLONG i;
{
    if (i >= BP_MININT_1W && i <= BP_MAXINT_1W) {
        return MAKEINT(i);
    } else {
        return bp_int_to_bigint(i);
    }
}

BPLONG bp_build_integer(i)
    BPLONG i;
{
    return picat_build_integer(i);
}

BPLONG picat_build_float(f)
    double f;
{
    return encodefloat1(f);
}

BPLONG bp_build_float(f)
    double f;
{
    return encodefloat1(f);
}

BPLONG picat_build_atom(name)
    const char *name;
{
    return ADDTAG(insert_sym(name, strlen(name), 0), ATM);
}

BPLONG bp_build_atom(name)
    const char *name;
{
    return picat_build_atom(name);
}

BPLONG picat_build_list()
{
    BPLONG res = ADDTAG(heap_top, LST);
    NEW_HEAP_FREE;
    NEW_HEAP_FREE;
    return res;
}

BPLONG bp_build_list()
{
    return picat_build_list();
}

BPLONG picat_build_nil()
{
    return nil_sym;
}

BPLONG bp_build_nil()
{
    return nil_sym;
}

BPLONG picat_build_structure(name, arity)
    char *name;
    BPLONG arity;
{
    BPLONG res;
    BPLONG i;
    *heap_top = (BPLONG)insert_sym(name, strlen(name), arity);
    res = ADDTAG(heap_top++, STR);
    for (i = 1; i <= arity; i++) {
        NEW_HEAP_FREE;
    }
    return res;
}

BPLONG bp_build_structure(name, arity)
    char *name;
    BPLONG arity;
{
    return picat_build_structure(name, arity);
}

BPLONG picat_build_array(n)
    BPLONG n;
{
    return picat_build_structure("{}", n);
}

/* */
int atom_2_term() {
    BPLONG op1, op2, op3;
    BPLONG_PTR top;

    op1 = ARG(1, 3);
    op2 = ARG(2, 3);
    op3 = ARG(3, 3);
    DEREF(op1);
    DEREF(op2);
    return bp_string_2_term(picat_get_atom_name(op1), op2, op3);
}

int string_2_term() {
    BPLONG op1, op2, op3, list, op;
    BPLONG_PTR top, ptr;
    char *str, *curr_char_ptr;
    BPLONG res, n = 0;

    op1 = ARG(1, 3);
    op2 = ARG(2, 3);
    op3 = ARG(3, 3);
    DEREF(op1);
    DEREF(op2);
    n = list_length(op1, op1);
    if (!ISLIST(op1) || n <= 0) {bp_exception = illegal_arguments; return BP_ERROR;};
    str = (char *)malloc(n+1);
    if (str == NULL) myquit(OUT_OF_MEMORY, "st");
    curr_char_ptr = str;
    list = op1;
    while (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        op = FOLLOW(ptr); DEREF(op); *curr_char_ptr++ = (char)INTVAL(op);
        list = FOLLOW(ptr+1); DEREF(list);
    }
    *curr_char_ptr = '\0';
    res = bp_string_2_term(str, op2, op3);
    free(str);
    return res;
}

int term_2_atom() {
    BPLONG op1, op2;
    char *str;

    op1 = ARG(1, 2);
    op2 = ARG(2, 2);
    str = bp_term_2_string(op1);
    return unify(op2, ADDTAG(BP_NEW_SYM(str, 0), ATM));
}

int term_2_string() {
    BPLONG op1, op2, list;
    char *str;

    op1 = ARG(1, 2);
    op2 = ARG(2, 2);
    str = bp_term_2_string(op1);
    list = bp_build_var();
    string2codes(str, list);
    return unify(op2, list);
}

int bp_string_2_term(str, term, vars)
    char *str;
    BPLONG term;
    BPLONG vars;
{
    BPLONG res;
    BPLONG Read;
    int old_bp_gc = bp_gc;

    bp_gc = 0;  /* disenable GC */
    string_in = str;
    lastc = ' ';
    Read = bp_build_structure("catch_read_vars_once", 2);
    unify(bp_get_arg(1, Read), term);
    unify(bp_get_arg(2, Read), vars);

    res = bp_call_term(Read);
    bp_gc = old_bp_gc;

    string_in = NULL;
    lastc = ' ';
    return res;
}

/* Call a prolog term, the return value can be BP_TRUE, BP_FALSE, or BP_ERROR
   The function call toam() does not return if an exception occurs
   during its execution that is not handled
*/
int bp_call_term(term)
    BPLONG term;
{
    BPLONG res;

    init_stack(0);

    FOLLOW(arreg+1) = term;

    if (GET_ETYPE(enter_dyn_call) != T_PRED) {
        bp_exception = illegal_predicate;
        return(BP_ERROR);
    }
    inst_begin = (BPLONG_PTR) GET_EP(enter_dyn_call);
    res = toam(inst_begin, arreg, local_top);
    curr_toam_status = TOAM_NOTSET;

    return res;
}

/* Call once(term), breg's value will be restored after the call
   The function call toam() does not return if an exception occurs
   during its execution that is not handled
*/
int bp_call_term_once(term)
    BPLONG term;
{
    BPLONG res;
    BPLONG_PTR old_b = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)breg);

    init_stack(0);

    FOLLOW(arreg+1) = term;
    if (GET_ETYPE(enter_dyn_call) != T_PRED) {
        bp_exception = illegal_predicate;
        return(BP_ERROR);
    }
    inst_begin = (BPLONG_PTR) GET_EP(enter_dyn_call);
    res = toam(inst_begin, arreg, local_top);
    breg = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)old_b);
    hbreg = (BPLONG_PTR)AR_H(breg);

    curr_toam_status = TOAM_NOTSET;

    return res;
}

/* Call catch(once(term),E,Handler), where Handler sets the exception variable to E before calling halt.
   The function call toam() is guaranteed to return.
*/
int bp_call_term_catch(term)
    BPLONG term;
{
    BPLONG res;
    BPLONG_PTR old_b = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)breg);

    init_stack(0);

    FOLLOW(arreg+1) = term;
    if (GET_ETYPE(enter_catch_call) != T_PRED) {
        bp_exception = illegal_predicate;
        return(BP_ERROR);
    }
    inst_begin = (BPLONG_PTR) GET_EP(enter_catch_call);
    res = toam(inst_begin, arreg, local_top);
    breg = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)old_b);
    hbreg = (BPLONG_PTR)AR_H(breg);

    curr_toam_status = TOAM_NOTSET;

    return res;
}

int c_set_eolcom_flag() {
    BPLONG flag = ARG(1, 1);

    DEREF(flag);
    eolcom_flag = INTVAL(flag);
    return BP_TRUE;
}

int c_set_bp_exception() {
    BPLONG ex = ARG(1, 1);

    DEREF(ex);
    bp_exception = ex;
    return BP_TRUE;
}

int bp_call_string(cmd)
    char *cmd;
{
    BPLONG Pcmd, vars, res;
    int old_bp_gc;

    Pcmd = picat_build_var();
    vars = picat_build_var();

    old_bp_gc = bp_gc;
    bp_gc = 0;  /* suppress garbage collection and stack expansion */
    if (bp_string_2_term(cmd, Pcmd, vars) != BP_TRUE) {
        bp_exception = illegal_arguments;
        bp_gc = old_bp_gc;
        return BP_ERROR;
    }
    bp_gc = old_bp_gc;
    res = bp_call_term(Pcmd);
    return res;
}

int bp_mount_query_string(cmd)
    char *cmd;
{
    BPLONG res;
    int old_bp_gc;
    BPLONG term = picat_build_var();
    BPLONG vars = picat_build_var();

    old_bp_gc = bp_gc;
    bp_gc = 0;
    res = bp_string_2_term(cmd, term, vars);
    if (res != BP_TRUE) {
        fprintf(stderr, "***Error: Failed to parse %s", cmd);
        bp_gc = old_bp_gc;
        return BP_ERROR;
    }
    bp_gc = old_bp_gc;
    return bp_mount_query_term(term);
}

int bp_mount_query_term(term)
    BPLONG term;
{
    init_stack(0);
    FOLLOW(arreg+1) = term;
    curr_toam_status = TOAM_MOUNTED;
    return 1;
}

int bp_next_solution()
{
    if (curr_toam_status == TOAM_NOTSET) {
        fprintf(stderr, "***Error: no goal is mounted\n");
        bp_exception = illegal_predicate;
        return BP_ERROR;
    } else if (curr_toam_status == TOAM_DONE) {
        return 0;
    } else if (curr_toam_status == TOAM_STARTED) {
        inst_begin = addr_fail;
    } else {  /* mounted */
        curr_toam_status = TOAM_STARTED;
        inst_begin = (BPLONG_PTR) GET_EP(enter_dyn_call);
    }
    return toam(inst_begin, arreg, local_top);
}

/**/
/***********************************************************
            return solutions as strings
***********************************************************/
typedef struct {
    BPLONG size;
    BPLONG pos;
    char *buf;
} BP_STR_BAG, *BP_STR_BAG_PTR;

BP_STR_BAG_PTR bp_sol_bag_ptr;

void initialize_bp_str_bag() {
    char *mallo();
    bp_sol_bag_ptr = (BP_STR_BAG_PTR)malloc(sizeof(BP_STR_BAG));
    bp_sol_bag_ptr->size = 50;
    bp_sol_bag_ptr->buf = (char *)malloc(bp_sol_bag_ptr->size);
    bp_sol_bag_ptr->pos = 0;
}

void expand_bp_str_bag() {
    char *new_buf;
    bp_sol_bag_ptr->size *= 2;
    new_buf = (char *)malloc(bp_sol_bag_ptr->size);
    strcpy(new_buf, bp_sol_bag_ptr->buf);
    free(bp_sol_bag_ptr->buf);
    bp_sol_bag_ptr->buf = new_buf;
}

void append_str_to_solution_bag(str, len, check_quote)
    char *str;
BPLONG len, check_quote;
{
    if (bp_sol_bag_ptr->pos + len + 10 >= bp_sol_bag_ptr->size) {  /* 10 may not be enough */
        expand_bp_str_bag();
        append_str_to_solution_bag(str, len, check_quote);
    } else {
        if (check_quote == 1 && single_quote_needed(str, len)) {
            len = strcpy_add_quote(bp_sol_bag_ptr->buf+bp_sol_bag_ptr->pos, str, len);
        } else {
            strcpy(bp_sol_bag_ptr->buf+bp_sol_bag_ptr->pos, str);
        }
        bp_sol_bag_ptr->pos += len;
    }
}

char *bp_term_2_string(term)
    BPLONG term;
{
    if (bp_sol_bag_ptr == NULL) initialize_bp_str_bag();
    bp_sol_bag_ptr->pos = 0;

    aux_term_2_string_term(term);

    return bp_sol_bag_ptr->buf;
}

void aux_term_2_string_term(term)
    BPLONG term;
{
    char buf[MAX_STR_LEN];
    SYM_REC_PTR sym_ptr;
    BPLONG i, arity;
    BPLONG_PTR top;

    SWITCH_OP(term, term_2_string_l,
              {sprintf(buf, "_" BPULONG_FMT_STR, (BPULONG)term);
                  append_str_to_solution_bag(buf, strlen(buf), 0);},
              {if (ISINT(term)) {
                      sprintf(buf, "%d", (int)INTVAL(term));
                      append_str_to_solution_bag(buf, strlen(buf), 0);
                  } else {
                      sym_ptr = GET_ATM_SYM_REC(term);
                      append_str_to_solution_bag(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 1);
                  }},
              {append_str_to_solution_bag("[", 1, 0);
                  aux_term_2_string_list(term);},
              {if (ISFLOAT(term)) {
                      sprintf(buf, "%lf", floatval(term));
                      append_str_to_solution_bag(buf, strlen(buf), 0);
                  } else if (IS_SUSP_VAR(term)) {
                      sprintf(buf, "_" BPULONG_FMT_STR, (BPULONG)term);
                      append_str_to_solution_bag(buf, strlen(buf), 1);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(term);
                      append_str_to_solution_bag(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 1);
                      arity = GET_ARITY(sym_ptr);
                      append_str_to_solution_bag("(", 1, 0);
                      UNTAG_ADDR(term);
                      for (i = 1; i <= arity; i++) {
                          aux_term_2_string_term(*((BPLONG_PTR)term+i));
                          if (i < arity) append_str_to_solution_bag(",", 1, 0);
                      }
                      append_str_to_solution_bag(")", 1, 0);
                  }},
              {sprintf(buf, "_" BPULONG_FMT_STR, (BPULONG)term);
                  append_str_to_solution_bag(buf, strlen(buf), 0);});
}

void aux_term_2_string_list(term)
    BPLONG term;
{
    BPLONG temp;
    BPLONG_PTR top;

    UNTAG_ADDR(term);
    temp = FOLLOW(term);
    aux_term_2_string_term(temp);
    temp = FOLLOW((BPLONG_PTR)term+1);
    DEREF(temp);
    if (ISLIST(temp)) {
        append_str_to_solution_bag(",", 1, 0);
        aux_term_2_string_list(temp);
    } else if (!ISNIL(temp)) {
        append_str_to_solution_bag("|", 1, 0);
        aux_term_2_string_term(temp);
        append_str_to_solution_bag("]", 1, 0);
    } else
        append_str_to_solution_bag("]", 1, 0);
}

void bp_write(term)
    BPLONG term;
{
    write_term(term);
}

void picat_write_term(term)
    BPLONG term;
{
    write_term(term);
}
/**/
int currentTime() {
#if (defined(WIN32) && defined(__MINGW32__))
    BPLONG t;
#else
    long t;
#endif
    struct tm *ct;
    BPLONG Year, Month, Day, Hour, Min, Sec;

    Year = bp_get_call_arg(1, 6);
    Month = bp_get_call_arg(2, 6);
    Day = bp_get_call_arg(3, 6);
    Hour = bp_get_call_arg(4, 6);
    Min = bp_get_call_arg(5, 6);
    Sec = bp_get_call_arg(6, 6);

    time(&t);
    ct = localtime(&t);

    return (unify(Year, bp_build_integer(ct->tm_year)) &&
            unify(Month, bp_build_integer(ct->tm_mon)) &&
            unify(Day, bp_build_integer(ct->tm_mday)) &&
            unify(Hour, bp_build_integer(ct->tm_hour)) &&
            unify(Min, bp_build_integer(ct->tm_min)) &&
            unify(Sec, bp_build_integer(ct->tm_sec)));
}

/*  Example usage: p(A1,A2) */
/*
  int test(){
  TERM a1, a2, a, b, c, f1, l1, f12;
  char *name_ptr;

  a1 = picat_get_call_arg(1, 2);       // first argument
  a2 = picat_get_call_arg(2, 2);       // second argument
  a = picat_build_atom("a");
  b = picat_build_atom("b");
  c = picat_build_atom("c");
  f1 = picat_build_structure("f", 1);  // f(1)
  picat_unify(picat_get_arg(1, f1), a);
  l1 = picat_build_list();             // [1]
  picat_unify(picat_get_car(l1), picat_build_integer(1));
  picat_unify(picat_get_cdr(l1), picat_build_nil());
  f12 = picat_build_float(1.2);        // 1.2

  if (!picat_is_atom(a1))
  return PICAT_FALSE;
  name_ptr = picat_get_name(a1);
  switch (*name_ptr){
  case 'a':
  return (picat_unify(a1, a) ? picat_unify(a2, f1) : PICAT_FALSE);
  case 'b':
  return (picat_unify(a1, b) ? picat_unify(a2, l1) : PICAT_FALSE);
  case 'c':
  return (picat_unify(a1, c) ? picat_unify(a2, f12) : PICAT_FALSE);
  default: return PICAT_FALSE;
  }
  }
*/
int c_CallBpFromC() {
    BPLONG goal;
    goal = ARG(1, 1);
    return bp_call_term(goal);
}

int c_START_TRACE() {

    trace_toam = 1;
    return 1;
}

int c_END_TRACE() {
    trace_toam = 0;
    return 1;
}

/*
  printStackTrace(){
  BPLONG_PTR p = arreg;

  while (p!=(BPLONG_PTR)AR_AR(p)){
  printEntrance((BPLONG_PTR)AR_CPS(p)-1);
  p = (BPLONG_PTR)AR_AR(p);
  }
  return BP_TRUE;
  }
*/
#ifdef SAT
int c_sat_embedded() {
    return BP_TRUE;
}
#else
int c_sat_embedded() {
    return BP_FALSE;
}
#endif

int c_MAXINT_f() {
    BPLONG v = ARG(1, 1);
    return unify(v, MAKEINT(BP_MAXINT_1W));
}

int c_MININT_f() {
    BPLONG v = ARG(1, 1);
    return unify(v, MAKEINT(BP_MININT_1W));
}

int c_IS_SMALL_INT_c() {
    BPLONG v = ARG(1, 1);
    DEREF(v);
    return (ISINT(v)) ? BP_TRUE : BP_FALSE;
}

#ifdef FANN
extern int fann_cpreds();
#endif

#ifdef PRISM
extern void bp4p_register_preds();
#endif

void Cboot() {
    insert_cpred("c_format_set_dest", 1, c_format_set_dest);
    insert_cpred("c_format_get_line_pos", 1, c_format_get_line_pos);
    insert_cpred("c_format_retrieve_codes", 2, c_format_retrieve_codes);

    insert_cpred("c_COPY_TERM", 2, c_COPY_TERM);
    insert_cpred("c_COPY_TERM_SHALLOW", 2, c_COPY_TERM_SHALLOW);
    insert_cpred("c_DECREMENTARG", 2, c_DECREMENTARG);
    insert_cpred("c_END_TRACE", 0, c_END_TRACE);
    insert_cpred("c_INCREMENTARG", 2, c_INCREMENTARG);
    insert_cpred("c_SHOW_NONDET_FRAME", 1, c_SHOW_NONDET_FRAME);
    insert_cpred("c_START_TRACE", 0, c_START_TRACE);
    insert_cpred("c_TABLE_BLOCKS", 1, c_TABLE_BLOCKS);
    insert_cpred("c_STATISTICS", 0, c_STATISTICS);
    insert_cpred("c_STATISTICS0", 12, c_STATISTICS0);
    insert_cpred("c_UNIFIABLE", 2, c_UNIFIABLE);
    insert_cpred("c_UNGETC", 1, c_UNGETC);
    insert_cpred("c_arg", 3, c_arg);
    insert_cpred("c_CURRENT_PREDICATES", 1, c_CURRENT_PREDICATES);
    insert_cpred("c_CURRENT_PREDICATE", 2, c_CURRENT_PREDICATE);
    insert_cpred("c_functor", 3, c_functor);
    insert_cpred("currentTime", 6, currentTime);
    insert_cpred("number_vars", 3, c_NUMBER_VARS);
    insert_cpred("numbervars", 3, c_NUMBER_VARS);
    insert_cpred("variant", 2, c_VARIANT);
    insert_cpred("callbp", 1, c_CallBpFromC);
    insert_cpred("c_GET_NONEMPTY_LINE", 0, c_GET_NONEMPTY_LINE);
    insert_cpred("c_FINISH_GET_LINE", 0, c_FINISH_GET_LINE);
    insert_cpred("c_INITIALIZE_TABLE", 0, c_INITIALIZE_TABLE);
    insert_cpred("c_table_reset_subgoal_ar", 0, c_table_reset_subgoal_ar);
    insert_cpred("c_TABLE_GET_ALL_ANSWERS", 2, c_TABLE_GET_ALL_ANSWERS);
    insert_cpred("c_TABLE_GET_ONE_ANSWER", 1, c_TABLE_GET_ONE_ANSWER);
    insert_cpred("c_OLD_GLOBAL_GET", 3, c_OLD_GLOBAL_GET);
    insert_cpred("c_OLD_GLOBAL_SET", 3, c_OLD_GLOBAL_SET);
    insert_cpred("old_global_get", 3, c_OLD_GLOBAL_GET);
    insert_cpred("old_global_set", 3, c_OLD_GLOBAL_SET);
    insert_cpred("unnumber_vars", 2, c_UNNUMBER_VARS);
    insert_cpred("c_SAVE_AR", 1, c_SAVE_AR);
    /* insert_cpred("c_WRITE_AR",1,c_WRITE_AR); */
    insert_cpred("subgoal_table_size", 1, c_SUBGOAL_TABLE_SIZE);
    insert_cpred("c_write_term", 1, c_write_term);
    insert_cpred("c_get_line_nos", 2, c_get_line_nos);
    insert_cpred("c_set_line_no", 1, c_set_line_no);

    insert_cpred("c_report_syntax_error", 1, c_report_syntax_error);
    insert_cpred("c_fmt_nl", 0, c_fmt_nl);
    insert_cpred("c_fmt_write_quick", 1, c_fmt_write_quick);
    insert_cpred("c_fmt_writeq_quick", 1, c_fmt_writeq_quick);
    insert_cpred("c_put_update_pos", 1, c_put_update_pos);
    insert_cpred("c_fmt_writename", 1, c_fmt_writename);
    insert_cpred("c_fmt_writeqname", 1, c_fmt_writeqname);

    insert_cpred("c_init_chars_pool", 0, c_init_chars_pool);
    insert_cpred("c_update_term_start_line_no", 0, c_update_term_start_line_no);
    /*
      insert_cpred("$start_gc",0,b_ENABLE_GC);
      insert_cpred("$stop_gc",0,b_DISABLE_GC);
    */
    insert_cpred("c_SET_ARG_PAREA", 3, c_SET_ARG_PAREA);
    insert_cpred("table_statistics", 0, table_statistics);
    insert_cpred("c_ASPN_i", 1, c_ASPN_i);

    /*  insert_cpred("printStackTrace",0,printStackTrace); */
#ifdef GC
    insert_cpred("number_of_gcs", 1, number_of_gcs);
#endif
#ifdef TABLE_STAT
    insert_cpred("init_num_answer_accesses", 0, init_num_answer_accesses);
    insert_cpred("get_num_answer_accesses", 1, get_num_answer_accesses);
#endif

    insert_cpred("c_HTABLE_HCODE", 2, c_HTABLE_HCODE);
    /* insert_cpred("c_TUPLES_TO_TRIE",3,c_TUPLES_TO_TRIE); */

    insert_cpred("c_table_cardinality_limit", 3, c_table_cardinality_limit);
    insert_cpred("c_set_all_table_cardinality_limit", 1, c_set_all_table_cardinality_limit);
    insert_cpred("c_init_global_each_session", 0, c_init_global_each_session);
    insert_cpred("c_GET_GC_TIME", 1, c_GET_GC_TIME);

    insert_cpred("fd_degree", 2, c_fd_degree);

    // insert_cpred("test",2,test);

    insert_cpred("c_set_bp_exception", 1, c_set_bp_exception);

    insert_cpred("c_SAME_ADDR", 2, c_SAME_ADDR);

    insert_cpred("c_sat_embedded", 0, c_sat_embedded);

    insert_cpred("bp_exit", 1, c_bp_exit);

    insert_cpred("c_LOAD_BYTE_CODE_FROM_BPLISTS", 6, c_LOAD_BYTE_CODE_FROM_BPLISTS);

    insert_cpred("c_GET_MODULE_SIGNATURE_cf", 2, c_GET_MODULE_SIGNATURE_cf);

    insert_cpred("c_PICAT_GETENV_cf", 2, c_PICAT_GETENV_cf);

    insert_cpred("c_PICAT_GET_CWD_f", 1, c_PICAT_GET_CWD_f);

    insert_cpred("c_PICAT_FORMAT_TO_STRING_ccff", 4, c_PICAT_FORMAT_TO_STRING_ccff);

    insert_cpred("c_CP_FILE_cc", 2, c_CP_FILE_cc);

    insert_cpred("c_MAXINT_f", 1, c_MAXINT_f);
    insert_cpred("c_MININT_f", 1, c_MININT_f);
    insert_cpred("c_IS_SMALL_INT_c", 1, c_IS_SMALL_INT_c);

    insert_cpred("c_MUL_MOD_cccf", 4, c_MUL_MOD_cccf);

    insert_cpred("c_set_eolcom_flag", 1, c_set_eolcom_flag);

    insert_cpred("c_bigint_sign_size", 3, c_bigint_sign_size);

    /* insert_cpred("show_susp_frames",0,show_susp_frames); */

    Cboot_numbervars();
    Cboot_mic();
    Cboot_debug();
    Cboot_assert();
    Cboot_delay();
    Cboot_domain();
#ifdef JAVA
    Cboot_plc();
#endif

#ifdef CPLEX
    Cboot_cplex();
#endif

    /*
      #ifdef GLPK
      Cboot_glpk();
      #endif
    */
    insert_cpred("c_sat_start_dump", 1, c_sat_start_dump);
    insert_cpred("c_sat_stop_dump", 0, c_sat_stop_dump);
    insert_cpred("c_sat_start_count", 1, c_sat_start_count);
    insert_cpred("c_sat_stop_count", 1, c_sat_stop_count);
    insert_cpred("c_sat_propagate_dom_bits", 2, c_sat_propagate_dom_bits);

    insert_cpred("c_REDUCE_DOMAINS_IC_EQ", 2, c_REDUCE_DOMAINS_IC_EQ);
    insert_cpred("c_REDUCE_DOMAINS_IC_GE", 2, c_REDUCE_DOMAINS_IC_GE);
    //  insert_cpred("c_REDUCE_DOMAIN_AC_ADD",3,c_REDUCE_DOMAIN_AC_ADD);
    insert_cpred("c_TA_TOP_f", 1, c_TA_TOP_f);
    Cboot_sat();
#ifdef SAT
    insert_cpred("c_call_espresso", 5, c_call_espresso);
    insert_cpred("c_call_espresso_pb", 6, c_call_espresso_pb);
#endif

#ifdef PRISM
    bp4p_register_preds();
#endif

#ifdef FANN
    fann_cpreds();
#endif
    //  Cboot_TP();
}

/* by S. Branch */
/*
  extern void Cboot_lcm(void);
  extern void Cboot_libdata(void);
  extern void Cboot_libklr(void);
  extern void Cboot_libprofare(void);
  extern void Cboot_libtaxtextdata(void);
  extern int c_current_host(void);
  extern int c_process_id(void);
  extern int c_socket_server_open(void);
  extern int c_socket_server_accept(void);
  extern int c_socket_server_close(void);
  extern int c_format_to_codes(void);
  extern int c_now(void);
  extern int c_term_hash(void);
  extern int c_random(void);
  extern int c_simple(void);

  void Cboot_TP(void)
  {
  insert_cpred("current_host", 1, c_current_host);                                                                      //branch
  insert_cpred("process_id", 1, c_process_id);                                                                          //branch
  insert_cpred("socket_server_open", 2, c_socket_server_open);                                                          //branch
  insert_cpred("socket_server_accept", 4, c_socket_server_accept);                                                      //branch
  insert_cpred("socket_server_close", 1, c_socket_server_close);                                                        //branch
  insert_cpred("format_to_codes", 3, c_format_to_codes);                                                                        //branch
  insert_cpred("now", 1, c_now);                                                                                        //branch
  insert_cpred("term_hash", 2, c_term_hash);                                                                            //branch
  insert_cpred("random", 1, c_random);                                                                                  //branch
  insert_cpred("simple", 1, c_simple);                                                                                  //branch
  Cboot_lcm();                                                                                                          //branch
  Cboot_libdata();                                                                                                      //branch
  Cboot_libklr();                                                                                                       //branch
  Cboot_libprofare();                                                                                                   //branch
  Cboot_libtaxtextdata();                                                                                               //branch
  }

*/
