/********************************************************************
 *   File   : sapi.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2023
 *   Purpose: External language interface

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <stdlib.h>
#include "bprolog.h"
#include "sapi.h"
#include <stdarg.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#define chdir _chdir
#else
#include <unistd.h>
#endif

/* extern char *malloc();*/

BPLONG SP_list_len(SP_term_ref t);
int SP_get_list_n_chars(SP_term_ref t, SP_term_ref tail, long n, BPLONG *w, char *s);

char *SP_error_message(BPLONG errno) {
    quit("Not implemented \n");
}

SP_term_ref SP_new_term_ref() {
    SP_term_ref op = (SP_term_ref)heap_top;
    NEW_HEAP_NODE(nil_sym);
    return op;
}

BPULONG SP_atom_from_string(char *s) {
    return (BPLONG)insert_sym(s, strlen(s), 0);
}

char *SP_string_from_atom(BPULONG a) {
    return GET_NAME((SYM_REC_PTR)a);
}

UW16 SP_atom_length(BPULONG a) {
    return GET_LENGTH((SYM_REC_PTR)a);
}

void SP_put_term(SP_term_ref to, SP_term_ref from) {
    FOLLOW(to) = from;
}

/*
  Assigns to t a new Prolog variable.                                       
*/
int SP_put_variable(SP_term_ref t) {
    SP_term_ref op;
    NEW_VAR(op);
    FOLLOW(t) = op;
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog integer from a C long integer. 
*/
int SP_put_integer(SP_term_ref t, long l) {
    FOLLOW(t) = MAKEINT(l);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog oat from a C double. 
*/
int SP_put_float(SP_term_ref t, double d) {
    FOLLOW(t) = encodefloat1(d);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog atom from a, which must be the canonical representation
  of a Prolog atom. 
*/
int SP_put_atom(SP_term_ref t, unsigned long a) {
    FOLLOW(t) = ADDTAG(a, ATM);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog atom from a encoded C string 
*/
int SP_put_string(SP_term_ref t, char *name) {
    FOLLOW(t) = ADDTAG(insert_sym(name, strlen(name), 0), ATM);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog integer from a C pointer. 
*/
int SP_put_address(SP_term_ref t, void *pointer) {
    FOLLOW(t) = ADDTAG((BPLONG)pointer, INT_TAG);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog list of the character codes represented by the encoded
  string s, prepended to the value of tail.
*/
int SP_put_list_chars(SP_term_ref t, SP_term_ref tail, char *s) {
    BPLONG i, len = strlen(s);

    for (i = 0; i < len; i++) {
        FOLLOW(t) = ADDTAG(heap_top, LST);
        FOLLOW(heap_top++) = MAKEINT(*s++);
        t = (SP_term_ref)heap_top++;
    }
    FOLLOW(t) = tail;
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog list of the character codes represented by the frst n bytes
  in encoded string s, prepended in front of the value of tail.
*/
int SP_put_list_n_chars(SP_term_ref t, SP_term_ref tail, long n, char *s) {
    BPLONG i, len;
    len = strlen(s);
    len = (len > n ? n : len);
    for (i = 0; i < len; i++) {
        FOLLOW(t) = ADDTAG(heap_top, LST);
        FOLLOW(heap_top++) = MAKEINT(*s++);
        t = (SP_term_ref)heap_top++;
    }
    FOLLOW(t) = tail;
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog number by parsing the string in s.
*/
int SP_put_number_chars(SP_term_ref t, char *s) {
    BPLONG vars;
    SP_term_ref term;

    NEW_VAR(term);
    vars = bp_build_var();
    bp_string_2_term(s, term, vars);
    FOLLOW(t) = term;
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog compound term with all the arguments unbound variables.
  If arity is 0, assigns the Prolog atom whose canonical representation is name
  to t. This is similar to calling functor/3 with the frst argument unbound and
  the second and third arguments bound to an atom and an integer, respectively.
*/
int SP_put_functor(SP_term_ref t, BPLONG name, BPLONG arity) {
    SP_term_ref term;
    NEW_VAR(term);
    cfunctor1(term, ADDTAG(name, ATM), MAKEINT(arity));
    FOLLOW(t) = term;
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog list whose head and tail are both unbound variables.
*/
int SP_put_list(SP_term_ref t) {
    FOLLOW(t) = ADDTAG(heap_top, LST);
    NEW_HEAP_FREE;
    NEW_HEAP_FREE;
    return SP_SUCCESS;
}

/*
  This is similar to calling =../2 with the first argument unbound and the
  second argument bound.
*/
int SP_cons_functor(SP_term_ref t, BPLONG name, BPLONG arity, ...) {
    va_list ap;
    SP_term_ref arg;
    BPLONG i;
    SYM_REC_PTR sym_ptr = insert_sym(GET_NAME((SYM_REC_PTR)name), GET_LENGTH((SYM_REC_PTR)name), arity);
    BPLONG_PTR top;

    if (arity == 0) {
        FOLLOW(t) = ADDTAG((BPLONG)sym_ptr, ATM);
        return SP_SUCCESS;
    }
    FOLLOW(t) = ADDTAG(heap_top, STR);
    FOLLOW(heap_top++) = (BPLONG)sym_ptr;

    va_start(ap, arity);
    for (i = 1; i <= arity; i++) {
        arg = va_arg(ap, SP_term_ref);
        BUILD_VALUE(arg, sp_cons_functor);
    }
    va_end(ap);
    return SP_SUCCESS;
}

/*
  Assigns to t a Prolog list whose head and tail are the values of head and tail.
*/
int SP_cons_list(SP_term_ref t, SP_term_ref head, SP_term_ref tail) {
    BPLONG_PTR top;
    FOLLOW(t) = ADDTAG(heap_top, LST);
    BUILD_VALUE(head, bp_cons_list1);
    BUILD_VALUE(tail, bp_cons_list2);
    return SP_SUCCESS;
}

/*******************************************************************/
/*
  Assigns to *l the C long corresponding to a Prolog number.
*/
int SP_get_integer(SP_term_ref t, long *l) {
    BPLONG_PTR top;
    DEREF(t);
    if (!ISINT(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    *l = INTVAL(t);
    return SP_SUCCESS;
}

/*
  Assigns to *d the C double corresponding to a Prolog number.
*/
int SP_get_float(SP_term_ref t, double *d) {
    double temp_d, floatval();
    BPLONG_PTR top;
    DEREF(t);
    if (ISINT(t)) {
        temp_d = (double)INTVAL(t);
    } else if (ISFLOAT(t)) {
        temp_d = floatval(t);
    } else {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    *d = temp_d;
    return SP_SUCCESS;
}

/*
  Assigns to *a the canonical representation of a Prolog atom.
*/
int SP_get_atom(SP_term_ref t, BPULONG *a) {
    BPLONG_PTR top;
    DEREF(t);
    if (!ISATOM(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    *a = UNTAGGED_ADDR(t);
    return SP_SUCCESS;
}

/* Assigns to *name a pointer to the encoded string representing the name of a
   Prolog atom. 
*/
int SP_get_string(SP_term_ref t, char **name) {
    BPLONG_PTR top;
    DEREF(t);
    if (!ISATOM(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    *name = GET_NAME((SYM_REC_PTR)UNTAGGED_ADDR(t));
    printf("%s\n", *name);
    return SP_SUCCESS;
}

/*
  Assigns to *pointer a C pointer from a Prolog term. The term should be an
  integer whose value should be a valid second argument to SP_put_address()
*/
int SP_get_address(SP_term_ref t, void **pointer) {
    BPLONG_PTR top;
    DEREF(t);
    if (!ISINT(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    *pointer = (void *)INTVAL(t);
    return SP_SUCCESS;
}

/*
  Assigns to *s a zero-terminated array containing an encoded string which cor-
  responds to the given Prolog list of character codes. The array is subject to
  reuse by other support functions, so if the value is going to be used on a more
  than temporary basis, it must be moved elsewhere.
*/
int SP_get_list_chars(SP_term_ref t, char **s) {
    BPLONG_PTR top;
    BPLONG n;
    BPLONG w;
    char *buf;
    SP_term_ref tail = SP_new_term_ref();
    DEREF(t);
    if (!ISLIST(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    n = SP_list_len(t);
    buf = (char *)malloc(n+1);
    if (buf == NULL) myquit(OUT_OF_MEMORY, "sp");
    *s = buf;
    return SP_get_list_n_chars(t, tail, n, &w, buf);
}

/*
  Copies into s the encoded string representing the character codes in the initial
  elements of list t, so that at most n bytes are used. The number of bytes
  actually written is assigned to *w. tail is set to the remainder of the list. The
  array s must have room for at least n bytes.
*/
int SP_get_list_n_chars(SP_term_ref t, SP_term_ref tail, long n, BPLONG *w, char *s) {
    BPLONG_PTR top;
    BPLONG i, len;
    BPLONG code;
    DEREF(t);
    if (!ISLIST(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    len = SP_list_len(t);
    len = (len > n ? n : len);
    for (i = 0; i < len; i++) {
        UNTAG_ADDR(t);
        code = FOLLOW(t); DEREF(code);
        if (!ISINT(code)) {
            bp_exception = illegal_arguments;
            return SP_ERROR;
        }
        s[i] = INTVAL(code);
        t = FOLLOW((BPLONG_PTR)t+1);
        DEREF(t);
    }
    s[len] = '\0';
    *w = len;
    FOLLOW(tail) = t;
    return SP_SUCCESS;
}

BPLONG SP_list_len(SP_term_ref t) {
    BPLONG_PTR top;
    BPLONG len = 0;
    DEREF(t);
    while (ISLIST(t)) {
        t = GET_CDR(t);
        DEREF(t);
        len++;
    }
    return len;
}

/*
  Assigns to *s a zero-terminated array of characters corresponding to the printed
  representation of a Prolog number. The array is subject to reuse by other
  support functions, so if the value is going to be used on a more than temporary
  basis, it must be moved elsewhere.
*/
int SP_get_number_chars(SP_term_ref t, char **s) {
    BPLONG_PTR top;
    char buf[80];
    double floatval();
    char *ptr;
    DEREF(t);
    if (ISINT(t)) {
        sprintf(buf, BPLONG_FMT_STR, INTVAL(t));
    } else if (ISFLOAT(t)) {
        sprintf(buf, "%lf", floatval(t));
    } else {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    ptr = (char *)malloc(strlen(buf)+1);
    if (ptr == NULL) myquit(OUT_OF_MEMORY, "sp");
    strcpy(ptr, buf);
    *s = ptr;
    return SP_SUCCESS;
}

/*
  Assigns to *name and *arity the canonical representation and arity of the
  principal functor of a Prolog compound term. If the value of t is an atom, then
  that atom is assigned to *name and 0 is assigned to *arity. This is similar to
  calling functor/3 with the first argument bound to a compound term or an
  atom and the second and third arguments unbound.
*/
int SP_get_functor(SP_term_ref t, unsigned long *name, BPLONG *arity) {
    BPLONG_PTR top;
    BPLONG func;
    BPLONG n;
    DEREF(t);

    if (ISREF(t) || ISNUM(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    NEW_VAR(func); NEW_VAR(n);
    cfunctor1(t, func, n);
    DEREF(func); DEREF(n);
    *name = UNTAGGED_ADDR(func);
    *arity = INTVAL(n);
    return SP_SUCCESS;
}

/*
  Assigns to head and tail the head and tail of a Prolog list.
*/
int SP_get_list(SP_term_ref t, SP_term_ref head, SP_term_ref tail) {
    BPLONG_PTR top;
    DEREF(t);
    if (!ISLIST(t)) {
        bp_exception = illegal_arguments;
        return SP_ERROR;
    }
    UNTAG_ADDR(t);
    FOLLOW(head) = FOLLOW(t);
    FOLLOW(tail) = FOLLOW((BPLONG_PTR)t+1);
    return SP_SUCCESS;
}

/*
  Assigns to arg the i:th argument of a Prolog compound term. This is similar
  to calling arg/3 with the third argument unbound.
*/

void SP_get_arg(BPLONG i, SP_term_ref t, SP_term_ref arg) {
    BPLONG_PTR top;
    BPLONG temp;
    NEW_VAR(temp);
    carg1(MAKEINT(i), t, temp);
    DEREF(temp);
    FOLLOW(arg) = temp;
}

/*******************************************************************/
/*
  Depending on the type of the term t, one of SP_TYPE_VARIABLE, SP_TYPE_
  INTEGER, SP_TYPE_FLOAT, SP_TYPE_ATOM, or SP_TYPE_COMPOUND is returned.
*/
int SP_term_type(SP_term_ref t) {
    BPLONG_PTR top;

    SWITCH_OP(t, SP_term_type_1,
              {return SP_TYPE_VARIABLE;},
              {if (ISINT(t)) return SP_TYPE_INTEGER;
                  else return SP_TYPE_ATOM;
              },
              {return SP_TYPE_COMPOUND;},
              {if (IS_SUSP_VAR(t)) return SP_TYPE_VARIABLE;
                  else if (ISFLOAT(t)) return SP_TYPE_FLOAT;
                  else return SP_TYPE_COMPOUND;},
              {return SP_TYPE_VARIABLE;});
}

/*
  Returns nonzero if the term is a Prolog variable, zero otherwise.
*/
int SP_is_variable(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return (ISREF(t) || IS_SUSP_VAR(t));
}

/*
  Returns nonzero if the term is a Prolog integer, zero otherwise.
*/
int SP_is_integer(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISINT(t);
}

/*
  Returns nonzero if the term is a Prolog oat, zero otherwise.
*/
int SP_is_float(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISFLOAT(t);
}

/*
  Returns nonzero if the term is a Prolog atom, zero otherwise.
*/
int SP_is_atom(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISATOM(t);
}

/*
  Returns nonzero if the term is a Prolog compound term, zero otherwise.
*/
int SP_is_compound(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISCOMPOUND(t);
}

/*
  Returns nonzero if the term is a Prolog list, zero otherwise.
*/
int SP_is_list(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISLIST(t);
}

/*
  Returns nonzero if the term is an atomic Prolog term, zero otherwise.
*/
int SP_is_atomic(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return (ISATOM(t) || ISNUM(t));
}

/*
  Returns nonzero if the term is a Prolog number, zero otherwise.
*/
int SP_is_number(SP_term_ref t) {
    BPLONG_PTR top;
    DEREF(t);
    return ISNUM(t);
}

int SP_unify(SP_term_ref x, SP_term_ref y) {
    return unify(x, y);
}

/*
  Returns -1 if x @< y, 0 if x == y and 1 if x @> y
*/
int SP_compare(SP_term_ref x, SP_term_ref y) {
    return bp_compare(x, y);
}

/*******************************************************************/
void * SP_malloc(BPULONG size) {
    return malloc(size);
}

void * SP_calloc(BPULONG nmemb, BPULONG size) {
    void * calloc();
    return calloc(nmemb, size);
}

void * SP_realloc(void *ptr, BPULONG size) {
    return malloc(size);
}

void SP_free(void *ptr) {
    free(ptr);
}

int SP_chdir(const char *path) {
    if(chdir(path)) return SP_ERROR;
    return SP_SUCCESS;
}

char *SP_getcwd(char *buf, BPULONG size) {
    quit("SP_getcwd not implemented\n");
}

/*******************************************************************/
SP_pred_ref SP_predicate(char *name_string, long arity, char *module_string) {
    SYM_REC_PTR sym_ptr;
    sym_ptr = insert_sym(name_string, strlen(name_string), arity);
    return sym_ptr;
}

SP_pred_ref SP_pred(BPULONG name_atom, long arity, BPULONG module_atom) {
    SYM_REC_PTR sym_ptr = (SYM_REC_PTR)name_atom;
    return SP_predicate(GET_NAME(sym_ptr), arity, NULL);
}

int SP_query(SP_pred_ref predicate, ...) {
    SYM_REC_PTR sym_ptr;
    va_list ap;
    SP_term_ref arg;
    BPLONG query;
    BPLONG i, arity;
    BPLONG_PTR top;
    char *name;

    arity = GET_ARITY(predicate);
    printf("arity=" BPLONG_FMT_STR "\n", arity);

    if (arity == 0) {
        query = ADDTAG((BPLONG)predicate, ATM);
    } else {
        name = GET_NAME(predicate);
        *heap_top = (BPLONG)insert_sym(name, strlen(name), arity);
        query = ADDTAG(heap_top++, STR);
        va_start(ap, predicate);
        for (i = 0; i < arity; i++) {
            arg = va_arg(ap, SP_term_ref);
            BUILD_VALUE(arg, sp_query);
        }
        va_end(ap);
    }
    write_term(query);
    return bp_call_term(query);
}

void SP_query_cut_fail(SP_pred_ref predicate, ...) {
    quit("SP_query_cut_fail not implemented\n");
}

