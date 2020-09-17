/********************************************************************
 *   File   : main.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "bprolog.h"
int main(int argc, char *argv[]);
static INLINE int bprolog_main(int argc, char *argv[]);

int bprolog_main(argc, argv)
    int argc;
    char *argv[];
{
    initialize_bprolog(argc, argv);
    if (disassem) {
        dis();
        printf("The byte code file is dumped in the file dump.pil\n");
        return (0);
    }
    else {
#ifdef PICAT
        if (main_args != nil_sym) {
            use_gl_getline = 0;
        }
#endif
        bp_call_term(ADDTAG(insert_sym("$bp_first_call", 14, 0), ATM));
        // toam(inst_begin,arreg,local_top);
        return (0);
    }
}

int main(argc, argv)
    int argc;
    char *argv[];
{
    return bprolog_main(argc, argv);
}






