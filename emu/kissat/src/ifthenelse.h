#ifndef _ifthenelse_h_INCLUDED
#define _ifthenelse_h_INCLUDED

#include "kissat_bool.h"

struct kissat;

bool kissat_find_if_then_else_gate (struct kissat *,
				    unsigned lit, unsigned negative);

#endif
