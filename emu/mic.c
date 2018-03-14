/********************************************************************
 *   File   : mic.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018
 *   Purpose: miscellaneous functions
 *            Includes MurmurHash by Austin Appleby

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
/* mic.c */
#ifdef WIN32
#include <time.h>
#include <windows.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#ifdef DARWIN
#include <sys/time.h>
#else
#include <sys/times.h>
#endif
#include <sys/resource.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "inst.h"

#include "basic.h"
#include "bapi.h"
#include "term.h"
#include "frame.h"
#include "event.h"

#if defined(WIN32) && defined(M64BITS)
#define sys_getpid() _getpid()
#else
#define sys_getpid() getpid()
#endif

#define BP_COMPARE_VALS(val1,val2)    (((BPLONG)val1 == (BPLONG)val2) ? 0 : (((BPLONG)val1 > (BPLONG)val2) ? 1 : -1))

#define BP_COMPARE_UNSIGNED_VALS(val1,val2)    (((BPULONG)val1 == (BPULONG)val2) ? 0 : (((BPULONG)val1 > (BPULONG)val2) ? 1 : -1))

#define GLOBALIZE_VAR(term){                                    \
        DEREF(term);                                            \
        if (ISREF(term) && (BPULONG)term>(BPULONG)heap_top){    \
            FOLLOW(term) = (BPLONG)heap_top;                    \
            NEW_HEAP_FREE;                                      \
        } else if (IS_SUSP_VAR(term)) {                         \
            term = (BPLONG)UNTAGGED_TOPON_ADDR(term);           \
        }                                                       \
    }

#define COMPARE_FLOAT_FLOAT(f1,f2){             \
        if (f1<f2) return -1L;                  \
        else if (f1 == f2) return 0;            \
        else return 1;                          \
    }

extern BPLONG no_gcs;
extern BPLONG gc_time;
extern BPLONG gc_threshold;
extern char *string_in;

BPLONG greater_than_sym,less_than_sym,equal_sym;

int c_ref_equal(){
    BPLONG op1 = ARG(1,2);
    BPLONG op2 = ARG(2,2);
    BPLONG_PTR top;
  
    DEREF(op1);DEREF(op2);
    return  (op1 == op2);
}

int c_NEXT_PRIME(){
    BPLONG n;
    BPLONG prime;
    n = ARG(1,2); DEREF(n);
    prime = ARG(2,2);
    return unify(prime,MAKEINT(bp_prime(INTVAL(n))));
}

int c_set_debugging_susp(){
    debugging_susp = 1;
    return BP_TRUE;
}

int c_unset_debugging_susp(){
    debugging_susp = 0;
    return BP_TRUE;
}

int c_UNDERSCORE_NAME(){
    BPLONG atom;
    SYM_REC_PTR sym_ptr;
    char *name;
    BPLONG_PTR top;

    atom = ARG(1,1);
    DEREF(atom);
    sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(atom);
    name = GET_NAME(sym_ptr);
    return *name == '_';
}

BPLONG time_t_2_prolog(t)
    time_t *t;
{

    BPLONG time_in_prolog;
    struct tm *tp;

    tp = gmtime(t);
    time_in_prolog = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)BP_NEW_SYM("time",6);
    FOLLOW(heap_top++) = MAKEINT(tp->tm_year+1900);
    FOLLOW(heap_top++) = MAKEINT(tp->tm_mon);
    FOLLOW(heap_top++) = MAKEINT(tp->tm_mday);  
    FOLLOW(heap_top++) = MAKEINT(tp->tm_hour);  
    FOLLOW(heap_top++) = MAKEINT(tp->tm_min);  
    FOLLOW(heap_top++) = MAKEINT(tp->tm_sec);
    return time_in_prolog;
}

int c_confirm_copy_right(){
    if (confirm_copy_right == 1) return BP_TRUE; else return BP_FALSE;
}

#ifdef WIN32
BPLONG cputime()
{
    return (BPLONG)(1000*((float)clock() / (float)CLOCKS_PER_SEC));
}
#else
BPLONG cputime()
{
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    return (BPLONG)(rusage.ru_utime.tv_sec*1000 + rusage.ru_utime.tv_usec/1000);
}
#endif
/*
  BPLONG cputime()
  { struct tms t;
  (void)times(&t);
  return (1000*t.tms_utime/CLK_TCK); 
  }
*/

int c_OS_TYPE_f(){
    SYM_REC_PTR sym_ptr;
    BPLONG op;
  
    op = unix_atom;
#ifdef WIN32
    op = windows_atom;
#endif
#ifdef CYGWIN
    op = cygwin_atom;
#endif
  
    return unify(ARG(1,1),op);
}

int c_WDAY_f(){
    time_t t,*tp;
    struct tm *green_t_ptr;
    tp = &t;
    time(tp);
    green_t_ptr = localtime(tp);
    unify(ARG(1,1),MAKEINT(green_t_ptr->tm_wday));
    return BP_TRUE;
}

int c_TIME_ffffff(){
    BPLONG y,mo,d,h,mi,s;
    time_t t,*tp;
    struct tm *green_t_ptr;
    tp = &t;
    y = ARG(1,6);
    mo = ARG(2,6);
    d = ARG(3,6);
    h = ARG(4,6);
    mi = ARG(5,6);
    s = ARG(6,6);
  
    time(tp);
    green_t_ptr = localtime(tp);
    unify(y,MAKEINT(green_t_ptr->tm_year));
    unify(mo,MAKEINT(green_t_ptr->tm_mon));
    unify(d,MAKEINT(green_t_ptr->tm_mday));  
    unify(h,MAKEINT(green_t_ptr->tm_hour));  
    unify(mi,MAKEINT(green_t_ptr->tm_min));  
    unify(s,MAKEINT(green_t_ptr->tm_sec));
    return BP_TRUE;
}

int c_GETENV_cf(){
    BPLONG var,value;
    char *name, *val;
    BPLONG_PTR top;

    var = ARG(1,2);

    DEREF(var);
    value = ARG(2,2);
    if (!ISATOM(var)){
        exception = illegal_arguments; return BP_ERROR;
    }
    name = GET_NAME((SYM_REC_PTR)UNTAGGED_ADDR(var));
    val = getenv(name);
    if (val == NULL){
        return BP_FALSE;
    }
    unify(value,ADDTAG((BPLONG)insert_sym(val,strlen(val),0),ATM));
    return BP_TRUE;
}
  
int b_PICAT_COMPARE_fcc(BPLONG res, BPLONG term1, BPLONG term2){
    int i;

    i = bp_compare(term1, term2);
    ASSIGN_f_atom(res,MAKEINT(i));
    return BP_TRUE;
}

int b_COMPARE_fcc(res,t1,t2)
    BPLONG res,t1,t2;
{
    /* t1, t2: two terms to be compared; 
       res = '<' if t1<t2;
       res = '>' if t1>t2;
       res = '=' if t1=t2;
    */
    int i;
  
    i = bp_compare(t1, t2);
    if (i == 0) {
        ASSIGN_f_atom(res,equal_sym);
    } else if (i>0) {
        ASSIGN_f_atom(res,greater_than_sym);
    } else {
        ASSIGN_f_atom(res,less_than_sym);
    }
    return BP_TRUE;
}

int compare(val1, val2)
    BPLONG val1, val2;
{
    return bp_compare(val1,val2);
}

/* val1>val2 return positive; val1=val2 return 0; val1<val2 return negative */
int bp_compare(BPLONG val1, BPLONG val2)
{
    BPLONG_PTR        top;
    BPLONG            a, b;
    int               c;
    SYM_REC_PTR       sym_ptr1, sym_ptr2;

l_start:
    DEREF(val2);
    SWITCH_OP(val1,lcompare1,
              {if (ISREF(val2)) return BP_COMPARE_UNSIGNED_VALS(val1,val2); else return -1L;},
            
              {SWITCH_OP(val2,lcompare2,
                         {return 1;}, 
                         {goto compare_atom_atom;}, 
                         {return -1L;}, 
                         {if (IS_FLOAT_PSC(val2)) goto compare_atom_float; else if (IS_BIGINT_PSC(val2)) goto compare_atom_bigint; else return -1L;},
                         {return 1;});},

              {if (!ISLIST(val2)) 
                      return 1;
                  else {
                      UNTAG_ADDR(val1);
                      UNTAG_ADDR(val2);
                      if (val1==val2) return 0;
                      c = bp_compare(FOLLOW(val1), FOLLOW(val2));
                      if (c){
                          return c;
                      } else {
                          val1 = FOLLOW((BPLONG_PTR)val1+1);
                          val2 = FOLLOW((BPLONG_PTR)val2+1);
                          goto l_start;
                      }}},
             
              {if (IS_FLOAT_PSC(val1)) 
                      return compare_float_unknown(val1,val2);
                  if (IS_BIGINT_PSC(val1)) goto compare_bigint_unknown;
                  SWITCH_OP(val2,lcompare3,
                            {return 1;},
                            {return 1;},
                            {return -1L;},
                            {if (IS_FLOAT_PSC(val2)) return 1; else goto compare_symbol;},
                            {return 1;});},

              {SWITCH_OP(val2,lcompare4,
                         {return BP_COMPARE_UNSIGNED_VALS(val1,val2);},
                         {return -1L;},
                         {return -1L;},
                         {return -1L;},
                         {return BP_COMPARE_UNSIGNED_VALS(val1,val2);});});

compare_atom_atom:
    if (ISINT(val1)){
        if (ISINT(val2)){
            val1 = INTVAL(val1); val2 = INTVAL(val2);
            return BP_COMPARE_VALS(val1,val2);
        } else
            return -1L; /* int atom */
    }  else {
        if (ISATOM(val2)) 
            goto compare_symbol;
        else
            return 1; /* atom int */
    }

compare_atom_bigint:
    if (ISINT(val1)) {
        if (bp_sign_bigint(val2) < 0)
            return 1;
        else
            return -1L;
    } else
        return 1; /* atom int */

compare_bigint_atom:
    if (ISINT(val2)) {
        if (bp_sign_bigint(val1) < 0)
            return -1L;
        else
            return 1;
    } else
        return -1L; /* int atom */

compare_atom_float:
    if (ISINT(val1)) {
        double f1 = (double)INTVAL(val1);
        double f2 = floatval(val2);
        COMPARE_FLOAT_FLOAT(f1,f2);
    } else {
        return 1;
    }

compare_bigint_unknown:
    SWITCH_OP(val2,lcompare_bigint_unknown,
              {return 1;}, 
              {goto compare_bigint_atom;}, 
              {return -1L;}, 
              {if (IS_FLOAT_PSC(val2)) return compare_bigint_float(val1,val2); else if (IS_BIGINT_PSC(val2)) return bp_compare_bigint_bigint(val1,val2); else return -1L;},
              {return 1;});
   
compare_symbol:
    if (val1 == val2) return 0;
    sym_ptr1 = GET_SYM_REC(val1);
    sym_ptr2 = GET_SYM_REC(val2);
    a = GET_ARITY(sym_ptr1);
    b = GET_ARITY(sym_ptr2);
    if (a != b)
        return BP_COMPARE_VALS(a,b);
    c = comalpha(sym_ptr1, sym_ptr2);
    if (c || a == 0)
        return c;
    UNTAG_ADDR(val1);
    UNTAG_ADDR(val2);
    for (b = 1; b <= a; b++) {
        c = bp_compare(FOLLOW(((BPLONG_PTR)val1)+b),
                       FOLLOW(((BPLONG_PTR)val2)+b));
        if (c)
            break;
    }
    return c;
}

int compare_bigint_float(BPLONG val1, BPLONG val2)
{
    double f1,f2;
    f1 = bp_bigint_to_double(val1);
    f2 = floatval(val2);
    COMPARE_FLOAT_FLOAT(f1,f2);
}

int compare_float_unknown(BPLONG val1, BPLONG val2)
{
    double f1,f2;

    f1 = floatval(val1);
    DEREF(val2);
    if (ISFLOAT(val2)){
        f2 = floatval(val2);
        COMPARE_FLOAT_FLOAT(f1,f2);
    } else if (ISREF(val2)) {
        return 1;
    } else if (ISINT(val2)){
        f2 = (double)INTVAL(val2);
        COMPARE_FLOAT_FLOAT(f1,f2);
    } else if (IS_BIGINT(val2)){
        f2 = bp_bigint_to_double(val2);
        COMPARE_FLOAT_FLOAT(f1,f2);
    } else 
        return -1L;
}

int comalpha(sym_ptr1, sym_ptr2)
    SYM_REC_PTR sym_ptr1, sym_ptr2;
{
    if (sym_ptr1 == sym_ptr2)
        return 0;
    return strcmp(GET_NAME(sym_ptr1),GET_NAME(sym_ptr2));
}

int c_INCREMENTARG(){
    register BPLONG     op1,op2;
    BPLONG_PTR top;

    op1 = ARG(1,2);
    op2 = ARG(2,2);

    DEREF(op1);
    DEREF(op2);
    if (!ISINT(op1)) {
        exception = c_type_error(et_INTEGER,op1); 
        return BP_ERROR;
    }
    op1 = INTVAL(op1);
    if (ISSTRUCT(op2) && op1>0 && op1 <= GET_STR_SYM_ARITY(op2)) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1;
        op2 = *top;
        PUSHTRAIL_H_ATOMIC(top,op2);
        if (!ISINT(op2)) {
            exception = c_type_error(et_INTEGER,op2); 
            return BP_ERROR;
        }
        *top = MAKEINT(INTVAL(op2)+1);
        return BP_TRUE;
    }
    else if (ISLIST(op2) && op1>0 && op1 <= 2) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1-1;
        op2 = *top;
        PUSHTRAIL_H_ATOMIC(top,op2);
        if (!ISINT(op2)) {
            exception = c_type_error(et_INTEGER,op2); 
            return BP_ERROR;
        }
        *top = MAKEINT(INTVAL(op2)+1);
        return BP_TRUE;
    }
    exception = structure_expected; return BP_ERROR;
}

int c_DECREMENTARG(){
    register BPLONG     op1,op2;
    BPLONG_PTR top;
  
    op1 = ARG(1,2);
    op2 = ARG(2,2);
  
    DEREF(op1);DEREF(op2);
    if (!ISINT(op1)) {
        exception = c_type_error(et_INTEGER,op1); 
        return BP_ERROR;
    }
    op1 = INTVAL(op1);
    if (ISSTRUCT(op2) && op1>0 && op1 <= GET_STR_SYM_ARITY(op2)) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1;
        op2 = *top;
        PUSHTRAIL_H_ATOMIC(top,op2);
        if (!ISINT(op2)) {
            exception = c_type_error(et_INTEGER,op2); 
            return BP_ERROR;
        }
        *top = MAKEINT(INTVAL(op2)-1); 
        return BP_TRUE;
    }
    else if (ISLIST(op2) && op1 > 0 && op1 <= 2) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1-1;
        op2 = *top;
        PUSHTRAIL_H_ATOMIC(top,op2);
        if (!ISINT(op2)) {
            exception = c_type_error(et_INTEGER,op2);
            return BP_ERROR;
        }
        *top = MAKEINT(INTVAL(op2)-1);
        return BP_TRUE;
    }
    exception = structure_expected; return BP_ERROR;
}

int b_DESTRUCTIVE_SET_ARG_ccc(op1,op2,op3)
    register BPLONG op1,op2,op3;
{
    BPLONG old_op3,op3_copy;
    BPLONG_PTR top;
    BPLONG var_no = 0;

    DEREF(op1);DEREF(op2);DEREF(op3);
  
    if (IS_SUSP_VAR(op3)) op3 = UNTAGGED_ADDR(op3);
    if (!ISINT(op1)) {
        exception = c_type_error(et_INTEGER,op1); 
        return BP_ERROR;
    }
    op1 = INTVAL(op1);
    if (ISSTRUCT(op2) && op1>0 && op1 <= GET_STR_SYM_ARITY(op2)) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1;
    }
    else if (ISLIST(op2) && op1>0 && op1 <= 2) {
        top = (BPLONG_PTR)UNTAGGED_ADDR(op2)+op1-1;
    } else {
        exception = structure_expected; return BP_ERROR;
    }
    if (top<hbreg){
        old_op3 = FOLLOW(top);
        release_term_space(old_op3);
        op3_copy = copy_term_heap_to_parea_with_varno(op3,&var_no);
        if (op3_copy == BP_ERROR) return BP_ERROR;
        if (var_no != 0){
            exception = illegal_arguments;
            return BP_ERROR;
        }
        FOLLOW(top) = op3_copy;
    } else {
        FOLLOW(top) = op3;
    }
    return BP_TRUE;
}

int b_CHAR_CODE_cf(ch,code)  /* the code of op1 is op2 */ 
    BPLONG ch,code;
{
    SYM_REC_PTR       sym_ptr;
    CHAR_PTR name;
    BPLONG_PTR top;

    DEREF(ch);
    if (ISATOM(ch)){
        BPLONG len;
        sym_ptr = GET_ATM_SYM_REC(ch);
        name    = GET_NAME(sym_ptr);
        len = GET_LENGTH(sym_ptr);
        if (len == 1){
            ASSIGN_f_atom(code,MAKEINT(*name));
            return BP_TRUE;
        } else if ((len<=4) && (utf8_nchars(name) == 1)){
            ASSIGN_f_atom(code,MAKEINT(utf8_char_to_codepoint(&name)));
            return BP_TRUE;      
        }
        exception = c_type_error(et_CHARACTER,ch);
        return BP_ERROR;
    }
    if (ISREF(ch)){
        exception = et_INSTANTIATION_ERROR;
    } else {
        exception = c_type_error(et_CHARACTER,ch);
    }
    return BP_ERROR;
}

int b_CHAR_CODE_fc(ch,code)  /* the code of op1 is op2 */ 
    BPLONG ch,code;
{
    BPLONG_PTR top;
    char s[5], *ch_ptr;
  

    DEREF(code); code = INTVAL(code);

    ch_ptr = utf8_codepoint_to_str(code,s);
    *ch_ptr = '\0';
    ASSIGN_f_atom(ch,ADDTAG(insert_sym(s,(ch_ptr-s),0),ATM));
    return BP_TRUE;
}

int string2codes(str,list)
    char *str;
    BPLONG list;
{
    BPLONG temp,code;
    BPLONG_PTR tail;

    if (*str == '\0'){ /* empty string */
        return unify(list,nil_sym);
    }
    temp = ADDTAG(heap_top,LST);
  
    FOLLOW(heap_top++) = MAKEINT(*str++);
    tail = heap_top; heap_top++;
    while (*str != '\0'){
        FOLLOW(tail) = ADDTAG(heap_top,LST);
        code = utf8_char_to_codepoint(&str);
        FOLLOW(heap_top++) = MAKEINT(code);
        tail = heap_top++;
    }
    FOLLOW(tail) = nil_sym;
    return unify(list,temp);
}  

int var_or_atomic(op)
    BPLONG op;
{
    BPLONG_PTR top;

    SWITCH_OP(op,
              l_var_or_atomic,
              {return 1;},
              {return 1;},
              {return 0;},
              {if (IS_FLOAT_PSC(op)) return 1; else return 0;},
              {return 1;});
    return 0;
}

int b_BLDATOM_fc(op1,op2)
    BPLONG     op1,op2;
{            /* make op1 to be an atom with name string op2       */

    BPLONG     a, n;
    CHAR_PTR name,s;
    BPLONG  op3,orig_op2;
    BPLONG_PTR top,ptr;
  
    orig_op2 = op2;
    name = s = (char *)heap_top;
    n = 0;
  
    DEREF(op2);
    while (ISLIST(op2)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op2);
        op3 = FOLLOW(ptr);  
        DEREF(op3);
        if (!ISINT(op3)) {
            if (ISREF(op3)){
                exception = et_INSTANTIATION_ERROR;
            } else {
                exception = c_representation_error(et_CHARACTER_CODE);
            }
            return BP_ERROR;
        }
        a = INTVAL(op3);
        /*
          if (a < 0 || a > 255) {
          exception = c_representation_error(et_CHARACTER_CODE);
          return BP_ERROR;
          }
        */
        if (a <= 127){
            *s++ = (CHAR)a;
            n++;
        } else {
            CHAR_PTR ch_ptr;
            ch_ptr = utf8_codepoint_to_str(a,s);
            n += (ch_ptr-s);
            s = ch_ptr;
        }
        op2 = FOLLOW(ptr+1);
        DEREF(op2);
    }
    if (!ISNIL(op2)) {
        if (ISREF(op2)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_LIST,orig_op2);
        }
        return BP_ERROR;
    }
    *s = '\0';
    ASSIGN_f_atom(op1,ADDTAG(insert_sym(name, n, 0),ATM));
    return BP_TRUE;
}  /* b_BLDATOM */

int b_BLDNUM_fc(op1,op2)
    BPLONG     op1,op2;
{            /* make op1 the number with name string op2       */
    BPLONG     a, n;
    CHAR_PTR s,name;
    BPLONG  op3,sign,orig_op2;
    BPLONG_PTR top,ptr;
  
    s = name = (char *)heap_top;
    n = 0;
    orig_op2 = op2;

    DEREF(op2);
    sign = 1;
    /*
      if (!ISLIST(op2)){
      exception = c_type_error(et_LIST,op2);
      return BP_ERROR;
      }
    */
    while (ISLIST(op2)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(op2);
        op3 = FOLLOW(ptr);  
        DEREF(op3);
        if (!ISINT(op3)) {
            if (ISREF(op3)){
                exception = et_INSTANTIATION_ERROR;
            } else {
                exception = c_representation_error(et_CHARACTER_CODE);
            }
            return BP_ERROR;
        }
        a = INTVAL(op3);
        if (a < 0 ) {
            exception = c_representation_error(et_CHARACTER_CODE);
            return BP_ERROR;
        }
        *s++ = (CHAR)a;
        n++;
        op2 = FOLLOW(ptr+1);
        DEREF(op2);
    }
    if (!ISNIL(op2)){
        if (ISREF(op2)){
            exception = et_INSTANTIATION_ERROR;
        } else 
            exception = c_type_error(et_LIST,orig_op2);
        return BP_ERROR;
    }
    *s = '\0';
    string_in = malloc(strlen(name)+1);
    strcpy(string_in,name); 
    name = string_in;
    lastc = *string_in++;
    if (lastc == '-') {
        sign = -1;
        lastc = *string_in++;
    }
    {
        BPLONG token_t,token_v;
        int res;
        token_t = bp_build_var();
        token_v = bp_build_var();
        res = b_NEXT_TOKEN_ff(token_t,token_v);
        free(name); 
        string_in = NULL;
        if (res == BP_ERROR){
            exception = c_syntax_error(invalid_number_format);
            lastc = ' ';
            return BP_ERROR;
        }
        if (lastc != '\0'){
            exception = c_syntax_error(invalid_number_format);
            lastc = ' ';
            return BP_ERROR;
        }
        lastc = ' ';
        DEREF(token_v);
        if (ISINT(token_v)){
            if (sign == 1){
                ASSIGN_f_atom(op1,token_v);
            } else {
                ASSIGN_f_atom(op1,MAKEINT(-1*INTVAL(token_v)));
            }
            return BP_TRUE;
        } else if (ISFLOAT(token_v)){
            if (sign == 1){
                ASSIGN_sv_heap_term(op1,token_v);
            } else {
                ASSIGN_sv_heap_term(op1,encodefloat1(-1.0*floatval(token_v)));
            }
            return BP_TRUE;
        } else {
            exception = c_syntax_error(invalid_number_format);
            return BP_ERROR;
        }
    }
}   /* b_BLDNUM */


int b_SYSTEM0_cf(op1,op2)  /* op1: a list of int (string) for CShell commands */
    BPLONG     op1,op2;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;
    CHAR     s[MAX_STR_LEN];

    DEREF(op1);
    if (ISATOM(op1)){
        sym_ptr = GET_ATM_SYM_REC(op1);
        namestring(sym_ptr, s);
    } else if (b_IS_STRING_c(op1)){
        CHAR_PTR ch_ptr = s;
        BPLONG_PTR lst_ptr;

        while (ISLIST(op1)){
            BPLONG elm;
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op1);
            elm = FOLLOW(lst_ptr); DEREF(elm);
            op1 = FOLLOW(lst_ptr+1); DEREF(op1);
            sym_ptr = GET_ATM_SYM_REC(elm);
            if (ch_ptr+GET_LENGTH(sym_ptr) >= s+MAX_STR_LEN){
                exception = et_STRING_TOO_LONG;
                return BP_ERROR;
            }
            ch_ptr = namestring(sym_ptr, ch_ptr);
        }
    } else {
#ifdef PICAT
        exception = string_expected; return BP_ERROR;
#else
        exception = atom_expected; return BP_ERROR;
#endif
    }
    ASSIGN_f_atom(op2,MAKEINT(system(s)));
    return BP_TRUE;
}

int c_LOAD_cfc(){
    BPLONG op1,op2,op3;
    BPLONG_PTR top;
    SYM_REC_PTR         sym_ptr;

    op1 = ARG(1,3);
    op2 = ARG(2,3);
    op3 = ARG(3,3);

    DEREF(op1);DEREF(op3);
    sym_ptr = GET_ATM_SYM_REC(op1);
    if (dyn_loader(sym_ptr,INTVAL(op3),1) != 0){
        exception = c_representation_error(invalid_byte_file);
        return BP_ERROR;
    }
    return BP_TRUE;
}
  
int b_LOAD_cfc(op1,op2,op3)
    register BPLONG     op1,op2,op3;
{  /* arg1: the byte code file to be loaded
    * arg2: the return code, 0 => success
    * arg3: file type
    */
    register BPLONG_PTR top;
    SYM_REC_PTR         sym_ptr;

    DEREF(op1);DEREF(op3);
    sym_ptr = GET_ATM_SYM_REC(op1);
    op1 = dyn_loader(sym_ptr,INTVAL(op3),0);
    if (op1 == 1){
        exception = c_representation_error(invalid_byte_file);
        return BP_ERROR;
    } else if (op1 == -1){
        exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    ASSIGN_f_atom(op2, MAKEINT(op1));
    return BP_TRUE;
}

BPLONG compute_total_parea_size(){
    BPLONG total_parea_size;

    total_parea_size = 0;
    top = parea_low_addr;
    do {
        total_parea_size  += sizeof(BPLONG)*FOLLOW(top+1); /* next,size,actual_parea */
        top = (BPLONG_PTR)FOLLOW(top);
    } while (top != NULL);
    return total_parea_size;
}

int c_STATISTICS()
{
    BPLONG total_parea_size;

    fprintf(stderr,"\n");
    fprintf(stderr,"Stack+Heap:    %10s bytes\n",format_comma_separated_int(stack_size*sizeof(BPLONG))); 
    /*  fprintf("Total limit:  %10ld bytes\n",stack_size_limit*sizeof(BPLONG)); */
    fprintf(stderr,"  Stack in use:%10s bytes\n",format_comma_separated_int(((BPULONG)stack_up_addr-(BPULONG)arreg)));
    fprintf(stderr,"  Heap in use: %10s bytes\n\n", format_comma_separated_int(((BPULONG)heap_top-(BPULONG)stack_low_addr)));

    total_parea_size = compute_total_parea_size();
    fprintf(stderr,"Program:       %10s bytes\n", format_comma_separated_int(total_parea_size)); 
    fprintf(stderr,"  In use:      %10s bytes\n", format_comma_separated_int(total_parea_size-total_free_records_size()-((BPULONG)parea_up_addr-(BPULONG)curr_fence)));
    fprintf(stderr,"  Symbols:     %10s\n\n",  format_comma_separated_int(number_of_symbols));

    fprintf(stderr,"Trail:         %10s bytes\n", format_comma_separated_int(trail_size*sizeof(BPLONG))); 
    fprintf(stderr,"  In use:      %10s bytes\n\n", format_comma_separated_int(((BPULONG)trail_up_addr-(BPULONG)trail_top)));

    {
        int nSubgoals, maxGTCollisions, nAnswers,maxATCollisions, nTerms, maxTTCollisions,total_size,total_size_in_use;
        float aveGTCollisions, aveATCollisions, aveTTCollisions;

        total_size = table_area_size();
        total_size_in_use = total_size-table_area_notin_use();
        if (total_size_in_use != 0){
            fprintf(stderr,"Table:         %10s bytes\n", format_comma_separated_int(total_size*sizeof(BPLONG)));
            fprintf(stderr,"  In use:      %10s bytes\n", format_comma_separated_int(total_size_in_use*sizeof(BPLONG)));
            subgoal_table_statistics(&nSubgoals, &maxGTCollisions, &aveGTCollisions,
                                     &nAnswers, &maxATCollisions, &aveATCollisions,
                                     &nTerms, &maxTTCollisions, &aveTTCollisions);
            fprintf(stderr,"  Subgoals: Total(%d), Hash collisions(Max=%d, Ave=%.2f)\n", nSubgoals,maxGTCollisions,aveGTCollisions);
            fprintf(stderr,"  Answers:  Total(%d), Hash collisions(Max=%d, Ave=%.2f)\n", nAnswers,maxATCollisions,aveATCollisions);
            fprintf(stderr,"  Terms:    Total(%d), Hash collisions(Max=%d, Ave=%.2f)\n\n", nTerms,maxTTCollisions,aveTTCollisions);
        }
    }
  
    /*
      {
      int num_of_empty_buckets, len_of_longest_chain;
      symbol_table_statistics(&num_of_empty_buckets, &len_of_longest_chain);
      fprintf(stderr,"Symbol table:\n");
      fprintf(stderr,"  Total symbols(%d), Total buckets(%d), Empty buckets(%d)\n",
      (int)number_of_symbols,
      BUCKET_CHAIN,
      num_of_empty_buckets);
      fprintf(stderr,"  Hash chain lengths: Longest(%d) Average(%.2f)\n\n",  
      len_of_longest_chain,
      (float)number_of_symbols/(BUCKET_CHAIN-num_of_empty_buckets));
      }
    */

    fprintf(stderr,"Memory manager:\n");
    fprintf(stderr,"  GC:           Calls(%d), Time(%d ms)\n", (int)no_gcs,(int)gc_time);
    fprintf(stderr,"  Expansions:   Stack+Heap(%d), Program(%d), Trail(%d), Table(%d)\n\n",(int)num_stack_expansions, (int)num_parea_expansions, (int)num_trail_expansions, (int)table_area_num_expansions());

    //  fprintf(stderr,"FD backtracks:     %5d\n\n",  (int)n_backtracks);
    return BP_TRUE;
}

int c_TABLE_BLOCKS(){
    return unify(ARG(1,1),MAKEINT(table_area_size()/NUMBERED_TERM_BLOCK_SIZE));
}

int c_STATISTICS0()
{
  
    BPLONG total_parea_size,total_parea_used;

    total_parea_size = compute_total_parea_size();
    total_parea_used = total_parea_size-total_free_records_size()-((BPULONG)parea_up_addr-(BPULONG)curr_fence);
    if (!unify(ARG(1,12), MAKEINT(total_parea_size)))
        return BP_FALSE;     /* max program area */
    if (!unify(ARG(2,12), MAKEINT(total_parea_used)))
        return BP_FALSE;     /* program area in use */
    if (!unify(ARG(3,12), MAKEINT(((BPULONG)heap_top-(BPULONG)stack_low_addr))))
        return BP_FALSE;     /* heap in use */
    if (!unify(ARG(4,12), MAKEINT(((BPULONG)local_top-(BPULONG)heap_top))))
        return BP_FALSE;     /* stack area (local, global) free */
    if (!unify(ARG(5,12), MAKEINT(((BPULONG)stack_up_addr-(BPULONG)local_top))))
        return BP_FALSE;     /* control stack in use */
    if (!unify(ARG(6,12), MAKEINT(stack_size*sizeof(BPLONG))))
        return BP_FALSE;     /* total stack area size */
    if (!unify(ARG(7,12), MAKEINT(trail_size*sizeof(BPLONG))))
        return BP_FALSE;     /* total trail size */
    if (!unify(ARG(8,12), MAKEINT(((BPULONG)trail_up_addr-(BPULONG)trail_top))))
        return BP_FALSE;
    if (!unify(ARG(9,12), MAKEINT(table_area_size()*sizeof(BPLONG))))
        return BP_FALSE;
    if (!unify(ARG(10,12), MAKEINT((table_area_size()-table_area_notin_use())*sizeof(BPLONG))))
        return BP_FALSE;
#ifdef GC
    if (!unify(ARG(11,12), MAKEINT(no_gcs)))
        return BP_FALSE;
#else
    if (!unify(ARG(11,12), MAKEINT(0)))
        return BP_FALSE;
#endif
    if (!unify(ARG(12,12), MAKEINT(n_backtracks)))
        return BP_FALSE;

    return BP_TRUE;
}

int b_HASHVAL_cf(op1,op2)  /* op1 a term, op2 the hash value of op1 */
    BPLONG     op1, op2;
{
    ASSIGN_f_atom(op2,MAKEINT(bp_hashval(op1)));
    return BP_TRUE;
}

/* iterative version for computing hashcode, which avoids native stack overflow */
BPLONG bp_hashval_list(BPLONG term){
    BPLONG prev_term,car,cdr,hcode_sum,this_hcode;
    BPLONG_PTR term_ptr;

    //  printf("hashcode of "); write_term(term); printf(" is %x ",bp_hashval_naive_list(term));
    prev_term = nil_sym;
    //  printf("hashval_list ");write_term(term);printf("\n");
lab_reverse: /* ugly!! but has to consider the case when the tail is a free variable */
    term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    cdr = FOLLOW(term_ptr+1);
lab_test_cdr:
    if (ISLIST(cdr)){
        FOLLOW(term_ptr+1) = prev_term;
        prev_term = term;
        term = cdr;
        goto lab_reverse;
    } else if (ISREF(cdr) && !ISFREE(cdr)){ /* shorten the reference chain */
        BPLONG_PTR tmp_ptr = term_ptr+1;
        PUSHTRAIL_H_NONATOMIC(tmp_ptr, cdr);
        FOLLOW(tmp_ptr) = FOLLOW(cdr);
        cdr = FOLLOW(cdr);
        goto lab_test_cdr;
    }

    hcode_sum = bp_hashval(cdr);

    /* cdr is no longer tagged LST. Once here, the original list has been reversed, except for the last cons.
       Now reverse it back while computing the hash code 
    */
lab_reverse_back:
    car = FOLLOW(term_ptr);
    if (TAG(car) == ATM){
        this_hcode = ((car & HASH_BITS)>>2);
    } else {
        this_hcode = bp_hashval(car);
    }
    if (this_hcode != 0){
        hcode_sum = this_hcode + hcode_sum*MULTIPLIER+1;
        hcode_sum = hcode_sum & HASH_BITS;
    }
    cdr = term;
    term = prev_term;
    if (ISLIST(term)){
        term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        prev_term = FOLLOW(term_ptr+1);
        FOLLOW(term_ptr+1) = cdr;
        goto lab_reverse_back;
    }
    /* once here, the list is reversed back */
    return hcode_sum;
}

/* NOTE: A tabled term always has its hash code stored with it, so it needs not be recomputed. */
BPLONG bp_hashval(BPLONG op){
    BPLONG  i,arity,hcode_sum,this_hcode;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top, term_ptr;
  
    SWITCH_OP(op,hashval_lab,
              {return 0;},
              {return  (((op & HASH_BITS)>>2));},
              {
                  if (ISLIST(op)){
                      term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
                      if (!IS_HEAP_REFERENCE(term_ptr)){         
                          return (FOLLOW(term_ptr-2) & HASH_BITS);
                      }
                      return bp_hashval_list(op);
                  } else return 0;
              },
              {
                  term_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
                  if (!IS_HEAP_REFERENCE(term_ptr)){         
                      return (FOLLOW(term_ptr-2) & HASH_BITS);
                  }
                  sym_ptr = (SYM_REC_PTR)FOLLOW(term_ptr);
                  arity = GET_ARITY(sym_ptr);
                  hcode_sum = (((BPLONG)sym_ptr & HASH_BITS)>>2);
                  hcode_sum += bp_hashval(*(term_ptr+1));
                  for (i=2;i<=arity;i++){
                      this_hcode = bp_hashval(*(term_ptr+i));
                      if (this_hcode != 0) hcode_sum = MurmurHash3_x86_32_uint32((UW32)this_hcode,(UW32)hcode_sum);    
                  }
                  return (hcode_sum & HASH_BITS);
              },
              {return 0;});
}

int b_HASHVAL1_cf(op1,op2)  /* op1 a term, op2 the hash value of the main functor of op1*/
    BPLONG     op1, op2;
{
    BPLONG hashcode;
  
    BP_HASH_CODE1(op1,hashcode,lab);
    ASSIGN_f_atom(op2,MAKEINT(hashcode));
    return BP_TRUE;
}

int b_HASHTABLE_GET_ccf(table,key,value)
    BPLONG table,key,value;
{
    SYM_REC_PTR  sym_ptr;
    BPLONG res,buckets;
    BPLONG_PTR ptr,top;
    BPLONG index,size;
  
    /*  write_term(key);printf("   "); write_term(table);printf("\n"); */
    DEREF(key);
    if (ISREF(key)){
        exception = nonvariable_expected;
        return BP_ERROR;
    }

    DEREF(table);
    if (!ISSTRUCT(table)){
        exception = illegal_arguments;
        return BP_ERROR;
    }
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(table);
    if ((SYM_REC_PTR)FOLLOW(ptr) != hashtable_psc){
        exception = illegal_arguments;
        return BP_ERROR;
    }
    buckets = FOLLOW(ptr+2); /* $hshtb(Count,Buckets) */
    DEREF(buckets);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(buckets);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
    size = GET_ARITY(sym_ptr);
    index = bp_hashval(key) % size + 1;
    res = hashtable_lookup_chain(FOLLOW(ptr+index),key);
    if (res == 0) return 0;
    DEREF(res); /* avoid creating cyclic terms by setarg? */
    //  printf("%x hashtable_get ", res); write_term(res); printf("\n");
    ASSIGN_sv_heap_term(value,res);
    return BP_TRUE;
}

int hashtable_contains_key(table,key)
    BPLONG table,key;
{
    SYM_REC_PTR  sym_ptr;
    BPLONG res,buckets;
    BPLONG_PTR ptr,top;
    BPLONG index,size;
  
    /*  write_term(key);printf("   "); write_term(table);printf("\n"); */
    DEREF(key);
    DEREF(table);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(table);
    buckets = FOLLOW(ptr+2); /* $hshtb(Count,Buckets) */
    DEREF(buckets);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(buckets);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
    size = GET_ARITY(sym_ptr);
    index = bp_hashval(key) % size + 1;
    res = hashtable_lookup_chain(FOLLOW(ptr+index),key);
    if (res == 0) return 0;
    return 1;
}

int bp_is_hashtable(term)
    BPLONG term;
{
    BPLONG_PTR ptr;
    DEREF(term);
    if (!ISSTRUCT(term)) return BP_FALSE;
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
    return FOLLOW(ptr) == (BPLONG)hashtable_psc;
}

BPLONG bp_hashtable_get(table,key)
    BPLONG table,key;
{
    SYM_REC_PTR  sym_ptr;
    BPLONG buckets;
    BPLONG_PTR ptr;
    BPLONG index,size;
  
    /*  write_term(key);printf("   "); write_term(table);printf("\n"); */
    DEREF_NONVAR(table);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(table);
    buckets = FOLLOW(ptr+2); /* $hshtb(Count,Buckets) */
    DEREF_NONVAR(buckets);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(buckets);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
    size = GET_ARITY(sym_ptr);
    index = bp_hashval(key) % size + 1;
    return hashtable_lookup_chain(FOLLOW(ptr+index),key);
}
  
BPLONG hashtable_lookup_chain(chain,key)
    BPLONG chain,key;
{
    BPLONG_PTR top,ptr,str_ptr;
    BPLONG car,key1;

    DEREF(chain);
    while (ISLIST(chain)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(chain);
        car = FOLLOW(ptr); DEREF(car);
        str_ptr = (BPLONG_PTR)UNTAGGED_ADDR(car);
        key1 = FOLLOW(str_ptr+1); DEREF(key1);
        if (key1 == key || (TAG(key) != ATM && key_identical(key1,key))){
            return FOLLOW(str_ptr+2);
        }
        chain = FOLLOW(ptr+1);
        DEREF(chain);
    }
    return 0;
}

BPLONG make_struct1(f,op1)
    char *f;
    BPLONG op1;
{
    SYM_REC_PTR sym_ptr;
    BPLONG return_value;
    BPLONG_PTR top;

    sym_ptr = BP_NEW_SYM(f,1);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    BUILD_VALUE(op1,lab1);
    return return_value;
}

BPLONG make_struct2(f,op1,op2)
    char *f;
BPLONG op1,op2;
{
    SYM_REC_PTR sym_ptr;
    BPLONG return_value;
    BPLONG_PTR top;

    sym_ptr = BP_NEW_SYM(f,2);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    BUILD_VALUE(op1,lab1);
    BUILD_VALUE(op2,lab2);
    return return_value;
}

BPLONG make_struct3(f,op1,op2,op3)
    char *f;
BPLONG op1,op2,op3;
{
    SYM_REC_PTR sym_ptr;
    BPLONG return_value;
    BPLONG_PTR top;

    sym_ptr = BP_NEW_SYM(f,3);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    BUILD_VALUE(op1,lab1);
    BUILD_VALUE(op2,lab2);
    BUILD_VALUE(op3,lab3);
    return return_value;
}

BPLONG make_struct4(f,op1,op2,op3,op4)
    char *f;
BPLONG op1,op2,op3,op4;
{
    SYM_REC_PTR sym_ptr;
    BPLONG return_value;
    BPLONG_PTR top;

    sym_ptr = BP_NEW_SYM(f,4);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    BUILD_VALUE(op1,lab1);
    BUILD_VALUE(op2,lab2);
    BUILD_VALUE(op3,lab3);
    BUILD_VALUE(op4,lab4);
    return return_value;
}

BPLONG make_struct5(f,op1,op2,op3,op4,op5)
    char *f;
BPLONG op1,op2,op3,op4,op5;
{
    SYM_REC_PTR sym_ptr;
    BPLONG return_value;
    BPLONG_PTR top;

    sym_ptr = BP_NEW_SYM(f,5);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    BUILD_VALUE(op1,lab1);
    BUILD_VALUE(op2,lab2);
    BUILD_VALUE(op3,lab3);
    BUILD_VALUE(op4,lab4);
    BUILD_VALUE(op5,lab5);
    return return_value;
}

BPLONG make_struct_dummy(sym_ptr)
    SYM_REC_PTR sym_ptr;
{
    BPLONG arity,i;
    BPLONG return_value;

    arity = GET_ARITY(sym_ptr);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    for (i=1;i<=arity;i++){
        NEW_HEAP_FREE;
    }
    return return_value;
}

BPLONG make_struct_holders(sym_ptr,init_val)
    SYM_REC_PTR sym_ptr;
    BPLONG init_val;
{
    BPLONG arity,i;
    BPLONG return_value;

    arity = GET_ARITY(sym_ptr);
    return_value = ADDTAG(heap_top,STR);
    NEW_HEAP_NODE((BPLONG)sym_ptr);
    for (i=1;i<=arity;i++){
        FOLLOW(heap_top++) = init_val;
    }
    return return_value;
}

BPLONG make_cons_dummy(){
    BPLONG return_value;
  
    return_value = ADDTAG(heap_top,LST);
    NEW_HEAP_FREE;
    NEW_HEAP_FREE;
    return return_value;
}

BPLONG make_struct_with_args(fp,sym_ptr)
    BPLONG_PTR fp;
    SYM_REC_PTR sym_ptr;
{
    BPLONG arity,i;
    BPLONG return_value;
    BPLONG_PTR top;
    BPLONG op;

    arity = GET_ARITY(sym_ptr);
    if (arity == 0) 
        return ADDTAG(insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),0),ATM);
    else {
        return_value = ADDTAG(heap_top,STR);
        NEW_HEAP_NODE((BPLONG)sym_ptr);
        for (i=arity;i>=1;i--){
            op = FOLLOW(fp+i);
            BUILD_VALUE(op,lab1);
        }
        return return_value;
    }
}

/* return the number of bytes in an atom */
int b_GET_LENGTH_cf(op1,op2)
    BPLONG op1,op2;
{
    BPLONG_PTR top;

    DEREF(op1);
    ASSIGN_f_atom(op2,MAKEINT(GET_SYM_LENGTH(op1)));
    return BP_TRUE;
}

/* return the number of utf-8 chars in an atom */
int b_GET_UTF8_NCHARS_cf(op1,op2)
    BPLONG op1,op2;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;

    DEREF(op1);
    sym_ptr = GET_SYM_REC(op1);
    if (GET_LENGTH(sym_ptr) == 1){ /* if the number of bytes is 1, then the number of chars must be 1 */
        ASSIGN_f_atom(op2,BP_ONE);
    } else {
        ASSIGN_f_atom(op2,MAKEINT(utf8_nchars(GET_NAME(sym_ptr))));
    }
    return BP_TRUE;
}
  
int c_SHOW_NONDET_FRAME() {
    BPLONG op1,i;
    BPLONG_PTR tempreg;

    op1 = ARG(1,1);
    op1 = INTVAL(op1);
    tempreg = (BPLONG_PTR)*arreg;
    printf("AR=   = %lx\n",tempreg);
    printf("(AR)  = %lx\n", *tempreg); 
    printf("CPS   = %lx\n", *(tempreg-1)); 
    printf("TOP   = %lx\n", *(tempreg-2)); 
    printf("B     = %lx\n", *(tempreg-3)); 
    printf("CPF   = %lx\n", *(tempreg-4)); 
    printf("H     = %lx\n", *(tempreg-5)); 
    printf("T     = %lx\n", *(tempreg-6)); 
    for (i=1;i<= op1 ;i++) {
        printf("%lx",*(tempreg-6-i));
        if (*(tempreg-6-i) == (BPLONG)(tempreg-6-i))
            printf(" ***\n");
        else
            if (((BPULONG)*(tempreg-6-i) > (BPULONG)heap_top) && ((BPULONG)*(tempreg-6-i) < (BPULONG)(tempreg-6-op1)))
                printf(" ???\n");
            else 
                printf("\n");
    }
    return BP_TRUE;
}

int b_CPUTIME_f(op1)
    BPLONG op1;
{
    BPLONG msec;

    msec = cputime();
    ASSIGN_f_atom(op1,MAKEINT(msec));
    return BP_TRUE;
}
  
int membchk() 
{
    BPLONG op1,op2;
    BPLONG_PTR top;

    op1 = ARG(1,2);  DEREF(op1);
    op2 = ARG(2,2);
    return membchk2(op1,op2);
}

int membchk2(x,list)
    BPLONG x,list;
{
    BPLONG car;
    BPLONG_PTR top;

    DEREF(list);
    while (ISLIST(list)){
        top = (BPLONG_PTR)UNTAGGED_ADDR(list);
        car = FOLLOW(top);
        if (bp_identical(x,car)) return BP_TRUE;
        list = FOLLOW(top+1);
        DEREF(list);
    }
    return BP_FALSE;
}
    

void quit(s)
    CHAR_PTR s;
{
#ifdef BPSOLVER
    //  fprintf(stdout,"%% UNKNOWN\n");
    fprintf(stderr,"%% "); fprintf(stderr,"%s\n",s);
    exit(1);
#else
    c_STATISTICS();                \
    fprintf(stderr,s);
    exit(0);
#endif
}

/* concatinate s1 and s2 into s3 */

void scat(s1, s2, s3)
    CHAR_PTR s1, s2, s3;
{
    while (*s1)             /* copy s1 into s3, without the EOS */
        *s3++ = *s1++;
    while ((*s3++ = *s2++));   /* add s2 onto s3, including the EOS */
}


/*********** functor(T,F,N) *********/
int c_functor(){
    BPLONG op1,op2,op3;

    op1 = ARG(1,3);
    op2 = ARG(2,3);
    op3 = ARG(3,3);
    return cfunctor1(op1,op2,op3);
}

int cfunctor1(op1,op2,op3)
    BPLONG op1,op2,op3;
{
    BPLONG i;
    BPLONG top1,top2;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR top;

    DEREF(op1);
    SWITCH_OP(op1,n_functor1,
              {goto functor_vdd;},
              { top1 = MAKEINT(0);
                  if (unify(op2,op1) && unify(op3,top1))
                      return BP_TRUE;
                  else
                      return BP_FALSE;},
              {top1 = (BPLONG)UNTAGGED_ADDR(period_sym);
                  top2 = MAKEINT(2);
                  goto functor_compound_d_d;},
        
              {sym_ptr = GET_SYM_REC(op1);
                  top1 = (BPLONG)insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),0);
                  top2 = MAKEINT(GET_ARITY(sym_ptr));
                  goto functor_compound_d_d;},
        
              {goto functor_vdd;});

functor_compound_d_d:
    top1 = ADDTAG(top1,ATM);
    return (unify(op2,top1) && unify(op3,top2)) ?  1 : 0;

functor_vdd:
    DEREF(op2);
    DEREF(op3);
    if (ISNUM(op2)) {
        if (ISREF(op3)){
            exception = et_INSTANTIATION_ERROR;
            return BP_ERROR;
        } else if (ISINT(op3)){
            if (INTVAL(op3) != 0){
                exception = c_type_error(et_ATOM,op2);
                return BP_ERROR;
            } else {
                return unify(op1,op2);
            }
        } else {
            exception = c_type_error(et_INTEGER,op3);
            return BP_ERROR;
        }
    }
    if (!ISATOM(op2)) {
        exception = c_type_error(et_ATOMIC,op2);
        return BP_ERROR;    
    }
    DEREF(op3);
    if (!ISINT(op3)) {
        if (ISREF(op3)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_INTEGER,op3);
        }
        return BP_ERROR;
    }
    op3 = INTVAL(op3);
    if (op3<0) {
        exception = c_domain_error(et_NOT_LESS_THAN_ZERO,MAKEINT(op3));
        return BP_ERROR;
    }
    sym_ptr = GET_SYM_REC(op2);
    LOCAL_OVERFLOW_CHECK("functor");
    if (op3 == 0) {
        unify(op1,ADDTAG(insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),op3),ATM));
        return BP_TRUE;
    };
    if (op3 == 2 && GET_NAME(sym_ptr)[0] == '.' && GET_LENGTH(sym_ptr) == 1) {
        unify(op1,ADDTAG(heap_top,LST));
    } else {
        top1 = (BPLONG)insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),op3);
        unify(op1,ADDTAG(heap_top,STR));
        *heap_top++ = top1;
    }
    for (i=0;i<op3;heap_top++,i++)
        MAKE_FREE(BPLONG,*heap_top);
    return BP_TRUE;
}

/************* arg(N,T,A) *************/
int c_arg(){
    BPLONG op1,op2,op3;
  
    op1 = ARG(1,3);
    op2 = ARG(2,3);
    op3 = ARG(3,3);
  
    return carg1(op1,op2,op3);
}

int b_GEN_ARG_ccf(Index,Comp,Arg)
    BPLONG Index,Comp,Arg;
{
    BPLONG res;
  
    res = bp_access_one_array(Comp,Index);
    if (res == BP_ERROR) return BP_ERROR;
    ASSIGN_v_heap_term(Arg,res);
    return BP_TRUE;
}
    
int carg1(op1,op2,op3)
    BPLONG op1,op2,op3;
{
    BPLONG_PTR top;

    DEREF(op1);
    op1 = INTVAL(op1);
    if (op1 <= 0) {
/*    printf("type error: index for arg must be > 0");*/
        exception = out_of_range; return BP_ERROR;
    }
    DEREF(op2);
    if (ISSTRUCT(op2) && op1 <= GET_SYM_ARITY(op2)){
        if (unify(*((BPLONG_PTR) UNTAGGED_ADDR(op2) + op1), op3))
            return BP_TRUE;
        else 
            return BP_FALSE;
    }
    if (ISLIST(op2) && op1 <= 2){
        if (unify(*((BPLONG_PTR) UNTAGGED_ADDR(op2) + op1 - 1), op3))
            return BP_TRUE;
        else
            return BP_FALSE;
    }  else {
/*    printf("instantiation error: second argument of arg must be instantiated\n");*/
        exception = structure_expected; return BP_ERROR;
    }
}

int c_get_main_args(){
    BPLONG list;
  
    list = ARG(1,1);

    return unify(list,main_args);
}


int c_SAVE_AR(){
    BPLONG AR;
  
    AR = ARG(1,1);
    return unify(AR,ADDTAG(FOLLOW(arreg),ATM));
}

/*
  c_WRITE_AR(){
  BPLONG AR;
  BPLONG_PTR top;
  
  AR = ARG(1,1);
  DEREF(AR);
  printf("%x",UNTAGGED_ADDR(AR));
  return BP_TRUE;
  }

  c_ANCESTOR_FRAME(){
  BPLONG_PTR ancestorAr,ar;
  BPLONG arg=ARG(1,1);
  ancestorAr = (BPLONG_PTR)UNTAGGED_ADDR(arg);
  ar = arreg;

  while (ar<ancestorAr){
  ar = (BPLONG_PTR)AR_AR(ar);
  }
  return ar == ancestorAr;
  }
*/
#ifdef GC
int c_set_gc_threshold(){
    BPLONG_PTR top;
    BPLONG value = ARG(1,1);
    DEREF(value);
    gc_threshold = INTVAL(value);
    if (gc_threshold<=2) gc_threshold=3;
    return BP_TRUE;
}
#endif

int b_NTH_ELM_ccf(i,l,v)
    BPLONG i,l,v;
{
    BPLONG_PTR ptr,top;
    BPLONG elm;

    DEREF(i); i = INTVAL(i);
    if (i<=0) return BP_FALSE;
    DEREF(l);
    while (ISLIST(l)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(l);
        elm = FOLLOW(ptr);
        if (i == 1){ ASSIGN_f_atom(v,elm); return BP_TRUE; }
        i--;
        l = FOLLOW(ptr+1);
        DEREF(l);
    }
    return BP_FALSE;
}

    
void myquit(overflow_type,src)
    BPLONG overflow_type;
    char *src;
{
#ifdef BPSOLVER
    // fprintf(stdout,"%% UNKNOWN\n");
    fprintf(stderr,"%% stack overflow in "); fprintf(stderr,"%s\n",src);
    exit(1);
#else
    switch (overflow_type) {
    case STACK_OVERFLOW:
        c_STATISTICS();
        fprintf(stderr,"\nStack overflow in \"%s\" after %lld garbage collections and %lld stack expansions.\n",src,no_gcs,num_stack_expansions);
#ifndef PICAT
        fprintf(stderr,"Please start B-Prolog with more stack space as\n");
        fprintf(stderr,"   bp -s xxx\n");
        fprintf(stderr,"where xxx > %lld.\n",(int)stack_size);
#endif
        exit(1);

    case TRAIL_OVERFLOW:
        c_STATISTICS();
        fprintf(stderr,"\nTRAIL stack overflow in \"%s\" after %lld garbage collections and %lld trail expansions.\n",src,no_gcs,num_trail_expansions);
#ifndef PICAT
        fprintf(stderr,"Please start B-Prolog with more trail stack space as\n");
        fprintf(stderr,"   bp -b xxx\n");
        fprintf(stderr,"where xxx > %lld.\n",trail_size);
#endif
        exit(1);

    case PAREA_OVERFLOW:
        c_STATISTICS();
        fprintf(stderr,"\nProgram area overflow in \"%s\" after %lld expansions.\n",src,num_parea_expansions);
        exit(1);

    case OUT_OF_MEMORY:
        c_STATISTICS();
        fprintf(stderr,"\nOut of memory in \"%s\"\n",src);
        exit(1);

    default:
        exit(1);
    }
#endif
}

#define HTABLE_HCODE(tuple_ptr,arity,hcode){    \
        BPLONG op;                              \
        hcode = FOLLOW(tuple_ptr+1);            \
        DEREF_NONVAR(hcode);                    \
        hcode = INTVAL(hcode);                  \
        for (i=2;i<=arity;i++){                 \
            op = FOLLOW(tuple_ptr+i);           \
            DEREF_NONVAR(op);                   \
            hcode += MULTIPLIER*INTVAL(op)+1;   \
        }                                       \
        hcode = (hcode & HASH_BITS);            \
    } 

/* Hashtable talored to tuples of integers */
/* htable = '{}'(B1,...,Bn) where Bi is a bucket */
int c_HTABLE_HCODE(){

    BPLONG Tuple,Code,arity,hcode,i;
    BPLONG_PTR tuple_ptr;
    SYM_REC_PTR sym_ptr;
  
    Tuple = ARG(1,2);
    Code = ARG(2,2);
    DEREF_NONVAR(Tuple);
    tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuple);
    sym_ptr = (SYM_REC_PTR)FOLLOW(tuple_ptr);
    arity = GET_ARITY(sym_ptr);
    HTABLE_HCODE(tuple_ptr,arity,hcode);
    unify(Code,MAKEINT(hcode));
    return BP_TRUE;
}

/* n and arity is untagged, args of tuple_ptr are dereferenced */
int htable_contains_tuple(htable_ptr,n,tuple_ptr,arity)
    BPLONG n,arity;
BPLONG_PTR htable_ptr,tuple_ptr;
{
    BPLONG hcode,tuple,i,lst;
    BPLONG_PTR ptr,another_tuple_ptr;
    /*  
        printf("htable_contains\n"); 
        write_term(ADDTAG(htable_ptr,STR)); printf("\n");
        write_term(ADDTAG(tuple_ptr,STR)); printf("\n");
    */
    HTABLE_HCODE(tuple_ptr,arity,hcode);
    i = hcode % n + 1;
    lst = FOLLOW(htable_ptr+i);
    DEREF(lst);
    //  printf("chain = "); write_term(lst); printf("\n");
loop1: while (ISLIST(lst)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        tuple = FOLLOW(ptr); DEREF_NONVAR(tuple);
        another_tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(tuple);
        lst = FOLLOW(ptr+1); DEREF(lst);
        for (i=1;i<=arity;i++){ /* lookup the chain */
            BPLONG op1,op2;
            op1 = FOLLOW(another_tuple_ptr+i);  DEREF_NONVAR(op1);
            op2 = FOLLOW(tuple_ptr+i); DEREF_NONVAR(op2);
            if (op1 != op2) goto loop1;
        }
        return BP_TRUE;
    }
    return BP_FALSE;
}

int b_HTABLE_CONTAINS_TUPLE(Htable,Tuple)
    BPLONG Htable,Tuple;
{
    SYM_REC_PTR sym_ptr;
    BPLONG arity,htable_size;
    BPLONG_PTR htable_ptr,tuple_ptr;
  
    DEREF_NONVAR(Htable);
    htable_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Htable);
    sym_ptr = (SYM_REC_PTR)FOLLOW(htable_ptr);
    htable_size = GET_ARITY(sym_ptr);
  
    DEREF_NONVAR(Tuple);
    tuple_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Tuple);
    sym_ptr = (SYM_REC_PTR)FOLLOW(tuple_ptr);
    arity = GET_ARITY(sym_ptr);

    return htable_contains_tuple(htable_ptr,htable_size,tuple_ptr,arity);
}

int b_GLOBAL_HEAP_VTABLE_REF_f(op)
    BPLONG op;
{
    ASSIGN_sv_heap_term(op,FOLLOW(breg0+NUM_CG_GLOBALS+1));
    return BP_TRUE;
}

int b_GLOBAL_HEAP_GET_cf(key,value)
    BPLONG key,value;
{
    int res;
    BPLONG table = FOLLOW(breg0+NUM_CG_GLOBALS+1);
    res = b_HASHTABLE_GET_ccf(table,key,value);
    //  printf("global_heap_get res=%x",res); write_term(key); printf("\n");
    if (res == BP_ERROR) return BP_FALSE;
    return res;
}

int c_GET_REDEFINE_WARNING(){
    BPLONG warn = ARG(1,1);
    return unify(warn,MAKEINT(redefine_warning));
}

int c_SET_REDEFINE_WARNING(){
    BPLONG warn = ARG(1,1);
    DEREF(warn); warn = INTVAL(warn);
    redefine_warning = warn;
    return BP_TRUE;
}

/* attr must be an atom or integer, and it must exist */
BPLONG fast_get_attr(sv_ptr,attr)
    BPLONG_PTR sv_ptr;
    BPLONG attr;
{
    BPLONG_PTR ptr,pair_ptr;
    BPLONG attrs,pair,attr1;

    attrs = DV_attached(sv_ptr);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(attrs);
    attrs = FOLLOW(ptr+1); /* $attrs(AttrValueList) */
    DEREF_NONVAR(attrs);
    for (;;){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(attrs);
        pair = FOLLOW(ptr);
        DEREF_NONVAR(pair);
        pair_ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
        attr1 = FOLLOW(pair_ptr+1); DEREF_NONVAR(attr1); /* (attr,value) */
        if (attr == attr1) return FOLLOW(pair_ptr+2);
        attrs = FOLLOW(ptr+1);DEREF_NONVAR(attrs);
    }
    return BP_FALSE;
}

int b_GET_ATTR_ccf(var,attr,value)
    BPLONG var,attr,value;
{
    BPLONG_PTR ptr,pair_ptr;
    BPLONG attrs,pair,attr1;

    DEREF(var);
    if (!IS_SUSP_VAR(var)){
        return BP_FALSE;
    }
    ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
    attrs = DV_attached(ptr);
    /*  DEREF(attrs); */
    if (!ISSTRUCT(attrs)){ 
        return BP_FALSE;
    }
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(attrs);
    attrs = FOLLOW(ptr+1); /* $attrs(AttrValueList) */
    DEREF(attrs);
    DEREF(attr);
    while (ISLIST(attrs)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(attrs);
        pair = FOLLOW(ptr);
        DEREF(pair);
        pair_ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
        attr1 = FOLLOW(pair_ptr+1); DEREF(attr1); /* (attr,value) */
        if (attr == attr1 || (TAG(attr) != ATM && key_identical(attr,attr1))){ASSIGN_f_atom(value,FOLLOW(pair_ptr+2));return BP_TRUE;}
        attrs = FOLLOW(ptr+1);DEREF(attrs);
    }
    return BP_FALSE;
}

BPLONG c_domain_error(type,term)
    BPLONG type, term;
{
    BPLONG err;

    GLOBALIZE_VAR(term);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_DOMAIN_ERROR;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = term;
    return err;
}

BPLONG c_evaluation_error(type,obj)
    BPLONG type,obj;
{
    BPLONG err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_EVALUATION_ERROR;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = obj;
    return err;
}

BPLONG c_update_error(type)
    BPLONG type;
{
    BPLONG err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_UPDATE_ERROR;
    FOLLOW(heap_top++) = type;
    return err;
}

BPLONG c_existence_error(type,term) 
    BPLONG type, term;
{
    BPLONG err;

    GLOBALIZE_VAR(term);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_EXISTENCE_ERROR;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = term;
    return err;
}
     
BPLONG c_permission_error(type1,type2,term) 
    BPLONG type1,type2, term;
{
    BPLONG err;

    GLOBALIZE_VAR(term);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_PERMISSION_ERROR;
    FOLLOW(heap_top++) = type1;
    FOLLOW(heap_top++) = type2;
    FOLLOW(heap_top++) = term;
    return err;
}

BPLONG c_representation_error(type) 
    BPLONG type;
{
    BPLONG err;

    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_REPRESENTATION_ERROR;
    FOLLOW(heap_top++) = type;
    return err;
}

BPLONG is_iso_exception(exception)
    BPLONG exception;
{
    DEREF(exception);
    return (exception == et_INSTANTIATION_ERROR ||
            !ISATOM(exception));
}

BPLONG c_builtin_error1(type,op1) 
    BPLONG type,op1;
{
    BPLONG err;
  
    GLOBALIZE_VAR(op1);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_BUILTIN_ERROR1;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = op1;
    return err;
}

BPLONG c_builtin_error2(type,op1,op2) 
    BPLONG type,op1,op2;
{
    BPLONG err;
  
    GLOBALIZE_VAR(op1);
    GLOBALIZE_VAR(op2);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_BUILTIN_ERROR2;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = op1;
    FOLLOW(heap_top++) = op2;
    return err;
}

BPLONG c_builtin_error3(type,op1,op2,op3) 
    BPLONG type,op1,op2,op3;
{
    BPLONG err;
  
    GLOBALIZE_VAR(op1);
    GLOBALIZE_VAR(op2);
    GLOBALIZE_VAR(op3);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_BUILTIN_ERROR3;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = op1;
    FOLLOW(heap_top++) = op2;
    FOLLOW(heap_top++) = op3;
    return err;
}

BPLONG c_builtin_error4(type,op1,op2,op3,op4) 
    BPLONG type,op1,op2,op3,op4;
{
    BPLONG err;
  
    GLOBALIZE_VAR(op1);
    GLOBALIZE_VAR(op2);
    GLOBALIZE_VAR(op3);
    GLOBALIZE_VAR(op4);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_BUILTIN_ERROR4;
    FOLLOW(heap_top++) = type;
    FOLLOW(heap_top++) = op1;
    FOLLOW(heap_top++) = op2;
    FOLLOW(heap_top++) = op3;
    FOLLOW(heap_top++) = op4;
    return err;
}

BPLONG c_type_error(type,term) 
    BPLONG type, term;
{
    BPLONG err;
    SYM_REC_PTR sym_ptr;
  
    GLOBALIZE_VAR(term);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_TYPE_ERROR;
    FOLLOW(heap_top++) = type;
    if (type == et_EVALUABLE){
        if (ISATOM(term)){
            FOLLOW(heap_top) = ADDTAG((heap_top+1),STR);
            heap_top++;
            FOLLOW(heap_top++) = (BPLONG)slash_psc;
            FOLLOW(heap_top++) = term;
            FOLLOW(heap_top++) = BP_ZERO;
        } else if (ISSTRUCT(term)){
            sym_ptr = (SYM_REC_PTR)FOLLOW(UNTAGGED_ADDR(term));
            FOLLOW(heap_top) = ADDTAG((heap_top+1),STR);
            heap_top++;
            FOLLOW(heap_top++) = (BPLONG)slash_psc;
            FOLLOW(heap_top++) = ADDTAG(BP_NEW_SYM(GET_NAME(sym_ptr),0),ATM);
            FOLLOW(heap_top++) = MAKEINT(GET_ARITY(sym_ptr));
        } else {
            FOLLOW(heap_top++) = term;
        }
    } else {
        FOLLOW(heap_top++) = term;
    }
    return err;
}
 
BPLONG c_syntax_error(term)
    BPLONG term;
{
    BPLONG err;

    GLOBALIZE_VAR(term);
    err = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)str_SYNTAX_ERROR;
    FOLLOW(heap_top++) = term;
    return err;
}

BPLONG c_stream_struct(Index)
    BPLONG Index;
{
    BPLONG src;
  
    src = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)BP_NEW_SYM("$stream",1);
    FOLLOW(heap_top++) = MAKEINT(Index);
    return src;
}

BPLONG c_error_src(str,arity)
    char *str;
    int arity;
{
    BPLONG src;
  
    src = ADDTAG(heap_top,STR);
    FOLLOW(heap_top++) = (BPLONG)slash_psc;
    FOLLOW(heap_top++) = ADDTAG(BP_NEW_SYM(str,0),ATM);
    FOLLOW(heap_top++) = MAKEINT(arity);

    return src;
}

int b_LIST_LENGTH_cff(lst,lstr,len)
    BPLONG lst,lstr,len;
{
    BPLONG_PTR top;
    int i = 0;

restart:
    while (ISLIST(lst)){
        i++;
        lst = *((BPLONG_PTR)UNTAGGED_ADDR(lst)+1);
    }
    DEREF(lst);  if (ISLIST(lst)) goto restart;
    ASSIGN_sv_heap_term(lstr,lst);
    ASSIGN_f_atom(len,MAKEINT(i));
    return BP_TRUE;
}

int b_PICAT_LENGTH_cf(term,len)
    BPLONG term,len;
{
    BPLONG_PTR top;
    BPLONG i;
    //  printf("=>PICAT_LENGTH "); write_term(term); printf("\n");
start:
    switch (TAG(term)) { 
    case REF: 
        NDEREF(term, start);
        exception = nonvariable_expected;
        return BP_ERROR;
    case ATM:
        if (ISINT(term)){
            exception = c_type_error(no_number_expected,term);      
            return BP_ERROR;
        }
        if (term == nil_sym || term == empty_set) {
            ASSIGN_f_atom(len,BP_ZERO);
            return BP_TRUE;
        }
        return b_GET_UTF8_NCHARS_cf(term,len);
    case LST:
        i = list_length(term,term);
        if (i == BP_ERROR){
            exception = c_type_error(et_LIST,term);
            return BP_ERROR;
        }
        ASSIGN_f_atom(len,MAKEINT(i));
        return BP_TRUE;
    case STR:
    {
        SYM_REC_PTR sym_ptr;

        if (IS_SUSP_VAR(term)){
            exception = nonvariable_expected;
            return BP_ERROR;
        }
        sym_ptr = GET_STR_SYM_REC(term);
        if (sym_ptr == bigint_psc || sym_ptr == float_psc){
            exception = c_type_error(no_number_expected,term);      
            return BP_ERROR;
        }
        ASSIGN_f_atom(len,MAKEINT(GET_ARITY(sym_ptr)));
        return BP_TRUE;
    }
    return BP_TRUE;
    }
}

/* len > 0 */
BPLONG intarray_to_intlist(array,len)
    BPLONG_PTR array;
    BPLONG len;
{
    BPLONG i,lst0;
    BPLONG_PTR ptr;

    lst0 = ADDTAG(heap_top,LST);
    FOLLOW(heap_top++) = MAKEINT(array[0]);
    ptr = heap_top++;
    for (i=1;i<len;i++){
        FOLLOW(ptr) = ADDTAG(heap_top,LST);
        FOLLOW(heap_top++) = MAKEINT(array[i]);
        ptr = heap_top++;
    }
    FOLLOW(ptr) = nil_sym;
    return lst0;
}

BPLONG termarray_to_termlist(array,len)
    BPLONG_PTR array;
    BPLONG len;
{
    BPLONG i,lst0;
    BPLONG_PTR ptr;

    lst0 = ADDTAG(heap_top,LST);
    FOLLOW(heap_top++) = array[0];
    ptr = heap_top++;
    for (i=1;i<len;i++){
        FOLLOW(ptr) = ADDTAG(heap_top,LST);
        FOLLOW(heap_top++) = array[i];
        ptr = heap_top++;
    }
    FOLLOW(ptr) = nil_sym;
    return lst0;
}

#define BP_ASORTED 1  /* < */
#define BP_DSORTED 2  /* > */
#define BP_NOT_SORTED 3

/* Scan the list and do the following:
   1. check if the list is already sorted (upward or downward)
   2. compute the length of the list
   3. copy the list's elements to array
*/
int bp_already_sorted_int_list(BPLONG lst, BPLONG_PTR len_ptr, BPLONG_PTR arr)
{
    BPLONG_PTR top,ptr;
    BPLONG elm,pre,lst0;
    int len,asorted,dsorted;

    //  printf("arr = %lx heap_top=%lx local_top = %lx\n",arr,heap_top,local_top);
    asorted = 1; dsorted = 1; 
    lst0 = lst;
    len = 0;
    DEREF(lst);
    if (ISLIST(lst)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        pre = FOLLOW(ptr); DEREF(pre);
        if (!ISINT(pre)) return BP_FALSE;
        pre = INTVAL(pre);
        FOLLOW(arr) = pre;
        len++;
        lst = FOLLOW(ptr+1); DEREF(lst);
    }

    while (ISLIST(lst)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        elm = FOLLOW(ptr); DEREF(elm);
        if (!ISINT(elm)) return BP_FALSE;
        elm = INTVAL(elm);
        FOLLOW(arr+len) = elm;
        if (arr+len >= local_top) return BP_FALSE; /* list is too big */
        len++;
        if (pre>elm) asorted = 0;
        if (pre<elm) dsorted = 0;
        pre = elm;
        lst = FOLLOW(ptr+1); DEREF(lst);
    }
    if (!ISNIL(lst)){
        if (ISREF(lst)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_LIST,lst0);
        }
        return BP_ERROR;
    }

    FOLLOW(len_ptr) = len;

    if (asorted == 1){
        return BP_ASORTED;
    } else if (dsorted == 1){
        return BP_DSORTED;
    } else {
        return BP_NOT_SORTED;
    }
}

/* Test if lst is an already-sorted list and compute lst's length. */
int bp_already_sorted_term_list(BPLONG lst, BPLONG_PTR len_ptr, BPLONG_PTR arr)
{
    BPLONG_PTR top,ptr;
    BPLONG elm,pre,lst0;
    int len,asorted,dsorted;

    len = 0;
    lst0 = lst;
    asorted = 1; dsorted = 1;
    DEREF(lst);
    if (ISLIST(lst)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        pre = FOLLOW(ptr); 
        FOLLOW(arr) = pre;
        len++;
        lst = FOLLOW(ptr+1); DEREF(lst);
    }
    while (ISLIST(lst)){
        int res;
    
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        elm = FOLLOW(ptr); 
        res = bp_compare(pre,elm);
        if (res>0){
            asorted = 0;
        } else if (res<0){
            dsorted = 0;
        }
        FOLLOW(arr+len) = elm;
        len++;
        if (arr+len >= local_top) return BP_FALSE; /* list is too big */
        pre = elm;
        lst = FOLLOW(ptr+1); DEREF(lst);
    }
    if (!ISNIL(lst)){
        if (ISREF(lst)){
            exception = et_INSTANTIATION_ERROR;
        } else {
            exception = c_type_error(et_LIST,lst0);
        }
        return BP_ERROR;
    }
  
    FOLLOW(len_ptr) = len;
    if (asorted == 1){
        return BP_ASORTED;
    } else if (dsorted == 1){
        return BP_DSORTED;
    } else {
        return BP_NOT_SORTED;
    }
}

///  quickSort
//
//  This public-domain C implementation by Darel Rex Finley.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7
// Improved by Neng-Fa Zhou for efficiently sorting arrays that contain duplicates.

#define  MAX_LEVELS  300
void qsort_int_array(BPLONG_PTR arr, BPLONG len) {
    BPLONG  piv;
    int beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap;

    beg[0]=0; end[0]=len;
    while (i >= 0) {
        L = beg[i]; R = end[i]-1;
        if (L < R) {
            swap = L + (R-L)/2;
            piv = arr[swap]; arr[swap] = arr[L]; arr[L] = piv;
            while (L < R) {
                while (L < R && arr[R] > piv) R--;
                arr[L++] = arr[R];
                while (L < R && arr[L] <= piv) L++;
                if (L < R) arr[R--] = arr[L]; 
            }
            arr[R] = piv; 
            L = R-1;
            while (L >= beg[i] && arr[L] == piv) L--;
            beg[i+1] = R+1; end[i+1] = end[i]; end[i++] = L+1;
            if (end[i]-beg[i] > end[i-1]-beg[i-1]) {
                swap = beg[i]; beg[i] = beg[i-1]; beg[i-1] = swap;
                swap = end[i]; end[i] = end[i-1]; end[i-1] = swap; 
            }
        } else {
            i--; 
        }
    }
}

void qsort_term_array(BPLONG_PTR arr, BPLONG len) {
    BPLONG  piv, beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap;

    beg[0] = 0; end[0] = len;
    while (i >= 0) {
        L = beg[i]; R = end[i]-1;
        if (L < R) {
            swap = L + (R-L)/2;
            piv = arr[swap]; arr[swap] = arr[L]; arr[L] = piv;
            while (L < R) {
                while (L < R && bp_compare(arr[R],piv) > 0) R--;
                arr[L++] = arr[R];
                while (L < R && bp_compare(arr[L],piv) <= 0) L++;
                if (L < R) arr[R--] = arr[L]; 
            }
            arr[R] = piv; 
            L = R-1;
            while (L >= beg[i] && arr[L] == piv) L--;  /* use ==, not bp_compare, is intentional */

            beg[i+1] = R+1; end[i+1] = end[i]; end[i++] = L+1;
            if (end[i]-beg[i] > end[i-1]-beg[i-1]) {
                swap = beg[i]; beg[i] = beg[i-1]; beg[i-1] = swap;
                swap = end[i]; end[i] = end[i-1]; end[i-1] = swap; 
            }
        } else {
            i--; 
        }
    }
}


BPLONG bp_reverse_list(lst)
    BPLONG lst;
{
    BPLONG_PTR top,ptr;
    BPLONG elm,rev_lst;
    rev_lst = nil_sym;
  
    DEREF(lst);
    while (ISLIST(lst)){
        BPLONG tmp_lst;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        elm = FOLLOW(ptr); DEREF(elm);
        tmp_lst = ADDTAG(heap_top,LST);
        NEW_HEAP_NODE(elm);
        NEW_HEAP_NODE(rev_lst);
        rev_lst = tmp_lst;
        lst = FOLLOW(ptr+1); DEREF(lst);
    }
    return rev_lst;
}

/* If the given list is a sorted integer list (either upward or downward), then return the sorted list;
   If the given list is an integer list that is not too long (depending on the available stack space),
   then use qsort; otherwise fail, doing nothing.
*/
int b_sort_int_list(lst,sorted_lst)
    BPLONG lst,sorted_lst;
{
    BPLONG len;
    BPLONG_PTR arr;

    arr = local_top-(local_top-heap_top)/3;  /* use 1/3 of the available area of the global stack */
  
    switch (bp_already_sorted_int_list(lst,&len,arr)){
    case BP_ASORTED:
        return unify(sorted_lst,lst);

    case BP_DSORTED:
        if (local_top-heap_top <= 2*len) return BP_FALSE; /* not enought space for temporary use */
        return unify(sorted_lst,bp_reverse_list(lst));

    case BP_ERROR:
        return BP_ERROR;

    case BP_FALSE: /* not an int list */
        return BP_FALSE;
    }

    /* come here the lst is an integer list, lst is not sorted, and lst is not too long */
    qsort_int_array(arr,len);
    return unify(sorted_lst,intarray_to_intlist(arr,len));
}

int c_sort_int_list(){
    return b_sort_int_list(ARG(1,2),ARG(2,2));
}

/* 
   If the given list is sorted (upward or downward), then return the list; 
   otherwise, if the given list is not too long, then use qsort;
   otherwise fail doing nothing 
*/
int c_sort_term_list(){
    BPLONG len,lst,sorted_lst;
    BPLONG_PTR arr;

    lst = ARG(1,2);
    sorted_lst = ARG(2,2);

    arr = local_top-(local_top-heap_top)/3;  /* use 1/3 of the available area of the global stack */
    switch (bp_already_sorted_term_list(lst,&len,arr)){
    case BP_ASORTED:
        return unify(sorted_lst,lst);
    
    case BP_DSORTED:
        return unify(sorted_lst,bp_reverse_list(lst));

    case BP_ERROR:
        return BP_ERROR;

    case BP_FALSE:
        return BP_FALSE;
    }

    /* come here if not sorted, and the list is not too long */
    qsort_term_array(arr,len);
    return unify(sorted_lst,termarray_to_termlist(arr,len));
}


int b_DEREF_c(T)
    BPLONG T;
{
    BPLONG_PTR ptr,top;
    BPLONG e;
  
    DEREF(T);
    if (ISSTRUCT(T)){
        SYM_REC_PTR sym_ptr;
        BPLONG i,n;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(T);
        sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
        n = GET_ARITY(sym_ptr);
        for (i=1;i<=n;i++){
            e = FOLLOW(ptr+i); DEREF(e);
            if (IS_SUSP_VAR(e)){
                e = UNTAGGED_ADDR(e);
            }
            b_DEREF_c(e);
            top = ptr+i;
            PUSHTRAIL_H_NONATOMIC(top,FOLLOW(top));
            FOLLOW(top) = e;
        }
    } else if (ISLIST(T)){
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(T);    
        e = FOLLOW(ptr); DEREF(e);
        if (IS_SUSP_VAR(e)){
            e = UNTAGGED_ADDR(e);
        }
        b_DEREF_c(e);
        PUSHTRAIL_H_NONATOMIC(ptr,FOLLOW(ptr));
        FOLLOW(ptr) = e;
        ptr++;
        e = FOLLOW(ptr); DEREF(e);
        if (IS_SUSP_VAR(e)){
            e = UNTAGGED_ADDR(e);
        }
        b_DEREF_c(e);
        PUSHTRAIL_H_NONATOMIC(ptr,FOLLOW(ptr));
        FOLLOW(ptr) = e;
    }
    return BP_TRUE;
}

int c_set_exception(){
    BPLONG op;
    op = ARG(1,1);
    exception = op;
    return BP_TRUE;
}

int b_IS_STRUCT_c(BPLONG term){
    SWITCH_OP_STRUCT(term,lab1,{return BP_FALSE;}, {return BP_TRUE;}, {return BP_FALSE;});
    return BP_FALSE;
}
  
int b_IS_COMPOUND_c(BPLONG term){
    DEREF(term);
    return (ISCOMPOUND(term)) ? BP_TRUE : BP_FALSE;
}

int b_IS_GROUND_c(BPLONG term){
    return check_ground_using_faa(term);
}

/* term is a list of atoms */
int b_IS_STRING_c(BPLONG term){
    int len;
    DEREF(term);
    while (ISLIST(term)){
        BPLONG c;
        BPLONG_PTR list_ptr;
        SYM_REC_PTR sym_ptr;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        term = FOLLOW(list_ptr+1); DEREF(term);
        c = FOLLOW(list_ptr); DEREF(c);
        if (!ISATOM(c)) return BP_FALSE;
        sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(c);
        len = GET_LENGTH(sym_ptr);
        if (len == 1 || ((len<=4) && (utf8_nchars(GET_NAME(sym_ptr)) == 1))){
            /* it is a char */
        } else {
            return BP_FALSE;
        }
    }
    if (term != nil_sym) return BP_FALSE;
    return BP_TRUE;
}

int b_IS_MAP_c(BPLONG term){
    SWITCH_OP_STRUCT(term,lab1,
                     {return BP_FALSE;}, 
                     {
                         SYM_REC_PTR sym = GET_STR_SYM_REC(term);
                         return (sym == hashtable_psc || sym == ghashtable_psc || sym == thashtable_psc) ? BP_TRUE : BP_FALSE;
                     }, 
                     {return BP_FALSE;});
    return BP_FALSE;
}

int b_IS_ARRAY_c(BPLONG term){  /* is Picat array */
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;

    SWITCH_OP_STRUCT(term,lab1,
                     {return BP_FALSE;}, 
                     {sym_ptr = (SYM_REC_PTR)GET_STR_SYM_REC(term);
                         char_ptr = GET_NAME(sym_ptr);
                         return (GET_LENGTH(sym_ptr) == 2 && *char_ptr == '{' && *(char_ptr+1) == '}') ? BP_TRUE : BP_FALSE;}, 
                     {return BP_FALSE;});
    return term == empty_set;
}

int b_IS_CHAR_c(BPLONG term){  
    SYM_REC_PTR sym_ptr;
    BPLONG len;

    DEREF(term);
    if (!ISATOM(term)) return BP_FALSE;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(term);
    len = GET_LENGTH(sym_ptr);
    if (len == 1 || (len<=4 && utf8_nchars(GET_NAME(sym_ptr)) == 1)){
        return BP_TRUE;
    } else {
        return BP_FALSE;
    }
}

int b_IS_DIGIT_c(BPLONG term){  
    SYM_REC_PTR sym_ptr;
    char c, *char_ptr;
    DEREF(term);
    if (!ISATOM(term)) return BP_FALSE;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(term);
    if (GET_LENGTH(sym_ptr) != 1) return BP_FALSE;
    char_ptr = (char *)GET_NAME(sym_ptr);
    c = *char_ptr;
    return (c>='0' && c<='9') ? BP_TRUE : BP_FALSE;
}

int b_IS_LOWERCASE_c(BPLONG term){  
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;

    DEREF(term);
    if (!ISATOM(term)) return BP_FALSE;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(term);
    if (GET_LENGTH(sym_ptr) != 1) return BP_FALSE;
    char_ptr = GET_NAME(sym_ptr);
    return (*char_ptr>='a' && *char_ptr<='z') ? BP_TRUE : BP_FALSE;
}

int b_IS_UPPERCASE_c(BPLONG term){  
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;

    DEREF(term);
    if (!ISATOM(term)) return BP_FALSE;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(term);
    if (GET_LENGTH(sym_ptr) != 1) return BP_FALSE;
    char_ptr = GET_NAME(sym_ptr);
    return (*char_ptr>='A' && *char_ptr<='Z') ? BP_TRUE : BP_FALSE;
}

int b_IS_LIST_c(BPLONG term){  
    SWITCH_OP_LST(term,lab1,
                  {return BP_FALSE;}, 
                  {return BP_TRUE;},
                  {return BP_FALSE;});
    return (term == nil_sym) ? BP_TRUE : BP_FALSE;
}

int b_STRUCT_ARITY_cf(BPLONG term, BPLONG arity){
    SWITCH_OP_STRUCT(term,lab1,
                     {return BP_FALSE;}, 
                     {SYM_REC_PTR sym_ptr;
                         sym_ptr = (SYM_REC_PTR)GET_STR_SYM_REC(term);
                         ASSIGN_f_atom(arity,MAKEINT(GET_ARITY(sym_ptr)));
                         return BP_TRUE;
                     },
                     {return BP_FALSE;});
    return BP_FALSE;
}

int b_STRUCT_NAME_cf(BPLONG term, BPLONG name){
    SWITCH_OP_STRUCT(term,lab1,
                     {return BP_FALSE;}, 
                     {SYM_REC_PTR sym_ptr;
                         sym_ptr = (SYM_REC_PTR)GET_STR_SYM_REC(term);
                         ASSIGN_f_atom(name,ADDTAG(insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),0),ATM));
                         return BP_TRUE;
                     },
                     {return BP_FALSE;});
    return BP_FALSE;
}

int b_NEW_STRUCT_ccf(BPLONG name, BPLONG arity, BPLONG term){
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR ptr;

    DEREF(name);
    if (!ISATOM(name)) return BP_FALSE;
    DEREF(arity);
    if (!ISINT(arity)) return BP_FALSE;
    arity = INTVAL(arity);
    if (arity<=0 || arity>MAX_ARITY) return BP_FALSE;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(name);
    FOLLOW(heap_top) = (BPLONG)insert_sym(GET_NAME(sym_ptr),GET_LENGTH(sym_ptr),arity);
    ASSIGN_v_heap_term(term, ADDTAG(heap_top,STR));
    heap_top++;
    ptr = heap_top;
    heap_top += arity;
    LOCAL_OVERFLOW_CHECK("new_struct");
    while (ptr<heap_top){
        FOLLOW(ptr) = (BPLONG)ptr;
        ptr++;
    }
    return BP_TRUE;
}

int c_getpid(){
    BPLONG op = ARG(1,1);
    return unify(op,MAKEINT(sys_getpid()));
}

void Cboot_mic()
{
    greater_than_sym = ADDTAG(BP_NEW_SYM(">",0),ATM);
    less_than_sym = ADDTAG(BP_NEW_SYM("<",0),ATM);
    equal_sym = ADDTAG(BP_NEW_SYM("=",0),ATM);

    insert_cpred("getpid",1,c_getpid);
    insert_cpred("c_OS_TYPE_f",1,c_OS_TYPE_f);
    insert_cpred("c_get_main_args",1,c_get_main_args);
    insert_cpred("c_chdir",1,c_chdir);
    insert_cpred("getcwd",1,c_get_cwd);
    insert_cpred("c_UNDERSCORE_NAME",1,c_UNDERSCORE_NAME);
    /*  insert_cpred("c_ANCESTOR_FRAME",1,c_ANCESTOR_FRAME); */
    insert_cpred("c_set_debugging_susp",0,c_set_debugging_susp);
    insert_cpred("c_unset_debugging_susp",0,c_unset_debugging_susp);
#ifdef GC
    insert_cpred("c_set_gc_threshold",1,c_set_gc_threshold);
#endif
    insert_cpred("c_WDAY_f",1,c_WDAY_f);
    insert_cpred("c_TIME_ffffff",6,c_TIME_ffffff);
    insert_cpred("c_GETENV_cf",2,c_GETENV_cf);
    insert_cpred("atom_2_term",3,atom_2_term);
    insert_cpred("string_2_term",3,string_2_term);
    insert_cpred("term2atom",2,term_2_atom);
    insert_cpred("term2string",2,term_2_string);
    insert_cpred("ref_equal",2,c_ref_equal);
    insert_cpred("cg_is_component",1,c_cg_is_component);
    insert_cpred("c_post_event",3,c_post_event);
    insert_cpred("c_timer",1,c_timer); 
    insert_cpred("c_kill_timer",1,c_kill_timer); 
    insert_cpred("c_sleep",1,c_sleep); 
    /**/
    insert_cpred("c_FORMAT_PRINT_INTEGER",3,c_FORMAT_PRINT_INTEGER); 
    insert_cpred("c_FORMAT_PRINT_FLOAT",3,c_FORMAT_PRINT_FLOAT); 
    /**/
    insert_cpred("c_findall_pre",1,c_findall_pre); 
    insert_cpred("c_findall_post",1,c_findall_post); 
    insert_cpred("c_FINDALL_GET",2,c_FINDALL_GET);
    insert_cpred("c_FINDALL_AREA_SIZE",1,c_FINDALL_AREA_SIZE);
    insert_cpred("c_global_set_bpp",1,c_global_set_bpp); 
    insert_cpred("c_global_get_bpp",1,c_global_get_bpp); 
    insert_cpred("c_confirm_copy_right",0,c_confirm_copy_right); 
    insert_cpred("c_NEXT_PRIME",2,c_NEXT_PRIME); 
    insert_cpred("c_GET_REDEFINE_WARNING",1,c_GET_REDEFINE_WARNING); 
    insert_cpred("c_SET_REDEFINE_WARNING",1,c_SET_REDEFINE_WARNING); 
    insert_cpred("c_LOAD_cfc",3,c_LOAD_cfc); 
    insert_cpred("file_stat",2,file_stat); 
    insert_cpred("c_file_type",2,c_file_type); 
    insert_cpred("c_file_permission",2,c_file_permission); 
    insert_cpred("c_directory_files",2,c_directory_list); 
    insert_cpred("make_directory",1,c_mkdir); 
    insert_cpred("delete_directory",1,c_rmdir); 
    insert_cpred("delete_file",1,c_rm_file); 
    insert_cpred("copy_file",2,c_cp_file); 
    insert_cpred("rename_file",2,c_rename); 
    insert_cpred("c_sort_int_list",2,c_sort_int_list);
    insert_cpred("$start_critical_region",0,c_start_critical_region);
    insert_cpred("$end_critical_region",0,c_end_critical_region);
    insert_cpred("$in_critical_region",0,c_in_critical_region);
    insert_cpred("c_set_exception",1,c_set_exception);
    insert_cpred("c_sort_term_list",2,c_sort_term_list);
}

#if defined(_MSC_VER)
#define FORCE_INLINE    __forceinline
#else    // defined(_MSC_VER)
#define    FORCE_INLINE __attribute__((always_inline))
#endif

#define    ROTL32(x,r) (x << r) | (x >> (32 - r))

#if defined(_MSC_VER)
FORCE_INLINE UW32 fmix ( UW32 h )
#else
    FORCE_INLINE inline UW32 fmix ( UW32 h )
#endif
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

UW32 MurmurHash3_x86_32_uint32( const UW32 key, UW32 seed)
{
    int i;
    UW32 h1 = seed;
    UW32 c1 = 0xcc9e2d51;
    UW32 c2 = 0x1b873593;
    UW32 k1 = key;
    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;
    
    h1 ^= k1;
    h1 = ROTL32(h1,13); 
    h1 = h1*5+0xe6546b64;

    h1 ^= 4;

    h1 = fmix(h1);

    return h1;
} 

int c_bp_exit(){
    BPLONG code = ARG(1,1);
    DEREF(code);
    if (!ISINT(code)){
        exception = illegal_arguments; 
        return BP_ERROR; 
    } 
    code = INTVAL(code);
    exit(code);
}

  
int b_PICAT_ARG_ccf(BPLONG Index,BPLONG Comp,BPLONG Arg)
{
    BPLONG_PTR arr_ptr;
    BPLONG res;
  
    SWITCH_OP_INT(Index,
                  lab_picat_arg_1,
                  {exception = et_INSTANTIATION_ERROR;
                      return BP_ERROR;
                  },
                  {Index = INTVAL(Index);
                      if (Index<=0){
                          exception = out_of_bound;
                          return BP_ERROR;
                      };
                      SWITCH_OP(Comp,
                                lab_picat_arg_2,
                                {exception = et_INSTANTIATION_ERROR;
                                    return BP_ERROR;
                                },
                                {exception = c_type_error(et_COMPOUND,Comp);
                                    return BP_ERROR;
                                },
                                {while (Index>1 && ISLIST(Comp)){
                                        arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp); 
                                        Comp = FOLLOW(arr_ptr+1);DEREF(Comp);
                                        Index--;
                                    } 
                                    if (Index == 1 && ISLIST(Comp)){
                                        arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp); 
                                        res = FOLLOW(arr_ptr);
                                        ASSIGN_v_heap_term(Arg, res);
                                        return BP_TRUE;
                                    } else {
                                        exception = out_of_bound;
                                        return BP_ERROR;
                                    }
                                },
                                {BPLONG arity;
                                    SYM_REC_PTR sym_ptr;
                                    arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp);
                                    sym_ptr = (SYM_REC_PTR)FOLLOW(arr_ptr);
                                    arity = GET_ARITY(sym_ptr);
                                    if (Index>arity){
                                        exception = out_of_bound;
                                        return BP_ERROR;
                                    }
                                    res = FOLLOW(arr_ptr+Index);
                                    ASSIGN_v_heap_term(Arg, res);
                                    return BP_TRUE;
                                },
                                {exception = et_INSTANTIATION_ERROR;
                                    return BP_ERROR;
                                })
                          },
                  {exception = c_type_error(et_INTEGER,Index);
                      return BP_ERROR;
                  });
    return BP_FALSE;
}

int b_PICAT_SETARG_ccc(BPLONG Index,BPLONG Comp,BPLONG Arg){
    BPLONG_PTR arr_ptr;
  
    DEREF(Arg);
    if (ISREF(Arg)){
        if ((BPLONG_PTR)Arg>heap_top){ /* globalize the variable */
            PUSHTRAIL_s(Arg);
            FOLLOW(Arg) = (BPLONG)heap_top;
            FOLLOW(heap_top) = (BPLONG)heap_top;
            heap_top++;
        }
    }
    SWITCH_OP_INT(Index,
                  lab_picat_arg_1,
                  {exception = et_INSTANTIATION_ERROR;
                      return BP_ERROR;
                  },
                  {Index = INTVAL(Index);
                      if (Index<=0){
                          exception = out_of_bound;
                          return BP_ERROR;
                      };
                      SWITCH_OP(Comp,
                                lab_picat_arg_2,
                                {exception = et_INSTANTIATION_ERROR;
                                    return BP_ERROR;
                                },
                                {exception = c_type_error(et_COMPOUND,Comp);
                                    return BP_ERROR;
                                },
                                {while (Index>1 && ISLIST(Comp)){
                                        arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp); 
                                        Comp = FOLLOW(arr_ptr+1);DEREF(Comp);
                                        Index--;
                                    } 
                                    if (Index == 1 && ISLIST(Comp)){
                                        arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp); 
                                        if (!IS_HEAP_REFERENCE(arr_ptr)){
                                            exception = c_update_error(et_UPDATE);
                                            return BP_ERROR;
                                        }
                                        PUSHTRAIL_H_NONATOMIC(arr_ptr,FOLLOW(arr_ptr));
                                        FOLLOW(arr_ptr) = Arg;
                                    } else {
                                        exception = out_of_bound;
                                        return BP_ERROR;
                                    }
                                },
                                {BPLONG arity;
                                    SYM_REC_PTR sym_ptr;
                                    arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Comp);
                                    sym_ptr = (SYM_REC_PTR)FOLLOW(arr_ptr);
                                    arity = GET_ARITY(sym_ptr);
                                    if (Index>arity){
                                        exception = out_of_bound;
                                        return BP_ERROR;
                                    }
                                    arr_ptr = arr_ptr+Index;
                                    if (!IS_HEAP_REFERENCE(arr_ptr)){
                                        exception = c_update_error(et_UPDATE);
                                        return BP_ERROR;
                                    }
                                    PUSHTRAIL_H_NONATOMIC(arr_ptr,FOLLOW(arr_ptr));
                                    FOLLOW(arr_ptr) = Arg;
                                },
                                {exception = et_INSTANTIATION_ERROR;
                                    return BP_ERROR;
                                })
                          },
                  {exception = c_type_error(et_INTEGER,Index);
                      return BP_ERROR;
                  });
    return BP_TRUE;
}

/* insert t into lst and bind ret_lst to the resulting list */
int b_INSERT_ORDERED_ccf(BPLONG lst, BPLONG t, BPLONG ret_lst){
    BPLONG lst_cp;
    BPLONG_PTR tail_ptr = &lst_cp;

    DEREF(lst); 
    while (ISLIST(lst)){
        BPLONG_PTR list_ptr;
        BPLONG t1;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        t1 = FOLLOW(list_ptr); 
        if (bp_compare(t,t1) <= 0){
            break; /* exit the while loop */
        } else {
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            NEW_HEAP_NODE(t1);
            tail_ptr = heap_top++;
            lst = FOLLOW(list_ptr+1); DEREF(lst);
        }
    }
    if (!ISNIL(lst) && !ISLIST(lst)){
        exception = c_type_error(et_LIST,lst);
        return BP_ERROR;
    }
    FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(t);
    NEW_HEAP_NODE(lst);
    ASSIGN_v_heap_term(ret_lst,lst_cp);

    return BP_TRUE;
}

/* insert t into lst if t is not included in lst, and bind ret_lst to the resulting list */
int b_INSERT_ORDERED_NO_DUP_ccf(BPLONG lst, BPLONG t, BPLONG ret_lst){
    BPLONG lst_cp, lst0;
    BPLONG_PTR heap_top0;
    BPLONG_PTR tail_ptr = &lst_cp;
  
    DEREF(lst); 
    lst0 = lst; heap_top0 = heap_top;
    while (ISLIST(lst)){
        BPLONG_PTR list_ptr;
        BPLONG t1;
        int res;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        t1 = FOLLOW(list_ptr); 
        res = bp_compare(t,t1);
        if (res < 0){
            break; /* exit the while loop */
        } if (res == 0){
            ASSIGN_v_heap_term(ret_lst,lst0);
            heap_top = heap_top0;
            return BP_TRUE;
        } else {
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            NEW_HEAP_NODE(t1);
            tail_ptr = heap_top++;
            lst = FOLLOW(list_ptr+1); DEREF(lst);
        }
    }
    if (!ISNIL(lst) && !ISLIST(lst)){
        exception = c_type_error(et_LIST,lst);
        return BP_ERROR;
    }
    FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(t);
    NEW_HEAP_NODE(lst);
    ASSIGN_v_heap_term(ret_lst,lst_cp);

    return BP_TRUE;
}

/* insert t into lst and bind ret_lst to the resulting list */
int b_INSERT_ORDERED_DOWN_ccf(BPLONG lst, BPLONG t, BPLONG ret_lst){
    BPLONG lst_cp;
    BPLONG_PTR tail_ptr = &lst_cp;

    DEREF(lst); 
    while (ISLIST(lst)){
        BPLONG_PTR list_ptr;
        BPLONG t1;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        t1 = FOLLOW(list_ptr); 
        if (bp_compare(t,t1) >= 0){
            break; /* exit the while loop */
        } else {
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            NEW_HEAP_NODE(t1);
            tail_ptr = heap_top++;
            lst = FOLLOW(list_ptr+1); DEREF(lst);
        }
    }
    if (!ISNIL(lst) && !ISLIST(lst)){
        exception = c_type_error(et_LIST,lst);
        return BP_ERROR;
    }
    FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(t);
    NEW_HEAP_NODE(lst);
    ASSIGN_v_heap_term(ret_lst,lst_cp);

    return BP_TRUE;
}

/* insert t into lst if t is not included in lst, and bind ret_lst to the resulting list */
int b_INSERT_ORDERED_DOWN_NO_DUP_ccf(BPLONG lst, BPLONG t, BPLONG ret_lst){
    BPLONG lst_cp, lst0;
    BPLONG_PTR heap_top0;
    BPLONG_PTR tail_ptr = &lst_cp;

    DEREF(lst); 
    lst0 = lst; heap_top0 = heap_top;
    while (ISLIST(lst)){
        BPLONG_PTR list_ptr;
        BPLONG t1;
        int res;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        t1 = FOLLOW(list_ptr); 
        res = bp_compare(t,t1);
        if (res > 0){
            break; /* exit the while loop */
        } else if (res == 0) {
            ASSIGN_v_heap_term(ret_lst,lst0);
            heap_top = heap_top0;
            return BP_TRUE;
        } else {
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            NEW_HEAP_NODE(t1);
            tail_ptr = heap_top++;
            lst = FOLLOW(list_ptr+1); DEREF(lst);
        }
    }
    if (!ISNIL(lst) && !ISLIST(lst)){
        exception = c_type_error(et_LIST,lst);
        return BP_ERROR;
    }
    FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(t);
    NEW_HEAP_NODE(lst);
    ASSIGN_v_heap_term(ret_lst,lst_cp);

    return BP_TRUE;
}

/* insert t into the state list and bind ret_lst to the resulting list */
int b_INSERT_STATE_LIST_ccf(BPLONG lst, BPLONG t, BPLONG ret_lst){
    BPLONG lst_cp;
    BPLONG_PTR tail_ptr = &lst_cp;
  
    /*
      printf("=>insert "); write_term(lst); printf("\n");
      printf("         "); write_term(t); printf("\n");
    */
    DEREF(lst); 
    while (ISLIST(lst)){
        BPLONG_PTR list_ptr;
        BPLONG t1;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        t1 = FOLLOW(list_ptr); 
        if (bp_compare(t,t1) <= 0){
            goto real_insert_elm;
        } else {
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            NEW_HEAP_NODE(t1);
            tail_ptr = heap_top++;
            lst = FOLLOW(list_ptr+1); DEREF(lst);
        }
    }
    if (!ISNIL(lst)){
        exception = c_type_error(et_LIST,lst);
        return BP_ERROR;
    }
real_insert_elm:
    FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
    NEW_HEAP_NODE(t);
    NEW_HEAP_NODE(lst);
    ASSIGN_v_heap_term(ret_lst,lst_cp);
    return BP_TRUE;
}

  
  
