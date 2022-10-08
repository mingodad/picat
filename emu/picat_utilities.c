#include "picat.h"
#include <stdlib.h>
#include <string.h>

TERM cstring_to_picat(char* ch_ptr, int n) {
  BPLONG ret_lst;
  BPLONG_PTR ret_lst_ptr;
  
  ret_lst_ptr = &ret_lst;

  while (n > 0) {
	char *ch_ptr0 = ch_ptr;
	int code_len;
	
	utf8_char_to_codepoint(&ch_ptr);
	code_len = ch_ptr - ch_ptr0;
	n -= code_len;
	FOLLOW(heap_top) = 	ADDTAG(insert_sym(ch_ptr0, code_len, 0), ATM);
	FOLLOW(ret_lst_ptr) = ADDTAG(heap_top, LST);
	heap_top++;
	ret_lst_ptr = heap_top;
	heap_top++;
	LOCAL_OVERFLOW_CHECK("cstring_to_picat");
  }
  FOLLOW(ret_lst_ptr) = nil_sym;
  return ret_lst;
}

//Translate Picat String To C-string
char* picat_string_to_cstring(TERM t) {
  BPLONG car;
  CHAR_PTR s0, s;
  BPLONG_PTR top, ptr;
  SYM_REC_PTR sym_ptr;
  BPLONG i, len, n = 0;

  s0 = s = (char *)heap_top;

  DEREF(t);
  while (ISLIST(t)) {
	CHAR_PTR name_ptr;
	ptr = (BPLONG_PTR)UNTAGGED_ADDR(t);
	car = FOLLOW(ptr);
	DEREF(car);                                             // assume car is an atom, will crash if car is not an atom
	sym_ptr = (SYM_REC_PTR)UNTAGGED_ADDR(car);
	name_ptr = GET_NAME(sym_ptr);
	len = GET_LENGTH(sym_ptr);
	for (i = 0; i < len; i++){
	  *s++ = *name_ptr++;
	}
	n += len;
	t = FOLLOW(ptr+1);
	DEREF(t);
  }
  if ((BPLONG_PTR)s >= local_top) {
	myquit(STACK_OVERFLOW, "to_cstring");
  }
  *s = '\0';
  s = (char *)malloc(n+1);
  strcpy(s, s0);
  return s;
}

TERM picat_get_list_end(TERM l) {
    if (picat_is_list(l)) {
        TERM car = picat_get_car(l);
        TERM cdr = picat_get_cdr(l);
        while (!picat_is_var(cdr)) {
            car = cdr;
            cdr = picat_get_cdr(car);
        }

        return cdr;
    }
    return l;
}


TERM new_picat_map(int C) {
    TERM t = picat_build_structure("$hshtb", 2);

    picat_unify(picat_get_arg(1, t), picat_build_integer(0));
    picat_unify(picat_get_arg(2, t), picat_build_structure("hashtable", C));

    return t;
}

void add_to_picat_map(TERM m, TERM key, TERM value) {
    int C = picat_get_struct_arity(picat_get_arg(2, m));

    //Get hash value
    TERM HashVal = MAKEINT(bp_hashval(key));
    int Index = picat_get_integer(HashVal) % C + 1;

    //Create pair to add to map
    TERM Pair = picat_build_structure("=", 2);
    picat_unify(picat_get_arg(1, Pair), key);
    picat_unify(picat_get_arg(2, Pair), value);

    //Place pair in a list and that list into the hashtable
    TERM Bucket = picat_build_list();
    picat_unify(picat_get_car(Bucket), Pair);

    TERM Hashtable = picat_get_arg(2, m);
    TERM H_Bucket = picat_get_list_end(picat_get_arg(Index, Hashtable));
    picat_unify(H_Bucket, Bucket);

    //Update count of map
    TERM Count = picat_get_arg(1, m);
    Count = picat_build_integer(picat_get_integer(Count) + 1);
}

TERM picat_map_get(TERM map, TERM key) {
    int C = picat_get_struct_arity(picat_get_arg(2, map));

    //Get hash value
    TERM HashVal = MAKEINT(bp_hashval(key));
    int Index = picat_get_integer(HashVal) % C + 1;

    TERM Hashtable = picat_get_arg(2, map);
    TERM Bucket = picat_get_arg(Index, Hashtable);

    TERM car = picat_get_car(Bucket);
    TERM cdr = picat_get_cdr(Bucket);

    while (!picat_is_var(car)) {
        TERM K = picat_get_arg(1, car);

        char* a = picat_string_to_cstring(K), *b = picat_string_to_cstring(key);

        //printf("key: %s\n", a);
        //printf("base: %s\n", b);

        if (strcmp(picat_string_to_cstring(K), picat_string_to_cstring(key)) == 0) {
            //printf("found\n");
            return picat_get_arg(2, car);
        }

        TERM temp = cdr;
        car = picat_get_car(temp);
        cdr = picat_get_cdr(temp);
    }


    printf("cant find ");
    //picat_write_term(key);
    //printf("\n");
}
