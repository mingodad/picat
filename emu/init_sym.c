/********************************************************************
 *   File   : init_sym.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "inst.h"
#include "term.h"
#include <string.h>

extern BPLONG interrupt_sym;
extern SYM_REC_PTR ball_psc;

void init_sym() {

    nil_sym = ADDTAG(insert_sym("[]", 2, 0), ATM);
    period_sym = ADDTAG(insert_sym(".", 1, 0), ATM);
    call_damon_load_atom = ADDTAG(insert_sym("$call_damon_load", 16, 0), ATM);

    list_psc = insert_sym(".", 1, 2);
    list_psc_int = ADDTAG((BPLONG)list_psc, INT_TAG);
#ifdef M64BITS
    list_psc_hashcode = ((BPLONG)list_psc & HASH_BITS) >> 3;
#else
    list_psc_hashcode = ((BPLONG)list_psc & HASH_BITS) >> 2;
#endif

    v2_psc = insert_sym("v", 1, 2);
    v3_psc = insert_sym("v", 1, 3);
    v4_psc = insert_sym("v", 1, 4);
    findall_result0 = insert_sym("$findall_result", 15, 0);
    findall_result1 = insert_sym("$findall_result", 15, 1);
    findall_result2 = insert_sym("$findall_result", 15, 2);
    set_constructor_psc = BP_NEW_SYM("{}", 1);
    interval_psc = BP_NEW_SYM("..", 2);
    comma_psc = insert_sym(",", 1, 2);
    call_cleanup_psc = BP_NEW_SYM("_$call_cleanup_call", 0);
    float_psc = insert_sym("$float", 6, 3);
    bigint_psc = insert_sym("$bigint", 7, 2);
#ifdef XCSP_PICAT
	xcsp_vx_psc = insert_sym("_$xcsp_vx", 9, 1);
#endif
    address_psc = insert_sym("$address", 8, 2);  //branch
    socket_psc = insert_sym("$socket", 7, 1);  //branch
    stream_psc = insert_sym("$stream", 7, 1);  //branch

    timer_psc = BP_NEW_SYM("$timer", 5);
    enter_dyn_call = insert_sym("$trace_call", 11, 1);
    enter_catch_call = BP_NEW_SYM("$catch_call", 1);
    damon_load = insert_sym("$damon_load", 11, 0);
    slash_psc = insert_sym("/", 1, 2);
    vv_eq_c_arc = BP_NEW_SYM("vv_eq_c_ARC", 3);
    v_eq_vc_arc = BP_NEW_SYM("v_eq_vc_ARC", 3);
    uu_eq_c_arc = BP_NEW_SYM("uu_eq_c_ARC", 5);
    u_eq_uc_arc = BP_NEW_SYM("u_eq_uc_ARC", 5);
    plan_psc = BP_NEW_SYM("_$plan", 2);
    register_event_listener = BP_NEW_SYM("registerEventListener", 2);
    picat_log_psc = BP_NEW_SYM("_$picat_log", 0);

    windows_atom = ADDTAG(insert_sym("windows", 7, 0), ATM);
    cygwin_atom = ADDTAG(insert_sym("cygwin", 6, 0), ATM);
    unix_atom = ADDTAG(insert_sym("unix", 4, 0), ATM);

    thread_psc = BP_NEW_SYM("$thread", 4);
    hashtable_psc = BP_NEW_SYM("$hshtb", 2);
    ghashtable_psc = BP_NEW_SYM("$ghshtb", 2);
    thashtable_psc = BP_NEW_SYM("$thshtb", 2);
    attr_cfd_atm = ADDTAG(BP_NEW_SYM("_$attr_cfd", 0), ATM);
    cfd_func_pair = BP_NEW_SYM("_$func_pair", 3);
    attr_neq_atm = ADDTAG(BP_NEW_SYM("_$attr_neq", 0), ATM);
    thread_start_psc = BP_NEW_SYM("$thread_start", 2);
    unknown_exception = ADDTAG(BP_NEW_SYM("unknown_exception", 0), ATM);
    handle_exception_psc = BP_NEW_SYM("handle_exception", 2);
    forward_exception_psc = BP_NEW_SYM("$forward_exception", 1);
    c_object_ref_sym = BP_NEW_SYM("$addr", 1);
    star_atom = ADDTAG(BP_NEW_SYM("*", 0), ATM);

    interrupt_sym = ADDTAG(BP_NEW_SYM("interrupt", 0), ATM);
    ball_psc = BP_NEW_SYM("$ball", 0);
    equal_psc = BP_NEW_SYM("=", 2);
    planner_explored_depth_psc = BP_NEW_SYM("_$planner_explored_depth", 0);

    eof_atom = ADDTAG(BP_NEW_SYM("end_of_file", 0), ATM);
    true_atom = ADDTAG(BP_NEW_SYM("true", 0), ATM);
    t_atom = ADDTAG(BP_NEW_SYM("t", 0), ATM);
    f_atom = ADDTAG(BP_NEW_SYM("f", 0), ATM);
    dec_ref_atom = ADDTAG(BP_NEW_SYM("_$dec_ref", 0), ATM);
    dec_retr_atom = ADDTAG(BP_NEW_SYM("_$dec_retr", 0), ATM);
    dots_atom = ADDTAG(BP_NEW_SYM("...", 0), ATM);
    dummy_event_object = ADDTAG(BP_NEW_SYM("$dummy_event_obj", 0), ATM);
    failure_atom = ADDTAG(BP_NEW_SYM("failure", 0), ATM);
    empty_set = ADDTAG(BP_NEW_SYM("{}", 0), ATM);
    not_care_symbol = empty_set;
    atom_expected = ADDTAG(BP_NEW_SYM("atom_expected", 0), ATM);
    string_expected = ADDTAG(BP_NEW_SYM("string_expected", 0), ATM);
    invalid_number_format = ADDTAG(BP_NEW_SYM("invalid_number_format", 0), ATM);
    integer_expected = ADDTAG(BP_NEW_SYM("integer_expected", 0), ATM);
    number_expected = ADDTAG(BP_NEW_SYM("number_expected", 0), ATM);
    no_number_expected = ADDTAG(BP_NEW_SYM("no_number_expected", 0), ATM);
    arith_expr_expected = ADDTAG(BP_NEW_SYM("arith_expr_expected", 0), ATM);
    list_expected = ADDTAG(BP_NEW_SYM("list_expected", 0), ATM);
    structure_expected = ADDTAG(BP_NEW_SYM("structure_expected", 0), ATM);
    compound_expected = ADDTAG(BP_NEW_SYM("compund_expected", 0), ATM);
    char_code_expected = ADDTAG(BP_NEW_SYM("char_code_expected", 0), ATM);
    char_expected = ADDTAG(BP_NEW_SYM("char_expected", 0), ATM);
    ground_expected = ADDTAG(BP_NEW_SYM("ground_expected", 0), ATM);
    float_format_expected = ADDTAG(BP_NEW_SYM("float_format_expected", 0), ATM);
    operator_type_expected = ADDTAG(BP_NEW_SYM("operator_type_expected", 0), ATM);
    variable_expected = ADDTAG(BP_NEW_SYM("variable_expected", 0), ATM);
    nonvariable_expected = ADDTAG(BP_NEW_SYM("nonvariable_expected", 0), ATM);
    input_stream_expected = ADDTAG(BP_NEW_SYM("input_stream_expected", 0), ATM);
    output_stream_expected = ADDTAG(BP_NEW_SYM("output_stream_expected", 0), ATM);
    could_not_open_stream = ADDTAG(BP_NEW_SYM("could_not_open_stream", 0), ATM);
    file_does_not_exist = ADDTAG(BP_NEW_SYM("file_does_not_exist", 0), ATM);
    divide_by_zero = ADDTAG(BP_NEW_SYM("divide_by_zero", 0), ATM);
    illegal_expression = ADDTAG(BP_NEW_SYM("invalid_expression", 0), ATM);
    illegal_timer = ADDTAG(BP_NEW_SYM("invalid_timer", 0), ATM);
    illegal_list = ADDTAG(BP_NEW_SYM("invalid_list", 0), ATM);
    illegal_mode_string = ADDTAG(BP_NEW_SYM("invalid_mode_string", 0), ATM);
    illegal_predicate = ADDTAG(BP_NEW_SYM("invalid_predicate", 0), ATM);
    not_modifiable = ADDTAG(BP_NEW_SYM("not_modifiable", 0), ATM);
    out_of_range = ADDTAG(BP_NEW_SYM("out_of_range", 0), ATM);
    bp_out_of_memory_atom = ADDTAG(BP_NEW_SYM("out_of_memory", 0), ATM);
    bp_out_of_memory_stack_heap = ADDTAG(BP_NEW_SYM("stack_heap", 0), ATM);
    bp_out_of_memory_table = ADDTAG(BP_NEW_SYM("table", 0), ATM);
    undefined_predicate = ADDTAG(BP_NEW_SYM("undefined_predicate", 0), ATM);
    invalid_byte_file = ADDTAG(BP_NEW_SYM("invalid_byte_file", 0), ATM);
    code_area_overflow = ADDTAG(BP_NEW_SYM("code_area_overflow", 0), ATM);
    control_stack_overflow = ADDTAG(BP_NEW_SYM("control_stack_overflow", 0), ATM);
    integer_overflow = ADDTAG(BP_NEW_SYM("integer_overflow", 0), ATM);
    trail_stack_overflow = ADDTAG(BP_NEW_SYM("trail_stack_overflow", 0), ATM);
    permission_error = ADDTAG(BP_NEW_SYM("permission_error", 0), ATM);
    java_exception = ADDTAG(BP_NEW_SYM("java_exception", 0), ATM);
    bp_initialization_error = ADDTAG(BP_NEW_SYM("bp_initialization_error", 0), ATM);
    run_time_error = ADDTAG(BP_NEW_SYM("run_time_error", 0), ATM);

    illegal_arguments = ADDTAG(BP_NEW_SYM("invalid_argument", 0), ATM);
    out_of_bound = ADDTAG(BP_NEW_SYM("out_of_bound", 0), ATM);
    illegal_event = ADDTAG(BP_NEW_SYM("invalid_event_pattern", 0), ATM);

    output_mode_error = ADDTAG(BP_NEW_SYM("output_mode_error", 0), ATM);

    init_arith_sym();

    cg_init_sym();
    main_args = nil_sym;

    init_error_sym();

    init_char_sym();
}

void init_error_sym() {
    str_BUILTIN_ERROR1 = BP_NEW_SYM("type_args", 2);
    str_BUILTIN_ERROR2 = BP_NEW_SYM("type_args", 3);
    str_BUILTIN_ERROR3 = BP_NEW_SYM("type_args", 4);
    str_BUILTIN_ERROR4 = BP_NEW_SYM("type_args", 5);
    str_DOMAIN_ERROR = BP_NEW_SYM("domain_error", 2);
    str_EVALUATION_ERROR = BP_NEW_SYM("evaluation_error", 2);
    str_UPDATE_ERROR = BP_NEW_SYM("update_error", 1);
    str_EXISTENCE_ERROR = BP_NEW_SYM("existence_error", 2);
    str_PERMISSION_ERROR = BP_NEW_SYM("permission_error", 3);
    str_REPRESENTATION_ERROR = BP_NEW_SYM("representation_error", 1);
    str_RESOURCE_ERROR = BP_NEW_SYM("resource_error", 1);
    str_TYPE_ERROR = BP_NEW_SYM("type_error", 2);
    str_SYNTAX_ERROR = BP_NEW_SYM("syntax_error", 1);
    error_psc = BP_NEW_SYM("error", 2);

    et_UPDATE = ADDTAG(BP_NEW_SYM("setarg", 0), ATM);
    et_ACCESS = ADDTAG(BP_NEW_SYM("access", 0), ATM);
    et_ATOM = ADDTAG(BP_NEW_SYM("atom", 0), ATM);
    et_ATOMIC = ADDTAG(BP_NEW_SYM("atomic", 0), ATM);
    et_BINARY_STREAM = ADDTAG(BP_NEW_SYM("binary_stream", 0), ATM);
    et_BYTE = ADDTAG(BP_NEW_SYM("byte", 0), ATM);
    et_CALLABLE = ADDTAG(BP_NEW_SYM("callable", 0), ATM);
    et_CHARACTER_CODE = ADDTAG(BP_NEW_SYM("character_code", 0), ATM);
    et_CHARACTER = ADDTAG(BP_NEW_SYM("character", 0), ATM);
    et_COMPOUND = ADDTAG(BP_NEW_SYM("compound", 0), ATM);
    et_CREATE = ADDTAG(BP_NEW_SYM("create", 0), ATM);
    et_EVALUABLE = ADDTAG(BP_NEW_SYM("evaluable", 0), ATM);
    et_FLAG = ADDTAG(BP_NEW_SYM("flag", 0), ATM);
    et_FLAG_VALUE = ADDTAG(BP_NEW_SYM("flag_value", 0), ATM);
    et_FLOAT_OVERFLOW = ADDTAG(BP_NEW_SYM("float_overflow", 0), ATM);
    et_INPUT = ADDTAG(BP_NEW_SYM("input", 0), ATM);
    et_INTEGER = ADDTAG(BP_NEW_SYM("integer", 0), ATM);
    et_INT_OVERFLOW = ADDTAG(BP_NEW_SYM("int_overflow", 0), ATM);
    et_IN_BYTE = ADDTAG(BP_NEW_SYM("in_byte", 0), ATM);
    et_IN_CHARACTER = ADDTAG(BP_NEW_SYM("in_character", 0), ATM);
    et_IN_ESCCHARACTER = ADDTAG(BP_NEW_SYM("wrong_escape_character", 0), ATM);
    et_IN_CHARACTER_CODE = ADDTAG(BP_NEW_SYM("in_character_code", 0), ATM);
    et_INSTANTIATION_ERROR = ADDTAG(BP_NEW_SYM("instantiation_error", 0), ATM);
    et_IO_MODE = ADDTAG(BP_NEW_SYM("io_mode", 0), ATM);
    et_LIST = ADDTAG(BP_NEW_SYM("list", 0), ATM);
    et_MAX_ARITY = ADDTAG(BP_NEW_SYM("max_arity", 0), ATM);
    et_MAX_INTEGER = ADDTAG(BP_NEW_SYM("max_integer", 0), ATM);
    et_MODIFY = ADDTAG(BP_NEW_SYM("modify", 0), ATM);
    et_NON_EMPTY_LIST = ADDTAG(BP_NEW_SYM("non_empty_list", 0), ATM);
    et_NOT_LESS_THAN_ZERO = ADDTAG(BP_NEW_SYM("not_less_than_zero", 0), ATM);
    et_NUMBER = ADDTAG(BP_NEW_SYM("number", 0), ATM);
    et_OPEN = ADDTAG(BP_NEW_SYM("open", 0), ATM);
    et_OPERATOR = ADDTAG(BP_NEW_SYM("operator", 0), ATM);
    et_OPERATOR_PRIORITY = ADDTAG(BP_NEW_SYM("operator_priority", 0), ATM);
    et_OUTPUT = ADDTAG(BP_NEW_SYM("output", 0), ATM);
    et_PAST_END_OF_STREAM = ADDTAG(BP_NEW_SYM("past_end_of_stream", 0), ATM);
    et_PREDICATE_INDICATOR = ADDTAG(BP_NEW_SYM("predicate_indicator", 0), ATM);
    et_PRIVATE_PROCEDURE = ADDTAG(BP_NEW_SYM("private_procedure", 0), ATM);
    et_PROLOG_FLAG = ADDTAG(BP_NEW_SYM("prolog_flag", 0), ATM);
    et_PROCEDURE = ADDTAG(BP_NEW_SYM("procedure", 0), ATM);
    et_READ_OPTION = ADDTAG(BP_NEW_SYM("read_option", 0), ATM);
    et_REPOSITION = ADDTAG(BP_NEW_SYM("reposition", 0), ATM);
    et_SOURCE_SINK = ADDTAG(BP_NEW_SYM("source_sink", 0), ATM);
    et_STATIC_PROCEDURE = ADDTAG(BP_NEW_SYM("static_procedure", 0), ATM);
    et_STREAM = ADDTAG(BP_NEW_SYM("stream", 0), ATM);
    et_STREAM_OPTION = ADDTAG(BP_NEW_SYM("stream_option", 0), ATM);
    et_STREAM_OR_ALIAS = ADDTAG(BP_NEW_SYM("stream_or_alias", 0), ATM);
    et_STREAM_PROPERTY = ADDTAG(BP_NEW_SYM("stream_property", 0), ATM);
    et_TEXT_STREAM = ADDTAG(BP_NEW_SYM("text_stream", 0), ATM);
    et_UNDEFINED = ADDTAG(BP_NEW_SYM("undefined", 0), ATM);
    et_UNDERFLOW = ADDTAG(BP_NEW_SYM("underflow", 0), ATM);
    et_VARIABLE = ADDTAG(BP_NEW_SYM("variable", 0), ATM);
    et_WRITE_OPTION = ADDTAG(BP_NEW_SYM("write_option", 0), ATM);
    et_ZERO_DIVISOR = ADDTAG(BP_NEW_SYM("zero_divisor", 0), ATM);
    et_BIGINT_TOO_LONG = ADDTAG(BP_NEW_SYM("bigint_too_long", 0), ATM);
    et_STRING_TOO_LONG = ADDTAG(BP_NEW_SYM("string_too_long", 0), ATM);

    {
        BPLONG_PTR ptr;

        ALIGN(CHAR_PTR, curr_fence);
        ptr = (BPLONG_PTR)curr_fence;
        et_OUT_OF_MEMORY = ADDTAG((BPLONG)ptr, STR);  /* resource_error(out_of_memory) */
        FOLLOW(ptr++) = (BPLONG)str_RESOURCE_ERROR;
        FOLLOW(ptr++) = bp_out_of_memory_atom;

        et_OUT_OF_MEMORY_STACK = ADDTAG((BPLONG)ptr, STR);  /* error(_,_) */
        FOLLOW(ptr++) = (BPLONG)error_psc;
        FOLLOW(ptr++) = et_OUT_OF_MEMORY;
        FOLLOW(ptr++) = bp_out_of_memory_stack_heap;

        et_OUT_OF_MEMORY_TABLE = ADDTAG((BPLONG)ptr, STR);  /* error(_,_) */
        FOLLOW(ptr++) = (BPLONG)error_psc;
        FOLLOW(ptr++) = et_OUT_OF_MEMORY;
        FOLLOW(ptr++) = bp_out_of_memory_table;

        curr_fence = (CHAR_PTR)ptr;
    }
}

void init_char_sym() {
    int i;
    char c;
    for (i = 1; i < AlphabetSize; i++) {
        c = (char)i;
        char_sym_table[i] = ADDTAG(insert_sym(&c, 1, 0), ATM);
    }
}






