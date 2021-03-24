/********************************************************************
 *   File   : kapi.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021
 *   Purpose: External language interface

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "bprolog.h"
#include "kapi.h"
#include <stdlib.h>
#include "frame.h"

#ifdef M64BITS
#define HALF_WORD_LONG 32
#else
#define HALF_WORD_LONG 16
#endif

SYM_REC_PTR objectRef;
extern char *string_in;
char *bp_get_name();

/* Prolog to C */
/*
  PvalueOfInt(BPLONG term){
  DEREF(term);
  return INTVAL(term);
  }
*/
double PvalueOfReal(term)
    BPLONG term;
{
    BPLONG_PTR top;

    double floatval();
    DEREF(term);
    return floatval(term);
}


BPLONG PvalueOfAddr(term)
    BPLONG term;
{
    BPULONG w1, w2;
    BPLONG_PTR top;

    DEREF(term);
    UNTAG_ADDR(term);
    w1 = INTVAL(*((BPLONG_PTR)term+1));
    w2 = INTVAL(*((BPLONG_PTR)term+2));
    return (w2 << HALF_WORD_LONG | w1);
}

char* PnumberToStr(term)
    BPLONG term;
{
    printf("PnumberToStr used\n");
    return " ";
}

/* Manipulate compound terms */

UW32 ParityOfCompound(term)
    BPLONG term;
{
    BPLONG_PTR top;

    DEREF(term);
    return GET_ARITY(GET_SYM_REC(term));
}

BPLONG PargOfCompound(term, n)
    BPLONG term;
    BPLONG n;
{
    return picat_get_arg(n+1, term);
}

/* Return functors as strings*/

char *PascOfFunc(term)
    BPLONG term;
{
    return bp_get_name(term);
}

char *PascOfAtom(term)
    BPLONG term;
{
    return bp_get_name(term);
}

char *PnameToAsc(term)
    BPLONG term;
{
    printf("PnameToAsc used \n");
    return " ";
}

/* UNIFY */
int Punify(term1, term2)
    BPLONG term1;
    BPLONG term2;
{
    return unify(term1, term2);
}

int PuInt(term, i)
    BPLONG term;
    BPLONG i;
{
    BPLONG op;

    op = MAKEINT(i);
    return unify(term, op);
}

int PuAtom(term, str)
    BPLONG term;
    char *str;
{
    BPLONG op = picat_build_atom(str);
    return unify(term, op);
}

void PuStr(term, str)
    BPLONG term;
    char *str;
{
    string2codes(str, term);
}

int PuAddr(term, a)
    BPLONG term;
    void *a;
{
    register BPLONG temp;

    temp = ADDTAG(heap_top, STR);
    NEW_HEAP_NODE((BPLONG)objectRef);  /* '$addr'(int1,int2) */
    NEW_HEAP_NODE(MAKEINT(((BPULONG)a << HALF_WORD_LONG) >> HALF_WORD_LONG));
    NEW_HEAP_NODE(MAKEINT(((BPULONG)a >> HALF_WORD_LONG)));
    return unify(term, temp);
}

int PuReal(term, d)
    BPLONG term;
    double d;
{
    return unify(term, encodefloat1(d));
}

/* Create terms */
BPLONG PnewVar() {
    return picat_build_var();
}

BPLONG Patom(str)
    char *str;
{
    return picat_build_atom(str);
}

BPLONG create_FUNCTOR(name, arity)
    char * name;
    BPLONG arity;
{
    return bp_build_structure(name, arity);
}

BPLONG Pfunctorn(name, arity)
    BPLONG name;
    BPLONG arity;
{
    char *str;

    str = bp_get_name(name);
    return bp_build_structure(str, arity);
}

/* Manipulate lists */
BPLONG get_CAR(term)
    BPLONG term;
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term));
}

BPLONG PlistCar(term)
    BPLONG term;
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term));
}

BPLONG get_CDR(term)
    BPLONG term;
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term)+1);
}

BPLONG PlistCdr(term)
    BPLONG term;
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term)+1);
}

BPLONG PlistLength(term)
    BPLONG term;
{
    register BPLONG len = 0;
    BPLONG_PTR top;

    DEREF(term);
    while (!PisEndList(term)) {
        len++;
        term = PlistCdr(term);
        DEREF(term);
    }
    return len;
}

BPLONG Plistn(length)
    BPLONG length;
{
    BPLONG lst, tail, tmp;

    if (length <= 0) return picat_build_nil();
    else {
        length--;
        lst = bp_build_list();
        tail = bp_get_cdr(lst);
        while (length > 0) {
            tmp = bp_build_list();
            FOLLOW(tail) = tmp;
            tail = bp_get_cdr(tmp);
            length--;
        }
        FOLLOW(tail) = bp_build_nil();
    }
    return lst;
}

int PcallF(term)
    BPLONG term;
{
    return bp_call_term(term);
}

int PcallX(command)
    BPLONG command;
{
    return bp_call_term(command);
}

int PexecP(cmd)
    char *cmd;
{
    return bp_call_string(cmd);
}

int Pexecute(cmd)
    char *cmd;
{
    return bp_call_string(cmd);
}

int PinitP(argc, argv)
    int argc;
    char **argv;
{
    return initialize_bprolog(argc, argv);
}

void jni_interface() {
}

void plc_sup() {
}















