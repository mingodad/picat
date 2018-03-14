/********************************************************************
 *   File   : dis.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "basic.h"
#include "inst.h"
#include "term.h"
/*
  #define DEBUG_DIS 
*/
static FILE *filedes;

void dis()
{
#ifdef  DEBUG_DIS
    filedes = stdout;
#else
    filedes = fopen("dump.pil", "w");
#endif
  
    dis_data();
    dis_text();
    fflush(filedes);
    fclose(filedes);
}

void dis_data()
{
    SYM_REC_PTR psc_ptr;
    BPLONG        i;

    fprintf(filedes, "\n/* data below: name, arity, type, axnd entry */\n\n");
    printf("==> dis_data\n");
   
    for (i = 0; i < BUCKET_CHAIN; ++i) {
        psc_ptr = hash_table[i];
        while (psc_ptr!=NULL) {
            fprintf(filedes,BPULONG_FMT_STR  ": ", (BPLONG)psc_ptr);
            curr_out = filedes;
            bp_write_pname(GET_NAME(psc_ptr));
            fprintf(filedes, "/%d,\t", GET_ARITY(psc_ptr));
            switch (GET_ETYPE(psc_ptr)) {
            case T_ORDI: fprintf(filedes, "ORDI");  break;
            case T_DYNA: fprintf(filedes, "DYNA");  break;
            case T_INTP: fprintf(filedes, "INTP");  break;
            case T_PRED: fprintf(filedes, "PRED");  break;
            case C_PRED: fprintf(filedes, "CPRED");  break;
            }
            fprintf(filedes, ",  %lx\n", (unsigned long int)GET_EP(psc_ptr));
            psc_ptr = GET_NEXT(psc_ptr);
        }
    }
    fprintf(filedes, "\n");
}

/*
  void symbol_table_statistics(int *num_of_empty_buckets, int *len_of_longest_chain){
  SYM_REC_PTR psc_ptr;
  int i,count;
  
  *num_of_empty_buckets = 0;
  *len_of_longest_chain = 0;
  
  for (i = 0; i < BUCKET_CHAIN; ++i) {
  psc_ptr = hash_table[i];
  count = 0;
  while (psc_ptr!=NULL) {
  psc_ptr = GET_NEXT(psc_ptr);
  count++;
  }
  if (count==0)
  *num_of_empty_buckets = *num_of_empty_buckets+1;
  if (count > *len_of_longest_chain)    
  *len_of_longest_chain = count;
  }
  }
*/
void dis_text()
{
    printf("==> dis_text\n");
    fprintf(filedes, "\n/*  text below  */\n");
    cpreg = inst_begin;

    printf("inst_begin=%lx\n",(unsigned long int) inst_begin);
    do { 
        fprintf(filedes, "\nNew segment below \n\n");
        while (*cpreg != endfile)
            print_inst(filedes);
        cpreg++;cpreg++;
    } while ((cpreg = (BPLONG_PTR)*cpreg));  
}

void print_inst(filedes)
    FILE *filedes;
{

    BPLONG     opcode;
    BPLONG     i,n;
    SYM_REC_PTR sym_ptr;
    if (num_line)
        fprintf(filedes, "%lx\t", (unsigned long int) cpreg);
    opcode = *cpreg++;

    //  printf("dis %s\n",inst_name[opcode]); 
  
    if (opcode == tabsize) {
        i=*cpreg++;
        fprintf(filedes, "\t " BPULONG_FMT_STR "\n",i);

        while (i>0) {
            if (num_line) fprintf(filedes, "%lx\t",(unsigned long int) cpreg);
            fprintf(filedes, "\t " BPULONG_FMT_STR "\n",*cpreg++);
            i--;
        }
    } else {
#include "dis_inst.h" 
    }
}  /* end of print_inst */

void dis_addr(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    fprintf(filedes, "' " BPULONG_FMT_STR "'",(operand));
}

void dis_y(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    fprintf(filedes, " %d",(int)operand); 
}

void dis_constant(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    SYM_REC_PTR sym_ptr;
    if (ISINT(operand)) fprintf(filedes, "c(%d)",(int)INTVAL(operand)); 
    else {
        sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(operand);
        fprintf(filedes, "c('%s')",GET_NAME(sym_ptr));
    }
}

void dis_literal(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    fprintf(filedes, " %d",(int)operand); 
}

void dis_z(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    if (TAG(operand)==ATM){
        dis_constant(filedes,operand);
    } else if (TAG(operand)==0){ /* vy */
        if (operand==0) 
            fprintf(filedes, " w"); 
        else 
            fprintf(filedes, " v(%d)",(int)(operand>>2)); 
    } else {
        fprintf(filedes, " u(%d)",(int)(operand>>2)); 
    }
}

void dis_struct(filedes,operand)
    FILE *filedes;
    BPLONG operand;
{
    SYM_REC_PTR sym_ptr = (SYM_REC_PTR)operand;
    fprintf(filedes, "'%s'/%d",GET_NAME(sym_ptr),GET_ARITY(sym_ptr));
}

void dis_ys(filedes,n)
    FILE *filedes;
    BPLONG n;
{
    while (n>0){
        dis_y(filedes,*cpreg++);
        n--;
        if (n>=1) fprintf(filedes,",");
    }
}

void dis_zs(filedes,n)
    FILE *filedes;
    BPLONG n;
{
    while (n>0){
        dis_z(filedes,*cpreg++);
        n--;
        if (n>=1) fprintf(filedes,",");
    }
}



  


