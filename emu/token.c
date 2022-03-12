/********************************************************************
    File        : token.c
    Author      : Richard A. O'Keefe
    Modified by : Deeporn H. Beardsley & Saumya Debray & Neng-Fa Zhou
    Updated     : Sep. 2006
    Purpose     : Tokenizer for B-Prolog.
********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "basic.h"
#include "term.h"
#include "bapi.h"
#include <errno.h>
#include <sys/types.h>

/*  We used to use an 8-bit character set under VMS, but 7-bit ASCII
 *  elsewhere.  Now that DIS 8859/1 exists (a draft international
 *  standard for an 8-bit extension of ASCII) we use that, and we are
 *  in luck: it is almost identical to the VMS character set.
 */
#define TOKEN_CHECK_EXCEPTION() {               \
        if (bp_exception != (BPLONG)NULL) {     \
            return BP_ERROR;                    \
        }                                       \
    }

#define TOKEN_CHECK_EXCEPTION_STRING() {        \
        if (bp_exception != (BPLONG)NULL) {     \
            return BP_ERROR;                    \
        }                                       \
    }

#define StrCpy(dst, src) (void)strcpy(dst, src)
#define Printf (void)printf
#define Sprintf (void)sprintf
#define Fprintf (void)fprintf

#define InRange(X, L, U) ((unsigned)((X)-(L)) <= (unsigned)((U)-(L)))
#define IsLayout(X) InRange(InType(X), SPACE, EOLN)

/*  VERY IMPORTANT NOTE: I assume that the stdio library returns the value
 *  EOF when character input hits the end of the file, and that this value
 *  is actually the integer -1.  You will note the DigVal(), InType(), and
 *  OuType() macros below, and there is a ChType() macro used in crack().
 *  They all depend on this assumption.
 */

#define DIGIT 0  /* 0 .. 9 */
#define BREAK 1  /* _ */
#define UPPER 2  /* A .. Z */
#define LOWER 3  /* a .. z */
#define SIGN 4  /* -/+*<=>#@$\^&~`:.? */
#define NOBLE 5  /* !; (don't form compounds) */
#define PUNCT 6  /* (),[]|{}% */
#define ATMQT 7  /* ' (atom quote) */
#define LISQT 8  /* " (list quote) */
#define STRQT 9  /* $ (string quote), disallowed by n.f. */
#define CHRQT 10  /* ` (character quote, maybe), disallowed by n.f. */
#define TILDE 11  /* ~ (like character quote but buggy) */
#define SPACE 12  /* layout and control chars */
#define EOLN 13  /* line terminators ^J ^L */
#define REALO 14  /* floating point number */
#define EOFCH 15  /* end of file */
#define ALPHA DIGIT  /* any of digit, break, upper, lower */
#define BEGIN BREAK  /* atom left-paren pair */
#define ENDCL EOLN  /* end of clause token */
#define RREAL 16  /* radix number(real) - overflowed */
#define RDIGIT 17  /* radix number(int) */
#define DOLLAR 18
#define BIGINT 19  /* bigint $bigint(SignSize,Digits) */

#define InType(c) (intab.chtype+1)[c]
#define DigVal(c) (digval+1)[c]

#define INC_LINE_NO curr_line_no++;

BYTE outqt[EOFCH+1];

/* codes = 0;  chars = 1; atom = 2 */
#ifdef PICAT
int double_quotes_flag = 1;
#else
int double_quotes_flag = 0;
#endif

struct CHARS
{
    int eolcom;  /* End-of-line comment, default % */
    int endeol;  /* early terminator of eolcoms, default none */
    int begcom;  /* In-line comment start, default / */
    int astcom;  /* In-line comment second, default * */
    int endcom;  /* In-line comment finish, default / */
    int radix;  /* Radix character, default ' */
    int dpoint;  /* Decimal point, default . */
    int escape;  /* String escape character, default \ */
    int termin;  /* Terminates a clause */
    CHAR chtype[AlphabetSize+1];
};

struct CHARS intab =  /* Special character table */
{
    '%',  /* eolcom: end of line comments */
    -1,  /* endeol: early end for eolcoms */
    '/',  /* begcom: in-line comments */
    '*',  /* astcom: in-line comments */
    '/',  /* endcom: in-line comments */
    '\'',  /* radix : radix separator */
    '.',  /* dpoint: decimal point */
    -1,  /* escape: string escape character */
    '.',  /* termin: ends clause, sign or solo */
    {
        EOFCH,  /* really the -1th element of the table: */
        /*  ^@      ^A      ^B      ^C      ^D      ^E      ^F      ^G      */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  ^H      ^I      ^J      ^K      ^L      ^M      ^N      ^O      */
        SPACE, SPACE, EOLN, SPACE, EOLN, SPACE, SPACE, SPACE,
        /*  ^P      ^Q      ^R      ^S      ^T      ^U      ^V      ^W      */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  ^X      ^Y      ^Z      ^[      ^\      ^]      ^^      ^_      */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  sp      !       "       #       $       %       &       '       */
        SPACE, NOBLE, LISQT, SIGN, DOLLAR, PUNCT, SIGN, ATMQT,
        /*  (       )       *       +       ,       -       .       /       */
        PUNCT, PUNCT, SIGN, SIGN, PUNCT, SIGN, SIGN, SIGN,
        /*  0       1       2       3       4       5       6       7       */
        DIGIT, DIGIT, DIGIT, DIGIT, DIGIT, DIGIT, DIGIT, DIGIT,
        /*  8       9       :       ;       <       =       >       ?       */
        DIGIT, DIGIT, SIGN, PUNCT, SIGN, SIGN, SIGN, SIGN,
        /*  @       A       B       C       D       E       F       G       */
        SIGN, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
        /*  H       I       J       K       L       M       N       O       */
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
        /*  P       Q       R       S       T       U       V       W       */
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
        /*  X       Y       Z       [       \       ]       ^       _       */
        UPPER, UPPER, UPPER, PUNCT, SIGN, PUNCT, SIGN, BREAK,
        /*  `       a       b       c       d       e       f       g       */
        SIGN, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
        /*  h       i       j       k       l       m       n       o       */
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
        /*  p       q       r       s       t       u       v       w       */
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
        /*  x       y       z       {       |       }       ~       ^?      */
        LOWER, LOWER, LOWER, PUNCT, PUNCT, PUNCT, SIGN, SPACE,
        /*  128     129     130     131     132     133     134     135     */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  136     137     138     139     140     141     142     143     */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  144     145     146     147     148     149     150     151     */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  152     153     154     155     156     157     158     159     */
        SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
        /*  NBSP    !-inv   cents   pounds  ching   yen     brobar  section */
        SPACE, SIGN, SIGN, SIGN, SIGN, SIGN, SIGN, SIGN,
        /*  "accent copyr   -a ord  <<      nothook SHY     (reg)   ovbar   */
        SIGN, SIGN, LOWER, SIGN, SIGN, SIGN, SIGN, SIGN,
        /*  degrees +/-     super 2 super 3 -       micron  pilcrow -       */
        SIGN, SIGN, LOWER, LOWER, SIGN, SIGN, SIGN, SIGN,
        /*  ,       super 1 -o ord  >>      1/4     1/2     3/4     ?-inv   */
        SIGN, LOWER, LOWER, SIGN, SIGN, SIGN, SIGN, SIGN,
        /*  `A      'A      ^A      ~A      "A      oA      AE      ,C      */
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
        /*  `E      'E      ^E      "E      `I      'I      ^I      "I      */
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
        /*  ETH     ~N      `O      'O      ^O      ~O      "O      x times */
#ifdef vms
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER,
#else
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, SIGN,
#endif
        /*  /O      `U      'U      ^U      "U      'Y      THORN   ,B      */
        UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, UPPER, LOWER,
        /*  `a      'a      ^a      ~a      "a      oa      ae      ,c      */
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
        /*  `e      'e      ^e      "e      `i      'i      ^i      "i      */
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
        /*  eth     ~n      `o      'o      ^o      ~o      "o      -:-     */
#ifdef vms
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER,
#else
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, SIGN,
#endif
        /*  /o      `u      'u      ^u      "u      'y      thorn  "y       */
#ifdef vms
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, SPACE
#else
        LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER, LOWER
#endif
    }
};

CHAR digval[AlphabetSize+1] =
{
    99,  /* really the -1th element of the table */
    /*  ^@      ^A      ^B      ^C      ^D      ^E      ^F      ^G      */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  ^H      ^I      ^J      ^K      ^L      ^M      ^N      ^O      */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  ^P      ^Q      ^R      ^S      ^T      ^U      ^V      ^W      */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  ^X      ^Y      ^Z      ^[      ^\      ^]      ^^      ^_      */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  sp      !       "       #       $       %       &       '       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  (       )       *       +       ,       -       .       /       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  0       1       2       3       4       5       6       7       */
    0, 1, 2, 3, 4, 5, 6, 7,
    /*  8       9       :       ;       <       =       >       ?       */
    8, 9, 99, 99, 99, 99, 99, 99,
    /*  @       A       B       C       D       E       F       G       */
    99, 10, 11, 12, 13, 14, 15, 99,
    /*  H       I       J       K       L       M       N       O       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  P       Q       R       S       T       U       V       W       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  X       Y       Z       [       \       ]       ^       _       */
    99, 99, 99, 99, 99, 99, 99, 0,  /*NB*/
    /*  `       a       b       c       d       e       f       g       */
    99, 10, 11, 12, 13, 14, 15, 99,
    /*  h       i       j       k       l       m       n       o       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  p       q       r       s       t       u       v       w       */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  x       y       z       {       |       }       ~       ^?      */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  128     129     130     131     132     133     134     135     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  136     137     138     139     140     141     142     143     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  144     145     146     147     148     149     150     151     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  152     153     154     155     156     157     158     159     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  160     161     162     163     164     165     166     167     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  168     169     170(-a) 171     172     173     174     175     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  176     177     178(2)  179(3)  180     181     182     183     */
    99, 99, 2, 3, 99, 99, 99, 99,
    /*  184     185(1)  186(-o) 187     188     189     190     191     */
    99, 1, 99, 99, 99, 99, 99, 99,
    /*  192     193     194     195     196     197     198     199     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  200     201     202     203     204     205     206     207     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  208     209     210     211     212     213     214     215     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  216     217     218     219     220     221     222     223     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  224     225     226     227     228     229     230     231     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  232     233     234     235     236     237     238     239     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  240     241     242     243     244     245     246     247     */
    99, 99, 99, 99, 99, 99, 99, 99,
    /*  248     249     250     251     252     253     254     255     */
    /*    99,     99,     99,     99,     99,     99,     99,     99  revised by NIDA  */
    99, 99, 99, 99, 99, 99, 99, 99
};


/* values returned to calling program */

#define SPECIAL 0  /* puncuation , ( ) [ ] ... */
#define VARO 1  /* type is a variable */
#define FUNC 2  /* type is atom( */
#define INTEGERO 3  /* type is a integer */
#define ATOMO 4  /* type is an atom */
#define ENDCLS 5  /* END of clause but not file */
#define USCORE 6  /* underscore '_' */
#define SEMI 7  /* ; */
#define BADEND 8  /* END of file, not end of clause */
#define STRING 9  /* type is a char string */
#define SPECIAL_NUM 10  /* type is float or bigint */

/*
  BPLONG cNUMERO = 0, cATOMO   = 0, cFUNC = 0, cVARO   = 0, cUSCORE = 0,
  cSTRING = 0, cSPECIAL = 0, cSEMI = 0, cENDCLS = 0, cENDPRG = 0;
*/
extern FILE *curr_in, *curr_out;  /* current input, output streams */

extern BPLONG in_file_i;

char *string_in = NULL;

BPLONG term_start_line_no, term_start_pool_index;

char chars_pool[MAX_CHARS_IN_POOL];
int chars_pool_index = 0;

#define MAX_TOKENS_IN_TERM 1000
int token_start_pos[MAX_TOKENS_IN_TERM];
int next_token_index = 0;

#ifdef BPSOLVER
#define BP_GETC_STRING(c) {                     \
        c = *string_in;                         \
        if (c != 0) {                           \
            string_in++;                        \
        }                                       \
    }

#define BP_UNGETC_STRING {                      \
        string_in--;                            \
    }

#define BP_GETC(card, c) {                      \
        c = getc(card);                         \
    }

#define BP_UNGETC(c, card) {                    \
        ungetc((char)c, card);                  \
    }
#else
#define BP_GETC_STRING(c) {                                             \
        c = *string_in;                                                 \
        if (c != 0) {                                                   \
            string_in++;                                                \
            if (chars_pool_index < MAX_CHARS_IN_POOL) chars_pool[chars_pool_index++] = (char)c; \
        }                                                               \
    }

#define BP_UNGETC_STRING {                      \
        string_in--;                            \
        chars_pool_index--;                     \
    }

#define BP_GETC(card, c) {                                              \
        c = getc(card);                                                 \
        if (c >= 0) {                                                   \
            if (chars_pool_index < MAX_CHARS_IN_POOL) chars_pool[chars_pool_index++] = (char)c; \
        }                                                               \
    }

#define BP_UNGETC(c, card) {                    \
        ungetc((char)c, card);                  \
        chars_pool_index--;                     \
    }
#endif

/* convert a code point to char array s, which has n remaining slots */
#define UTF8_CODEPOINT_TO_STR(code, s, n) {             \
        if (code <= 127) {                              \
            n--;                                        \
            if (n < 0) {                                \
                printAtomStr(tok2long);                 \
            } else {                                    \
                *s++ = (BYTE)code;                      \
            }                                           \
        } else {                                        \
            if (n < 4) {                                \
                printAtomStr(tok2long);                 \
            } else {                                    \
                CHAR_PTR s1;                            \
                s1 = utf8_codepoint_to_str(code, s);    \
                n -= (s1-s);                            \
                s = s1;                                 \
            }                                           \
        }                                               \
    }

CHAR AtomStr[MAX_STR_LEN];
BPLONG list_p;
BPLONG rtnint;
double double_v;
BPLONG rad_int;

CHAR tok2long[] = "token too long";
CHAR eofinrem[] = "end of file in comment";
CHAR badexpt[] = "bad exponent";
CHAR badradix[] = "radix > 36";


char *utf8_codepoint_to_str(int code, CHAR_PTR s) {
    if (code < 0x80) {
        *s++ = code;
    } else if (code < 0x800) {
        *s++ = 192+code/64;
        *s++ = 128+code%64;
    } else if (code < 0x10000) {
        *s++ = 224+code/4096;
        *s++ = 128+code/64%64;
        *s++ = 128+code%64;
    } else {
        *s++ = 240+code/262144;
        *s++ = 128+code/4096%64;
        *s++ = 128+code/64%64;
        *s++ = 128+code%64;
    }
    return s;
}

/* return the codepoint of the current utf-8 char, setting s_ptr to the beginning of the next char */
int utf8_char_to_codepoint(CHAR_PTR *s_ptr) {
    CHAR_PTR s;
    int c, b2, b3, b4;

    s = *s_ptr;
    //  printf(" utf8_char_to_codepoint %s\n",s);
    c = *s++;

    if (c & 0x80) {  /* leading byte of a utf8 char? */
        if ((c & 0xe0) == 0xc0) {  /* 110xxxxx */
            b2 = *s++;
            if ((b2 & 0xc0) == 0x80) {  /* 110xxxxx 10xxxxxx */
                c = (((c & 0x1f) << 6) | (b2 & 0x3f));
            } else {
                s--;
            }
        } else if ((c & 0xf0) == 0xe0) {  /* 1110xxxx */
            b2 = *s++;
            if ((b2 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx */
                b3 = *s++;
                if ((b3 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx 10xxxxxx */
                    c = (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
                } else {
                    s--;
                    s--;
                }
            } else {
                s--;
            }
        } else if ((c & 0xf8) == 0xf0) {  /* 11110xxx */
            b2 = *s++;
            if ((b2 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx */
                b3 = *s++;
                if ((b3 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx */
                    b4 = *s++;
                    if ((b4 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                        c = (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
                    } else {
                        s--;
                        s--;
                        s--;
                    }
                } else {
                    s--;
                    s--;
                }
            } else {
                s--;
            }
        }
    }
    *s_ptr = s;
    return c;
}


/* read a utf8 char whose leading byte is c */
int utf8_getc(FILE *curr_in, int c) {
    int b2, b3, b4;

    if ((c & 0xe0) == 0xc0) {  /* 110xxxxx */
        b2 = getc(curr_in);
        if ((b2 & 0xc0) == 0x80) {  /* 110xxxxx 10xxxxxx */
            return (((c & 0x1f) << 6) | (b2 & 0x3f));
        } else {  /* not utf8 char */
            if (b2 > 0) {ungetc((char)b2, curr_in);} /* don't unget EOF */
        }
    } else if ((c & 0xf0) == 0xe0) {  /* 1110xxxx */
        b2 = getc(curr_in);
        if ((b2 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx */
            b3 = getc(curr_in);
            if ((b3 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx 10xxxxxx */
                return (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
            } else {
                if (b3 > 0) {ungetc((char)b3, curr_in);}
                ungetc((char)b2, curr_in);
            }
        } else {
            if (b2 > 0) {ungetc((char)b2, curr_in);}
        }
    } else if ((c & 0xf8) == 0xf0) {  /* 11110xxx */
        b2 = getc(curr_in);
        if ((b2 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx */
            b3 = getc(curr_in);
            if ((b3 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx */
                b4 = getc(curr_in);
                if ((b4 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                    return (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
                } else {
                    if (b4 > 0) {ungetc((char)b4, curr_in);}
                    ungetc((char)b3, curr_in);
                    ungetc((char)b2, curr_in);
                }
            } else {
                if (b3 > 0) {ungetc((char)b3, curr_in);}
                ungetc((char)b2, curr_in);
            }
        } else {
            if (b2 > 0) {ungetc((char)b2, curr_in);}
        }
    }
    return c;  /* an ASCII char or a char in a different encoding */
}

/* Return the number of utf-8 chars in s. */
int utf8_nchars(char *s) {
    unsigned int c;
    int count = 0;

    //  printf("utf8_nchars(%s) %x\n",s,s);

    c = *s++;
    while (c != '\0') {
        count++;
        if (c & 0x80) {  /* leading byte of a utf8 char? */
            if ((c & 0xe0) == 0xc0) {  /* 110xxxxx */
                c = *s++;
                if ((c & 0xc0) == 0x80) {  /* 110xxxxx 10xxxxxx */
                    /* two bytes, but one char */
                } else {
                    s--;  /* not utf8, couting bytes instead */
                }
            } else if ((c & 0xf0) == 0xe0) {  /* 1110xxxx */
                c = *s++;
                if ((c & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx */
                    c = *s++;
                    if ((c & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx 10xxxxxx */
                        /* three bytes, but one char */
                    } else {
                        s--;
                        s--;
                    }
                } else {
                    s--;
                }
            } else if ((c & 0xf8) == 0xf0) {  /* 11110xxx */
                c = *s++;
                if ((c & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx */
                    c = *s++;
                    if ((c & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx */
                        c = *s++;
                        if ((c & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                            /* four bytes, but one char */
                        } else {
                            s--;
                            s--;
                            s--;
                        }
                    } else {
                        s--;
                        s--;
                    }
                } else {
                    s--;
                }
            }
        }
        c = *s++;
    }
    return count;
}

void SyntaxError(message)
    CHAR_PTR message;
{
    //  Fprintf(stderr, "Syntax error: %s\n", message);
    bp_exception = c_syntax_error(ADDTAG(insert_sym(message, strlen(message), 0), ATM));
}

/*  GetToken() reads a single token from the input stream and returns
 *  its type, which is one of
 *      DIGIT   -- a number
 *      BEGIN   -- an atom( pair
 *      LOWER   -- an atom
 *      UPPER   -- a variable
 *      PUNCT   -- a single punctuation mark
 *      LISQT   -- a quoted list of character codes
 *      STRQT   -- a quoted string disallowed by n.f.
 *      ENDCL   -- end of clause (normally '.\n').
 *      EOFCH   -- signifies end-of-file.
 *      RREAL   -- a real, from some radix notation, in double_v.
 *      RDIGIT  -- an integer, from some radix notation, in rad_int.
 *      DOLLAR  -- an atom or an operator
 *  In all cases except the last, the text of the token is in AtomStr.
 *  There are two questions: between which pairs of adjacent tokens is
 *  a space (a) necessary, (b) desirable?  There is an additional
 *  dummy token type used by the output routines, namely
 *      NOBLE   -- extra space is definitely not needed.
 *  I leave it as an exercise for the reader to answer question (a).
 *  Since this program is to produce output I find palatable (even if
 *  it isn't exactly what I'd write myself), extra spaces are ok.  In
 *  fact, the main use of this program is as an editor command, so it
 *  is normal to do a bit of manual post-processing.  Question (b) is
 *  the one to worry about then.  My answer is that a space is never
 *  written
 *      - after  PUNCT ( [ { |
 *      - before PUNCT ) ] } | , <ENDCL>
 *  is written after comma only sometimes, and is otherwise always
 *  written.  The variable lastput thus takes these values:
 *      ALPHA   -- put a space except before PUNCT
 *      SIGN    -- as alpha, but different so ENDCL knows to put a space.
 *      NOBLE   -- don't put a space
 *      ENDCL   -- just ended a clause
 *      EOFCH   -- at beginning of file
 */

int handleEndInQuoted()
{
    bp_exception = c_syntax_error(et_IN_CHARACTER);
    return BP_ERROR;
}

/*  read_utf8_character(FILE* card, BYTE q)
 *  reads one character from a quoted atom, list, string, or character.
 *  Doubled quotes are read as single characters, otherwise a
 *  quote is returned as -1 and lastc is set to the next character.
 *  If the input syntax has character escapes, they are processed.
 *  Note that many more character escape sequences are accepted than
 *  are generated.  There is a divergence from C: \xhh sequences are
 *  two hexadecimal digits long, not three.
 *
 *  Extended by nfz to read UTF-8 characters, including unicode escapes
 *  in the form \uhhhh...h
 */

int read_utf8_character(FILE * card, int q) {
    int c, b2, b3, b4;

    BP_GETC(card, c);
    if (c < 0) {  /* EOF */
        lastc = c;
        return handleEndInQuoted();
    }
    if (q == -1) return c;
    if (c == q) {
        BP_GETC(card, c);
        if (c == q) return c;  /* q is also used as an escape */
        lastc = c;
        return -1;
    }

    if (c == '\n') {INC_LINE_NO;}

    if (c != '\\') {  /* not the escape char */
        if (c & 0x80) {  /* leading byte of a utf8 char? */
            if ((c & 0xe0) == 0xc0) {  /* 110xxxxx */
                BP_GETC(card, b2);
                if ((b2 & 0xc0) == 0x80) {  /* 110xxxxx 10xxxxxx */
                    return (((c & 0x1f) << 6) | (b2 & 0x3f));
                } else {  /* not utf8 char */
                    if (b2 > 0) {BP_UNGETC(b2, card);} /* don't unget EOF */
                }
            } else if ((c & 0xf0) == 0xe0) {  /* 1110xxxx */
                BP_GETC(card, b2);
                if ((b2 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx */
                    BP_GETC(card, b3);
                    if ((b3 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx 10xxxxxx */
                        return (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
                    } else {
                        if (b3 > 0) {BP_UNGETC(b3, card);}
                        BP_UNGETC(b2, card);
                    }
                } else {
                    if (b2 > 0) {BP_UNGETC(b2, card);}
                }
            } else if ((c & 0xf8) == 0xf0) {  /* 11110xxx */
                BP_GETC(card, b2);
                if ((b2 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx */
                    BP_GETC(card, b3);
                    if ((b3 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx */
                        BP_GETC(card, b4);
                        if ((b4 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                            return (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
                        } else {
                            if (b4 > 0) {BP_UNGETC(b4, card);}
                            BP_UNGETC(b3, card);
                            BP_UNGETC(b2, card);
                        }
                    } else {
                        if (b3 > 0) {BP_UNGETC(b3, card);}
                        BP_UNGETC(b2, card);
                    }
                } else {
                    if (b2 > 0) {BP_UNGETC(b2, card);}
                }
            }
        }
        return c;  /* an ASCII char or a char in a different encoding */
    }

    /*  If we get here, we have read the "\" of an escape sequence  */

    BP_GETC(card, c);
    switch (c) {
    case EOF:
        clearerr(curr_in);
        //    return handleEndInQuoted(q,-1);
        return handleEndInQuoted();
    case '\r':
    {
        char dummy;
        BP_GETC(card, dummy);
    }
    case '\n':
        INC_LINE_NO;
        return read_utf8_character(card, q);
    case 'a':  /* alarm */
        return 7;
    case 'b':  /* backspace */
        return 8;
    case 'r':  /* return */
        return 13;
    case 'f':  /* formfeed */
        return 12;
    case 't':  /* tab */
        return 9;
    case 'n':  /* newline */
        return 10;
    case 'v':  /* vertical tab */
        return 11;
    case '\\':
    case '"':
    case '\'':
    case '`':
        return c;
    case 'x': case 'X':  /* hexadecimal */
    {BPLONG n;
            n = 0;
            BP_GETC(card, c);
            while (c != '\'' && c != '\\' && c != '\n' && c != -1) {
                if (DigVal(c) <= 15) {
                    n = (n << 4) + DigVal(c);
                    BP_GETC(card, c);
                } else {
                    bp_exception = c_syntax_error(et_IN_CHARACTER);
                    return n;
                }
            }
            if (c != '\\') {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
            return n;
    }

    case 'u': case 'U':  /* utf-8 escape */
    {BPLONG n, i;
        n = 0;
        for (i = 1; i <= 4; i++) {  /* \uxxxx */
            BP_GETC(card, c);
            if (DigVal(c) <= 15 && c != '_') {
                n = (n << 4) + DigVal(c);
            } else {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
        }
        BP_GETC(card, c);
        if (DigVal(c) <= 15 && c != '_') {  /* \uxxxxxxxx */
            n = (n << 4) + DigVal(c);
            for (i = 1; i <= 3; i++) {
                BP_GETC(card, c);
                if (DigVal(c) <= 15 && c != '_') {
                    n = (n << 4) + DigVal(c);
                } else {
                    bp_exception = c_syntax_error(et_IN_CHARACTER);
                    return n;
                }
            }
        } else {
            if (c > 0) {BP_UNGETC(c, card);}
        }
        return n;
    }

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    {BPLONG n;
        n = DigVal(c);
        BP_GETC(card, c);
        while (c != '\'' && c != '\\' && c != '\n' && c != -1) {
            if (DigVal(c) <= 7) {
                n = (n << 3) + DigVal(c);
                BP_GETC(card, c);
            } else {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
        }
        if (c != '\\') {
            bp_exception = c_syntax_error(et_IN_CHARACTER);
            return n;
        }
        return n;
    }
    default:
        fprintf(stderr, "*** invalid escape character\n");
        bp_exception = c_syntax_error(et_IN_ESCCHARACTER);
        return c;
    }
}

int handleEolInQuoted2()
{
    lastc = '\0';
    bp_exception = c_syntax_error(et_IN_CHARACTER);
    return BP_ERROR;
}

int read_utf8_character_string(int q) {
    int c, b2, b3, b4;

    BP_GETC_STRING(c);
    if (c == '\0') return handleEolInQuoted2();
    if (q == -1) return c;

    if (c == q) {
        BP_GETC_STRING(c);
        if (c == q) return c;  /* q is also used as an escape */
        lastc = c;
        return -1;
    }

    if (c != '\\') {  /* escape char */
        if (c & 0x80) {  /* leading byte of a utf8 char? */
            if ((c & 0xe0) == 0xc0) {  /* 110xxxxx */
                BP_GETC_STRING(b2);
                if ((b2 & 0xc0) == 0x80) {  /* 110xxxxx 10xxxxxx */
                    return (((c & 0x1f) << 6) | (b2 & 0x3f));
                } else {  /* not utf8 char */
                    if (b2 != 0) {BP_UNGETC_STRING;} /* do't unget '\0' */
                }
            } else if ((c & 0xf0) == 0xe0) {  /* 1110xxxx */
                BP_GETC_STRING(b2);
                if ((b2 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx */
                    BP_GETC_STRING(b3);
                    if ((b3 & 0xc0) == 0x80) {  /* 1110xxxx 10xxxxxx 10xxxxxx */
                        return (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
                    } else {
                        if (b3 > 0) { BP_UNGETC_STRING;}
                        BP_UNGETC_STRING;
                    }
                } else {
                    if (b2 > 0) {BP_UNGETC_STRING;}
                }
            } else if ((c & 0xf8) == 0xf0) {  /* 11110xxx */
                BP_GETC_STRING(b2);
                if ((b2 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx */
                    BP_GETC_STRING(b3);
                    if ((b3 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx */
                        BP_GETC_STRING(b4);
                        if ((b4 & 0xc0) == 0x80) {  /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                            return (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
                        } else {
                            if (b4 > 0) {BP_UNGETC_STRING;}
                            BP_UNGETC_STRING;
                            BP_UNGETC_STRING;
                        }
                    } else {
                        if (b3 > 0) {BP_UNGETC_STRING;}
                        BP_UNGETC_STRING;
                    }
                } else {
                    if (b2 > 0) {BP_UNGETC_STRING;}
                }
            }
        }
        return c;
    }
    /*  If we get here, we have read the "\" of an escape sequence  */

    BP_GETC_STRING(c);
    switch (c) {
    case '\n':
    case '\0':
        return handleEolInQuoted2();
    case 'a':  /* alarm */
        return 7;
    case 'b':  /* backspace */
        return 8;
    case 'r':  /* return */
        return 13;
    case 'f':  /* formfeed */
        return 12;
    case 't':  /* tab */
        return 9;
    case 'n':  /* newline */
        return 10;
    case 'v':  /* vertical tab */
        return 11;
    case '\\':
    case '"':
    case '\'':
    case '`':
        return c;
    case 'x':  /* hexadecimal */
    {BPLONG n;
            n = 0;
            BP_GETC_STRING(c);
            while (c != '\0' && c != '\'' && c != '\\' && c != '\n') {
                if (DigVal(c) <= 15) {
                    n = (n << 4) + DigVal(c);
                    BP_GETC_STRING(c);
                } else {
                    bp_exception = c_syntax_error(et_IN_CHARACTER);
                    return n;
                }
            }
            if (c == '\n') BP_UNGETC_STRING;
            if (c != '\\') {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
            return n;
    }

    case 'u': case 'U':  /* unicode (utf-8) escape */
    {BPLONG n, i;
        n = 0;
        for (i = 1; i <= 4; i++) {  /* \uxxxx */
            BP_GETC_STRING(c);
            if (DigVal(c) <= 15 && c != '_') {
                n = (n << 4) + DigVal(c);
            } else {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
        }
        BP_GETC_STRING(c);
        if (DigVal(c) <= 15 && c != '_') {  /* \uxxxxxxxx */
            n = (n << 4) + DigVal(c);
            for (i = 1; i <= 3; i++) {
                BP_GETC_STRING(c);
                if (DigVal(c) <= 15 && c != '_') {
                    n = (n << 4) + DigVal(c);
                } else {
                    bp_exception = c_syntax_error(et_IN_CHARACTER);
                    return n;
                }
            }
        } else {
            if (c > 0) {BP_UNGETC_STRING;}
        }
        return n;
    }

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    {BPLONG n;
        n = DigVal(c);
        BP_GETC_STRING(c);
        while (c != '\0' && c != '\'' && c != '\\' && c != '\n') {
            if (DigVal(c) <= 7) {
                n = (n << 3) + DigVal(c);
                BP_GETC_STRING(c);
            } else {
                bp_exception = c_syntax_error(et_IN_CHARACTER);
                return n;
            }
        }
        if (c == '\n') BP_UNGETC_STRING;
        if (c != '\\') {
            bp_exception = c_syntax_error(et_IN_CHARACTER);
            return n;
        }
        // printf("char = %d\n",n);
        return n;
    }
    default:
        fprintf(stderr, "*** invalid escape character\n");
        bp_exception = c_syntax_error(et_IN_ESCCHARACTER);
        return c;
    }
}

/*  com0plain(card, endeol)
 *  These comments have the form
 *      <eolcom> <char>* <newline>                      {PUNCT}
 *  or  <eolcom><eolcom> <char>* <newline>              {SIGN }
 *  depending on the classification of <eolcom>.  Note that we could
 *  handle ADA comments with no trouble at all.  There was a Pop-2
 *  dialect which had end-of-line comments using "!" where the comment
 *  could also be terminated by "!".  You could obtain the effect of
 *  including a "!" in the comment by doubling it, but what you had
 *  then was of course two comments.  The endeol parameter of this
 *  function allows the handling of comments like that which can be
 *  terminated either by a new-line character or an <endeol>, whichever
 *  comes first.  For ordinary purposes, endeol = -1 will do fine.
 *  When this is called, the initial <eolcom>s have been consumed.
 *  We return the first character after the comment.
 *  If the end of the source file is encountered, we do not treat it
 *  as an error, but quietly close the comment and return EOF as the
 *  "FOLLOWing" character.
 */

int com0plain(card, endeol)
    FILE *card;  /* source file */
    int endeol;  /* The closing character "!" */
{
    int c;

    BP_GETC(card, c);
    while (c >= 0 && c != '\n' && c != endeol) {BP_GETC(card, c);}
    ;
    if (c >= 0) {
        BP_GETC(card, c);
        INC_LINE_NO;
    }
    return c;
}

int com0plain_string(endeol)
    int endeol;  /* The closing character "!" */
{
    int c;

    while ((c = *string_in++) > 0 && c != '\n' && c != endeol)
        ;
    if (c > 0)
        c = *string_in++;
    return c;
}

/*  The states in the next two functions are
 *      0       - after an uninteresting character
 *      1       - after an "astcom"
 *      2       - after a  "begcom"
 *  Assuming begcom = "(", astom = "#", endcom = ")",
 *  com2plain will accept "(#)" as a complete comment.  This can
 *  be changed by initialising the state to 0 rather than 1.
 *  The same is true of com2nest, which accepts "(#(#)#) as a
 *  complete comment.  Changing it would be rather harder.
 *  Fixing the bug where the closing <astcom> is copied if it is
 *  not an asterisk may entail rejecting "(#)".
 */

/*  com2plain(card, astcom, endcom)
 *  handles PL/I-style comments, that is, comments which begin with
 *  a pair of characters <begcom><astcom> and end with a pair of
 *  chracters <astcom><endcom>, where nesting is not allowed.  For
 *  example, if we take begcom='(', astcom='*', endcom=')' as in
 *  Pascal, the comment "(* not a (* plain *)^ comment *) ends at
 *  the "^".
 *  For this kind of comment, it is perfectly sensible for any of
 *  the characters to be equal.  For example, if all three of the
 *  bracket characters are "#", then "## stuff ##" is a comment.
 *  When this is called, the initial <begcom><astcom> has been consumed.
 */

int com2plain(card, astcom, endcom)
    FILE *card;  /* source file */
    int astcom;  /* The asterisk character "*" */
    int endcom;  /* The closing character "/" */
{
    int c;
    int state;

    BP_GETC(card, c);
    for (state = 0; c >= 0; ) {
        if (c == '\n') {INC_LINE_NO;}
        if (c == endcom && state)
            break;
        state = c == astcom;
        BP_GETC(card, c);
    }
    if (c < 0) {
        SyntaxError(eofinrem);
        return BP_ERROR;
    }
    return BP_TRUE;
}

int com2plain_string(astcom, endcom)
    int astcom;  /* The asterisk character "*" */
    int endcom;  /* The closing character "/" */
{
    int c;
    int state;

    BP_GETC_STRING(c);
    for (state = 0; c > 0; ) {
        if (c == endcom && state)
            break;
        state = c == astcom;
        BP_GETC_STRING(c);
    }
    if (c == 0) {
        SyntaxError(eofinrem);
        return BP_ERROR;
    }
    return BP_TRUE;
}


int GetToken()
{
    FILE *card = curr_in;
    CHAR_PTR s = AtomStr;
    int c, d;
    BPLONG n = MAX_STR_LEN;
    BPLONG_PTR newpair, list_head;
    BPLONG oldv, newv;

    c = lastc;

START:
    switch (InType(c)) {
    case DIGIT:
        /* The FOLLOWing kinds of numbers exist:
         * (1) unsigned decimal integers: d+
         * (2) unsigned based integers: d+Ro+[R]
         * (3) unsigned floats: d* [. d*] [e +/-] d+
         * (4) characters: 0Rc[R]
         * (5) binary (0b|0B)..., octal (0o|0O)..., and hexadecimal (0x|0X)...
         * We allow underscores in numbers too, ignoring them. 
         */
        if (c == '0') {
            BP_GETC(card, d);
            if (d == 'b' || d == 'B' || d == 'o' || d == 'O' || d == 'x' || d == 'X') {  /* binary, octal, hexadecimal*/
                int base;
                if (d == 'b' || d == 'B') base = 2;
                else if (d == 'o' || d == 'O') base = 8;
                else base = 16;
                BP_GETC(card, c);  /* first char right after must be a valid digit of the base */
                if (DigVal(c) < 0 || DigVal(c) >= base || c == '_') {
                    bp_exception = c_syntax_error(et_INTEGER);
                    return BP_ERROR;
                }
                newv = 0;
                do {
                    oldv = newv;
                    newv = newv*base + DigVal(c);

                    if (newv < oldv || newv > BP_MAXINT_1W) {  /* begin read bigint */
                        BPLONG big_base = bp_int_to_bigint(base);
                        rad_int = bp_int_to_bigint(oldv);
                        do {
                            rad_int = bp_mul_bigint_bigint(rad_int, big_base);
                            if (rad_int == BP_ERROR) return BP_ERROR;
                            if (ISINT(rad_int)) rad_int = bp_int_to_bigint(INTVAL(rad_int));
                            rad_int = bp_add_bigint_bigint(rad_int, bp_int_to_bigint(DigVal(c)));
                            BP_GETC(card, c);
                            while (c == '_') {BP_GETC(card, c);}
                        } while (DigVal(c) >= 0 && DigVal(c) < base);
                        if (DigVal(c) < 99) {
                            bp_exception = c_syntax_error(et_INTEGER);
                            return BP_ERROR;
                        }
                        lastc = c;
                        return BIGINT;
                    }  /* end read bigint */

                    BP_GETC(card, c);
                    while (c == '_') {BP_GETC(card, c);}
                } while (DigVal(c) >= 0 && DigVal(c) < base);
                if (DigVal(c) < 99) {
                    bp_exception = c_syntax_error(et_INTEGER);
                    return BP_ERROR;
                }
                rad_int = newv;
                lastc = c;
                return RDIGIT;
            } else {
                *s++ = c; c = d;
            }
        }
        while (InType(c) == DIGIT || c == '_') {
            if (c != '_') *s++ = c;
            BP_GETC(card, c);
        }
        if (c == intab.radix) {  /* ' */
            *s = 0;
            for (d = 0, s = AtomStr; (c = *s++); ) {
                d = d*10-'0'+c;
                if (d > 36) {
                    bp_exception = et_INT_OVERFLOW;
                    return BP_ERROR;
                }
            }
            if (d == 0) {  /*  0'c['] is a character code  */
                d = read_utf8_character(card, '\'');
                //                TOKEN_CHECK_EXCEPTION();
                rad_int = d;
                BP_GETC(card, d);
                if (d == intab.radix) {
                    BP_GETC(card, lastc);
                } else {
                    lastc = d;
                }
                return RDIGIT;
            }
            BP_GETC(card, c);
            newv = 0;
            while (DigVal(c) < 99) {
                if (c != '_') {
                    oldv = newv;
                    if (DigVal(c) >= d) {
                        bp_exception = et_INT_OVERFLOW;
                        return BP_ERROR;
                    }
                    newv = newv*d + DigVal(c);

                    if (newv < oldv || newv > BP_MAXINT_1W) {  /* begin read bigint */
                        BPLONG big_base = bp_int_to_bigint(d);
                        rad_int = bp_int_to_bigint(oldv);
                        while (DigVal(c) < 99) {
                            if (c != '_') {
                                if (DigVal(c) >= d) {
                                    bp_exception = et_INT_OVERFLOW;
                                    return BP_ERROR;
                                }
                                rad_int = bp_mul_bigint_bigint(rad_int, big_base);
                                if (rad_int == BP_ERROR) return BP_ERROR;
                                if (ISINT(rad_int)) rad_int = bp_int_to_bigint(INTVAL(rad_int));
                                rad_int = bp_add_bigint_bigint(rad_int, bp_int_to_bigint(DigVal(c)));
                            }
                            BP_GETC(card, c);
                        }
                        if (c == intab.radix)
                            BP_GETC(card, c);
                        lastc = c;
                        return BIGINT;
                    }  /* end read bigint */

                }
                BP_GETC(card, c);
            }
            rad_int = newv;
            if (c == intab.radix)
                BP_GETC(card, c);
            lastc = c;
            return RDIGIT;
        } else if (c == intab.dpoint) {  /* . */
            BP_GETC(card, d);
            if (InType(d) == DIGIT) {
                *s++ = '.';
                do {
                    if (d != '_')
                        *s++ = (BYTE)d;
                    BP_GETC(card, d);
                } while (InType(d) <= BREAK);
                if ((d | 32) == 'e') {
                    *s++ = 'E';
                    BP_GETC(card, d);
                    if (d == '-') {
                        *s++ = (BYTE)d;
                        BP_GETC(card, d);
                    } else if (d == '+')
                        BP_GETC(card, d);
                    if (InType(d) > BREAK) {
                        SyntaxError(badexpt);
                        return BP_ERROR;
                    }
                    do {
                        if (d != '_')
                            *s++ = (BYTE)d;
                        BP_GETC(card, d);
                    } while (InType(d) <= BREAK);
                }
                c = d;
                *s = 0;
                lastc = c;
                return REALO;
            } else  /* c has not changed */
                BP_UNGETC(d, card);
        }
        *s = 0;
        lastc = c;
        return DIGIT;

    case BREAK:
    case UPPER:
        do {
            if (--n < 0) printAtomStr(tok2long);
            *s++ = (BYTE)c;
            BP_GETC(card, c);
        } while (InType(c) <= LOWER);
        *s = 0;
        lastc = c;
        rtnint = (BPLONG)(s - AtomStr);
        return UPPER;

    case LOWER:
    lab_lower_case:
        do {
            if (--n < 0) printAtomStr(tok2long);
            *s++ = (BYTE)c;
            BP_GETC(card, c);
        } while (InType(c) <= LOWER);
        *s = 0;
    SYMBOL:
#ifdef PICAT
        lastc = c;
        rtnint = (BPLONG)(s - AtomStr);
        return LOWER;
#else
        if (c == '(') {
            BP_GETC(card, lastc);
            rtnint = (BPLONG)(s - AtomStr);
            return BEGIN;
        } else {
            lastc = c;
            rtnint = (BPLONG)(s - AtomStr);
            return LOWER;
        }
#endif

    case SIGN:
    lab_sign_case:
#ifdef BPSOLVER
        if (c == '?') c = '.';
#endif
        *s = (BYTE)c;
        BP_GETC(card, d);
    lab_sign_stream:
        if (c == intab.begcom && d == intab.astcom) {
        ASTCOM:
            if (com2plain(card, d, intab.endcom) == BP_ERROR) return BP_ERROR;
            BP_GETC(card, c);
            goto START;
            /*                  
                                } else if (c == '&' && d == '&') {
                                *s++ = '&';
                                *s++ = '&';
                                *s = '\0';
                                lastc = ' ';
                                rtnint = 2;
                                return LOWER;
            */
        } else if (c == '.') {
            if (d == '.') {  /* .. */
                *++s = (BYTE)d;
                BP_GETC(card, d);
                if (d == '+' || d == '-') {
                    lastc = c = d;
                    *++s = 0;
                    rtnint = (BPLONG)(s - AtomStr);
                    return LOWER;
                }
            }
        } else if (c == '#' && d == '!') {   // e.g., #!=
            if (--n == 0) printAtomStr(tok2long);
            *++s = (BYTE)d;
            BP_GETC(card, d);
        }
        while (InType(d) == SIGN) {   
            if (--n == 0) printAtomStr(tok2long);
            *++s = (BYTE)d;
            BP_GETC(card, d);
        }
        *++s = 0;
        if (InType(d) >= SPACE && c == intab.termin && AtomStr[1] == 0) {
            lastc = d;
            return ENDCL;  /* i.e. '.' FOLLOWed by layout */
        }
        c = d;
        goto SYMBOL;

    case NOBLE:
        if (c == intab.termin) {
            *s = 0;
            lastc = ' ';
            return ENDCL;
#ifndef BPSOLVER
        } else if (eolcom_flag == 1 && c == intab.eolcom) {
            c = com0plain(card, intab.endeol);
            goto START;
#endif
        }
        *s = (BYTE)c;
        BP_GETC(card, d);
        if (d == '='){     // e.g., !=
            goto lab_sign_stream;
        }
        *++s = 0;
        c = d;
        lastc = d;
        goto SYMBOL;

    case PUNCT:
        if (c == intab.termin) {
            *s = 0;
            lastc = ' ';
            return ENDCL;
#ifndef BPSOLVER
        } else if (eolcom_flag == 1 && c == intab.eolcom) {
            c = com0plain(card, intab.endeol);
            goto START;
#endif
        }
        BP_GETC(card, d);
        if (c == intab.begcom && d == intab.astcom)
            goto ASTCOM;
        if (c == '|' && d == '|') {
            *s++ = '|';
            *s++ = '|';
            *s = '\0';
            lastc = ' ';
            rtnint = 2;
            return LOWER;
        }

        /*  If we arrive here, c is an ordinary punctuation mark  */

#ifdef PICAT
#else
        if (c == '(') *s++ = ' ';  /* need to distingusih between atom( and atom ( */
#endif
        lastc = d;
        *s++ = c;
        *s = 0;
        rtnint = (BPLONG)(s - AtomStr);
        return PUNCT;

    case ATMQT:
        while ((d = read_utf8_character(card, c)) >= 0 && bp_exception == (BPLONG)NULL) {
            UTF8_CODEPOINT_TO_STR(d, s, n);
        }
        //              TOKEN_CHECK_EXCEPTION();
        *s = '\0';
        //    rtnint = (BPLONG) (s - AtomStr);
        c = lastc;
        goto SYMBOL;

    case LISQT:
        /* check for potential heap overflow */
        list_head = newpair = heap_top;
        while ((d = read_utf8_character(card, c)) >= 0 && bp_exception == (BPLONG)NULL) {
            if (local_top-heap_top <= LARGE_MARGIN) {
                myquit(STACK_OVERFLOW, "tk");
            }
            heap_top += 2;
            if (double_quotes_flag == 0)  /* codes */
                *newpair++ = MAKEINT(d);
            else {  /* chars */
                s = AtomStr; n = MAX_STR_LEN;
                UTF8_CODEPOINT_TO_STR(d, s, n);
                *s = '\0';
                *newpair++ = ADDTAG(insert_sym((CHAR_PTR)AtomStr, (s-AtomStr), 0), ATM);
            }
            *newpair++ = ADDTAG(heap_top, LST);
        }
        //              TOKEN_CHECK_EXCEPTION();
        if (list_head == heap_top)  /* null string */
            list_p = nil_sym;
        else {
            *(--newpair) = nil_sym;
            list_p = ADDTAG(list_head, LST);
        }
        return LISQT;

    case EOLN: INC_LINE_NO;
    case SPACE:
        BP_GETC(card, c);
        lastc = ' ';
        goto START;

    case EOFCH:
        clearerr(curr_in);
        lastc = ' ';
        return EOFCH;

    case DOLLAR:  /* $ if the next char is lower then $ is treated as LOWER otherwise as SIGN */
#ifdef PICAT
        lastc = ' ';
        *s++ = c;
        *s = 0;
        rtnint = (BPLONG)(s - AtomStr);
        return PUNCT;
#else
        {int next_char_type;
            char next_c;

            BP_GETC(card, next_c);
            next_char_type = InType(next_c);
            while (next_char_type == DOLLAR)  //branch
            {  //branch
                if (--n < 0) printAtomStr(tok2long);  //branch
                *s++ = (BYTE)next_c;  //branch
                BP_GETC(card, next_c);  //branch
                next_char_type = InType(next_c);  //branch
            }  //branch
            BP_UNGETC(next_c, card);
            if (next_char_type == LOWER)
                goto lab_lower_case;
            else
                goto lab_sign_case;
        }
#endif
    }
    bp_exception = c_syntax_error(et_IN_CHARACTER);
    return BP_ERROR;
    /*
      fprintf(stderr, "Internal error: InType(%d)==%d\n", c, InType(c));
      abort();               
    */

end_of_clause:
    AtomStr[0] = '.'; AtomStr[1] = '\0'; lastc = ' '; return ENDCL;

    /*NOTREACHED*/
}

void printAtomStr(message)
    char *message;
{
    /*  AtomStr[100] = '.';AtomStr[101] = '.';AtomStr[102] = '.';AtomStr[103] = '\0'; */
    Fprintf(stderr, "%s\n", AtomStr);
    SyntaxError(message);
}

int GetTokenString()
{
    CHAR_PTR s = AtomStr;
    int c, d;
    BPLONG n = MAX_STR_LEN;
    BPLONG_PTR newpair, list_head;
    BPLONG oldv, newv;

    c = lastc;
START:
    if (c == '\0') goto end_of_clause;
    switch (InType(c)) {
    case DIGIT:
        /* The FOLLOWing kinds of numbers exist:
         * (1) unsigned decimal integers: d+
         * (2) unsigned based integers: d+Ro+[R]
         * (3) unsigned floats: d* [. d*] [e +/-] d+
         * (4) characters: 0Rc[R]
         * (5) binary (0b|0B)..., octal (0o|0O)..., and hexadecimal (0x|0X)...
         * We allow underscores in numbers too, ignoring them. (disallowed, but not completely n.f.)
         */
        if (c == '0') {
            BP_GETC_STRING(d);
            if (d == 'b' || d == 'B' || d == 'o' || d == 'O' || d == 'x' || d == 'X') {  /* binary, octal, hexadecimal*/
                int base;
                if (d == 'b' || d == 'B') base = 2;
                else if (d == 'o' || d == 'O') base = 8;
                else base = 16;
                BP_GETC_STRING(c);  /* first char right after must be a valid digit of the base */
                if (DigVal(c) < 0 || DigVal(c) >= base || c == '_') {
                    bp_exception = c_syntax_error(et_INTEGER);
                    return BP_ERROR;
                }
                newv = 0;
                do {
                    oldv = newv;
                    newv = newv*base + DigVal(c);

                    if (newv < oldv || newv > BP_MAXINT_1W) {  /* begin read bigint */
                        BPLONG big_base = bp_int_to_bigint(base);
                        rad_int = bp_int_to_bigint(oldv);
                        do {
                            rad_int = bp_mul_bigint_bigint(rad_int, big_base);
                            if (rad_int == BP_ERROR) return BP_ERROR;
                            if (ISINT(rad_int)) rad_int = bp_int_to_bigint(INTVAL(rad_int));
                            rad_int = bp_add_bigint_bigint(rad_int, bp_int_to_bigint(DigVal(c)));
                            BP_GETC_STRING(c);
                            while (c == '_') {BP_GETC_STRING(c);}
                        } while (DigVal(c) >= 0 && DigVal(c) < base);
                        if (DigVal(c) < 99) {
                            bp_exception = c_syntax_error(et_INTEGER);
                            return BP_ERROR;
                        }
                        lastc = c;
                        return BIGINT;
                    }  /* end read bigint */

                    BP_GETC_STRING(c);
                    while (c == '_') {BP_GETC_STRING(c);}
                } while (DigVal(c) >= 0 && DigVal(c) < base);
                if (DigVal(c) < 99) {
                    bp_exception = c_syntax_error(et_INTEGER);
                    return BP_ERROR;
                }
                rad_int = newv;
                lastc = c;
                return RDIGIT;
            } else {
                *s++ = c; c = d;
            }
        }
        while (InType(c) == DIGIT || c == '_') {
            if (c != '_') *s++ = c;
            BP_GETC_STRING(c);
        }
        if (c == intab.radix) {
            *s = 0;
            for (d = 0, s = AtomStr; (c = *s++); ) {
                d = d*10-'0'+c;
                if (d > 36) {
                    bp_exception = et_INT_OVERFLOW;
                    return BP_ERROR;
                }
            }
            if (d == 0) {  /*  0'c['] is a character code  */
                d = read_utf8_character_string('\'');
                //                TOKEN_CHECK_EXCEPTION_STRING();
                rad_int = d;
                BP_GETC_STRING(d);
                if (d == intab.radix) {
                    BP_GETC_STRING(lastc);
                } else {
                    lastc = d;
                }
                return RDIGIT;
            }
            BP_GETC_STRING(c);
            newv = 0;
            while (DigVal(c) < 99) {
                if (c != '_') {
                    oldv = newv;
                    if (DigVal(c) >= d) {
                        bp_exception = c_syntax_error(et_INTEGER);
                        return BP_ERROR;
                    }
                    newv = newv*d + DigVal(c);

                    if (newv < oldv || newv > BP_MAXINT_1W) {  /* begin read bigint */
                        BPLONG big_base = bp_int_to_bigint(d);
                        rad_int = bp_int_to_bigint(oldv);
                        while (DigVal(c) < 99) {
                            if (c != '_') {
                                if (DigVal(c) >= d) {
                                    bp_exception = et_INT_OVERFLOW;
                                    return BP_ERROR;
                                }
                                rad_int = bp_mul_bigint_bigint(rad_int, big_base);
                                if (rad_int == BP_ERROR) return BP_ERROR;
                                if (ISINT(rad_int)) rad_int = bp_int_to_bigint(INTVAL(rad_int));
                                rad_int = bp_add_bigint_bigint(rad_int, bp_int_to_bigint(DigVal(c)));
                                //              printf(" read bigint d=%d",DigVal(c)); write_term(rad_int); printf("\n");
                            }
                            BP_GETC_STRING(c);
                        }
                        if (c == intab.radix)
                            BP_GETC_STRING(c);
                        lastc = c;
                        return BIGINT;
                    }  /* end read bigint */

                }
                BP_GETC_STRING(c);
            }
            if (c == intab.radix) {
                BP_GETC_STRING(c);
            }
            rad_int = newv;
            lastc = c;
            return RDIGIT;
        } else if (c == intab.dpoint) {
            BP_GETC_STRING(d);
            if (InType(d) == DIGIT) {
                *s++ = '.';
                do {
                    if (d != '_')
                        *s++ = d;
                    BP_GETC_STRING(d);
                } while (InType(d) <= BREAK);
                if ((d | 32) == 'e') {
                    *s++ = 'E';
                    BP_GETC_STRING(d);
                    if (d == '-') {
                        *s++ = d;
                        BP_GETC_STRING(d);
                    } else if (d == '+')
                        BP_GETC_STRING(d);
                    if (InType(d) > BREAK) {
                        SyntaxError(badexpt);
                        return BP_ERROR;
                    }
                    do {
                        if (d != '_')
                            *s++ = d;
                        BP_GETC_STRING(d);
                    } while (InType(d) <= BREAK);
                }
                c = d;
                *s = 0;
                lastc = c;
                return REALO;
            } else  /* c has not changed */
                BP_UNGETC_STRING;
        }
        *s = 0;
        lastc = c;
        return DIGIT;

    case BREAK:
    case UPPER:
        do {
            if (--n < 0) printAtomStr(tok2long);
            *s++ = c;
            BP_GETC_STRING(c);
        } while (InType(c) <= LOWER);
        *s = 0;
        lastc = c;
        rtnint = (BPLONG)(s - AtomStr);
        return UPPER;

    case LOWER:
    lab_lower_case:
        do {
            if (--n < 0) printAtomStr(tok2long);
            *s++ = c;
            BP_GETC_STRING(c);
        } while (InType(c) <= LOWER);
        *s = 0;
    SYMBOL:
#ifdef PICAT
        lastc = c;
        rtnint = (BPLONG)(s - AtomStr);
        return LOWER;
#else
        if (c == '(') {
            BP_GETC_STRING(lastc);
            rtnint = (BPLONG)(s - AtomStr);
            return BEGIN;
        } else {
            lastc = c;
            rtnint = (BPLONG)(s - AtomStr);
            return LOWER;
        }
#endif

    case SIGN:
    lab_sign_case:
        *s = (char)c;
        BP_GETC_STRING(d);
    lab_sign_string:
        if (c == intab.begcom && d == intab.astcom) {
        ASTCOM:
            if (com2plain_string(d, intab.endcom) == BP_ERROR) return BP_ERROR;
            BP_GETC_STRING(c);
            goto START;
            /*                  
                                } else if (c == '&' && d == '&') {
                                *s++ = '&';
                                *s++ = '&';
                                *s = '\0';
                                lastc = ' ';
                                rtnint = 2;
                                return LOWER;
            */
        } else if (c == '.') {
            if (d == '.') {  /* .. */
                *++s = (BYTE)d;
                BP_GETC_STRING(d);
                if (d == '+' || d == '-') {
                    lastc = c = d;
                    *++s = 0;
                    rtnint = (BPLONG)(s - AtomStr);
                    return LOWER;
                }
            }
        } else if (c == '#' && d == '!') {   // e.g., #!=
            if (--n == 0) printAtomStr(tok2long);
            *++s = (BYTE)d;
            BP_GETC_STRING(d);
        }
        while (InType(d) == SIGN) {
            if (--n == 0) printAtomStr(tok2long);
            *++s = (char)d;
            BP_GETC_STRING(d);
        }
        *++s = 0;
        if (InType(d) >= SPACE && c == intab.termin && AtomStr[1] == 0) {
            lastc = d;
            return ENDCL;  /* i.e. '.' FOLLOWed by layout */
        }
        c = d;
        goto SYMBOL;

    case NOBLE:
        if (c == intab.termin) {
            *s = 0;
            goto end_of_clause;
#ifndef BPSOLVER
        } else if (eolcom_flag == 1 && c == intab.eolcom) {
            c = com0plain_string(intab.endeol);
            goto START;
#endif
        }
        *s = (char)c;
        BP_GETC_STRING(d);
        if (d == '='){     // e.g., !=
            goto lab_sign_string;
        }
        *++s = 0;
        c = d;
        lastc = d;
        goto SYMBOL;

    case PUNCT:
        if (c == intab.termin) {
            *s = 0;
            goto end_of_clause;
#ifndef BPSOLVER
        } else if (eolcom_flag == 1 && c == intab.eolcom) {
            c = com0plain_string(intab.endeol);
            goto START;
#endif
        }
        BP_GETC_STRING(d);
        if (c == intab.begcom && d == intab.astcom)
            goto ASTCOM;
        if (c == '|' && d == '|') {
            *s++ = '|';
            *s++ = '|';
            *s = '\0';
            lastc = ' ';
            rtnint = 2;
            return LOWER;
        }

        /*  If we arrive here, c is an ordinary punctuation mark  */

#ifdef PICAT
#else
        if (c == '(') *s++ = ' ';  /* need to distingusih between atom( and atom ( */
#endif
        lastc = d;
        *s++ = (char)c;
        *s = 0;
        rtnint = (BPLONG)(s - AtomStr);
        return PUNCT;

    case ATMQT:
        while ((d = read_utf8_character_string(c)) >= 0 && bp_exception == (BPLONG)NULL) {
            UTF8_CODEPOINT_TO_STR(d, s, n);
        }
        //  TOKEN_CHECK_EXCEPTION_STRING();
        *s = '\0';
        c = lastc;
        goto SYMBOL;

    case LISQT:
        /* check for potential heap overflow */
        list_head = newpair = heap_top;
        while ((d = read_utf8_character_string(c)) >= 0 && bp_exception == (BPLONG)NULL) {
            if (local_top - heap_top <= LARGE_MARGIN) {
                myquit(STACK_OVERFLOW, "tk");
            }
            heap_top += 2;
            if (double_quotes_flag == 0)  /* codes */
                *newpair++ = MAKEINT(d);
            else {  /* chars */
                s = AtomStr; n = MAX_STR_LEN;
                UTF8_CODEPOINT_TO_STR(d, s, n);
                *s = '\0';
                *newpair++ = ADDTAG(insert_sym((CHAR_PTR)AtomStr, (s-AtomStr), 0), ATM);
            }
            *newpair++ = ADDTAG(heap_top, LST);
        }
        // TOKEN_CHECK_EXCEPTION_STRING();
        if (list_head == heap_top)  /* null string */
            list_p = nil_sym;
        else {
            *(--newpair) = nil_sym;
            list_p = ADDTAG(list_head, LST);
        }
        return LISQT;

    case EOLN:
        goto end_of_clause;
    case SPACE:
        BP_GETC_STRING(c);
        lastc = ' ';
        goto START;

    case EOFCH:
        /*    clearerr(curr_in); */
        lastc = ' ';
        return EOFCH;

    case DOLLAR:  /* $ if the next char is lower the $ is lower otherwise treat $ as SIGN */
#ifdef PICAT
        lastc = ' ';
        *s++ = c;
        *s = 0;
        rtnint = (BPLONG)(s - AtomStr);
        return PUNCT;
#else
        {int next_char_type;
            char next_c;

            BP_GETC_STRING(next_c);
            next_char_type = InType(next_c);
            while (next_char_type == DOLLAR) {
                if (--n < 0) printAtomStr(tok2long);  //branch
                *s++ = (BYTE)next_c;
                BP_GETC_STRING(next_c);
                next_char_type = InType(next_c);
            }  //branch
            BP_UNGETC_STRING;
            if (next_char_type == LOWER)
                goto lab_lower_case;
            else
                goto lab_sign_case;
        }
#endif
    }
    bp_exception = c_syntax_error(et_IN_CHARACTER);
    return BP_ERROR;
    /*
      fprintf(stderr, "Internal error: InType(%d)==%d\n", c, InType(c));
      abort();   
    */
    /*NOTREACHED*/
end_of_clause:
    /* AtomStr[0]='.'; AtomStr[1]='\0';string_in = NULL; lastc =' '; return ENDCL; */
    AtomStr[0] = '.'; AtomStr[1] = '\0'; lastc = ' '; return ENDCL;
}

int b_NEXT_TOKEN_ff(op1, op2)
    BPLONG op1, op2;
{
    FILE *card = curr_in;
    BPLONG i;
    BPLONG len;
    BPLONG oldv, newv;
    BPLONG ptr;

    //  printf("=>next_token %s\n",string_in);
    bp_exception = (BPLONG)NULL;
#ifdef BPSOLVER
#else
    if (next_token_index < MAX_TOKENS_IN_TERM) {
        if (lastc == ' ')
            token_start_pos[next_token_index++] = chars_pool_index;
        else
            token_start_pos[next_token_index++] = chars_pool_index-1;
    }
#endif
    if (string_in == NULL) i = GetToken(); else i = GetTokenString();
    if (bp_exception != (BPLONG)NULL) {
        if (string_in == NULL) {
            char c;
            fprintf(stderr, "*** error until line %d\n", (int)curr_line_no);
            if (i == LISQT){
                picat_str_to_c_str(list_p, chars_pool, MAX_CHARS_IN_POOL);
                chars_pool[100] = '\0';
                fputs(chars_pool,stderr);
            }
            BP_GETC(card, c);
            while (c != -1 && c != '\n' && c != '\r') {
                if (c == '\n') {INC_LINE_NO;}
                BP_GETC(card, c);
            }
            lastc = c;
        }
        return BP_ERROR;
    }
    /*
      printf("next token %i %s\n",i,AtomStr);
      printf("     string_in=%s    %c\n",string_in,lastc);
    */
    switch (i) {
    case LOWER:
        ASSIGN_f_atom(op1, MAKEINT(ATOMO));
        ptr = ADDTAG(insert_sym((CHAR_PTR)AtomStr, rtnint, 0), ATM);
        ASSIGN_f_atom(op2, ptr);
        //    cATOMO++;
        break;
    case BEGIN:
        ASSIGN_f_atom(op1, MAKEINT(FUNC));
        ptr = ADDTAG(insert_sym((CHAR_PTR)AtomStr, rtnint, 0), ATM);
        ASSIGN_f_atom(op2, ptr);
        //    cFUNC++;
#ifdef BPSOLVER
#else
        if (next_token_index < MAX_TOKENS_IN_TERM) {  /* f( has two tokens, actually */
            token_start_pos[next_token_index++] = chars_pool_index;
        }
#endif
        break;
    case UPPER:
        if (AtomStr[0] == '_' && AtomStr[1] == 0) {
            ASSIGN_f_atom(op1, MAKEINT(USCORE));
            //      cUSCORE++;
        } else {
            ASSIGN_f_atom(op1, MAKEINT(VARO));
            //      cVARO++;
        }
        if (rtnint >= MAX_STR_LEN) {
            AtomStr[MAX_STR_LEN-1] = 0;
            rtnint = MAX_STR_LEN;
            printf("*** Name of constant too long: %s\n", AtomStr);
        }
        ptr = ADDTAG(insert_sym((CHAR_PTR)AtomStr, rtnint, 0), ATM);
        ASSIGN_f_atom(op2, ptr);
        break;
    case REALO:
        ASSIGN_sv_heap_term(op2, encodefloat1(atof((char *)AtomStr)));
        ASSIGN_f_atom(op1, MAKEINT(SPECIAL_NUM));
        //    cNUMERO++;
        break;
    case BIGINT:
        ASSIGN_sv_heap_term(op2, rad_int);
        ASSIGN_f_atom(op1, MAKEINT(SPECIAL_NUM));
        //    cNUMERO++;
        break;

    case RDIGIT:
        //    cNUMERO++;
        ASSIGN_f_atom(op1, MAKEINT(INTEGERO));
        ASSIGN_f_atom(op2, MAKEINT(rad_int));
        break;
    case DIGIT:
        newv = len = 0;
        while (AtomStr[len] != 0) {
            oldv = newv;
            newv = newv* 10 + DigVal(AtomStr[len++]);
            if (newv < oldv || newv > BP_MAXINT_1W) {
                BPLONG big_ten = bp_int_to_bigint(10);
                newv = bp_int_to_bigint(oldv);
                len--;
                while (AtomStr[len] != 0) {
                    newv = bp_mul_bigint_bigint(newv, big_ten);
                    if (newv == BP_ERROR) return BP_ERROR;
                    if (ISINT(newv)) newv = bp_int_to_bigint(INTVAL(newv));
                    newv = bp_add_bigint_bigint(newv, bp_int_to_bigint(DigVal(AtomStr[len++])));
                }
                ASSIGN_f_atom(op1, MAKEINT(SPECIAL_NUM));
                ASSIGN_sv_heap_term(op2, newv);
                return BP_TRUE;
            }
        }
        ASSIGN_f_atom(op1, MAKEINT(INTEGERO));
        ASSIGN_f_atom(op2, MAKEINT(newv));
        break;
    case LISQT:
        ASSIGN_f_atom(op1, MAKEINT(STRING));
        ASSIGN_sv_heap_term(op2, list_p);
        //    cSTRING++;
        break;
    case PUNCT:
        /* there are nine punctuation marks, */
        /* ( , )  [ | ]  { ; }  */
        /* % is listed as one, but isn't really. */
        if (AtomStr[0] == ';') {
            ASSIGN_f_atom(op1, MAKEINT(SEMI));
#ifdef PICAT
            ptr = ADDTAG(insert_sym((CHAR_PTR)AtomStr, rtnint, 0), ATM);
            ASSIGN_f_atom(op2, ptr);
#endif
            //      cSEMI++;
        } else {
            ASSIGN_f_atom(op1, MAKEINT(SPECIAL));
            //      cSPECIAL++;
            ptr = ADDTAG(insert_sym((CHAR_PTR)AtomStr, rtnint, 0), ATM);
            ASSIGN_f_atom(op2, ptr);
        }
        break;
    case ENDCL:
        ASSIGN_f_atom(op1, MAKEINT(ENDCLS));
        //    cENDCLS++;
        break;
    case EOFCH:
        ASSIGN_f_atom(op1, MAKEINT(BADEND));
        break;
    default:
        Fprintf(stderr, "Internal error " BPLONG_FMT_STR " %s\n", i, AtomStr);
    }
    //  write_term(op1); write_term(op2); printf("\n");
    return BP_TRUE;
}

int b_SET_FLAG_DOUBLE_QUOTES_c(FlagVal)
    BPLONG FlagVal;
{
    DEREF(FlagVal);
    double_quotes_flag = INTVAL(FlagVal);
    return BP_TRUE;
}


int b_GET_FLAG_DOUBLE_QUOTES_f(FlagVal)
    BPLONG FlagVal;
{
    ASSIGN_f_atom(FlagVal, MAKEINT(double_quotes_flag));
    return BP_TRUE;
}


int c_get_line_nos() {
    BPLONG curr_no, start_no;

    curr_no = ARG(1, 2);
    start_no = ARG(2, 2);

    unify(curr_no, MAKEINT(curr_line_no));
    unify(start_no, MAKEINT(term_start_line_no));
    return 1;
}

int c_set_line_no() {
    BPLONG_PTR top;
    BPLONG curr_no = ARG(1, 1);
    DEREF(curr_no);

    curr_line_no = INTVAL(curr_no);
    return BP_TRUE;
}

int c_init_chars_pool() {
    chars_pool_index = 0;
    next_token_index = 0;
    return BP_TRUE;
}

int c_update_term_start_line_no() {
    term_start_line_no = curr_line_no;
    term_start_pool_index = chars_pool_index;
    while (chars_pool[term_start_pool_index] != '\n' && term_start_pool_index > 0) term_start_pool_index--;  /* back up */
    if (chars_pool[term_start_pool_index] == '\n') term_start_pool_index++;

    return BP_TRUE;
}

int c_report_syntax_error() {
    BPLONG i, j, NTokensBefore, here_out;
    BPLONG char_no = 0;

    NTokensBefore = ARG(1, 1); DEREF(NTokensBefore); NTokensBefore = INTVAL(NTokensBefore);

    //  printf("report_syntax_error %d\n", NTokensBefore);

    if (NTokensBefore > MAX_TOKENS_IN_TERM) return BP_TRUE;

    here_out = 0;
    if (NTokensBefore == 0) here_out = 1;
    for (i = term_start_pool_index; i < chars_pool_index; i++) {
        if (NTokensBefore != 0 && i == token_start_pos[NTokensBefore]) {
#ifndef WIN32
            fputs("\033\[31m <<HERE>> \033\[39m\n", stderr);
#else
            fputs(" <<HERE>> \n", stderr);
#endif
            for (j = 0; j < char_no; j++) fputc(' ', stderr);
            here_out = 1;
        }
        fputc(chars_pool[i], stderr);
        if (chars_pool[i] == '\n') char_no = 0; else char_no++;
    }
    if (here_out == 0) fputs("<<HERE>>", stderr);

    fprintf(stderr, "\n\n");
    return BP_TRUE;
}



