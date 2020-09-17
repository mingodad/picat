/********************************************************************
 *   File   : init.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#ifdef unix
#include <unistd.h>
#endif
#include "bprolog.h"
#include "frame.h"
#include "inst.h"

#include <string.h>
#include "event.h"

BPLONG stack_size_limit = 1000000000;
#ifdef BPSOLVER
BPLONG stack_size = 250000000;
BPLONG trail_size = 4000000;
#else
BPLONG stack_size = 8000000;
BPLONG trail_size = 2000000;
#endif
BPLONG parea_size = 2000000;
BPLONG table_size = 1000000;

void print_picat_usage() {
    printf("Usage: picat [[-path Path]|[-log]|[-g InitGoal]|...] PicatMainFileName A1 A2 ...\n");
}

#ifdef WIN32
int setenv(const char *name, const char *value, int flag) {
    char *buff;
    int res;

    buff = malloc(strlen(name)+strlen(value)+2);
    strcpy(buff, name);
    strcat(buff, "=");
    strcat(buff, value);
    res = _putenv(buff);
    free(buff);
    return res;
    //  return SetEnvironmentVariable(name,value) ? 0 : -1;
}
#endif

/****************************************************************************/
void init_toam(argc, argv)
    int argc;
    char *argv[];
{
    BPLONG i;
    CHAR_PTR str;
    int success = 0;

    for (i = 0; i < BUCKET_CHAIN; i++) {
        hash_table[i] = (SYM_REC_PTR)NULL;
    }
    disassem = num_line = 0;
    i = 1;
    while (i < argc) {
        str = argv[i];
        if (str[0] == '-') {
            switch (str[1]) {
#ifdef PICAT
            case 'p':
                if (strcmp(str+1, "path") == 0) {
                    i++;
                    if (i > argc) {
                        print_picat_usage();
                        exit(0);
                    } else {
                        /*
                          int j;
                          printf("setenv %s\n",argv[i]);
                          for (j=i+1; j<argc; j++){
                          printf("      %s\n",argv[j]);
                          }
                        */
                        setenv("PICATPATH", argv[i], 1);
                    }
                }
                break;

            case '-':
                if (strcmp(str+2, "help") == 0) {
                    print_picat_usage();
                    exit(0);
                } else if (*(str+2) == 'v' || strcmp(str+2, "version") == 0) {
                    printf("Picat version 3.0b2\n");
                    exit(0);
                }

            case 'l':
                if (strcmp(str+1, "log") == 0) {  /* enable log printing */
                    break;
                }
            case 'g':
                use_gl_getline = 0;
                break;

            case 's': i++;
                sscanf(argv[i], "%ld", &stack_size);
                if (stack_size < 1000000) stack_size = 1000000;
                break;

#else
            case 'T':
            case 't':
                i++;
                sscanf(argv[i], "%ld", &table_size);
                if (table_size < 1000000) table_size = 1000000;
                break;

            case 'l': use_gl_getline = 0; break;

            case 'c': confirm_copy_right = 0; break;

            case 'n': num_line = 1; break;

            case 'd': disassem = 1; break;

            case 'S':
            case 's': i++;
                sscanf(argv[i], "%ld", &stack_size);
                if (stack_size < 1000000) stack_size = 1000000;
                break;

            case 'm': i++;
                sscanf(argv[i], "%ld", &stack_size_limit);
                break;

            case 'P':
            case 'p': i++;
                sscanf(argv[i], "%ld", &parea_size);
                if (parea_size < 1000000) parea_size = 1000000;
                break;

            case 'B':
            case 'b': i++;
                sscanf(argv[i], "%ld", &trail_size);
                if (trail_size < 1000000) trail_size = 1000000;
                break;

            case 'g':
                use_gl_getline = 0;
            case 'i':
                i++; break;

            case 'h':
                printf("Usage: bp [-p P] [-s S] [-b B] [-t T] [-i File] [-g Goal] [-h] [-l] bytecode_1 ... \n");
                printf("       P -- size for program area\n");
                printf("       S -- size for global and local stacks\n");
                printf("       B -- size for trail stack\n");
                printf("       T -- size for table area\n");
                printf("       l -- not use the command line editor\n");
                printf("       g -- initial goal\n");
                printf("       h -- help\n");
                exit(0);
#endif
            }
        }
        i++;
    }
    /*
      str = getenv("PICATPATH");
      if (str != NULL){
      printf("PICATPATH= %s\n",str);
      }
    */
    BP_MALLOC(stack_low_addr, stack_size);
    if (stack_low_addr == NULL) myquit(OUT_OF_MEMORY, "in");

    trail_low_addr = (BPLONG_PTR)malloc(trail_size*sizeof(BPLONG));
    if (trail_low_addr == NULL) myquit(OUT_OF_MEMORY, "in");

    trail_water_mark = trail_low_addr+LARGE_MARGIN;
    trail_water_mark0 = trail_low_addr+2;

    parea_low_addr = NULL;
    ALLOCATE_NEW_PAREA_BLOCK(parea_size, success);
    if (success == 0) quit("Not enough memory (init).\n");
    init_findall_area();


#ifdef GC
    gcQueueConstruct();
#endif

    stack_up_addr = stack_low_addr + stack_size -1;
    trail_up_addr = trail_low_addr + trail_size -1;

    dg_flag_word = 0;

    heap_top = stack_low_addr;
    trail_top = trail_up_addr;

    addr_fail = (BPLONG_PTR)curr_fence;
    *(BPLONG_PTR)curr_fence = fail;
    curr_fence += sizeof(BPLONG);

    *(BPLONG_PTR)curr_fence = 9;  /* MaxS for GC */
    curr_fence += sizeof(BPLONG);
    addr_halt = (BPLONG_PTR)curr_fence;
    *(BPLONG_PTR)curr_fence = halt;
    curr_fence += sizeof(BPLONG);

    *(BPLONG_PTR)curr_fence = 9;  /* MaxS for GC */
    curr_fence += sizeof(BPLONG);
    addr_halt0 = (BPLONG_PTR)curr_fence;
    *(BPLONG_PTR)curr_fence = halt0;
    curr_fence += sizeof(BPLONG);

    *(BPLONG_PTR)curr_fence = 13;  /* MaxS for GC */
    curr_fence += sizeof(BPLONG);
    addr_table_consume = (BPLONG_PTR)curr_fence;
    *(BPLONG_PTR)curr_fence = table_consume;
    curr_fence += sizeof(BPLONG);

    local_top = stack_up_addr;

    init_stack(NUM_CG_GLOBALS+2);
    breg0 = breg;  /* the global variables for cglib and global_heap variables are stored in this choice point */

    init_sym();

    cg_initialize();

    curr_toam_status = TOAM_NOTSET;

#ifdef NOTABLE
#else
    init_table_area();
    c_INITIALIZE_TABLE();
#endif
    initialize_free_records();
    init_picat_global_maps();
    inst_begin = 0;
    bp_exception = illegal_arguments;
}  /* end of init_toam */

/*****************************************************************************/
void init_stack(bsize)
    BPLONG bsize;
{
    BPLONG_PTR old_sfreg, old_arreg, old_breg;
    int i;

    old_sfreg = sfreg; old_arreg = arreg; old_breg = breg;

    sfreg = local_top;
    if (old_sfreg == NULL) old_sfreg = sfreg;
    AR_AR(sfreg) = (BPLONG)local_top;
    AR_CPS(sfreg) = (BPLONG)addr_halt;
    AR_TOP(sfreg) = (BPLONG)(local_top - SUSP_FRAME_SIZE);
    AR_BTM(sfreg) = ADDTAG((BPLONG)local_top, SUSP_FRAME_TAG);
    AR_REEP(sfreg) = (BPLONG)NULL;
    AR_PREV(sfreg) = (BPLONG)old_sfreg;
    AR_STATUS(sfreg) = SUSP_EXIT;
    AR_OUT(sfreg) = BP_ZERO;
    local_top -= SUSP_FRAME_SIZE;

    for (i = 0; i < bsize; i++) {
        FOLLOW(local_top-i) = (BPLONG)(local_top-i);
    }
    local_top -= bsize;  /* for holding global data for CGLIB,if not zero */

    breg = local_top;
    if (old_breg == NULL) old_breg = breg;
    if (old_arreg == NULL) old_arreg = breg;

    AR_AR(breg) = (BPLONG)old_arreg;
    AR_CPS(breg) = (BPLONG)addr_halt;  /* CPS : return BP_TRUE on final success */
    AR_TOP(breg) = (BPLONG)(breg - NONDET_FRAME_SIZE);
    AR_BTM(breg) = ADDTAG((BPLONG)(breg+bsize), NONDET_FRAME_TAG);
    AR_B(breg) = (BPLONG)old_breg;
    AR_CPF(breg) = (BPLONG)addr_halt0;  /* CPF : return BP_FALSE on final failure */
    AR_H(breg) = (BPLONG)heap_top;
    AR_T(breg) = (BPLONG)trail_top;
    AR_SF(breg) = (BPLONG)sfreg;
    hbreg = heap_top;
    local_top = breg-NONDET_FRAME_SIZE;

    FOLLOW(local_top--) = BP_ZERO;  /* for holding term in call_bprolog_term(term) */
    arreg = local_top;
    AR_AR(arreg) = (BPLONG)breg;
    AR_CPS(arreg) = (BPLONG)addr_halt;
    AR_BTM(arreg) = ADDTAG((BPLONG)(arreg+1), FLAT_FRAME_TAG);
    AR_TOP(arreg) = (BPLONG)(arreg-FLAT_FRAME_SIZE);
    local_top = arreg-FLAT_FRAME_SIZE;
}

int init_loading(argc, argv)
    int argc;
    char *argv[];
{
    CHAR_PTR str;
    BPLONG i;

#ifdef PICAT
    b_GLOBAL_SET_ccc(ADDTAG(picat_log_psc, ATM), MAKEINT(0), MAKEINT(0));
#endif

    i = 1;
    while (i < argc) {
        str = argv[i];
#ifdef PICAT
        if (str[0] == '-') {
            switch (str[1]) {
            case 'p':
                if (strcmp(str+1, "path") == 0)
                    i++;
                else
                    add_main_arg(str);
                break;
            case 'l':
                if (strcmp(str+1, "log") == 0) {  /* enable log printing */
                    b_GLOBAL_SET_ccc(ADDTAG(picat_log_psc, ATM), MAKEINT(0), MAKEINT(1));
                } else {
                    use_gl_getline = 0;
                }
                break;
            case 'v':
                break;
            case 's':
                i++;
                break;
            default:
                add_main_arg(str);
            }
        } else {
            add_main_arg(str);
        }
#else
        if (str[0] == '-') {
            switch (str[1]) {
            case 'T':
            case 't':
            case 'S':
            case 's':
            case 'm':
            case 'P':
            case 'p':
            case 'B':
            case 'b':
                i++; break;
            default:
                add_main_arg(argv[i]);
            }
        } else {
            if (is_bc_file(str)) {
                if (load_user_bc_file(str) == BP_ERROR) return BP_ERROR;
            } else {
                add_main_arg(str);
            }
        }
#endif
        i++;
    }
    return BP_TRUE;
}

int load_bp_out() {
    char name[256];
    char *s;

    s = getenv("BPDIR");
    if (s == NULL) {
        fprintf(stderr, "the environment variable BPDIR is not set\n");
        return BP_ERROR;
    };
    strcpy(name, s);
    strcat(name, "/bp.out");
    if (is_bc_file(name) && loader(name, 0, 1) != 10);
    else {
        /* char errormsg[128]; */
        fprintf(stderr, "File '%s' does not exist or is not compatible with version 7.5.\n", name);
        return BP_ERROR;
    }
    return BP_TRUE;
}

int is_bc_file(main_arg)
    CHAR_PTR main_arg;
{
    FILE *fp;
    BYTE magic;

    if (access(main_arg, 0) == 0) {
#ifdef MSDOS
        fp = fopen(main_arg, "rb");
#else
        fp = fopen(main_arg, "r");
#endif
        if (fp == NULL)
            return BP_FALSE;

        fread(&magic, 1, 1, fp);
        if (magic != 71) {fclose(fp); return BP_FALSE;}

        fread(&magic, 1, 1, fp);
        if (magic != 21) {fclose(fp); return BP_FALSE;}

        fread(&magic, 1, 1, fp);
        if (magic != 7) {fclose(fp); return BP_FALSE;}

        fread(&magic, 1, 1, fp);
        if (magic != 3) {fclose(fp); return BP_FALSE;}

        fclose(fp);
        return BP_TRUE;
    }
    return BP_FALSE;
}

int load_user_bc_file(name)
    char *name;
{
    BPLONG loaded_ok = loader(name, 1, 1);
    if (loaded_ok == 10) {
        fprintf(stderr, "File '%s' cannot be opened\n", name);
        return BP_ERROR;
    } else if (loaded_ok) {
        fprintf(stderr, "Error in loading initial files\n");
        return BP_ERROR;
    }
    return BP_TRUE;
}

void add_main_arg(main_arg)
    CHAR_PTR main_arg;
{
    BPLONG tmp;
    BPLONG parg;

#ifdef PICAT
    parg = c_str_to_picat_str0(main_arg);
#else
    parg = bp_build_atom(main_arg);
#endif
    tmp = main_args;
    main_args = bp_build_list();
    unify(bp_get_car(main_args), parg);
    unify(bp_get_cdr(main_args), tmp);
}

int c_init_global_each_session() {

    set_global_call_number(1);
    return BP_TRUE;
}






