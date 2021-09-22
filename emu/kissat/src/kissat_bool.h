#ifndef kissat_bool_h_INCLUDED
#define kissat_bool_h_INCLUDED
#ifdef WIN32
typedef unsigned bool;
#define false 0
#define true 1
#else
#include <stdbool.h>
#endif
#endif // kissat_bool