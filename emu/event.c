/********************************************************************
 *   File   : event.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021
 *   Purpose: event handling functions

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#endif
#include <stdlib.h>
#include <string.h>
#include "bprolog.h"
#include "event.h"
#include "inst.h"

#define WINDOW_ARITY 6
#define COMPONENT_ARITY 10
#define COMPONENT_NO 9
#define COMPONENT_HANDLER 10

SYM_REC_PTR cg_component_psc, cg_window_psc;
SYM_REC_PTR mouse_event_psc, key_event_psc, component_event_psc, item_event_psc, adjustment_event_psc;

BPLONG event_count = 0;
#define EVENT_POOL_SIZE 1000
CgEventInfo event_pool[EVENT_POOL_SIZE];

static BPLONG bpp_java_obj = MAKEINT(0);

#ifdef _WIN32
#define bp_sleep(interval) Sleep(interval)
CRITICAL_SECTION csCriticalSection;
#define ENTER_CRITICAL_SECTION EnterCriticalSection(&csCriticalSection);
#define LEAVE_CRITICAL_SECTION LeaveCriticalSection(&csCriticalSection);
#else
pthread_mutex_t csCriticalSection;
#define bp_sleep(interval) usleep(interval*1000)
#define ENTER_CRITICAL_SECTION pthread_mutex_lock(&csCriticalSection);
#define LEAVE_CRITICAL_SECTION pthread_mutex_unlock(&csCriticalSection);
#endif

char *eventNoNameTable[] = {
    "actionPerformed",
    "mouseClicked",
    "mousePressed",
    "mouseReleased",
    "mouseEntered",
    "mouseExited",
    "mouseMoved",
    "mouseDragged",
    "windowClosing",
    "windowOpened",
    "windowIconified",
    "windowDeiconified",
    "windowClosed",
    "windowActivated",
    "windowDeactivated",
    "focusGained",
    "focusLost",
    "keyPressed",
    "keyReleased",
    "keyTyped",
    "componentMoved",
    "componentResized",
    "componentHidden",
    "componentShown",
    "ItemStateChanged",
    "TextValueChanged",
    "AdjustmentValueChanged",
    "TimeOut"
};

SYM_REC_PTR cg_arc_psc;
SYM_REC_PTR cg_circle_psc;
SYM_REC_PTR cg_image_psc;
SYM_REC_PTR cg_line_psc;
SYM_REC_PTR cg_oval_psc;
SYM_REC_PTR cg_polygon_psc;
SYM_REC_PTR cg_square_psc;
SYM_REC_PTR cg_rectangle_psc;
SYM_REC_PTR cg_roundRectangle_psc;
SYM_REC_PTR cg_star_psc;
SYM_REC_PTR cg_triangle_psc;

SYM_REC_PTR cg_button_psc;
SYM_REC_PTR cg_label_psc;
SYM_REC_PTR cg_textbox_psc;
SYM_REC_PTR cg_textField_psc;
SYM_REC_PTR cg_textArea_psc;
SYM_REC_PTR cg_menuBar_psc;
SYM_REC_PTR cg_menu_psc;
SYM_REC_PTR cg_menuItem_psc;
SYM_REC_PTR cg_checkboxMenuItem_psc;
SYM_REC_PTR cg_checkbox_psc;
SYM_REC_PTR cg_list_psc;
SYM_REC_PTR cg_choice_psc;
/* SYM_REC_PTR cg_dialog_psc; */
SYM_REC_PTR cg_fileDialog_psc;
SYM_REC_PTR cg_scrollbar_psc;

int c_post_event() {
    BPLONG x = ARG(1, 3);
    BPLONG no = ARG(2, 3);
    BPLONG e = ARG(3, 3);

    return b_POST_EVENT_ccc(x, no, e);
}

int b_POST_EVENT_ccc(x, no, e)
    BPLONG x, no, e;
{
    BPLONG_PTR dv_ptr;

    DEREF(x);
    if (!IS_SUSP_VAR(x)) {
        if (ISREF(x)) return BP_TRUE;
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);
    DEREF(no); no = INTVAL(no);
    DEREF(e);
    switch (no) {
    case EVENT_VAR_INS:
        INSERT_TRIGGER_var_ins(dv_ptr);
        break;
    case EVENT_DVAR_MINMAX:
        INSERT_TRIGGER_minmax(dv_ptr);
        break;
    case EVENT_DVAR_DOM:
        INSERT_TRIGGER_dom(dv_ptr, e) ;
        break;
    case EVENT_DVAR_OUTER_DOM:
        INSERT_TRIGGER_outer_dom(dv_ptr, e) ;
        break;
    case EVENT_GENERAL:
        INSERT_TRIGGER_event(dv_ptr, e) ;
        break;
    }
    return 1;
}

/* Java cannot access Prolog's representation of components
   and Prolog cannot access Java's representation of components.
   Java and Prolog communicate through component id. Whenver an
   event occurs in a component, Prolog is notified about the id.
   Since Prolog maintains a database of all the components (pointered 
   to by breg0), it can get the id of the component.
*/
void cg_initialize() {
    BPLONG i;
    BPLONG comp, default_window;

    /* create a component for the default window */
    comp = ADDTAG(heap_top, STR);
    FOLLOW(heap_top) = (BPLONG)cg_component_psc;
    for (i = 1; i <= COMPONENT_ARITY; i++) {
        FOLLOW(heap_top+i) = (BPLONG)(heap_top+i);
    }
    FOLLOW(heap_top+COMPONENT_NO) = MAKEINT(0);
    heap_top += COMPONENT_ARITY+1;

    /* write_term(comp);printf("\n"); */
    /* create default window */
    default_window = ADDTAG(heap_top, STR);
    FOLLOW(heap_top) = (BPLONG)cg_window_psc;
    FOLLOW(heap_top+1) = comp;  /* the first arg is component */
    for (i = 2; i <= WINDOW_ARITY; i++) {
        FOLLOW(heap_top+i) = (BPLONG)(heap_top+i);
    }
    heap_top += WINDOW_ARITY+1;

    /* initialize CG global variables */
    FOLLOW(breg0+1) = default_window;
    /* write_term(default_window); */
    /* done in init.c
       for (i=2;i<=NUM_CG_GLOBALS;i++){
       FOLLOW(breg0+i) = (BPLONG)(breg0+i); 
       }
    */
#ifdef _WIN32
    InitializeCriticalSection(&csCriticalSection);
#else
    pthread_mutex_init(&csCriticalSection, NULL);
#endif
}

#ifdef JAVA
int c_cg_is_component() {
    BPLONG op = ARG(1, 1);
    BPLONG_PTR top;

    DEREF(op);
    return (ISSTRUCT(op) && cg_is_component(op));
}
#else
int c_cg_is_component() {
    return 0;
}
#endif

int cg_is_component(op)
    BPLONG op;
{
    BPLONG_PTR ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
    SYM_REC_PTR sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
    char *name = GET_NAME(sym_ptr);

    if (*name == '_') {
        return (sym_ptr == cg_component_psc ||
                sym_ptr == cg_window_psc ||
                sym_ptr == cg_arc_psc ||
                sym_ptr == cg_circle_psc ||
                sym_ptr == cg_image_psc ||
                sym_ptr == cg_line_psc ||
                sym_ptr == cg_oval_psc ||
                sym_ptr == cg_polygon_psc ||
                sym_ptr == cg_square_psc ||
                sym_ptr == cg_rectangle_psc ||
                sym_ptr == cg_roundRectangle_psc ||
                sym_ptr == cg_star_psc ||
                sym_ptr == cg_triangle_psc ||
                sym_ptr == cg_button_psc ||
                sym_ptr == cg_label_psc ||
                sym_ptr == cg_textbox_psc ||
                sym_ptr == cg_textField_psc ||
                sym_ptr == cg_textArea_psc ||
                /*          sym_ptr==cg_menuBar_psc || */
                sym_ptr == cg_menu_psc ||
                sym_ptr == cg_menuItem_psc ||
                sym_ptr == cg_checkboxMenuItem_psc ||
                sym_ptr == cg_checkbox_psc ||
                sym_ptr == cg_list_psc ||
                sym_ptr == cg_choice_psc ||
                sym_ptr == cg_scrollbar_psc ||
                sym_ptr == cg_fileDialog_psc);
    } else return 0;
}

void cg_init_sym() {  /* called init_sym */
    cg_component_psc = BP_NEW_SYM("_Component", 10);
    cg_window_psc = BP_NEW_SYM("_Window", 6);
    cg_arc_psc = BP_NEW_SYM("_Arc", 4);
    cg_circle_psc = BP_NEW_SYM("_Circle", 2);
    cg_image_psc = BP_NEW_SYM("_Image", 3);
    cg_line_psc = BP_NEW_SYM("_Line", 6);
    cg_oval_psc = BP_NEW_SYM("_Oval", 2);
    cg_polygon_psc = BP_NEW_SYM("_Polygon", 5);
    cg_square_psc = BP_NEW_SYM("_Square", 2);
    cg_rectangle_psc = BP_NEW_SYM("_Rectangle", 2);
    cg_roundRectangle_psc = BP_NEW_SYM("_RoundRectangle", 4);
    cg_star_psc = BP_NEW_SYM("_Star", 7);
    cg_triangle_psc = BP_NEW_SYM("_Triangle", 8);
    cg_button_psc = BP_NEW_SYM("_Button", 3);
    cg_label_psc = BP_NEW_SYM("_Label", 3);
    cg_textbox_psc = BP_NEW_SYM("_TextBox", 3);
    cg_textField_psc = BP_NEW_SYM("_TextField", 6);
    cg_textArea_psc = BP_NEW_SYM("_TextArea", 7);
    cg_menuBar_psc = BP_NEW_SYM("_MenuBar", 1);
    cg_menu_psc = BP_NEW_SYM("_Menu", 2);
    cg_menuItem_psc = BP_NEW_SYM("_MenuItem", 3);
    cg_checkboxMenuItem_psc = BP_NEW_SYM("_CheckboxMenuItem", 2);
    cg_checkbox_psc = BP_NEW_SYM("_Checkbox", 5);
    cg_list_psc = BP_NEW_SYM("_List", 3);
    cg_choice_psc = BP_NEW_SYM("_Choice", 3);
    cg_scrollbar_psc = BP_NEW_SYM("_Scrollbar", 1);
    cg_fileDialog_psc = BP_NEW_SYM("_FileDialog", 5);

    mouse_event_psc = BP_NEW_SYM("$mouse_event", 5);
    key_event_psc = BP_NEW_SYM("$key_event", 4);
    component_event_psc = BP_NEW_SYM("$component_event", 5);
    item_event_psc = BP_NEW_SYM("$item_event", 3);
    adjustment_event_psc = BP_NEW_SYM("$adjustment_event", 3);
}

int is_correct_event_source(event_no, source)
    BPLONG event_no;
    BPLONG source;
{
    BPLONG_PTR top;
    SYM_REC_PTR sym_ptr;

    DEREF(source);
    if (!ISSTRUCT(source)) return 0;
    sym_ptr = (SYM_REC_PTR)FOLLOW(UNTAGGED_ADDR(source));

    switch (event_no) {
    case ActionPerformed:
        return (sym_ptr == cg_button_psc || sym_ptr == cg_menuItem_psc || sym_ptr == cg_checkboxMenuItem_psc || cg_menu_psc || sym_ptr == cg_textField_psc
                || sym_ptr == cg_list_psc);

    case MouseClicked:
    case MousePressed:
    case MouseReleased:
    case MouseEntered:
    case MouseExited:
    case MouseMoved:
    case MouseDragged:
    case FocusGained:
    case FocusLost:
    case KeyPressed:
    case KeyReleased:
    case KeyTyped:
        return (sym_ptr == cg_button_psc || sym_ptr == cg_textField_psc || sym_ptr == cg_textArea_psc
                || sym_ptr == cg_list_psc || sym_ptr == cg_checkbox_psc || sym_ptr == cg_choice_psc
                || sym_ptr == cg_window_psc || sym_ptr == cg_fileDialog_psc);

    case WindowClosing:
    case WindowOpened:
    case WindowIconified:
    case WindowDeiconified:
    case WindowClosed:
    case WindowActivated:
    case WindowDeactivated:
    case ComponentMoved:
    case ComponentResized:
    case ComponentHidden:
    case ComponentShown:
        return (sym_ptr == cg_window_psc);


    case TextValueChanged:
        return (sym_ptr == cg_textField_psc || sym_ptr == cg_textArea_psc);

    case ItemStateChanged:
        return (sym_ptr == cg_checkbox_psc || sym_ptr == cg_checkboxMenuItem_psc || sym_ptr == cg_choice_psc || sym_ptr == cg_list_psc);

    case AdjustmentValueChanged:
        return (sym_ptr == cg_scrollbar_psc);

    case TimeOut:
        return (sym_ptr == timer_psc);
    default:
        fprintf(stderr, "Error: wrong event no %d", (int)event_no);
        return 0;
    }
}

BPLONG event_handler_type(frame)
    BPLONG_PTR frame;
{
    BPLONG_PTR ptr = (BPLONG_PTR)AR_REEP(frame)+4;  /* size of delay inst - 1*/
    if (FOLLOW(ptr) == trigger_cg_event_handler)
        return FOLLOW(ptr+1);
    else
        return 0;
}
/****************************************************************************
  All global data used in CGLIB are stored in the frame pointed to by breg0

            | sources of ItemStateChanged|
            |         ...                |
            | sources of ActionPerformed |
            | default_window             |   this slot was set in cg_initialize
  breg0 -> |                            |
*****************************************************************************/
/**look up the event handler**/
BPLONG cg_get_component_event_handler(comp)
    BPLONG comp;
{
    BPLONG_PTR ptr;
    BPLONG_PTR top;
    BPLONG handler;

    DEREF(comp);
    /* if (!ISSTRUCT(comp) || comp<0) return 0; */
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(comp);
    if (FOLLOW(ptr) == (BPLONG)timer_psc) {  /* $timer(compnent_no,event_var,interval,times,_) */
        handler = FOLLOW(ptr+2);
        DEREF(handler);
        return handler;
    } else if (FOLLOW(ptr) == (BPLONG)cg_component_psc) {
        handler = FOLLOW(ptr+10);  /* update this when the structure of 'cgComponent' is modified */
        DEREF(handler);
        return handler;
    } else {
        return cg_get_component_event_handler(FOLLOW(ptr+1));
    }
}

BPLONG cg_lookup_event_handler(type, comp_no)
    BPLONG type;
    BPLONG comp_no;
{
    BPLONG_PTR top, ptr;
    BPLONG comp;
    BPLONG list;

    // if (gc_is_working) {quit("Strange");}
    list = FOLLOW(breg0+(type-ActionPerformed+2));
    /*  printf("lookup event handler comp_no= %d gc_iw_working=%d\n",INTVAL(comp_no),gc_is_working); */
    DEREF(list);
    while (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        comp = FOLLOW(ptr);
        /*  write_term(cg_get_component_no(comp));printf("\n"); */
        if (cg_get_component_no(comp) == comp_no) {
            return cg_get_component_event_handler(comp);
        }
        list = FOLLOW(ptr+1);
        DEREF(list);
    }
    return 0;
}

BPLONG cg_lookup_component(type, comp_no)
    BPLONG type;
    BPLONG comp_no;
{
    BPLONG_PTR top, ptr;
    BPLONG comp;
    BPLONG list;
    // if (gc_is_working) {quit("Strange");}
    list = FOLLOW(breg0+(type-ActionPerformed+2));
    /*
      printf("lookup event handler comp_no= "); write_term(comp_no); 
      printf("list=%x,breg=%x breg->t=%x heap_top=%x trail_top=%x\n",list,breg,AR_T(breg),heap_top,trail_top);
    */
    DEREF(list);
    while (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        comp = FOLLOW(ptr);
        /*  write_term(cg_get_component_no(comp));printf("\n"); */
        if (cg_get_component_no(comp) == comp_no) {
            return comp;
        }
        list = FOLLOW(ptr+1);
        DEREF(list);
    }
    return 0;
}

BPLONG register_event_source(event_no, source)
    BPLONG event_no;
    BPLONG source;
{
    BPLONG comp_no;
    BPLONG res;
    /*
      printf("register_event_source %d",event_no); write_term(source); printf("\n"); 
    */
    DEREF(source);
    if (!is_correct_event_source(event_no, source)) {
        bp_exception = illegal_event;
        return BP_ERROR;
    }
    ENTER_CRITICAL_SECTION;
    comp_no = cg_get_component_no(source);
    attach_event_source(FOLLOW(breg0+(event_no-ActionPerformed+2)), source, comp_no, event_no);
    res = cg_get_component_event_handler(source);
    LEAVE_CRITICAL_SECTION;
    return res;
}

void attach_event_source(list, source, source_no, event_no)
    BPLONG list, source, source_no, event_no;
{
    BPLONG_PTR top, ptr;
    BPLONG comp, tmp;

    DEREF(list);
    while (ISLIST(list)) {
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(list);
        comp = FOLLOW(ptr);
        if (cg_get_component_no(comp) == source_no) return;
        list = FOLLOW(ptr+1);
        DEREF(list);
    }
    tmp = ADDTAG(heap_top, LST);
    FOLLOW(heap_top++) = source;
    NEW_HEAP_FREE;
    ASSIGN_v_heap_term(list, tmp);

    /*  printf("register_event_source comp_no=%d event_no=%d\n",INTVAL(source_no),event_no);  */
#ifdef JAVA
    javaRegisterEventListener(INTVAL(source_no), event_no);
#endif
}


/****/
int cg_get_default_window() {
    BPLONG window = ARG(1, 1);
    return unify(window, FOLLOW(breg0+1));
}

int c_cg_print_component() {
    BPLONG_PTR top;
    BPLONG op = ARG(1, 1);

    DEREF(op);
    cg_print_component(op);
    return BP_TRUE;
}

void cg_print_component(op)
    BPLONG op;
{
    BPLONG_PTR ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
    SYM_REC_PTR sym_ptr = (SYM_REC_PTR)FOLLOW(ptr);
    char *name = GET_NAME(sym_ptr);

    fprintf(curr_out, "%s", (name+1));
    fprintf(curr_out, "%d", (int)INTVAL(cg_get_component_no(op)));
}

/* the event handler is a domain variable in the component */
BPLONG cg_get_component_no(comp)
    BPLONG comp;
{
    BPLONG_PTR ptr;
    BPLONG_PTR top;
    BPLONG no;

    DEREF(comp);

    if (!ISSTRUCT(comp)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    ptr = (BPLONG_PTR)UNTAGGED_ADDR(comp);
    if (FOLLOW(ptr) == (BPLONG)timer_psc) {  /* $timer(component_no,event_var,interval,times,_) */
        no = FOLLOW(ptr+1);
        DEREF(no);
        return no;
    } if (FOLLOW(ptr) == (BPLONG)cg_component_psc) {
        /*  checkNoField(ptr+9,FOLLOW(ptr+9)); */
        no = FOLLOW(ptr+9);  /* update this when the structure of 'cgComponent' is modified */
        DEREF(no);
        return no;
    } else {
        return cg_get_component_no(FOLLOW(ptr+1));
    }
}

/* $timer(Component_no,EventVar,Interval,Status,_)
   fork a process that wakes up every Interval milliseconds
   and posts a time event.
*/
#define GET_TIMER_NO(timer_ptr) FOLLOW(timer_ptr+1)
#define GET_TIMER_EVENT_VAR(timer_ptr) FOLLOW(timer_ptr+2)
#define GET_TIMER_INTERVAL(timer_ptr) FOLLOW(timer_ptr+3)
#define GET_TIMER_STATUS_ADDR(timer_ptr) (timer_ptr+4)
#define GET_TIMER_STATUS(timer_ptr) FOLLOW(timer_ptr+4)
#define GET_TIMER_PID(timer_ptr) FOLLOW(timer_ptr+5)

#define TIMER_STATUS_STOP 0
#define TIMER_STATUS_RUN 1
#define TIMER_STATUS_DEAD 2

#ifdef _WIN32
DWORD WINAPI timerThread(LPVOID timer_no) {
#else
    void *timerThread(void *timer_no) {
#endif
        BPLONG component_no, interval, status;
        BPLONG_PTR ptr;
        BPLONG timer;

        component_no = *(BPLONG *)timer_no;
        /* must be synchronized with the garbage collector */
        for (; ; ) {
            ENTER_CRITICAL_SECTION;
            while (gc_is_working) bp_sleep(5);
            in_critical_region = 1;
            timer = cg_lookup_component(TimeOut, component_no);
            if (timer == 0) {
                in_critical_region = 0;
                LEAVE_CRITICAL_SECTION;
#ifdef _WIN32
                ExitThread(0);
#else
                pthread_exit(0);
#endif
            }
            DEREF(timer);
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(timer);
            interval = GET_TIMER_INTERVAL(ptr); DEREF(interval); interval = INTVAL(interval);

            in_critical_region = 0;
            LEAVE_CRITICAL_SECTION;

            bp_sleep(interval);

            ENTER_CRITICAL_SECTION;
            /* busy waiting, wait until gc is not working */
            while (gc_is_working) bp_sleep(5);
            in_critical_region = 1;
            timer = cg_lookup_component(TimeOut, component_no);
            if (timer == 0) {
                in_critical_region = 0;
                LEAVE_CRITICAL_SECTION;
                return 0;
            }
            DEREF(timer);
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(timer);
            status = GET_TIMER_STATUS(ptr); DEREF(status); status = INTVAL(status);

            if (status == TIMER_STATUS_RUN) {
                add_to_event_pool(component_no, TimeOut, (void *) MAKEINT(TimeOut));
            }
            in_critical_region = 0;
            LEAVE_CRITICAL_SECTION;

            if (status == TIMER_STATUS_DEAD) {
#ifdef _WIN32
                ExitThread(0);
#else
                pthread_exit(0);
#endif
            }
        }
    }

    /* timer is a term in the form $timer(comp_no,event_var,interval,status,_) */
    int c_timer() {
        BPLONG timer;
        BPLONG_PTR top, ptr;
        BPLONG timer_no;
        BPLONG_PTR timer_no_ptr;
#ifdef _WIN32
        BPLONG pid;
        HANDLE threadHandle;
#else
        pthread_t pid;
#endif
        in_critical_region = 1;
        timer = ARG(1, 1); DEREF(timer); ptr = (BPLONG_PTR)UNTAGGED_ADDR(timer);
        timer_no_ptr = (BPLONG_PTR)malloc(sizeof(BPLONG));
        timer_no = GET_TIMER_NO(ptr); DEREF(timer_no);
        register_event_source(TimeOut, timer);  /* register the timer as a CG component */
        FOLLOW(timer_no_ptr) = timer_no;

#ifdef _WIN32
        DWORD wpid;
        threadHandle = CreateThread(NULL, 1000, timerThread, timer_no_ptr, 0, &wpid);
        if (threadHandle == NULL) {
            quit("Failed to create a timer\n");
        }
        pid = wpid;
#else
        if (pthread_create( &pid, NULL, timerThread, (void*)timer_no_ptr) != 0) {
            quit("Failed to create a timer\n");
        }
        pthread_detach(pid);
#endif
        unify(GET_TIMER_PID(ptr), MAKE_INT_OR_BIGINT((BPLONG)pid));
        in_critical_region = 0;
        return BP_TRUE;
    }

    int c_kill_timer() {
        BPLONG timer;
        BPLONG_PTR ptr;

        /*
          #ifdef _WIN32
          BPLONG pid;
          #else
          pthread_t pid;
          #endif
        */

        timer = ARG(1, 1); DEREF(timer);
        ptr = (BPLONG_PTR)UNTAGGED_ADDR(timer);
        PUSHTRAIL_H_ATOMIC(GET_TIMER_STATUS_ADDR(ptr), GET_TIMER_STATUS(ptr));
        GET_TIMER_STATUS(ptr) = MAKEINT(TIMER_STATUS_DEAD);

        /*
          tmp = GET_TIMER_PID(ptr); DEREF(tmp);
          tmp = VALOF_INT_OR_BIGINT(tmp);
          #ifdef _WIN32
          pid = tmp;
          TerminateThread(pid);
          #else
          pid = (pthread_t)tmp;
          pthread_cancel(pid);
          #endif
        */
        return BP_TRUE;
    }

    int c_sleep() {
        BPLONG ms;
        ms = ARG(1, 1);
        DEREF(ms);
        ms = INTVAL(ms);
        bp_sleep(ms);
        return 1;
    }

    void add_to_event_pool(no, type, event_object)
        BPLONG no;
    BPLONG type;
    void *event_object;
    {
        if (event_count == EVENT_POOL_SIZE) {
            /* printf("event ignored %x\n",event_object); */
            if (!ISINT((BPLONG)event_object)) free(event_object);
            return;  /* ignore this event */
        }
        /* printf("added event to pool %d\n",INTVAL(no)); */
        event_pool[event_count].no = no;
        event_pool[event_count].type = type;
        event_pool[event_count].event_object = event_object;
        event_count++;
        toam_signal_vec |= EVENT_POOL_NONEMPTY;
    }

    /*
      void post_event_pool(){}
    */
    void post_event_pool() {
        BPLONG i;
        BPLONG no, type;
        BPLONG handler;
        void *eventObject;

        ENTER_CRITICAL_SECTION;
        for (i = 0; i < event_count; i++) {
            no = event_pool[i].no;
            type = event_pool[i].type;
            if ((handler = cg_lookup_event_handler(type, no)) == 0) continue;
            eventObject = event_pool[i].event_object;
            post_cg_event(handler, type, eventObject);
        }
        event_count = 0;
        toam_signal_vec &= ~EVENT_POOL_NONEMPTY;
        LEAVE_CRITICAL_SECTION;
    }

    void post_cg_event(handler, type, eo)
        BPLONG handler;
    BPLONG type;
    void *eo;
    {
        BPLONG_PTR sv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(handler);

        /*
          extern BPLONG no_gcs;
          printf("post %d\n",type);
          printf("GC = %d\n",no_gcs);
        */
        POST_EVENT(sv_ptr, type, eo);
    }

    int c_global_set_bpp() {
        BPLONG_PTR top;
        BPLONG op = ARG(1, 1);

        DEREF(op);
        bpp_java_obj = op;
        return 1;
    }

    int c_global_get_bpp() {
        BPLONG op = ARG(1, 1);

        return unify(op, bpp_java_obj);
    }

    int b_GET_ATTACHED_AGENTS_ccf(var, port, lists)
        BPLONG var, port, lists;
    {
        BPLONG_PTR dv_ptr;

        DEREF(var);
        if (!IS_SUSP_VAR(var)) {
            bp_exception = illegal_arguments;
            return -1;
        }
        dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
        DEREF(port); port = INTVAL(port);
        switch (port) {
        case EVENT_VAR_INS:
        case EVENT_DVAR_INS:
            ASSIGN_v_heap_term(lists, DV_ins_cs(dv_ptr));
            return BP_TRUE;

        case EVENT_DVAR_MINMAX:
            ASSIGN_v_heap_term(lists, DV_minmax_cs(dv_ptr));
            return BP_TRUE;

        case EVENT_DVAR_OUTER_DOM:
            ASSIGN_v_heap_term(lists, DV_outer_dom_cs(dv_ptr));
            return BP_TRUE;

        default:
            ASSIGN_v_heap_term(lists, DV_dom_cs(dv_ptr));
            return BP_TRUE;
        }
    }

    int b_AGENT_OCCUR_IN_CONJUNCTIVE_CHANNELS_cc(frame_offset, lists)
        BPLONG frame_offset, lists;
    {
        BPLONG_PTR ptr1, ptr2;
        BPLONG lst;

        DEREF(lists);
        /*  printf("occur_in_conjunctive_channels %d",INTVAL(frame_offset));write_term(lists);printf("\n"); */
        while (ISLIST(lists)) {
            ptr1 = (BPLONG_PTR)UNTAGGED_ADDR(lists);

            lst = FOLLOW(ptr1);
            while (ISLIST(lst)) {  /* no dereference is necessary for DV_cs...*/
                ptr2 = (BPLONG_PTR)UNTAGGED_ADDR(lst);
                if (frame_offset == FOLLOW(ptr2)) goto ctn;
                lst = FOLLOW(ptr2+1);
            }
            return BP_FALSE;

        ctn:
            lists = FOLLOW(ptr1+1);
            DEREF(lists);
        }
        return BP_TRUE;
    }


    int b_AGENT_OCCUR_IN_DISJUNCTIVE_CHANNELS_cc(frame_offset, lists)
        BPLONG frame_offset, lists;
    {
        BPLONG_PTR ptr1, ptr2;
        BPLONG lst;

        DEREF(lists);
        /*  printf("occur_in_conjunctive_channels %d",INTVAL(frame_offset));write_term(lists);printf("\n"); */
        while (ISLIST(lists)) {
            ptr1 = (BPLONG_PTR)UNTAGGED_ADDR(lists);

            lst = FOLLOW(ptr1);
            while (ISLIST(lst)) {  /* no dereference is necessary for DV_cs...*/
                ptr2 = (BPLONG_PTR)UNTAGGED_ADDR(lst);
                if (frame_offset == FOLLOW(ptr2)) return BP_TRUE;
                lst = FOLLOW(ptr2+1);
            }
            lists = FOLLOW(ptr1+1);
            DEREF(lists);
        }
        return BP_FALSE;
    }

    int c_start_critical_region() {
        in_critical_region = 1;
        return 1;
    }

    int c_end_critical_region() {
        in_critical_region = 0;
        return 1;
    }

    int c_in_critical_region() {
        return in_critical_region == 1;
    }

