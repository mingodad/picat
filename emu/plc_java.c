/*
 * plc_java.c: JIPL native interface for both Java and K-Prolog
 *
 * Copyright (c) 2000 Nobukuni Kino, KLS Research,Inc.
 * Copyright (c) 1997 Nobukuni Kino, ISAC,Inc.
 *
 * Permission to use, copy, modify, and distribute this
 * release of the software for RESEARCH/EVALUATION
 * purposes and without fee is hereby granted provided
 * that this copyright notice appears in all copies.
 * Notice that EDUCATIONAL purpose usage is NOT granted.
 *
 * The Java package name, com.kprolog.plc, is reserved for
 * KLS Research. So you should rename the package when you
 * apply JIPL to other language processors.
 *
 * Nobukuni Kino and KLS Reseach make no warranties
 * about the suitability of the software, either
 * expressed or implied, fitness for a particular
 * purpose.
 * Nobukuni Kino and KLS Reseach shall not be liable for
 * any damages suffered by licensee as a result of using,
 * modifying or distributing this software or its
 * derivatives.
 */
/*
  Modified by Neng-Fa Zhou

  com/kprolog => bprolog
  com_kprolog => bprolog
  isList(arg) ==> isList(arg) || ISNIL(arg)
  add DEREF(method) DEREF(obj)
  if (ISATOM(obj)) => javaMethod, javaGetField, javaSetField
  PnameToTerm => bp_string_2_term
*/
#include  <stdlib.h>
#include "bprolog.h"
#include "kapi.h"
#include "jni.h"
#include "event.h"
/*
  #define DEBUG_EVENT
*/
typedef struct _jobject *jref;

extern SYM_REC_PTR objectRef;
/* extern char *malloc();*/
/*
  char *malloc();
  char *bp_get_name();
*/

/* #include "Pinterface.h" */
static jclass class_Object;
static jclass class_ArrayOfObject;
static jclass class_String;
static jmethodID conv_String;
static jmethodID bytes_String;
static jclass class_Integer;
static jmethodID init_Integer;
static jclass class_BigInteger;
static jmethodID init_BigInteger;
static jclass class_Double;
static jmethodID init_Double;
static jclass class_Number;
static jclass class_Boolean;
static jclass class_Character;
static jclass class_Exception;
static jclass class_Plc;
static jmethodID result_Plc;
static jmethodID instance_Plc;
static jmethodID static_Plc;
static jmethodID set_Plc;
static jmethodID get_Plc;
static jmethodID new_Plc;
static JNIEnv *Env;

static void
java2prolog(JNIEnv *env, jobject val, BPLONG arg) {
    const char *str;
    if (!val) return;
    if ((*env)->IsInstanceOf(env, val, class_Number)) {
        jobject obj = (*env)->CallObjectMethod(env, val,
                                               (*env)->GetMethodID(env, (*env)->GetObjectClass(env, val),
                                                                   "toString", "()Ljava/lang/String;"),
                                               0);
        if (obj) str = (*env)->GetStringUTFChars(env, obj, NULL);
        else str = "invalid argument";
        /* !!! */
        if ((*env)->IsInstanceOf(env, val, class_Double) || (*env)->IsInstanceOf(env, val, class_Double)) {
            double atof();
            unify(arg, encodefloat1(atof(str)));
        } else {
            unify(arg, MAKEINT(atoi(str)));
        }
        /*      bprolog_string_2_term(str, arg);  */
        if (obj) {
            (*env)->ReleaseStringUTFChars(env, obj, str);
            (*env)->DeleteLocalRef(env, obj);
        }
    } else if ((*env)->IsInstanceOf(env, val, class_Boolean)) {
        jboolean b = (*env)->CallBooleanMethod(env, val,
                                               (*env)->GetMethodID(env, class_Boolean,
                                                                   "booleanValue", "()Z"),
                                               0);
        PuInt(arg, (BPLONG)b);
    } else if ((*env)->IsInstanceOf(env, val, class_String) ||
               (*env)->IsInstanceOf(env, val, class_Character)) {
        if ((*env)->IsInstanceOf(env, val, class_Character)) {
            val = (*env)->CallObjectMethod(env, val,
                                           (*env)->GetMethodID(env, class_Character,
                                                               "toString", "()Ljava/lang/String;"), 0);
        }
        str = (*env)->GetStringUTFChars(env, val, NULL);
        PuStr(arg, str);
        /*  PuAtom(arg, str); */
        (*env)->ReleaseStringUTFChars(env, val, str);
    } else if ((*env)->IsInstanceOf(env, val, class_ArrayOfObject)) {
        jsize i;
        BPLONG list;
        jsize len = (*env)->GetArrayLength(env, val);
        list = Plistn(len);
        unify(arg, list);
        for(i = 0; i < len; i++) {
            BPLONG car = PlistCar(arg);
            java2prolog(env, (*env)->GetObjectArrayElement(env, val, i), car);
            arg = PlistCdr(arg);
        }
    } else  /* if ((*env)->IsInstanceOf(env, val,class_Object)) */
#if defined(DEBUG)
        jobject mess = (*env)->CallObjectMethod(env, val,
                                                (*env)->GetMethodID(env, (*env)->GetObjectClass(env, val),
                                                                    "toString", "()Ljava/lang/String;"), 0);
    const char *str = (*env)->GetStringUTFChars(env, mess, NULL);
    (*env)->DeleteLocalRef(env, mess);
#endif
    PuAddr(arg, (*env)->NewGlobalRef(env, val));  /* +1 */
}
}
static jref
addr2ref(JNIEnv *env, BPLONG addr) {
    return (jref)PvalueOfAddr(addr);
}
static jref
prolog2java(JNIEnv *env, BPLONG arg) {
    const char *str;
    BPLONG_PTR top;
    double PvalueOfReal();
    char *PnumberToStr();

    //  printf("prolog2java "); write_term(arg); printf("\n");
    DEREF(arg);
    if (PisInt(arg)) {
        if (PisLong(arg)) {
            jref res;
            jref bignum;
            str = PnumberToStr(arg);
            res = (*env)->NewStringUTF(env, str);
            bignum = (*env)->NewObject(env, class_BigInteger, init_BigInteger, res);
            (*env)->DeleteLocalRef(env, res);
            return bignum;
        } else {
            return (*env)->NewObject(env, class_Integer, init_Integer,
                                     (jlong)PvalueOfInt(arg));
        }
    } else if (PisReal(arg)) {
        return (*env)->NewObject(env, class_Double, init_Double,
                                 (jdouble)PvalueOfReal(arg));
    } else if (PisAddr(arg)) {
        return addr2ref(env, arg);
        /*      } else if (str=(char*)PutfOfAtom(arg)) {
                return (*env)->NewStringUTF(env,str);
        */
    } else if (PisList(arg) || ISNIL(arg)) {
        int len = PlistLength(arg);
        jref array = (*env)->NewObjectArray(env, len, class_Object, 0);
        int i;
        jref res;
        BPLONG_PTR ptr;
        for(i = 0; i < len; i++) {
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(arg);
            res = prolog2java(env, FOLLOW(ptr));
            (*env)->SetObjectArrayElement(env, array, i, res);
            (*env)->DeleteLocalRef(env, res);
            arg = FOLLOW(ptr+1);
            DEREF(arg);
        }
        return array;
    } else if (PisAtom(arg)) {
        str = bp_get_name(arg);
        return (*env)->NewStringUTF(env, str);
    } else if (PisCompound(arg)) {
        int len = ParityOfCompound(arg);
        //              printf(" PisCompound %d ",len); write_term(arg); printf("\n");
        jref array = (*env)->NewObjectArray(env, len, class_Object, 0);
        int i;
        for(i = 0; i < len; i++) {
            jref res = prolog2java(env, PargOfCompound(arg, i));
            (*env)->SetObjectArrayElement(env, array, i, res);
            (*env)->DeleteLocalRef(env, res);
        }
        return array;
    } else {
        return (*env)->NewStringUTF(env, "?");
    }
}
static jref
prolog2javaArray(JNIEnv *env, BPLONG arg) {
    jref result;
    BPLONG_PTR top;

    DEREF(arg);
    //  printf("prolog2javaArray "); write_term(arg); printf("\n");
    if (PisCompound(arg)) result = prolog2java(env, arg);
    else result = (*env)->NewObjectArray(env, 0, class_Object, 0);
    return result;
}

static void
initialize(JNIEnv *env, jref arrayobj) {
    class_Object = (*env)->NewGlobalRef(env,
                                        (*env)->FindClass(env, "java/lang/Object"));
    class_String = (*env)->NewGlobalRef(env,
                                        (*env)->FindClass(env, "java/lang/String"));
    class_Integer = (*env)->NewGlobalRef(env,
                                         (*env)->FindClass(env, "java/lang/Integer"));
    class_BigInteger = (*env)->NewGlobalRef(env,
                                            (*env)->FindClass(env, "java/math/BigInteger"));
    class_Double = (*env)->NewGlobalRef(env,
                                        (*env)->FindClass(env, "java/lang/Double"));
    class_Number = (*env)->NewGlobalRef(env,
                                        (*env)->FindClass(env, "java/lang/Number"));
    class_Boolean = (*env)->NewGlobalRef(env,
                                         (*env)->FindClass(env, "java/lang/Boolean"));
    class_Character = (*env)->NewGlobalRef(env,
                                           (*env)->FindClass(env, "java/lang/Character"));
    class_Exception = (*env)->NewGlobalRef(env,
                                           (*env)->FindClass(env, "java/lang/Exception"));
#if FindClassResolved
    class_ArrayOfObject = (*env)->FindClass(env, "[Ljava/lang/Object;");
#else
    class_ArrayOfObject = (*env)->NewGlobalRef(env,
                                               (*env)->GetObjectClass(env, arrayobj));
#endif
    class_Plc = (*env)->NewGlobalRef(env,
                                     (*env)->FindClass(env, "bprolog/plc/Plc"));
    {
        Env = env;
        init_Integer = (*env)->GetMethodID(env, class_Integer, "<init>", "(I)V");
        init_BigInteger = (*env)->GetMethodID(env, class_BigInteger, "<init>", "(Ljava/lang/String;)V");
        init_Double = (*env)->GetMethodID(env, class_Double, "<init>", "(D)V");
        conv_String = (*env)->GetMethodID(env, class_String, "<init>", "([BLjava/lang/String;)V");
        bytes_String = (*env)->GetMethodID(env, class_String, "getBytes", "(Ljava/lang/String;)[B");
        result_Plc = (*env)->GetMethodID(env, class_Plc, "setResult",
                                         "(ILjava/lang/Object;)V");
        instance_Plc = (*env)->GetStaticMethodID(env, class_Plc, "instanceMethod",
                                                 "(Ljava/lang/Object;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;");
        static_Plc = (*env)->GetStaticMethodID(env, class_Plc, "staticMethod",
                                               "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;");
        set_Plc = (*env)->GetStaticMethodID(env, class_Plc, "setField",
                                            "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/String;Ljava/lang/Object;)V");
        get_Plc = (*env)->GetStaticMethodID(env, class_Plc, "getField",
                                            "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/String;)Ljava/lang/Object;");
        new_Plc = (*env)->GetStaticMethodID(env, class_Plc, "javaConstructor",
                                            "(Ljava/lang/Class;[Ljava/lang/Object;)Ljava/lang/Object;");
    }

#if defined(DEBUG)
    printf("%8x,%8x,%8x,%8x\n", class_ArrayOfObject, class_Plc, class_String, bytes_String);
    printf("%8x,%8x,%8x,%8x, env=%x\n", instance_Plc, static_Plc, new_Plc, env);
#endif
}
static int
callJavaException(JNIEnv *env) {
    const char *str;
    jobject mess = NULL;
    jobject e = (*env)->ExceptionOccurred(env);
    BPLONG command = Pfunctorn(Patom("jiplException"), 1);
    (*env)->ExceptionDescribe(env);
    if (e) {
        jclass exclass = (*env)->GetObjectClass(env, e);
        (*env)->CallObjectMethod(env, e,
                                 (*env)->GetMethodID(env, exclass, "printStackTrace", "()V"), 0);
        mess = (*env)->CallObjectMethod(env, e,
                                        (*env)->GetMethodID(env, exclass, "toString", "()Ljava/lang/String;"),
                                        0);
    }
    if (mess) str = (*env)->GetStringUTFChars(env, mess, NULL);
    else str = "unknown";
    /* !!! */
    /* PuStr(PargOfCompound(command, 0),str); */
    PuAtom(PargOfCompound(command, 0), str);
    if (mess) (*env)->ReleaseStringUTFChars(env, mess, str);
    PcallF(command);
    (*env)->ExceptionClear(env);
    bp_exception = java_exception;
    return BP_ERROR;
}
static jclass
findClass(JNIEnv *env, const char *name) {
    jclass clazz = (*env)->FindClass(env, name);
    if (!clazz) {
        char *lname = (char*)malloc(strlen(name)+32);
        if (lname == NULL) myquit(OUT_OF_MEMORY, "plc");
        sprintf(lname, "java/lang/%s", name);
        clazz = (*env)->FindClass(env, lname);
        free(lname);
    }
    if (!clazz) {
        (*env)->ThrowNew(env, (*env)->FindClass(env,
                                                "java/lang/ClassNotFoundException"), name);
    }
    return clazz;
}

int
javaMethod() {
    BPLONG_PTR top;
    BPLONG obj = ARG(1, 3);
    BPLONG method = ARG(2, 3);
    BPLONG result = ARG(3, 3);
    DEREF(obj);
    /*  printf("javaMethod ");write_term(method);printf("\n"); */
    return javaMethod1(obj, method, result);
}

int
javaMethod1(BPLONG obj, BPLONG method, BPLONG result) {
    JNIEnv *env = Env;
    const char *functor = (const char*)PascOfFunc(method);
    // printf("functor = %s\n", functor);
    /* !!! */
    jstring funcname = (*env)->NewStringUTF(env, functor);
    jref args = prolog2javaArray(env, method);
    jobject res;

    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    if (PisAddr(obj)) {  /* assuming, obj is an Instance */
        res = (*env)->CallStaticObjectMethod(env, class_Plc, instance_Plc,
                                             addr2ref(env, obj), funcname, args);
    } else if (ISATOM(obj)) {
        const char *class = (const char*)PascOfAtom(obj);
        jclass clazz;
        /* printf("findClass %s",class);write_term(obj);printf("\n"); */

        /* !!! */
        clazz = findClass(env, class);
        if (!clazz) return callJavaException(env);
        res = (*env)->CallStaticObjectMethod(env, class_Plc, static_Plc,
                                             clazz, funcname, args);
        (*env)->DeleteLocalRef(env, clazz);
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    (*env)->DeleteLocalRef(env, funcname);
    (*env)->DeleteLocalRef(env, args);
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    if (res) {
        java2prolog(env, res, result);
        (*env)->DeleteLocalRef(env, res);
    }
    return BP_TRUE;
}


int
javaRegisterEventListener(int source_no, int event_no) {
    JNIEnv *env = Env;
    const char *functor = "registerEventListener";
    jstring funcname = (*env)->NewStringUTF(env, functor);
    jref args;
    jref elm;
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    } else {
        const char *class = "bprolog/cg/BPP";
        jclass clazz;
        /* !!! */
        clazz = findClass(env, class);
        if (!clazz) return callJavaException(env);

        args = (*env)->NewObjectArray(env, 2, class_Object, 0);
        elm = (*env)->NewObject(env, class_Integer, init_Integer, (jlong)source_no);
        (*env)->SetObjectArrayElement(env, args, 0, elm);
        elm = (*env)->NewObject(env, class_Integer, init_Integer, (jlong)event_no);
        (*env)->SetObjectArrayElement(env, args, 1, elm);
        (*env)->CallStaticObjectMethod(env, class_Plc, static_Plc,
                                       clazz, funcname, args);
        (*env)->DeleteLocalRef(env, clazz);
        (*env)->DeleteLocalRef(env, elm);
        (*env)->DeleteLocalRef(env, args);
    }

    (*env)->DeleteLocalRef(env, funcname);
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    return BP_TRUE;
}

int
javaGetField() {
    BPLONG_PTR top;
    BPLONG obj, field, value;
    obj = ARG(1, 3);
    field = ARG(2, 3);
    value = ARG(3, 3);
    DEREF(obj);
    DEREF(field);
    DEREF(value);
    if (!ISATOM(field)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    return javaGetField1(obj, bp_get_name(field), value);
}

int
javaGetField1(BPLONG obj, const char*field, BPLONG value) {
    JNIEnv *env = Env;
    /* !!! */
    jstring fieldname = (*env)->NewStringUTF(env, field);
    jobject val;

    if (PisAddr(obj)) {  /* assuming, obj is an Instance */
        val = (*env)->CallStaticObjectMethod(env, class_Plc, get_Plc,
                                             NULL, addr2ref(env, obj), fieldname);
    } else if (ISATOM(obj)) {
        char *class = (char*)PascOfAtom(obj);
        /* !!! */
        jclass clazz = findClass(env, class);
        if (!clazz) return callJavaException(env);
        val = (*env)->CallStaticObjectMethod(env, class_Plc, get_Plc,
                                             clazz, NULL, fieldname);
        (*env)->DeleteLocalRef(env, clazz);
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }

    (*env)->DeleteLocalRef(env, fieldname);
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    java2prolog(env, val, value);
    return BP_TRUE;
}

int
javaSetField() {
    BPLONG_PTR top;
    BPLONG obj, field, value;

    obj = ARG(1, 3);
    field = ARG(2, 3);
    value = ARG(3, 3);
    DEREF(obj);
    DEREF(field);
    DEREF(value);
    if (!ISATOM(field)) {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }
    return javaSetField1(obj, bp_get_name(field), value);
}

int
javaSetField1(BPLONG obj, const char*field, BPLONG value) {
    JNIEnv *env = Env;
    /* !!! */
    jstring fieldname = (*env)->NewStringUTF(env, field);

    if (PisAddr(obj)) {  /* assuming, obj is an Instance */
        jref res = prolog2java(env, value);
        (*env)->CallStaticObjectMethod(env, class_Plc, set_Plc,
                                       NULL, addr2ref(env, obj), fieldname, res);
        (*env)->DeleteLocalRef(env, res);
    } else if (ISATOM(obj)) {
        char *class = (char*)PascOfAtom(obj);
        /* !!! */
        jclass clazz = findClass(env, class);
        jref res = prolog2java(env, value);
        if (!clazz) return callJavaException(env);
        (*env)->CallStaticObjectMethod(env, class_Plc, set_Plc,
                                       clazz, NULL, fieldname, res);
        (*env)->DeleteLocalRef(env, res);
        (*env)->DeleteLocalRef(env, clazz);
    } else {
        bp_exception = illegal_arguments;
        return BP_ERROR;
    }

    (*env)->DeleteLocalRef(env, fieldname);
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    return BP_TRUE;
}

int
javaConstructor() {
    BPLONG init = ARG(1, 2);
    BPLONG objaddr = ARG(2, 2);
    JNIEnv *env = Env;
    jref args = prolog2javaArray(env, init);
    const char *class = (const char*)PascOfFunc(init);
    jclass clazz;
    jobject obj;
    char fullname[64];

    /* !!!*/
    clazz = findClass(env, class);
    if (!clazz) return callJavaException(env);
    obj = (*env)->CallStaticObjectMethod(env, class_Plc, new_Plc, clazz, args);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, args);
    if ((*env)->ExceptionCheck(env)) {
        return callJavaException(env);
    }
    if (obj) {
        obj = (*env)->NewGlobalRef(env, obj);  /* +1 */
        return PuAddr(objaddr, (void*)obj);
    }
    else return BP_FALSE;  /* fail */
}

void javaDeleteRef2(BPLONG objaddr) {
    JNIEnv *env = Env;
    jref ref = (jref)PvalueOfAddr(objaddr);
    (*env)->DeleteGlobalRef(env, ref);  /* -1 */
}

int
javaDeleteRef() {
    BPLONG objaddr = ARG(1, 1);
    javaDeleteRef2(objaddr);
    return 1;
}

#include "bprolog_plc_Plc.h"
JNIEXPORT void JNICALL Java_bprolog_plc_Plc_startPlc(JNIEnv *env, jclass clazz,
                                                     jarray args, jarray oa) {
    int i;
    int argc = 0;
    const char* argv[24];
    static jboolean once = JNI_FALSE;

    if (once) return;
    once = JNI_TRUE;

    if ((BPLONG)((*env)->GetArrayLength(env, args)) > 24) quit("too many arguments in bpp");
    argv[argc++] = "jipl";
    for (i = (BPLONG)((*env)->GetArrayLength(env, args)); --i >= 0; ) {
        jobject arg = (*env)->GetObjectArrayElement(env, args, argc-1);
        /* !!! */
        argv[argc++] = (*env)->GetStringUTFChars(env, arg, NULL);
    }
    PinitP(argc, argv);
    /*  PexecP("true"); */
    for (i = (BPLONG)((*env)->GetArrayLength(env, args)); --i >= 0; ) {
        jobject arg = (*env)->GetObjectArrayElement(env, args, i);
        (*env)->ReleaseStringUTFChars(env, arg, argv[i+1]);
    }

    /* initialize compiled Prolog modules */
    /*  jni_interface(); */
    /*  plc_sup(); */
    initialize(env, oa);
}
JNIEXPORT jboolean JNICALL Java_bprolog_plc_Plc_exec(JNIEnv *env, jclass clazz,
                                                     jstring cmd) {
    jboolean result;
    const char *ucmd;

    /* !!!*/
    Env = env;
    result = Pexecute(ucmd = ((*env)->GetStringUTFChars(env, cmd, NULL)));
    (*env)->ReleaseStringUTFChars(env, cmd, ucmd);
    return result;
}
JNIEXPORT jboolean JNICALL Java_bprolog_plc_Plc_call(JNIEnv *env,
                                                     jobject self, jstring functor, jarray args) {
    int i;
    int rc;
    /* !!! */
    const char *name = (*env)->GetStringUTFChars(env, functor, NULL);
    int arity = (BPLONG)((*env)->GetArrayLength(env, args));
    BPLONG command = Pfunctorn(Patom(name), arity);

    Env = env;
    (*env)->ReleaseStringUTFChars(env, functor, name);
    for (i = 0; i < arity; i++) {
        jobject arg = (*env)->GetObjectArrayElement(env, args, i);
        if (arg) java2prolog(env, arg, PargOfCompound(command, i));
    }
    rc = PcallX(command);
    if (rc) {
        jobject arg;
        for (i = 0; i < arity; i++) {
            arg = (*env)->GetObjectArrayElement(env, args, i);
            if (arg == NULL) {
                jref res = prolog2java(env, PargOfCompound(command, i));
                (*env)->CallVoidMethod(env, self, result_Plc, i, res);
                (*env)->DeleteLocalRef(env, res);
            }
            (*env)->DeleteLocalRef(env, arg);
        }
    }
    return (jboolean)rc;
}

void Cboot_plc() {
    extern int c_cg_print_component();
    extern int cg_get_default_window();

    objectRef = insert_sym("$objectAddr", 11, 2);
    insert_cpred("javaConstructor", 2, javaConstructor);
    insert_cpred("javaDeleteRef", 1, javaDeleteRef);
    insert_cpred("javaMethod", 3, javaMethod);
    insert_cpred("javaGetField", 3, javaGetField);
    insert_cpred("javaSetField", 3, javaSetField);
    insert_cpred("cg_print_component", 1, c_cg_print_component);
    insert_cpred("cgDefaultWindow", 1, cg_get_default_window);
}

#include "bprolog_cg_CgEvent.h"

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1mouse_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong type, jlong x, jlong y, jlong count, jlong modifiers) {
    CgMouseEventClass mouseEventObject;

#ifdef DEBUG_EVENT
    printf("mouse event: %d %d\n", no, type);
#endif

    mouseEventObject = (CgMouseEventClass)malloc(sizeof(struct CgMouseEvent));
    if (mouseEventObject == NULL) myquit(OUT_OF_MEMORY, "plc");
    mouseEventObject->type = type;
    mouseEventObject->x = x;
    mouseEventObject->y = y;
    mouseEventObject->count = count;
    mouseEventObject->modifiers = modifiers;

    add_to_event_pool(MAKEINT(no), type, mouseEventObject);
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1window_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong type) {

#ifdef DEBUG_EVENT
    printf("window event: %d %d\n", no, type);
#endif


    add_to_event_pool(MAKEINT(no), type, (void *)MAKEINT(type));
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1focus_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong type) {

#ifdef DEBUG_EVENT
    printf("focus event: %d %d\n", no, type);
#endif

    add_to_event_pool(MAKEINT(no), type, (void *)MAKEINT(type));
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1key_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong type, jlong code, jchar ch, jlong modifiers) {
    CgKeyEventClass keyEventObject;

#ifdef DEBUG_EVENT
    printf("key event: %d %d\n", no, type);
#endif

    keyEventObject = (CgKeyEventClass)malloc(sizeof(struct CgKeyEvent));
    if (keyEventObject == NULL) myquit(OUT_OF_MEMORY, "plc");
    keyEventObject->type = type;
    keyEventObject->code = code;
    keyEventObject->ch = ch;
    keyEventObject->modifiers = modifiers;

    add_to_event_pool(MAKEINT(no), type, keyEventObject);
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1component_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong type, jlong x, jlong y, jlong width, jlong height) {

    CgComponentEventClass componentEventObject;

#ifdef DEBUG_EVENT
    printf("component event: %d %d\n", no, type);
#endif

    componentEventObject = (CgComponentEventClass)malloc(sizeof(struct CgComponentEvent));
    if (componentEventObject == NULL) myquit(OUT_OF_MEMORY, "plc");
    componentEventObject->type = type;
    componentEventObject->x = x;
    componentEventObject->y = y;
    componentEventObject->width = width;
    componentEventObject->height = height;

    add_to_event_pool(MAKEINT(no), type, componentEventObject);
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1action_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no) {

#ifdef DEBUG_EVENT
    printf("action event: %d\n", no);
#endif

    add_to_event_pool(MAKEINT(no), ActionPerformed, (void *)MAKEINT(ActionPerformed));
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1text_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jstring text) {
    const char *str;
#ifdef DEBUG_EVENT
    printf("text event: %d %d\n", no);
#endif
    add_to_event_pool(MAKEINT(no), TextValueChanged, (void *)MAKEINT(TextValueChanged));
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1item_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong index, jlong change) {

    CgItemEventClass itemEventObject;
#ifdef DEBUG_EVENT
    printf("item event: %d %d\n", no, index);
#endif
    itemEventObject = (CgItemEventClass)malloc(sizeof(struct CgItemEvent));
    if (itemEventObject == NULL) myquit(OUT_OF_MEMORY, "plc");
    itemEventObject->type = ItemStateChanged;
    itemEventObject->index = index;
    itemEventObject->change = change;
    add_to_event_pool(MAKEINT(no), ItemStateChanged, itemEventObject);
}

JNIEXPORT void JNICALL Java_bprolog_cg_CgEvent_post_1adjustment_1event_1to_1bprolog
(JNIEnv *env, jclass class, jlong no, jlong adjust_type, jlong value) {
    CgAdjustmentEventClass adjustmentEventObject;
    adjustmentEventObject = (CgAdjustmentEventClass)malloc(sizeof(struct CgAdjustmentEvent));
    if (adjustmentEventObject == NULL) myquit(OUT_OF_MEMORY, "plc");
    adjustmentEventObject->type = AdjustmentValueChanged;
    adjustmentEventObject->adjust_type = adjust_type;
    adjustmentEventObject->value = value;
    add_to_event_pool(MAKEINT(no), AdjustmentValueChanged, adjustmentEventObject);
}









