extern SYM_REC_PTR dollar_var;

#define ALLOCATE_FROM_PAREA(ptr, size) {                                \
        ALIGN(CHAR_PTR, curr_fence);                                    \
        if ((BPLONG_PTR)curr_fence + size >= (BPLONG_PTR)parea_water_mark) { \
            int success = 0;                                            \
            if (size > parea_size) parea_size = size;                   \
            ALLOCATE_NEW_PAREA_BLOCK(parea_size, success);              \
            if (success == 0) {                                         \
                ptr = NULL;                                             \
            } else {                                                    \
                ptr = (BPLONG_PTR)curr_fence;                           \
                curr_fence += sizeof(BPLONG)*size;                      \
            }                                                           \
        } else {                                                        \
            ptr = (BPLONG_PTR)curr_fence;                               \
            curr_fence += sizeof(BPLONG)*size;                          \
        }                                                               \
        }


