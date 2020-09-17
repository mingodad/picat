/********************************************************************
 *   File   : sat_bp.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020
 *   Purpose: interface between B-Prolog and GLPK

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include  <stdlib.h>
#ifdef GLPK
#include "glpk.h"
#include "bprolog.h"
extern double floatval();

#define GLP_SET_OBJ_COEF(lp, i, Pval, deft) {                   \
        if (ISREF(Pval)) {                                      \
            glp_set_obj_coef(lp, i, deft);                      \
        } else if (ISINT(Pval)) {                               \
            glp_set_obj_coef(lp, i, (double)INTVAL(Pval));      \
        } else {                                                \
            glp_set_obj_coef(lp, i, floatval(Pval));            \
        }                                                       \
    }

#define GLP_SET_COL_BOUNDS(lp, i, lower, upper) {                       \
        if (ISREF(lower)) {                                             \
            dob_val = 0.0;                                              \
        } else if (ISINT(lower)) {                                      \
            dob_val = (double)INTVAL(lower);                            \
        } else {                                                        \
            dob_val = floatval(lower);                                  \
        };                                                              \
        if (ISREF(upper)) {                                             \
            glp_set_col_bnds(lp, i, GLP_LO, dob_val, 0.0);              \
        } else if (ISINT(upper)) {                                      \
            glp_set_col_bnds(lp, i, GLP_DB, dob_val, (double)INTVAL(upper)); \
        } else {                                                        \
            glp_set_col_bnds(lp, i, GLP_DB, dob_val, floatval(upper));  \
        }                                                               \
    }

#define FREE_IF_NOT_NULL(ptr) if (ptr != NULL) free(ptr)

#define RELEASE_SOL_SPACE {                     \
        FREE_IF_NOT_NULL(x);                    \
    }

#define RELEASE_PROB_SPACE {                    \
        FREE_IF_NOT_NULL(rn);                   \
        FREE_IF_NOT_NULL(cn);                   \
        FREE_IF_NOT_NULL(rmatval);              \
}

int glpk_error(error_msg)
    char *error_msg;
{
    fprintf(stderr, "GLPK ERROR: %s\n ", error_msg);
    return BP_FALSE;
}

void dummy_print(const char *fmt, ...) {};

int c_lp_solve() {
    BPLONG NCols, NRows, NNZs, VarAttrs, ObjVector, ObjSense, Constrs, Sol;
    BPLONG_PTR top, ptr;
    BPLONG temp, lst;

    glp_prob *lp;
    int i, j, row;
    int status;
    int is_mip = 0;

    int *rn, *cn;
    double *rmatval, *x;
    double dob_val;


    NCols = ARG(1, 8); DEREF_NONVAR(NCols); NCols = INTVAL(NCols);
    NRows = ARG(2, 8); DEREF_NONVAR(NRows); NRows = INTVAL(NRows);
    NNZs = ARG(3, 8); DEREF_NONVAR(NNZs); NNZs = INTVAL(NNZs);
    VarAttrs = ARG(4, 8); DEREF_NONVAR(VarAttrs);
    ObjVector = ARG(5, 8); DEREF_NONVAR(ObjVector);
    ObjSense = ARG(6, 8); DEREF_NONVAR(ObjSense); ObjSense = INTVAL(ObjSense);
    Constrs = ARG(7, 8); DEREF_NONVAR(Constrs);
    Sol = ARG(8, 8);

    /* Create the problem. */
    lp = glp_create_prob();
    if ( lp == NULL ) {
        return glpk_error("lpx_create_prob()");
    }

    rn = (int *)malloc(sizeof(int)*(NNZs+1));
    cn = (int *)malloc(sizeof(int)*(NNZs+1));
    rmatval = (double *)malloc(sizeof(double)*(NNZs+1));

    if (rn == NULL ||
        cn == NULL ||
        rmatval == NULL) {
        RELEASE_PROB_SPACE;
        return glpk_error("No enough memory (GLPK)");
    }

    glp_add_rows(lp, NRows);
    glp_add_cols(lp, NCols);

    if (ObjSense == 0)
        glp_set_obj_dir(lp, GLP_MIN);
    else
        glp_set_obj_dir(lp, GLP_MAX);

    /* coefficients of the objective expression */
    //  printf("ObjSense=%d\n",ObjSense);
    //  write_term(ObjVector);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(ObjVector);
    for (i = 1; i <= NCols; i++) {
        temp = FOLLOW(ptr+i);
        DEREF(temp);
        GLP_SET_OBJ_COEF(lp, i, temp, 0.0);
        //      printf("obj[%d]=",i);write_term(temp);printf("\n");
    }

    /* lower and upper bounds */
    i = 1;
    lst = VarAttrs;
    while (ISLIST(lst)) {
        BPLONG lower, upper, VarAttr, varType;

        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        VarAttr = FOLLOW(ptr); DEREF_NONVAR(VarAttr);
        lst = FOLLOW(ptr+1); DEREF_NONVAR(lst);

        ptr = (BPLONG_PTR)UNTAGGED_ADDR(VarAttr);  /* $cplex_x(ColNo,Type,L,U,Constrs) */
        lower = FOLLOW(ptr+3); DEREF(lower);
        upper = FOLLOW(ptr+4); DEREF(upper);

        GLP_SET_COL_BOUNDS(lp, i, lower, upper);
        //      printf("col[%d] ",i); write_term(lower); printf(" "); write_term(upper); printf("\n");

        varType = FOLLOW(ptr+2); DEREF(varType);
        if (!ISREF(varType)) {
            is_mip = 1;
        }

        i++;
    }

    if (is_mip) {
        /*    lpx_set_class(lp,GLP_MIP); */

        i = 1;
        lst = VarAttrs;
        while (ISLIST(lst)) {
            BPLONG VarAttr, varType;

            ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
            VarAttr = FOLLOW(ptr); DEREF_NONVAR(VarAttr);
            lst = FOLLOW(ptr+1); DEREF_NONVAR(lst);

            ptr = (BPLONG_PTR)UNTAGGED_ADDR(VarAttr);  /* $cplex_x(ColNo,Type,L,U,Constrs) */
            varType = FOLLOW(ptr+2); DEREF(varType);
            if (!ISREF(varType)) {
                glp_set_col_kind(lp, i, GLP_IV);
            }
            i++;
        }
    }
    /*
      printf("constrs:");
      write_term(Constrs);
      printf("\n");
    */

    /* Now add the constraints */
    i = 1;  /* next nonzero to be put at index i */
    row = 1;  /* the current row number */
    while (ISLIST(Constrs)) {
        BPLONG CoeCols, Constr, Const, Coe, Col, ConstrType, pair;

        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Constrs);
        Constr = FOLLOW(ptr); DEREF_NONVAR(Constr);

        //      printf("constr:"); write_term(Constr); printf("\n");

        Constrs = FOLLOW(ptr+1); DEREF_NONVAR(Constrs);

        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Constr);  /* $cplex_c(NNZs,CoeCols,Const,ConstrType) */
        CoeCols = FOLLOW(ptr+2); DEREF_NONVAR(CoeCols);
        Const = FOLLOW(ptr+3); DEREF_NONVAR(Const);
        ConstrType = FOLLOW(ptr+4); DEREF_NONVAR(ConstrType); ConstrType = INTVAL(ConstrType);

        dob_val = (ISINT(Const)) ? (double)INTVAL(Const) : floatval(Const);
        if (ConstrType == 0)
            glp_set_row_bnds(lp, row, GLP_FX, dob_val, dob_val);
        else
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, dob_val);

        while (ISLIST(CoeCols)) {  /* [(Coe1,Col1),...,(ColN,ColN)] */
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(CoeCols);
            pair = FOLLOW(ptr); DEREF_NONVAR(pair);
            CoeCols = FOLLOW(ptr+1); DEREF_NONVAR(CoeCols);

            ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);  /* (Coe,Col) */
            Col = FOLLOW(ptr+2); DEREF_NONVAR(Col);

            rn[i] = row; cn[i] = (INTVAL(Col)+1);

            Coe = FOLLOW(ptr+1); DEREF_NONVAR(Coe);
            dob_val = (ISINT(Coe)) ? (double)INTVAL(Coe) : floatval(Coe);
            rmatval[i] = -dob_val;

            i++;
        }
        row++;
    }
    /*
      printf("lp=%x\n",lp);
      for (i=1;i<=NNZs;i++){
      printf("rn[%d]=%d\n",i,rn[i]);
      printf("cn[%d]=%d\n",i,cn[i]);
      printf("a[%d]=%f\n",i,rmatval[i]);
      }
    */
    glp_load_matrix(lp, NNZs, rn, cn, rmatval);

    //  printf("=>simplex\n");

    status = lpx_simplex(lp);
    if (status != LPX_E_OK) goto exit_glpk;
    if (is_mip) {
        status = lpx_integer(lp);
    }

exit_glpk:
    // printf("exit status = %d\n",status);
    if (status != LPX_E_OK) {
        RELEASE_PROB_SPACE;
        switch (status) {
        case LPX_E_FAULT:
            return glpk_error("unable to start search");
        case LPX_E_OBJLL:
        case LPX_E_OBJUL:
            return glpk_error("the objective function has reached its limit");
        case LPX_E_ITLIM:
            return glpk_error("Simplex has reached its iterations limit");
        case LPX_E_TMLIM:
            return glpk_error("Time limit has been reached");
        default:
            // return glpk_error("solver failure");
            return BP_FALSE;
        }
    }

    x = (double *) malloc (NCols * sizeof(double));

    if ( x == NULL) {
        RELEASE_PROB_SPACE;
        return glpk_error("No enough memory (GLPK)");
    }

    lst = nil_sym;
    for (j = NCols; j >= 1; j--) {
        BPLONG int_val;
        if (lpx_get_col_kind(lp, j) == LPX_CV) {
            dob_val = glp_get_col_prim(lp, j);
            temp = encodefloat1(dob_val);
        } else {
            int_val = lpx_mip_col_val(lp, j);
            temp = MAKEINT(int_val);
        }
        FOLLOW(heap_top) = temp;
        FOLLOW(heap_top+1) = lst;
        lst = ADDTAG(heap_top, LST);
        heap_top += 2;
    }
    /* write_term(lst);  */
    unify(lst, Sol);

    RELEASE_PROB_SPACE;
    RELEASE_SOL_SPACE;
    glp_delete_prob(lp);
    return BP_TRUE;
}

void Cboot_glpk() {
    insert_cpred("c_lp_solve", 8, c_lp_solve);
}
#endif
