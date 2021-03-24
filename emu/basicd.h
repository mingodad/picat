/********************************************************************
 *   File   : basicd.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2021

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
/*
  #ifdef TRACE_INSTS
  int exec_trace[10];
  #endif
*/
int bp_gc = 1;
int old_bp_gc;
int redefine_warning = 0;

int confirm_copy_right = 1;

#ifdef LINUX
BPLONG addr_top_bit = -1LL;  /* will be changed to 1000... or 0 later */
#endif

int use_tabling = 0;
BPLONG_PTR subgoalTable;
BPLONG subgoalTableBucketSize = 7919;
BPLONG subgoalTableEntriesCount;
BPLONG InitAnswerTableBucketSize = 7;

int curr_toam_status = TOAM_NOTSET;

BPLONG global_call_number = 0;
BPLONG number_of_spy_points = 0;
UW16 dg_flag_word;

FILE *curr_in, *curr_out;

BPLONG n_backtracks = 0;
BPLONG_PTR stack_low_addr;
BPLONG_PTR parea_low_addr = NULL;
BPLONG_PTR parea_water_mark;
BPLONG_PTR trail_low_addr;
BPLONG_PTR trail_water_mark;
BPLONG_PTR trail_water_mark0;

BPLONG_PTR stack_up_addr;
BPLONG_PTR stack_low_addr;
BPLONG_PTR trail_up_addr;

BPLONG_PTR cpreg;
CHAR_PTR curr_fence;  /* ptr to next free byte in perm space */
BPLONG_PTR parea_up_addr;  /* ptr to last+1 free byte in perm space */
BPLONG_PTR inst_begin;  /* ptr to the beginning of inst. array */
BPLONG_PTR addr_halt;
BPLONG_PTR addr_halt0;
BPLONG_PTR addr_fail;
BPLONG_PTR addr_table_consume;

BPLONG_PTR gc_low;
BPLONG_PTR gc_upper;
int gc_is_working;
BPLONG_PTR copy_area_low, copy_area_high;

BPLONG_PTR top;
BPLONG_PTR arreg = NULL;  /* latest activation record       */
BPLONG_PTR local_top;
BPLONG_PTR local_top0;
BPLONG_PTR breg = NULL;  /* latest choice point            */
BPLONG_PTR breg0;  /* choice point where global variables for cglib are stored */
BPLONG_PTR heap_top;  /* top of heap                  */
BPLONG_PTR trail_top;  /* top of trail stack           */
BPLONG_PTR hbreg;  /* heap back track point        */
BPLONG_PTR sfreg = NULL;  /* latest suspension frame      */
BPLONG_PTR gc_b;

BPLONG_PTR triggeredCs[MAXTRIGGERS];
int event_flag[MAXTRIGGERS];
BPLONG_PTR triggering_frame[MAXTRIGGERS];
BPLONG event_object[MAXTRIGGERS];  /* dom(X,E), etc. */
BPLONG trigger_no = 0;  /* list of woken frames         */
BPULONG toam_signal_vec;
int user_signal;
EVENT_FUNC event_func;
int in_critical_region = 0;
BPLONG fd_region_low;
BPLONG fd_region_up;

SYM_REC_PTR hash_table[BUCKET_CHAIN];
BPLONG char_sym_table[AlphabetSize];
BPLONG number_of_symbols = 0;
BPLONG curr_line_no = 1 ;

int disassem;
BPLONG num_line;  /* print instruction addresses on trace and disassem */
int trace_toam = 0;

BPLONG global_var_num;
int number_var_exception = 0;
BPLONG global_unnumbervar_max;
BPLONG_PTR global_unnumbervar_ptr;
BPLONG_PTR global_unnumbervar_watermark;

BPLONG bp_exception;
BPLONG foreign_exception = (BPLONG) NULL;
SYM_REC_PTR list_psc;
SYM_REC_PTR equal_psc;
SYM_REC_PTR planner_explored_depth_psc;
BPLONG list_psc_int;
BPLONG list_psc_hashcode;
SYM_REC_PTR error_psc;
SYM_REC_PTR v2_psc;
SYM_REC_PTR v3_psc;
SYM_REC_PTR v4_psc;
SYM_REC_PTR findall_result0;
SYM_REC_PTR findall_result1;
SYM_REC_PTR findall_result2;
SYM_REC_PTR interval_psc;
SYM_REC_PTR call_cleanup_psc;
SYM_REC_PTR plan_psc;
SYM_REC_PTR comma_psc;
SYM_REC_PTR set_constructor_psc;
SYM_REC_PTR float_psc;
SYM_REC_PTR address_psc;
SYM_REC_PTR socket_psc;
SYM_REC_PTR stream_psc;
SYM_REC_PTR bigint_psc;
SYM_REC_PTR timer_psc;
SYM_REC_PTR enter_dyn_call;
SYM_REC_PTR enter_catch_call;
SYM_REC_PTR damon_load;
SYM_REC_PTR slash_psc;
SYM_REC_PTR handle_exception_psc;
SYM_REC_PTR forward_exception_psc;
SYM_REC_PTR vv_eq_c_arc;
SYM_REC_PTR v_eq_vc_arc;
SYM_REC_PTR uu_eq_c_arc;
SYM_REC_PTR u_eq_uc_arc;
SYM_REC_PTR register_event_listener;

/* $addr(int) */
SYM_REC_PTR c_object_ref_sym;
SYM_REC_PTR picat_log_psc;

BPLONG windows_atom;
BPLONG cygwin_atom;
BPLONG unix_atom;

BPLONG unknown_exception;
BPLONG star_atom;
BPLONG eof_atom;
BPLONG true_atom;
BPLONG t_atom;
BPLONG f_atom;
BPLONG dec_ref_atom;
BPLONG dec_retr_atom;
BPLONG dummy_event_object;
BPLONG failure_atom;
BPLONG empty_set;
BPLONG not_care_symbol;
BPLONG nil_sym, period_sym, varid;
BPLONG atom_expected;
BPLONG string_expected;
BPLONG input_stream_expected;
BPLONG output_stream_expected;
BPLONG integer_expected;
BPLONG invalid_number_format;
BPLONG number_expected;
BPLONG no_number_expected;
BPLONG arith_expr_expected;
BPLONG list_expected;
BPLONG structure_expected;
BPLONG compound_expected;
BPLONG char_code_expected;
BPLONG char_expected;
BPLONG ground_expected;
BPLONG float_format_expected;
BPLONG operator_type_expected;
BPLONG variable_expected;
BPLONG nonvariable_expected;
BPLONG could_not_open_stream;
BPLONG file_does_not_exist;
BPLONG divide_by_zero;
BPLONG illegal_expression;
BPLONG illegal_timer;
BPLONG illegal_list;
BPLONG illegal_mode_string;
BPLONG illegal_predicate;
BPLONG not_modifiable;
BPLONG out_of_range;
BPLONG bp_out_of_memory_atom;
BPLONG bp_out_of_memory_stack_heap;
BPLONG bp_out_of_memory_table;
BPLONG undefined_predicate;
BPLONG invalid_byte_file;
BPLONG code_area_overflow;
BPLONG permission_error;
BPLONG control_stack_overflow;
BPLONG trail_stack_overflow;
BPLONG integer_overflow;
BPLONG illegal_arguments;
BPLONG out_of_bound;
BPLONG illegal_event;
BPLONG output_mode_error;
BPLONG java_exception;
BPLONG bp_initialization_error;
BPLONG run_time_error;
BPLONG call_damon_load_atom;
BPLONG dots_atom;

int debugging_susp = 0;

BPLONG main_args;

BPLONG eolcom_flag = 1;
BPLONG lastc = ' ';  /* previous character */

/* for threads */
SYM_REC_PTR thread_psc;
SYM_REC_PTR thread_start_psc;
SYM_REC_PTR cfd_func_pair;
BPLONG attr_cfd_atm;
BPLONG attr_neq_atm;
SYM_REC_PTR hashtable_psc;
SYM_REC_PTR ghashtable_psc;
SYM_REC_PTR thashtable_psc;
BPLONG thread_table_overflow;
BPLONG_PTR bp_threads;
BPLONG thread_table_size;
BPLONG thread_count;
BPLONG thread_current_no;
BPLONG thread_interruptable;

#ifdef BPSOLVER
BPLONG use_gl_getline = 0;
#else
BPLONG use_gl_getline = 1;
#endif
BPLONG num_stack_expansions = 0;
BPLONG num_trail_expansions = 0;
BPLONG num_parea_expansions = -1;

SYM_REC_PTR
str_BUILTIN_ERROR1,
    str_BUILTIN_ERROR2,
    str_BUILTIN_ERROR3,
    str_BUILTIN_ERROR4,
    str_DOMAIN_ERROR,
    str_UPDATE_ERROR,
    str_EVALUATION_ERROR,
    str_EXISTENCE_ERROR,
    str_PERMISSION_ERROR,
    str_REPRESENTATION_ERROR,
    str_RESOURCE_ERROR,
    str_TYPE_ERROR,
    str_SYNTAX_ERROR;

BPLONG
et_OUT_OF_MEMORY,
    et_OUT_OF_MEMORY_STACK,
    et_OUT_OF_MEMORY_TABLE,
    et_UPDATE,
    et_ACCESS,
    et_ATOM,
    et_ATOMIC,
    et_BINARY_STREAM,
    et_BYTE,
    et_CALLABLE,
    et_CHARACTER,
    et_CHARACTER_CODE,
    et_COMPOUND,
    et_CREATE,
    et_EVALUABLE,
    et_FLAG,
    et_FLAG_VALUE,
    et_FLOAT_OVERFLOW,
    et_INPUT,
    et_INTEGER,
    et_INT_OVERFLOW,
    et_IN_BYTE,
    et_IN_CHARACTER,
    et_IN_ESCCHARACTER,
    et_IN_CHARACTER_CODE,
    et_INSTANTIATION_ERROR,
    et_IO_MODE,
    et_LIST,
    et_MAX_ARITY,
    et_MAX_INTEGER,
    et_MODIFY,
    et_NON_EMPTY_LIST,
    et_NOT_LESS_THAN_ZERO,
    et_NUMBER,
    et_OPEN,
    et_OPERATOR,
    et_OPERATOR_PRIORITY,
    et_OUTPUT,
    et_PAST_END_OF_STREAM,
    et_PREDICATE_INDICATOR,
    et_PRIVATE_PROCEDURE,
    et_PROCEDURE,
    et_PROLOG_FLAG,
    et_READ_OPTION,
    et_REPOSITION,
    et_SOURCE_SINK,
    et_STATIC_PROCEDURE,
    et_STREAM,
    et_STREAM_OPTION,
    et_STREAM_OR_ALIAS,
    et_STREAM_PROPERTY,
    et_TEXT_STREAM,
    et_UNDEFINED,
    et_UNDERFLOW,
    et_VARIABLE,
    et_WRITE_OPTION,
    et_ZERO_DIVISOR,
    et_BIGINT_TOO_LONG,
    et_STRING_TOO_LONG;
