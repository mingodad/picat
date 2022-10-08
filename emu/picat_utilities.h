#ifndef PICAT_UTILITIES
#define PICAT_UTILITIES

#include "picat.h"

TERM cstring_to_picat(char* ch_ptr);

char* picat_string_to_cstring(TERM t);

TERM picat_get_list_end(TERM l);

TERM new_picat_map(int C);

void add_to_picat_map(TERM m, TERM key, TERM value);

TERM picat_map_get(TERM map, TERM key);

#endif
