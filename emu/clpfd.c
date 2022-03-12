/********************************************************************
 *   File   : clpfd.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2022
 *   Purpose: Primitives on domain variables and constraints

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include "bprolog.h"
#include "event.h"
#include "clpfd.h"
#include "frame.h"
#define CALL_DOMAIN_PREV(dv_ptr, elm, prev) {           \
        if (IS_IT_DOMAIN(dv_ptr))                       \
            prev = elm-1;                               \
        else                                            \
            prev = domain_prev_bv(dv_ptr, elm-1);       \
    }

#define CALL_DOMAIN_NEXT(dv_ptr, elm, next) {           \
        if (IS_IT_DOMAIN(dv_ptr))                       \
            next = elm+1;                               \
        else                                            \
            next = domain_next_bv(dv_ptr, elm+1);       \
    }

#define UNIFY_DVAR_VAL(B, val) {                                \
        if (IS_SUSP_VAR(B)) {                                   \
            dv_ptr_b = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(B);      \
            ASSIGN_DVAR(dv_ptr_b, val);                         \
            return BP_TRUE;                                     \
        } else {                                                \
            return B == val;                                    \
        }                                                       \
    }

int dvar_bv(op)
    BPLONG op;
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    DEREF(op);
    if (IS_SUSP_VAR(op)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
        if (IS_BV_DOMAIN(dv_ptr)) return 1;
    }
    return 0;
}

int dvar_excludable_or_int(op)
    BPLONG op;
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    DEREF(op);
    if (ISINT(op)) return 1;
    if (IS_SUSP_VAR(op)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
        if (IS_UN_DOMAIN(dv_ptr) ||
            IS_IT_DOMAIN(dv_ptr) && !IS_SMALL_DOMAIN(dv_ptr)) return 0;
        return 1;
    }
    return 0;
}

int b_EXCLUDABLE_LIST_c(list)
    BPLONG list;
{
    BPLONG_PTR ptr, top;
    BPLONG op;
    DEREF(list);
    while (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        op = FOLLOW(ptr);
        if (!dvar_excludable_or_int(op)) return BP_FALSE;
        list = FOLLOW(ptr+1);
        DEREF(list);
    }
    if (ISNIL(list)) return BP_TRUE;
    bp_exception = illegal_arguments;
    return BP_ERROR;
}

int nondvar(op)
    BPLONG op;
{
    return (dvar(op) ? 0 : 1);
}

int dvar(op)
    BPLONG op;
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    DEREF(op);
    if (IS_SUSP_VAR(op)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
        if (!IS_UN_DOMAIN(dv_ptr)) return 1;
    }
    return 0;
}

int dvar_or_int(op)
    BPLONG op;
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    DEREF(op);
    if (IS_SUSP_VAR(op)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
        if (!IS_UN_DOMAIN(dv_ptr)) return 1;
    } else if (ISINT(op))
        return 1;
    return 0;
}

/* check while counting. Return -1 immediately if the count exceeds limit. */
int n_vars_gt(count0, op, limit)
    BPLONG count0, op, limit;
{
    BPLONG i, arity;
    BPLONG_PTR top;

    DEREF(op);
    SWITCH_OP(op, n_n_vars_gt,
              {count0++; if (count0 > limit) return -1;},

              {},

              {UNTAG_ADDR(op);
                  if ((count0 = n_vars_gt(count0, FOLLOW(op), limit)) == -1) return -1;
                  if ((count0 = n_vars_gt(count0, FOLLOW((BPLONG_PTR)op+1), limit)) == -1) return -1;},

              {UNTAG_ADDR(op);
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op));
                  for (i = 1; i <= arity; i++)
                      if ((count0 = n_vars_gt(count0, FOLLOW(((BPLONG_PTR)op+i)), limit)) == -1) return -1;},

              {count0++; if (count0 > limit) return -1;});
    return count0;
}

int trigger_vars_ins(op)
    BPLONG op;
{
    BPLONG i, arity;
    BPLONG_PTR top, dv_ptr;

    SWITCH_OP(op, n_trigger_vars,
              {CREATE_SUSP_VAR_ins(op, arreg);},

              {},

              {UNTAG_ADDR(op);
                  trigger_vars_ins(FOLLOW(op));
                  trigger_vars_ins(FOLLOW((BPLONG_PTR)op+1));},

              {UNTAG_ADDR(op);
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op));
                  for (i = 1; i <= arity; i++)
                      trigger_vars_ins(FOLLOW(((BPLONG_PTR)op+i)));},

              {INSERT_SUSP_CALL(op, A_DV_ins_cs(dv_ptr), arreg);});
    return 1;
}

int trigger_vars_minmax(op)
    BPLONG op;
{
    BPLONG i, arity;
    BPLONG_PTR top, dv_ptr;

    SWITCH_OP(op, n_trigger_vars,
              {CREATE_SUSP_VAR_minmax(op, arreg);},

              {},

              {UNTAG_ADDR(op);
                  trigger_vars_minmax(FOLLOW(op));
                  trigger_vars_minmax(FOLLOW((BPLONG_PTR)op+1));},

              {UNTAG_ADDR(op);
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op));
                  for (i = 1; i <= arity; i++)
                      trigger_vars_minmax(FOLLOW(((BPLONG_PTR)op+i)));},

              {INSERT_SUSP_CALL(op, A_DV_minmax_cs(dv_ptr), arreg);});
    return 1;
}

int trigger_vars_dom(op)
    BPLONG op;
{
    BPLONG i, arity;
    BPLONG_PTR top, dv_ptr;

    SWITCH_OP(op, n_trigger_vars,
              {CREATE_SUSP_VAR_dom(op, arreg);},

              {},

              {UNTAG_ADDR(op);
                  trigger_vars_dom(FOLLOW(op));
                  trigger_vars_dom(FOLLOW((BPLONG_PTR)op+1));},

              {UNTAG_ADDR(op);
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op));
                  for (i = 1; i <= arity; i++)
                      trigger_vars_dom(FOLLOW(((BPLONG_PTR)op+i)));},

              {INSERT_SUSP_CALL(op, A_DV_dom_cs(dv_ptr), arreg);});
    return 1;
}

int trigger_vars_any_dom(op)
    BPLONG op;
{
    BPLONG i, arity;
    BPLONG_PTR top, dv_ptr;

    SWITCH_OP(op, n_trigger_vars,
              {CREATE_SUSP_VAR_any_dom(op, arreg);},

              {},

              {UNTAG_ADDR(op);
                  trigger_vars_any_dom(FOLLOW(op));
                  trigger_vars_any_dom(FOLLOW((BPLONG_PTR)op+1));},

              {UNTAG_ADDR(op);
                  arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op));
                  for (i = 1; i <= arity; i++)
                      trigger_vars_any_dom(FOLLOW(((BPLONG_PTR)op+i)));},

              {INSERT_SUSP_CALL(op, A_DV_dom_cs(dv_ptr), arreg);
                  INSERT_SUSP_CALL(op, A_DV_outer_dom_cs(dv_ptr), arreg);});
    return 1;
}

int exclude_elm_dvars() {
    BPLONG P_elm, P_list1, P_list2;

    P_elm = ARG(1, 3);
    P_list1 = ARG(2, 3);
    P_list2 = ARG(3, 3);

    return b_EXCLUDE_ELM_DVARS(P_elm, P_list1, P_list2);
}


int b_EXCLUDE_ELM_DVARS(P_elm, P_list1, P_list2)
    BPLONG P_elm, P_list1, P_list2;
{
    BPLONG elm, P_v, processing_part, P_list;
    BPLONG_PTR dv_ptr, ptr;

    DEREF_NONVAR(P_elm);
    elm = INTVAL(P_elm);

    processing_part = 1;
    P_list = P_list1;

start:
    DEREF_NONVAR(P_list);
    while (P_list != nil_sym) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(P_list);
        P_v = FOLLOW(ptr);

        DEREF_NONVAR(P_v);
        if (IS_SUSP_VAR(P_v)) {
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(P_v);
            domain_set_false_noint(dv_ptr, elm);
        } else if (P_v == P_elm) return 0;

        P_list = FOLLOW(ptr+1);
        DEREF_NONVAR(P_list);
    }
    if (processing_part == 1) {
        P_list = P_list2;
        processing_part = 2;
        goto start;
    }
    return 1;
}

/*
  exclude_elm_vcs(X,VCs)
  VCs=[(Y1,C1),...,(Yn,Cn)]
  ensure that Yi \= X-Ci
*/
int exclude_elm_vcs() {
    BPLONG elm, P_list;

    elm = ARG(1, 2);
    P_list = ARG(2, 2);
    return b_EXCLUDE_ELM_VCS(elm, P_list);
}

int b_EXCLUDE_ELM_VCS(elm, P_list)
    BPLONG elm, P_list;
{
    BPLONG xc;
    BPLONG_PTR dv_ptr, ptr;
    BPLONG P_pair, P_v, P_c;

    DEREF_NONVAR(elm);
    elm = INTVAL(elm);

    DEREF_NONVAR(P_list);
    while (P_list != nil_sym) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(P_list);
        P_list = FOLLOW(ptr+1);

        P_pair = FOLLOW(ptr);
        DEREF_NONVAR(P_pair);
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(P_pair);

        P_v = FOLLOW(ptr+1);  /* (V,C) */
        DEREF_NONVAR(P_v);
        P_c = FOLLOW(ptr+2);  /* (V,C) */
        DEREF_NONVAR(P_c);

        xc = elm-INTVAL(P_c);
        if (IS_SUSP_VAR(P_v)) {
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(P_v);
            domain_set_false_noint(dv_ptr, xc);
        } else if (INTVAL(P_v) == xc) return 0;

        DEREF_NONVAR(P_list);
    }
    return 1;
}

/*
  Select a variable based on the first-fail principle.
  No dereference needed because the list was just copied.
*/
int b_select_ff(Vars, BestVar)
    BPLONG BestVar, Vars;
{
    BPLONG Var;
    BPLONG_PTR dv_ptr, dv_ptr0, ptr;
    BPLONG size, size0;

    BPLONG Vars0;
    Vars0 = Vars;

    DEREF_NONVAR(Vars);
    while (ISLIST(Vars)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
        Var = FOLLOW(ptr); DEREF_NONVAR(Var);
        Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
        if (IS_SUSP_VAR(Var)) {
            dv_ptr0 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
            size0 = DV_size(dv_ptr0);  /* first dvar */
            while (ISLIST(Vars)) {
                if (size0 == 2) break;  /* no size can be smaller than 2 */
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
                Var = FOLLOW(ptr); DEREF_NONVAR(Var);
                Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
                if (IS_SUSP_VAR(Var)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
                    size = DV_size(dv_ptr);
                    if (size < size0) {
                        dv_ptr0 = dv_ptr; size0 = size;
                    }
                }
            }
            ASSIGN_v_heap_term(BestVar, (BPLONG)dv_ptr0);
            return 1;
        }
    }
    return 0;
}

/*
  Select a variable based on the first-fail principle,
  breaking ties by selecting a variable with the smallest lower bound.
*/
int b_SELECT_FF_MIN_cf(Vars, BestVar)
    BPLONG BestVar, Vars;
{
    BPLONG Var, size0, size;
    BPLONG_PTR dv_ptr, dv_ptr0, ptr;

    BPLONG Vars0;
    Vars0 = Vars;

    DEREF_NONVAR(Vars);
    while (ISLIST(Vars)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
        Var = FOLLOW(ptr); DEREF_NONVAR(Var);
        Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
        if (IS_SUSP_VAR(Var)) {
            dv_ptr0 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
            size0 = DV_size(dv_ptr0);
            while (ISLIST(Vars)) {
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
                Var = FOLLOW(ptr); DEREF_NONVAR(Var);
                Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
                if (IS_SUSP_VAR(Var)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
                    size = DV_size(dv_ptr);
                    if (size < size0 || (size == size0 && DV_first(dv_ptr) < DV_first(dv_ptr0))) {
                        dv_ptr0 = dv_ptr; size0 = size;
                    }
                }
            }
            ASSIGN_v_heap_term(BestVar, (BPLONG)dv_ptr0);
            return 1;
        }
    }
    return 0;
}

/*
  Select a variable based on the first-fail principle,
  breaking ties by selecting a variable with the largest uppper bound.
*/
int b_SELECT_FF_MAX_cf(Vars, BestVar)
    BPLONG BestVar, Vars;
{
    BPLONG Var, size, size0;
    BPLONG_PTR dv_ptr, dv_ptr0, ptr;

    BPLONG Vars0;
    Vars0 = Vars;

    DEREF_NONVAR(Vars);
    while (ISLIST(Vars)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
        Var = FOLLOW(ptr); DEREF_NONVAR(Var);
        Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
        if (IS_SUSP_VAR(Var)) {
            dv_ptr0 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
            size0 = DV_size(dv_ptr0);
            while (ISLIST(Vars)) {
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
                Var = FOLLOW(ptr); DEREF_NONVAR(Var);
                Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
                if (IS_SUSP_VAR(Var)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
                    size = DV_size(dv_ptr);
                    if (size < size0 || (size == size0 && DV_last(dv_ptr) > DV_last(dv_ptr0))) {
                        dv_ptr0 = dv_ptr; size0 = size;
                    }
                }
            }
            ASSIGN_v_heap_term(BestVar, (BPLONG)dv_ptr0);
            return 1;
        }
    }
    return 0;
}

/*
  Select a variable with the smallest lower bound, breaking ties by selecting 
  the left-most one with the smallest domain.
*/
int b_SELECT_MIN_cf(Vars, BestVar)
    BPLONG BestVar, Vars;
{
    BPLONG Var, min, min0;
    BPLONG_PTR dv_ptr, dv_ptr0, ptr;

    BPLONG Vars0;
    Vars0 = Vars;

    DEREF_NONVAR(Vars);
    while (ISLIST(Vars)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
        Var = FOLLOW(ptr); DEREF_NONVAR(Var);
        Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
        if (IS_SUSP_VAR(Var)) {
            dv_ptr0 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
            min0 = DV_first(dv_ptr0);
            while (ISLIST(Vars)) {
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
                Var = FOLLOW(ptr); DEREF_NONVAR(Var);
                Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
                if (IS_SUSP_VAR(Var)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
                    min = DV_first(dv_ptr);
                    if (min < min0 || (min == min0 && DV_size(dv_ptr) < DV_size(dv_ptr0))) {
                        dv_ptr0 = dv_ptr; min0 = min;
                    }
                }
            }
            ASSIGN_v_heap_term(BestVar, (BPLONG)dv_ptr0);
            return 1;
        }
    }
    return 0;
}

/*
  Select a variable with the smallest lower bound, breaking ties by selecting 
  the left-most one with the smallest domain.
*/
int b_SELECT_MAX_cf(Vars, BestVar)
    BPLONG BestVar, Vars;
{
    BPLONG Var, max0, max;
    BPLONG_PTR dv_ptr, dv_ptr0, ptr;

    BPLONG Vars0;
    Vars0 = Vars;

    DEREF_NONVAR(Vars);
    while (ISLIST(Vars)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
        Var = FOLLOW(ptr); DEREF_NONVAR(Var);
        Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
        if (IS_SUSP_VAR(Var)) {
            dv_ptr0 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
            max0 = DV_last(dv_ptr0);
            while (ISLIST(Vars)) {
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vars);
                Var = FOLLOW(ptr); DEREF_NONVAR(Var);
                Vars = FOLLOW(ptr+1); DEREF_NONVAR(Vars);
                if (IS_SUSP_VAR(Var)) {
                    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
                    max = DV_last(dv_ptr);
                    if (max > max0 || (max == max0 && DV_size(dv_ptr) < DV_size(dv_ptr0))) {
                        dv_ptr0 = dv_ptr; max0 = max;
                    }
                }
            }
            ASSIGN_v_heap_term(BestVar, (BPLONG)dv_ptr0);
            return 1;
        }
    }
    return 0;
}

int b_CONSTRAINTS_NUMBER_cf(Var, N)
    BPLONG Var, N;
{
    BPLONG_PTR ptr;
    BPLONG lst, count, attrs;
    BPLONG_PTR top;

    DEREF_NONVAR(Var);
    ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
    count = count_cs_list(DV_ins_cs(ptr));

    attrs = DV_attached(ptr);
    DEREF(attrs);
    if (ISSTRUCT(attrs)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(attrs);
        lst = FOLLOW(ptr+1);  /* $attrs(AttrValueList) */
        DEREF(lst);
        while (ISLIST(lst)) {
            BPLONG pair, attr_name, value;
            BPLONG_PTR pair_ptr;
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
            pair = FOLLOW(ptr); DEREF(pair);
            lst = FOLLOW(ptr+1); DEREF(lst);
            pair_ptr = (BPLONG_PTR)UNTAGGED_ADDR(pair);
            attr_name = FOLLOW(pair_ptr+1);
            DEREF_NONVAR(attr_name);  /* (name,value) */
            if (attr_name == attr_neq_atm) {
                value = FOLLOW(pair_ptr+2); DEREF_NONVAR(value);  /* combined_propagators(NeqVs,NeqVCs) */
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(value);
                count += count_cs_list(FOLLOW(ptr+1))+count_cs_list(FOLLOW(ptr+2));
            } else if (attr_name == attr_cfd_atm) {
                value = FOLLOW(pair_ptr+2); DEREF_NONVAR(value);  /* cs(L) */
                ptr = (BPLONG_PTR)UNTAGGED_ADDR(value);
                count += count_cs_list(FOLLOW(ptr+1));
            }
        }
    }

    ASSIGN_f_atom(N, MAKEINT(count));
    return BP_TRUE;
}

int count_cs_list(list)
    BPLONG list;
{
    int i = 0;
    BPLONG_PTR ptr;

    while (ISLIST(list)) {
        i ++;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        list = FOLLOW(ptr+1);
    }
    return i;
}

int c_fd_degree() {
    BPLONG Var, N, cs, count;
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    Var = ARG(1, 2);
    N = ARG(2, 2);

    DEREF(Var);
    if (!IS_SUSP_VAR(Var)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Var);
    cs = DV_ins_cs(dv_ptr);
    if (ISLIST(cs)) {
        DV_ins_cs(dv_ptr) = ADDTAG(UNTAGGED_ADDR(cs), STR);  /* mark it so it won't be counted twice */
        count = dvar_degree_count_connected_vars_cs(cs);
        DV_ins_cs(dv_ptr) = cs;
        dvar_degree_reset_cs_tags_cs(cs);
    } else {
        count = 0;
    }
    unify(N, MAKEINT(count));
    return BP_TRUE;
}

int dvar_degree_count_connected_vars_cs(cs)
    BPLONG cs;
{
    BPLONG_PTR ptr, sf;
    BPLONG constr, count = 0;

    while (ISLIST(cs)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs);  /* untag LST */
        constr = FOLLOW(ptr);  /* car */
        sf = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(constr));
        count += dvar_degree_count_connected_vars_frame(sf);
        cs = FOLLOW(ptr+1);  /* cdr */
    }
    return count;
}

int dvar_degree_count_connected_vars_frame(f)
    BPLONG_PTR f;
{
    BPLONG_PTR sp;
    BPLONG count = 0;

    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    while (sp > f) {
        count += dvar_degree_count_connected_vars_term(FOLLOW(sp));
        sp--;
    }
    return count;
}

int dvar_degree_count_connected_vars_term(term)
    BPLONG term;
{
    BPLONG_PTR dv_ptr, ptr, top;
    BPLONG cs, count;

    count = 0;
start:
    DEREF(term);
    if (IS_SUSP_VAR(term)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
        cs = DV_ins_cs(dv_ptr);
        if (ISLIST(cs)) {
            DV_ins_cs(dv_ptr) = ADDTAG(UNTAGGED_ADDR(cs), STR);  /* mark it so it won't be counted twice */
            count = 1;
        }
    } else if (ISLIST(term)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        count = dvar_degree_count_connected_vars_term(FOLLOW(ptr));
        term = FOLLOW(ptr+1);
        goto start;
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity, count;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
        count = 0;
        for (i = 1; i < arity; i++) {
            count += dvar_degree_count_connected_vars_term(FOLLOW(ptr+i));
        }
    }
    return count;
}

void dvar_degree_reset_cs_tags_cs(cs)
    BPLONG cs;
{
    BPLONG_PTR ptr, sf;
    BPLONG constr;

    while (ISLIST(cs)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs);  /* untag LST */
        constr = FOLLOW(ptr);  /* car */
        sf = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(constr));
        dvar_degree_reset_cs_tags_frame(sf);
        cs = FOLLOW(ptr+1);  /* cdr */
    }
}

void dvar_degree_reset_cs_tags_frame(f)
    BPLONG_PTR f;
{
    BPLONG_PTR sp;

    sp = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(f));
    while (sp > f) {
        dvar_degree_reset_cs_tags_term(FOLLOW(sp));
        sp--;
    }
}

void dvar_degree_reset_cs_tags_term(term)
    BPLONG term;
{
    BPLONG_PTR dv_ptr, ptr, top;
    BPLONG cs;

start:
    DEREF(term);
    if (IS_SUSP_VAR(term)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(term);
        cs = DV_ins_cs(dv_ptr);
        if (ISNIL(cs) || ISLIST(cs)) {
            return;
        } else {  /* restore the tag */
            cs = ADDTAG(UNTAGGED_ADDR(cs), LST);
            DV_ins_cs(dv_ptr) = cs;
            //      dvar_degree_reset_cs_tags_cs(cs);
        }
    } else if (ISLIST(term)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        dvar_degree_reset_cs_tags_term(FOLLOW(ptr));
        term = FOLLOW(ptr+1);
        goto start;
    } else if (ISSTRUCT(term)) {
        BPLONG i, arity;
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(term);
        arity = GET_ARITY((SYM_REC_PTR)FOLLOW(ptr));
        for (i = 1; i < arity; i++) {
            dvar_degree_reset_cs_tags_term(FOLLOW(ptr+i));
        }
    }
}

void display_constraint(frame)
    BPLONG_PTR frame;
{
    SYM_REC_PTR sym_ptr;
    BPLONG arity, i;

    sym_ptr = (SYM_REC_PTR)FOLLOW(((BPLONG_PTR)AR_REEP(frame)+2));
    arity = GET_ARITY(sym_ptr);
    bp_write_pname(GET_NAME(sym_ptr));
    fprintf(curr_out, "(");
    for (i = arity; i > 0; i--) {
        write_term1(*(frame+i), curr_out);
        if (i != 1) fprintf(curr_out, ",");
    }
    fprintf(curr_out, ")\n");
}

int display_constraints() {

    BPLONG_PTR frame;

    frame = sfreg;
    while (AR_PREV(frame) != (BPLONG)frame) {
        display_constraint(frame);
        frame = (BPLONG_PTR)AR_PREV(frame);
    }
    return 1;
}

int c_VV_EQ_C_CON() {
    BPLONG X, Y, C;

    X = ARG(1, 3);
    Y = ARG(2, 3);
    C = ARG(3, 3);
    return b_VV_EQ_C_CON_ccc(X, Y, C);
}

int b_VV_EQ_C_CON_ccc(X, Y, C)
    BPLONG X, Y, C;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y;

    DEREF_NONVAR(X); if (!IS_SUSP_VAR(X)) return BP_TRUE;
    DEREF_NONVAR(Y); if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    C = INTVAL(C);  /* C is dereferenced already */

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    if (IS_BV_DOMAIN(dv_ptr_y))
        c_VV_EQ_C_CON_aux(dv_ptr_x, dv_ptr_y, C);

    if (!IS_SUSP_VAR(FOLLOW(dv_ptr_x))) return BP_TRUE;

    if (IS_BV_DOMAIN(dv_ptr_x))
        c_VV_EQ_C_CON_aux(dv_ptr_y, dv_ptr_x, C);

    return BP_TRUE;
}

int c_VV_EQ_C_CON_aux(dv_ptr_x, dv_ptr_y, C)
    BPLONG C;
BPLONG_PTR dv_ptr_x, dv_ptr_y;
{
    BPLONG currX, currY, maxX, minY, maxY;


    /* for each x in X, there is an y in Y such that x+y=C */
    currX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minY = DV_first(dv_ptr_y); maxY = DV_last(dv_ptr_y);
    /* write_term(dv_ptr_x);printf("+"); write_term(dv_ptr_y); printf("_eq_%d",C);printf("\n"); */
    for (; ; ) {
        currY = C-currX;
        if (!dm_true(dv_ptr_y, currY)) {
            domain_set_false_aux(dv_ptr_x, currX);
            if (ISINT(FOLLOW(dv_ptr_x))) return BP_TRUE;
        }
        if (currX >= maxX) return BP_TRUE;
        if (IS_IT_DOMAIN(dv_ptr_x)) {
            currX++;
        } else {
            currX = domain_next_bv(dv_ptr_x, currX+1);
        }
    }
    return BP_TRUE;
}

/* X=Y+C */
int c_V_EQ_VC_CON() {
    BPLONG X, Y, C;

    X = ARG(1, 3);
    Y = ARG(2, 3);
    C = ARG(3, 3);
    return b_V_EQ_VC_CON_ccc(X, Y, C);
}

int b_V_EQ_VC_CON_ccc(X, Y, C)
    BPLONG X, Y, C;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y;

    DEREF_NONVAR(X); if (!IS_SUSP_VAR(X)) return BP_TRUE;
    DEREF_NONVAR(Y); if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    C = INTVAL(C);  /* no dereference is necessary */

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    if (IS_BV_DOMAIN(dv_ptr_y))
        c_V_EQ_VC_CON_aux(dv_ptr_x, dv_ptr_y, C);

    if (!IS_SUSP_VAR(FOLLOW(dv_ptr_x))) return BP_TRUE;

    if (IS_BV_DOMAIN(dv_ptr_x))
        c_V_EQ_VC_CON_aux(dv_ptr_y, dv_ptr_x, -C);
    return BP_TRUE;
}

int c_V_EQ_VC_CON_aux(dv_ptr_x, dv_ptr_y, C)
    BPLONG C;
BPLONG_PTR dv_ptr_x, dv_ptr_y;
{
    BPLONG currX, currY, maxX, minY, maxY;


    /* for each x in X, there is an y in Y such that x=y+C */
    currX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minY = DV_first(dv_ptr_y); maxY = DV_last(dv_ptr_y);
    /* write_term(dv_ptr_x);printf("_eq_"); write_term(dv_ptr_y); printf("+%d",C);printf("\n"); */
    for (; ; ) {
        currY = currX-C;
        if (!dm_true(dv_ptr_y, currY)) {
            /* printf("exclude(%d,",currX);write_term(dv_ptr_x);printf(")\n");  */
            domain_set_false_aux(dv_ptr_x, currX);
            if (ISINT(FOLLOW(dv_ptr_x))) return BP_TRUE;
        }
        if (currX >= maxX) return BP_TRUE;
        if (IS_IT_DOMAIN(dv_ptr_x)) {
            currX++;
        } else {
            currX = domain_next_bv(dv_ptr_x, currX+1);
        }
    }
    return BP_TRUE;
}

/* for each x in X, there is an y in Y such that A*x+B*y=C */
int c_UU_EQ_C_CON() {
    BPLONG A, X, B, Y, C;
    BPLONG_PTR dv_ptr_x, dv_ptr_y;

    A = ARG(1, 5); A = INTVAL(A);
    X = ARG(2, 5); DEREF_NONVAR(X); if (!IS_SUSP_VAR(X)) return BP_TRUE;
    B = ARG(3, 5); B = INTVAL(B);
    Y = ARG(4, 5); DEREF_NONVAR(Y); if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    C = ARG(5, 5); C = INTVAL(C);

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    if (IS_BV_DOMAIN(dv_ptr_y) || B != 1)
        c_UU_EQ_C_CON_aux(A, dv_ptr_x, B, dv_ptr_y, C);

    if (!IS_SUSP_VAR(FOLLOW(dv_ptr_x))) return BP_TRUE;

    if (IS_BV_DOMAIN(dv_ptr_x) || A != 1)
        c_UU_EQ_C_CON_aux(B, dv_ptr_y, A, dv_ptr_x, C);

    return BP_TRUE;
}

/* for each x in X, there is an y in Y such that A*x+B*y=C */
int c_UU_EQ_C_CON_aux(A, dv_ptr_x, B, dv_ptr_y, C)
    BPLONG A, B, C;
BPLONG_PTR dv_ptr_x, dv_ptr_y;
{
    BPLONG tmp, currX, currY, maxX, minY, maxY;

    currX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minY = DV_first(dv_ptr_y); maxY = DV_last(dv_ptr_y);
    for (; ; ) {
        tmp = C-A*currX;
        currY = tmp/B;
        if (B*currY != tmp || !dm_true(dv_ptr_y, currY)) {
            domain_set_false_aux(dv_ptr_x, currX);
            if (ISINT(FOLLOW(dv_ptr_x))) return BP_TRUE;
        }
        if (currX >= maxX) {return BP_TRUE;}
        if (IS_IT_DOMAIN(dv_ptr_x)) {
            currX++;
        } else {
            currX = domain_next_bv(dv_ptr_x, currX+1);
        }
    }
    return BP_TRUE;
}

void print_event_queue() {
    int i;
    printf("trigger_no=" BPLONG_FMT_STR "\n", trigger_no);
    for (i = 1; i <= trigger_no; i++) {
        printf("FLAG(%d) queue(" BPULONG_FMT_STR ")\n", event_flag[i], triggeredCs[i]);
    }
    if (trigger_no >= 1) printf("\n");
}

/* for each x in X, there is a y in Y such that A*x=B*y+C */
int c_U_EQ_UC_CON() {
    BPLONG A, X, B, Y, C;
    BPLONG_PTR dv_ptr_x, dv_ptr_y;

    A = ARG(1, 5); A = INTVAL(A);
    X = ARG(2, 5); DEREF_NONVAR(X); if (!IS_SUSP_VAR(X)) return BP_TRUE;
    B = ARG(3, 5); B = INTVAL(B);
    Y = ARG(4, 5); DEREF_NONVAR(Y); if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    C = ARG(5, 5); C = INTVAL(C);

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    if (IS_BV_DOMAIN(dv_ptr_y) || B != 1)
        c_U_EQ_UC_CON_aux(A, dv_ptr_x, B, dv_ptr_y, C);

    if (!IS_SUSP_VAR(FOLLOW(dv_ptr_x))) return BP_TRUE;

    if (IS_BV_DOMAIN(dv_ptr_x) || A != 1)
        c_U_EQ_UC_CON_aux(B, dv_ptr_y, A, dv_ptr_x, -C);
    return BP_TRUE;
}

/* for each x in X, there is a y in Y such that A*x=B*y+C */
int c_U_EQ_UC_CON_aux(A, dv_ptr_x, B, dv_ptr_y, C)
    BPLONG A, B, C;
BPLONG_PTR dv_ptr_x, dv_ptr_y;
{
    BPLONG tmp, currX, currY, maxX, minY, maxY;

    currX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minY = DV_first(dv_ptr_y); maxY = DV_last(dv_ptr_y);

    for (; ; ) {
        tmp = A*currX-C;
        currY = tmp/B;
        if (tmp != B*currY || !dm_true(dv_ptr_y, currY)) {
            domain_set_false_aux(dv_ptr_x, currX);
            if (ISINT(FOLLOW(dv_ptr_x))) return BP_TRUE;
        }
        if (currX >= maxX) return BP_TRUE;
        if (IS_IT_DOMAIN(dv_ptr_x)) {
            currX++;
        } else {
            currX = domain_next_bv(dv_ptr_x, currX+1);
        }
    }
    return BP_TRUE;
}

/*
  $set(SP,SA,Card,Ref,Univ,UnivSize,Tag,Notation)
*/

#define CLPSETTERM_PTR_GET_SP(ptr) FOLLOW(ptr+1)
#define CLPSETTERM_PTR_GET_SA(ptr) FOLLOW(ptr+2)
#define CLPSETTERM_PTR_GET_CARD(ptr) FOLLOW(ptr+3)
#define CLPSETTERM_PTR_GET_REF(ptr) FOLLOW(ptr+4)
#define CLPSETTERM_PTR_GET_USIZE(ptr) FOLLOW(ptr+6)
#define CLPSETTERM_PTR_GET_TAG(ptr) FOLLOW(ptr+7)
/** CLP(Set) **/
/*
  $clpset_check_when_card_bound_dvar(Card,S,SetTerm)=>true.
  fd_size(SP,SPSize),
  UpSize is SPSize-2, % not count dummies
  fd_size(SA,NotAddedSize),
  AddedSize is UnivSize-NotAddedSize+2,
  (Card=:=UpSize ->
  $clpset_indomain_pickup_all_possible(Ref,SP,SA,Notation,Tag)
  ;   
  Card=:=AddedSize -> 
  $clpsetterm_indomain_pickup_only_in(Ref,SP,SA,Card,Notation,Tag)
  ;
  true).
*/
int b_CLPSET_CARD_BOUND_c(SetTerm)
    BPLONG SetTerm;
{
    BPLONG_PTR top, ptr, sa_dv_ptr, sp_dv_ptr;
    BPLONG SP, SA, Card, Ref, USize, Tag;
    BPLONG SPSize, SASize;
    BPLONG card_low, card_up;

    DEREF(SetTerm);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(SetTerm);

    SP = CLPSETTERM_PTR_GET_SP(ptr); DEREF(SP);
    sp_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SP);
    SPSize = DV_size(sp_dv_ptr);

    SA = CLPSETTERM_PTR_GET_SA(ptr); DEREF(SA);
    sa_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SA);
    SASize = DV_size(sa_dv_ptr);

    USize = CLPSETTERM_PTR_GET_USIZE(ptr); DEREF(USize); USize = INTVAL(USize);

    Card = CLPSETTERM_PTR_GET_CARD(ptr); DEREF(Card); Card = INTVAL(Card);

    Ref = CLPSETTERM_PTR_GET_REF(ptr);

    Tag = CLPSETTERM_PTR_GET_TAG(ptr);

    card_low = USize-SASize+2; card_up = SPSize-2;
    if (Card > card_up || Card < card_low) return 0;
    if (Card == card_up ) {
        unify(Tag, BP_ONE);
        unify(Ref, clpset_pickup_all_possible(sp_dv_ptr, sa_dv_ptr));
    } else if (Card == card_low) {
        unify(Tag, BP_ONE);
        unify(Ref, clpset_pickup_only_in(sp_dv_ptr, sa_dv_ptr, Card));
    }
    return 1;
}

/*
  $clpset_check_when_low_updated_dvar(SA,S,SetTerm) => true.
  b_DM_COUNT_cf(SA,NotAddedSize),
  AddedSize is UnivSize-NotAddedSize+2,
  domain_region_min(Card,AddedSize),    % Card #>= AddedSize,
  ((integer(Card),Card==AddedSize)->
  $clpsetterm_indomain_pickup_only_in(Ref,SP,SA,Card,Notation,Tag)
  ;
  true
  ).
*/
int b_CLPSET_LOW_UPDATED_c(SetTerm)
    BPLONG SetTerm;
{
    BPLONG_PTR top, ptr, dv_ptr, sp_dv_ptr, sa_dv_ptr;
    BPLONG SP, SA, Card, Ref, USize, SASize;
    BPLONG new_card_low;

    DEREF(SetTerm);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(SetTerm);

    SA = CLPSETTERM_PTR_GET_SA(ptr); DEREF(SA);
    sa_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SA);
    SASize = DV_size(sa_dv_ptr);

    USize = CLPSETTERM_PTR_GET_USIZE(ptr); DEREF(USize); USize = INTVAL(USize);

    new_card_low = USize-SASize+2;

    Card = CLPSETTERM_PTR_GET_CARD(ptr); DEREF(Card);

    if (!ISINT(Card)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Card);
        if (domain_region_noint(dv_ptr, new_card_low, BP_MAXINT_1W) == 0) return 0;
        Card = DV_var(dv_ptr);
    }
    if (ISINT(Card) && INTVAL(Card) == new_card_low) {
        SP = CLPSETTERM_PTR_GET_SP(ptr); DEREF(SP);
        sp_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SP);
        Ref = CLPSETTERM_PTR_GET_REF(ptr);
        unify(Ref, clpset_pickup_only_in(sp_dv_ptr, sa_dv_ptr, INTVAL(Card)));
    }
    return 1;
}

/*
  $clpset_check_when_up_updated_dvar(SP,S,SetTerm) => 
  b_DM_COUNT_cf(SP,UpSize),
  UpBoundSize is UpSize-2, % not count dummies
  domain_region_max(Card,UpBoundSize),
  ((integer(Card),Card==UpBoundSize) -> 
  $clpset_indomain_pickup_all_possible(Ref,SP,SA,Notation,Tag)
  ;  
  true).
*/
int b_CLPSET_UP_UPDATED_c(SetTerm)
    BPLONG SetTerm;
{
    BPLONG_PTR top, ptr, dv_ptr, sa_dv_ptr, sp_dv_ptr;
    BPLONG SP, SA, Card, Ref, SPSize;
    BPLONG new_card_up;

    DEREF(SetTerm);
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(SetTerm);

    SP = CLPSETTERM_PTR_GET_SP(ptr); DEREF(SP);
    sp_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SP);
    SPSize = DV_size(sp_dv_ptr);

    new_card_up = SPSize-2;

    Card = CLPSETTERM_PTR_GET_CARD(ptr); DEREF(Card);
    if (!ISINT(Card)) {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Card);
        if (domain_region_noint(dv_ptr, BP_MININT_1W, new_card_up) == 0) return 0;
        Card = DV_var(dv_ptr);
    }

    if (ISINT(Card) && INTVAL(Card) == new_card_up) {
        SA = CLPSETTERM_PTR_GET_SA(ptr); DEREF(SA);
        sa_dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(SA);
        Ref = CLPSETTERM_PTR_GET_REF(ptr);
        unify(Ref, clpset_pickup_all_possible(sp_dv_ptr, sa_dv_ptr));
    }
    return 1;
}

/* SP and SA are derefered already */
/*
  $clpset_indomain_pickup_all_possible(S,SP,SA,{},Tag):-var(S),dvar(SP) :  %set notation
  domain_min_max(SP,First,Last),
  domain_next_inst(SP,First,RealFirst),
  (RealFirst==Last->Tag=1,S={};
  b_DM_PREV_ccf(SP,Last,RealLast),
  $clpset_dvar_indomain_pickup_all_possible(SP,SA,RealFirst,RealLast,Set),
  Tag=1,
  S={Set}).

  %%
  $clpset_dvar_indomain_pickup_all_possible(SP,SA,Cur,Last,Set):-Cur==Last :
  domain_set_false(SA,Cur),
  Set=Cur.
  $clpset_dvar_indomain_pickup_all_possible(SP,SA,Cur,Last,Set):-true :
  domain_next_inst(SP,Cur,Next),
  domain_set_false(SA,Cur),
  Set=(Cur,Set1),
  $clpset_dvar_indomain_pickup_all_possible(SP,SA,Next,Last,Set1).
*/
BPLONG clpset_pickup_all_possible(sp_dv_ptr, sa_dv_ptr)
    BPLONG_PTR sp_dv_ptr, sa_dv_ptr;
{
    BPLONG set0, return_val;
    BPLONG_PTR set_tail_ptr;
    BPLONG last, cur;

    cur = DV_first(sp_dv_ptr);
    last = DV_last(sp_dv_ptr);
    CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);  /* real first */
    if (cur == last) {
        return empty_set;
    }
    CALL_DOMAIN_PREV(sp_dv_ptr, last, last);  /* real last */
    set_tail_ptr = &set0;
    /**/
    while (cur != last) {
        FOLLOW(set_tail_ptr) = ADDTAG(heap_top, STR);
        FOLLOW(heap_top++) = (BPLONG)comma_psc;
        FOLLOW(heap_top++) = MAKEINT(cur);
        set_tail_ptr = heap_top++;
        domain_set_false_noint(sa_dv_ptr, cur);
        CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);
    }
    /* last elm */
    FOLLOW(set_tail_ptr) = MAKEINT(cur);
    domain_set_false_noint(sa_dv_ptr, cur);
    /**/
    return_val = ADDTAG(heap_top, STR);
    FOLLOW(heap_top++) = (BPLONG)set_constructor_psc;
    FOLLOW(heap_top++) = set0;
    /*  write_term(return_val);printf("\n"); */
    return return_val;
}

/*
  $clpsetterm_indomain_pickup_only_in(S,SP,SA,Card,{},Tag):-var(S),dvar(SP) :
  domain_min_max(SP,First,Last),
  domain_next_inst(SP,First,RealFirst),
  $clpset_dvar_indomain_pickup_only_in(SP,SA,RealFirst,Last,Set,Card),
  Tag=1,
  (Card=:=0->S={};S={Set}).

  %%
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Cur,Last,Set,Card):-Cur==Last : true.
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Cur,Last,Set,Card):-
  b_DM_TRUE_cc(SA,Cur) :
  domain_next_inst(SP,Cur,Next),
  domain_set_false(SP,Cur),
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Next,Last,Set,Card).
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Cur,Last,Set,Card):-Card==1 :
  Set=Cur,
  domain_next_inst(SP,Cur,Next),
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Next,Last,Set1,0).
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Cur,Last,Set,Card):-true :
  Set=(Cur,Set1),
  Card1 is Card-1,
  domain_next_inst(SP,Cur,Next),
  $clpset_dvar_indomain_pickup_only_in(SP,SA,Next,Last,Set1,Card1).
*/
BPLONG clpset_pickup_only_in(sp_dv_ptr, sa_dv_ptr, card)
    BPLONG_PTR sp_dv_ptr, sa_dv_ptr;
BPLONG card;
{
    BPLONG last, cur;

    BPLONG set0, return_val;
    BPLONG_PTR set_tail_ptr;

    cur = DV_first(sp_dv_ptr);
    last = DV_last(sp_dv_ptr);
    CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);  /* real first */
    if (card == 0) {clpset_exclude_all_possible(sp_dv_ptr, cur, last); return empty_set;}
    set_tail_ptr = &set0;
    /**/
    for (; ; ) {
        /* if (cur==last){printf("STRANGE %d %d %d\n",cur,last,card);exit(1);} */
        if (!dm_true(sa_dv_ptr, cur)) {  /* in low bound */
            if (card == 1) {
                FOLLOW(set_tail_ptr) = MAKEINT(cur);
                CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);
                clpset_exclude_all_possible(sp_dv_ptr, cur, last);
                return_val = ADDTAG(heap_top, STR);
                FOLLOW(heap_top++) = (BPLONG)set_constructor_psc;
                FOLLOW(heap_top++) = set0;
                /*      write_term(return_val);printf("\n"); */
                return return_val;
            }
            FOLLOW(set_tail_ptr) = ADDTAG(heap_top, STR);
            FOLLOW(heap_top++) = (BPLONG)comma_psc;
            FOLLOW(heap_top++) = MAKEINT(cur);
            set_tail_ptr = heap_top++;
            card--;
        } else {
            domain_set_false_noint(sp_dv_ptr, cur);
        }
        CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);
    }
}

void clpset_exclude_all_possible(sp_dv_ptr, cur, last)
    BPLONG_PTR sp_dv_ptr;
BPLONG cur, last;
{
    while (cur != last) {
        domain_set_false_noint(sp_dv_ptr, cur);
        CALL_DOMAIN_NEXT(sp_dv_ptr, cur, cur);
    }
}

/*
  $determinate_pred(reify_eq_constr_consistency,3):-true : true.
  reify_eq_constr_consistency(B,X,Y):-
  dvar(X),dvar(Y) :
  domain_min_max(X,MinX,MaxX),
  domain_min_max(Y,MinY,MaxY),
  (MinX>MaxY->B=0;
  MinY>MaxX->B=0;
  X==Y -> B=1;
  true).
  reify_eq_constr_consistency(B,X,Y):-
  dvar(X),integer(Y) :
  (b_DM_TRUE_cc(X,Y)->true;B=0).
  reify_eq_constr_consistency(B,X,Y):-
  integer(X),dvar(Y) :
  (b_DM_TRUE_cc(Y,X)->true;B=0).
  reify_eq_constr_consistency(B,X,Y):-true : true.
*/
int b_REIFY_EQ_CONSTR_ACTION(B, X, Y)
    BPLONG X, Y, B;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y, dv_ptr_b;

    DEREF_NONVAR(X); DEREF_NONVAR(Y);
    DEREF_NONVAR(B);
    if (X == Y) {
        UNIFY_DVAR_VAL(B, BP_ONE);
    }

    if (ISINT(X)) {
        if (ISINT(Y)) {
            UNIFY_DVAR_VAL(B, BP_ZERO);  /* X \= Y */
        }
        dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
        if (!dm_true(dv_ptr_y, INTVAL(X))) {
            UNIFY_DVAR_VAL(B, BP_ZERO);
        }
    } else {
        dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        if (ISINT(Y)) {
            if (!dm_true(dv_ptr_x, INTVAL(Y))) {
                UNIFY_DVAR_VAL(B, BP_ZERO);
            }
        } else {  /* X and Y are domain vars */
            dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
            if (dm_disjoint(dv_ptr_x, dv_ptr_y)) {
                UNIFY_DVAR_VAL(B, BP_ZERO);
            }
        }
    }
    return BP_TRUE;
}

/*
  $determinate_pred(reify_ge_constr_consistency,3):-true : true.
  %reify_ge_constr_consistency(B,X,Y):-write(reify_ge_constr_consistency(B,X,Y)),nl,fail.
  reify_ge_constr_consistency(B,X,Y):-
  domain_min_max(X,MinX,MaxX),
  domain_min_max(Y,MinY,MaxY),
  (MinX>=MaxY->B=1;
  MinY>MaxX->B=0;
  true).
  %    write('<=reify_ge_constr_consistency'(B,X,Y)),nl.
*/
int b_REIFY_GE_CONSTR_ACTION(B, X, Y)
    BPLONG X, Y, B;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y, dv_ptr_b;
    BPLONG min_x, max_x, min_y, max_y;

    DEREF_NONVAR(X); DEREF_NONVAR(Y);
    if (ISINT(X)) {
        min_x = max_x = INTVAL(X);
    } else {
        dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        min_x = DV_first(dv_ptr_x);
        max_x = DV_last(dv_ptr_x);
    }
    if (ISINT(Y)) {
        min_y = max_y = INTVAL(Y);
    } else {
        dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
        min_y = DV_first(dv_ptr_y);
        max_y = DV_last(dv_ptr_y);
    }
    if (min_x >= max_y) {
        DEREF_NONVAR(B);
        UNIFY_DVAR_VAL(B, BP_ONE);
    }
    if (max_x < min_y) {
        DEREF_NONVAR(B);
        UNIFY_DVAR_VAL(B, BP_ZERO);
    }
    return BP_TRUE;
}

/*
  $determinate_pred(reify_neq_constr_consistency,3):-true : true.
  reify_neq_constr_consistency(B,X,Y):-
  dvar(X),dvar(Y) :
  domain_min_max(X,MinX,MaxX),
  domain_min_max(Y,MinY,MaxY),
  (MinX>MaxY->B=1;
  MinY>MaxX->B=1;
  X==Y -> B=0;
  true).
  reify_neq_constr_consistency(B,X,Y):-
  dvar(X),integer(Y) :
  (b_DM_TRUE_cc(X,Y)->true;B=1).
  reify_neq_constr_consistency(B,X,Y):-
  integer(X),dvar(Y) :
  (b_DM_TRUE_cc(Y,X)->true;B=1).
  reify_neq_constr_consistency(B,X,Y):-true : true.
*/
int b_REIFY_NEQ_CONSTR_ACTION(B, X, Y)
    BPLONG B, X, Y;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y, dv_ptr_b;

    /*
      B = FOLLOW(arreg+3);DEREF_NONVAR(B); 
      X = FOLLOW(arreg+2);DEREF_NONVAR(X); 
      Y = FOLLOW(arreg+1);DEREF_NONVAR(Y); 
    */
    DEREF_NONVAR(B);
    DEREF_NONVAR(X);
    DEREF_NONVAR(Y);

    if (X == Y) {
        UNIFY_DVAR_VAL(B, BP_ZERO);
    }
    if (ISINT(X)) {
        if (ISINT(Y)) {
            UNIFY_DVAR_VAL(B, BP_ONE);  /* X \= Y */
        }
        dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
        if (!dm_true(dv_ptr_y, INTVAL(X))) {
            UNIFY_DVAR_VAL(B, BP_ONE);
        }
    } else {
        dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        if (ISINT(Y)) {
            if (!dm_true(dv_ptr_x, INTVAL(Y))) {
                UNIFY_DVAR_VAL(B, BP_ONE);
            }
        } else {  /* X and Y are domain vars */
            dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
            if (dm_disjoint(dv_ptr_x, dv_ptr_y)) {
                UNIFY_DVAR_VAL(B, BP_ONE);
            }
        }
    }
    return BP_TRUE;
}

/*
  for each x in X, |x| is in Y;
  for each y in Y, either y or -y is in X
*/
int b_ABS_CON_cc(X, Y)
    BPLONG X, Y;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y;
    BPLONG elm, melm, minX, maxX, minY, maxY;

    DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) return BP_TRUE;
    DEREF_NONVAR(Y);
    if (!IS_SUSP_VAR(Y)) return BP_TRUE;

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
    if (IS_IT_DOMAIN(dv_ptr_x) && IS_IT_DOMAIN(dv_ptr_y)) return BP_TRUE;

    minX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minY = DV_first(dv_ptr_y); maxY = DV_last(dv_ptr_y);

    for (elm = minY; elm <= maxY; elm++) {
        melm = -elm;
        if (!dm_true(dv_ptr_y, elm)) {
            domain_set_false_aux(dv_ptr_x, elm);
            domain_set_false_aux(dv_ptr_x, melm);
        } else if (!dm_true(dv_ptr_x, elm) &&
                   !dm_true(dv_ptr_x, melm)) {
            domain_set_false_aux(dv_ptr_y, elm);
        }
    }
    return BP_TRUE;
}

/*
  fd_abs_x_to_y(X,Y):-
  domain_min_max(X,MinX,MaxX),
  (MinX >= 0 -> domain_region(Y,MinX,MaxX);
  MaxX =< 0 -> Lower is -MaxX, Upper is -MinX, domain_region(Y,Lower,Upper);
  AbsMinX is -MinX,
  (AbsMinX>MaxX->Up is AbsMinX;Up is MaxX),
  domain_region_max(Y,Up)).
*/
/* when X updated */
int b_FD_ABS_X_TO_Y(X, Y)
    BPLONG X, Y;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y;
    BPLONG minX, maxX, up;

    DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) {
        minX = maxX = INTVAL(X);
    } else {
        dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        minX = DV_first(dv_ptr_x);
        maxX = DV_last(dv_ptr_x);
    }
    DEREF_NONVAR(Y);
    if (ISINT(Y)) {
        Y = INTVAL(Y);
        if (minX > -Y) {
            return unify(X, MAKEINT(Y));
        } else if (maxX < Y) {
            return unify(X, MAKEINT(-Y));
        }
        return BP_TRUE;
    }
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
    if (minX >= 0) {
        return domain_region_noint(dv_ptr_y, minX, maxX);
    } else if (maxX <= 0) {
        return domain_region_noint(dv_ptr_y, -maxX, -minX);
    } else {
        up = -minX;
        if (maxX > up) up = maxX;
        return domain_region_noint(dv_ptr_y, 0, up);
    }
}

/* X mod Y = Z (precondition integer(Y),Y>0,min(X)>=0)
   for each x in X if (x mod Y) is not in Z, then exclude x from X
*/
int b_MOD_CON_ccc(X, Y, Z)
    BPLONG X, Y, Z;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_z;
    BPLONG currX, maxX;

    DEREF_NONVAR(Y); Y = INTVAL(Y);
    DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) return BP_TRUE;
    DEREF_NONVAR(Z);
    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    currX = DV_first(dv_ptr_x);
    maxX = DV_last(dv_ptr_x);
    if (ISINT(Z)) {
        Z = INTVAL(Z);
        while (currX <= maxX) {
            if (currX%Y != Z) {
                if (domain_set_false_aux(dv_ptr_x, currX) == BP_FALSE) return BP_FALSE;
            }
            if (IS_IT_DOMAIN(dv_ptr_x)) {
                currX++;
            } else {
                currX = domain_next_bv(dv_ptr_x, currX+1);
            }
        }
    } else {
        dv_ptr_z = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Z);
        while (currX <= maxX) {
            if (!dm_true(dv_ptr_z, currX % Y)) {
                if (domain_set_false_aux(dv_ptr_x, currX) == BP_FALSE) return BP_FALSE;
            }
            if (IS_IT_DOMAIN(dv_ptr_x)) {
                currX++;
            } else {
                currX = domain_next_bv(dv_ptr_x, currX+1);
            }
        }
    }
    return BP_TRUE;
}


/* X // Y = Z (precondition integer(Y), Y>0 min(X)>=0)
   Z in min(X)//Y..max(X)//Y
   X in min(Z)*Y..(max(Z)+1)*Y-1
*/
int b_IDIV_CON_ccc(X, Y, Z)
    BPLONG X, Y, Z;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_z;
    BPLONG low, up;

    /*  printf("=>check_idiv"); write_term(X); printf(" ");write_term(Y); printf(" ");write_term(Z); printf("\n"); */

    DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) return BP_TRUE;
    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    DEREF_NONVAR(Y); Y = INTVAL(Y);
    low = DV_first(dv_ptr_x);
    up = DV_last(dv_ptr_x);
    if (low > BP_MININT_1W) low = low/Y;
    if (up < BP_MAXINT_1W) up = up/Y;

    DEREF_NONVAR(Z);
    if (ISINT(Z)) {
        Z = INTVAL(Z);
        if (Z < low || Z > up) return BP_FALSE;
        if (domain_region_noint(dv_ptr_x, Y*Z, Y*(Z+1)-1) == BP_FALSE) return BP_FALSE;
    } else {
        dv_ptr_z = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Z);
        if (domain_region_noint(dv_ptr_z, low, up) == BP_FALSE) return BP_FALSE;
        low = DV_first(dv_ptr_z);
        up = DV_last(dv_ptr_z);
        if (low > BP_MININT_1W && up < BP_MAXINT_1W) {
            if (domain_region_noint(dv_ptr_x, Y*low, Y*(up+1)-1) == BP_FALSE) return BP_FALSE;
        }
    }
    return BP_TRUE;
}

/*
  for each x in X, either x-N or x+N must be in Y.
  for each y in Y, either y-N or y+N must be in X.
*/
int b_ABS_DIFF_CON_ccc(X, Y, N)
    BPLONG X, Y, N;
{
    BPLONG_PTR dv_ptr_x, dv_ptr_y;
    BPLONG elm, min, max;

    DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) return BP_TRUE;
    DEREF_NONVAR(Y);
    if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    DEREF_NONVAR(N);
    N = INTVAL(N);

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
    min = DV_first(dv_ptr_x); max = DV_last(dv_ptr_x);
    for (elm = min; elm <= max; elm++) {
        if (dm_true(dv_ptr_y, elm-N) || dm_true(dv_ptr_y, elm+N));
        else if (domain_set_false_aux(dv_ptr_x, elm) == BP_FALSE) return BP_FALSE;
    }

    if (!IS_SUSP_VAR(FOLLOW(dv_ptr_x))) return BP_TRUE;
    min = DV_first(dv_ptr_y); max = DV_last(dv_ptr_y);
    for (elm = min; elm <= max; elm++) {
        if (dm_true(dv_ptr_x, elm-N) || dm_true(dv_ptr_x, elm+N));
        else if (domain_set_false_aux(dv_ptr_y, elm) == BP_FALSE) return BP_FALSE;
    }

    return BP_TRUE;
}

/* abs(X-Y) = N
   triggered after Ex is excluded from X
*/
int b_ABS_DIFF_X_TO_Y(Ex)
    BPLONG Ex;
{
    BPLONG X, Y, N, Ey;
    BPLONG_PTR dv_ptr_x, dv_ptr_y;

    X = FOLLOW(arreg+3); DEREF_NONVAR(X);
    if (!IS_SUSP_VAR(X)) return BP_TRUE;

    Y = FOLLOW(arreg+2); DEREF_NONVAR(Y);
    if (!IS_SUSP_VAR(Y)) return BP_TRUE;

    N = FOLLOW(arreg+1); DEREF_NONVAR(N); N = INTVAL(N);
    DEREF_NONVAR(Ex); Ex = INTVAL(Ex);

    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);

    Ey = Ex-N;
    if (!dm_true(dv_ptr_x, Ey-N)) {
        if (domain_set_false_aux(dv_ptr_y, Ey) == BP_FALSE) return BP_FALSE;
    }
    Ey = Ex+N;
    if (!dm_true(dv_ptr_x, Ey+N)) {
        if (domain_set_false_aux(dv_ptr_y, Ey) == BP_FALSE) return BP_FALSE;
    }
    return BP_TRUE;
}

/* abs(abs(X)-abs(Y)) \= N: either X or Y is bound */
int b_ABS_ABS_DIFF_NEQ()
{
    BPLONG X, Y, N, t;
    BPLONG_PTR dv_ptr;

    X = FOLLOW(arreg+3); DEREF_NONVAR(X);
    Y = FOLLOW(arreg+2); DEREF_NONVAR(Y);
    N = FOLLOW(arreg+1); DEREF_NONVAR(N); N = INTVAL(N);

    if (!IS_SUSP_VAR(X)) {
        X = INTVAL(X);
        if (!IS_SUSP_VAR(Y)) {
            Y = INTVAL(Y);
            if (X < 0) X = -X;
            if (Y < 0) Y = -Y;
            if (X-Y == N || Y-X == N) return BP_FALSE; else return BP_TRUE;
        } else {
            dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
        }
    } else {
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
        X = INTVAL(Y);  /* Y must be an integer */
    }

    if (X < 0) X = -X;
    t = X+N;
    domain_set_false_noint(dv_ptr, t);
    if (domain_set_false_aux(dv_ptr, -t) == BP_FALSE) return BP_FALSE;
    t = X-N;
    if (t >= 0) {
        if (domain_set_false_aux(dv_ptr, t) == BP_FALSE) return BP_FALSE;
        if (domain_set_false_aux(dv_ptr, -t) == BP_FALSE) return BP_FALSE;
    }
    return BP_TRUE;
}
