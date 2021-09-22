#ifndef _backward_h_INCLUDED
#define _backward_h_INCLUDED

#include "reference.h"

#include "kissat_bool.h"

struct kissat;

bool kissat_backward_subsume_temporary (struct kissat *, reference ignore);

#endif
