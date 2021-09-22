#ifndef _reduce_h_INCLUDED
#define _reduce_h_INCLUDED

#include "kissat_bool.h"

struct kissat;

bool kissat_reducing (struct kissat *);
int kissat_reduce (struct kissat *);

#endif
