/*
  module: cvrout.c
  purpose: cube and cover output routines
*/

#include "espresso.h"

void fprint_pla(FILE *fp, pPLA PLA, int output_type)
{
  int num;
  register pcube last, p;

  printf("output_type = %d\n",output_type);

  //  fpr_header(fp, PLA, output_type);

  num = 0;
  if (output_type & F_type) num += (PLA->F)->count;
  fprintf(fp, ".p %d\n", num);

  /* quick patch 01/17/85 to support TPLA ! */
  foreach_set(PLA->F, last, p) {
	print_cube(fp, p, "01");
  }
  fprintf(fp, ".e\n");
}

void fpr_header(FILE *fp, pPLA PLA, int output_type)
{
  register int i, var;
  int first, last;

  /* .type keyword gives logical type */
  /* Check for binary or multiple-valued labels */
  if (cube.num_mv_vars <= 1) {
	fprintf(fp, ".i %d\n", cube.num_binary_vars);
	if (cube.output != -1)
	  fprintf(fp, ".o %d\n", cube.part_size[cube.output]);
  } else {
	fprintf(fp, ".mv %d %d", cube.num_vars, cube.num_binary_vars);
	for(var = cube.num_binary_vars; var < cube.num_vars; var++)
	  fprintf(fp, " %d", cube.part_size[var]);
	fprintf(fp, "\n");
  }

  /* binary valued labels */
  if (PLA->label != NIL(char *) && PLA->label[1] != NIL(char)
	  && cube.num_binary_vars > 0) {
	fprintf(fp, ".ilb");
	for(var = 0; var < cube.num_binary_vars; var++)
	  fprintf(fp, " %s", INLABEL(var));
	putc('\n', fp);
  }

  /* output-part (last multiple-valued variable) labels */
  if (PLA->label != NIL(char *) &&
	  PLA->label[cube.first_part[cube.output]] != NIL(char)
	  && cube.output != -1) {
	fprintf(fp, ".ob");
	for(i = 0; i < cube.part_size[cube.output]; i++)
	  fprintf(fp, " %s", OUTLABEL(i));
	putc('\n', fp);
  }

  /* multiple-valued labels */
  for(var = cube.num_binary_vars; var < cube.num_vars-1; var++) {
	first = cube.first_part[var];
	last = cube.last_part[var];
	if (PLA->label != NULL && PLA->label[first] != NULL) {
	  fprintf(fp, ".label var=%d", var);
	  for(i = first; i <= last; i++) {
		fprintf(fp, " %s", PLA->label[i]);
	  }
	  putc('\n', fp);
	}
  }

  if (PLA->phase != (pcube) NULL) {
	first = cube.first_part[cube.output];
	last = cube.last_part[cube.output];
	fprintf(fp, "#.phase ");
	for(i = first; i <= last; i++)
	  putc(is_in_set(PLA->phase,i) ? '1' : '0', fp);
	fprintf(fp, "\n");
  }
}

char *fmt_cube(register pset c, register char *out_map, register char *s)
{
  register int i, var, last, len = 0;

  for(var = 0; var < cube.num_binary_vars; var++) {
	s[len++] = "?01-" [GETINPUT(c, var)];
  }
  for(var = cube.num_binary_vars; var < cube.num_vars - 1; var++) {
	s[len++] = ' ';
	for(i = cube.first_part[var]; i <= cube.last_part[var]; i++) {
	  s[len++] = "01" [is_in_set(c, i) != 0];
	}
  }
  if (cube.output != -1) {
	last = cube.last_part[cube.output];
	s[len++] = ' ';
	for(i = cube.first_part[cube.output]; i <= last; i++) {
	  s[len++] = out_map [is_in_set(c, i) != 0];
	}
  }
  s[len] = '\0';
  return s;
}

void print_cube(register FILE *fp, register pset c, register char *out_map)
{
  register int i, var, ch;
  int last;

  for(var = 0; var < cube.num_binary_vars; var++) {
	ch = "?01-" [GETINPUT(c, var)];
	putc(ch, fp);
  }
  /*
  for(var = cube.num_binary_vars; var < cube.num_vars - 1; var++) {
	putc(' ', fp);
	for(i = cube.first_part[var]; i <= cube.last_part[var]; i++) {
	  ch = "01" [is_in_set(c, i) != 0];
	  putc(ch, fp);
	}
  }
  */
  /*
  if (cube.output != -1) {
	last = cube.last_part[cube.output];
	putc(' ', fp);
	for(i = cube.first_part[cube.output]; i <= last; i++) {
	  ch = out_map [is_in_set(c, i) != 0];
	  putc(ch, fp);
	}
  }
  */
  putc('\n', fp);
}


char *pc1(pset c)
{static char s1[256];return fmt_cube(c, "01", s1);}
char *pc2(pset c)
{static char s2[256];return fmt_cube(c, "01", s2);}



void cprint(pset_family T)
{
  register pcube p, last;

  foreach_set(T, last, p)
	printf("%s\n", pc1(p));
}


void makeup_labels(pPLA PLA)
{
  int var, i, ind;

  if (PLA->label == (char **) NULL)
	PLA_labels(PLA);

  for(var = 0; var < cube.num_vars; var++)
	for(i = 0; i < cube.part_size[var]; i++) {
	  ind = cube.first_part[var] + i;
	  if (PLA->label[ind] == (char *) NULL) {
		PLA->label[ind] = ALLOC(char, 16);
		if (var < cube.num_binary_vars)
		  if ((i % 2) == 0)
			(void) sprintf(PLA->label[ind], "v%d.bar", var);
		  else
			(void) sprintf(PLA->label[ind], "v%d", var);
		else
		  (void) sprintf(PLA->label[ind], "v%d.%d", var, i);
	  }
	}
}


