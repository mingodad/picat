/********************************************************************
 *   File   : clpfd.h
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2018

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "frame.h"

#define SOUND_C_MUL_U(a,u)  ((u>=BP_MAXINT_1W) ? BP_MAXINT_1W : a*u)
#define SOUND_C_MUL_L(a,l)  ((l<=BP_MININT_1W) ? BP_MININT_1W : a*l)

#define SOUND_C_ADD_U(a,u)  ((u >= BP_MAXINT_1W) ? BP_MAXINT_1W : a+u)
#define SOUND_C_ADD_L(a,l)  ((l <= BP_MININT_1W) ? BP_MININT_1W : a+l)
#define SOUND_U_ADD_U(u1,u2)  ((u1>=BP_MAXINT_1W || u2 >= BP_MAXINT_1W) ? BP_MAXINT_1W : u1+u2)
#define SOUND_L_ADD_L(l1,l2)  ((l1<=BP_MININT_1W || l2 <= BP_MININT_1W) ? BP_MININT_1W : l1+l2)

#define SOUND_L_SUB_U(l,u)  ((l<=BP_MININT_1W || u >= BP_MAXINT_1W) ? BP_MININT_1W : l-u)
#define SOUND_U_SUB_L(u,l)  ((l<=BP_MININT_1W || u >= BP_MAXINT_1W) ? BP_MAXINT_1W : u-l)
#define SOUND_U_SUB_C(u,a)  ((u >= BP_MAXINT_1W) ? BP_MAXINT_1W : u-a)
#define SOUND_L_SUB_C(l,a)  ((l <= BP_MININT_1W) ? BP_MININT_1W : l-a)

#define SOUND_C_SUB_L(a,l)  ((l <= BP_MININT_1W) ? BP_MAXINT_1W : a-l)
#define SOUND_C_SUB_U(a,u)  ((u >= BP_MAXINT_1W) ? BP_MININT_1W : a-u)

/* a must be positive */
#define SOUND_UP_DIV_L(V,X,a){					\
    if (X<=BP_MININT_1W){							\
	  V = BP_MININT_1W;							\
    } else {									\
	  UP_DIV(V,X,a);							\
    }											\
  }

/* a must be positive */
#define SOUND_LOW_DIV_U(V,X,a){					\
    if (X>=BP_MAXINT_1W){							\
	  V = BP_MAXINT_1W;							\
    } else {									\
	  LOW_DIV(V,X,a);							\
    }											\
  }

#define IS_SMALL_DOMAIN(dv_ptr) (DV_first(dv_ptr)>=fd_region_low && DV_last(dv_ptr)<=fd_region_up)

#define IS_SMALL_DOMAIN2(min,max) (min>=fd_region_low && max <=fd_region_up)

#define CALL_COUNT_DOMAIN_ELMS(dv_ptr,from,to) (from>to ? 0 : count_domain_elms(dv_ptr,from,to))

#define CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,from,to) (from>to ? 0 : exclude_domain_elms(dv_ptr,from,to))


#define RETURN_0 return 0

#define RETURN_1 return 1

#define CALL_DOMAIN_REGION(dv_ptr,min,max){				\
    if (min>max) RETURN_0;								\
    if (min > dv_ptr_first) {							\
	  if (max < dv_ptr_last) {							\
		CALL_DOMAIN_REGION_MIN_MAX(dv_ptr,min,max);}	\
	  else {											\
		CALL_DOMAIN_REGION_MIN(dv_ptr,min);}			\
    } else {											\
	  if (max < dv_ptr_last) {							\
		CALL_DOMAIN_REGION_MAX(dv_ptr,max);}			\
	}													\
  }

#define CALL_DOMAIN_REGION_MIN_MAX(dv_ptr,min,max){						\
	if (min==max){														\
	  if (IS_BV_DOMAIN(dv_ptr) && !dm_true(dv_ptr,min)) RETURN_0;		\
	  dv_ptr_first = min; dv_ptr_last = max;							\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(min));									\
	} else if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=dv_ptr_first;k<min;k++){									\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
        for (k=max+1;k<=dv_ptr_last;k++){								\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = max-min+1;													\
	  UPDATE_FIRST_LAST_SIZE(dv_ptr,dv_ptr_first,min,dv_ptr_last,max,size); \
	  INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
	  dv_ptr_first = min; dv_ptr_last = max;							\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);	\
		size += CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);	\
		size = DV_size(dv_ptr)-size-2;									\
		if (size>1) {													\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_first));		\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_last));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);		\
		size += CALL_COUNT_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);		\
		size = DV_size(dv_ptr)-size-2;									\
	  }																	\
	  if (size == 0) return 0;											\
	  min = domain_next_bv(dv_ptr,min);									\
	  if (size == 1){													\
		dv_ptr_first = min; dv_ptr_last = min;							\
		ASSIGN_DVAR(dv_ptr,MAKEINT(min));								\
	  } else {															\
		max = domain_prev_bv(dv_ptr,max);								\
		UPDATE_FIRST_LAST_SIZE(dv_ptr,dv_ptr_first,min,dv_ptr_last,max,size); \
		INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
		dv_ptr_first = min; dv_ptr_last = max;							\
	  }																	\
	}																	\
  }

#define CALL_DOMAIN_REGION_MIN(dv_ptr,min){								\
	if (min >= dv_ptr_last) {											\
	  if (min>dv_ptr_last) RETURN_0;									\
	  dv_ptr_first = min;												\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(min));									\
	} else if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=dv_ptr_first;k<min;k++){									\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = dv_ptr_last-min+1;											\
	  UPDATE_FIRST_SIZE(dv_ptr,dv_ptr_first,min,size);					\
	  INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
	  dv_ptr_first = min;												\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);	\
		size = DV_size(dv_ptr)-size-1;									\
		if (size>1) {													\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_first));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);		\
		size = DV_size(dv_ptr)-size-1;									\
	  }																	\
	  if (size == 1){													\
		dv_ptr_first = dv_ptr_last;										\
		ASSIGN_DVAR(dv_ptr,MAKEINT(dv_ptr_last));						\
	  } else {															\
		min = domain_next_bv(dv_ptr,min);								\
		UPDATE_FIRST_SIZE(dv_ptr,dv_ptr_first,min,size);				\
		INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
		dv_ptr_first = min;												\
	  }																	\
	}																	\
  }

#define CALL_DOMAIN_REGION_MAX(dv_ptr,max){								\
	if (max <= dv_ptr_first){											\
	  if (max < dv_ptr_first) RETURN_0;									\
	  dv_ptr_last = max;												\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(max));									\
	} else  if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=max+1;k<=dv_ptr_last;k++){								\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = max-dv_ptr_first+1;										\
	  UPDATE_LAST_SIZE(dv_ptr,dv_ptr_last,max,size);					\
	  INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
	  dv_ptr_last = max;												\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);	\
		size = DV_size(dv_ptr)-size-1;									\
		if (size>1) {													\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_last));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);		\
		size = DV_size(dv_ptr)-size-1;									\
	  }																	\
      if (size == 1){													\
        dv_ptr_last = dv_ptr_first;										\
        ASSIGN_DVAR(dv_ptr,MAKEINT(dv_ptr_first));						\
	  } else {															\
        max = domain_prev_bv(dv_ptr,max);								\
        UPDATE_LAST_SIZE(dv_ptr,dv_ptr_last,max,size);					\
        INSERT_TRIGGER_minmax_checkseed(dv_ptr,arreg);					\
        dv_ptr_last = max;												\
	  }																	\
	}																	\
  }

#define CALL_DOMAIN_REGION_noseed(dv_ptr,min,max){			\
    if (min>max) RETURN_0;									\
    if (min > dv_ptr_first) {								\
	  if (max < dv_ptr_last) {								\
		CALL_DOMAIN_REGION_MIN_MAX_noseed(dv_ptr,min,max);} \
	  else {												\
		CALL_DOMAIN_REGION_MIN_noseed(dv_ptr,min);}			\
    } else {												\
	  if (max < dv_ptr_last) {								\
		CALL_DOMAIN_REGION_MAX_noseed(dv_ptr,max);}			\
	}														\
  }

#define CALL_DOMAIN_REGION_MIN_MAX_noseed(dv_ptr,min,max){				\
	if (min==max){														\
	  if (IS_BV_DOMAIN(dv_ptr) && !dm_true(dv_ptr,min)) RETURN_0;		\
	  dv_ptr_first = min; dv_ptr_last = max;							\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(min));									\
	} else if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=dv_ptr_first;k<min;k++){									\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
        for (k=max+1;k<=dv_ptr_last;k++){								\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = max-min+1;													\
	  UPDATE_FIRST_LAST_SIZE(dv_ptr,dv_ptr_first,min,dv_ptr_last,max,size); \
	  INSERT_TRIGGER_minmax(dv_ptr);									\
	  dv_ptr_first = min; dv_ptr_last = max;							\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);	\
		size += CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);	\
		size = DV_size(dv_ptr)-size-2;									\
		if (size>1) {													\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_first));		\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_last));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);		\
		size += CALL_COUNT_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);		\
		size = DV_size(dv_ptr)-size-2;									\
	  }																	\
	  if (size == 0) return 0;											\
	  min = domain_next_bv(dv_ptr,min);									\
	  if (size == 1){													\
		dv_ptr_first = min; dv_ptr_last = min;							\
		ASSIGN_DVAR(dv_ptr,MAKEINT(min));								\
	  } else {															\
		max = domain_prev_bv(dv_ptr,max);								\
		UPDATE_FIRST_LAST_SIZE(dv_ptr,dv_ptr_first,min,dv_ptr_last,max,size); \
		INSERT_TRIGGER_minmax(dv_ptr);									\
		dv_ptr_first = min; dv_ptr_last = max;							\
	  }																	\
	}																	\
  }

#define CALL_DOMAIN_REGION_MIN_noseed(dv_ptr,min){						\
	if (min >= dv_ptr_last) {											\
	  if (min>dv_ptr_last) RETURN_0;									\
	  dv_ptr_first = min;												\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(min));									\
	} else if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=dv_ptr_first;k<min;k++){									\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = dv_ptr_last-min+1;											\
	  UPDATE_FIRST_SIZE(dv_ptr,dv_ptr_first,min,size);					\
	  INSERT_TRIGGER_minmax(dv_ptr);									\
	  dv_ptr_first = min;												\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);	\
		size = DV_size(dv_ptr)-size-1;									\
		if (size>1) {													\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_first));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,dv_ptr_first+1,min-1);		\
		size = DV_size(dv_ptr)-size-1;									\
	  }																	\
	  if (size == 1){													\
		dv_ptr_first = dv_ptr_last;										\
		ASSIGN_DVAR(dv_ptr,MAKEINT(dv_ptr_last));						\
	  } else {															\
		min = domain_next_bv(dv_ptr,min);								\
		UPDATE_FIRST_SIZE(dv_ptr,dv_ptr_first,min,size);				\
		INSERT_TRIGGER_minmax(dv_ptr);									\
		dv_ptr_first = min;												\
	  }																	\
	}																	\
  }

#define CALL_DOMAIN_REGION_MAX_noseed(dv_ptr,max){						\
	if (max <= dv_ptr_first){											\
	  if (max < dv_ptr_first) RETURN_0;									\
	  dv_ptr_last = max;												\
	  ASSIGN_DVAR(dv_ptr,MAKEINT(max));									\
	} else  if (IS_IT_DOMAIN(dv_ptr)){									\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
        for (k=max+1;k<=dv_ptr_last;k++){								\
		  INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(k));					\
        }																\
	  }																	\
	  size = max-dv_ptr_first+1;										\
	  UPDATE_LAST_SIZE(dv_ptr,dv_ptr_last,max,size);					\
	  INSERT_TRIGGER_minmax(dv_ptr);									\
	  dv_ptr_last = max;												\
	} else {															\
	  if (DV_outer_dom_cs(dv_ptr)!=nil_sym){							\
		size = CALL_EXCLUDE_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);	\
		size = DV_size(dv_ptr)-size-1;									\
		if (size>1) {													\
          INSERT_TRIGGER_outer_dom0(dv_ptr,MAKEINT(dv_ptr_last));		\
		}																\
	  } else {															\
		size = CALL_COUNT_DOMAIN_ELMS(dv_ptr,max+1,dv_ptr_last-1);		\
		size = DV_size(dv_ptr)-size-1;									\
	  }																	\
	  if (size == 1){													\
        dv_ptr_last = dv_ptr_first;										\
        ASSIGN_DVAR(dv_ptr,MAKEINT(dv_ptr_first));						\
	  } else {															\
        max = domain_prev_bv(dv_ptr,max);								\
        UPDATE_LAST_SIZE(dv_ptr,dv_ptr_last,max,size);					\
        INSERT_TRIGGER_minmax(dv_ptr);									\
        dv_ptr_last = max;												\
	  }																	\
	}																	\
  }

/* t2 = t1+a*x  (a>0) */
#define COMPUTE_LOW_UPPER_BOUNDS_p(a,x,l1,u1,l2,u2){	\
	if (IS_SUSP_VAR(x)){								\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);		\
      first = DV_first(dv_ptr);							\
      last = DV_last(dv_ptr);							\
      temp_i = SOUND_C_MUL_L(a,first);					\
      l2 = SOUND_L_ADD_L(l1,temp_i);					\
      temp_i = SOUND_C_MUL_U(a,last);					\
      u2 = SOUND_U_ADD_U(u1,temp_i);					\
	} else {											\
      x = a*INTVAL(x);									\
      l2 = SOUND_C_ADD_L(x,l1);							\
      u2 = SOUND_C_ADD_U(x,u1);							\
	}													\
  }

/* t2 = t1+a*x  (a>0) */
#define COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p(a,x,l1,u1,l2,u2){ \
	if (IS_SUSP_VAR(x)){										\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);				\
      first = DV_first(dv_ptr);									\
      last = DV_last(dv_ptr);									\
      l2 = l1+a*first;											\
      u2 = u1+a*last;											\
	} else {													\
      x = a*INTVAL(x);											\
      l2 = l1+x;												\
      u2 = u1+x;												\
	}															\
  }

/* t2 = t1-a*x  (a>0) */
#define COMPUTE_LOW_UPPER_BOUNDS_m(a,x,l1,u1,l2,u2){	\
	if (IS_SUSP_VAR(x)){								\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);		\
      first = DV_first(dv_ptr);							\
      last = DV_last(dv_ptr);							\
      temp_i = SOUND_C_MUL_U(a,last);					\
      l2 = SOUND_L_SUB_U(l1,temp_i);					\
      temp_i = SOUND_C_MUL_L(a,first);					\
      u2 = SOUND_U_SUB_L(u1,temp_i);					\
	} else {											\
      x = a*INTVAL(x);									\
      l2 = SOUND_L_SUB_C(l1,x);							\
      u2 = SOUND_U_SUB_C(u1,x);							\
	}													\
  }

/* t2 = t1-a*x  (a>0) */
#define COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m(a,x,l1,u1,l2,u2){ \
	if (IS_SUSP_VAR(x)){										\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);				\
      first = DV_first(dv_ptr);									\
      last = DV_last(dv_ptr);									\
      l2 = l1-a*last;											\
      u2 = u1-a*first;											\
	} else {													\
      x = a*INTVAL(x);											\
      l2 = l1-x;												\
      u2 = u1-x;												\
	}															\
  }

/* t2 = t1+x */
#define COMPUTE_LOW_UPPER_BOUNDS_p1(x,l1,u1,l2,u2){ \
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      first = DV_first(dv_ptr);						\
      last = DV_last(dv_ptr);						\
      l2 = SOUND_L_ADD_L(l1,first);					\
      u2 = SOUND_U_ADD_U(u1,last);					\
	} else {										\
      x = INTVAL(x);								\
      l2 = SOUND_C_ADD_L(x,l1);						\
      u2 = SOUND_C_ADD_U(x,u1);						\
	}												\
  }

/* t2 = t1+x */
#define COMPUTE_LOW_UPPER_BOUNDS_nooverflow_p1(x,l1,u1,l2,u2){	\
	if (IS_SUSP_VAR(x)){										\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);				\
      first = DV_first(dv_ptr);									\
      last = DV_last(dv_ptr);									\
      l2 = l1+first;											\
      u2 = u1+last;												\
	} else {													\
      x = INTVAL(x);											\
      l2 = l1+x;												\
      u2 = u1+x;												\
	}															\
  }

/* t2 = t1-x */
#define COMPUTE_LOW_UPPER_BOUNDS_m1(x,l1,u1,l2,u2){ \
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      first = DV_first(dv_ptr);						\
      last = DV_last(dv_ptr);						\
      l2 = SOUND_L_SUB_U(l1,last);					\
      u2 = SOUND_U_SUB_L(u1,first);					\
	} else {										\
      x = INTVAL(x);								\
      l2 = SOUND_L_SUB_C(l1,x);						\
      u2 = SOUND_U_SUB_C(u1,x);						\
	}												\
  }

/* t2 = t1-x */
#define COMPUTE_LOW_UPPER_BOUNDS_nooverflow_m1(x,l1,u1,l2,u2){	\
	if (IS_SUSP_VAR(x)){										\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);				\
      first = DV_first(dv_ptr);									\
      last = DV_last(dv_ptr);									\
      l2 = l1-last;												\
      u2 = u1-first;											\
	} else {													\
      x = INTVAL(x);											\
      l2 = l1-x;												\
      u2 = u1-x;												\
	}															\
  }

/*
  t2 = t1+a*x (a>0) 
  min(t2)-max(a*x)<= t1 <= max(t2)-min(a*x) 
  min(t2)-max(t1)<= a*x <= max(t2)-min(t1) 
*/
#define REDUCE_DOMAIN_p(a,x,l1,u1,l2,u2){			\
	first = SOUND_L_SUB_U(l2,u1);					\
	last = SOUND_U_SUB_L(u2,l1);					\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      SOUND_UP_DIV_L(first0,first,a);				\
      SOUND_LOW_DIV_U(last0,last,a);				\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);		\
      temp_i = SOUND_C_MUL_U(a,dv_ptr_last);		\
      first = SOUND_L_SUB_U(l2,temp_i);				\
      temp_i = SOUND_C_MUL_L(a,dv_ptr_first);		\
      last = SOUND_U_SUB_L(u2,temp_i);				\
    } else {										\
      x = a*INTVAL(x);								\
      if (x < first || x > last) RETURN_0;			\
      first = SOUND_L_SUB_C(l2,x);					\
      last = SOUND_U_SUB_C(u2,x);					\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  }

/*
  t2 = t1+a*x (a>0) 
  min(t2)-max(a*x)<= t1 <= max(t2)-min(a*x) 
  min(t2)-max(t1)<= a*x <= max(t2)-min(t1) 
*/
#define REDUCE_DOMAIN_nooverflow_p(a,x,l1,u1,l2,u2){	\
	first = l2-u1;										\
	last = u2-l1;										\
	if (IS_SUSP_VAR(x)){								\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);		\
      dv_ptr_first = DV_first(dv_ptr);					\
      dv_ptr_last = DV_last(dv_ptr);					\
      UP_DIV(first0,first,a);							\
      LOW_DIV(last0,last,a);							\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);			\
      first = l2-a*dv_ptr_last;							\
      last = u2-a*dv_ptr_first;							\
    } else {											\
      x = a*INTVAL(x);									\
      if (x < first || x > last) RETURN_0;				\
      first = l2-x;										\
      last = u2-x;										\
	}													\
	if (first<=l1) {									\
	  if (last >= u1) RETURN_1;							\
	  u1 = last;										\
	} else {											\
	  l1 = first;										\
	  if (last<u1) u1=last;								\
	}													\
	if (u1<l1) RETURN_0;								\
  }

/*
  0 = t1+a*x (a>0) 
  -max(t1)<= a*x <= -min(t1) 
  -max(a*x)<= t1 <= -min(a*x) 
*/
#define REDUCE_LAST_TERM_DOMAIN_p(a,x,l1,u1){		\
	first = -u1;									\
	last = -l1;										\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      SOUND_UP_DIV_L(first0,first,a);				\
      SOUND_LOW_DIV_U(last0,last,a);				\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);		\
      temp_i = SOUND_C_MUL_U(a,dv_ptr_last);		\
      first = -temp_i;								\
      temp_i = SOUND_C_MUL_L(a,dv_ptr_first);		\
      last = -temp_i;								\
    } else {										\
      x = a*INTVAL(x);								\
      if (x < first || x > last) RETURN_0;			\
      first = last = -x;							\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  }

/*
  0 = t1+a*x (a>0) 
  -max(t1)<= a*x <= -min(t1) 
  -max(a*x)<= t1 <= -min(a*x) 
*/
#define REDUCE_LAST_TERM_DOMAIN_nooverflow_p(a,x,l1,u1){	\
	first = -u1;											\
	last = -l1;												\
	if (IS_SUSP_VAR(x)){									\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);			\
      dv_ptr_first = DV_first(dv_ptr);						\
      dv_ptr_last = DV_last(dv_ptr);						\
      UP_DIV(first0,first,a);								\
      LOW_DIV(last0,last,a);								\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);				\
      first = -a*dv_ptr_last;								\
      last = -a*dv_ptr_first;								\
    } else {												\
      x = a*INTVAL(x);										\
      if (x < first || x > last) RETURN_0;					\
      first = last = -x;									\
	}														\
	if (first<=l1) {										\
	  if (last >= u1) RETURN_1;								\
	  u1 = last;											\
	} else {												\
	  l1 = first;											\
	  if (last<u1) u1=last;									\
	}														\
	if (u1<l1) RETURN_0;									\
  }


/* 
   t2 = t1+x 
   min(t2)-max(x)<= t1 <= max(t2)-min(x) 
   min(t2)-max(t1)<= x <= max(t2)-min(t1) 
*/
#define REDUCE_DOMAIN_p1(x,l1,u1,l2,u2){			\
	first = SOUND_L_SUB_U(l2,u1);					\
	last = SOUND_U_SUB_L(u2,l1);					\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = SOUND_L_SUB_U(l2,dv_ptr_last);		\
      last = SOUND_U_SUB_L(u2,dv_ptr_first);		\
    } else {										\
      x = INTVAL(x);								\
      if (x < first || x > last) RETURN_0;			\
      first = SOUND_L_SUB_C(l2,x);					\
      last = SOUND_U_SUB_C(u2,x);					\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  }

/* 
   t2 = t1+x 
   min(t2)-max(x)<= t1 <= max(t2)-min(x) 
   min(t2)-max(t1)<= x <= max(t2)-min(t1) 
*/
#define REDUCE_DOMAIN_nooverflow_p1(x,l1,u1,l2,u2){ \
	first = l2-u1;									\
	last = u2-l1;									\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = l2-dv_ptr_last;						\
      last = u2-dv_ptr_first;						\
    } else {										\
      x = INTVAL(x);								\
      if (x < first || x > last) RETURN_0;			\
      first = l2-x;									\
      last = u2-x;									\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  }

/* 
   0 = t1+x 
   -max(x)<= t1 <= -min(x) 
   -max(t1)<= x <= -min(t1) 
*/
#define REDUCE_LAST_TERM_DOMAIN_p1(x,l1,u1){		\
	first = -u1;									\
	last = -l1;										\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = -dv_ptr_last;							\
      last = -dv_ptr_first;							\
    } else {										\
      x = INTVAL(x);								\
      if (x < first || x > last) RETURN_0;			\
      first = last = -x;							\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  }


/* 
   t2 = t1-a*x 
   min(t2)+a*min(x)<= t1 <= max(t2)+a*max(x) 
   min(t1)-max(t2)<= a*x <= max(t1)-min(t2) 
*/
#define REDUCE_DOMAIN_m(a,x,l1,u1,l2,u2){			\
	first = SOUND_L_SUB_U(l1,u2);					\
	last = SOUND_U_SUB_L(u1,l2);					\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      SOUND_UP_DIV_L(first0,first,a);				\
      SOUND_LOW_DIV_U(last0,last,a);				\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);		\
      temp_i = SOUND_C_MUL_L(a,dv_ptr_first);		\
      first = SOUND_L_ADD_L(l2,temp_i);				\
      temp_i = SOUND_C_MUL_U(a,dv_ptr_last);		\
      last = SOUND_U_ADD_U(u2,temp_i);				\
	} else {										\
      x = a*INTVAL(x);								\
      if (x<first || x> last) RETURN_0;				\
      first = SOUND_C_ADD_L(x,l2);					\
      last = SOUND_C_ADD_U(x,u2);					\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  } 


/* 
   t2 = t1-a*x 
   min(t2)+a*min(x)<= t1 <= max(t2)+a*max(x) 
   min(t1)-max(t2)<= a*x <= max(t1)-min(t2) 
*/
#define REDUCE_DOMAIN_nooverflow_m(a,x,l1,u1,l2,u2){	\
	first = l1-u2;										\
	last = u1-l2;										\
	if (IS_SUSP_VAR(x)){								\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);		\
      dv_ptr_first = DV_first(dv_ptr);					\
      dv_ptr_last = DV_last(dv_ptr);					\
      UP_DIV(first0,first,a);							\
      LOW_DIV(last0,last,a);							\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);			\
      first = l2+a*dv_ptr_first;						\
      last = u2+a*dv_ptr_last;							\
	} else {											\
      x = a*INTVAL(x);									\
      if (x<first || x> last) RETURN_0;					\
      first = x+l2;										\
      last = x+u2;										\
	}													\
	if (first<=l1) {									\
	  if (last >= u1) RETURN_1;							\
	  u1 = last;										\
	} else {											\
	  l1 = first;										\
	  if (last<u1) u1=last;								\
	}													\
	if (u1<l1) RETURN_0;								\
  } 

/* 
   0 = t1-a*x 
   min(t1)<= a*x <= max(t1)
   a*min(x)<= t1 <= a*max(x) 
*/
#define REDUCE_LAST_TERM_DOMAIN_m(a,x,l1,u1){		\
	first = l1;										\
	last = u1;										\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      SOUND_UP_DIV_L(first0,first,a);				\
      SOUND_LOW_DIV_U(last0,last,a);				\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);		\
      first = SOUND_C_MUL_L(a,dv_ptr_first);		\
      last = SOUND_C_MUL_U(a,dv_ptr_last);			\
	} else {										\
      x = a*INTVAL(x);								\
      if (x<first || x> last) RETURN_0;				\
      first = last = x;								\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  } 

/* 
   0 = t1-a*x 
   min(t1)<= a*x <= max(t1)
   a*min(x)<= t1 <= a*max(x) 
*/
#define REDUCE_LAST_TERM_DOMAIN_nooverflow_m(a,x,l1,u1){	\
	first = l1;												\
	last = u1;												\
	if (IS_SUSP_VAR(x)){									\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);			\
      dv_ptr_first = DV_first(dv_ptr);						\
      dv_ptr_last = DV_last(dv_ptr);						\
      UP_DIV(first0,first,a);								\
      LOW_DIV(last0,last,a);								\
      CALL_DOMAIN_REGION(dv_ptr,first0,last0);				\
      first = a*dv_ptr_first;								\
      last = a*dv_ptr_last;									\
	} else {												\
      x = a*INTVAL(x);										\
      if (x<first || x> last) RETURN_0;						\
      first = last = x;										\
	}														\
	if (first<=l1) {										\
	  if (last >= u1) RETURN_1;								\
	  u1 = last;											\
	} else {												\
	  l1 = first;											\
	  if (last<u1) u1=last;									\
	}														\
	if (u1<l1) RETURN_0;									\
  } 

/* 
   t2 = t1-x 
   min(t2)+min(x)<= t1 <= max(t2)+max(x) 
   min(t1)-max(t2)<= x <= max(t1)-min(t2) 
*/
#define REDUCE_DOMAIN_m1(x,l1,u1,l2,u2){			\
	first = SOUND_L_SUB_U(l1,u2);					\
	last = SOUND_U_SUB_L(u1,l2);					\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = SOUND_L_ADD_L(l2,dv_ptr_first);		\
      last = SOUND_U_ADD_U(u2,dv_ptr_last);			\
	} else {										\
      x = INTVAL(x);								\
      if (x<first || x> last) RETURN_0;				\
      first = SOUND_C_ADD_L(x,l2);					\
      last = SOUND_C_ADD_U(x,u2);					\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  } 


/* 
   t2 = t1-x 
   min(t2)+min(x)<= t1 <= max(t2)+max(x) 
   min(t1)-max(t2)<= x <= max(t1)-min(t2) 
*/
#define REDUCE_DOMAIN_nooverflow_m1(x,l1,u1,l2,u2){ \
	first = l1-u2;									\
	last = u1-l2;									\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = l2+dv_ptr_first;						\
      last = u2+dv_ptr_last;						\
	} else {										\
      x = INTVAL(x);								\
      if (x<first || x> last) RETURN_0;				\
      first = x+l2;									\
      last = x+u2;									\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  } 

/* 
   0 = t1-x 
   min(x)<= t1 <= max(x) 
   min(t1)<= x <= max(t1) 
*/
#define REDUCE_LAST_TERM_DOMAIN_m1(x,l1,u1){		\
	first = l1;										\
	last = u1;										\
	if (IS_SUSP_VAR(x)){							\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);	\
      dv_ptr_first = DV_first(dv_ptr);				\
      dv_ptr_last = DV_last(dv_ptr);				\
      CALL_DOMAIN_REGION(dv_ptr,first,last);		\
      first = dv_ptr_first;							\
      last = dv_ptr_last;							\
	} else {										\
      x = INTVAL(x);								\
      if (x<first || x> last) RETURN_0;				\
      first = last = x;								\
	}												\
	if (first<=l1) {								\
	  if (last >= u1) RETURN_1;						\
	  u1 = last;									\
	} else {										\
	  l1 = first;									\
	  if (last<u1) u1=last;							\
	}												\
	if (u1<l1) RETURN_0;							\
  } 

/* 
   t2 = t1+ax >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   ax >= l2-u1 
   t1 >= l2-a*max(x)  
*/
#define REDUCE_DOMAIN_GE_p(a,x,l1,u1,l2){								\
	first = SOUND_L_SUB_U(l2,u1);										\
	if (IS_SUSP_VAR(x)){												\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);						\
      dv_ptr_first = DV_first(dv_ptr);									\
      dv_ptr_last = DV_last(dv_ptr);									\
      SOUND_UP_DIV_L(first0,first,a);									\
      if (first0>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first0);	\
      temp_i = SOUND_C_MUL_U(a,dv_ptr_last);							\
      first = SOUND_L_SUB_U(l2,temp_i);									\
    } else {															\
      x = a*INTVAL(x);													\
      if (x<first) RETURN_0;											\
      first = SOUND_L_SUB_C(l2,x);										\
    }																	\
    if (first<l1){RETURN_1;}											\
    l1=first;															\
  }

/* 
   t2 = t1+ax >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   ax >= l2-u1 
   t1 >= l2-a*max(x)  
*/
#define REDUCE_DOMAIN_GE_nooverflow_p(a,x,l1,u1,l2){					\
	first = l2-u1;														\
	if (IS_SUSP_VAR(x)){												\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);						\
      dv_ptr_first = DV_first(dv_ptr);									\
      dv_ptr_last = DV_last(dv_ptr);									\
      UP_DIV(first0,first,a);											\
      if (first0>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first0);	\
      first = l2-a*dv_ptr_last;											\
    } else {															\
      x = a*INTVAL(x);													\
      if (x<first) RETURN_0;											\
      first = l2-x;														\
    }																	\
    if (first<l1){RETURN_1;}											\
    l1=first;															\
  }

/* 
   t1+ax >= 0  (a>0) 
   ax >= -u1 
   t1 >= -a*max(x)  
*/
#define REDUCE_LAST_TERM_DOMAIN_GE_p(a,x,l1,u1){						\
	first = -u1;														\
	if (IS_SUSP_VAR(x)){												\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);						\
      dv_ptr_first = DV_first(dv_ptr);									\
      dv_ptr_last = DV_last(dv_ptr);									\
      SOUND_UP_DIV_L(first0,first,a);									\
      if (first0>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first0);	\
      temp_i = SOUND_C_MUL_U(a,dv_ptr_last);							\
      first = -temp_i;													\
    } else {															\
      x = a*INTVAL(x);													\
      if (x<first) RETURN_0;											\
      first = -x;														\
    }																	\
    if (first<l1){RETURN_1;}											\
    l1=first;															\
  }

/* 
   t1+ax >= 0  (a>0) 
   ax >= -u1 
   t1 >= -a*max(x)  
*/
#define REDUCE_LAST_TERM_DOMAIN_GE_nooverflow_p(a,x,l1,u1){				\
	first = -u1;														\
	if (IS_SUSP_VAR(x)){												\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);						\
      dv_ptr_first = DV_first(dv_ptr);									\
      dv_ptr_last = DV_last(dv_ptr);									\
      UP_DIV(first0,first,a);											\
      if (first0>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first0);	\
      first = -a*dv_ptr_last;											\
    } else {															\
      x = a*INTVAL(x);													\
      if (x<first) RETURN_0;											\
      first = -x;														\
    }																	\
    if (first<l1){RETURN_1;}											\
    l1=first;															\
  }

/* 
   t2 = t1+x >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   x >= l2-u1 
   t1 >= l2-max(x)  
*/
#define REDUCE_DOMAIN_GE_p1(x,l1,u1,l2){							\
	first = SOUND_L_SUB_U(l2,u1);									\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      if (first>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first); \
      first = SOUND_L_SUB_U(l2,dv_ptr_last);						\
    } else {														\
      x = INTVAL(x);												\
      if (x<first) RETURN_0;										\
      first = SOUND_L_SUB_C(l2,x);									\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  }

/* 
   t2 = t1+x >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   x >= l2-u1 
   t1 >= l2-max(x)  
*/
#define REDUCE_DOMAIN_GE_nooverflow_p1(x,l1,u1,l2){					\
	first = l2-u1;													\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      if (first>dv_ptr_first) CALL_DOMAIN_REGION_MIN(dv_ptr,first); \
      first = l2-dv_ptr_last;										\
    } else {														\
      x = INTVAL(x);												\
      if (x<first) RETURN_0;										\
      first = l2-x;													\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  }

/* 
   t2 = t1-ax >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   ax =< u1-l2 
   t1 >= l2+a*min(x)  
*/
#define REDUCE_DOMAIN_GE_m(a,x,l1,u1,l2){							\
	last = SOUND_U_SUB_L(u1,l2);									\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      SOUND_LOW_DIV_U(last0,last,a);								\
      if (last0<dv_ptr_last) CALL_DOMAIN_REGION_MAX(dv_ptr,last0);	\
      temp_i = SOUND_C_MUL_L(a,dv_ptr_first);						\
      first = SOUND_L_ADD_L(l2,temp_i);								\
    } else {														\
      x = a*INTVAL(x);												\
      if (x>last) RETURN_0;											\
      first = SOUND_C_ADD_L(x,l2);									\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  } 

/* 
   t2 = t1-ax >= l2  (a>0) 
   u1 = max(t1) l2 = min(t2) 
   ax =< u1-l2 
   t1 >= l2+a*min(x)  
*/
#define REDUCE_DOMAIN_GE_nooverflow_m(a,x,l1,u1,l2){				\
	last = u1-l2;													\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      LOW_DIV(last0,last,a);										\
      if (last0<dv_ptr_last) CALL_DOMAIN_REGION_MAX(dv_ptr,last0);	\
      first = l2+a*dv_ptr_first;									\
    } else {														\
      x = a*INTVAL(x);												\
      if (x>last) RETURN_0;											\
      first = x+l2;													\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  } 

/* 
   t2 = t1-x >= l2  
   x =< u1-l2 
   t1 >= l2+min(x)  
*/
#define REDUCE_DOMAIN_GE_m1(x,l1,u1,l2){							\
	last = SOUND_U_SUB_L(u1,l2);									\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      if (last<dv_ptr_last) CALL_DOMAIN_REGION_MAX(dv_ptr,last);	\
      first = SOUND_L_ADD_L(l2,dv_ptr_first);						\
    } else {														\
      x = INTVAL(x);												\
      if (x>last) RETURN_0;											\
      first = SOUND_C_ADD_L(x,l2);									\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  } 


/* 
   t2 = t1-x >= l2  
   x =< u1-l2 
   t1 >= l2+min(x)  
*/
#define REDUCE_DOMAIN_GE_nooverflow_m1(x,l1,u1,l2){					\
	last = u1-l2;													\
	if (IS_SUSP_VAR(x)){											\
      dv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(x);					\
      dv_ptr_first = DV_first(dv_ptr);								\
      dv_ptr_last = DV_last(dv_ptr);								\
      if (last<dv_ptr_last) CALL_DOMAIN_REGION_MAX(dv_ptr,last);		\
      first = l2+dv_ptr_first;										\
    } else {														\
      x = INTVAL(x);												\
      if (x>last) RETURN_0;											\
      first = l2+x;													\
    }																\
    if (first<l1) RETURN_1;											\
    l1=first;														\
  } 

