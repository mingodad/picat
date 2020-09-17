switch (current_opcode) {
case noop:
case cut_return:
case return_det:
case return_nondet:
case cut:
case cut0:
case fail:
case cut_fail:
case fail0:
case halt:
case halt0:
case para_w:
case para_nil:
case fetch_w:
case fetch_2w:
case fetch_3w:
case fetch_4w:
case fetch_910:
case fetch_45:
case unify_arg_w:
case unify_arg_2w:
case unify_arg_3w:
case unify_arg_4w:
case unify_arg_5w:
case build_arg_5w:
case build_arg_4w:
case build_arg_3w:
case build_arg_2w:
case build_arg_w:
case catch_clean_up:
case return_commit:
case return_delay:
case end_delay:
case v_in_cv_int:
case v_in_vc_int:
case u_in_cu_int:
case u_in_uc_int:
case table_produce:
case table_consume:
case table_neck_no_reeval:
case last_tabled_call:
case table_neck:
case table_eager_consume:
case set_catcher_frame:
    break;
case noop1:
case nondet:
case fetch_ws:
case trigger_ins_min_max:
case interval_consistent_eq:
case interval_consistent_eq_nocoe:
case interval_consistent_ge:
case call_binary_constr_eq:
case bcp:
case table_set_new_bit:
case bcp1:
case tabsize:
    LoadLiteralFromCArray;
    break;
case allocate_det_b:
case allocate_det:
case allocate_nondet:
case allocate_susp:
case table_allocate:
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    LoadStructFromCArray;
    LoadLiteralFromCArray;
    break;
case unify_constant_return_det:
case unify_constant_return_nondet:
case para_uc:
case unify_constant:
case unify_arg_v0c:
case build_arg_v0c:
case move_constant:
    LoadYFromCArray;
    LoadConstantFromCArray;
    break;
case cut_unify_value_return_det:
case unify_value_return_det:
case unify_value_return_nondet:
case unify_value_cut:
case para_uv:
case para_uuw:
case para_vv:
case para_uu:
case fetch_2v:
case unify_arg_v0u:
case unify_arg_uv0:
case unify_arg_2v0:
case unify_arg_2u:
case build_arg_v0u:
case build_arg_uv0:
case build_arg_2v0:
case build_arg_2u:
case unify_cons_u:
case unify_cons_v0:
case unify_cons_uw:
case unify_cons_v0w:
case unify_cons_wv0:
case unify_value:
case move_value:
case add_u1v:
case sub_u1v:
case functor_arity:
case throw_ball:
case domain_set_false_yy:
case activate_first_agent:
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case fork_unify_constant_return_nondet:
case fork_unify_constant:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    break;
case fork_unify_nil_unify_value_return_nondet:
case fork_unify_cons_uu:
case fork_unify_cons_uv:
case fork_unify_cons_vu:
case fork_unify_cons_vv:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case bp_fork:
case jmp:
case hash_jmpn_nil:
case hash_jmpn_list0:
case memb_le:
case memb_el:
case table_check_completion:
case table_cut:
case endfile:
    LoadAddrFromCArray;
    break;
case getbreg_y:
case putbreg_y:
case getpbreg_y:
case para_uw:
case para_u:
case para_v:
case fetch_v:
case fetch_vw:
case fetch_wv:
case unify_nil:
case unify_arg_u:
case unify_arg_read_u:
case unify_arg_v:
case unify_arg_v0:
case unify_arg_v0w:
case build_arg_v:
case build_arg_v0w:
case build_arg_v0:
case build_arg_u:
case cut_unify_cons_w:
case unify_cons_w:
case unify_cons_ww:
case unify_cons0_v910:
case move_var:
case add1:
case sub1:
case garbage_collect:
case trigger_var_ins:
case trigger_var_minmax:
case trigger_var_dom:
case trigger_var_any_dom:
case fetch_event_object:
case u_eq_cu_dom:
case u_eq_uc_dom:
    LoadYFromCArray;
    break;
case call0:
case last_call0:
    LoadStructFromCArray;
    LoadLiteralFromCArray;
    break;
case call_uv_d:
case call_uv_det:
case call_uv_nondet:
case call_uv_ot:
case call_2u_d:
case call_2u_det:
case call_2u_nondet:
case call_2u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_uuv_d:
case call_uuv_det:
case call_uuv_nondet:
case call_uuv_ot:
case call_uvv_d:
case call_uvv_det:
case call_uvv_nondet:
case call_uvv_ot:
case call_uvu_d:
case call_uvu_det:
case call_uvu_nondet:
case call_uvu_ot:
case call_3u_d:
case call_3u_det:
case call_3u_nondet:
case call_3u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_uvc_d:
case call_uvc_det:
case call_uvc_nondet:
case call_uvc_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadLiteralFromCArray;
    break;
case call_uvuv_d:
case call_uvuv_det:
case call_uvuv_nondet:
case call_uvuv_ot:
case call_uuuv_d:
case call_uuuv_det:
case call_uuuv_nondet:
case call_uuuv_ot:
case call_4u_d:
case call_4u_det:
case call_4u_nondet:
case call_4u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_uuuuv_d:
case call_uuuuv_det:
case call_uuuuv_nondet:
case call_uuuuv_ot:
case call_5u_d:
case call_5u_det:
case call_5u_nondet:
case call_5u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call0_d:
case last_call0_sa_d:
case last_call0_sa_det:
case last_call0_sa_nondet:
case last_call0_sa_ot:
case tr_det_call0:
case tr_nondet_call0:
    LoadAddrFromCArray;
    LoadLiteralFromCArray;
    break;
case call_v_d:
case call_v_det:
case call_v_nondet:
case call_v_ot:
case call_u_d:
case call_u_det:
case call_u_nondet:
case call_u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_6u_d:
case call_6u_det:
case call_6u_nondet:
case call_6u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_7u_d:
case call_7u_det:
case call_7u_nondet:
case call_7u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_8u_d:
case call_8u_det:
case call_8u_nondet:
case call_8u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_9u_d:
case call_9u_det:
case call_9u_nondet:
case call_9u_ot:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_cu_d:
case call_cu_det:
case call_cu_nondet:
case call_cu_ot:
    LoadAddrFromCArray;
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_cuu_d:
case call_cuu_det:
case call_cuu_nondet:
case call_cuu_ot:
    LoadAddrFromCArray;
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_cuuu_d:
case call_cuuu_det:
case call_cuuu_nondet:
case call_cuuu_ot:
    LoadAddrFromCArray;
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_cuuuu_d:
case call_cuuuu_det:
case call_cuuuu_nondet:
case call_cuuuu_ot:
    LoadAddrFromCArray;
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case call_var:
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadZsFromCArray(n);
    LoadLiteralFromCArray;
    break;
case call_var0:
case last_call_var0:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    break;
case jmpn_eq_constant:
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_nil:
    LoadYFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_eq_struct:
    LoadYFromCArray;
    LoadStructFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_eq_struct_fetch_v:
    LoadYFromCArray;
    LoadStructFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadYFromCArray;
    break;
case switch_cons:
case switch_cons_vv:
    LoadYFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case switch_cons_car:
    LoadYFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadYFromCArray;
    break;
case switch_cons_910:
    LoadYFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_eql_uu:
case jmp_eql_uu:
case jmp_eql_uu_fail:
case jmp_eql_uu_ot:
case jmpn_gt_uu:
case jmpn_ge_uu:
case jmpn_id_uu:
case jmp_id_uu:
case conc:
case leng:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_eql_uc:
case jmp_eql_uc:
case jmpn_id_uc:
case jmp_id_uc:
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_gt_ui:
case jmpn_ge_ui:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_gt_iu:
case jmpn_ge_iu:
    LoadLiteralFromCArray;
    LoadYFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_var_y:
case jmpn_var_y_fail0:
case jmpn_var_y_ot:
case jmp_var_y:
case jmpn_atom_y:
case jmpn_atomic_y:
case jmpn_atomic_y_fail0:
case jmpn_atomic_y_ot:
case jmpn_num_y:
case jmpn_float_y:
case jmpn_int_y:
case jmpn_dvar_y:
case jmpn_susp_var_y:
    LoadYFromCArray;
    LoadAddrFromCArray;
    break;
case hash:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    break;
case hash_jmpn_constant:
    LoadConstantFromCArray;
    LoadAddrFromCArray;
    break;
case hash_branch_constant:
    LoadConstantFromCArray;
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    break;
case hash_jmpn_struct:
case hash_branch_struct:
    LoadStructFromCArray;
    LoadAddrFromCArray;
    break;
case hash_jmpn_list:
case fork_unify_cons_uw:
case fork_unify_cons_vw:
case fork_unify_value:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case para_uuuv:
case fetch_4v:
case unify_arg_4v0:
case build_arg_4v0:
case build_arg_4u:
case filter_clauses:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case para_uuv:
case para_uuu:
case para_uuuw:
case fetch_3v:
case unify_arg_3v0:
case unify_arg_3u:
case build_arg_3v0:
case build_arg_3u:
case cut_unify_cons_uu:
case unify_cons_uu:
case cut_unify_cons_uv:
case unify_cons_uv0:
case cut_unify_cons_vv:
case unify_cons_v0v0:
case cut_unify_cons_vu:
case unify_cons_v0u:
case move_cons0_uv:
case move_cons0_uu:
case add_uuv:
case sub_uuv:
case mul_uuv:
case idiv_uuv:
case arg:
case functor_uvv:
case functor_vuu:
case get_clause_copy:
case domain_next_inst_yyy:
case domain_min_max_yyy:
case activate_agents_conjunction:
case activate_agents_disjunction:
case arg_no_chk:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case para_cuv:
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case para_cuuv:
    LoadConstantFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case para_uuuc:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    break;
case para_uuc:
case unify_cons_uc:
case cut_unify_cons_vc:
case unify_cons_v0c:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    break;
case para_c:
case unify_arg_c:
case unify_arg_wc:
case build_arg_c:
case build_arg_wc:
    LoadConstantFromCArray;
    break;
case fork_unify_nil:
case fork_unify_cons_v910:
    LoadAddrFromCArray;
    LoadYFromCArray;
    break;
case unify_struct:
case unify_struct_cut:
case move_struct:
case move_struct0:
    LoadYFromCArray;
    LoadStructFromCArray;
    break;
case fork_unify_struct:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadStructFromCArray;
    break;
case unify_struct_arg_uv0:
case unify_struct_arg_2v0:
case unify_struct_arg_2u:
    LoadYFromCArray;
    LoadStructFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case unify_struct_arg_v0:
case unify_struct_arg_u:
    LoadYFromCArray;
    LoadStructFromCArray;
    LoadYFromCArray;
    break;
case unify_struct_arg_c:
case unify_struct_arg_wc:
    LoadYFromCArray;
    LoadStructFromCArray;
    LoadConstantFromCArray;
    break;
case unify_arg_struct:
case build_arg_struct:
    LoadStructFromCArray;
    break;
case unify_arg_list:
case build_arg_list:
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1)+1;
    LoadZsFromCArray(n);
    break;
case unify_arg_2c:
case build_arg_2c:
    LoadConstantFromCArray;
    LoadConstantFromCArray;
    break;
case unify_arg_3c:
case build_arg_3c:
    LoadConstantFromCArray;
    LoadConstantFromCArray;
    LoadConstantFromCArray;
    break;
case unify_list:
case move_list:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1)+1;
    LoadZsFromCArray(n);
    break;
case unify_comp_list:
case move_comp_list:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadZsFromCArray(n);
    break;
case unify_cons:
case move_cons:
case domain_region:
case domain_nogood_region:
    LoadYFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case unify_cons_cu:
case cut_unify_cons_cv:
case unify_cons_cv0:
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadYFromCArray;
    break;
case unify_cons_cc:
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadConstantFromCArray;
    break;
case fork_unify_cons:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case fork_unify_cons_uc:
case fork_unify_cons_vc:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    break;
case fork_unify_cons_cc:
    LoadAddrFromCArray;
    LoadYFromCArray;
    LoadConstantFromCArray;
    LoadConstantFromCArray;
    break;
case move_comp_list1:
case domain_region_min:
case domain_region_max:
    LoadYFromCArray;
    LoadZFromCArray;
    break;
case and:
case or:
case lshiftl:
case lshiftr:
case mul:
case div:
case idiv:
case divge:
case divle:
case mod:
    LoadZFromCArray;
    LoadZFromCArray;
    LoadYFromCArray;
    break;
case complement:
    LoadZFromCArray;
    LoadYFromCArray;
    break;
case add_uiv:
case sub_uiv:
case idiv_uiv:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    LoadYFromCArray;
    break;
case sub_iuv:
case mul_iuv:
case arg0:
case setarg0_no_chk:
    LoadLiteralFromCArray;
    LoadYFromCArray;
    LoadYFromCArray;
    break;
case setarg:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadZFromCArray;
    break;
case setarg0:
    LoadLiteralFromCArray;
    LoadYFromCArray;
    LoadZFromCArray;
    break;
case functor:
    LoadZFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case builtin0:
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    break;
case builtin1:
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    LoadZFromCArray;
    break;
case builtin2:
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case builtin3:
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case builtin4:
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    LoadZFromCArray;
    break;
case move_vars:
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    break;
case last_call:
    LoadLiteralFromCArray;
    LoadStructFromCArray;
    sym_ptr = (SYM_REC_PTR)FOLLOW(inst_addr-1);
    n = GET_ARITY(sym_ptr);
    LoadZsFromCArray(n);
    LoadLiteralFromCArray;
    break;
case last_call_d:
case last_call1_d:
    LoadAddrFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadZsFromCArray(n);
    LoadLiteralFromCArray;
    break;
case last_call_au_d:
case last_call_au_det:
case last_call_au_nondet:
case last_call_au_ot:
case last_call1_au_d:
case tr_det_call_au:
case tr_det_call1_au:
case tr_nondet_call_au:
case tr_nondet_call1_au:
    LoadAddrFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadYsFromCArray(n);
    LoadLiteralFromCArray;
    break;
case last_call0_d:
    LoadAddrFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    break;
case tr_det_call2_au:
case tr_nondet_call2_au:
    LoadAddrFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadYsFromCArray(n);
    LoadLiteralFromCArray;
    break;
case no_vars_gt:
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    LoadAddrFromCArray;
    break;
case trigger_cg_event_handler:
    LoadLiteralFromCArray;
    LoadYFromCArray;
    break;
case delay:
    LoadAddrFromCArray;
    LoadAddrFromCArray;
    LoadStructFromCArray;
    LoadAddrFromCArray;
    break;
case jmpn_unif:
case jmp_unif:
    LoadZFromCArray;
    LoadZFromCArray;
    LoadAddrFromCArray;
    break;
case table_mode:
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    LoadLiteralFromCArray;
    break;
case asp_decode:
    LoadYFromCArray;
    LoadYFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadYsFromCArray(n);
    break;
case asp_add_tuple:
    LoadYFromCArray;
    LoadLiteralFromCArray;
    n = FOLLOW(inst_addr-1);
    LoadYsFromCArray(n);
    break;
}
