/********************************************************************
 *   File   : kapi.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024
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

/* Prolog to C */
/*
  PvalueOfInt(BPLONG term){
  DEREF(term);
  return INTVAL(term);
  }
*/
double PvalueOfReal(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return floatval(term);
}


BPLONG PvalueOfAddr(BPLONG term)
{
    BPULONG w1, w2;
    BPLONG_PTR top;

    DEREF(term);
    UNTAG_ADDR(term);
    w1 = INTVAL(*((BPLONG_PTR)term+1));
    w2 = INTVAL(*((BPLONG_PTR)term+2));
    return (w2 << HALF_WORD_LONG | w1);
}

char* PnumberToStr(BPLONG term)
{
    printf("PnumberToStr used\n");
    return " ";
}

/* Manipulate compound terms */

UW32 ParityOfCompound(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return GET_ARITY(GET_SYM_REC(term));
}

BPLONG PargOfCompound(BPLONG term, BPLONG n)
{
    return picat_get_arg(n+1, term);
}

/* Return functors as strings*/

char *PascOfFunc(BPLONG term)
{
    return bp_get_name(term);
}

char *PascOfAtom(BPLONG term)
{
    return bp_get_name(term);
}

char *PnameToAsc(BPLONG term)
{
    printf("PnameToAsc used \n");
    return " ";
}

/* UNIFY */
int Punify(BPLONG term1, BPLONG term2)
{
    return unify(term1, term2);
}

int PuInt(BPLONG term, BPLONG i)
{
    BPLONG op;

    op = MAKEINT(i);
    return unify(term, op);
}

int PuAtom(BPLONG term, char *str)
{
    BPLONG op = picat_build_atom(str);
    return unify(term, op);
}

void PuStr(BPLONG term, char *str)
{
    string2codes(str, term);
}

int PuAddr(BPLONG term, void *a)
{
    register BPLONG temp;

    temp = ADDTAG(heap_top, STR);
    NEW_HEAP_NODE((BPLONG)objectRef);  /* '$addr'(int1,int2) */
    NEW_HEAP_NODE(MAKEINT(((BPULONG)a << HALF_WORD_LONG) >> HALF_WORD_LONG));
    NEW_HEAP_NODE(MAKEINT(((BPULONG)a >> HALF_WORD_LONG)));
    return unify(term, temp);
}

int PuReal(BPLONG term, double d)
{
    return unify(term, encodefloat1(d));
}

/* Create terms */
BPLONG PnewVar() {
    return picat_build_var();
}

BPLONG Patom(char *str)
{
    return picat_build_atom(str);
}

BPLONG create_FUNCTOR(char *name, BPLONG arity)
{
    return bp_build_structure(name, arity);
}

BPLONG Pfunctorn(BPLONG name, BPLONG arity)
{
    char *str;

    str = bp_get_name(name);
    return bp_build_structure(str, arity);
}

/* Manipulate lists */
BPLONG get_CAR(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term));
}

BPLONG PlistCar(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term));
}

BPLONG get_CDR(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term)+1);
}

BPLONG PlistCdr(BPLONG term)
{
    BPLONG_PTR top;

    DEREF(term);
    return *((BPLONG_PTR)UNTAGGED_ADDR(term)+1);
}

BPLONG PlistLength(BPLONG term)
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

BPLONG Plistn(BPLONG length)
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

int PcallF(BPLONG term)
{
    return bp_call_term(term);
}

int PcallX(BPLONG command)
{
    return bp_call_term(command);
}

int PexecP(char *cmd)
{
    return bp_call_string(cmd);
}

int Pexecute(char *cmd)
{
    return bp_call_string(cmd);
}

int PinitP(int argc, char **argv)
{
    return initialize_bprolog(argc, argv);
}

void jni_interface() {
}

void plc_sup() {
}















