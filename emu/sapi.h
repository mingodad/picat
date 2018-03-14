/********************************************************************
 *   File   : sapi.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

/* SICStus Prolog APIs */

#define SP_ERROR -1
#define SP_FAILURE 0
#define SP_SUCCESS 1

#define SP_TYPE_VARIABLE BP_TYPE_VARIABLE
#define SP_TYPE_INTEGER BP_TYPE_INTEGER
#define SP_TYPE_FLOAT BP_TYPE_FLOAT
#define SP_TYPE_ATOM BP_TYPE_ATOM 
#define SP_TYPE_COMPOUND BP_TYPE_COMPOUND

int SP_errno;


#define SP_term_ref BP_term_ref

#define SP_pred_ref BP_pred_ref

extern SP_pred_ref SP_predicate();
extern SP_pred_ref SP_pred();
extern SP_term_ref SP_new_term_ref();
extern char *SP_string_from_atom();
extern char *SP_alloc();






