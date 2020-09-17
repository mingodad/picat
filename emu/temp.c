/* Const-X = Y, X is a bit-vector domain */
int c_CLPFD_SUB_AC_ccc() {
    BPLONG Const, X, Y, i, sizeY;
    BPLONG_PTR dv_ptr_x, dv_ptr_y;
    BPLONG elmX, maxX, elmY, minY, maxY;

    Const = ARG(1, 3); DEREF_NONVAR(Const); Const = INTVAL(Const);
    X = ARG(2, 3); DEREF_NONVAR(X);
    Y = ARG(3, 3); DEREF_NONVAR(Y);

    //  printf("=> SUB_AC "); write_term(Const); printf(" "); write_term(X); printf(" "); write_term(Y); printf("\n");

    if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
    minY = DV_first(dv_ptr_y);
    maxY = DV_last(dv_ptr_y);
    sizeY = maxY-minY+1;
    if (local_top - heap_top <= sizeY || DV_size(dv_ptr_y) > 512) {
        return BP_TRUE;  /* do not enforce AC on Z*/
    }
    ptr = local_top-sizeY;
    for (i = 1; i < sizeY-1; i++) {
        *(ptr+i) = 0;  /* 0 means unsupported, minY and maxY are supported */
    }
    dv_ptr_x = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(X);
    elmX = DV_first(dv_ptr_x);
    maxX = DV_last(dv_ptr_x);
    for (; ; ) {  /* iterate over X */
        *(ptr+(Const-elmX-minY)) = 1;  /* Const-elmX is supported in Y */
        if (elmX == maxX) break;  /* exit loop of X */
        elmX++;
        elmX = domain_next_bv(dv_ptr_x, elmX);
    }

    elmY = minY+1;
    for (i = 1; i < sizeY-1; i++) {
        if (*(ptr+i) == 0) {  /* elmY is unsupported */
            domain_set_false_noint(dv_ptr_y, elmY);
        }
        elmY++;
    }
    //  printf("<= SUB_AC "); write_term(Const); printf(" "); write_term(X); printf(" "); write_term(Y); printf("\n");

    return BP_TRUE;
}
