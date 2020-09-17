/********************************************************************
 *   File   : kapi.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2020

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

/* Test Prolog terms in C */

#define PisVar(term) ISREF(term)

#define PisAtom(term) ISATOM(term)

#define PisInt(term) ISINT(term)

#define PisLong(term) 0

#define PisReal(term) ISFLOAT(term)

#define PisList(term) ISLIST(term)

#define PisEndList(term) ISNIL(term)

#define PisAddr(term) (ISSTRUCT(term) && FOLLOW(UNTAGGED_ADDR(term)) == (BPLONG)objectRef)

#define PisCompound(term) ISSTRUCT(term)

#define PvalueOfInt(term) INTVAL(term)








