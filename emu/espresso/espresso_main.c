/*
 * Revision Control Information
 *
 * $Source$
 * $Author$
 * $Revision$
 * $Date$
 *
 */
/*
 *  Main driver for espresso
 *
 *  Old style -do xxx, -out xxx, etc. are still supported.
 */

#include "espresso.h"
#include "main.h"		/* table definitions for options */
#include <unistd.h>

static int input_type = FD_type;

void getPLA(char *fname, pPLA *PLA, int out_type);

// int main(int argc, char **argv)
int espresso_main(int argc, char **argv)
{

  int i, j, first, last, strategy, out_type, option;
  pPLA PLA, PLA1;
  pcover F, Fold, Dold;
  pset last1, p;
  cost_t cost;
  bool error, exact_cover;

  out_type = F_type;		/* default -o: default is ON-set only */

  error = FALSE;
#ifdef RANDOM
  srandom(314973);
#endif

  option = 0;			/* default -D: ESPRESSO */
  debug = 0;			/* default -d: no debugging info */
  verbose_debug = FALSE;	/* default -v: not verbose */
  print_solution = TRUE;	/* default -x: print the solution (!) */
  summary = FALSE;		/* default -s: no summary */
  trace = FALSE;		/* default -t: no trace information */
  strategy = 0;		/* default -S: strategy number */
  first = -1;			/* default -R: select range */
  last = -1;
  remove_essential = TRUE;	/* default -e: */
  force_irredundant = TRUE;
  unwrap_onset = TRUE;
  single_expand = FALSE;
  pos = FALSE;
  recompute_onset = FALSE;
  use_super_gasp = FALSE;
  use_random_order = FALSE;
  kiss = FALSE;
  echo_comments = TRUE;
  echo_unknown_commands = TRUE;
  exact_cover = FALSE;	/* for -qm option, the default */

  PLA = PLA1 = NIL(PLA_t);
  getPLA("ex2_2.txt", &PLA, out_type);

  fprint_pla(stderr,PLA, FD_type);

  /*  case KEY_ESPRESSO: */
  Fold = sf_save(PLA->F);
  PLA->F = espresso(PLA->F, PLA->D, PLA->R);
  EXECUTE(error=verify(PLA->F,Fold,PLA->D), VERIFY_TIME, PLA->F, cost);
  if (error) {
	print_solution = FALSE;
	PLA->F = Fold;
	(void) check_consistency(PLA);
  } else {
	free_cover(Fold);
  }

  /* Output the solution */
  EXECUTE(fprint_pla(stdout, PLA, out_type), WRITE_TIME, PLA->F, cost);

  /* Crash and burn if there was a verify error */
  if (error) {
	fatal("cover verification failed");
  }

  /* cleanup all used memory */
  free_PLA(PLA);
  FREE(cube.part_size);
  setdown_cube();             /* free the cube/cdata structure data */
  sf_cleanup();               /* free unused set structures */
  sm_cleanup();               /* sparse matrix cleanup */

  exit(0);
  return 0;
}


void getPLA(char *fname, pPLA *PLA, int out_type)
{
  FILE *fp;
  int needs_dcset, needs_offset;
  if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr, "Unable to open %s\n", fname);
	exit(1);
  }
  needs_dcset = option_table[0].needs_dcset;
  needs_offset = option_table[0].needs_offset;

  printf("  needs_dcset = %d needs_offset = %d \n", needs_dcset, needs_offset);

  if (read_pla(fp, needs_dcset, needs_offset, input_type, PLA) == EOF) {
	fprintf(stderr, "Unable to find PLA on file %s\n", fname);
	exit(1);
  }
  (*PLA)->filename = strdup(fname);
  filename = (*PLA)->filename;
  fclose(fp);
}


