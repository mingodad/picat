/********************************************************************
 *   File   : event.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "frame.h"

#define SUSP_START 0L
#define SUSP_REENT 1L
#define SUSP_SLEEP 2L
#define SUSP_RUN 3L
#define SUSP_EXIT 4L
#define SUSP_CLONE 5L


#define HIGH_PRIORITY 1
#define LOW_PRIORITY 0

#define FRAME_IS_START(F) (AR_STATUS(F) == SUSP_START)
#define FRAME_IS_DEAD(F) (AR_STATUS(F) == SUSP_EXIT)
#define FRAME_IS_CLONE(F) (AR_STATUS(F) == SUSP_CLONE)
#define FRAME_IS_SLEEP(F) (TAG(AR_STATUS(F)) == SUSP_SLEEP)
#define FRAME_IS_RUN(F) (TAG(AR_STATUS(F)) == SUSP_RUN)
#define FRAME_IS_REENT(F) (TAG(AR_STATUS(F)) == SUSP_REENT)

#define EVENT_VAR_INS 0
#define EVENT_DVAR_INS 1
#define EVENT_DVAR_MINMAX 2
#define EVENT_DVAR_DOM 4
#define EVENT_DVAR_OUTER_DOM 5
#define EVENT_GENERAL 6

#define KILL_SUSP_FRAME {                       \
        AR_STATUS(arreg) = SUSP_EXIT;           \
    }

#define ASSIGN_DVAR(dv_ptr, value) {                    \
        PUSHTRAIL_H_NONATOMIC(dv_ptr, DV_var(dv_ptr));  \
        DV_var(dv_ptr) = value;                         \
        INSERT_TRIGGER_dvar_ins(dv_ptr); }

#define UPDATE_FIRST_LAST_SIZE(dv_ptr, old_first, first, old_last, last, size) { \
        if (dv_ptr < hbreg) {                                           \
            top = A_DV_first(dv_ptr);                                   \
            PUSHTRAILC_ATOMIC(top, old_first);                          \
            FOLLOW(top) = first;                                        \
            top = A_DV_last(dv_ptr);                                    \
            PUSHTRAILC_ATOMIC(top, old_last);                           \
            FOLLOW(top) = last;                                         \
            top = A_DV_size(dv_ptr);                                    \
            PUSHTRAILC_ATOMIC(top, FOLLOW(top));                        \
            FOLLOW(top) = size;                                         \
        } else {                                                        \
            DV_first(dv_ptr) = first;                                   \
            DV_last(dv_ptr) = last;                                     \
            DV_size(dv_ptr) = size;                                     \
        }                                                               \
        INSERT_TRIGGER_minmax(dv_ptr);                                  \
    }

#define UPDATE_FIRST_SIZE(dv_ptr, old_first, first, size) {     \
        if (dv_ptr < hbreg) {                                   \
            top = A_DV_first(dv_ptr);                           \
            PUSHTRAILC_ATOMIC(top, old_first);                  \
            FOLLOW(top) = first;                                \
            top = A_DV_size(dv_ptr);                            \
            PUSHTRAILC_ATOMIC(top, FOLLOW(top));                \
            FOLLOW(top) = size;                                 \
        } else {                                                \
            DV_first(dv_ptr) = first;                           \
            DV_size(dv_ptr) = size;                             \
        }                                                       \
        INSERT_TRIGGER_minmax(dv_ptr);                          \
    }

#define UPDATE_LAST_SIZE(dv_ptr, old_last, last, size) {        \
        if (dv_ptr < hbreg) {                                   \
            top = A_DV_last(dv_ptr);                            \
            PUSHTRAILC_ATOMIC(top, old_last);                   \
            FOLLOW(top) = last;                                 \
            top = A_DV_size(dv_ptr);                            \
            PUSHTRAILC_ATOMIC(top, FOLLOW(top));                \
            FOLLOW(top) = size;                                 \
        } else {                                                \
            DV_last(dv_ptr) = last;                             \
            DV_size(dv_ptr) = size;                             \
        }                                                       \
        INSERT_TRIGGER_minmax(dv_ptr);                          \
    }

#define UPDATE_SIZE(dv_ptr, old_size, size)     \
    top = A_DV_size(dv_ptr);                    \
    if (dv_ptr < hbreg) {                       \
        PUSHTRAILC_ATOMIC(top, old_size);       \
    };                                          \
    FOLLOW(top) = size;                         \

#define LOOKUP_DOMAIN_ELM(dv_ptr, elm, hashtab, size, de_ptr, index, ret_code) \
    if (hashtab == NULL) {                                              \
        if (elm < 0 || elm > size) ret_code;                            \
        de_ptr = dv_ptr+SIZE_OF_DV+elm*SIZE_OF_DE;                      \
    } else {                                                            \
        index = lookup_dm_elm(elm, hashtab, size);                      \
        if (index >= 0)                                                 \
            de_ptr = dv_ptr+SIZE_OF_DV+index*SIZE_OF_DE;                \
        else ret_code;                                                  \
    }

#define LOOKUP_DOMAIN_ELM_1(dv_ptr, elm, hashtab, size, de_ptr, index, ret_code) \
    if (hashtab == NULL) {                                              \
        de_ptr = dv_ptr+SIZE_OF_DV+elm*SIZE_OF_DE;                      \
    } else {                                                            \
        index = lookup_dm_elm(elm, hashtab, size);                      \
        de_ptr = dv_ptr+SIZE_OF_DV+index*SIZE_OF_DE;                    \
    }

#define INITIALIZE_DV_CS(cs, AR) {                                      \
        cs = ADDTAG(heap_top, LST);                                     \
        FOLLOW(heap_top) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)AR, INT_TAG); \
        FOLLOW(heap_top+1) = nil_sym;                                   \
        heap_top += 2; }

#define CREATE_SUSP_VAR_nocs(op1) {                     \
        FOLLOW(op1) = (BPLONG)heap_top;                 \
        PUSHTRAIL(op1);                                 \
        dv_ptr = heap_top;                              \
        DV_var(dv_ptr) = ((BPLONG)heap_top | SUSP);     \
        DV_type(dv_ptr) = UN_DOMAIN;                    \
        DV_ins_cs(dv_ptr) = nil_sym;                    \
        DV_minmax_cs(dv_ptr) = nil_sym;                 \
        DV_dom_cs(dv_ptr) = nil_sym;                    \
        DV_outer_dom_cs(dv_ptr) = nil_sym;              \
        DV_attached(dv_ptr) = nil_sym;                  \
        heap_top += SIZE_OF_DV;                         \
    }

#define CREATE_SUSP_VAR(op1) {                          \
        FOLLOW(op1) = (BPLONG)heap_top;                 \
        PUSHTRAIL(op1);                                 \
        dv_ptr = heap_top;                              \
        DV_var(dv_ptr) = ((BPLONG)heap_top | SUSP);     \
        DV_type(dv_ptr) = UN_DOMAIN;                    \
        DV_attached(dv_ptr) = nil_sym;                  \
        heap_top += SIZE_OF_DV;                         \
    }

#define CREATE_SUSP_VAR_ins(op1, ar) {                  \
        CREATE_SUSP_VAR(op1);                           \
        DV_minmax_cs(dv_ptr) = nil_sym;                 \
        DV_dom_cs(dv_ptr) = nil_sym;                    \
        DV_outer_dom_cs(dv_ptr) = nil_sym;              \
        INITIALIZE_DV_CS(DV_ins_cs(dv_ptr), ar);}

#define CREATE_SUSP_VAR_minmax(op1, ar) {               \
        CREATE_SUSP_VAR(op1);                           \
        DV_ins_cs(dv_ptr) = nil_sym;                    \
        DV_dom_cs(dv_ptr) = nil_sym;                    \
        DV_outer_dom_cs(dv_ptr) = nil_sym;              \
        INITIALIZE_DV_CS(DV_minmax_cs(dv_ptr), ar);}

#define CREATE_SUSP_VAR_dom(op1, ar) {                  \
        CREATE_SUSP_VAR(op1);                           \
        DV_ins_cs(dv_ptr) = nil_sym;                    \
        DV_minmax_cs(dv_ptr) = nil_sym;                 \
        DV_outer_dom_cs(dv_ptr) = nil_sym;              \
        INITIALIZE_DV_CS(DV_dom_cs(dv_ptr), ar);}

#define CREATE_SUSP_VAR_any_dom(op1, ar) {              \
        CREATE_SUSP_VAR(op1);                           \
        DV_ins_cs(dv_ptr) = nil_sym;                    \
        DV_minmax_cs(dv_ptr) = nil_sym;                 \
        INITIALIZE_DV_CS(DV_dom_cs(dv_ptr), ar);        \
        INITIALIZE_DV_CS(DV_outer_dom_cs(dv_ptr), ar);}

#define INSERT_SUSP_CALL(op, cs_addr, ar) {                             \
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);                   \
        top = cs_addr;                                                  \
        PUSHTRAIL_H_NONATOMIC(top, FOLLOW(top));                        \
        *heap_top = ADDTAG((BPULONG)stack_up_addr-(BPULONG)ar, INT_TAG); \
        *(heap_top+1) = FOLLOW(top);                                    \
        FOLLOW(top) = ADDTAG(heap_top, LST);                            \
        heap_top += 2;                                                  \
    }

#define INSERT_SUSP_CALL_NOINS(op, cs_addr, ar) INSERT_SUSP_CALL(op, cs_addr, ar)

#define POST_EVENT(sv_ptr, type, eo) {                          \
        toam_signal_vec |= TRIGGER_ON;                          \
        triggeredCs[++trigger_no] = A_DV_dom_cs(sv_ptr);        \
        event_flag[trigger_no] = type;                          \
        event_object[trigger_no] = (BPLONG)eo;                  \
    }

#define ActionPerformed 10
#define MouseClicked 11
#define MousePressed 12
#define MouseReleased 13
#define MouseEntered 14
#define MouseExited 15
#define MouseMoved 16
#define MouseDragged 17
#define WindowClosing 18
#define WindowOpened 19
#define WindowIconified 20
#define WindowDeiconified 21
#define WindowClosed 22
#define WindowActivated 23
#define WindowDeactivated 24
#define FocusGained 25
#define FocusLost 26
#define KeyPressed 27
#define KeyReleased 28
#define KeyTyped 29
#define ComponentMoved 30
#define ComponentResized 31
#define ComponentHidden 32
#define ComponentShown 33
#define ItemStateChanged 34
#define TextValueChanged 35
#define AdjustmentValueChanged 36
#define TimeOut 37

#define NUM_CG_GLOBALS TimeOut-ActionPerformed+2  /* 2 slots for default window */

#define IgnoredEvent -999

#define IS_MOUSE_EVENT(no) (no >= MouseClicked && no <= MouseDragged)

typedef struct CgKeyEvent {
    BPLONG type;
    BPLONG code;
    char ch;
    BPLONG modifiers;
} *CgKeyEventClass;

typedef struct CgMouseEvent {
    BPLONG type;
    BPLONG x;
    BPLONG y;
    BPLONG count;
    BPLONG modifiers;
} *CgMouseEventClass;

typedef struct CgComponentEvent {
    BPLONG type;
    BPLONG x;
    BPLONG y;
    BPLONG width;
    BPLONG height;
} *CgComponentEventClass;

typedef struct CgItemEvent {
    BPLONG type;
    BPLONG index;
    BPLONG change;
} *CgItemEventClass;

typedef struct CgAdjustmentEvent {
    BPLONG type;
    BPLONG adjust_type;
    BPLONG value;
} *CgAdjustmentEventClass;

typedef struct event_info {
    BPLONG no;
    BPLONG type;
    void *event_object;
} CgEventInfo;

/* Thread */
#define THREAD_TIMER(ptr) FOLLOW(ptr+1)
#define THREAD_CALL(ptr) FOLLOW(ptr+2)
#define THREAD_STATUS(ptr) FOLLOW(ptr+3)
#define THREAD_PRIORITY(ptr) FOLLOW(ptr+4)

#define A_THREAD_TIMER(ptr) (ptr+1)
#define A_THREAD_CALL(ptr) (ptr+2)
#define A_THREAD_STATUS(ptr) (ptr+3)
#define A_THREAD_PRIORITY(ptr) (ptr+4)

#define THREAD_START 0
#define THREAD_RUN 1
#define THREAD_READY 2
#define THREAD_SLEEP 3
#define THREAD_EXIT 4

#define TAGGED_THREAD_START MAKEINT(0)
#define TAGGED_THREAD_RUN MAKEINT(1)
#define TAGGED_THREAD_READY MAKEINT(2)
#define TAGGED_THREAD_SLEEP MAKEINT(3)
#define TAGGED_THREAD_EXIT MAKEINT(4)

