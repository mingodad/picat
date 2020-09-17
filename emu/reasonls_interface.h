#pragma once
#include "bprolog.h"


#ifdef __cplusplus
extern "C" {
#endif
    void maple_init();
    void maple_add_lit(int lit0);
    int maple_start_solver();
    int maple_get_binding(int varNum);


#ifdef __cplusplus
}
#endif
