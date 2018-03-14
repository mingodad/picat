/********************************************************************
 *   File   : sat_bp.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018
 *   Purpose: SAT interface for B-Prolog and Picat

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "bprolog.h"

#ifdef SAT
#include "cadical/src/ccadical.h"
static CCaDiCaL *sat_solver;

#define SAT_INIT sat_solver = ccadical_init()
#define SAT_ADD_LIT(lit) ccadical_add(sat_solver, lit)
#define SAT_START_SOLVER res = ccadical_sat(sat_solver)
#define SAT_SATISFIABLE res == 10
#define SAT_GET_BINDING(varNum) ((ccadical_deref(sat_solver, varNum) > 0) ? BP_ONE : BP_ZERO)
#endif

static int sat_dump_flag = 0;
static int sat_count_flag = 0;
static int num_cls = 0;
static int sat_var_num;

int b_SAT_GET_INC_VAR_NUM_f(BPLONG Num){
    ASSIGN_f_atom(Num,MAKEINT(sat_var_num));
    sat_var_num++;
    return BP_TRUE;
}

int c_sat_start_dump(){
    sat_dump_flag = 1;
    return BP_TRUE;
}

int c_sat_stop_dump(){
    sat_dump_flag = 0;
    return BP_TRUE;
}

/* initialize the global variable number, and initialize counters */
int c_sat_start_count(){
    BPLONG num = ARG(1,1);
    DEREF_NONVAR(num);
    sat_var_num = (int)INTVAL(num);
  
    sat_dump_flag = 1;
    sat_count_flag = 1;
    num_cls = 0;
    return BP_TRUE;
}

int c_sat_stop_count(){
    sat_dump_flag = 0;
    sat_count_flag = 0;
    unify(ARG(1,1), MAKEINT(num_cls));
    num_cls = 0;
    return BP_TRUE;
}

#ifdef SAT
/* cl is a list of literals */
int b_SAT_ADD_CL_c(BPLONG cl){
    BPLONG lit;
    BPLONG cl0 = cl;

    DEREF_NONVAR(cl0);
    cl = cl0;

    //  printf(" => add_cl "); write_term(cl); printf("\n");
        
    /* skip this clause if it contains 't' */
    while (ISLIST(cl)){
        BPLONG_PTR lst_ptr;
        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl);
        lit = FOLLOW(lst_ptr); DEREF_NONVAR(lit); 
        if (lit == t_atom){
            return BP_TRUE;
        }
        cl = FOLLOW(lst_ptr+1); DEREF_NONVAR(cl);
    }

    cl = cl0;
    if (sat_dump_flag) {
        while (ISLIST(cl)){
            BPLONG_PTR lst_ptr;
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl);
            lit = FOLLOW(lst_ptr); DEREF_NONVAR(lit); 
            cl = FOLLOW(lst_ptr+1); DEREF_NONVAR(cl);
            if (lit != f_atom){
                write_term(lit); 
                write_space();
            }
        }
        num_cls++;
        write_term(BP_ZERO);
        b_NL();
    } else {
        while (ISLIST(cl)){
            BPLONG_PTR lst_ptr;
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl);
            lit = FOLLOW(lst_ptr); DEREF_NONVAR(lit); 
            cl = FOLLOW(lst_ptr+1); DEREF_NONVAR(cl);
            if (lit != f_atom){   /* skip the literal 'f' */
                lit = INTVAL(lit);
                // printf("%d ",lit);
                SAT_ADD_LIT(lit);
            }
        }
        /*
          if (!ISNIL(cl)){
          exception = c_type_error(et_LIST,cl);
          return BP_ERROR;
          }
        */
        // printf("\n");
        SAT_ADD_LIT(0); 
    }
        
    return BP_TRUE;
}

int c_sat_init(){
    BPLONG num = ARG(1,1);

    DEREF_NONVAR(num);
    sat_var_num = (int)INTVAL(num);
  
    SAT_INIT;
    return BP_TRUE;
}

int c_sat_start(){
    BPLONG lst,res;
    BPLONG_PTR top;

    lst = ARG(1,1);
    DEREF_NONVAR(lst); 

    //  printf("=>sat_start "); write_term(lst); printf("\n");
        
    SAT_START_SOLVER;

    //  printf("<= solver\n");
        
    if (SAT_SATISFIABLE){
        BPLONG_PTR ptr;
        BPLONG var,varNum;

        while (ISLIST(lst)){
            BPLONG_PTR sv_ptr;
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
            var = FOLLOW(ptr); DEREF(var);
            if (IS_SUSP_VAR(var)){
                sv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
                varNum = fast_get_attr(sv_ptr,et_NUMBER);
                DEREF(varNum);
                varNum = INTVAL(varNum);
                unify(var,SAT_GET_BINDING(varNum));
            }
            lst = FOLLOW(ptr+1); DEREF(lst);
        }
        return BP_TRUE;
    } 
    return BP_FALSE;
}

void Cboot_sat(){
    insert_cpred("c_sat_init",1,c_sat_init);
    insert_cpred("c_sat_start",1,c_sat_start);
}
#else
int c_sat_init(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}

int c_sat_start(){
    printf("SAT not supported for MVC. Use the cygwin version.\n");
    return BP_FALSE;
}
void Cboot_sat(){
    insert_cpred("c_sat_init",0,c_sat_init);
    insert_cpred("c_sat_start",1,c_sat_start);
}

#endif

/* 
   If BV is a variable, then return the number attribute attached to BV.
   BV can be the negation of another variable '$\\'(BV1) or a constant (0 or 1).
   If BV=0, then Num=f; if BV=1; then Num=t 
*/
int b_SAT_RETRIEVE_BNUM_cff(BPLONG BV, BPLONG Num, BPLONG MNum){
lab_start:
    DEREF_NONVAR(BV);
    //  printf("=> BNUM %x ",BV); write_term(BV); printf("\n");
  
    if (ISINT(BV)){
        if (BV == BP_ONE){
            ASSIGN_f_atom(Num,t_atom);
            ASSIGN_f_atom(MNum,f_atom);
        } else {
            ASSIGN_f_atom(Num,f_atom);
            ASSIGN_f_atom(MNum,t_atom);
        }
    } else if (IS_SUSP_VAR(BV)){
        BPLONG varNum;
        BPLONG_PTR sv_ptr;
        
        sv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(BV);
        varNum = fast_get_attr(sv_ptr,et_NUMBER);
        DEREF_NONVAR(varNum);
        ASSIGN_f_atom(Num,varNum);
        ASSIGN_f_atom(MNum,MAKEINT(-(INTVAL(varNum))));
    } else {   /* it must be  '$\\'(B) */
        BPLONG_PTR ptr;
        BPLONG tmp;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(BV);
        BV = FOLLOW(ptr+1);
        tmp = Num; Num = MNum; MNum = tmp;
        goto lab_start;
    }
    //  printf("<= BNUM %x ",BV); write_term(Num); printf(" "); write_term(MNum); printf("\n");
    return BP_TRUE;
}

/*
  Let LogBitVect= <X(n-1),...,X1,X0>. For each bit position i, if all the values
  in X's domain have the same bit value B in the position, then set Xi = B.
*/
int c_sat_propagate_dom_bits(){
    BPLONG X, LogBitVect;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR dv_ptr, vect_ptr;
    int i, elm, last, n;
	int mark_vect[64];   /* the number of bits can't exceed 64 */

    X = ARG(1,2);
    LogBitVect = ARG(2,2);
  
    DEREF_NONVAR(X);
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
  
    DEREF_NONVAR(LogBitVect);
    vect_ptr = (BPLONG_PTR)UNTAGGED_ADDR(LogBitVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(vect_ptr);
    n = GET_ARITY(sym_ptr);

	//	printf("=> c_sat_propagate_dom_bits (%d)", n); write_term(X); printf(" "); write_term(LogBitVect); printf("\n");

    vect_ptr++;                  /* the vector changes from 1-based to 0-based */
    for (i = 0; i < n; i++){
	  mark_vect[i] = 0;
    }
  
    elm = DV_first(dv_ptr);
    last = DV_last(dv_ptr);
    for (;;){
        int val;

        val = (elm >= 0) ? elm : -elm;
        for (i = 0; i < n; i++){
            mark_vect[i] = (mark_vect[i] | ((val%2 == 0) ? 2 : 1));   // mark if this bit is 0 or 1
            val >>= 1;
        }

        if (elm == last) break;
        elm++;
        if (!IS_IT_DOMAIN(dv_ptr)) elm = domain_next_bv(dv_ptr,elm); 
    }

    for (i = 0; i < n; i++){
        int flag = mark_vect[i];
        if (flag == 1) {                    // all the domain values have 1 in ith position
            if (!unify(*(vect_ptr+i), BP_ONE)) return BP_FALSE;
        } else if (flag == 2) {             // all the domain values have 0 in ith position
            if (!unify(*(vect_ptr+i), BP_ZERO)) return BP_FALSE;
        }
    }
	//	printf("<= c_sat_propagate_dom_bits (%d)", n); write_term(X); printf(" "); write_term(LogBitVect); printf("\n");
    return BP_TRUE;
}



