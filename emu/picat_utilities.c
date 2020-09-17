#include "picat.h"
#include "stdlib.h"

TERM cstring_to_picat(char* v, int n) {
    if (v == NULL || n <= 0) {
        return picat_build_nil();
    }

    TERM ret = picat_build_list(), car, cdr;
    car = picat_get_car(ret);
    cdr = picat_get_cdr(ret);


    for (int i = 0; i < n - 1; i++) {
        char c[] = { v[i], "\0" };
        picat_unify(car, picat_build_atom(c));
        TERM temp = picat_build_list();
        picat_unify(cdr, temp);

        car = picat_get_car(temp);
        cdr = picat_get_cdr(temp);
    }
    char c[] = { v[n - 1], "\0" };
    picat_unify(car, picat_build_atom(c));
    picat_unify(cdr, picat_build_nil());

    return ret;
}

//Translate Picat String To C-string
char* picat_string_to_cstring(TERM t) {
    int mult = 1;
    char* str = calloc(512, sizeof(char));

    TERM tList = t;
    TERM x = picat_get_car(tList);
    int i = 0;

    while (x != picat_build_integer(0)) {
        i++;
        if (i >= 512 * mult) {
            mult++;
            char *buffer = realloc(str, 512 * mult);
            str = buffer;
        }
        strcat(str, picat_get_atom_name(x));
        tList = picat_get_cdr(tList);
        x = picat_get_car(tList);
    }
    return str;
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
