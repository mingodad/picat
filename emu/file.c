/********************************************************************
 *   File   : file.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2024

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/
#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include "basic.h"
#include "term.h"
#include "bapi.h"
#include "dynamic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "extern_decl.h"
extern char *string_in;

#ifndef WIN32
#include <dirent.h>
#endif

#ifdef WIN32
#include <time.h>
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <time.h>
#ifdef DARWIN
#include <sys/time.h>
#else
#include <sys/times.h>
#endif
#include <sys/resource.h>
#endif

#ifdef __MINGW32__
#include <direct.h>
#define mkdir _mkdir
#define rmdir _rmdir
#define S_ISLNK(x) 0
#define S_ISSOCK(x) 0
#endif 

#if defined(WIN32) && !defined(__MINGW32__) 
#define S_ISREG(mode) (mode & S_IFREG)
#define S_ISDIR(mode) (mode & S_IFDIR)
#define S_ISFIFO(mode) (mode & _S_IFIFO)
#define S_ISLNK(mode) 0
#define S_ISSOCK(mode) 0
#endif

#ifdef WIN32
#define CHANGE_FILE_SEPARATOR(buf) replace_str_char(buf, '/', '\\')
#else
#define CHANGE_FILE_SEPARATOR(buf) replace_str_char(buf, '\\', '/')
#endif

#if defined(WIN32) && defined(M64BITS)
#define sys_access(a, b) _access(a, b)
#else
int access(const char *pathname, int mode);
#define sys_access(a, b) access(a, b)
#endif

#define CAR 1
#define CDR 0
#define MAXFILES 40

#define EMIT4(op) {                                                     \
        b4 = (BYTE)(op & 0xff);                                         \
        op >>= 8;                                                       \
        b3 = (BYTE)(op & 0xff);                                         \
        op >>= 8;                                                       \
        b2 = (BYTE)(op & 0xff);                                         \
        op >>= 8;                                                       \
        b1 = (BYTE)op;                                                  \
        putc(b1, curr_out); putc(b2, curr_out); putc(b3, curr_out); putc(b4, curr_out);}

BPLONG line_position = 0;
BPLONG out_line_no = 0;
// static UW16 fileerrors = 0;  /* abort, or not on file errors */
static SYM_REC_PTR user_sym_ptr, user_input_sym_ptr, user_output_sym_ptr, user_error_sym_ptr, sym_ptr;

static FILE *tempfile;
static CHAR full_file_name[MAX_FILE_NAME_LEN];

static char bp_buf[MAX_STR_LEN];

static BPLONG user_word, user_input_word, user_output_word, user_error_word;

int format_output_dest = 0;
extern char chars_pool[MAX_CHARS_IN_POOL];
extern int chars_pool_index;

#define CHECK_CHARS_POOL_OVERFLOW(len)                  \
    if (chars_pool_index+len >= MAX_CHARS_IN_POOL) {    \
        bp_exception = out_of_range;                    \
        return BP_ERROR;                                \
    }

#define CHECK_FILE_INDEX(index)                                         \
    if (index < 0 || index > file_tab_end || file_table[index].fdes == NULL) { \
        bp_exception = could_not_open_stream;                           \
        return BP_ERROR;                                                \
    }


struct ftab_ent
{
    BPLONG mode;  /* 0:read, 1:write, 2:append */
    BPLONG name_atom;  /* name of the stream */
    FILE *fdes;  /* file descriptor for this constant */
    BPLONG aliases;
    BPLONG lastc;
    BPLONG line_no;
    BPLONG line_position;
    BYTE eos;  /* at, past, not */
    BYTE eof_action;  /* ERROR, CODE, or RESET */
    BYTE type;  /* BINARY or TEXT */
};

#define READ_MODE 0
#define WRITE_MODE 1
#define APPEND_MODE 2
#define SOCKET 3

#define STREAM_TYPE_BINARY 0
#define STREAM_TYPE_TEXT 1

#define STREAM_EOF_ACTION_ERROR 0
#define STREAM_EOF_ACTION_CODE 1
#define STREAM_EOF_ACTION_RESET 2

#define STREAM_AT_EOS 0
#define STREAM_PAST_EOS 1
#define STREAM_NOT_EOS 2

/* table of open files; 0 is always stdin, 1 is always stdout */
static struct ftab_ent file_table[MAXFILES];
static BPLONG file_tab_end = 0;  /* last used entry in file_table */

/* index of current input (output) stream in file_table */
static BPLONG in_file_i, out_file_i;

/**********************************************************************/
void replace_str_char(char *buf, char och, char nch) {
    int i = 0;
    while (*(buf+i) != '\0') {
        if (*(buf+i) == och) {
            *(buf+i) = nch;
        }
        i++;
    }
}


/*
  monitor_in_file_i(char *str,int i){
  if (in_file_i>40){
  printf("%s %d in_file_i=%d\n",str,i,in_file_i);
  quit("strange in_file_i\n");
  }
  }
*/
int b_STREAM_IS_OPEN_c(BPLONG Index)
{
    DEREF(Index);
    Index = INTVAL(Index);
    if (Index < 0 || Index > file_tab_end) return BP_FALSE;
    if (file_table[Index].fdes == NULL) return BP_FALSE; else return BP_TRUE;
}

int b_STREAM_SET_TYPE_cc(BPLONG Index, BPLONG Type)
{
    DEREF(Index); DEREF(Type);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    file_table[Index].type = (BYTE)INTVAL(Type);
    return BP_TRUE;
}

int b_STREAM_SET_EOF_ACTION_cc(BPLONG Index, BPLONG Action)
{
    DEREF(Index); DEREF(Action);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    file_table[Index].eof_action = (BYTE)INTVAL(Action);
    return BP_TRUE;
}

int b_STREAM_ADD_ALIAS_cc(BPLONG Index, BPLONG Atom)
{
    BPLONG_PTR ptr;
    DEREF(Index); DEREF(Atom);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    ALLOCATE_FROM_PAREA(ptr, 2);
    if (ptr == NULL) {
        bp_exception = et_OUT_OF_MEMORY;
        return BP_ERROR;
    }
    FOLLOW(ptr) = Atom;
    FOLLOW(ptr+1) = file_table[Index].aliases;
    file_table[Index].aliases = ADDTAG(ptr, LST);
    return BP_TRUE;
}

int b_STREAM_UPDATE_EOS() {
    if (file_table[in_file_i].eos != STREAM_NOT_EOS &&
        file_table[in_file_i].eof_action == STREAM_EOF_ACTION_ERROR) {
        bp_exception = c_permission_error(et_INPUT, et_PAST_END_OF_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
    file_table[in_file_i].eos = STREAM_PAST_EOS;
    return BP_TRUE;
}

int b_STREAM_GET_FILE_NAME_cf(BPLONG Index, BPLONG Name)
{
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    ASSIGN_f_atom(Name, file_table[Index].name_atom);
    return BP_TRUE;
}

int b_STREAM_GET_MODE_cf(BPLONG Index, BPLONG Mode)
{
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    ASSIGN_f_atom(Mode, MAKEINT(file_table[Index].mode));
    return BP_TRUE;
}

int b_STREAM_GET_ALIASES_cf(BPLONG Index, BPLONG Aliases)
{
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    ASSIGN_f_atom(Aliases, file_table[Index].aliases);
    return BP_TRUE;
}

int b_STREAM_GET_EOS_cf(BPLONG Index, BPLONG Eos)
{
    BPLONG res;
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    if (Index == 0) {
        ASSIGN_f_atom(Eos, MAKEINT(STREAM_NOT_EOS));
        return BP_TRUE;
    }
    if (file_table[Index].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, c_stream_struct(Index));
        return BP_ERROR;
    }
    if (file_table[Index].mode != READ_MODE && file_table[Index].mode != SOCKET) return BP_FALSE;

    if (file_table[Index].eos == STREAM_NOT_EOS) {
        BPLONG n = getc(file_table[Index].fdes);
        if (n == EOF) {
            res = STREAM_AT_EOS;
        } else {
            res = STREAM_NOT_EOS;
        }
        ungetc(n, file_table[Index].fdes);
    } else {
        res = file_table[Index].eos;
    }
    ASSIGN_f_atom(Eos, MAKEINT(res));
    return BP_TRUE;
}

int b_STREAM_GET_EOF_ACTION_cf(BPLONG Index, BPLONG EofAction)
{
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    if (file_table[Index].mode != READ_MODE && file_table[Index].mode != SOCKET) return BP_FALSE;
    ASSIGN_f_atom(EofAction, MAKEINT(file_table[Index].eof_action));
    return BP_TRUE;
}

int b_STREAM_GET_TYPE_cf(BPLONG Index, BPLONG Type)
{
    DEREF(Index);
    Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    ASSIGN_f_atom(Type, MAKEINT(file_table[Index].type));
    return BP_TRUE;
}

int b_STREAM_CHECK_CURRENT_TEXT_INPUT() {

    if (file_table[in_file_i].type == STREAM_TYPE_TEXT) {
        return BP_TRUE;
    } else {
        bp_exception = c_permission_error(et_INPUT, et_BINARY_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
}

int b_STREAM_CHECK_CURRENT_TEXT_OUTPUT() {
    if (file_table[out_file_i].type == STREAM_TYPE_TEXT) {
        return BP_TRUE;
    } else {
        bp_exception = c_permission_error(et_OUTPUT, et_BINARY_STREAM, c_stream_struct(out_file_i));
        return BP_ERROR;
    }
}

/**********************************************************************/
void bp_trim_trailing_zeros(char *loc_bp_buf)
{
    BPLONG len;

    len = strlen(loc_bp_buf)-1;
    while (loc_bp_buf[len] == '0') len--;

    if (loc_bp_buf[len] == '.') {loc_bp_buf[++len] = '0';}
    loc_bp_buf[len+1] = '\0';

}

void bp_write_insert_dot_zero_if_needed(char *loc_bp_buf)
{
    int i, j, dot_found, len, e_index;
    char c;
    dot_found = 0;
    e_index = 0;
    len = strlen(loc_bp_buf);

    for (i = 0; i < len; i++) {
        c = loc_bp_buf[i];
        if (c == '.') dot_found = 1;
        if (c == 'e') {
            e_index = i;
            break;
        }
    }
    if (e_index != 0) {
        if (dot_found == 0) {
            for (j = len; j >= e_index; j--) {
                loc_bp_buf[j+2] = loc_bp_buf[j];
            }
            loc_bp_buf[e_index] = '.';
            loc_bp_buf[e_index+1] = '0';
        }
    } else {
        if (dot_found == 0) {
            loc_bp_buf[len] = '.';
            loc_bp_buf[len+1] = '0';
            loc_bp_buf[len+2] = '\0';
        }
    }
}

void bp_write_double(BPLONG op)
{
    double d;

    d = floatval(op);
    sprintf(bp_buf, "%.15lf", d);
    bp_trim_trailing_zeros(bp_buf);
    if (strlen(bp_buf) >= 10) {
        sprintf(bp_buf, "%g", d);
        bp_write_insert_dot_zero_if_needed(bp_buf);
    }
    fprintf(curr_out, "%s", bp_buf);
}

int bp_write_double_update_pos(BPLONG op)
{
    double d;
    BPLONG len;

    d = floatval(op);
    sprintf(bp_buf, "%.15lf", d);
    bp_trim_trailing_zeros(bp_buf);
    if (strlen(bp_buf) >= 10) {
        sprintf(bp_buf, "%g", d);
        bp_write_insert_dot_zero_if_needed(bp_buf);
    }
    len = strlen(bp_buf);
    line_position += len;
    if (format_output_dest == 0) {
        fputs(bp_buf, curr_out);
        fflush(curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    return BP_TRUE;
}

int bp_write_bigint(BPLONG op)
{
    int i = bp_write_bigint_to_str(op, bp_buf, MAX_STR_LEN);
    if (i == BP_ERROR) return BP_ERROR;
    fputs(bp_buf+i, curr_out);
    return BP_TRUE;
}


int bp_write_bigint_update_pos(BPLONG op)
{
    int i, len;

    i = bp_write_bigint_to_str(op, bp_buf, MAX_STR_LEN);
    if (i == BP_ERROR) return BP_ERROR;
    len = strlen(bp_buf+i);
    line_position += len;
    if (format_output_dest == 0) {
        fputs(bp_buf+i, curr_out);
        fflush(curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf+i);
        chars_pool_index += len;
    }
    return BP_TRUE;
}

#ifdef PICAT
int check_file_term(BPLONG term)
{
    DEREF(term);
    if (ISATOM(term) || b_IS_STRING_c(term)) {
        return BP_TRUE;
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
}
#else
int check_file_term(BPLONG term)
{
    SYM_REC_PTR sym_ptr;

    DEREF(term);
    if (!ISATOM(term)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    return BP_TRUE;
}
#endif

char *expand_file_name(char *s1) {
    CHAR_PTR s2;
    BPLONG i;
    int len;

    if (*s1 == '~') {
        s2 = getenv("HOME");
        if (s2 == NULL) {
            fputs("the environment variable HOME is not set\n", stderr); exit(1);
        }
        if (*(s1+1) == '/') {
            scat(s2, &s1[1], full_file_name);
        } else {
            i = strlen(s2);
            i--;
            while (*(s2 + i) != '/') {
                *(s2+i) = '\0';
                i--;
            }
            scat(s2, &s1[1], full_file_name);
        }
    } else {
        strcpy(full_file_name, s1);
    }
    len = strlen(full_file_name);
    if (len > 1 && (full_file_name[len-1] == '/' || full_file_name[len-1] == '\\'))  /* get rid of trailing /*/
        full_file_name[len-1] = '\0';
    return full_file_name;
}

#ifdef PICAT
char *get_file_name(BPLONG op) {
    CHAR s1[MAX_STR_LEN];

    DEREF(op);
    if (ISATOM(op)) {
        namestring(GET_SYM_REC(op), s1);
    } else {
        picat_str_to_c_str(op, s1, MAX_STR_LEN);
    }
    return expand_file_name(s1);
}
#else
char *get_file_name(BPLONG op)
{
    CHAR s1[MAX_STR_LEN];

    DEREF(op);
    namestring(GET_SYM_REC(op), s1);
    return expand_file_name(s1);
}
#endif

void bp_write_pname(CHAR_PTR name_ptr)
{
    fputs(name_ptr, curr_out);
    fflush(curr_out);
}

int bp_write_pname_update_pos(CHAR_PTR name_ptr, BPLONG length)
{
    line_position += length;
    if (format_output_dest == 0) {
        fputs(name_ptr, curr_out);
        fflush(curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(length);
        strcpy((chars_pool+chars_pool_index), name_ptr);
        chars_pool_index += length;
    }
    return BP_TRUE;
}

int graphic_char(char ch)
{
    switch (ch) {
    case '/':
    case '.':
    case '#':
    case '$':
    case '&':
    case '*':
    case '+':
    case '-':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '^':
    case '~':
    case '\\':
        return 1;
    default: return 0;
    }
}

int single_quote_needed(CHAR_PTR name_ptr, BPLONG length)
{
    CHAR ch;
    BPLONG i;

    /*  printf("single_quote_needed? %s\n",name_ptr);*/

    ch = *name_ptr;
    if (ch >= 'a' && ch <= 'z') {
        for (i = 1; i < length; i++) {
            ch = *(name_ptr+i);
            if (!((ch >= '0' && ch <= '9') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch == '_') ||
                  (ch >= 'a' && ch <= 'z')))
                return 1;
        }
        return 0;
    }
    return 1;
}

/* copy the str to buf, adding quotes when necessary */
int strcpy_add_quote(CHAR_PTR buf, CHAR_PTR str, BPLONG len)
{
    int i, new_len = len+2;
    *buf++ = '\'';
    for (i = 1; i <= len; ++i) {
        switch (*str) {
        case '\'':
        case '\\': *buf++ = '\\'; *buf++ = *str; new_len += 2; break;
        case '\a': *buf++ = '\\'; *buf++ = 'a'; new_len += 2; break;
        case '\b': *buf++ = '\\'; *buf++ = 'b'; new_len += 2; break;
        case '\r': *buf++ = '\\'; *buf++ = 'r'; new_len += 2; break;
        case '\f': *buf++ = '\\'; *buf++ = 'f'; new_len += 2; break;
        case '\t': *buf++ = '\\'; *buf++ = 't'; new_len += 2; break;
        case '\n': *buf++ = '\\'; *buf++ = 'n'; new_len += 2; break;
        case '\v': *buf++ = '\\'; *buf++ = 'v'; new_len += 2; break;
        default: *buf++ = *str;
        }
        str++;
    }
    *buf++ = '\'';
    *buf = '\0';
    return new_len;
}

void bp_write_qname_to_bp_buf(CHAR_PTR name_ptr, UW16 length)
{
    UW16 i, need_to_quote;
    int c;
    int bp_buf_index = 0;

    if (length >= MAX_STR_LEN) {fprintf(stderr, "%s\n", name_ptr); quit("atom too long\n");}
    need_to_quote = single_quote_needed(name_ptr, length);
    if (need_to_quote) {
        bp_buf[bp_buf_index++] = '\'';
        for (i = 1; i <= length; ++i) {
            c = *name_ptr;
            switch (c) {
            case '\'':
            case '\\': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = c; break;
            case '\a': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'a'; break;
            case '\b': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'b'; break;
            case '\r': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'r'; break;
            case '\f': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'f'; break;
            case '\t': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 't'; break;
            case '\n': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'n'; break;
            case '\v': bp_buf[bp_buf_index++] = '\\'; bp_buf[bp_buf_index++] = 'v'; break;
            default:
                bp_buf[bp_buf_index++] = c;
            }
            name_ptr++;
            if (bp_buf_index >= MAX_STR_LEN) {fprintf(stderr, "%s\n", name_ptr); quit("atom too long\n");}
        }
        bp_buf[bp_buf_index++] = '\'';
        bp_buf[bp_buf_index] = '\0';
    } else {
        strcpy(bp_buf, name_ptr);
    }
}

void bp_write_qname(CHAR_PTR name_ptr, BPLONG length)
{
    bp_write_qname_to_bp_buf(name_ptr, length);
    fputs(bp_buf, curr_out);
    fflush(curr_out);
}

int bp_write_qname_update_pos(CHAR_PTR name_ptr, BPLONG length)
{
    BPLONG len;

    bp_write_qname_to_bp_buf(name_ptr, length);
    len = strlen(bp_buf);
    line_position += len;
    if (format_output_dest == 0) {
        fputs(bp_buf, curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    return BP_TRUE;
}


int bp_write_var_update_pos(BPLONG op)
{
    BPLONG len;

    sprintf(bp_buf, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);
    len = strlen(bp_buf);
    line_position += len;
    if (format_output_dest == 0) {
        fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    return BP_TRUE;
}

int bp_write_domain_update_pos(BPLONG_PTR dv_ptr)
{
    BPLONG i, first, last;
    BPLONG low, high;

    first = DV_first(dv_ptr);
    last = DV_last(dv_ptr);

    bp_write_char_update_pos(':'); bp_write_char_update_pos(':'); bp_write_char_update_pos('[');
    if (IS_IT_DOMAIN(dv_ptr)) {
        bp_write_int_update_pos(first);
        bp_write_char_update_pos('.');
        bp_write_char_update_pos('.');
        bp_write_int_update_pos(last);
        return bp_write_char_update_pos(']');
    } else {
        low = first;
        i = first+1;
        while (i < last) {
            while(i < last && dm_true(dv_ptr, i)) i++;
            if (i == last) break;
            high = i-1;
            if (low == high) {
                bp_write_int_update_pos(low);
            } else {
                bp_write_int_update_pos(low);
                bp_write_char_update_pos('.');
                bp_write_char_update_pos('.');
                bp_write_int_update_pos(high);
            }
            bp_write_char_update_pos(',');
            i++;
            while (!dm_true(dv_ptr, i) && i < last) i++;
            low = i;
        }
        high = last;
        if (low == high) {
            bp_write_int_update_pos(low);
        } else {
            bp_write_int_update_pos(low);
            bp_write_char_update_pos('.');
            bp_write_char_update_pos('.');
            bp_write_int_update_pos(high);
        }
        return bp_write_char_update_pos(']');
    }
}

int bp_write_suspvar_update_pos(BPLONG op)
{
    BPLONG len;
    BPLONG_PTR dv_ptr;

    dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
    sprintf(bp_buf, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);
    len = strlen(bp_buf);
    line_position += len;
    if (format_output_dest == 0) {
        fprintf(curr_out, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    if (!IS_UN_DOMAIN(dv_ptr))
        return bp_write_domain_update_pos(dv_ptr);

    return BP_TRUE;
}



/* op is untagged */
int bp_write_int_update_pos(BPLONG op)
{
    BPLONG len;

    sprintf(bp_buf, BPLONG_FMT_STR, op);
    len = strlen(bp_buf);
    line_position += len;
    if (format_output_dest == 0) {
        fprintf(curr_out, BPLONG_FMT_STR, op);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }

    return BP_TRUE;
}

int bp_write_char_update_pos(char c)
{
    line_position++;
    if (format_output_dest == 0) {
        putc(c, curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(1);
        chars_pool[chars_pool_index++] = c;
    }

    return BP_TRUE;
}

int get_file_index(BPLONG cword, BPLONG mode)
{
    int i;

    for (i = 0; i <= file_tab_end; i++) {
        if (file_table[i].name_atom == cword && file_table[i].fdes != NULL)
            if (mode == file_table[i].mode ||
                (mode == APPEND_MODE && file_table[i].mode == WRITE_MODE) ||
                (mode == WRITE_MODE && file_table[i].mode == APPEND_MODE) ||
                file_table[i].mode == SOCKET)
                return i;
    }
    return -1L;
}

int next_file_index()
{
    int i;

    for (i = 3; i <= file_tab_end; i++)
        if (file_table[i].fdes == NULL) return i;
    if (file_tab_end >= MAXFILES-1) {
        return -1L;
    }
    file_tab_end++;
    return file_tab_end;
}

void release_file_index(int i)
{
    file_table[i].fdes = NULL;
    file_table[i].name_atom = nil_sym;
    file_table[i].aliases = nil_sym;
}

/*
  %   Context = 0b000 for alpha
  %   Context = 0b001 for quote
  %   Context = 0b010 for other
  %   Context = 0b100 for punct
*/
int b_ATOM_MODE_cf(BPLONG op1, BPLONG op2)
{
    SYM_REC_PTR sym_ptr;
    BPLONG i, length;
    char ch, *name_ptr;
    BPLONG_PTR top;
    int atom_mode = 0;

    DEREF(op1);
    sym_ptr = GET_SYM_REC(op1);
    length = GET_LENGTH(sym_ptr);
    name_ptr = GET_NAME(sym_ptr);

    ch = *name_ptr;
    switch (ch) {
    case '/': if (length >= 2 && *(name_ptr+1) == '*') atom_mode = 1; else atom_mode = 2; break;
    case '.': if (ch == '.' && length == 1) atom_mode = 1; else atom_mode = 2; break;
    case '#':
    case '$':
    case '&':
    case '*':
    case '+':
    case '-':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '^':
    case '~':
    case '\\':
    {for (i = 1; i < length; i++) {
                if (!graphic_char(*(name_ptr+i))) {atom_mode = 1; goto atom_mode_end;}
            };
            atom_mode = 2;
            break;
    }

    case '[':
        if (length == 2 && *(name_ptr+1) == ']') atom_mode = 2; else atom_mode = 1;
        break;

    case '{':
        if (length == 2 && *(name_ptr+1) == '}') atom_mode = 2; else atom_mode = 1;
        break;

    default:
        if (ch >= 'a' && ch <= 'z') {
            for (i = 1; i < length; i++) {
                ch = *(name_ptr+i);
                if (!((ch >= '0' && ch <= '9') ||
                      (ch >= 'A' && ch <= 'Z') ||
                      (ch == '_') ||
                      (ch >= 'a' && ch <= 'z'))) {
                    atom_mode = 1;
                    goto atom_mode_end;
                }
            }
        } else {
            atom_mode = 1;
        }
    }

atom_mode_end:
    ASSIGN_f_atom(op2, MAKEINT(atom_mode));
    return BP_TRUE;
}


int b_NORMAL_ATOM_c(BPLONG op)
{
    SYM_REC_PTR sym_ptr;
    BPLONG length;
    char *name_ptr;
    BPLONG_PTR top;

    DEREF(op);
    sym_ptr = GET_SYM_REC(op);
    length = GET_LENGTH(sym_ptr);
    name_ptr = GET_NAME(sym_ptr);

    if (single_quote_needed(name_ptr, length)) return BP_FALSE;
    return BP_TRUE;
}

int b_WRITENAME_c(BPLONG op)
{
    BPLONG_PTR top, dv_ptr;
    /*
      if (file_table[out_file_i].type==STREAM_TYPE_BINARY){
      bp_exception = c_permission_error(et_OUTPUT,et_BINARY_STREAM,c_stream_struct(out_file_i));
      return BP_ERROR;
      }
    */
    SWITCH_OP(op, writename_1,
              {fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      bp_write_pname(GET_NAME(sym_ptr));
                  } else {
                      op = INTVAL(op);
                      fprintf(curr_out, BPLONG_FMT_STR, op);
                  }
              },
              {fprintf(curr_out, ".");},
              {
                  if (IS_FLOAT_PSC(op)) {
                      bp_write_double(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint(op);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(op);
                      bp_write_pname(GET_NAME(sym_ptr));
                  }
              },
              {
                  dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
                  fprintf(curr_out, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);
                  if (!IS_UN_DOMAIN(dv_ptr))
                      print_domain(dv_ptr);});

    return BP_TRUE;
}

/* output to curr_out if output_dest==0, and chars_pool if output_dest==1 */
int c_format_set_dest() {
    BPLONG op;

    op = ARG(1, 1);
    DEREF(op);
    format_output_dest = INTVAL(op);
    if (format_output_dest == 1) {
        chars_pool_index = 0;
    }
    return BP_TRUE;
}

int c_format_get_line_pos() {
    BPLONG op;

    op = ARG(1, 1);
    return unify(op, MAKEINT(line_position));
}

int c_format_retrieve_codes() {
    BPLONG Codes, CodesR, i;

    Codes = ARG(1, 2);
    CodesR = ARG(2, 2);
    if (chars_pool_index > 0) {
        BPLONG lst0, lst;
        lst = lst0 = ADDTAG(heap_top, LST);
        for (i = 0; i < chars_pool_index-1; i++) {
            NEW_HEAP_NODE(MAKEINT(chars_pool[i]));
            lst = ADDTAG((heap_top+1), LST);
            NEW_HEAP_NODE(lst);
        }
        NEW_HEAP_NODE(MAKEINT(chars_pool[chars_pool_index-1]));
        lst = (BPLONG)heap_top;
        NEW_HEAP_FREE;
        unify(Codes, lst0);
        unify(CodesR, lst);
        return BP_TRUE;
    } else {
        return unify(Codes, CodesR);
    }
}

int c_fmt_writename() {
    BPLONG op;
    BPLONG_PTR top;

    op = ARG(1, 1);

    SWITCH_OP(op, writename_update_pos1,
              {bp_write_var_update_pos(op);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      return bp_write_pname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      return bp_write_int_update_pos(INTVAL(op));
                  }
              },
              {return bp_write_char_update_pos('.');},
              {
                  if (IS_FLOAT_PSC(op)) {
                      return bp_write_double_update_pos(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint_update_pos(op);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(op);
                      return bp_write_pname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  }
              },
              {return bp_write_suspvar_update_pos(op);});

    return BP_TRUE;
}

int b_WRITE_QUICK_c(BPLONG op)
{
    BPLONG_PTR dv_ptr;
    BPLONG_PTR top;

    if (file_table[out_file_i].type == STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_OUTPUT, et_BINARY_STREAM, c_stream_struct(out_file_i));
        return BP_ERROR;
    }
    SWITCH_OP(op, write_quick_1,
              {fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      bp_write_pname(GET_NAME(sym_ptr));
                  } else {
                      op = INTVAL(op);
                      fprintf(curr_out, BPLONG_FMT_STR, op);
                  }
              },
              {return BP_FALSE;},
              {
                  if (IS_FLOAT_PSC(op)) {
                      bp_write_double(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      BPLONG_PTR ptr;
                      BPLONG size;
                      ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
                      size = FOLLOW(ptr+1); size = INTVAL(size);
                      if (size > 36) return BP_FALSE;
                      return bp_write_bigint(op);
                  } else if (cg_is_component(op)) {
                      cg_print_component(op);
                  } else return BP_FALSE;
              },
              {
                  dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
                  fprintf(curr_out, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);
                  if (!IS_UN_DOMAIN(dv_ptr))
                      print_domain(dv_ptr);});

    return BP_TRUE;
}

int c_fmt_write_quick() {
    BPLONG op;
    BPLONG_PTR top;

    op = ARG(1, 1);
    SWITCH_OP(op, write_quick_1,
              {return bp_write_var_update_pos(op);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      return bp_write_pname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      return bp_write_int_update_pos(INTVAL(op));
                  }
              },
              {return BP_FALSE;},
              {if (IS_FLOAT_PSC(op)) {
                      return bp_write_double_update_pos(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint_update_pos(op);
                  } else if (cg_is_component(op)) {
                      cg_print_component(op);
                  } else return BP_FALSE;},
              {return bp_write_suspvar_update_pos(op);});

    return BP_TRUE;
}

int b_WRITEQNAME_c(BPLONG op)
{
    BPLONG_PTR top;
    BPLONG_PTR dv_ptr;

    SWITCH_OP(op, writeqname_1,
              {fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      bp_write_qname(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      op = INTVAL(op);
                      fprintf(curr_out, BPLONG_FMT_STR, op);
                  }
              },
              {fprintf(curr_out, ".");},
              {
                  if (IS_FLOAT_PSC(op)) {
                      bp_write_double(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint(op);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(op);
                      bp_write_qname(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  }
              },
              {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
                  fprintf(curr_out, "_0" BPULONG_FMT_STR , (BPULONG)dv_ptr-(BPULONG)stack_low_addr);});
    return BP_TRUE;
}

int c_fmt_writeqname() {
    BPLONG op;
    BPLONG_PTR top;

    op = ARG(1, 1);
    SWITCH_OP(op, writeqname_1,
              {return bp_write_var_update_pos(op);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      return bp_write_qname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      return bp_write_int_update_pos(INTVAL(op));
                  }
              },
              {return bp_write_char_update_pos('.');},
              {if (IS_FLOAT_PSC(op)) {
                      return bp_write_double_update_pos(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint_update_pos(op);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(op);
                      return bp_write_qname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  }
              },
              {return bp_write_suspvar_update_pos(op);});
    return BP_TRUE;
}

int b_WRITEQ_QUICK_c(BPLONG op)
{
    BPLONG_PTR top;
    BPLONG_PTR dv_ptr;

    SWITCH_OP(op, writeqquick_1,
              {fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      bp_write_qname(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      op = INTVAL(op);
                      fprintf(curr_out, BPLONG_FMT_STR, op);
                  }
              },
              {return BP_FALSE;},
              {
                  if (IS_FLOAT_PSC(op)) {
                      bp_write_double(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      BPLONG_PTR ptr;
                      BPLONG size;
                      ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
                      size = FOLLOW(ptr+1); size = INTVAL(size);
                      if (size > 36) return BP_FALSE;
                      return bp_write_bigint(op);
                  } else return BP_FALSE;
              },
              {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
                  fprintf(curr_out, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);});
    return BP_TRUE;
}

int c_fmt_writeq_quick() {
    BPLONG op;
    BPLONG_PTR top;

    op = ARG(1, 1);
    SWITCH_OP(op, writeqquick_1,
              {return bp_write_var_update_pos(op);},
              {
                  if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      return bp_write_qname_update_pos(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
                  } else {
                      return bp_write_int_update_pos(INTVAL(op));
                  }
              },
              {return BP_FALSE;},
              {if (IS_FLOAT_PSC(op))
                      return bp_write_double_update_pos(op);
                  else if (IS_BIGINT_PSC(op))
                      return bp_write_bigint_update_pos(op);
                  else return BP_FALSE;},
              {return bp_write_suspvar_update_pos(op);});
    return BP_TRUE;
}

int c_fmt_nl() {
    out_line_no++; line_position = 0;
    if (format_output_dest == 0) {
        putc('\n', curr_out);
        fflush(curr_out);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(1);
        chars_pool[chars_pool_index++] = '\n';
    }
    return BP_TRUE;
}

int b_NL()
{
    putc('\n', curr_out);
    fflush(curr_out);
    return BP_TRUE;
}

void write_space() {
    putc(' ', curr_out);
}


/* deprecated */
int b_PUT_BYTE_c(BPLONG op)
{
    BPLONG_PTR top;

    DEREF(op);
    if (ISREF(op)) {
        bp_exception = et_INSTANTIATION_ERROR;
        return BP_ERROR;
    }
    if (!ISINT(op)) {
        bp_exception = c_type_error(et_BYTE, op);
        return BP_ERROR;
    }
    op = INTVAL(op);
    if (op < 0 || op > 255) {
        bp_exception = c_type_error(et_BYTE, MAKEINT(op));
        return BP_ERROR;
    }
    if (file_table[out_file_i].type == STREAM_TYPE_TEXT) {
        bp_exception = c_permission_error(et_OUTPUT, et_TEXT_STREAM, c_stream_struct(out_file_i));
        return BP_ERROR;
    }
    putc(op, curr_out);

    return BP_TRUE;
}

/* deprecated */
int b_PUT_CODE_c(BPLONG op)
{
    BPLONG_PTR top;

    DEREF(op);
    if (ISREF(op)) {
        bp_exception = et_INSTANTIATION_ERROR;
        return BP_ERROR;
    }
    if (!ISINT(op)) {
        bp_exception = c_type_error(et_INTEGER, op);
        return BP_ERROR;
    }
    op = INTVAL(op);
    /*
      if (op<0 || op>255){
      bp_exception = c_representation_error(et_CHARACTER_CODE);
      return BP_ERROR;
      }
    */
    if (file_table[out_file_i].type == STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_OUTPUT, et_BINARY_STREAM, c_stream_struct(out_file_i));
        return BP_ERROR;
    }
    if (op <= 127) {
        putc(op, curr_out);
    } else {  /* unicode, write in utf8 format */
        char s[5], *ch_ptr, *ch_ptr0;
        ch_ptr = utf8_codepoint_to_str(op, s);
        ch_ptr0 = s;
        while (ch_ptr0 < ch_ptr) {
            putc(*ch_ptr0++, curr_out);
        }
    }
    return BP_TRUE;
}

int b_PUT_c(BPLONG op)
{
    BPLONG res;
    BPLONG_PTR top;

    DEREF(op);
    if (ISINT(op)) goto b_put;
    res = eval_arith(op);
    if (res == BP_ERROR || (!ISINT(res))) {
        bp_exception = c_type_error(et_INTEGER, res); return BP_ERROR;
    }
    op = res;
b_put:
    op = INTVAL(op);
    putc(op, curr_out);

    return BP_TRUE;
}

int c_put_update_pos() {
    BPLONG op;
    BPLONG res;
    BPLONG_PTR top;

    op = ARG(1, 1);
    DEREF(op);
    if (ISINT(op)) goto b_put;
    res = eval_arith(op);
    if (res == BP_ERROR || (!ISINT(res))) {
        bp_exception = c_type_error(et_INTEGER, res); return BP_ERROR;
    }
    op = res;
b_put:
    op = INTVAL(op);
    if ((char)op == '\n') {
        line_position = 0;
    } else {
        line_position++;
    }
    if (format_output_dest == 0) {
        b_put_char_code(curr_out, op);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(1);
        chars_pool[chars_pool_index++] = (char)op;
    }
    return BP_TRUE;
}

int b_TAB_c(BPLONG op)
{
    register BPLONG_PTR top;
    BPLONG res;

    DEREF(op);
    if (ISINT(op)) goto b_tab;
    res = eval_arith(op);
    if (res == BP_ERROR || (!ISINT(res))) {
        bp_exception = c_type_error(et_INTEGER, res); return BP_ERROR;
    }
    op = res;
b_tab:
    op = INTVAL(op);
    if (op < 0)
        return 1;
    line_position += op;
    for ( ; op > 0; op--)
        putc(' ', curr_out);

    return 1;
}

/* deprecated */
int b_GET_BYTE_f(BPLONG op)
{
    BPLONG n;
    BPLONG_PTR top;

    DEREF(op);
    if (file_table[in_file_i].type == STREAM_TYPE_TEXT) {
        bp_exception = c_permission_error(et_INPUT, et_TEXT_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
    n = getc(curr_in);

    if (n == EOF) {
        clearerr(curr_in);
        if (file_table[in_file_i].eos == STREAM_PAST_EOS &&
            file_table[in_file_i].eof_action == STREAM_EOF_ACTION_ERROR) {
            bp_exception = c_permission_error(et_INPUT, et_PAST_END_OF_STREAM, c_stream_struct(in_file_i));
            return BP_ERROR;
        };
        file_table[in_file_i].eos = STREAM_PAST_EOS;
    }

    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}

/* deprecated */
int b_GET_CODE_f(BPLONG op)
{
    BPLONG n;
    BPLONG_PTR top;

    DEREF(op);
    if (file_table[in_file_i].type == STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_INPUT, et_BINARY_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
    n = getc(curr_in);

    if (n == 0) {
        bp_exception = c_representation_error(et_CHARACTER);
        return BP_ERROR;
    }
    if (n == EOF) {
        clearerr(curr_in);
        if (file_table[in_file_i].eos == STREAM_PAST_EOS &&
            file_table[in_file_i].eof_action == STREAM_EOF_ACTION_ERROR) {
            bp_exception = c_permission_error(et_INPUT, et_PAST_END_OF_STREAM, c_stream_struct(in_file_i));
            return BP_ERROR;
        };
        file_table[in_file_i].eos = STREAM_PAST_EOS;
    }
    if (n & 0x80) {  /* leading byte of a utf8 char? */
        n = utf8_getc(curr_in, n);
    }

    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}

int b_GET0_f(BPLONG op)
{
    BPLONG n;

    n = getc(curr_in);

    if (n == EOF)
        clearerr(curr_in);

    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}

/* deprecated */
int b_PEEK_BYTE_f(BPLONG op)
{
    BPLONG n;

    if (file_table[in_file_i].eos == STREAM_PAST_EOS &&
        file_table[in_file_i].eof_action == STREAM_EOF_ACTION_ERROR) {
        bp_exception = c_permission_error(et_INPUT, et_PAST_END_OF_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
    if (file_table[in_file_i].type != STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_INPUT, et_TEXT_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }

    n = getc(curr_in);

    if (n == EOF)
        clearerr(curr_in);
    else
        ungetc(n, curr_in);

    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}

/* deprecated */
int b_PEEK_CODE_f(BPLONG op)
{
    BPLONG n;

    if (file_table[in_file_i].eos == STREAM_PAST_EOS &&
        file_table[in_file_i].eof_action == STREAM_EOF_ACTION_ERROR) {
        bp_exception = c_permission_error(et_INPUT, et_PAST_END_OF_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }
    if (file_table[in_file_i].type != STREAM_TYPE_TEXT) {
        bp_exception = c_permission_error(et_INPUT, et_BINARY_STREAM, c_stream_struct(in_file_i));
        return BP_ERROR;
    }

    n = getc(curr_in);

    if (n == EOF) {
        clearerr(curr_in);
    } else if (n == 0) {
        bp_exception = c_representation_error(et_CHARACTER);
        return BP_ERROR;
    }

    if (n & 0x80) {  /* leading byte of a utf8 char? */
        char s[5], *ch_ptr;
        n = utf8_getc(curr_in, n);
        ch_ptr = utf8_codepoint_to_str(n, s);
        while (ch_ptr > s) {
            ch_ptr--;
            ungetc(*ch_ptr, curr_in);
        }
    } else {
        ungetc(n, curr_in);
    }
    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}


int c_UNGETC()
{
    BPLONG op;
    BPLONG_PTR top;

    op = ARG(1, 1);
    DEREF(op);
    lastc = INTVAL(op);
    return 1;
}

int b_GET_f(BPLONG op)
{
    BPLONG n;

    do {
        n = getc(curr_in);
    } while (n != EOF && (n <= 32 || n >= 127));
    if (n == EOF) {
        clearerr(curr_in);
    }

    ASSIGN_f_atom(op, MAKEINT(n));
    return 1;
}

int c_rm_file() {
    BPLONG op;

    op = ARG(1, 1);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);
    CHANGE_FILE_SEPARATOR(full_file_name);
    return (remove(full_file_name) == 0) ? BP_TRUE : BP_FALSE;
}

int c_cp_file() {
    char *f_name1, *f_name2;

    f_name1 = (char *)bp_get_atom_name(ARG(1, 2));
    f_name2 = (char *)bp_get_atom_name(ARG(2, 2));
    if (f_name1 == NULL || f_name2 == NULL) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }

#ifdef WIN32
    strcpy(bp_buf, "copy ");
#else
    strcpy(bp_buf, "cp ");
#endif
    strcat(bp_buf, f_name1);
    strcat(bp_buf, " ");
    strcat(bp_buf, f_name2);

    CHANGE_FILE_SEPARATOR(bp_buf);
    system(bp_buf);
    return BP_TRUE;
}


/* copy a file */
int c_CP_FILE_cc() {
    BPLONG FDIn, FDOut;
    FILE *in_fptr;
    FILE *out_fptr;
    BPLONG FDIndex, b;

    FDIn = ARG(1, 2);
    FDOut = ARG(2, 2);

    DEREF(FDIn); FDIndex = INTVAL(FDIn);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;
    DEREF(FDOut); FDIndex = INTVAL(FDOut);
    CHECK_FILE_INDEX(FDIndex);
    out_fptr = file_table[FDIndex].fdes;
    if (file_table[FDIndex].mode == 0) {
        bp_exception = output_stream_expected;
        return BP_ERROR;
    }
    b = getc(in_fptr);
    while (b != EOF) {
        putc(b, out_fptr);
        b = getc(in_fptr);
    }
    return BP_TRUE;
}

int b_SEE_c(BPLONG fop)
{
    register BPLONG_PTR top;
    BPLONG temp_in_file_i;

    DEREF(fop);
    file_table[in_file_i].lastc = lastc;
    file_table[in_file_i].line_no = curr_line_no;
    if (fop == user_word) {
        temp_in_file_i = 0;
    } else {
        temp_in_file_i = get_file_index(fop, READ_MODE);
    }
    if (temp_in_file_i < 0) {  /* not in table */
        get_file_name(fop);
#ifdef WIN32
        tempfile = fopen(full_file_name, "rb");
#else
        tempfile = fopen(full_file_name, "r");
#endif
        if (tempfile == NULL) {
            bp_exception = c_permission_error(et_OPEN, et_SOURCE_SINK, fop);
            return BP_ERROR;
        }
        temp_in_file_i = next_file_index();
        if (temp_in_file_i < 0) {
            bp_exception = out_of_range; return BP_ERROR;
        }
        in_file_i = temp_in_file_i;
        file_table[in_file_i].mode = READ_MODE;
        file_table[in_file_i].name_atom = fop;
        file_table[in_file_i].fdes = tempfile;
        file_table[in_file_i].type = STREAM_TYPE_TEXT;
        lastc = ' ';
        file_table[in_file_i].lastc = ' ';
        curr_line_no = 1;
        file_table[in_file_i].line_no = curr_line_no;
    } else {
        in_file_i = temp_in_file_i;  /* take it from table */

        lastc = file_table[in_file_i].lastc;
        curr_line_no = file_table[in_file_i].line_no;
    }
    curr_in = file_table[in_file_i].fdes;
    return 1;
}


int b_SEEN()
{
    if (in_file_i >= 3) {
        fclose(curr_in);
        release_file_index(in_file_i);
    }
    in_file_i = 0;  /* reset to user */
    curr_in = file_table[in_file_i].fdes;
    return 1;
}

int b_SEEING_f(BPLONG temp)
{
    if (in_file_i == 0) {
        ASSIGN_f_atom(temp, user_word);
    } else {
        ASSIGN_f_atom(temp, file_table[in_file_i].name_atom);
    }
    return 1;
}

/* fop: file name */
/* sop: 0 -> open `w'-write; 1 -> open `a'-append */
int b_TELL_cc(BPLONG fop, BPLONG mode)
{

    BPLONG_PTR top;
    BPLONG temp_out_file_i;

    DEREF(fop); DEREF(mode);

    file_table[out_file_i].line_no = out_line_no;
    file_table[out_file_i].line_position = line_position;

    if (fop == user_word) {
        temp_out_file_i = 1;
    } else if (fop == user_error_word) {
        temp_out_file_i = 2;
    } else {
        temp_out_file_i = get_file_index(fop, WRITE_MODE);
    }

    if (temp_out_file_i < 0) {  /* not in table */
        get_file_name(fop);
        if(INTVAL(mode) == 1)
#ifdef WIN32
            tempfile = fopen(full_file_name, "ab");
#else
        tempfile = fopen(full_file_name, "a");
#endif
        else
#ifdef WIN32
            tempfile = fopen(full_file_name, "wb");
#else
        tempfile = fopen(full_file_name, "w");
#endif
        if (tempfile == NULL) {
            bp_exception = c_permission_error(et_OPEN, et_SOURCE_SINK, fop);
            return BP_ERROR;
        }
        temp_out_file_i = next_file_index();
        if (temp_out_file_i < 0) {
            bp_exception = out_of_range; return BP_ERROR;
        }
        file_table[temp_out_file_i].mode = WRITE_MODE;
        file_table[temp_out_file_i].name_atom = fop;
        file_table[temp_out_file_i].fdes = tempfile;
        file_table[temp_out_file_i].type = STREAM_TYPE_TEXT;
        out_line_no = 0;
        line_position = 0;
    } else {
        out_line_no = file_table[temp_out_file_i].line_no;
        line_position = file_table[temp_out_file_i].line_position;
    }
    out_file_i = temp_out_file_i;
    curr_out = file_table[out_file_i].fdes;

    /*
      printf("out_file_i=%d\n",out_file_i);
      printf("stderr=%x, curr_out=%x file_table[2].fdes=%x\n",stderr,curr_out,file_table[2].fdes);
      if (curr_out==stderr) printf("stderr\n"); else printf("not stderr\n");
    */
    return 1;
}

int b_TOLD()
{
    if (out_file_i >= 3) {
        fclose(file_table[out_file_i].fdes);
        release_file_index(out_file_i);
    }
    out_file_i = 1;  /* reset to user */
    curr_out = file_table[out_file_i].fdes;
    return 1;
}

int b_TELLING_f(BPLONG temp)  /* arg1: unified with the current out put file name */
{
    if (out_file_i == 1) {
        ASSIGN_f_atom(temp, user_word);
    } else {
        ASSIGN_f_atom(temp, file_table[out_file_i].name_atom);
    }
    return 1;
}

/* fop: file name */
/* sop: 0 -> read; 1 -> write; 2 -> append*/
int b_OPEN_ccf(BPLONG fop, BPLONG sop, BPLONG Index)
{
    register BPLONG_PTR top;
    BPLONG index, mode;

    DEREF(sop);
    DEREF(fop);
    mode = INTVAL(sop);
    get_file_name(fop);
    if (mode == READ_MODE && sys_access(full_file_name, mode) != 0) {
        bp_exception = c_existence_error(et_SOURCE_SINK, fop);
        return BP_ERROR;
    }
    switch (mode) {
    case 0:
#ifdef WIN32
        tempfile = fopen(full_file_name, "rb");
#else
        tempfile = fopen(full_file_name, "r");
#endif
        break;
    case 1:
#ifdef WIN32
        tempfile = fopen(full_file_name, "wb");
#else
        tempfile = fopen(full_file_name, "w");
#endif
        break;
    case 2:
#ifdef WIN32
        tempfile = fopen(full_file_name, "ab");
#else
        tempfile = fopen(full_file_name, "a");
#endif
        break;
    default:
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    if (!tempfile) {
        bp_exception = c_permission_error(et_OPEN, et_SOURCE_SINK, fop);
        return BP_ERROR;
    }
    index = next_file_index();
    if (index < 0) {
        bp_exception = out_of_range; return BP_ERROR;
    }
    file_table[index].mode = mode;
    file_table[index].name_atom = fop;
    file_table[index].fdes = tempfile;
    file_table[index].aliases = nil_sym;
    file_table[index].eos = STREAM_NOT_EOS;
    file_table[index].type = STREAM_TYPE_TEXT;  /* default type */

    /**/
    if (mode == 0) {
        file_table[index].line_no = 1;
        file_table[index].lastc = ' ';
    } else {
        file_table[index].line_no = 0;
        file_table[index].line_position = 0;
    }
    ASSIGN_f_atom(Index, MAKEINT(index));
    return 1;
}

/* this function is by Steve Branch */
int get_socket_fd(int index) {
#ifdef WIN32
    printf("get_socket_fd not supported for non-Linux platforms\n");
    return 0;
#else
#ifdef CYGWIN
    printf("get_socket_fd not supported for non-Linux platforms\n");
    return 0;
#else
#ifdef DARWIN
    printf("get_socket_fd not supported for non-Linux platforms\n");
    return 0;
#else
#ifdef ANDROID
    return(file_table[index].fdes->_file);
#else
#ifdef __EMSCRIPTEN__
    printf("get_socket_fd not supported for wasm platforms\n");
    return 0;
#else
    return(file_table[index].fdes->_fileno);
#endif
#endif
#endif
#endif
#endif
}

/* this function is by Steve Branch */
int allocate_socket_file(FILE *file, char *name)
{
    int index;

    index = next_file_index();

    if (index > 0)
    {
        file_table[index].mode = SOCKET;
        file_table[index].name_atom = ADDTAG(insert_sym(name, strlen(name), 0), ATM);
        file_table[index].fdes = file;
        file_table[index].line_no = 0;
        file_table[index].line_position = 0;
        file_table[index].type = STREAM_TYPE_TEXT;
        file_table[index].eos = STREAM_NOT_EOS;
        file_table[index].eof_action = STREAM_EOF_ACTION_RESET;
    }

    return(index);
}

int b_CLOSE_c(BPLONG Index)
{
    BPLONG_PTR top;
    BPLONG i;

    DEREF(Index);
    i = INTVAL(Index);
    CHECK_FILE_INDEX(i);
    if (i >= 3) {  /* not user, stderr */
        if (i >= MAXFILES || file_table[i].fdes == NULL) {
            return BP_FALSE;
        }
        fclose(file_table[i].fdes);
        release_file_index(i);
        if (in_file_i == i) {
            in_file_i = 0;  /* reset to user */
            curr_in = file_table[in_file_i].fdes;
        } else if (out_file_i == i) {
            out_file_i = 1;  /* reset to user */
            curr_out = file_table[out_file_i].fdes;
        }
        return 1;
    }
    return 1;
}

void file_init() {
    BPLONG i;
    BPLONG_PTR ptr;

    user_sym_ptr = BP_NEW_SYM("user", 0);
    user_word = ADDTAG(user_sym_ptr, ATM);

    user_input_sym_ptr = BP_NEW_SYM("user_input", 0);
    user_input_word = ADDTAG(user_input_sym_ptr, ATM);

    user_output_sym_ptr = BP_NEW_SYM("user_output", 0);
    user_output_word = ADDTAG(user_output_sym_ptr, ATM);

    user_error_sym_ptr = BP_NEW_SYM("user_error", 0);
    user_error_word = ADDTAG(user_error_sym_ptr, ATM);

    file_table[0].mode = READ_MODE;
    file_table[0].name_atom = user_input_word;
    file_table[0].fdes = stdin;
    file_table[0].lastc = ' ';
    file_table[0].line_no = 1;
    file_table[0].type = STREAM_TYPE_TEXT;
    file_table[0].eos = STREAM_NOT_EOS;
    file_table[0].eof_action = STREAM_EOF_ACTION_RESET;

    ALLOCATE_FROM_PAREA(ptr, 2);  /* ptr==NULL not needed here */
    FOLLOW(ptr) = user_input_word;
    FOLLOW(ptr+1) = nil_sym;
    file_table[0].aliases = ADDTAG(ptr, LST);

    in_file_i = 0;
    curr_in = stdin;

    file_table[1].mode = APPEND_MODE;
    file_table[1].name_atom = user_output_word;
    file_table[1].fdes = stdout;
    file_table[1].line_no = 0;
    file_table[1].line_position = 0;
    file_table[1].type = STREAM_TYPE_TEXT;
    file_table[1].eos = STREAM_NOT_EOS;
    file_table[1].eof_action = STREAM_EOF_ACTION_RESET;


    ALLOCATE_FROM_PAREA(ptr, 2);  /* ptr==NULL not needed here */
    FOLLOW(ptr) = user_output_word;
    FOLLOW(ptr+1) = nil_sym;
    file_table[1].aliases = ADDTAG(ptr, LST);

    file_table[2].mode = APPEND_MODE;
    file_table[2].name_atom = user_error_word;
    file_table[2].fdes = stderr;
    file_table[2].line_no = 0;
    file_table[2].line_position = 0;
    file_table[2].type = STREAM_TYPE_TEXT;
    file_table[2].eos = STREAM_NOT_EOS;
    file_table[2].eof_action = STREAM_EOF_ACTION_RESET;


    ALLOCATE_FROM_PAREA(ptr, 2);  /* ptr==NULL not needed here */
    FOLLOW(ptr) = user_error_word;
    FOLLOW(ptr+1) = nil_sym;
    file_table[2].aliases = ADDTAG(ptr, LST);

    out_file_i = 1;
    curr_out = stdout;

    file_tab_end = 2;

    for (i = 3; i < MAXFILES; i++) {
        file_table[i].fdes = NULL;
        file_table[i].aliases = nil_sym;
    }
}

int b_ACCESS_ccf(BPLONG op1, BPLONG op2, BPLONG op3)
{
    BPLONG mode, r;
    register BPLONG_PTR top;

    DEREF(op1); DEREF(op2);

    if (check_file_term(op1) != BP_TRUE) return BP_ERROR;
    get_file_name(op1);
    if (ISINT(op2)) {
        mode = INTVAL(op2);
    } else {
        bp_exception = c_type_error(et_INTEGER, op2); return BP_ERROR;
    };
    if (socket_file_name(full_file_name))
    {
        r = 0;
    }
    else
    {
        r = sys_access(full_file_name, mode);
    }
    ASSIGN_f_atom(op3, MAKEINT(r));
    return 1;
}

/* this function is by Steve Branch */
int socket_file_name(const char* filename)
{
    unsigned int address_node = 0;
    int address_byte = 0;
    int filename_len = strlen(filename);
    int i, j;

    for (i = 0; i < filename_len; i++)
    {
        j = filename[i] - '0';
        if (j >= 0 && j <= 9)
        {
            address_node = address_node * 10 + j;
        }
        else if (filename[i] == '.')
        {
            address_byte++;
            if (address_node > 255 || address_byte > 3)
            {
                return(0);
            }
            address_node = 0;
        }
        else
        {
            return(0);
        }
    }
    return(address_node <= 255 && address_byte == 3);
}

/*
  dev_t     st_dev     ID of device containing file
  ino_t     st_ino     file serial number
  mode_t    st_mode    mode of file (see below)
  nlink_t   st_nlink   number of links to the file
  uid_t     st_uid     user ID of file
  gid_t     st_gid     group ID of file
  dev_t     st_rdev    device ID (if file is character or block special)
  off_t     st_size    file size in bytes (if file is a regular file)
  time_t    st_atime   time of last access
  time_t    st_mtime   time of last data modification
  time_t    st_ctime   time of last status change
*/

#define STAT_SIZE 12
int file_stat()
{
    struct stat buf;
    BPLONG bp_stat;
    BPLONG_PTR ptr;
    BPLONG op;

    op = ARG(1, 2);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);

    if (stat(full_file_name, &buf) == -1L) {
        bp_exception = file_does_not_exist;
        return -1L;
    }
    ptr = heap_top;
    heap_top += STAT_SIZE;
    bp_stat = ADDTAG(ptr, STR);
    FOLLOW(ptr) = (BPLONG)BP_NEW_SYM("stat", STAT_SIZE-1);
    FOLLOW(ptr+1) = MAKEINT(buf.st_dev);
    FOLLOW(ptr+2) = MAKEINT(buf.st_ino);
    FOLLOW(ptr+3) = MAKEINT(buf.st_mode);
    FOLLOW(ptr+4) = MAKEINT(buf.st_nlink);
    FOLLOW(ptr+5) = MAKEINT(buf.st_uid);
    FOLLOW(ptr+6) = MAKEINT(buf.st_gid);
    FOLLOW(ptr+7) = MAKEINT(buf.st_rdev);
    FOLLOW(ptr+8) = MAKEINT(buf.st_size);
    FOLLOW(ptr+9) = time_t_2_prolog(&buf.st_atime);
    FOLLOW(ptr+10) = time_t_2_prolog(&buf.st_mtime);
    FOLLOW(ptr+11) = time_t_2_prolog(&buf.st_ctime);
    return unify(ARG(2, 2), bp_stat);
}

int c_file_permission()
{
#ifdef WIN32
    struct stat buf;
#endif
    BPLONG op;
    int perm = 0;

    op = ARG(1, 2);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);


#ifdef WIN32
    if (stat(full_file_name, &buf) == -1L) {
        bp_exception = file_does_not_exist;
        return -1L;
    }

    if (buf.st_mode & _S_IREAD) {
        perm = (perm | 0x4);
    }
    if (buf.st_mode & _S_IWRITE) {
        perm = (perm | 0x2);
    }
    if (buf.st_mode & _S_IEXEC) {
        perm = (perm | 0x1);
    }
#else
    if (sys_access(full_file_name, F_OK) < 0) {
        bp_exception = file_does_not_exist;
        return -1L;
    }
    if (sys_access(full_file_name, R_OK) == 0) {
        perm = (perm | 0x4);
    }
    if (sys_access(full_file_name, W_OK) == 0) {
        perm = (perm | 0x2);
    }
    if (sys_access(full_file_name, X_OK) == 0) {
        perm = (perm | 0x1);
    }
#endif
    return unify(ARG(2, 2), MAKEINT(perm));
}

int c_file_type() {
    struct stat buf;
    char *t;
    BPLONG op;

    op = ARG(1, 2);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);

    if (stat(full_file_name, &buf) == -1L) {
        bp_exception = file_does_not_exist;
        return -1L;
    }
    if (S_ISREG(buf.st_mode)) {
        t = "regular";
    } else if (S_ISDIR(buf.st_mode)) {
        t = "directory";
    } else if (S_ISFIFO(buf.st_mode)) {
        t = "fifo";
    } else if (S_ISLNK(buf.st_mode)) {
        t = "symbolic_link";
    } else if (S_ISSOCK(buf.st_mode)) {
        t = "socket";
    } else {
        t = "unknown";
    }
    return unify(ARG(2, 2), ADDTAG((BPLONG)BP_NEW_SYM(t, 0), ATM));
}

int c_directory_list() {
    BPLONG Name, List;

    Name = ARG(1, 2);
    List = ARG(2, 2);
    if (check_file_term(Name) != BP_TRUE) return BP_ERROR;
    get_file_name(Name);

#ifdef PICAT
    return c_directory_list_picat(List);
#else
    return c_directory_list_bp(List);
#endif
}

int c_directory_list_picat(BPLONG List) {
    struct stat buf;
    BPLONG lst;
    BPLONG_PTR lst_ptr;

    if (stat(full_file_name, &buf) == -1L) {
        bp_exception = file_does_not_exist;
        return BP_ERROR;
    }
    if (!S_ISDIR(buf.st_mode)) {
        return unify(List, nil_sym);
    }

#ifdef WIN32
    {WIN32_FIND_DATA found_file;
        HANDLE hFind = INVALID_HANDLE_VALUE;

        strcat(full_file_name, "\\*");
        if ((hFind = FindFirstFile(full_file_name, &found_file)) == INVALID_HANDLE_VALUE) {
            return unify(List, nil_sym);
        }
        lst = ADDTAG(heap_top, LST);
        while (1) {
            lst_ptr = heap_top;
            heap_top += 2;

            LOCAL_OVERFLOW_CHECK("file_list");
            FOLLOW(lst_ptr) = c_str_to_picat_str0(found_file.cFileName);
            if (FindNextFile(hFind, &found_file) == 0) {
                FOLLOW(lst_ptr+1) = nil_sym; break;
            }
            FOLLOW(lst_ptr+1) = ADDTAG(heap_top, LST);
        }
        FindClose( hFind );
        return unify(List, lst);
    }
#else
    {DIR *bp_dir;
        struct dirent *bp_dir_ptr;

        if ((bp_dir = opendir(full_file_name)) == NULL) {
            return unify(List, nil_sym);
        }
        bp_dir_ptr = readdir(bp_dir);
        if (bp_dir_ptr == NULL) return unify(List, nil_sym);
        lst = ADDTAG(heap_top, LST);
        while (1) {
            lst_ptr = heap_top;
            heap_top += 2;

            LOCAL_OVERFLOW_CHECK("file_list");

            FOLLOW(lst_ptr) = c_str_to_picat_str0(bp_dir_ptr->d_name);
            bp_dir_ptr = readdir(bp_dir);
            if (bp_dir_ptr == NULL) {
                FOLLOW(lst_ptr+1) = nil_sym; break;
            }
            FOLLOW(lst_ptr+1) = ADDTAG(heap_top, LST);
        }
        closedir(bp_dir);
        return unify(List, lst);
    }
#endif
}

int c_directory_list_bp(BPLONG List) {
    struct stat buf;
    BPLONG lst;
    BPLONG_PTR lst_ptr;

    if (stat(full_file_name, &buf) == -1L) {
        bp_exception = file_does_not_exist;
        return BP_ERROR;
    }
    if (!S_ISDIR(buf.st_mode)) {
        return unify(List, nil_sym);
    }

#ifdef WIN32
    {struct _finddata_t found_file;
        long hFind;

        strcat(full_file_name, "/*");
        if ((hFind = _findfirst(full_file_name, &found_file)) == -1L) {
            return unify(List, nil_sym);
        }

        lst = ADDTAG(heap_top, LST);
        while (1) {
            lst_ptr = heap_top;
            heap_top += 2;

            LOCAL_OVERFLOW_CHECK("file_list");

            FOLLOW(lst_ptr) = ADDTAG(BP_NEW_SYM(found_file.name, 0), ATM);
            if (_findnext( hFind, &found_file) != 0) {
                FOLLOW(lst_ptr+1) = nil_sym; break;
            }
            FOLLOW(lst_ptr+1) = ADDTAG(heap_top, LST);
        }
        _findclose( hFind );
        return unify(List, lst);
    }
#else
    {DIR *bp_dir;
        struct dirent *bp_dir_ptr;

        if ((bp_dir = opendir(full_file_name)) == NULL) {
            return unify(List, nil_sym);
        }
        bp_dir_ptr = readdir(bp_dir);
        if (bp_dir_ptr == NULL) return unify(List, nil_sym);
        lst = ADDTAG(heap_top, LST);
        while (1) {
            lst_ptr = heap_top;
            heap_top += 2;

            LOCAL_OVERFLOW_CHECK("file_list");

            FOLLOW(lst_ptr) = ADDTAG(BP_NEW_SYM(bp_dir_ptr->d_name, 0), ATM);
            bp_dir_ptr = readdir(bp_dir);
            if (bp_dir_ptr == NULL) {
                FOLLOW(lst_ptr+1) = nil_sym; break;
            }
            FOLLOW(lst_ptr+1) = ADDTAG(heap_top, LST);
        }
        closedir(bp_dir);
        return unify(List, lst);
    }
#endif
}

int c_mkdir()
{
    int res;
    BPLONG op;

    op = ARG(1, 1);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);

#ifdef WIN32
    res = _mkdir(full_file_name);
#else
    res = mkdir(full_file_name, 0777);
#endif
    if (res == 0) {
        return BP_TRUE;
    } else {
        bp_exception = permission_error;
        return BP_ERROR;
    }
}

int c_rmdir()
{
    int res;
    BPLONG op;

    op = ARG(1, 1);
    if (check_file_term(op) != BP_TRUE) return BP_ERROR;
    get_file_name(op);


#ifdef WIN32
    res = _rmdir(full_file_name);
#else
    res = rmdir(full_file_name);
#endif
    if (res == 0) {
        return BP_TRUE;
    } else {
        bp_exception = permission_error;
        return BP_ERROR;
    }
}

int c_rename() {
    BPLONG res, op1, op2;
    char file_name1[MAX_STR_LEN], file_name2[MAX_STR_LEN];

    op1 = ARG(1, 2);
    if (check_file_term(op1) != BP_TRUE) return BP_ERROR;
    get_file_name(op1);
    strcpy(file_name1, full_file_name);
    op2 = ARG(2, 2);
    if (check_file_term(op2) != BP_TRUE) return BP_ERROR;
    get_file_name(op2);
    strcpy(file_name2, full_file_name);

    //  printf("%s %s\n",file_name1,file_name2);

    res = rename(file_name1, file_name2);
    if (res == 0) {
        return BP_TRUE;
    }
    bp_exception = permission_error;
    return res;
}

int c_chdir() {
    BPLONG dir = ARG(1, 1);
    BPLONG_PTR top;

    DEREF(dir);
    if (check_file_term(dir) != BP_TRUE) return BP_ERROR;
    get_file_name(dir);
    if (chdir(full_file_name) != 0) return BP_FALSE;
    return BP_TRUE;
}

int c_get_cwd() {
    BPLONG dir = ARG(1, 1);
    BPLONG_PTR top;
    char f_name[MAX_STR_LEN];

    DEREF(dir);
    getcwd(f_name, MAX_STR_LEN-1);
    return unify(dir, ADDTAG(BP_NEW_SYM(f_name, 0), ATM));
}

int c_write_term() {
    BPLONG op;

    op = ARG(1, 1);
    write_term(op);
    return 1;
}

void write_term1(BPLONG op, FILE *fp)
{
    FILE *tmp;
    tmp = curr_out;
    curr_out = fp;
    write_term(op);
    curr_out = tmp;
}

void write_list(BPLONG op)
{
    BPLONG_PTR top, list_ptr;

lab_start:
    list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
    op = FOLLOW(list_ptr);
    write_term(op);
    op = FOLLOW(list_ptr+1);
    DEREF(op);
    if (ISLIST(op)) {
        fprintf(curr_out, ",");
        goto lab_start;
    } else if (ISREF(op) || IsNumberedVar(op)) {
        fprintf(curr_out, "|");
        write_term(op);
        fprintf(curr_out, "]");
        fflush(curr_out);
    } else{
        fprintf(curr_out, "]");
        fflush(curr_out);
    }
}

int write_term(BPLONG op)
{
    SYM_REC_PTR sym_ptr;
    BPLONG op1;
    BPLONG_PTR dv_ptr;
    BPLONG i, arity;
    BPLONG_PTR top;

    SWITCH_OP(op, write_term_1,
              {fprintf(curr_out, "_" BPULONG_FMT_STR, op-(BPULONG)stack_low_addr);},

              {if (ISATOM(op)) {
                      sym_ptr = GET_ATM_SYM_REC(op);
                      bp_write_pname(GET_NAME(sym_ptr));
                  }
                  else fprintf(curr_out, BPLONG_FMT_STR, INTVAL(op));},

              {if (IsNumberedVar(op)) {fprintf(curr_out, "$V(" BPULONG_FMT_STR ")", INTVAL(op));} else { fprintf(curr_out, "["); write_list(op);}},

              {if (IS_FLOAT_PSC(op)) {
                      bp_write_double(op);
                  } else if (IS_BIGINT_PSC(op)) {
                      return bp_write_bigint(op);
                  } else if (cg_is_component(op)) {
                      cg_print_component(op);
                  } else {
                      sym_ptr = GET_STR_SYM_REC(op);
                      bp_write_pname(GET_NAME(sym_ptr));
                      if (GET_ARITY(sym_ptr) > 0) {
                          fprintf(curr_out, "(");
                          UNTAG_ADDR(op);
                          arity = GET_ARITY(sym_ptr);
                          for (i = 1; i <= arity; i++) {
                              op1 = *((BPLONG_PTR)op+i);
                              DEREF(op1);
                              write_term(op1);
                              if (i < arity) {
                                  fprintf(curr_out, ",");
                              }
                          }
                          fprintf(curr_out, ")");
                      }}},

              {dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(op);
                  fprintf(curr_out, "_0" BPULONG_FMT_STR, (BPULONG)dv_ptr-(BPULONG)stack_low_addr);
                  if (!IS_UN_DOMAIN(dv_ptr))
                      print_domain(dv_ptr);});
    return BP_TRUE;
}


int b_WRITE_IMAGE_c(BPLONG op)
{
    return write_image(op);
}

int write_image(BPLONG op)
{
    SYM_REC_PTR sym_ptr;
    BPLONG op1;
    BPLONG i, arity;
    BPLONG_PTR top;

    DEREF(op);
    switch (TAG(op)) {
    case REF :
        fprintf(curr_out, "var      " BPULONG_FMT_STR "\n", op);
        break;
    case LST :
        fprintf(curr_out, "list     " BPULONG_FMT_STR "\n", UNTAGGED_ADDR(op));
        UNTAG_ADDR(op);
        op1 = *(BPLONG_PTR)op;
        DEREF(op1);
        fprintf(curr_out, "car      " BPULONG_FMT_STR "\n", op1);
        op1 = *((BPLONG_PTR)op+1);
        DEREF(op1);
        fprintf(curr_out, "cdr      " BPULONG_FMT_STR "\n", op1);
        op1 = *(BPLONG_PTR)op;
        DEREF(op1);
        write_image(op1);
        op1 = *((BPLONG_PTR)op+1);
        DEREF(op1);
        write_image(op1);
        break;
    case ATM :
        if (ISATOM(op))
            fprintf(curr_out, "atom     " BPULONG_FMT_STR "\n", UNTAGGED_ADDR(op));
        else
            fprintf(curr_out, "int      " BPLONG_FMT_STR "\n", INTVAL(op));
        break;
    case STR:
        fprintf(curr_out, "str      " BPULONG_FMT_STR "\n", UNTAGGED_ADDR(op));
        sym_ptr = GET_STR_SYM_REC(op);
        UNTAG_ADDR(op);
        arity = GET_ARITY(sym_ptr);
        for (i = 1; i <= arity; i++) {
            op1 = *((BPLONG_PTR)op+i);
            DEREF(op1);
            fprintf(curr_out, BPLONG_FMT_STR "-arg    " BPULONG_FMT_STR "\n", i, op1);
        }
        for (i = 1; i <= arity; i++) {
            op1 = *((BPLONG_PTR)op+i);
            DEREF(op1);
            write_image(op1);
        }
        break;
    }
    return 1;
}


/**************** for ISO Prolog *********************/
int b_CURRENT_INPUT_f(BPLONG Index)
{

    ASSIGN_f_atom(Index, MAKEINT(in_file_i));
    return 1;
}

int b_CURRENT_OUTPUT_f(BPLONG Index)
{
    ASSIGN_f_atom(Index, MAKEINT(out_file_i));
    return 1;
}

int b_SET_BINARY_INPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_in_file_i;

    DEREF(Index);
    temp_in_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_in_file_i);

    if (file_table[temp_in_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_in_file_i].mode != READ_MODE) {
        bp_exception = c_permission_error(et_INPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_in_file_i].type != STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_INPUT, et_TEXT_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_input(temp_in_file_i);
}

int b_SET_TEXT_INPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_in_file_i;
    DEREF(Index);

    temp_in_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_in_file_i);
    if (file_table[temp_in_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_in_file_i].mode != READ_MODE && file_table[temp_in_file_i].mode != SOCKET) {
        bp_exception = c_permission_error(et_INPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_in_file_i].type != STREAM_TYPE_TEXT) {
        bp_exception = c_permission_error(et_INPUT, et_BINARY_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_input(temp_in_file_i);
}

int b_SET_INPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_in_file_i;
    DEREF(Index);
    temp_in_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_in_file_i);
    if (file_table[temp_in_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_in_file_i].mode != READ_MODE && file_table[temp_in_file_i].mode != SOCKET) {
        bp_exception = c_permission_error(et_INPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_input(temp_in_file_i);
}

int cc_set_input(BPLONG temp_in_file_i)
{
    file_table[in_file_i].lastc = lastc;
    file_table[in_file_i].line_no = curr_line_no;

    curr_in = file_table[temp_in_file_i].fdes;
    lastc = file_table[temp_in_file_i].lastc;
    curr_line_no = file_table[temp_in_file_i].line_no;
    in_file_i = temp_in_file_i;
    return 1;
}

int b_SET_BINARY_OUTPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_out_file_i;

    DEREF(Index);

    temp_out_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_out_file_i);
    if (file_table[temp_out_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_out_file_i].mode == READ_MODE) {
        bp_exception = c_permission_error(et_OUTPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_out_file_i].type != STREAM_TYPE_BINARY) {
        bp_exception = c_permission_error(et_OUTPUT, et_TEXT_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_output(temp_out_file_i);
}

int b_SET_TEXT_OUTPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_out_file_i;

    DEREF(Index);

    temp_out_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_out_file_i);
    if (file_table[temp_out_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_out_file_i].mode == READ_MODE) {
        bp_exception = c_permission_error(et_OUTPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_out_file_i].type != STREAM_TYPE_TEXT) {
        bp_exception = c_permission_error(et_OUTPUT, et_BINARY_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_output(temp_out_file_i);
}

int b_SET_OUTPUT_cc(BPLONG Index, BPLONG Source)
{
    BPLONG_PTR top;
    BPLONG temp_out_file_i;

    DEREF(Index);

    temp_out_file_i = INTVAL(Index);
    CHECK_FILE_INDEX(temp_out_file_i);
    if (file_table[temp_out_file_i].fdes == NULL) {
        bp_exception = c_existence_error(et_STREAM, Source);
        return BP_ERROR;
    }
    if (file_table[temp_out_file_i].mode == READ_MODE) {
        bp_exception = c_permission_error(et_OUTPUT, et_STREAM, Source);
        return BP_ERROR;
    }
    return cc_set_output(temp_out_file_i);
}

int cc_set_output(BPLONG temp_out_file_i)
{
    file_table[out_file_i].line_no = out_line_no;
    file_table[out_file_i].line_position = line_position;

    curr_out = file_table[temp_out_file_i].fdes;
    out_line_no = file_table[temp_out_file_i].line_no;
    line_position = file_table[temp_out_file_i].line_position;
    out_file_i = temp_out_file_i;
    return BP_TRUE;
}


int b_FLUSH_OUTPUT() {
    file_table[out_file_i].line_no = out_line_no;
    file_table[out_file_i].line_position = line_position;
    fflush(curr_out);
    return 1;
}

/********* functions called by asm.pl *********/
#ifdef M64BITS
int b_ASPN_i(BPLONG op1)
{
    int op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    op = (int)INTVAL(op1);
    op = MAKEINT32(op);
    EMIT4(op);
    return 1;
}

int c_ASPN_i()
{
    BPLONG op1;
    int op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    op1 = ARG(1, 1);
    DEREF(op1);
    op = (int)INTVAL(op1);
    op = MAKEINT32(op);
    EMIT4(op);
    return 1;
}
#else
int b_ASPN_i(BPLONG op1)
{
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    EMIT4(op1);
    return 1;
}

int c_ASPN_i()
{
    BPLONG op1;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    op1 = ARG(1, 1);
    DEREF(op1);
    EMIT4(op1);
    return 1;
}
#endif

int b_ASPN_c(BPLONG op1)
{
    BPLONG op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    op = INTVAL(op1);
    EMIT4(op);
    return 1;
}

int b_ASPN2_cc(BPLONG op1, BPLONG op2)
{
    BPLONG op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    op = INTVAL(op1);
    EMIT4(op);
    DEREF(op2);
    op = INTVAL(op2);
    EMIT4(op);
    return 1;
}

int b_ASPN3_ccc(BPLONG op1, BPLONG op2, BPLONG op3)
{
    BPLONG op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    op = INTVAL(op1);
    EMIT4(op);
    DEREF(op2);
    op = INTVAL(op2);
    EMIT4(op);
    DEREF(op3);
    op = INTVAL(op3);
    EMIT4(op);
    return 1;
}

int b_ASPN4_cccc(BPLONG op1, BPLONG op2, BPLONG op3, BPLONG op4)
{
    BPLONG op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    DEREF(op1);
    op = INTVAL(op1);
    EMIT4(op);
    DEREF(op2);
    op = INTVAL(op2);
    EMIT4(op);
    DEREF(op3);
    op = INTVAL(op3);
    EMIT4(op);
    DEREF(op4);
    op = INTVAL(op4);
    EMIT4(op);
    return 1;
}

int aspn() {
    BPLONG op;
    BYTE b1, b2, b3, b4;
    BPLONG_PTR top;

    op = ARG(1, 1);
    DEREF(op);
    op = INTVAL(op);
    EMIT4(op);
    return 1;
}

int b_GET_LINE_NO_cf(BPLONG Index, BPLONG no)
{
    BPLONG_PTR top;

    DEREF(Index); Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    file_table[in_file_i].line_no = curr_line_no;
    ASSIGN_f_atom(no, MAKEINT(file_table[Index].line_no));
    return 1;
}

int b_GET_LINE_POS_cf(BPLONG Index, BPLONG pos)
{
    BPLONG_PTR top;

    DEREF(Index); Index = INTVAL(Index);
    CHECK_FILE_INDEX(Index);
    file_table[out_file_i].line_position = line_position;
    ASSIGN_f_atom(pos, MAKEINT(file_table[Index].line_position));
    return 1;
}

int c_FORMAT_PRINT_INTEGER() {
    BPLONG control, arg, number, len;
    BPLONG_PTR top;
    char format[20];

    control = ARG(1, 3); DEREF(control); control = INTVAL(control);
    arg = ARG(2, 3); DEREF(arg);
    number = ARG(3, 3); DEREF(number); number = INTVAL(number);

    arg = eval_arith(arg);
    if (ISINT(arg)) {
        arg = INTVAL(arg);
    } else {  /* double (*/
        arg = (BPLONG)floatval(arg);
    }
    format[0] = '%';
    sprintf(&format[1], "%d%c", (int)number, (int) control);
    sprintf(bp_buf, format, arg);
    len = strlen(bp_buf);
    line_position += len;

    if (format_output_dest == 0) {
        fprintf(curr_out, format, arg);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    return BP_TRUE;
}

int c_FORMAT_PRINT_FLOAT() {
    BPLONG control, arg, number, len;
    BPLONG_PTR top;
    double val;
    char format[20];

    control = ARG(1, 3); DEREF(control); control = INTVAL(control);
    arg = ARG(2, 3); DEREF(arg);
    number = ARG(3, 3); DEREF(number); number = INTVAL(number);

    arg = eval_arith(arg);
    if (ISINT(arg)) {
        val = (double)INTVAL(arg);
    } else {  /* double (*/
        val = floatval(arg);
    }
    format[0] = '%';
    format[1] = '.';
    sprintf(&format[2], "%d%c", (int)number, (int) control);
    sprintf(bp_buf, format, val);
    len = strlen(bp_buf);
    line_position += len;

    if (format_output_dest == 0) {
        fprintf(curr_out, format, val);
    } else {
        CHECK_CHARS_POOL_OVERFLOW(len);
        strcpy((chars_pool+chars_pool_index), bp_buf);
        chars_pool_index += len;
    }
    return BP_TRUE;
}

char *format_comma_separated_int(BPLONG amt) {
    char loc_buf[100];
    int i, j, c;

    sprintf(loc_buf, BPLONG_FMT_STR, amt);
    i = strlen(loc_buf)-1;
    j = MAX_STR_LEN-1;
    c = 0;
    bp_buf[j--] = '\0';
    while (i >= 0) {
        bp_buf[j--] = loc_buf[i--];
        c++;
        if (c%3 == 0) bp_buf[j--] = ',';
    }
    j++;
    if (bp_buf[j] == ',') j++;
    return (bp_buf+j);
}


int b_NAME0_cf(BPLONG op1, BPLONG op2)  /* op2 is made to be the string of the name of op1*/
{
    SYM_REC_PTR psc_ptr;
    CHAR_PTR name;
    register BPLONG_PTR top;

    DEREF(op1);
    if (ISATOM(op1)) {
        psc_ptr = GET_ATM_SYM_REC(op1);
        name = GET_NAME(psc_ptr);
        return string2codes(name, op2);
    } else if (ISINT(op1)) {
        sprintf(bp_buf, "%d", (int)INTVAL(op1));
        return string2codes(bp_buf, op2);
    } else if (IS_BIGINT(op1)) {
        int i = bp_write_bigint_to_str(op1, bp_buf, MAX_STR_LEN);
        if (i == BP_ERROR) {
            bp_exception = et_OUT_OF_MEMORY;
            return BP_ERROR;
        }
        return string2codes((bp_buf+i), op2);
    } else if (ISFLOAT(op1)) {
        sprintf(bp_buf, "%.15lf", floatval(op1));
        bp_trim_trailing_zeros(bp_buf);
        return string2codes(bp_buf, op2);
    } else {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
}  /* b_NAME0 */

/* a1 and a2 must be atoms */
int b_ATOM_CONCAT_ccf(BPLONG a1, BPLONG a2, BPLONG a3) {
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;
    int len;

    DEREF(a1);
    sym_ptr = GET_ATM_SYM_REC(a1);
    len = GET_LENGTH(sym_ptr);
    sprintf(bp_buf, "%s", GET_NAME(sym_ptr));
    char_ptr = bp_buf+len;
    DEREF(a2);
    sym_ptr = GET_ATM_SYM_REC(a2);
    len += GET_LENGTH(sym_ptr);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    if (len >= MAX_STR_LEN) {
        bp_exception = ADDTAG(str_RESOURCE_ERROR, ATM);
        return BP_ERROR;
    }
    ASSIGN_f_atom(a3, ADDTAG(insert_sym(bp_buf, len, 0), ATM));
    return BP_TRUE;
}

/*
  matm = "e$$glb$$" ++ atm
*/
int c_module_glb_pred_name() {
    BPLONG atm, matm;
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;
    int len;
  
    atm = ARG(1, 2);
    matm = ARG(2, 2);
  
    bp_buf[0] = 'e';  bp_buf[1] = '$'; bp_buf[2] = '$';
    bp_buf[3] = 'g';  bp_buf[4] = 'l'; bp_buf[5] = 'b';
    bp_buf[6] = '$';  bp_buf[7] = '$'; 
  
    char_ptr = bp_buf+8;

    DEREF(atm);
    sym_ptr = GET_ATM_SYM_REC(atm);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;
    *char_ptr = '\0';
    len = char_ptr - bp_buf;

    unify(matm, ADDTAG(insert_sym(bp_buf, len, 0), ATM));
    return BP_TRUE;
}

/*
  matm = "e$$glb$$f$$" ++ atm
*/
int c_module_glb_func_name() {
    BPLONG atm, matm;
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;
    int len;

    atm = ARG(1, 2);
    matm = ARG(2, 2);
  
    bp_buf[0] = 'e';  bp_buf[1] = '$'; bp_buf[2] = '$';
    bp_buf[3] = 'g';  bp_buf[4] = 'l'; bp_buf[5] = 'b';
    bp_buf[6] = '$';  bp_buf[7] = '$';
    bp_buf[8] = 'f';  bp_buf[9] = '$';  bp_buf[10] = '$';   
  
    char_ptr = bp_buf+11;

    DEREF(atm);
    sym_ptr = GET_ATM_SYM_REC(atm);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;
    *char_ptr = '\0';
    len = char_ptr - bp_buf;

    unify(matm, ADDTAG(insert_sym(bp_buf, len, 0), ATM));
    return BP_TRUE;
}

/*
  matm = "e$$" ++ m ++ "$$" ++ atm
*/
int c_module_qualified_pred_name() {
    BPLONG m, atm, matm;
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;
    int len;

    m = ARG(1, 3);
    atm = ARG(2, 3);
    matm = ARG(3, 3);
    bp_buf[0] = 'e';  bp_buf[1] = '$'; bp_buf[2] = '$';
    char_ptr = bp_buf+3;

    DEREF(m);
    sym_ptr = GET_ATM_SYM_REC(m);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;

    char_ptr[0] = '$';
    char_ptr[1] = '$';
    char_ptr += 2;

    DEREF(atm);
    sym_ptr = GET_ATM_SYM_REC(atm);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;
    *char_ptr = '\0';
    len = char_ptr - bp_buf;

    unify(matm, ADDTAG(insert_sym(bp_buf, len, 0), ATM));
    return BP_TRUE;
}

/*
  matm = "e$$" ++ m ++ "$$f$$" ++ atm
*/
int c_module_qualified_func_name() {
    BPLONG m, atm, matm;
    SYM_REC_PTR sym_ptr;
    CHAR_PTR char_ptr;
    int len;

    m = ARG(1, 3);
    atm = ARG(2, 3);
    matm = ARG(3, 3);
    bp_buf[0] = 'e';  bp_buf[1] = '$'; bp_buf[2] = '$';
    char_ptr = bp_buf+3;

    DEREF(m);
    sym_ptr = GET_ATM_SYM_REC(m);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;

    char_ptr[0] = '$';  char_ptr[1] = '$';
    char_ptr[2] = 'f';  char_ptr[3] = '$';  char_ptr[4] = '$';  
    char_ptr += 5;

    DEREF(atm);
    sym_ptr = GET_ATM_SYM_REC(atm);
    sprintf(char_ptr, "%s", GET_NAME(sym_ptr));
    len = GET_LENGTH(sym_ptr);
    char_ptr += len;
    *char_ptr = '\0';
    len = char_ptr - bp_buf;

    unify(matm, ADDTAG(insert_sym(bp_buf, len, 0), ATM));
    return BP_TRUE;
}

/* for Picat */
int print_term_to_buf(BPLONG term) {
    DEREF(term);
    if (ISREF(term)) {
        sprintf(bp_buf, "_" BPULONG_FMT_STR, (BPULONG)term-(BPULONG)stack_low_addr);
    } else if (ISINT(term)) {
        sprintf(bp_buf, BPLONG_FMT_STR, INTVAL(term));
    } else if (ISATOM(term)) {
        SYM_REC_PTR sym_ptr;
        sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(term);
        sprintf(bp_buf, "%s", GET_NAME(sym_ptr));
    } else if (TAG(term) == STR) {
        if (IS_SUSP_VAR(term)) {
            sprintf(bp_buf, "_0" BPULONG_FMT_STR, (BPULONG)UNTAGGED_TOPON_ADDR(term)-(BPULONG)stack_low_addr);
        } else if (ISFLOAT(term)) {
            sprintf(bp_buf, "%.15lf", floatval(term));
            bp_trim_trailing_zeros(bp_buf);
        } else if (IS_BIGINT(term)) {
            int j, i = bp_write_bigint_to_str(term, bp_buf, MAX_STR_LEN);  /* stored in bp_buf from index i to MAX_STR_LEN-1 */
            if (i == BP_ERROR) return BP_ERROR;
            j = 0;
            while (i != MAX_STR_LEN) {  /* copy to the beginning of bp_buf */
                bp_buf[j] = bp_buf[i];
                i++; j++;
            }
        } else {
            return BP_FALSE;
        }
    } else {
        return BP_FALSE;
    }

    return BP_TRUE;
}

BPLONG c_str_to_picat_str0(CHAR_PTR str) {
    c_str_to_picat_str(str, (BPLONG)(local_top-1), (BPLONG)local_top);
    FOLLOW(FOLLOW(local_top)) = nil_sym;
    return FOLLOW(local_top-1);
}

void c_str_to_picat_str(CHAR_PTR str, BPLONG lst, BPLONG lstr) {
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;
    CHAR_PTR ch_ptr;

    ret_lst_ptr = &ret_lst;
    ch_ptr = str;

    if ((*ch_ptr) == '\0') {
        FOLLOW(heap_top) = (BPLONG)heap_top;
        ASSIGN_v_heap_term(lst, (BPLONG)heap_top);
        ASSIGN_v_heap_term(lstr, (BPLONG)heap_top);
        heap_top++;
        return;
    }
    while ((*ch_ptr) != '\0') {
        CHAR_PTR ch_ptr0 = ch_ptr;
        utf8_char_to_codepoint(&ch_ptr);
        FOLLOW(heap_top) = ADDTAG(insert_sym(ch_ptr0, (ch_ptr-ch_ptr0), 0), ATM);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top;
        heap_top++;
        LOCAL_OVERFLOW_CHECK("to_string");
    }
    FOLLOW(ret_lst_ptr) = (BPLONG)ret_lst_ptr;
    ASSIGN_v_heap_term(lst, ret_lst);
    ASSIGN_v_heap_term(lstr, (BPLONG)ret_lst_ptr);
}

/* change Picat string (lst) to a C string (buf) */
void picat_str_to_c_str(BPLONG lst, char *buf, BPLONG buf_size) {
    CHAR_PTR ch_ptr = buf;
    CHAR_PTR s;
    int j, len, i = 0;

    //  printf("=>picat_str_to_c_str "); write_term(lst); printf("\n");

    while (ISLIST(lst)) {
        BPLONG_PTR lst_ptr;
        BPLONG car;
        SYM_REC_PTR sym_ptr;

        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
        lst = FOLLOW(lst_ptr+1); DEREF(lst);
        car = FOLLOW(lst_ptr); DEREF(car);  /* car must be a single-char atom */
        sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(car);
        s = GET_NAME(sym_ptr);
        len = GET_LENGTH(sym_ptr);
        if (i+len >= buf_size) {
            quit("buff overfolow in picat_str_to_c_str");
        }
        for (j = 0; j < len; j++) {
            *(ch_ptr+i) = *(s+j);
            i++;
        }
    }
    *(ch_ptr+i) = '\0';
}

/* term must be one of the following: variable, atom, integer, and real */
int b_TO_STRING_cff(BPLONG term, BPLONG lst, BPLONG lstr) {
    if (print_term_to_buf(term) == BP_FALSE) return BP_FALSE;
    c_str_to_picat_str(bp_buf, lst, lstr);
    return BP_TRUE;
}

int b_TO_QUOTED_STRING_cff(BPLONG term, BPLONG lst, BPLONG lstr) {
    SYM_REC_PTR sym_ptr;

    DEREF(term);
    if (!ISATOM(term)) {
        return b_TO_STRING_cff(term, lst, lstr);
    }
    sym_ptr = GET_ATM_SYM_REC(term);
    bp_write_qname_to_bp_buf(GET_NAME(sym_ptr), GET_LENGTH(sym_ptr));
    c_str_to_picat_str(bp_buf, lst, lstr);
    return BP_TRUE;
}

int b_TO_CODES_cff(BPLONG term, BPLONG lst, BPLONG lstr) {
    BPLONG ret_lst, code;
    BPLONG_PTR ret_lst_ptr;
    char *ch_ptr;
    if (print_term_to_buf(term) == BP_ERROR) return BP_ERROR;

    ret_lst_ptr = &ret_lst;
    ch_ptr = bp_buf;

    while (*ch_ptr != '\0') {
        code = utf8_char_to_codepoint(&ch_ptr);
        FOLLOW(heap_top) = MAKEINT(code);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top;
        heap_top++;
        LOCAL_OVERFLOW_CHECK("to_string");
    }
    if (ret_lst_ptr == &ret_lst) {  /* for the atom '' */
        return unify(lst, lstr);  /* nothing is added to the difference list */
    } else {
        FOLLOW(ret_lst_ptr) = (BPLONG)ret_lst_ptr;
        ASSIGN_v_heap_term(lst, ret_lst);
        ASSIGN_v_heap_term(lstr, (BPLONG)ret_lst_ptr);
        return BP_TRUE;
    }
}

/* print a primitive into FD, fails if term is not primitive */
int b_PICAT_PRINT_PRIMITIVE_cc(BPLONG FDIndex, BPLONG term) {
    DEREF(FDIndex);
    out_file_i = INTVAL(FDIndex);
    //  CHECK_FILE_INDEX(out_file_i);
    curr_out = file_table[out_file_i].fdes;
    return b_WRITE_QUICK_c(term);
}

/* write a primitive into FD, fails if term is not primitive */
int b_PICAT_WRITE_PRIMITIVE_cc(BPLONG FDIndex, BPLONG term) {
    DEREF(FDIndex);
    out_file_i = INTVAL(FDIndex);
    //  CHECK_FILE_INDEX(out_file_i);
    curr_out = file_table[out_file_i].fdes;

    return b_WRITEQ_QUICK_c(term);
}

int c_PICAT_FORMAT_TO_STRING_ccff() {
    BPLONG Format, Val, Str, StrR;
    char *ch_ptr;

    char format_str[MAX_STR_LEN];

    Format = ARG(1, 4); DEREF(Format);
    Val = ARG(2, 4); DEREF(Val);
    Str = ARG(3, 4); DEREF(Str);
    StrR = ARG(4, 4); DEREF(StrR);

    picat_str_to_c_str(Format, format_str, MAX_STR_LEN);
    ch_ptr = bp_buf;
    if (ISINT(Val)) {
        sprintf(ch_ptr, format_str, INTVAL(Val));
    } else if (ISFLOAT(Val)) {
        sprintf(ch_ptr, format_str, floatval(Val));
    } else if (IS_BIGINT(Val)) {
        sprintf(ch_ptr, format_str, bp_bigint_to_int(Val));
    } else {  /* Val must be a Picat string */
        char *str = (CHAR_PTR)heap_top+10000;
        picat_str_to_c_str(Val, str, MAX_STR_LEN);
        sprintf(ch_ptr, format_str, str);
    }
    c_str_to_picat_str(ch_ptr, Str, StrR);
    return BP_TRUE;
}

int c_PICAT_GETENV_cf() {
    BPLONG EnvVarName, EnvValStr;
    CHAR_PTR env_val;

    EnvVarName = ARG(1, 2); DEREF(EnvVarName);
    EnvValStr = ARG(2, 2);
    if (!ISATOM(EnvVarName) && !b_IS_STRING_c(EnvVarName)) {
        bp_exception = illegal_arguments; return BP_ERROR;
    }
    get_file_name(EnvVarName);
    env_val = getenv(full_file_name);
    if (env_val == NULL) {
        return BP_FALSE;
    }
    FOLLOW(local_top) = (BPLONG)local_top;  /* borrow this cell to store the tail */
    c_str_to_picat_str(env_val, EnvValStr, (BPLONG)local_top);
    FOLLOW(FOLLOW(local_top)) = nil_sym;
    return BP_TRUE;
}

int c_PICAT_GET_CWD_f() {
    BPLONG dir = ARG(1, 1);
    BPLONG_PTR top;
    char f_name[MAX_STR_LEN];

    DEREF(dir);
    getcwd(f_name, MAX_STR_LEN-1);

    FOLLOW(local_top) = (BPLONG)local_top;  /* borrow this cell to store the tail */
    c_str_to_picat_str(f_name, dir, (BPLONG)local_top);
    FOLLOW(FOLLOW(local_top)) = nil_sym;
    return BP_TRUE;
}

int b_GET_NEXT_PICAT_TOKEN_cff(BPLONG FDIndex, BPLONG TokenType, BPLONG TokenVal) {
    FILE *in_fptr;
    CHAR tmp_lastc = ' ';
    int tmp_line_no, res;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
#ifdef BPSOLVER
    if (FDIndex != in_file_i) {
        curr_in = file_table[FDIndex].fdes;
        lastc = file_table[FDIndex].lastc;
    }
#else
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    if (FDIndex != in_file_i) {
        in_fptr = curr_in;
        tmp_lastc = lastc;
        tmp_line_no = curr_line_no;

        curr_in = file_table[FDIndex].fdes;
        lastc = file_table[FDIndex].lastc;
        curr_line_no = file_table[FDIndex].line_no;
    }
#endif

    res = b_NEXT_TOKEN_ff(TokenType, TokenVal);

    if (FDIndex != in_file_i) {
        file_table[FDIndex].lastc = lastc;
        file_table[FDIndex].line_no = curr_line_no;

        curr_in = in_fptr;
        lastc = tmp_lastc;
        curr_line_no = tmp_line_no;
    }
    return (res == BP_ERROR ? BP_ERROR : BP_TRUE);
}

/***********************************************/
/* read one byte */
int b_READ_BYTE_cf(BPLONG FDIndex, BPLONG Byt) {
    FILE *in_fptr;
    BPLONG b;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    file_table[FDIndex].lastc = ' ';

    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;
    b = getc(in_fptr);
    if (b == EOF) clearerr(in_fptr);
    ASSIGN_f_atom(Byt, MAKEINT(b));

    return BP_TRUE;
}


int b_PEEK_BYTE_cf(BPLONG FDIndex, BPLONG Byt) {
    FILE *in_fptr;
    BPLONG b;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;
    b = getc(in_fptr);
    if (b == EOF) {
        clearerr(in_fptr);
    } else {
        ungetc(b, in_fptr);
    }
    ASSIGN_f_atom(Byt, MAKEINT(b));
    return BP_TRUE;
}

/* read up to N (>0) bytes */
int b_READ_BYTE_ccf(BPLONG FDIndex, BPLONG N, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;

    DEREF(N); N = INTVAL(N);
    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF && N > 0) {
        FOLLOW(heap_top) = MAKEINT(b);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        LOCAL_OVERFLOW_CHECK("read_byte");
        b = getc(in_fptr);
        N--;
    }
    if (b == EOF) {
        clearerr(in_fptr);
    } else {
        ungetc((char)b, in_fptr);
    }
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/***********************************************/
int b_READ_CHAR_cf(BPLONG FDIndex, BPLONG Ch) {
    FILE *in_fptr;
    BPLONG b;


    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;
    file_table[FDIndex].lastc = ' ';

    b = getc(in_fptr);
    if (b == EOF) {
        clearerr(in_fptr);
        b = eof_atom;
    } else {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            *ch_ptr = '\0';
            b = ADDTAG(insert_sym(s, (ch_ptr-s), 0), ATM);
        } else {
            b = char_sym_table[(char)b];
        }
    }
    ASSIGN_f_atom(Ch, b);
    return BP_TRUE;
}

int b_PEEK_CHAR_cf(BPLONG FDIndex, BPLONG Ch) {
    FILE *in_fptr;
    BPLONG b;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;

    b = getc(in_fptr);
    if (b == EOF) {
        clearerr(in_fptr);
        b = eof_atom;
    } else {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            *ch_ptr = '\0';
            b = ADDTAG(insert_sym(s, (ch_ptr-s), 0), ATM);
            while (ch_ptr > s) {
                ch_ptr--;
                ungetc(*ch_ptr, in_fptr);
            }
        } else {
            ungetc((char)b, in_fptr);
            b = char_sym_table[(char)b];
        }
    }
    ASSIGN_f_atom(Ch, b);
    return BP_TRUE;
}

/* read up to N (>0) chars */
int b_READ_CHAR_ccf(BPLONG FDIndex, BPLONG N, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;
    DEREF(N); N = INTVAL(N);
    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF && N > 0) {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            *ch_ptr = '\0';
            b = ADDTAG(insert_sym(s, (ch_ptr-s), 0), ATM);
        } else {
            b = char_sym_table[(char)b];
        }
        FOLLOW(heap_top) = b;
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        LOCAL_OVERFLOW_CHECK("read_char");
        b = getc(in_fptr);
        N--;
    }
    if (b == EOF) {
        clearerr(in_fptr);
    } else {
        ungetc((char)b, in_fptr);
    }
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/***********************************************/
int b_READ_CHAR_CODE_cf(BPLONG FDIndex, BPLONG Ch) {
    FILE *in_fptr;
    BPLONG b;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);

    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;

    b = getc(in_fptr);
    if (b == EOF) {
        clearerr(in_fptr);
        b = -1;
    } else {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            b = utf8_getc(in_fptr, b);
        }
    }
    ASSIGN_f_atom(Ch, MAKEINT(b));
    return BP_TRUE;
}

int b_PEEK_CHAR_CODE_cf(BPLONG FDIndex, BPLONG Ch) {
    FILE *in_fptr;
    BPLONG b;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    in_fptr = file_table[FDIndex].fdes;

    b = getc(in_fptr);
    if (b == EOF) {
        clearerr(in_fptr);
        b = -1;
    } else {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            while (ch_ptr > s) {
                ch_ptr--;
                ungetc(*ch_ptr, in_fptr);
            }
        } else {
            ungetc((char)b, in_fptr);
        }
    }
    ASSIGN_f_atom(Ch, MAKEINT(b));
    return BP_TRUE;
}

/* read up to N (>0) chars */
int b_READ_CHAR_CODE_ccf(BPLONG FDIndex, BPLONG N, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;
    DEREF(N); N = INTVAL(N);
    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF && N > 0) {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            b = utf8_getc(in_fptr, b);
        }
        FOLLOW(heap_top) = MAKEINT(b);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        LOCAL_OVERFLOW_CHECK("read_char_code");
        b = getc(in_fptr);
        N--;
    }
    if (b == EOF) {
        clearerr(in_fptr);
    } else {
        ungetc((char)b, in_fptr);
    }
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/***********************************************/
int b_READ_FILE_BYTES_cf(BPLONG FDIndex, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    in_fptr = file_table[FDIndex].fdes;
    file_table[FDIndex].lastc = ' ';

    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF) {
        FOLLOW(heap_top) = MAKEINT(b);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        b = getc(in_fptr);
    }
    clearerr(in_fptr);
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/* read all the chars in a file into a list */
int b_READ_FILE_CHARS_cf(BPLONG FDIndex, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    in_fptr = file_table[FDIndex].fdes;
    file_table[FDIndex].lastc = ' ';

    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF) {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            *ch_ptr = '\0';
            b = ADDTAG(insert_sym(s, (ch_ptr-s), 0), ATM);
        } else {
            b = char_sym_table[(char)b];
        }
        FOLLOW(heap_top) = b;
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        b = getc(in_fptr);
    }
    clearerr(in_fptr);
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/* read all the char codes in a file into a list */
int b_READ_FILE_CODES_cf(BPLONG FDIndex, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;

    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF) {
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            b = utf8_getc(in_fptr, b);
        }
        FOLLOW(heap_top) = MAKEINT(b);
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        ret_lst_ptr = heap_top++;
        b = getc(in_fptr);
    }
    clearerr(in_fptr);
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/* read the next line into a list */
int b_READ_LINE_cf(BPLONG FDIndex, BPLONG Lst) {
    FILE *in_fptr;
    BPLONG b;
    BPLONG ret_lst;
    BPLONG_PTR ret_lst_ptr;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    if (file_table[FDIndex].mode != 0) {
        bp_exception = input_stream_expected;
        return BP_ERROR;
    }
    file_table[FDIndex].lastc = ' ';
    in_fptr = file_table[FDIndex].fdes;

    b = getc(in_fptr);
    ret_lst_ptr = &ret_lst;
    while (b != EOF && b != '\n') {
        char c = (char)b;
        if (c == '\r') {
            b = getc(in_fptr);
            continue;
        }
        if (b & 0x80) {  /* leading byte of a utf8 char? */
            char s[5], *ch_ptr;
            b = utf8_getc(in_fptr, b);
            ch_ptr = utf8_codepoint_to_str(b, s);
            *ch_ptr = '\0';
            b = ADDTAG(insert_sym(s, (ch_ptr-s), 0), ATM);
        } else {
            b = char_sym_table[(char)b];
        }
        FOLLOW(heap_top) = b;
        FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
        heap_top++;
        LOCAL_OVERFLOW_CHECK_WITH_MARGIN("read_line", 1000);
        ret_lst_ptr = heap_top++;
        b = getc(in_fptr);
    }
    if (b == EOF) {
        clearerr(in_fptr);
        if (ret_lst_ptr == &ret_lst) {
            ASSIGN_f_atom(Lst, eof_atom);
            return BP_TRUE;
        }
    }
    FOLLOW(ret_lst_ptr) = nil_sym;
    ASSIGN_v_heap_term(Lst, ret_lst);

    return BP_TRUE;
}

/* write one byte or a list of bytes into FD */
int b_WRITE_BYTE_cc(BPLONG FDIndex, BPLONG op) {
    FILE *out_fptr;
    BPLONG op0;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    out_fptr = file_table[FDIndex].fdes;
    if (file_table[FDIndex].mode == 0) {
        bp_exception = output_stream_expected;
        return BP_ERROR;
    }
    DEREF(op);
    op0 = op;
    if (ISINT(op)) {
        op = INTVAL(op);
        if (op < 0 || op > 255) {
            bp_exception = c_type_error(et_BYTE, op0);
            return BP_ERROR;
        }
        putc(op, out_fptr);
        return BP_TRUE;
    } else if (ISLIST(op)) {
        BPLONG byt;
        BPLONG_PTR lst_ptr;

        do {
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
            byt = FOLLOW(lst_ptr); DEREF(byt);
            if (ISINT(byt)) {
                byt = INTVAL(byt);
                if (byt < 0 || byt > 255) {
                    bp_exception = c_type_error(et_BYTE, op0);
                    return BP_ERROR;
                }
                putc(byt, out_fptr);
            } else {
                bp_exception = c_type_error(et_INTEGER, op0);
                return BP_ERROR;
            }
            op = FOLLOW(lst_ptr+1); DEREF(op);
        } while (ISLIST(op));
        if (op == nil_sym) {
            return BP_TRUE;
        } else {
            bp_exception = c_type_error(et_LIST, op0);
            return BP_ERROR;
        }
    } else {
        if (ISREF(op)) {
            bp_exception = et_INSTANTIATION_ERROR;
            return BP_ERROR;
        }
        bp_exception = c_type_error(et_BYTE, op);
        return BP_ERROR;
    }
}


/* op is known to be an atom */
int b_put_char(FILE *out_fptr, BPLONG op) {
    SYM_REC_PTR sym_ptr;
    BPLONG len;
    char *ch_ptr;
    sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(op);
    len = GET_LENGTH(sym_ptr);
    ch_ptr = GET_NAME(sym_ptr);
    if (len == 1) {
        putc(*ch_ptr, out_fptr);
    } else if ((len <= 4) && (utf8_nchars(ch_ptr) == 1)) {
        int c;
        c = *ch_ptr++;
        while (c != '\0') {
            putc(c, out_fptr);
            c = *ch_ptr++;
        }
    } else {
        bp_exception = char_expected;
        return BP_ERROR;
    }
    return BP_TRUE;
}

/* write one char or a list of chars into FD */
int b_WRITE_CHAR_cc(BPLONG FDIndex, BPLONG op) {
    FILE *out_fptr;
    BPLONG op0;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    out_fptr = file_table[FDIndex].fdes;
    if (file_table[FDIndex].mode == 0) {
        bp_exception = output_stream_expected;
        return BP_ERROR;
    }
    DEREF(op);
    op0 = op;
    if (ISATOM(op)) {
        if (b_put_char(out_fptr, op) == BP_ERROR) {
            return BP_ERROR;
        }
        return BP_TRUE;
    } else if (ISLIST(op)) {
        BPLONG atm;
        BPLONG_PTR lst_ptr;

        do {
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
            atm = FOLLOW(lst_ptr); DEREF(atm);
            if (ISATOM(atm)) {
                if (b_put_char(out_fptr, atm) == BP_ERROR) {
                    return BP_ERROR;
                }
            } else {
                bp_exception = c_type_error(atom_expected, atm);
                return BP_ERROR;
            }
            op = FOLLOW(lst_ptr+1); DEREF(op);
        } while (ISLIST(op));
        if (op == nil_sym) {
            return BP_TRUE;
        } else {
            bp_exception = c_type_error(et_LIST, op0);
            return BP_ERROR;
        }
    } else {
        if (ISREF(op)) {
            bp_exception = et_INSTANTIATION_ERROR;
            return BP_ERROR;
        }
        bp_exception = c_type_error(et_CHARACTER, op);
        return BP_ERROR;
    }
}

/* op is known to be an integer */
void b_put_char_code(FILE *out_fptr, BPLONG op) {
    if (op <= 127) {
        putc(op, out_fptr);
    } else {  /* unicode, write in utf8 format */
        char s[5], *ch_ptr, *ch_ptr0;
        ch_ptr = utf8_codepoint_to_str(op, s);
        ch_ptr0 = s;
        while (ch_ptr0 < ch_ptr) {
            putc(*ch_ptr0++, out_fptr);
        }
    }
}

/* write one char code or a list of char codes into FD */
int b_WRITE_CHAR_CODE_cc(BPLONG FDIndex, BPLONG op) {
    FILE *out_fptr;
    BPLONG op0;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    out_fptr = file_table[FDIndex].fdes;
    if (file_table[FDIndex].mode == 0) {
        bp_exception = output_stream_expected;
        return BP_ERROR;
    }
    DEREF(op);
    op0 = op;
    if (ISINT(op)) {
        b_put_char_code(out_fptr, INTVAL(op));
        return BP_TRUE;
    } else if (ISLIST(op)) {
        BPLONG elm;
        BPLONG_PTR lst_ptr;

        do {
            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(op);
            elm = FOLLOW(lst_ptr); DEREF(elm);
            if (ISINT(elm)) {
                b_put_char_code(out_fptr, INTVAL(elm));
            } else {
                bp_exception = c_type_error(et_INTEGER, elm);
                return BP_ERROR;
            }
            op = FOLLOW(lst_ptr+1); DEREF(op);
        } while (ISLIST(op));
        if (op == nil_sym) {
            return BP_TRUE;
        } else {
            bp_exception = c_type_error(et_LIST, op0);
            return BP_ERROR;
        }
    } else {
        if (ISREF(op)) {
            bp_exception = et_INSTANTIATION_ERROR;
            return BP_ERROR;
        }
        bp_exception = c_type_error(et_INTEGER, op);
        return BP_ERROR;
    }
}

/* write a string into FD */
int b_PICAT_PRINT_STRING_cc(BPLONG FDIndex, BPLONG Lst) {
    FILE *out_fptr;
    BPLONG i, len;

    DEREF(FDIndex); FDIndex = INTVAL(FDIndex);
    CHECK_FILE_INDEX(FDIndex);
    DEREF(Lst);
    out_fptr = file_table[FDIndex].fdes;

    while (ISLIST(Lst)) {
        BPLONG_PTR lst_ptr;
        BPLONG elm;
        SYM_REC_PTR sym_ptr;
        CHAR_PTR ch_ptr;

        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Lst);
        elm = FOLLOW(lst_ptr); DEREF(elm);
        Lst = FOLLOW(lst_ptr+1); DEREF(Lst);
        sym_ptr = GET_ATM_SYM_REC(elm);
        ch_ptr = GET_NAME(sym_ptr);
        len = GET_LENGTH(sym_ptr);
        for (i = 0; i < len; i++) {
            putc(*(ch_ptr+i), out_fptr);
        }
    }
    return BP_TRUE;
}

int b_SET_STRING_TO_PARSE_c(BPLONG Str) {
    CHAR_PTR ch_ptr = bp_buf;

    DEREF(Str);
    while (ISLIST(Str)) {
        BPLONG c;
        BPLONG_PTR list_ptr;
        SYM_REC_PTR sym_ptr;

        list_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Str);
        Str = FOLLOW(list_ptr+1); DEREF(Str);
        c = FOLLOW(list_ptr); DEREF(c);
        if (!ISATOM(c)) return BP_FALSE;
        sym_ptr = (SYM_REC_PTR)GET_ATM_SYM_REC(c);
        if (GET_LENGTH(sym_ptr) != 1) return BP_FALSE;
        *ch_ptr++ = *GET_NAME(sym_ptr);
        if (ch_ptr >= bp_buf + MAX_STR_LEN) {
            bp_exception = et_STRING_TOO_LONG;
            return BP_ERROR;
        }
    }
    if (Str != nil_sym) return BP_FALSE;
    *ch_ptr = '\0';
    string_in = bp_buf;
    lastc = ' ';
    return BP_TRUE;
}
