/********************************************************************
 *   File   : loader.c
 *   Author : Updated by Neng-Fa ZHOU 1994-2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "basic.h"
#include "inst.h"
#include "term.h"
#include "bapi.h"
#ifdef linux
#include <endian.h>  /* attempt to define endianness */
#endif

#define MAXSYMS BUCKET_CHAIN

#ifdef M64BITS
void inline IGUR(int i) {}  /* Ignore GCC Unused Result */
void IGUR(int i);  /* see https://stackoverflow.com/a/16245669/490291 */
#define READ_DATA_ONLY(x, y) IGUR( fread(x, sizeof(*x), y, fp))
#else
#define READ_DATA_ONLY(x, y) fread(x, sizeof(*x), y, fp)
#endif

#define READ_DATA(x, y) (y - fread(x, sizeof(*x), y, fp))
#define RELOC_ADDR(offset) ((BPLONG_PTR)curr_fence + offset)
#define BUILTIN 1

/* macro for creating hash chains */
#ifdef GCC
#define GEN_COND_JUMP(opcode, arg2, arg3, ep)           \
    *(void **)ep++ = (void **)jmp_table[opcode];        \
    *ep++ = (BPLONG)arg2;                               \
    *ep++ = (BPLONG)arg3;

#define GEN_HASH_BRANCH(opcode, val, lab_neq, lab_eq, ep)       \
    *(void **)ep++ = (void **)jmp_table[opcode];                \
    *ep++ = (BPLONG)val;                                        \
    *ep++ = (BPLONG)lab_neq;                                    \
    *ep++ = (BPLONG)lab_eq;

#define GEN_COND_JUMP2(opcode, arg2, ep)                \
    *(void **)ep++ = (void **)jmp_table[opcode];        \
    *ep++ = (BPLONG)arg2;

#define GEN_JUMP(opcode, arg1, ep)                      \
    *(void **)ep++ = (void **)jmp_table[opcode];        \
    *ep++ = (BPLONG)arg1;
#else
#define GEN_COND_JUMP(opcode, arg2, arg3, ep)   \
    *ep++ = opcode;                             \
    *ep++ = (BPLONG)arg2;                       \
    *ep++ = (BPLONG)arg3;

#define GEN_HASH_BRANCH(opcode, val, lab_neq, lab_eq, ep)       \
    *(void **)ep++ = (void **)opcode;                           \
    *ep++ = (BPLONG)val;                                        \
    *ep++ = (BPLONG)lab_neq;                                    \
    *ep++ = (BPLONG)lab_eq;

#define GEN_COND_JUMP2(opcode, arg2, ep)        \
    *ep++ = opcode;                             \
    *ep++ = (BPLONG)arg2;

#define GEN_JUMP(opcode, arg1, ep)              \
    *ep++ = opcode;                             \
    *ep++ = (BPLONG)arg1;
#endif

#define INVALID_BYTE_CODE {                     \
        bp_exception = invalid_byte_file;       \
        fclose(fp);                             \
        return BP_ERROR;                        \
    }

#define CHECK_PCODE(ptr, size)                                          \
    if ((CHAR_PTR)ptr + 1000 + size >= (CHAR_PTR)parea_up_addr) {       \
        myquit(PAREA_OVERFLOW, "ld");                                   \
    }

/****************************************************************************/
/* fixes the byte-backwards problem.  It is passed a pointer to a           */
/* sequence of 4 bytes read in from a file as bytes.  It then converts      */
/* those bytes to represent a number.  This code works for any machine, and */
/* makes the byte-code machine independent.                                 */
/****************************************************************************/
#define BB4(bptr) (BPLONG)((((((*(BYTE_PTR)bptr << 8) | *((BYTE_PTR)bptr+1)) << 8) | *((BYTE_PTR)bptr+2)) << 8) | *((BYTE_PTR)bptr+3))

#define LoadLiteral {                           \
        READ_DATA_ONLY(buf_for_read, 1);        \
        *inst_addr++ = BB4(buf_for_read);       \
        count++;                                \
    }

#define LoadY LoadLiteral

#ifdef M64BITS
#define LoadConstant {                                          \
        READ_DATA_ONLY(buf_for_read, 1);                        \
        temp32 = (int)BB4(buf_for_read);                        \
        if (ISATOM(temp32)) {                                   \
            *inst_addr = ADDTAG(reloc_table[temp32 >> 2], ATM); \
        } else {                                                \
            *inst_addr = MAKEINT(((temp32 << 1) >> 3));         \
        }                                                       \
        inst_addr++;                                            \
        count++;                                                \
    }
#else
#define LoadConstant {                                          \
        READ_DATA_ONLY(buf_for_read, 1);                        \
        temp = BB4(buf_for_read);                               \
        if (ISINT(temp)) {                                      \
            *inst_addr = temp;                                  \
        } else {                                                \
            *inst_addr = ADDTAG(reloc_table[temp >> 2], ATM);   \
        }                                                       \
        inst_addr++;                                            \
        count++;                                                \
    }
#endif

#define LoadAddr {                                      \
        READ_DATA_ONLY(buf_for_read, 1);                \
        temp = BB4(buf_for_read);                       \
        *inst_addr++ = (BPLONG)RELOC_ADDR(temp);        \
        count++;                                        \
    }

#define LoadStruct {                            \
        READ_DATA_ONLY(buf_for_read, 1);        \
        temp = BB4(buf_for_read);               \
        *inst_addr = (BPLONG)reloc_table[temp]; \
        inst_addr++;                            \
        count++;                                \
    }

#ifdef M64BITS
#define LoadZ {                                                 \
        READ_DATA_ONLY(buf_for_read, 1);                        \
        temp32 = BB4(buf_for_read);                             \
        if (ISATOM(temp32)) {                                   \
            *inst_addr = ADDTAG(reloc_table[temp32 >> 2], ATM); \
        } else if (TAG(temp32) == ATM) {                        \
            *inst_addr = MAKEINT((temp32 << 1) >> 3);           \
        } else {                                                \
            *inst_addr = temp32;                                \
        }                                                       \
        inst_addr++; count++;                                   \
    }
#else
#define LoadZ {                                                 \
        READ_DATA_ONLY(buf_for_read, 1);                        \
        temp = BB4(buf_for_read);                               \
        if (ISATOM(temp)) {                                     \
            *inst_addr = ADDTAG(reloc_table[temp >> 2], ATM);   \
        } else {                                                \
            *inst_addr = temp;                                  \
        }                                                       \
        inst_addr++; count++;                                   \
    }
#endif

#define LoadZs(n) {                             \
        while (n > 0) {                         \
            LoadZ;                              \
            n--;                                \
        }                                       \
    }

#define LoadYs(n) {                             \
        while (n > 0) {                         \
            LoadLiteral;                        \
            n--;                                \
        }                                       \
    }

#define LoadConstants(n) {                      \
        while (n > 0) {                         \
            LoadZ;                              \
            n--;                                \
        }                                       \
    }

extern void **jmp_table;

static BPLONG_PTR inst_addr;
static BPLONG_PTR hptr;
static SYM_REC_PTR reloc_table[MAXSYMS];
static BPLONG_PTR last_text;

BPLONG eof_flag;
BPLONG psc_bytes, text_bytes, index_bytes;
BYTE magic;
BYTE op_mode;
#define OP_MODE_BUILD 0
#define OP_MODE_FETCH 1
#define OP_MODE_UNKNOWN 2

struct hrec {
    BPLONG l;
    BPLONG_PTR link;
};

struct hrec *indextab = NULL;
BPLONG index_table_size = 0;

BYTE dynload = 0;
BPLONG temp;
int temp32;

static FILE *fp;

UW32 buf_for_read[7];

static void inserth(BPLONG ttype, BPLONG val, BPLONG_PTR label, struct hrec *bucket);
static UW32 bp_str_hash(const char *key, int length, UW32 initval);
static INLINE SYM_REC_PTR search(CHAR_PTR name, BPLONG length, BPLONG arity, SYM_REC_PTR sym_ptr);

/****************************************************************************/
int load_bytecode_header() {
    if (magic != 71) {
        printf("Incompatible bytecode file\n");
        INVALID_BYTE_CODE;
    }

    eof_flag = READ_DATA(&magic, 1);
    if (magic != 21) {
        printf("Incompatible bytecode file\n");
        INVALID_BYTE_CODE;
    }

    eof_flag = READ_DATA(&magic, 1);
    if (magic != 7) {
        printf("Incompatible bytecode file\n");
        INVALID_BYTE_CODE;
    }

    eof_flag = READ_DATA(&magic, 1);
    if (magic != 3) {
        printf("Incompatible bytecode file\n");
        INVALID_BYTE_CODE;
    }

    if ((eof_flag = READ_DATA(buf_for_read, 1))) {
        INVALID_BYTE_CODE;
    }
    psc_bytes = BB4(buf_for_read);
    if ((eof_flag = READ_DATA(buf_for_read, 1))) {
        INVALID_BYTE_CODE;
    }
    text_bytes = BB4(buf_for_read);
    if ((eof_flag = READ_DATA(buf_for_read, 1))) {
        INVALID_BYTE_CODE;
    }
    index_bytes = BB4(buf_for_read);
    return BP_TRUE;
}

int loader(file, file_type, load_damon)
    CHAR_PTR file;
    BPLONG file_type;
    BPLONG load_damon;
{
    BPLONG err_msg;
    BPLONG total_size;
    FILE *old_fp;

#ifdef WIN32
    fp = fopen(file, "rb");
#else
    fp = fopen(file, "r");
#endif
    if (fp == NULL) {
        printf("file %s not exist\n", file);
        return 1;
    }
    /* printf("\n     ...... loading file %s curr_fence=%x\n", file,curr_fence); */

    while ((eof_flag = READ_DATA(&magic, 1)) == 0) {
        if (load_bytecode_header() == BP_ERROR)
            return 1;

        /* printf("text_bytes = %lx, index_bytes = %lx psc_bytes = %lx\n", text_bytes, index_bytes, psc_bytes); */
        total_size = sizeof(BPLONG)*text_bytes + index_bytes + psc_bytes + 10000;
        /* printf("total_size = %lx water_mark = %lx\n", (CHAR_PTR)curr_fence+total_size, parea_water_mark); */
        if ((CHAR_PTR)curr_fence+total_size > (CHAR_PTR)parea_water_mark) {
            int success = 0;
            if (total_size > parea_size) parea_size = total_size;
            ALLOCATE_NEW_PAREA_BLOCK(parea_size, success);
            if (success == 0) return -1;
        }

        err_msg = load_syms(file_type);
        if (err_msg != 0) {
            fclose(fp);
            printf("failed loading symbols\n");
            return 1;  /* eventually upper level routines will determine */
        }

        err_msg = load_text();
        if (err_msg != 0) {
            printf("error " BPLONG_FMT_STR " loading file %s: bad text segment\n", err_msg, file);
            return 1;  /* eventually upper level routines will determine */
        }

        err_msg = load_hashtab();
        if (err_msg != 0) {
            printf("error " BPLONG_FMT_STR " in (index) loading file %s: bad index segment\n", err_msg, file);
            return 1;  /*eventually upper level routines will determine */
        }

        if ((eof_flag = READ_DATA(buf_for_read, 2)))
            return 1;

        *inst_addr = BB4(buf_for_read);
        if (*inst_addr != endfile)
            *inst_addr = endfile;
        inst_addr++;  /* skip opcode */
        *inst_addr = 0;  /* force 0 address (BPLONG) */
        last_text = (BPLONG_PTR)inst_addr;
        inst_addr++;
        curr_fence = (CHAR_PTR)inst_addr;
        old_fp = fp;
        if (disassem != 1 && load_damon != 0) {
            bp_call_term_catch(call_damon_load_atom);
        }
        fp = old_fp;
    }
    /* printf("<=loader curr_fence=%x\n",curr_fence); */
    fclose(fp);
    return 0;
}  /* end of loader */

/*************************************************************************/
int load_syms(file_type)
    BPLONG file_type;
{
    CHAR name[256];
    BPLONG ep_offset;
    BPLONG i = 0, j, count = 0;
    //  CHAR     temp_name[MAX_STR_LEN];
    BYTE temp_len;
    BYTE temp_arity;

    /*   printf("=> load_syms \n"); */
    while (count < psc_bytes && eof_flag == 0) {
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 2;
        ep_offset = BB4(buf_for_read);
        if ((eof_flag = READ_DATA(&temp_arity, 1)))
            return 3;
        if ((eof_flag = READ_DATA(&temp_len, 1)))
            return 4;
        if ((eof_flag = READ_DATA(name, temp_len)))
            return 5;
        /*
          if (dynload == 1 ) {
          strncpy(temp_name,name,temp_len);
          printf("load (%s,%i)\n",temp_name,temp_len); 
          }
        */
        if (i >= MAXSYMS) {
            printf("Symbol table overflow:only %i symbols are allowed in one file\n", MAXSYMS);
            return 1;
        }
        reloc_table[i] = insert_sym(name, temp_len, temp_arity);
        set_temp_ep(reloc_table[i], ep_offset*sizeof(BPLONG));
        count += temp_len + 6;
        i++;
    }

    if (count != psc_bytes) {
        printf("error in load_syms count=%i,psc_bytes=%i\n", (int)count, (int)psc_bytes);
        return 1;
    }

    ALIGN(CHAR_PTR, curr_fence);

    for (j = 0; j < i; j++)
        set_real_ep(reloc_table[j], curr_fence);
    return 0;
}  /* end of load_syms */

/************************************************************************
   Given the name of a bytecode module, this function binds Lst to a list of 
   public predicate and function symbols defined in the module 
*************************************************************************/
int c_GET_MODULE_SIGNATURE_cf() {
    BPLONG File;
    CHAR_PTR file_name;
    SYM_REC_PTR sym_ptr;
    CHAR name[256];
    BPLONG ep_offset;
    BPLONG count = 0;
    BYTE temp_len;
    BYTE temp_arity;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    File = ARG(1, 2); DEREF(File);
    if (!ISATOM(File)) {
        bp_exception = atom_expected;
        return BP_FALSE;
    }
    sym_ptr = GET_ATM_SYM_REC(File);
    file_name = GET_NAME(sym_ptr);

#ifdef WIN32
    fp = fopen(file_name, "rb");
#else
    fp = fopen(file_name, "r");
#endif
    if (fp == NULL) {
        printf("file %s not exist\n", file_name);
        bp_exception = file_does_not_exist;
        return BP_ERROR;
    }
    /* printf("\n     ...... loading file %s curr_fence=%x\n", file,curr_fence); */

    READ_DATA(&magic, 1);
    if (load_bytecode_header() == BP_ERROR)
        return BP_ERROR;

    ret_lst_ptr = &ret_lst;
    while (count < psc_bytes && eof_flag == 0) {
        if ((eof_flag = READ_DATA(buf_for_read, 1))) {
            INVALID_BYTE_CODE;
        }
        ep_offset = BB4(buf_for_read);
        if ((eof_flag = READ_DATA(&temp_arity, 1))) {
            INVALID_BYTE_CODE;
        }
        if ((eof_flag = READ_DATA(&temp_len, 1))) {
            INVALID_BYTE_CODE;
        }
        if ((eof_flag = READ_DATA(name, temp_len))) {
            INVALID_BYTE_CODE;
        }

        //      printf("handling %s offset=%d\n",name,ep_offset);

        if (ep_offset >= 0 && temp_len > 3 && *name == 'e' && *(name+1) == '$' && *(name+2) == '$') {  /* public symbols take the form "e$$modulename$$symname" */
            int i = 4;
            while (i < temp_len && (*(name+i-1) != '$' || *(name+i) != '$')) i++;
            // printf("end with  %s\n",name+i+1);
            if (i != temp_len && (i != 7 || (i == 7 && (*(name+3) != 'g' || *(name+4) != 'l' || *(name+5) != 'b')))) {  /* the module glb is the name of the global module */
                int is_pred = 1;
                if (*(name+i+1) == 'f' && *(name+i+2) == '$' && *(name+i+3) == '$') {
                    i += 3;
                    is_pred = 0;
                }
                sym_ptr = insert_sym(name+i+1, temp_len-i-1, 0);  /* list takes the form [name,arity,is_pred,qualified_name ...]  */
                FOLLOW(heap_top) = ADDTAG(sym_ptr, ATM);
                FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
                heap_top++;
                ret_lst_ptr = heap_top;
                heap_top++;
                //[]
                FOLLOW(heap_top) = MAKEINT((is_pred == 1) ? temp_arity : temp_arity-1);
                FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
                heap_top++;
                ret_lst_ptr = heap_top;
                heap_top++;
                //[]
                FOLLOW(heap_top) = MAKEINT(is_pred);
                FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
                heap_top++;
                ret_lst_ptr = heap_top;
                heap_top++;
                //[]
                sym_ptr = insert_sym(name, temp_len, 0);
                FOLLOW(heap_top) = ADDTAG(sym_ptr, ATM);
                FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
                heap_top++;
                ret_lst_ptr = heap_top;
                heap_top++;

                LOCAL_OVERFLOW_CHECK("get_syms");
            }
        }
        count += temp_len + 6;
    }
    FOLLOW(ret_lst_ptr) = nil_sym;
    fclose(fp);
    unify(ret_lst, ARG(2, 2));
    return BP_TRUE;
}

/************************************************************************/
int load_text()
{
    BPLONG current_opcode = 0;
    BPLONG count = 0;
    BPLONG n;
    SYM_REC_PTR sym_ptr;

    /*   printf("==> load_text \n");*/

    /* set text segments chain */
    inst_addr = (BPLONG_PTR)curr_fence;

    if (inst_begin == 0)
        inst_begin = (BPLONG_PTR)inst_addr;
    else
        *last_text = (BPLONG)inst_addr;

    CHECK_PCODE(curr_fence, sizeof(BPLONG)*text_bytes);

    while (count < text_bytes && (eof_flag = READ_DATA(buf_for_read, 1)) == 0) {
        current_opcode = BB4(buf_for_read);

#ifdef GCC
        *(void **)inst_addr++ = jmp_table[current_opcode];
#else
        *inst_addr++ = current_opcode;
#endif

#include "load_inst.h"
        count++;

    }
    if (count != text_bytes) {
        return 9;  /* missing instructions */
    }
    return 0;
}  /* end of load_text */


int load_hashtab()
{
    BPLONG hash_inst_addr, alt, clause_no, temp_len;
    BPLONG count = 0;

    CHECK_PCODE((CHAR_PTR)inst_addr, index_bytes);
    /*   printf("==> load_hashtab \n"); */
    while (count < index_bytes && eof_flag == 0) {
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 10;
        hash_inst_addr = BB4(buf_for_read);
        hash_inst_addr = (BPLONG)RELOC_ADDR(hash_inst_addr);
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 10;
        //        hash_reg = BB4(buf_for_read);
        BB4(buf_for_read);
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 1;
        clause_no = BB4(buf_for_read);
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 1;
        alt = BB4(buf_for_read);
        alt = (BPLONG)RELOC_ADDR(alt);
        if (eof_flag = get_index_tab(clause_no, &temp_len))
            return eof_flag;
        inst_addr = gen_index(hash_inst_addr, clause_no, alt);
        count += (16 + temp_len);
    }
    return 0;
}


int get_index_tab(clause_no, lenptr)
    BPLONG clause_no;
    BPLONG_PTR lenptr;
{
    BPLONG hashval, size, j;
    BPLONG count = 0;
    BYTE type;
    BPLONG val, ttype;
    BPLONG_PTR label;

    hptr = heap_top;
    size = bp_hsize(clause_no);
    if (size > index_table_size) {
        if (indextab != NULL) free(indextab);
        indextab = (struct hrec *)malloc(sizeof(struct hrec)*size);
        index_table_size = size;
    }

    for (j = 0; j < size; j++) {
        indextab[j].l = 0;
        indextab[j].link = (BPLONG_PTR)(&(indextab[j].link));
    }
    for (j = 0; j < clause_no; j++) {
        if ((eof_flag = READ_DATA(&type, 1)))
            return 11;
        switch (type) {
        case 'i': if ((eof_flag = READ_DATA(buf_for_read, 1)))
                return 12;
            val = BB4(buf_for_read);
            val = MAKEINT(val);
            count += 9;
            ttype = 0;
            break;
        case 's': if ((eof_flag = READ_DATA(buf_for_read, 1)))
                return 12;
            val = BB4(buf_for_read);
            count += 9;
            val = (BPLONG)reloc_table[val];
            if (val == (BPLONG)list_psc) {
                ttype = 1;
            }
            else {
                ttype = 2;
            }
            break;
        case 'c': if ((eof_flag = READ_DATA(buf_for_read, 1)))
                return 12;
            val = BB4(buf_for_read);
            count += 9;
            val = (BPLONG)reloc_table[val];
            if (val == UNTAGGED_ADDR(nil_sym))
                ttype = 3;
            else
                ttype = 4;
            val = ADDTAG(val, ATM);
            break;
        default:
            printf("WARNING: unknown type %c in get_index_tab\n", type);
            ttype = 0;
            val = 0;
        }
        if ((eof_flag = READ_DATA(buf_for_read, 1)))
            return 13;
        label = (BPLONG_PTR)BB4(buf_for_read);
        label = RELOC_ADDR((BPLONG)label);
        hashval = IHASH(val, size);
        inserth(ttype, val, label, &indextab[hashval]);
    }
    *lenptr = count;
    return 0;
}  /* end of get_index_tab */


#define IS_LAST_HASH_ALT(temp) FOLLOW(FOLLOW(temp)) == FOLLOW(temp)

BPLONG_PTR gen_index(BPLONG hash_inst_addr, BPLONG clause_no, BPLONG alt)
{
    BPLONG_PTR ep1, ep2;
    BPLONG j, size;
    BPLONG ttype, val;
    BPLONG_PTR label, jumpaddr, temp;

    size = bp_hsize(clause_no);
    ep1 = inst_addr;
    *ep1++ = tabsize;
    *ep1++ = size;
    ep2 = inst_addr + 2 + size;
    temp = (BPLONG_PTR)(hash_inst_addr) + 2;  /* fill the slot in the hash inst*/
    *temp++ = size;
    *temp = (BPLONG)ep1;

    for (j = 0; j < size; j++) {
        if (indextab[j].l == 0) {
            *ep1++ = alt;
        } else if (indextab[j].l == 1) {
            *ep1++ = (BPLONG)*((BPLONG_PTR)(indextab[j].link) + 2);
        } else {  /* create conditional jump insts */
            *ep1++ = (BPLONG)ep2;
            temp = (BPLONG_PTR)(indextab[j].link);
            while (*temp != (BPLONG)temp) {
                ttype = *temp++;
                val = *temp++;
                label = (BPLONG_PTR)*temp++;
                switch (ttype) {
                case 0 :
                    if (IS_LAST_HASH_ALT(temp)) {
                        GEN_JUMP(jmp, label, ep2);
                    } else {
                        jumpaddr = ep2+4;  /* size(hash_branch_constant)=4, size(hash_jmpn_constant)=3 */
                        GEN_HASH_BRANCH(hash_branch_constant, val, jumpaddr, ((BPLONG_PTR)label+3), ep2);
                    }
                    break;
                case 1 :
                    if (IS_LAST_HASH_ALT(temp)) {
                        GEN_JUMP(jmp, label, ep2);
                    } else {
                        jumpaddr = ep2+4;
                        GEN_COND_JUMP2(hash_jmpn_list0, jumpaddr, ep2);
                        GEN_JUMP(jmp, label, ep2);
                    }
                    break;
                case 2 :
                    if (IS_LAST_HASH_ALT(temp)) {
                        GEN_JUMP(jmp, label, ep2);
                    } else {
                        jumpaddr = ep2+4;  /* size(hash_branch_struct)=4, size(hash_jmpn_struct)=3 */
                        GEN_HASH_BRANCH(hash_branch_struct, val, jumpaddr, ((BPLONG_PTR)label+3), ep2);
                    }
                    break;
                case 3 :
                    if (IS_LAST_HASH_ALT(temp)) {
                        GEN_JUMP(jmp, label, ep2);
                    } else {
                        jumpaddr = ep2+4;
                        GEN_COND_JUMP2(hash_jmpn_nil, jumpaddr, ep2);
                        GEN_JUMP(jmp, label, ep2);
                    }
                    break;
                case 4 :
                    if (IS_LAST_HASH_ALT(temp)) {
                        GEN_JUMP(jmp, label, ep2);
                    } else {
                        jumpaddr = ep2+5;
                        GEN_COND_JUMP(hash_jmpn_constant, val, jumpaddr, ep2);
                        GEN_JUMP(jmp, label, ep2);
                    }
                    break;
                }
                temp = (BPLONG_PTR)*temp;
                CHECK_PCODE((CHAR_PTR)ep2, 0);
            }
        }
    }
    return ep2;
}

static void inserth(ttype, val, label, bucket)
    BPLONG ttype, val;
BPLONG_PTR label;
struct hrec *bucket;
{
    BPLONG_PTR temp;

    bucket->l++;
    temp = (BPLONG_PTR)&(bucket->link);
    if (bucket->l > 1) {
        temp = (BPLONG_PTR)*temp;
        while ((BPLONG_PTR)*temp != temp)
        {

            /*    printf("type = %i \n", (BPLONG)*temp++);
                  printf("val  = %i \n", (BPLONG)*temp++);
                  printf("label= %x \n", (BPLONG)*temp++);*/
            temp = (BPLONG_PTR)*(temp+3);
        }
    }
    *temp = (BPLONG)hptr;
    *hptr++ = ttype;
    *hptr++ = val;
    *hptr++ = (BPLONG)label;
    *hptr++ = *temp + sizeof(BPLONG)*3;  /* *hptr++ = (BPLONG)hptr; */
}

BPLONG bp_prime(numentry)
    BPLONG numentry;
{
    BPLONG i, j, temp;

    temp = numentry + 1;
    if ((temp%2) == 0) temp++;

hashsod:
    j = temp / 2 + 1;
    for (i = 3; i <= j; i += 2) {
        if ((temp % i) == 0) {
            temp += 2;
            goto hashsod;
        }
    }
    return temp;
}

/* return a prime-look number */
BPLONG bp_hsize(numentry)
    BPLONG numentry;
{
    BPLONG i, j, temp;

    temp = numentry + 1;
    if ((temp%2) == 0) temp++;
    j = temp / 2 + 1;
    if (j > 29) j = 29;
hashsod:
    for (i = 3; i <= j; i += 2) {
        if ((temp % i) == 0) {
            temp += 2;
            goto hashsod;
        }
    }
    return temp;
}


int dyn_loader(sym_ptr, file_type, load_damon)
    SYM_REC_PTR sym_ptr;
    BPLONG file_type;
    BPLONG load_damon;
{
    CHAR s[256], s1[256], s3[256];
    CHAR_PTR s2;
    BPLONG i;

    dynload = 1;
    namestring(sym_ptr, s1);
    if (*s1 == '/' || *s1 == '.')
        return loader(s1, file_type, load_damon);
    else if (file_type != BUILTIN) {
        if (*s1 == '~') {
            s2 = getenv("HOME");
            if (s2 == NULL) {
                fputs("the environment variable HOME is not set\n", stderr);
            }
            if (*(s1+1) == '/') {
                scat(s2, &s1[1], s3);
                return loader(s3, file_type, load_damon);
            }
            else {
                i = strlen(s2);
                i--;
                while (*(s2 + i) != '/') {
                    *(s2+i) = '\0';
                    i--;
                }
                scat(s2, &s1[1], s3);
                return loader(s3, file_type, load_damon);
            }
        }
        return loader(s1, file_type, load_damon);
    }
    else {
        s2 = getenv("SIMPATH");
        if (s2 == NULL) {
            fputs("the environment variable SIMPATH is not set\n", stderr);
        }
        while (1) {
            while (*s2 == ':' || *s2 == ' ')
                s2++;
            i = 0;
            if (*s2 == '\0')  /* file not found */
                return 1;
            while (*s2 && *s2 != ' ' && *s2 != ':')
                s[i++] = *s2++;
            s[i++] = '/';
            s[i] = '\0';
            scat(s, s1, s3);
            return loader(s3, file_type, load_damon);
        }
    }
}

#ifdef ORIG_HASH
UW32 bp_str_hash( const char *name, int length, UW32 arity) {
    BPLONG i = 0;

    i = arity + 1;
    if (length > 0) {  /* first */
        i = i + *name;
        if (length > 1) {  /* last */
            i = (i << 2) + *(name + length - 1);
            if (length > 2) {  /* middle */
                i = (i << 2) + *(name + length / 2);
                if (length > 3)
                    i = (i << 2) + *(name+(length / 2) - 1);
            }
        }
    }
    if (i < 0) i = -i;
    return i % BUCKET_CHAIN;
}
#else
/*
 * My best guess at if you are big-endian or little-endian.  This may
 * need adjustment.
 */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) &&               \
     __BYTE_ORDER == __LITTLE_ENDIAN) ||                                \
    (defined(i386) || defined(__i386__) || defined(__i486__) ||         \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) &&                \
       __BYTE_ORDER == __BIG_ENDIAN) ||                                 \
    (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 1
#else
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 0
#endif

#define hashsize(n) ((UW32)1 << (n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x, k) (((x) << (k)) | ((x) >> (32-(k))))

/*
  -------------------------------------------------------------------------------
  mix -- mix 3 32-bit values reversibly.

  This is reversible, so any information in (a,b,c) before mix() is
  still in (a,b,c) after mix().

  If four pairs of (a,b,c) inputs are run through mix(), or through
  mix() in reverse, there are at least 32 bits of the output that
  are sometimes the same for one pair and different for another pair.
  This was tested for:
  * pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
  * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
  * the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

  Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
  satisfy this are
  4  6  8 16 19  4
  9 15  3 18 27 15
  14  9  3  7 17  3
  Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
  for "differ" defined as + with a one-bit base and a two-bit delta.  I
  used http://burtleburtle.net/bob/hash/avalanche.html to choose 
  the operations, constants, and arrangements of the variables.

  This does not achieve avalanche.  There are input bits of (a,b,c)
  that fail to affect some output bits of (a,b,c), especially of a.  The
  most thoroughly mixed value is c, but it doesn't really even achieve
  avalanche in c.

  This allows some parallelism.  Read-after-writes are good at doubling
  the number of bits affected, so the goal of mixing pulls in the opposite
  direction as the goal of parallelism.  I did what I could.  Rotates
  seem to cost as much as shifts on every machine I could lay my hands
  on, and rotates are much kinder to the top and bottom bits, so I used
  rotates.
  -------------------------------------------------------------------------------
*/
#define mix(a, b, c)                            \
    {                                           \
        a -= c; a ^= rot(c, 4); c += b;         \
        b -= a; b ^= rot(a, 6); a += c;         \
        c -= b; c ^= rot(b, 8); b += a;         \
        a -= c; a ^= rot(c, 16); c += b;        \
        b -= a; b ^= rot(a, 19); a += c;        \
        c -= b; c ^= rot(b, 4); b += a;         \
    }

/*
  -------------------------------------------------------------------------------
  final -- final mixing of 3 32-bit values (a,b,c) into c

  Pairs of (a,b,c) values differing in only a few bits will usually
  produce values of c that look totally different.  This was tested for
  * pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
  * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
  * the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

  These constants passed:
  14 11 25 16 4 14 24
  12 14 25 16 4 14 24
  and these came close:
  4  8 15 26 3 22 24
  10  8 15 26 3 22 24
  11  8 15 26 3 22 24
  -------------------------------------------------------------------------------
*/
#define final(a, b, c)                          \
    {                                           \
        c ^= b; c -= rot(b, 14);                \
        a ^= c; a -= rot(c, 11);                \
        b ^= a; b -= rot(a, 25);                \
        c ^= b; c -= rot(b, 16);                \
        a ^= c; a -= rot(c, 4);                 \
        b ^= a; b -= rot(a, 14);                \
        c ^= b; c -= rot(b, 24);                \
    }

/*
  -------------------------------------------------------------------------------
  hashlittle() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  length  : the length of the key, counting by bytes
  initval : can be any 4-byte value
  Returns a 32-bit value.  Every bit of the key affects every bit of
  the return value.  Two keys differing by one or two bits will have
  totally different hash values.

  The best hash table sizes are powers of 2.  There is no need to do
  mod a prime (mod is sooo slow!).  If you need less than 32 bits,
  use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
  In which case, the hash table should have hashsize(10) elements.

  If you are hashing n strings (uint8_t **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);

  By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net.  You may use this
  code any way you wish, private, educational, or commercial.  It's free.

  Use for hash table lookup, or anything where one collision in 2^^32 is
  acceptable.  Do NOT use for cryptographic purposes.
  -------------------------------------------------------------------------------
*/

UW32 bp_str_hash( const char *key, int length, UW32 initval)
{
    UW32 a, b, c;  /* internal state */
    union { const char *ptr; int i; } u;  /* needed for Mac Powerbook G4 */

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + ((UW32)length) + initval;
    u.i = length;

    u.ptr = key;
    if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
        const UW32 *k = (const UW32 *)key;  /* read 32-bit chunks */
        const BYTE *k8;

        /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
        while (length > 12)
        {
            a += k[0];
            b += k[1];
            c += k[2];
            mix(a, b, c);
            length -= 12;
            k += 3;
        }

        /*----------------------------- handle the last (probably partial) block */
        /*
         * "k[2]&0xffffff" actually reads beyond the end of the string, but
         * then masks off the part it's not allowed to read.  Because the
         * string is aligned, the masked-off tail is in the same word as the
         * rest of the string.  Every machine with memory protection I've seen
         * does it on word boundaries, so is OK with this.  But VALGRIND will
         * still catch it and complain.  The masking trick does make the hash
         * noticably faster for short strings (like English words).
         */
#ifndef VALGRIND

        switch(length)
        {
        case 12: c += k[2]; b += k[1]; a += k[0]; break;
        case 11: c += k[2]&0xffffff; b += k[1]; a += k[0]; break;
        case 10: c += k[2]&0xffff; b += k[1]; a += k[0]; break;
        case 9 : c += k[2]&0xff; b += k[1]; a += k[0]; break;
        case 8 : b += k[1]; a += k[0]; break;
        case 7 : b += k[1]&0xffffff; a += k[0]; break;
        case 6 : b += k[1]&0xffff; a += k[0]; break;
        case 5 : b += k[1]&0xff; a += k[0]; break;
        case 4 : a += k[0]; break;
        case 3 : a += k[0]&0xffffff; break;
        case 2 : a += k[0]&0xffff; break;
        case 1 : a += k[0]&0xff; break;
        case 0 : break;  // return c;              /* zero length strings require no mixing */
        }

#else  /* make valgrind happy */

        k8 = (const BYTE *)k;
        switch(length)
        {
        case 12: c += k[2]; b += k[1]; a += k[0]; break;
        case 11: c += ((UW32)k8[10]) << 16;  /* fall through */
        case 10: c += ((UW32)k8[9]) << 8;  /* fall through */
        case 9 : c += k8[8];  /* fall through */
        case 8 : b += k[1]; a += k[0]; break;
        case 7 : b += ((UW32)k8[6]) << 16;  /* fall through */
        case 6 : b += ((UW32)k8[5]) << 8;  /* fall through */
        case 5 : b += k8[4];  /* fall through */
        case 4 : a += k[0]; break;
        case 3 : a += ((UW32)k8[2]) << 16;  /* fall through */
        case 2 : a += ((UW32)k8[1]) << 8;  /* fall through */
        case 1 : a += k8[0]; break;
        case 0 : break;  // return c;
        }

#endif  /* !valgrind */

    } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
        const UW16 *k = (const UW16 *)key;  /* read 16-bit chunks */
        const BYTE *k8;

        /*--------------- all but last block: aligned reads and different mixing */
        while (length > 12)
        {
            a += k[0] + (((UW32)k[1]) << 16);
            b += k[2] + (((UW32)k[3]) << 16);
            c += k[4] + (((UW32)k[5]) << 16);
            mix(a, b, c);
            length -= 12;
            k += 6;
        }

        /*----------------------------- handle the last (probably partial) block */
        k8 = (const BYTE *)k;
        switch(length)
        {
        case 12: c += k[4]+(((UW32)k[5]) << 16);
            b += k[2]+(((UW32)k[3]) << 16);
            a += k[0]+(((UW32)k[1]) << 16);
            break;
        case 11: c += ((UW32)k8[10]) << 16;  /* fall through */
        case 10: c += k[4];
            b += k[2]+(((UW32)k[3]) << 16);
            a += k[0]+(((UW32)k[1]) << 16);
            break;
        case 9 : c += k8[8];  /* fall through */
        case 8 : b += k[2]+(((UW32)k[3]) << 16);
            a += k[0]+(((UW32)k[1]) << 16);
            break;
        case 7 : b += ((UW32)k8[6]) << 16;  /* fall through */
        case 6 : b += k[2];
            a += k[0]+(((UW32)k[1]) << 16);
            break;
        case 5 : b += k8[4];  /* fall through */
        case 4 : a += k[0]+(((UW32)k[1]) << 16);
            break;
        case 3 : a += ((UW32)k8[2]) << 16;  /* fall through */
        case 2 : a += k[0];
            break;
        case 1 : a += k8[0];
            break;
        case 0 : break;  // return c;                     /* zero length requires no mixing */
        }

    } else {  /* need to read the key one byte at a time */
        const BYTE *k = (const BYTE *)key;

        /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
        while (length > 12)
        {
            a += k[0];
            a += ((UW32)k[1]) << 8;
            a += ((UW32)k[2]) << 16;
            a += ((UW32)k[3]) << 24;
            b += k[4];
            b += ((UW32)k[5]) << 8;
            b += ((UW32)k[6]) << 16;
            b += ((UW32)k[7]) << 24;
            c += k[8];
            c += ((UW32)k[9]) << 8;
            c += ((UW32)k[10]) << 16;
            c += ((UW32)k[11]) << 24;
            mix(a, b, c);
            length -= 12;
            k += 12;
        }

        /*-------------------------------- last block: affect all 32 bits of (c) */
        switch(length)  /* all the case statements fall through */
        {
        case 12: c += ((UW32)k[11]) << 24;
        case 11: c += ((UW32)k[10]) << 16;
        case 10: c += ((UW32)k[9]) << 8;
        case 9 : c += k[8];
        case 8 : b += ((UW32)k[7]) << 24;
        case 7 : b += ((UW32)k[6]) << 16;
        case 6 : b += ((UW32)k[5]) << 8;
        case 5 : b += k[4];
        case 4 : a += ((UW32)k[3]) << 24;
        case 3 : a += ((UW32)k[2]) << 16;
        case 2 : a += ((UW32)k[1]) << 8;
        case 1 : a += k[0];
            break;
        case 0 : break;  // return c;
        }
    }

    final(a, b, c);
    return (c & (BUCKET_CHAIN - 1));
}
#endif

/****************************************************************************/
INLINE SYM_REC_PTR search(name, length, arity, sym_ptr)
    CHAR_PTR name;
    BPLONG arity;
    BPLONG length;
    SYM_REC_PTR sym_ptr;
{
    unsigned short i;
    CHAR_PTR nameptr;

    while (sym_ptr != NULL) {
        if (arity == GET_ARITY(sym_ptr) && length == GET_LENGTH(sym_ptr)) {
            nameptr = GET_NAME(sym_ptr);
            for (i = 0; i < length; i++)
                if (*(name + i) != *(nameptr + i)) goto cont;
            return sym_ptr;
        }
    cont:
        sym_ptr = GET_NEXT(sym_ptr);
    }
    return sym_ptr;
}  /* end of search */

/****************************************************************************/
SYM_REC_PTR insert(name, length, arity)
    const char *name;
    BPLONG length;
    BPLONG arity;
{
    return insert_sym(name, length, arity);
}

SYM_REC_PTR insert_sym(name, length, arity)
    const char *name;
    BPLONG length;
    BPLONG arity;
{
    BPLONG bucket_no;
    BPLONG i;
    SYM_REC_PTR sym_ptr;
    CHAR_PTR nameptr;

    bucket_no = bp_str_hash(name, length, arity);
    sym_ptr = (SYM_REC_PTR)hash_table[bucket_no];
    while (sym_ptr) {
        if (arity == GET_ARITY(sym_ptr) && length == GET_LENGTH(sym_ptr)) {
            nameptr = GET_NAME(sym_ptr);
            for (i = 0; i < length; i++)
                if (*(name + i) != *(nameptr + i)) goto cont;
            return sym_ptr;
        }
    cont:
        sym_ptr = GET_NEXT(sym_ptr);
    }
    /* insert */
    number_of_symbols++;
    if (curr_fence >= (CHAR_PTR)parea_water_mark) {
        int success = 0;
        ALLOCATE_NEW_PAREA_BLOCK(parea_size, success);
        if (success == 0) myquit(OUT_OF_MEMORY, "ld");
    }

    ALIGN(CHAR_PTR, curr_fence);  /* insert a sym record */
    sym_ptr = (SYM_REC_PTR)curr_fence;
    curr_fence += sizeof(struct sym_rec);

    GET_SPY(sym_ptr) = 0;
    GET_ETYPE(sym_ptr) = T_ORDI;
    GET_EP(sym_ptr) = (int (*)(void))nil_sym;
    GET_ARITY(sym_ptr) = (UW32)arity;
    GET_LENGTH(sym_ptr) = (UW16)length;
    GET_NEXT(sym_ptr) = hash_table[bucket_no];
    GET_NAME(sym_ptr) = curr_fence;

    hash_table[bucket_no] = sym_ptr;

    for (i = 0; i < length; i++)
        *curr_fence++ = *name++;
    *curr_fence++ = '\0';

    /*  CHECK_PCODE(curr_fence,0); */

    /* ALIGN(CHAR_PTR, curr_fence); */
    return sym_ptr;
}  /* end of insert */

/*********************************************/
SYM_REC_PTR insert_cpred(name, arity, func)
    CHAR_PTR name;
    BPLONG arity;
    int (*func)(void);
{
    BPLONG length;
    SYM_REC_PTR sym_ptr;

    length = strlen(name);
    sym_ptr = insert_sym(name, length, arity);
    GET_ETYPE(sym_ptr) = C_PRED;
    GET_EP(sym_ptr) = (int (*)(void))func;
    return sym_ptr;
}

int set_temp_ep(sym_ptr, ep)
    SYM_REC_PTR sym_ptr;
    BPLONG ep;
{
    if (ep >= 0) {
        GET_ETYPE(sym_ptr) = T_TEMP_PRED;
        GET_EP(sym_ptr) = (int (*)(void))ep;
    }
    return 0;
}

/*********************************************/
void set_real_ep(sym_ptr, base)
    SYM_REC_PTR sym_ptr;
    CHAR_PTR base;
{
    if (GET_ETYPE(sym_ptr) == T_TEMP_PRED) {
        GET_EP(sym_ptr) = (int (*)(void))(base + (BPLONG)GET_EP(sym_ptr));  /*???*/
        GET_ETYPE(sym_ptr) = T_PRED;
    }
}


/************************************?????????????????????*********/
/*
  unset_file_ptr(sym_ptr)
  SYM_REC_PTR sym_ptr;
  {
  GET_ETYPE(sym_ptr) = T_ORDI;
  (BPLONG)GET_EP(sym_ptr) = 0;
  }
*/

/************************************?????????????????????*********/
CHAR_PTR namestring(sym_ptr, s)
    SYM_REC_PTR sym_ptr;
    CHAR_PTR s;
{
    BPLONG i, len;
    CHAR_PTR st;

    len = GET_LENGTH(sym_ptr);
    st = GET_NAME(sym_ptr);

#ifdef DEBUG_NAMESTRING
    printf("namestring: len = %d    string = %s\n", len, st);
#endif
    for (i = 0; i < len; i++)
        *s++ = *st++;
    *s = '\0';
    return s;
}

int c_CURRENT_PREDICATE() {
    BPLONG bucket_no, length;
    SYM_REC_PTR sym_ptr;
    BPLONG f, n;
    char *name;
    BPLONG_PTR top;

    f = ARG(1, 2); DEREF(f); if (!ISATOM(f)) {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    n = ARG(2, 2); DEREF(n); if (!ISINT(n)) {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    n = INTVAL(n);

    sym_ptr = GET_ATM_SYM_REC(f);
    name = GET_NAME(sym_ptr);
    length = strlen(name);
    bucket_no = bp_str_hash(name, length , n);
    sym_ptr = search(name, length, n, hash_table[bucket_no]);
    if (sym_ptr == NULL) return BP_FALSE;

    switch (GET_ETYPE(sym_ptr)) {
    case T_DYNA:
    case T_INTP:
    case T_PRED:
    case C_PRED:
        return BP_TRUE;
    default: return BP_FALSE;
    }
}

int c_CURRENT_PREDICATES() {
    BPLONG list, temp0, temp1, cell;
    BPLONG i;
    SYM_REC_PTR sym_ptr;

    list = ARG(1, 1);

    temp1 = nil_sym;
    for (i = 0; i < BUCKET_CHAIN; ++i) {
        sym_ptr = hash_table[i];
        while (sym_ptr != NULL) {
            switch (GET_ETYPE(sym_ptr)) {
            case T_DYNA:
            case T_INTP:
            case T_PRED:
            case C_PRED:
                cell = ADDTAG(insert_sym(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr), 0), ATM);
                cell = make_struct2("/", cell, MAKEINT(GET_ARITY(sym_ptr)));
                temp0 = ADDTAG((BPLONG)heap_top, LST);
                NEW_HEAP_NODE(cell);
                NEW_HEAP_NODE(temp1);
                temp1 = temp0;
                LOCAL_OVERFLOW_CHECK("loader");
                break;
            default: break;
            }
            sym_ptr = GET_NEXT(sym_ptr);
        }
    }
    return unify(list, temp1);
}

SYM_REC_PTR look_for_sym_with_entrance(p)
    BPLONG_PTR p;
{
    BPLONG i;
    SYM_REC_PTR sym_ptr;
    for (i = 0; i < BUCKET_CHAIN; ++i) {
        sym_ptr = hash_table[i];
        while (sym_ptr != NULL) {
            if ((BPLONG)GET_ETYPE(sym_ptr) == T_PRED && (BPLONG)GET_EP(sym_ptr) == (BPLONG)p) return sym_ptr;
            sym_ptr = GET_NEXT(sym_ptr);
        }
    }
    return NULL;
}

/* load from in-memory lists */
#define READ_FROM_BCINSTS(val) {                        \
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(BCInsts);  \
        val = FOLLOW(list_ptr);                         \
        DEREF(val);                                     \
        BCInsts = FOLLOW(list_ptr+1);                   \
        DEREF(BCInsts);                                 \
    }

#define LoadLiteralFromBPList {                 \
        READ_FROM_BCINSTS(temp);                \
        *inst_addr = INTVAL(temp);              \
        inst_addr++;                            \
    }

#define LoadYFromBPList LoadLiteralFromBPList

/* the structure integer(I) means constant integer I */
#define LoadConstantFromBPList {                                \
        READ_FROM_BCINSTS(temp);                                \
        if (ISSTRUCT(temp)) {                                   \
            BPLONG_PTR struct_ptr;                              \
            BPLONG i;                                           \
            struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(temp);       \
            i = FOLLOW(struct_ptr+1);                           \
            DEREF(i);                                           \
            *inst_addr = i;                                     \
        } else {                                                \
            temp = INTVAL(temp);                                \
            *inst_addr = ADDTAG(reloc_table[temp >> 2], ATM);   \
        }                                                       \
        inst_addr++;                                            \
    }

#define LoadAddrFromBPList {                    \
        READ_FROM_BCINSTS(temp);                \
        temp = INTVAL(temp);                    \
        *inst_addr = (BPLONG)RELOC_ADDR(temp);  \
        inst_addr++;                            \
    }

#define LoadStructFromBPList {                  \
        READ_FROM_BCINSTS(temp);                \
        temp = INTVAL(temp);                    \
        *inst_addr = (BPLONG)reloc_table[temp]; \
        inst_addr++;                            \
    }

/*  Z is integer(i) or an integer. The integer means C, u(y), or v(y) depending on the last two bits */
#define LoadZFromBPList {                                               \
        READ_FROM_BCINSTS(temp);                                        \
        if (ISSTRUCT(temp)) {                                           \
            BPLONG_PTR struct_ptr;                                      \
            BPLONG i;                                                   \
            struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(temp);               \
            i = FOLLOW(struct_ptr+1);                                   \
            DEREF(i);                                                   \
            *inst_addr = i;                                             \
        } else {                                                        \
            temp = INTVAL(temp);                                        \
            if (TAG(temp) == ATM) {                                     \
                *inst_addr = ADDTAG(reloc_table[temp >> 2], ATM);       \
            } else {                                                    \
                *inst_addr = temp;                                      \
            }                                                           \
        }                                                               \
        inst_addr++;                                                    \
    }

#define LoadZsFromBPList(n) {                   \
        while (n > 0) {                         \
            LoadZFromBPList;                    \
            n--;                                \
        }                                       \
    }

#define LoadYsFromBPList(n) {                   \
        while (n > 0) {                         \
            LoadLiteralFromBPList;              \
            n--;                                \
        }                                       \
    }

#define LoadConstantsFromBPList(n) {            \
        while (n > 0) {                         \
            LoadZFromBPList;                    \
            n--;                                \
        }                                       \
    }

/* Load in-memory byte codes */
int c_LOAD_BYTE_CODE_FROM_BPLISTS() {
    BPLONG BCSyms, BCInsts, BCHashTabs;
    BPLONG total_size;

    void load_syms_fromlist();
    void load_text_fromlist();
    void load_hashtab_fromlist();

    psc_bytes = ARG(1, 6); DEREF(psc_bytes); psc_bytes = INTVAL(psc_bytes);
    text_bytes = ARG(2, 6); DEREF(text_bytes); text_bytes = INTVAL(text_bytes);
    index_bytes = ARG(3, 6); DEREF(index_bytes); index_bytes = INTVAL(index_bytes);
    BCSyms = ARG(4, 6); DEREF(BCSyms);
    BCInsts = ARG(5, 6); DEREF(BCInsts);
    BCHashTabs = ARG(6, 6); DEREF(BCHashTabs);

    /*
      printf("=>load %d %d %d\n",psc_bytes,text_bytes,index_bytes);
      write_term(BCSyms); printf("\n");
      write_term(BCInsts); printf("\n");
      write_term(BCHashTabs); printf("\n");
    */

    total_size = sizeof(BPLONG)*(text_bytes + index_bytes + psc_bytes) + 10000;
    if ((CHAR_PTR)curr_fence+total_size > (CHAR_PTR)parea_water_mark) {
        int success = 0;
        if (total_size > parea_size) parea_size = total_size;
        ALLOCATE_NEW_PAREA_BLOCK(parea_size, success);
        if (success == 0) {
            bp_exception = bp_out_of_memory_atom;
            return BP_ERROR;
        }
    }
    load_syms_fromlist(BCSyms);
    load_text_fromlist(BCInsts);
    load_hashtab_fromlist(BCHashTabs);

    *inst_addr++ = endfile;
    *inst_addr = 0;  /* force 0 address (BPLONG) */
    last_text = (BPLONG_PTR)inst_addr;
    inst_addr++;
    curr_fence = (CHAR_PTR)inst_addr;
    return BP_TRUE;
}  /* end of loader */

/* Create symbols
   Each BCSym takes the form sym(EpOffset,Arity,Len,Sym).
*/
void load_syms_fromlist(BCSyms)
    BPLONG BCSyms;
{
    BPLONG i, j;

    i = 0;
    while (ISLIST(BCSyms)) {
        BPLONG_PTR list_ptr, struct_ptr;
        BPLONG sym_struct, ep_offset, arity, len, atm;
        SYM_REC_PTR sym_ptr;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(BCSyms);
        sym_struct = FOLLOW(list_ptr); DEREF(sym_struct);
        BCSyms = FOLLOW(list_ptr+1); DEREF(BCSyms);
        DEREF(sym_struct);
        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(sym_struct);
        ep_offset = FOLLOW(struct_ptr+1); DEREF(ep_offset); ep_offset = INTVAL(ep_offset);
        arity = FOLLOW(struct_ptr+2); DEREF(arity); arity = INTVAL(arity);
        len = FOLLOW(struct_ptr+3); DEREF(len); len = INTVAL(len);
        atm = FOLLOW(struct_ptr+4); DEREF(atm);
        sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(atm);
        reloc_table[i] = insert_sym(GET_NAME(sym_ptr), len, arity);
        set_temp_ep(reloc_table[i], ep_offset*sizeof(BPLONG));

        i++;
        if (i >= MAXSYMS) {
            bp_exception = out_of_range;
            quit("Out of range in symbol table");
        }
    }
    ALIGN(CHAR_PTR, curr_fence);
    for (j = 0; j < i; j++)
        set_real_ep(reloc_table[j], curr_fence);
}

void load_text_fromlist(BCInsts)
    BPLONG BCInsts;
{
    BPLONG n;
    SYM_REC_PTR sym_ptr;

    /* load text */
    inst_addr = (BPLONG_PTR)curr_fence;
    if (inst_begin == 0)
        inst_begin = (BPLONG_PTR)inst_addr;
    else
        *last_text = (BPLONG)inst_addr;

    CHECK_PCODE(curr_fence, sizeof(BPLONG)*text_bytes);

    while (ISLIST(BCInsts)) {
        BPLONG current_opcode;
        BPLONG_PTR list_ptr;
        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(BCInsts);
        current_opcode = FOLLOW(list_ptr); DEREF(current_opcode); current_opcode = INTVAL(current_opcode);
        BCInsts = FOLLOW(list_ptr+1); DEREF(BCInsts);

#ifdef GCC
        *(void **)inst_addr++ = jmp_table[current_opcode];
#else
        *inst_addr++ = current_opcode;
#endif

#include "load_inst_frombplist.h"
    }
}

/* load hash tables used by hash instructions in text
   BCHashTabs takes the form after_hash(Lab1,Op,Num,Lab2,HashArgs)
   where each HashArg takes the form hash_arg(Type,Nval,Lab)
*/
void load_hashtab_fromlist(BCHashTabs)
    BPLONG BCHashTabs;
{
    void get_index_tab_fromlist();

    CHECK_PCODE((CHAR_PTR)inst_addr, index_bytes);
    while (ISLIST(BCHashTabs)) {
        BPLONG hashtab, hash_inst_addr, hash_reg, alt, clause_no, HashArgs;
        BPLONG_PTR list_ptr, struct_ptr;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(BCHashTabs);
        hashtab = FOLLOW(list_ptr); DEREF(hashtab);
        BCHashTabs = FOLLOW(list_ptr+1); DEREF(BCHashTabs);
        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(hashtab);

        temp = FOLLOW(struct_ptr+1); DEREF(temp); temp = INTVAL(temp);
        hash_inst_addr = (BPLONG)RELOC_ADDR(temp);

        hash_reg = FOLLOW(struct_ptr+2); DEREF(hash_reg); hash_reg = INTVAL(hash_reg);

        clause_no = FOLLOW(struct_ptr+3); DEREF(clause_no); clause_no = INTVAL(clause_no);

        temp = FOLLOW(struct_ptr+4); DEREF(temp); temp = INTVAL(temp);
        alt = (BPLONG)RELOC_ADDR(temp);

        HashArgs = FOLLOW(struct_ptr+5); DEREF(HashArgs);

        get_index_tab_fromlist(HashArgs, clause_no);
        inst_addr = gen_index(hash_inst_addr, clause_no, alt);
    }
}

/* Recall that each HashArg takes the form hash_arg(Type,Nval,Lab) */
void get_index_tab_fromlist(HashArgs, clause_no)
    BPLONG HashArgs, clause_no;
{
    BPLONG hashval, size, j;
    BPLONG val, ttype;
    BPLONG_PTR label;

    hptr = heap_top;
    size = bp_hsize(clause_no);
    if (size > index_table_size) {
        if (indextab != NULL) free(indextab);
        indextab = (struct hrec *)malloc(sizeof(struct hrec)*size);
        index_table_size = size;
    }

    for (j = 0; j < size; j++) {
        indextab[j].l = 0;
        indextab[j].link = (BPLONG_PTR)(&(indextab[j].link));
    }

    while (ISLIST(HashArgs)) {
        BPLONG hash_item, type;
        BPLONG_PTR list_ptr, struct_ptr;
        SYM_REC_PTR sym_ptr;
        char *nameptr;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(HashArgs);
        hash_item = FOLLOW(list_ptr); DEREF(hash_item);
        HashArgs = FOLLOW(list_ptr+1); DEREF(HashArgs);
        struct_ptr = (BPLONG_PTR)UNTAGGED_ADDR(hash_item);

        type = FOLLOW(struct_ptr+1); DEREF(type);  /* type = c, i, or t */
        sym_ptr = GET_ATM_SYM_REC(type);
        nameptr = GET_NAME(sym_ptr);

        val = FOLLOW(struct_ptr+2); DEREF(val);
        switch (*nameptr) {
        case 'i':
            ttype = 0;
            break;
        case 's':
            val = INTVAL(val);
            val = (BPLONG)reloc_table[val];
            if (val == (BPLONG)list_psc) {
                ttype = 1;
            } else {
                ttype = 2;
            }
            break;
        case 'c':
            val = INTVAL(val);
            val = (BPLONG)reloc_table[val];
            if (val == UNTAGGED_ADDR(nil_sym))
                ttype = 3;
            else
                ttype = 4;
            val = ADDTAG(val, ATM);
            break;
        default:
            printf("WARNING: unknown type %c in get_index_tab\n", *nameptr);
            ttype = 0;
            val = 0;
        }
        temp = FOLLOW(struct_ptr+3); DEREF(temp); temp = INTVAL(temp);
        label = RELOC_ADDR(temp);
        hashval = IHASH(val, size);
        inserth(ttype, val, label, &indextab[hashval]);
    }
}


/********************************************************************/
#define LoadLiteralFromCArray {                 \
        *inst_addr = bc_insts[count++];         \
        inst_addr++;                            \
    }

#define LoadYFromCArray LoadLiteralFromCArray

/* bc_insts[count+1] should be tagged with INT or not depending on bc_insts[count] (1 means tagged and 0 means untagged */
#define LoadConstantFromCArray {                                        \
        if (bc_optags[count] == 1) {                                    \
            *inst_addr = MAKEINT(bc_insts[count++]);                    \
        } else {                                                        \
            *inst_addr = ADDTAG(reloc_table[bc_insts[count++] >> 2], ATM); \
        }                                                               \
        inst_addr++;                                                    \
    }

#define LoadAddrFromCArray {                    \
        temp = bc_insts[count++];               \
        *inst_addr = (BPLONG)RELOC_ADDR(temp);  \
        inst_addr++;                            \
    }

#define LoadStructFromCArray {                  \
        temp = bc_insts[count++];               \
        *inst_addr = (BPLONG)reloc_table[temp]; \
        inst_addr++;                            \
    }

/*  Z is integer(i) or an integer. The integer means C, u(y), or v(y) depending on the last two bits */
#define LoadZFromCArray {                                               \
        if (bc_optags[count] == 1) {                                    \
            *inst_addr = MAKEINT(bc_insts[count++]);                    \
        } else {                                                        \
            temp = bc_insts[count++];                                   \
            if (TAG(temp) == ATM) {                                     \
                *inst_addr = ADDTAG(reloc_table[temp >> 2], ATM);       \
            } else {                                                    \
                *inst_addr = temp;                                      \
            }                                                           \
        }                                                               \
        inst_addr++;                                                    \
    }

#define LoadZsFromCArray(n) {                   \
        while (n > 0) {                         \
            LoadZFromCArray;                    \
            n--;                                \
        }                                       \
    }

#define LoadYsFromCArray(n) {                   \
        while (n > 0) {                         \
            LoadLiteralFromCArray;              \
            n--;                                \
        }                                       \
    }

#define LoadConstantsFromCArray(n) {            \
        while (n > 0) {                         \
            LoadZFromCArray;                    \
            n--;                                \
        }                                       \
    }

typedef struct {
    int offset;
    BYTE arity;
    BYTE len;
    char *name;
} BC_SYM;

#ifdef FZN_PICAT_SAT
#include "fzn_picat_sat_bc.h"
#elif FZN_PICAT_CP
#include "fzn_picat_cp_bc.h"
#elif FZN_PICAT_MIP
#include "fzn_picat_mip_bc.h"
#elif FZN_PICAT_SMT
#include "fzn_picat_smt_bc.h"
#elif PB_PICAT
#include "pb_picat_bc.h"
#elif XCSP_PICAT
#include "xcsp_picat_bc.h"
#elif PICAT
#include "picat_bc.h"
#else
#include "bp_bc.h"
#endif

/* Load byte codes stored in C arrays */
int load_byte_code_from_c_array() {
    void load_syms_from_c_array();
    void load_text_from_c_array();
    void load_hashtab_from_c_array();

    load_syms_from_c_array();
    load_text_from_c_array();
    load_hashtab_from_c_array();

    *inst_addr++ = endfile;
    *inst_addr = 0;  /* force 0 address (BPLONG) */
    last_text = (BPLONG_PTR)inst_addr;
    inst_addr++;
    curr_fence = (CHAR_PTR)inst_addr;
    return BP_TRUE;
}

void load_syms_from_c_array() {
    BPLONG i;
    BPLONG num_of_syms = sizeof(bc_syms)/sizeof(BC_SYM);

    //  printf("number of syms=%d\n",num_of_syms);

    for (i = 0; i < num_of_syms; i++) {
        reloc_table[i] = insert_sym(bc_syms[i].name, bc_syms[i].len, bc_syms[i].arity);
        set_temp_ep(reloc_table[i], (bc_syms[i].offset)*sizeof(BPLONG));
    }
    ALIGN(CHAR_PTR, curr_fence);
    for (i = 0; i < num_of_syms; i++)
        set_real_ep(reloc_table[i], curr_fence);
}

void load_text_from_c_array() {
    BPLONG n;
    SYM_REC_PTR sym_ptr;
    BPLONG current_opcode = 0;
    BPLONG count;
    BPLONG text_array_size = sizeof(bc_insts)/sizeof(int);

    //  printf("text size=%d\n",text_array_size);

    inst_addr = (BPLONG_PTR)curr_fence;
    if (inst_begin == 0)
        inst_begin = (BPLONG_PTR)inst_addr;
    else
        *last_text = (BPLONG)inst_addr;
    count = 0;
    while (count < text_array_size) {
        current_opcode = bc_insts[count++];

        /*    printf("op=%d\n",current_opcode);
              printf("\t%s",inst_name[current_opcode]);
        */

#ifdef GCC
        *(void **)inst_addr++ = jmp_table[current_opcode];
#else
        *inst_addr++ = current_opcode;
#endif
#include "load_inst_fromcarray.h"
    }
}

void load_hashtab_from_c_array() {
    BPLONG hash_inst_addr, alt, clause_no;
    BPLONG count, hash_array_size;

    BPLONG hashval, size, j;
    BYTE type;
    BPLONG val, ttype;
    BPLONG_PTR label;

    BPLONG n_hashtabs = 0;
    count = 0;
    hash_array_size = sizeof(bc_indecies)/sizeof(int);

    //  printf("hash_array_size =%d\n",hash_array_size);

    while (count < hash_array_size) {
        n_hashtabs++;
        hash_inst_addr = (BPLONG)RELOC_ADDR(bc_indecies[count++]);
        //        hash_reg = bc_indecies[count++];
        bc_indecies[count++];
        clause_no = bc_indecies[count++];
        alt = (BPLONG)RELOC_ADDR(bc_indecies[count++]);

        //    printf("lab=%d hash_reg=%d clause_no=%d alt=%d\n",hash_inst_addr,hash_reg,clause_no,alt);

        hptr = heap_top;
        size = bp_hsize(clause_no);
        if (size > index_table_size) {
            if (indextab != NULL) free(indextab);
            indextab = (struct hrec *)malloc(sizeof(struct hrec)*size);
            index_table_size = size;
        }

        for (j = 0; j < size; j++) {
            indextab[j].l = 0;
            indextab[j].link = (BPLONG_PTR)(&(indextab[j].link));
        }

        for (j = 0; j < clause_no; j++) {
            type = (BYTE)bc_indecies[count++];
            val = bc_indecies[count++];
            //      printf("type=%d val=%d\n",type,val);
            switch (type) {
            case 'i':
                ttype = 0;
                val = MAKEINT(val);
                break;
            case 's':
                val = (BPLONG)reloc_table[val];
                if (val == (BPLONG)list_psc) {
                    ttype = 1;
                } else {
                    ttype = 2;
                }
                break;
            case 'c':
                val = (BPLONG)reloc_table[val];
                if (val == UNTAGGED_ADDR(nil_sym))
                    ttype = 3;
                else
                    ttype = 4;
                val = ADDTAG(val, ATM);
                break;
            default:
                printf("ERROR: unknown type %c in get_index_tab\n", type);
                exit(1);
            }
            label = RELOC_ADDR(bc_indecies[count++]);
            hashval = IHASH(val, size);
            inserth(ttype, val, label, &indextab[hashval]);
        }
        inst_addr = gen_index(hash_inst_addr, clause_no, alt);
    }
}
