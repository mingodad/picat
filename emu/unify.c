/********************************************************************
 *   File   : unify.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "term.h"
#include "bapi.h"
#include "event.h"
#include "clpfd.h"

#define UNIFY_DVAR_SUSP(dv_ptr1, dv_ptr2) {bind_susp_susp(dv_ptr1, dv_ptr2); INSERT_TRIGGER_var_ins(dv_ptr1);}

#define UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr, first, last, size) {       \
        if (DV_first(dv_ptr) != first) {                                \
            if (DV_last(dv_ptr) != last) {                              \
                UPDATE_FIRST_LAST_SIZE(dv_ptr, DV_first(dv_ptr), first, DV_last(dv_ptr), last, size); \
            } else {                                                    \
                UPDATE_FIRST_SIZE(dv_ptr, DV_first(dv_ptr), first, size); \
            }                                                           \
        } else if (DV_last(dv_ptr) != last) {                           \
            UPDATE_LAST_SIZE(dv_ptr, DV_last(dv_ptr), last, size);      \
        }                                                               \
        }

#define DV_NEQ_C(dv_ptr, x) {                                           \
        if (!IS_SUSP_VAR(FOLLOW(dv_ptr))) return (FOLLOW(dv_ptr) == MAKEINT(x)) ? 0 : 1; \
        domain_set_false_aux(dv_ptr, x);                                \
        }


int unify(op1, op2)
    register BPLONG op1, op2;
{
    register BPLONG_PTR top;
    register BPLONG arity, i;
    BPLONG tmp_op;
    BPLONG_PTR dv_ptr;
    /*
      printf("unify "); write_term(op1); printf(" "); write_term(op2); printf("\n");
    */
nunify:
    switch (TAG(op1)) {
    case REF:
        NDEREF(op1, nunify);
        SWITCH_OP_VAR(op2, unify_v_d,
                      {if (op1 != op2) {
                              if ((BPULONG)op1 < (BPULONG)op2) {
                                  if ((BPULONG)op1 < (BPULONG)heap_top) {  /* op1 not in loc stack */
                                      FOLLOW(op2) = op1;
                                      PUSHTRAIL(op2);
                                  } else {  /* op1 points to op2 */
                                      PUSHTRAIL_s(op1); FOLLOW(op1) = (BPLONG)heap_top;
                                      PUSHTRAIL_s(op2); FOLLOW(op2) = (BPLONG)heap_top;
                                      NEW_HEAP_FREE;
                                  }
                              } else {  /* op1 > op2 */
                                  if ((BPULONG)op2 < (BPULONG)heap_top) {
                                      FOLLOW(op1) = op2;
                                      PUSHTRAIL(op1);
                                  } else {
                                      PUSHTRAIL_s(op1); FOLLOW(op1) = (BPLONG)heap_top;
                                      PUSHTRAIL_s(op2); FOLLOW(op2) = (BPLONG)heap_top;
                                      NEW_HEAP_FREE;
                                  }
                              }
                          }
                          return 1;},
                      {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
                          FOLLOW(op1) = (BPLONG)dv_ptr;
                          PUSHTRAIL(op1);
                          return 1;},
                      {FOLLOW(op1) = op2;
                          PUSHTRAIL(op1);
                          return 1;});

    case ATM:
        SWITCH_OP_ATM(op2, unify_atom_d,
                      {FOLLOW(op2) = op1;
                          PUSHTRAIL(op2);
                          return 1;},
                      {if (op1 == op2) return 1;},
                      {goto bind_value_susp;});
        return 0;

    case LST:
        SWITCH_OP_LST(op2, unify_lst_d,
                      {FOLLOW(op2) = op1;
                          PUSHTRAIL(op2);
                          return 1;},
                      {if (op1 == op2) return 1;
                          UNTAG_ADDR(op1);
                          UNTAG_ADDR(op2);
                          if (!unify(*(BPLONG_PTR) op1, *(BPLONG_PTR) op2)) return 0;
                          op1 = *((BPLONG_PTR) op1 + 1);
                          op2 = *((BPLONG_PTR) op2 + 1);
                          goto nunify;},
                      {goto bind_value_susp;});
        return 0;

    case STR:
        if (op1 < 0) goto unify_susp_d;
        SWITCH_OP_STRUCT(op2, unify_str_d,
                         {FOLLOW(op2) = op1;
                             PUSHTRAIL(op2);
                             return 1;},
                         {if (op1 == op2) return 1;
                             UNTAG_ADDR(op1);
                             UNTAG_ADDR(op2);
                             if (FOLLOW(op1) != FOLLOW(op2))
                                 return 0;
                             arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op1));
                             for (i = 1; i < arity; i++) {
                                 if (!unify(*((BPLONG_PTR) op1 + i), *((BPLONG_PTR) op2 + i))) {
                                     return 0;
                                 }
                             }
                             op1 = *((BPLONG_PTR) op1 + arity);
                             op2 = *((BPLONG_PTR) op2 + arity);
                             goto nunify;},
                         {goto bind_value_susp;});
        return 0;
    }

    SWITCH_OP_VAR(op2, unify_susp_d,
                  {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
                      FOLLOW(op2) = (BPLONG)dv_ptr;
                      PUSHTRAIL(op2);
                      return 1;},
                  {if (op1 == op2) return 1; else return unify_suspvar_suspvar(op1, op2);},
                  {goto bind_susp_value;});


bind_value_susp:
    tmp_op = op1; op1 = op2; op2 = tmp_op;
bind_susp_value:
    top = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
    if (!IS_UN_DOMAIN(top)) {
        if (!ISINT(op2)) return 0;
        tmp_op = INTVAL(op2);
        if (!dm_true(top, tmp_op)) return 0;
    }
    INSERT_TRIGGER_var_ins(top);
    PUSHTRAIL_H_NONATOMIC(top, op1);
    FOLLOW(top) = op2;
    return 1;
}

/*
  op1: non-stack var
  op2: a var 
*/
void unify_nsv_v(op1, op2)
    register BPLONG op1, op2;
{
    if (op1 != op2) {
        if ((BPULONG)op1 < (BPULONG)op2) {
            FOLLOW(op2) = op1;
            PUSHTRAIL(op2);
        } else {  /* op1 points to op2 */
            FOLLOW(op1) = op2;
            PUSHTRAIL_h(op1);
        }
    }
}

/*
  op1,op2 are both lists
*/
int unify_lst_lst(op1, op2)
    register BPLONG op1, op2;
{
    if (op1 == op2) return 1;
    UNTAG_ADDR(op1);
    UNTAG_ADDR(op2);
    if (!unify(*(BPLONG_PTR) op1, *(BPLONG_PTR) op2)) return 0;
    op1 = *((BPLONG_PTR) op1 + 1);
    op2 = *((BPLONG_PTR) op2 + 1);
    return unify(op1, op2);
}

/*
  op1,op2 are both structures
*/
int unify_str_str(op1, op2)
    register BPLONG op1, op2;
{
    register BPLONG arity, i;

    if (op1 == op2) return 1;
    UNTAG_ADDR(op1);
    UNTAG_ADDR(op2);
    if (FOLLOW(op1) != FOLLOW(op2))
        return 0;
    arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op1));
    for (i = 1; i < arity; i++) {
        if (!unify(*((BPLONG_PTR) op1 + i), *((BPLONG_PTR) op2 + i))) {
            return 0;
        }
    }
    op1 = *((BPLONG_PTR) op1 + arity);
    op2 = *((BPLONG_PTR) op2 + arity);
    return unify(op1, op2);
}

int is_IDENTICAL(op1, op2)
    BPLONG op1, op2;
{
    return bp_identical(op1, op2);
}

int bp_identical(op1, op2)
    register BPLONG op1, op2;
{
    register BPLONG_PTR top;
    register BPLONG arity, i;

    DEREF(op2);
    SWITCH_OP(op1, nidentical,
              {if (op1 == op2) return 1;},
              {if (op1 == op2) return 1;},
              {if (ISLIST(op2)) {
                      if (op1 != op2) {
                          UNTAG_ADDR(op1);
                          UNTAG_ADDR(op2);
                          if (!bp_identical(*(BPLONG_PTR)op1, *(BPLONG_PTR)op2)) return 0;
                          op1 = *(((BPLONG_PTR)op1)+1);
                          op2 = *(((BPLONG_PTR)op2)+1); DEREF(op2);
                          goto nidentical;
                      }
                      return 1;
                  }},
              {if (ISSTRUCT(op2)) {
                      if (op1 != op2) {  /* a != b */
                          UNTAG_ADDR(op1);
                          UNTAG_ADDR(op2);
                          if (FOLLOW(op1) != FOLLOW(op2))  /* 0(a) != 0(b) */
                              return 0;
                          else {
                              arity = GET_ARITY((SYM_REC_PTR)(FOLLOW(op1)));
                              for (i = 1; i < arity; i++)
                                  if (!bp_identical(*((BPLONG_PTR)op1 + i), *((BPLONG_PTR)op2 + i))) return 0;
                              op1 = *((BPLONG_PTR)op1 + arity);
                              op2 = *((BPLONG_PTR)op2 + arity); DEREF(op2);
                              goto nidentical;
                          }
                      }
                      return 1;
                  }},
              {if (op1 == op2) return 1;});
    return 0;
}

int is_UNIFIABLE(t1, t2)
    BPLONG t1, t2;
{
    BPLONG_PTR trail_top0, hbreg0;
    BPLONG initial_diff0, trigger_no0;
    int res;

    /* Logically equivalent to not(not(t1 = t2)).
       Does not work well if suspension variables are included. */
    DEREF(t1); DEREF(t2);
    if (ISREF(t1) || ISREF(t2)) return BP_TRUE;

    hbreg0 = hbreg;
    trigger_no0 = trigger_no;
    hbreg = heap_top;
    initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;

    res = unify(t1, t2);
    trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
    UNDO_TRAILING;

    trigger_no = trigger_no0;
    hbreg = hbreg0;
    return res;
}

int c_UNIFIABLE() {
    register BPLONG op1, op2;
    op1 = ARG(1, 2); op2 = ARG(2, 2);
    return is_UNIFIABLE(op1, op2);
}

int unify_suspvar_suspvar(op1, op2)
    BPLONG op1, op2;
{
    BPLONG_PTR dv_ptr1, dv_ptr2;

    if (op1 == op2) return 1;
    dv_ptr1 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op1);
    dv_ptr2 = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op2);
    if (!IS_UN_DOMAIN(dv_ptr1))
        if (!IS_UN_DOMAIN(dv_ptr2)) {
            return unify_dvar_dvar(dv_ptr1, dv_ptr2);
        } else {
            UNIFY_DVAR_SUSP(dv_ptr1, dv_ptr2);
            return BP_TRUE;
        }
    else
        if (!IS_UN_DOMAIN(dv_ptr2)) {
            UNIFY_DVAR_SUSP(dv_ptr2, dv_ptr1);
            return BP_TRUE;
        }
        else {
            BPLONG_PTR top;
            if (dv_ptr1 < dv_ptr2) top = dm_clone(dv_ptr1); else top = dm_clone(dv_ptr2);
            bind_susp_susp(top, dv_ptr1);
            bind_susp_susp(top, dv_ptr2);
            INSERT_TRIGGER_var_ins(top);
            return BP_TRUE;
        }
}

int unify_dvar_dvar(dv_ptr1, dv_ptr2)
    BPLONG_PTR dv_ptr1, dv_ptr2;
{
    BPLONG first, last, count;
    BPLONG_PTR top;

    first = (DV_first(dv_ptr1) >= DV_first(dv_ptr2)) ? DV_first(dv_ptr1) : DV_first(dv_ptr2);
    last = (DV_last(dv_ptr1) <= DV_last(dv_ptr2)) ? DV_last(dv_ptr1) : DV_last(dv_ptr2);
    if (first > last) return 0;

    if (IS_IT_DOMAIN(dv_ptr1))
        if (IS_IT_DOMAIN(dv_ptr2))
            goto itdvar_itdvar;
        else {
            goto itdvar_bvdvar;
        }
    else if (IS_IT_DOMAIN(dv_ptr2)) {
        top = dv_ptr1; dv_ptr1 = dv_ptr2; dv_ptr2 = top;
        goto itdvar_bvdvar;
    }

bvdvar_bvdvar:
    {
        BPLONG elm, updated;
        updated = 1;
        while (first < last && updated == 1) {  /* find first that is in both domains */
            updated = 0;
            first = domain_next_bv(dv_ptr1, first);
            elm = domain_next_bv(dv_ptr2, first);
            if (elm != first) {
                updated = 1;
                first = elm;
            }
        }
        updated = 1;
        while (first < last && updated == 1) {  /* find last that is in both domains */
            updated = 0;
            last = domain_prev_bv(dv_ptr1, last);
            elm = domain_prev_bv(dv_ptr2, last);
            if (elm != last) {
                updated = 1;
                last = elm;
            }
        }
        if (first > last) return 0;
        if (last == first) {
            if (dm_true(dv_ptr1, first) && dm_true(dv_ptr2, first)) {
                ASSIGN_DVAR(dv_ptr1, MAKEINT(first));
                ASSIGN_DVAR(dv_ptr2, MAKEINT(first));
                return 1;
            } else return 0;
        }

        if (!exclude_dom_comp_aux(dv_ptr1, dv_ptr2) || !exclude_dom_comp_aux(dv_ptr2, dv_ptr1)) return 0;  /* bit vector intersection */

        count = count_domain_elms(dv_ptr2, first, last);
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr1, first, last, count);
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr2, first, last, count);
        if (dv_ptr1 < dv_ptr2) top = dm_clone(dv_ptr1); else top = dm_clone(dv_ptr2);
        bind_susp_susp(top, dv_ptr1);
        bind_susp_susp(top, dv_ptr2);
        INSERT_TRIGGER_var_ins(top);
        return BP_TRUE;
    }

itdvar_itdvar:
    {
        if (last == first) {
            ASSIGN_DVAR(dv_ptr1, MAKEINT(first));
            ASSIGN_DVAR(dv_ptr2, MAKEINT(first));
            return 1;
        }

        if (DV_outer_dom_cs(dv_ptr1) != nil_sym) {
            if (!exclude_dom_comp_aux(dv_ptr2, dv_ptr1)) return 0;
        }
        if (DV_outer_dom_cs(dv_ptr2) != nil_sym) {
            if (!exclude_dom_comp_aux(dv_ptr1, dv_ptr2)) return 0;
        }

        count = last-first+1;
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr1, first, last, count);
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr2, first, last, count);
        if (dv_ptr1 < dv_ptr2) top = dm_clone(dv_ptr1); else top = dm_clone(dv_ptr2);
        bind_susp_susp(top, dv_ptr1);
        bind_susp_susp(top, dv_ptr2);
        INSERT_TRIGGER_var_ins(top);
        return BP_TRUE;
    }

itdvar_bvdvar:
    {
        first = domain_next_bv(dv_ptr2, first);
        last = domain_prev_bv(dv_ptr2, last);
        if (first > last) return 0;
        if (last == first) {
            ASSIGN_DVAR(dv_ptr1, MAKEINT(first));
            ASSIGN_DVAR(dv_ptr2, MAKEINT(first));
            return 1;
        }
        if (DV_outer_dom_cs(dv_ptr1) != nil_sym) {
            if (!exclude_dom_comp_aux(dv_ptr2, dv_ptr1)) return 0;
        }
        if (DV_outer_dom_cs(dv_ptr2) != nil_sym) {
            if (!exclude_dom_comp_aux(dv_ptr1, dv_ptr2)) return 0;
        }

        count = count_domain_elms(dv_ptr2, first, last);
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr2, first, last, count);
        UNIFY_UPDATE_FIRST_LAST_SIZE(dv_ptr1, first, last, count);
        top = dm_clone(dv_ptr2);
        bind_susp_susp(top, dv_ptr1);
        bind_susp_susp(top, dv_ptr2);
        INSERT_TRIGGER_var_ins(top);
        return BP_TRUE;
    }
}

/* let dv_ptr2 point to dv_ptr1 */
int bind_susp_susp(dv_ptr1, dv_ptr2)
    BPLONG_PTR dv_ptr1, dv_ptr2;
{
    /*
      printf("bind_susp_susp dv_ptr1=%x dv_ptr2=%x %d\n",(BPULONG)dv_ptr1-(BPULONG)stack_low_addr,(BPULONG)dv_ptr2-(BPULONG)stack_low_addr,dv_ptr2>dv_ptr1);
    */
    PUSHTRAIL_H_NONATOMIC((BPLONG)dv_ptr2, FOLLOW(dv_ptr2));
    FOLLOW(dv_ptr2) = (BPLONG)dv_ptr1;
    /*
      printf("ins(%x) new_ref(%x): \n",(BPULONG)dv_ptr2-(BPULONG)stack_low_addr,(BPULONG)dv_ptr1-(BPULONG)stack_low_addr);
      printf("dv_ins_cs1");write_term(DV_ins_cs(dv_ptr1));
      printf("dv_ins_cs2");write_term(DV_ins_cs(dv_ptr2));
      printf("\n");
    */
    if (DV_ins_cs(dv_ptr2) != nil_sym)
        merge_cs(A_DV_ins_cs(dv_ptr1), DV_ins_cs(dv_ptr2));
    if (DV_minmax_cs(dv_ptr2) != nil_sym)
        merge_cs(A_DV_minmax_cs(dv_ptr1), DV_minmax_cs(dv_ptr2));
    if (DV_dom_cs(dv_ptr2) != nil_sym)
        merge_cs(A_DV_dom_cs(dv_ptr1), DV_dom_cs(dv_ptr2));
    if (DV_outer_dom_cs(dv_ptr2) != nil_sym)
        merge_cs(A_DV_outer_dom_cs(dv_ptr1), DV_outer_dom_cs(dv_ptr2));

    // printf("dv_ins_cs1");write_term(DV_ins_cs(dv_ptr1)); printf("\n\n");
    return BP_TRUE;
}

/* No dereference needed,
   Linear-time merger */
void merge_cs(addr_head_cs1, cs2)
    BPLONG_PTR addr_head_cs1;
    BPLONG cs2;
{
    BPLONG elm, lst, cs1, new_cs2;
    BPLONG_PTR ptr, constr_ar;

    /*
      printf("merge cs len=%d\n",list_length(cs2,cs2));
      write_term(FOLLOW(addr_head_cs1)); printf("\n");
      write_term(cs2); printf("\n");
    */
    cs1 = FOLLOW(addr_head_cs1);
    /* mark frames in cs1 */
    lst = cs1;
    while (ISLIST(lst)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        elm = FOLLOW(ptr);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(elm));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) | TOP_BIT);  /* mark it */
        lst = FOLLOW(ptr+1);
    }

    new_cs2 = cs1;  /* to be added to the beginning of cs1 */
    while (ISLIST(cs2)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(cs2);
        elm = FOLLOW(ptr);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(elm));
        if ((AR_STATUS(constr_ar) & TOP_BIT_MASK) || FRAME_IS_DEAD(constr_ar)) {  /* exists in cs1, then drop it */
        } else {
            NEW_LIST_NODE(elm, new_cs2, new_cs2);
            if (local_top-heap_top <= LARGE_MARGIN) myquit(STACK_OVERFLOW, "un");
        }
        cs2 = FOLLOW(ptr+1);
    }

    /* unmark cs1 */
    lst = cs1;
    while (ISLIST(lst)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        elm = FOLLOW(ptr);
        constr_ar = (BPLONG_PTR)((BPULONG)stack_up_addr-(BPULONG)UNTAGGED_CONT(elm));
        AR_STATUS(constr_ar) = (AR_STATUS(constr_ar) & ~TOP_BIT);
        lst = FOLLOW(ptr+1);
    }

    /* trail addr_head_cs1 if low-priority agents added */
    if (FOLLOW(addr_head_cs1) != new_cs2) {
        PUSHTRAIL_H_NONATOMIC((BPLONG)addr_head_cs1, FOLLOW(addr_head_cs1));
        FOLLOW(addr_head_cs1) = new_cs2;
    }
    /*
      write_term(FOLLOW(addr_head_cs1)); printf("\n");
    */
}


/* same as identical(op1,op2) except that variables are not compared */
int key_identical(op1, op2)
    BPLONG op1, op2;
{
    register BPLONG_PTR top;
    register BPLONG arity, i;

key_identical_start:
    switch (TAG(op1)) {
    case REF:
        NDEREF(op1, key_identical_start);
        SWITCH_OP_VAR(op2, key_identical_v_d,
                      {return 1;},
                      {return 1;},
                      {return 0;});

    case ATM:
        SWITCH_OP_ATM(op2, key_identical_atom_d,
                      {return 0;},
                      {if (op1 == op2) return 1;},
                      {return 0;});
        return 0;

    case LST:
        if (IsNumberedVar(op1)) {
            DEREF(op2);
            if (ISREF(op2) || IsNumberedVar(op2)) {
                return 1;
            } else {
                return 0;
            }
        }
        SWITCH_OP_LST(op2, key_identical_lst_d,
                      {return 0;},
                      { if (op1 == op2) return 1;
                          if (IsNumberedVar(op2)) return 0;
                          UNTAG_ADDR(op1);
                          UNTAG_ADDR(op2);
                          if (!key_identical(*(BPLONG_PTR) op1, *(BPLONG_PTR) op2)) return 0;
                          op1 = *((BPLONG_PTR) op1 + 1);
                          op2 = *((BPLONG_PTR) op2 + 1);
                          goto key_identical_start;},
                      {return 0;});
        return 0;

    case STR:
        if (op1 < 0) return 0;
        SWITCH_OP_STRUCT(op2, key_identical_str_d,
                         {return 0;},
                         {if (op1 == op2) return 1;
                             UNTAG_ADDR(op1);
                             UNTAG_ADDR(op2);
                             if (FOLLOW(op1) != FOLLOW(op2))
                                 return 0;
                             arity = GET_ARITY((SYM_REC_PTR)FOLLOW(op1));
                             for (i = 1; i < arity; i++) {
                                 if (!key_identical(*((BPLONG_PTR) op1 + i), *((BPLONG_PTR) op2 + i))) {
                                     return 0;
                                 }
                             }
                             op1 = *((BPLONG_PTR) op1 + arity);
                             op2 = *((BPLONG_PTR) op2 + arity);
                             goto key_identical_start;},
                         {return 0;});
        return 0;
    }
    return 0;
}


/*
  t2 is a numbered term, which needs to be dereferenced
*/
int unifyNumberedTerms(BPLONG t1, BPLONG t2) {
    BPLONG_PTR top;
    BPLONG i, arity, op1, op2;

beginning:
    DEREF_NONVAR(t2);
    SWITCH_OP_NOSUSP(t1,
                     start,
                     {NDEREF(t1, start);
                         ASSIGN_TRAIL_VALUE(t1, NumberVar(global_var_num));
                         global_var_num++;
                         return (FOLLOW(t1) == t2);
                     },
                     {return (t1 == t2);},
                     {if (IsNumberedVar(t1)) return (t1 == t2);
                         if (!ISLIST(t2)) return 0;
                         UNTAG_ADDR(t1); UNTAG_ADDR(t2);
                         op1 = *(BPLONG_PTR)t1;
                         op2 = *(BPLONG_PTR)t2;
                         if (op1 != op2 && !unifyNumberedTerms(op1, op2)) return 0;
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
                             op1 = *((BPLONG_PTR)t1+i);
                             op2 = *((BPLONG_PTR)t2+i);
                             if (op1 != op2 && !unifyNumberedTerms(op1, op2)) return 0;
                         }
                         return 1;
                     });
}

int c_SAME_ADDR() {
    BPLONG X, Y;
    BPLONG_PTR top;

    X = ARG(1, 2);
    Y = ARG(2, 2);
    DEREF(X); DEREF(Y);
    return (X == Y) ? BP_TRUE : BP_FALSE;
}

