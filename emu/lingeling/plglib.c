/* Copyright 2010-2011 Armin Biere, Johannes Kepler University, Linz, Austria */
/* API extracted from 'plingeling.c' by Neng-Fa Zhou, July 2017                                   */

#include "lglib.h"

#include <assert.h>
#if !defined(CYGWIN) && !defined(ANDROID)
#include <execinfo.h>
#endif
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#define NWORKERS 8
#define NUNITS (1<<9)
#define NOEQ 0

extern LGL *bp_lgl;

#define NEW_MEM(PTR,N)										\
  do {														\
    size_t BYTES = (N) * sizeof *(PTR);						\
    if (!((PTR) = malloc (BYTES))) die ("out of memory");	\
    allocated += BYTES;										\
    assert (allocated >= 0);                                \
    memset ((PTR), 0, BYTES);								\
  } while (0)

#define RESIZE_MEM(PTR,O,N)												\
  do {																	\
    size_t OLD_BYTES = (O) * sizeof *(PTR);								\
    size_t NEW_BYTES = (N) * sizeof *(PTR);								\
    assert (NEW_BYTES >= OLD_BYTES);									\
    assert (allocated >= OLD_BYTES);									\
    if (!((PTR) = realloc ((PTR), (NEW_BYTES)))) die ("out of memory");	\
	allocated += (NEW_BYTES-OLD_BYTES);									\
	assert (allocated >= 0);											\
	memset (((char*)(PTR)) + OLD_BYTES, 0, NEW_BYTES - OLD_BYTES);		\
  } while (0)

typedef struct Worker {
  LGL * lgl;
  pthread_t thread;
  int res, fixed;
  int units[NUNITS], nunits;
  struct {
    struct { int calls, produced, consumed; } units;
    struct { int produced, consumed; } eqs;
    int produced, consumed;
  } stats;
} Worker;

static int nworkers;
static Worker * workers = NULL;
extern int sat_nvars;
extern int sat_nvars_limit;
static int * vals, * fixed, * repr;
static int nfixed, globalres;
static int verbose, plain, noeq = NOEQ;
#ifndef NLGLOG
static int loglevel;
#endif
static size_t allocated;
// static int catchedsig;
static double start;

struct { int units, eqs; } syncs;
static int done, termchks, units, eqs, flushed;
static pthread_mutex_t donemutex;            // = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t msgmutex;             // = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fixedmutex;           // = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t reprmutex;            // = PTHREAD_MUTEX_INITIALIZER;

static double currentime (void) {
  double res = 0;
  struct timeval tv;
  if (!gettimeofday (&tv, 0)) res = 1e-6 * tv.tv_usec, res += tv.tv_sec;
  return res;
}

static double getime () { return currentime () - start; }


#define msg3(a1,a2,a3) 
#define msg4(a1,a2,a3,a4) 
#define msg5(a1,a2,a3,a4,a5)
#define msg6(a1,a2,a3,a4,a5,a6)
#define msg7(a1,a2,a3,a4,a5,a6,a7)

/*
#define msg3(a1,a2,a3) msg(a1,a2,a3)
#define msg4(a1,a2,a3,a4) msg(a1,a2,a3,a4)
#define msg5(a1,a2,a3,a4,a5) msg(a1,a2,a3,a4,a5)
#define msg6(a1,a2,a3,a4,a5,a6) msg(a1,a2,a3,a4,a5,a6)
#define msg7(a1,a2,a3,a4,a5,a6,a7)  msg(a1,a2,a3,a4,a5,a6,a7)
*/

/*
static void msg (int wid, int level, const char * fmt, ...) {
  va_list ap;
  //  if (verbose < level) return;
  pthread_mutex_lock (&msgmutex);
  if (wid < 0) printf ("c - "); else printf ("c %d ", wid);
  printf ("W %6.1f ", getime ());
  va_start (ap, fmt);
  vfprintf (stdout, fmt, ap);
  fputc ('\n', stdout);
  fflush (stdout);
  pthread_mutex_unlock (&msgmutex);
}
*/
static void die (const char * fmt, ...) {
  /*
  va_list ap;
  fputs ("*** plingeling error: ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fputc ('\n', stderr);
  fflush (stderr);
  */
  exit (1);
}

static void warn (const char * fmt, ...) {
  /*
  va_list ap;
  fputs ("*** plingeling warning: ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fputc ('\n', stderr);
  fflush (stderr);
  */
}
/*
static void (*sig_int_handler)(int);
static void (*sig_segv_handler)(int);
static void (*sig_abrt_handler)(int);
static void (*sig_term_handler)(int);

static void resetsighandlers (void) {
  (void) signal (SIGINT, sig_int_handler);
  (void) signal (SIGSEGV, sig_segv_handler);
  (void) signal (SIGABRT, sig_abrt_handler);
  (void) signal (SIGTERM, sig_term_handler);
}

static void caughtsigmsg (int sig) {
  if (!verbose) return;
  printf ("c\nc CAUGHT SIGNAL %d\nc\n", sig);
  fflush (stdout);
}
*/
/*
static void catchsig (int sig) {
  if (!catchedsig) {
#if 0
    void * a[10];
    size_t s;
    s = backtrace (a, sizeof (sizeof a/sizeof *a));
    fprintf (stderr, "*** plingeling.c: caught signal %d at:\n", sig);
    backtrace_symbols_fd (a, s, 2);
#endif
    catchedsig = 1;
    caughtsigmsg (sig);
	//    if (verbose) stats (), caughtsigmsg (sig);
  }
  resetsighandlers ();
  if (!getenv ("LGLNABORT")) raise (sig); else exit (1);
}

static void setsighandlers (void) {
  sig_int_handler = signal (SIGINT, catchsig);
  sig_segv_handler = signal (SIGSEGV, catchsig);
  sig_abrt_handler = signal (SIGABRT, catchsig);
  sig_term_handler = signal (SIGTERM, catchsig);
}
*/
void plgl_release(Worker *workers){
  int i;
  
  for (i = 0; i < nworkers; i++) {
    lglrelease (workers[i].lgl);
    // pthread_cancel(workers[i].thread);
    msg4 (-1, 2, "released worker %d", i);
  }
  free (workers);
  free (fixed);
  if (!noeq) free (repr);
  free (vals);
}

static int term (void * voidptr) {
  Worker * worker = voidptr;
  int wid = worker - workers, res;
  assert (0 <= wid && wid < nworkers);
  msg3 (wid, 3, "checking early termination");
  if (pthread_mutex_lock (&donemutex))
    warn ("failed to lock 'done' mutex in termination check");
  res = done;
  termchks++;
  if (pthread_mutex_unlock (&donemutex)) 
    warn ("failed to unlock 'done' mutex in termination check");
  msg4 (wid, 3, "early termination check %s", res ? "succeeded" : "failed");
  return res;
}


static void flush (Worker * worker, int keep_locked) {
  int wid = worker - workers;
  int lit, idx, val, tmp, i;
  assert (worker->nunits);
  msg4 (wid, 2, "flushing %d units", worker->nunits);
  if (pthread_mutex_lock (&fixedmutex))
    warn ("failed to lock 'fixed' mutex in flush");
  flushed++;
  for (i = 0; i < worker->nunits; i++) {
    lit = worker->units[i];
    idx = abs (lit);
    assert (1 <= idx && idx <= sat_nvars);
    assert (0 <= wid && wid < nworkers);
    worker->stats.units.calls++;
    val = (lit < 0) ? -1 : 1;
    tmp = vals[idx];
    if (!tmp) {
      assert (nfixed < sat_nvars);
      fixed[nfixed++] = lit;
      vals[idx] = val;
      assert (!fixed[nfixed]);
      worker->stats.units.produced++;
      worker->stats.produced++;
      units++;
    } else if (tmp == -val) {
      if (pthread_mutex_lock (&donemutex))
        warn ("failed to lock 'done' mutex flushing unit");
      if (!globalres) msg3 (wid, 1, "mismatched unit");
      globalres = 20;
      done = 1;
      if (pthread_mutex_unlock (&donemutex)) 
        warn ("failed to unlock 'done' mutex flushing unit");
      break;
    } else assert (tmp == val);
  }
  worker->nunits = 0;
  if (keep_locked) return;
  if (pthread_mutex_unlock (&fixedmutex)) 
    warn ("failed to unlock 'fixed' mutex in flush");
}

static void produce (void * voidptr, int lit) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  assert (worker->nunits < NUNITS);
  worker->units[worker->nunits++] = lit;
  msg4 (wid, 3, "producing unit %d", lit);
  if (worker->nunits == NUNITS) flush (worker, 0);
}

static void consume (void * voidptr, int ** fromptr, int ** toptr) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  if (worker->nunits) flush (worker, 1);
  else if (pthread_mutex_lock (&fixedmutex))
    warn ("failed to lock 'fixed' mutex in consume");
  msg3 (wid, 3, "starting unit synchronization");
  syncs.units++;
  *fromptr = fixed + worker->fixed;
  *toptr = fixed + nfixed;
  if (pthread_mutex_unlock (&fixedmutex))
    warn ("failed to unlock 'fixed' in consume");
}

static int * lockrepr (void * voidptr) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  if (pthread_mutex_lock (&reprmutex))
    warn ("failed to lock 'repr' mutex");
  msg3 (wid, 3, "starting equivalences synchronization");
  syncs.eqs++;
  return repr;
}

static void unlockrepr (void * voidptr, int consumed, int produced) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  msg5 (wid, 3, 
       "finished equivalences synchronization: %d consumed, %d produced",
       consumed, produced);
  worker->stats.eqs.consumed += consumed;
  worker->stats.eqs.produced += produced;
  worker->stats.consumed += consumed;
  worker->stats.produced += produced;
  eqs += produced;
  assert (eqs < sat_nvars);
  if (pthread_mutex_unlock (&reprmutex))
    warn ("failed to unlock 'repr' mutex");
}

static void consumed (void * voidptr, int consumed) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  worker->stats.units.consumed += consumed;
  worker->stats.consumed += consumed;
  msg4 (wid, 3, "consuming %d units", consumed);
}

static void msglock (void * voidptr) {
  (void) voidptr;
  pthread_mutex_lock (&msgmutex);
}

static void msgunlock (void * voidptr) {
  (void) voidptr;
  pthread_mutex_unlock (&msgmutex);
}

static void * work (void * voidptr) {
  Worker * worker = voidptr;
  int wid = worker - workers;
  LGL * lgl = worker->lgl;
  assert (0 <= wid && wid < nworkers);
  msg3 (wid, 1, "running");
  assert (workers <= worker && worker < workers + nworkers);
  worker->res = lglsat (lgl);
  msg4 (wid, 1, "result %d", worker->res);
  if (pthread_mutex_lock (&donemutex))
    warn ("failed to lock 'done' mutex in worker");
  done = 1;
  if (pthread_mutex_unlock (&donemutex)) 
    warn ("failed to unlock 'done' mutex in worker");
  msg7 (wid, 2, "%d decisions, %d conflicts, %.0f props, %.1f MB",
       lglgetdecs (lgl), lglgetconfs (lgl), lglgetprops (lgl), lglmb (lgl));
  if (verbose >= 2) {
    if (pthread_mutex_lock (&fixedmutex))
      warn ("failed to lock 'fixed' in work");
    msg7 (wid, 2, "consumed %d units %.0f%%, produced %d units %.0f%%",
         worker->stats.units.consumed, 
         percent (worker->stats.units.consumed, nfixed),
         worker->stats.units.produced, 
         percent (worker->stats.units.produced, nfixed));
    if (pthread_mutex_unlock (&fixedmutex))
      warn ("failed to unlock 'fixed' in work");
  }
  return worker->res ? worker : 0;
}

static void setopt (int wid, const char * opt, int val) {
  Worker * w;
  LGL * lgl; 
  int old;
  assert (0 <= wid && wid < nworkers);
  w = workers + wid;
  lgl = w->lgl;
  assert (lglhasopt (lgl, opt));
  old = lglgetopt (lgl, opt);
  if (old == val) return;
  msg6 (wid, 1, "--%s=%d (instead of %d)", opt, val, old);
  lglsetopt (lgl, opt, val);
}

static void set10x (int wid, const char * opt) {
  int old, val;
  Worker * w;
  LGL * lgl; 
  assert (0 <= wid && wid < nworkers);
  w = workers + wid;
  lgl = w->lgl;
  assert (lglhasopt (lgl, opt));
  old = lglgetopt (lgl, opt);
  val = 10*old;
  msg6 (wid, 1, "--%s=%d (instead of %d)", opt, val, old);
  lglsetopt (lgl, opt, val);
}

void plgl_resize_dyn_arrays(){
  int new_sat_nvars_limit = 2*sat_nvars_limit;
  RESIZE_MEM (fixed, sat_nvars_limit, new_sat_nvars_limit);
  RESIZE_MEM (vals, sat_nvars_limit, new_sat_nvars_limit);
  if (!noeq) RESIZE_MEM (repr, sat_nvars_limit, new_sat_nvars_limit);
  sat_nvars_limit = new_sat_nvars_limit;
}

void plgl_add_lit0(){
  int i;
  for (i = 0; i < nworkers; i++)
    lgladd (workers[i].lgl, 0);
}

void plgl_add_lit(int lit){
  int i;

  for (i = 0; i < nworkers; i++)
    lgladd (workers[i].lgl, lit);
}
  
void plgl_init(int nworkers0){
  Worker * w;
  int i, val;
  LGL * lgl;

  nworkers = nworkers0;
  if (workers != NULL)
    plgl_release(workers);
  
  nfixed = 0;
  globalres = 0;
  verbose = 0;
  plain = 0;
  noeq = NOEQ;
#ifndef NLGLOG
  loglevel = 0;
#endif
  allocated = 0;
  done = 0;
  termchks = 0;
  units = 0;
  eqs = 0;
  flushed = 0;
  pthread_mutex_init(&donemutex, NULL);
  pthread_mutex_init(&msgmutex, NULL);
  pthread_mutex_init(&fixedmutex, NULL);
  pthread_mutex_init(&reprmutex, NULL);  
  /*  
  donemutex = PTHREAD_MUTEX_INITIALIZER;
  msgmutex = PTHREAD_MUTEX_INITIALIZER;
  fixedmutex = PTHREAD_MUTEX_INITIALIZER;
  reprmutex = PTHREAD_MUTEX_INITIALIZER;
  */
  start = currentime ();

  //  printf("%%sat_nvars = %d\n",sat_nvars);
  NEW_MEM (fixed, sat_nvars + 1);
  NEW_MEM (vals, sat_nvars + 1);
  if (!noeq) NEW_MEM (repr, sat_nvars + 1);
  
  //
  NEW_MEM (workers, nworkers);
  for (i = 0; i < nworkers; i++) {
    w = workers + i;
    lgl = lglinit ();
    w->lgl = lgl;
    lglsetid (lgl, i, nworkers);
    lglsetime (lgl, getime);
    lglsetopt (lgl, "verbose", verbose);
#ifndef NLGLOG
    lglsetopt (lgl, "log", loglevel);
#endif
    setopt (i, "seed", i);
    setopt (i, "phase", -((i+1) % 3 - 1));
    switch (i&3) {
    default: val = 2; break;
    case 1: val = -1; break;
    case 2: val = 1; break;
    }
    setopt (i, "bias", val);
    if ((i&3)==1) set10x (i, "lftreleff");
    if ((i&3)==2) setopt (i, "elmnostr", 2);
    if (i == 3) setopt (i, "plain", 1);
    else if ((i&3)==3) set10x (i, "prbreleff");
    lglseterm (lgl, term, w);
    if (!plain) {
      lglsetproduceunit (lgl, produce, w);
      lglsetconsumeunits (lgl, consume, w);
      if (!noeq) {
        lglsetlockeq (lgl, lockrepr, w);
        lglsetunlockeq (lgl, unlockrepr, w);
      }
      lglsetconsumedunits (lgl, consumed, w);
      lglsetmsglock (lgl, msglock, msgunlock, w);
    }
    msg3 (i, 2, "initialized");
  }
  //  setsighandlers ();
}

int plgl_start (LGL **ptr_lgl) {
  Worker * w, * winner, *maxconsumer, * maxproducer;
  int i, res;
 
  for (i = 0; i < nworkers; i++) {
    if (pthread_create (&workers[i].thread, 0, work, workers + i))
      die ("failed to create worker thread %d", i);
    msg4 (-1, 2, "started worker %d", i);
  }
  maxproducer = maxconsumer = winner = 0;
  res = 0;
  msg4 (-1, 2, "joining %d workers", nworkers);
  for (i = 0; i < nworkers; i++) {
    w = workers + i;
    if (pthread_join (w->thread, 0))
      die ("failed to join worker thread %d", i);
    msg4 (-1, 2, "joined worker %d", i);
    if (w->res) {
      if (!res) {
        res = w->res;
        winner = w;
        msg5 (-1, 1, "worker %d is the winner with result %d", i, res);
      } else if (res != w->res) die ("result discrepancy");
    }
    if (!maxconsumer || w->stats.consumed > maxconsumer->stats.consumed)
      maxconsumer = w;
    if (!maxproducer || w->stats.produced > maxproducer->stats.produced)
      maxproducer = w;
  }
  if (!res) res = globalres;
  if (!res) die ("no result by any worker");
  *ptr_lgl = winner->lgl;
  return res;
}

