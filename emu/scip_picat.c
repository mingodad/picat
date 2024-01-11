#include "bprolog.h"
#include <scip/scip.h>
#include <scip/misc.h>
#include <scip/scipdefplugins.h>
SCIP *scip_solver;
SCIP_VAR **scip_vars;
SCIP_CONS **scip_constrs;
int scip_num_of_vars;
int scip_num_of_constrs;

int c_scip_init(){
  BPLONG MaxColNum = ARG(1, 3); DEREF_NONVAR(MaxColNum);
  BPLONG NumOfConstrs = ARG(2, 3); DEREF_NONVAR(NumOfConstrs);
  BPLONG Sense = ARG(3, 3); DEREF_NONVAR(Sense);
  int i;
  
  scip_num_of_vars = INTVAL(MaxColNum);
  scip_num_of_constrs = INTVAL(NumOfConstrs);
  //  printf("scip_init %ld %ld\n", scip_num_of_vars, scip_num_of_constrs);
  SCIPcreate(&scip_solver);
  // load default plugins linke separators, heuristics, etc.
  SCIPincludeDefaultPlugins(scip_solver);
  // disable scip output to stdout
  SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_solver), TRUE);
  // create an empty problem
  SCIPcreateProb(scip_solver, "", NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  if (INTVAL(Sense) == 0)
    SCIPsetObjsense(scip_solver, SCIP_OBJSENSE_MINIMIZE);
  else
    SCIPsetObjsense(scip_solver, SCIP_OBJSENSE_MAXIMIZE);
  scip_vars = (SCIP_VAR *)malloc(sizeof(SCIP_VAR *) * scip_num_of_vars);
  for (i = 0; i < scip_num_of_vars; i++){
    scip_vars[i] = (SCIP_VAR *)NULL;
  }
  scip_constrs = (SCIP_CONS *)malloc(sizeof(SCIP_CONS *) * scip_num_of_constrs);
  for (i = 0; i < scip_num_of_constrs; i++){
    scip_constrs[i] = (SCIP_CONS *)NULL;
  }
				  
  return BP_TRUE;
}

// ColNo,LB,UB,ObjCoe
int c_scip_post_var(){
  BPLONG ColNo = ARG(1,5); DEREF_NONVAR(ColNo); ColNo = INTVAL(ColNo);
  BPLONG VarType = ARG(2,5); DEREF_NONVAR(VarType); VarType = INTVAL(VarType);
  BPLONG LB = ARG(3,5); DEREF_NONVAR(LB); 
  BPLONG UB = ARG(4,5); DEREF_NONVAR(UB); 
  BPLONG ObjCoe = ARG(5,5); DEREF_NONVAR(ObjCoe); ObjCoe = INTVAL(ObjCoe);
  char name[10];
  SCIP_Real scip_lb, scip_ub;

  //  printf("scip_post_var (%ld) %ld %ld\n", ColNo, LB, UB);
  
  name[0] = 'X';
  sprintf(name+1, "%ld", ColNo);
  if (ISINT(LB)){
    LB = INTVAL(LB);
    scip_lb = (LB == -72057594037927935) ? -SCIPinfinity(scip_solver) : LB;
  } else {
    scip_lb = floatval(LB);
  }
  if (ISINT(UB)){
    UB = INTVAL(UB);
    scip_ub = (UB == 72057594037927935) ? SCIPinfinity(scip_solver) : UB;
  }else {
    scip_ub = floatval(UB);
  }
  if (VarType == 0){
    SCIPcreateVar(scip_solver, &scip_vars[ColNo], name, scip_lb, scip_ub, ObjCoe, SCIP_VARTYPE_INTEGER, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
  } else {
    SCIPcreateVar(scip_solver, &scip_vars[ColNo], name, scip_lb, scip_ub, ObjCoe, SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
  }
  SCIPaddVar(scip_solver, scip_vars[ColNo]);
  //  printf("scip_post_var\n");
  return BP_TRUE;
}

// c_scip_create_constr(ConstrNum,LB,UB)
int c_scip_create_constr(){
  BPLONG ConstrNum = ARG(1,3); DEREF_NONVAR(ConstrNum); ConstrNum = INTVAL(ConstrNum);
  BPLONG LB = ARG(2,3); DEREF_NONVAR(LB); 
  BPLONG UB = ARG(3,3); DEREF_NONVAR(UB); 
  SCIP_Real scip_lb, scip_ub;

  //  printf("=> scip_create_constr\n");
  if (ISINT(LB)){
    LB = INTVAL(LB);
    scip_lb = (LB == -72057594037927935) ? -SCIPinfinity(scip_solver) : LB;
  } else {
    scip_lb = floatval(LB);
  }
  if (ISINT(UB)){
    UB = INTVAL(UB);
    scip_ub = (UB == 72057594037927935) ? SCIPinfinity(scip_solver) : UB;
  }else {
    scip_ub = floatval(UB);
  }

  SCIPcreateConsLinear(scip_solver, &scip_constrs[ConstrNum], "", 0, NULL, NULL, scip_lb, scip_ub, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);

  //  printf("<= scip_create_constr\n");
  return BP_TRUE;
}

// c_scip_add_constr_coe(Coe,ColNo,ConstrNum)
int c_scip_add_constr_coe(){
  BPLONG Coe = ARG(1,3); DEREF_NONVAR(Coe); 
  BPLONG ColNo = ARG(2,3); DEREF_NONVAR(ColNo); ColNo = INTVAL(ColNo);
  BPLONG ConstrNum = ARG(3,3); DEREF_NONVAR(ConstrNum); ConstrNum = INTVAL(ConstrNum);
  SCIP_Real scip_coe;

  //  printf("=> scip_add_constr_coe\n");
  scip_coe = NUMVAL(Coe);
  SCIPaddCoefLinear(scip_solver, scip_constrs[ConstrNum], scip_vars[ColNo], scip_coe);  
  //  printf("<= scip_add_constr_coe\n");
  return BP_TRUE;
}

int c_scip_solve(){
  BPLONG Sol = ARG(1,1);   
  int i;
  /*
  printf("scip_solve\n");
  write_term(Sol);
  printf("\n");
  */
  for (i = 0; i < scip_num_of_constrs; i++){
    if (scip_constrs[i] != NULL)
      SCIPaddCons(scip_solver, scip_constrs[i]);
  }
  /*  
  printf("=>print\n");
  SCIPwriteOrigProblem(scip_solver, "__tmp.lp", NULL, FALSE);
  printf("<=print\n");
  */
  SCIPsolve(scip_solver);
  SCIP_SOL * sol = SCIPgetBestSol(scip_solver);
  if (sol == NULL){
    return BP_FALSE;
  } else {
    BPLONG_PTR arr_ptr;
    DEREF_NONVAR(Sol);
    arr_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Sol);
    for (i = 0; i < scip_num_of_vars; i++){
      //      printf("%3f", SCIPgetSolVal(scip_solver, sol, scip_vars[i]));
      unify(arr_ptr[i+1], MAKEINT((BPLONG)SCIPgetSolVal(scip_solver, sol, scip_vars[i])));
    }
  }
  return BP_TRUE;  
}

int c_scip_release(){
  int i;
  //  printf("scip_release\n");
  for (i = 0; i < scip_num_of_vars; i++){
    if (scip_vars[i] != NULL)
      SCIPreleaseVar(scip_solver, &scip_vars[i]);
  }
  for (i = 0; i < scip_num_of_constrs; i++){
    if (scip_constrs[i] != NULL)
      SCIPreleaseCons(scip_solver, &scip_constrs[i]);
  }
  free(scip_vars);
  free(scip_constrs);
  return BP_TRUE;
}

void Cboot_scip(){
  insert_cpred("c_scip_init", 3, c_scip_init);
  insert_cpred("c_scip_post_var", 5, c_scip_post_var);
  insert_cpred("c_scip_create_constr", 3, c_scip_create_constr);
  insert_cpred("c_scip_add_constr_coe", 3, c_scip_add_constr_coe);
  insert_cpred("c_scip_solve", 1, c_scip_solve);
  insert_cpred("c_scip_release", 0, c_scip_release);
}
