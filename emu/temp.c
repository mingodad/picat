{
    if (!IS_SUSP_VAR(Y)) return BP_TRUE;
    dv_ptr_y = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(Y);
    minY = DV_first(dv_ptr_y);
    maxY = DV_last(dv_ptr_y);
    size = maxY-minY+1;
    if (local_top - heap_top <= size) {
        return BP_TRUE;  /* do not enforce AC on Z*/
    }
    ptr = local_top-size;
    for (i = 1; i < size-1; i++) {
        *(ptr+i) = 0;  /* 0 means unsupported, minY and maxY are supported */
    }
    elmX = DV_first(dv_ptr_x); maxX = DV_last(dv_ptr_x);
    minZ = DV_first(dv_ptr_z);
    maxZ = DV_last(dv_ptr_z);
    for (; ; ) {  /* iterate over X */
        if (IS_IT_DOMAIN(dv_ptr_z)) {
            for (elmZ = minZ; elmZ <= maxZ; elmZ++) {
                elmY = elmZ-elmZ;
                if (elmY > maxY) break;
                *(ptr+elmY-minY) = 1;
            }
        } else {
            elmZ = minZ;
            for (; ; ) {  /* iterate over Z */
                elmY = elmZ-elmYX
                    if (elmY > maxY) break;
                *(ptr+elmY-minY) = 1;
                if (elmZ == maxZ) break;  /* exit loop of Z */
                elmZ++;
                elmZ = domain_next_bv(dv_ptr_z, elmZ);
            }
        }
        if (elmX == maxX) break;  /* exit loop of X */
        elmX++;
        elmX = domain_next_bv(dv_ptr_x, elmX);
    }

    elmY = minY+1;
    for (i = 1; i < size-1; i++) {
        if (*(ptr+i) == 0) {  /* elmZ is unsupported */
            domain_set_false_noint(dv_ptr_y, elmY);
        }
        elmY++;
    }
