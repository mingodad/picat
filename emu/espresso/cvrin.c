/*
  module: cvrin.c
  purpose: cube and cover input routines
*/

#include "espresso.h"

static bool line_length_error;
static int lineno;

void skip_line(register FILE *fpin, register FILE *fpout, register int echo)
{
  register int ch;
  while ((ch=getc(fpin)) != EOF && ch != '\n')
	if (echo)
	  putc(ch, fpout);
  if (echo)
	putc('\n', fpout);
  lineno++;
}

char *get_word(register FILE *fp, register char *word)
{
  register int ch, i = 0;
  while ((ch = getc(fp)) != EOF && isspace(ch))
	;
  word[i++] = ch;
  while ((ch = getc(fp)) != EOF && ! isspace(ch))
	word[i++] = ch;
  word[i++] = '\0';
  return word;
}

/*
 *  Yes, I know this routine is a mess
 */
void read_cube(register FILE *fp, pPLA PLA)
{
  register int var, i;
  pcube cf = cube.temp[0], cr = cube.temp[1], cd = cube.temp[2];
  bool savef = FALSE, saved = FALSE, saver = FALSE;
  char token[256],ch; 			/* for kiss read hack */
  int varx, first, last, offset;	/* for kiss read hack */

  //  printf("read_cube number_binary_vars=%d\n",cube.num_binary_vars);
  set_clear(cf, cube.size);

  /* Loop and read binary variables */
  for(var = 0; var < cube.num_binary_vars; var++){
	ch = getc(fp);
	printf("%c",ch);
	switch(ch){
	case EOF:
	  goto bad_char;
	case '\n':
	  if (! line_length_error)
		fprintf(stderr, "product term(s) %s\n",
				"span more than one line (warning only)");
	  line_length_error = TRUE;
	  lineno++;
	  var--;
	  break;
	case ' ': case '|': case '\t':
	  var--;
	  break;
	case '2': case '-':
	  set_insert(cf, var*2+1);
	case '0':
	  set_insert(cf, var*2);
	  break;
	case '1':
	  set_insert(cf, var*2+1);
	  break;
	case '?':
	  break;
	default:
	  goto bad_char;
	}
  }

  printf("\n");
  /* Loop for last multiple-valued variable */
  set_copy(cr, cf);
  set_copy(cd, cf);

  //  printf("cube.first_part[var]=%d cube.last_part[var]=%d\n",cube.first_part[var], cube.last_part[var]);

  for(i = cube.first_part[var]; i <= cube.last_part[var]; i++){
	ch = getc(fp);
	switch (ch) {
	case EOF:
	  goto bad_char;
	case '\n':
	  if (! line_length_error)
		fprintf(stderr, "product term(s) %s\n",
				"span more than one line (warning only)");
	  line_length_error = TRUE;
	  lineno++;
	  i--;
	  break;
	case ' ': case '|': case '\t':
	  i--;
	  break;
	case '4': case '1':
	  if (PLA->pla_type & F_type)
		set_insert(cf, i), savef = TRUE;
	  break;
	case '3': case '0':
	  if (PLA->pla_type & R_type)
		set_insert(cr, i), saver = TRUE;
	  break;
	case '2': case '-':
	  if (PLA->pla_type & D_type)
		set_insert(cd, i), saved = TRUE;
	case '~':
	  break;
	default:
	  goto bad_char;
	}
  }
  printf("\n");
  if (savef) PLA->F = sf_addset(PLA->F, cf);
  if (saved) PLA->D = sf_addset(PLA->D, cd);
  if (saver) PLA->R = sf_addset(PLA->R, cr);
  return;

 bad_char:
  fprintf(stderr, "(warning): input line #%d ignored\n", lineno);
  skip_line(fp, stdout, TRUE);
  return;
}

void parse_pla(FILE *fp, pPLA PLA)
{
  int i, var, ch, np, last;
  char word[256];

  lineno = 1;
  line_length_error = FALSE;

 loop:
  switch(ch = getc(fp)) {
  case EOF:
	return;

  case '\n':
	lineno++;

  case ' ': case '\t': case '\f': case '\r':
	break;

  case '#':
	(void) ungetc(ch, fp);
	skip_line(fp, stdout, echo_comments);
	break;

  case '.':
	/* .i gives the cube input size (binary-functions only) */
	if (equal(get_word(fp, word), "i")) {
	  if (cube.fullset != NULL) {
		fprintf(stderr, "extra .i ignored\n");
		skip_line(fp, stdout, /* echo */ FALSE);
	  } else {
		if (fscanf(fp, "%d", &cube.num_binary_vars) != 1)
		  fatal("error reading .i");
		cube.num_vars = cube.num_binary_vars + 1;
		cube.part_size = ALLOC(int, cube.num_vars);
	  }

	  /* .o gives the cube output size (binary-functions only) */
	} else if (equal(word, "o")) {
	  if (cube.fullset != NULL) {
		fprintf(stderr, "extra .o ignored\n");
		skip_line(fp, stdout, /* echo */ FALSE);
	  } else {
		if (cube.part_size == NULL)
		  fatal(".o cannot appear before .i");
		if (fscanf(fp, "%d", &(cube.part_size[cube.num_vars-1]))!=1)
		  fatal("error reading .o");
		cube_setup();
		PLA_labels(PLA);
	  }
	}
  default:
	(void) ungetc(ch, fp);
	if (cube.fullset == NULL) {
	  /*		fatal("unknown PLA size, need .i/.o or .mv");*/
	  if (echo_comments)
		putchar('#');
	  skip_line(fp, stdout, echo_comments);
	  break;
	}
	if (PLA->F == NULL) {
	  PLA->F = new_cover(10);
	  PLA->D = new_cover(10);
	  PLA->R = new_cover(10);
	}
	read_cube(fp, PLA);
  }
  goto loop;
}
/*
  read_pla -- read a PLA from a file

  Input stops when ".e" is encountered in the input file, or upon reaching
  end of file.

  Returns the PLA in the variable PLA after massaging the "symbolic"
  representation into a positional cube notation of the ON-set, OFF-set,
  and the DC-set.

  needs_dcset and needs_offset control the computation of the OFF-set
  and DC-set (i.e., if either needs to be computed, then it will be
  computed via complement only if the corresponding option is TRUE.)
  pla_type specifies the interpretation to be used when reading the
  PLA.

  The phase of the output functions is adjusted according to the
  global option "pos" or according to an imbedded .phase option in
  the input file.  Note that either phase option implies that the
  OFF-set be computed regardless of whether the caller needs it
  explicitly or not.

  Bit pairing of the binary variables is performed according to an
  imbedded .pair option in the input file.

  The global cube structure also reflects the sizes of the PLA which
  was just read.  If these fields have already been set, then any
  subsequent PLA must conform to these sizes.

  The global flags trace and summary control the output produced
  during the read.

  Returns a status code as a result:
  EOF (-1) : End of file reached before any data was read
  > 0	 : Operation successful
*/

int read_pla(FILE *fp, int needs_dcset, int needs_offset, int pla_type, pPLA *PLA_return)
{
  pPLA PLA;
  int i, second, third;
  long time;
  cost_t cost;

  /* Allocate and initialize the PLA structure */
  PLA = *PLA_return = new_PLA();
  PLA->pla_type = pla_type;

  /* Read the pla */
  time = ptime();
  parse_pla(fp, PLA);

  /* Check for nothing on the file -- implies reached EOF */
  if (PLA->F == NULL) {
	return EOF;
  }

  after_setup_pla(needs_dcset, needs_offset, PLA);
  return 1;
}

int after_setup_pla(int needs_dcset, int needs_offset, pPLA PLA)
{
  int i, second, third;
  long time;
  cost_t cost;

  /* This hack merges the next-state field with the outputs */
  for(i = 0; i < cube.num_vars; i++) {
	cube.part_size[i] = ABS(cube.part_size[i]);
  }

  /* Decide how to break PLA into ON-set, OFF-set and DC-set */
  time = ptime();
  if (pos || PLA->phase != NULL || PLA->symbolic_output != NIL(symbolic_t)) {
	needs_offset = TRUE;
  }
  if (needs_offset && (PLA->pla_type==F_type || PLA->pla_type==FD_type)) {
	free_cover(PLA->R);
	PLA->R = complement(cube2list(PLA->F, PLA->D));
  } else if (needs_dcset && PLA->pla_type == FR_type) {
	pcover X;
	free_cover(PLA->D);
	/* hack, why not? */
	X = d1merge(sf_join(PLA->F, PLA->R), cube.num_vars - 1);
	PLA->D = complement(cube1list(X));
	free_cover(X);
  } else if (PLA->pla_type == R_type || PLA->pla_type == DR_type) {
	free_cover(PLA->F);
	PLA->F = complement(cube2list(PLA->D, PLA->R));
  }

  /* Check for phase rearrangement of the functions */
  if (pos) {
	pcover onset = PLA->F;
	PLA->F = PLA->R;
	PLA->R = onset;
	PLA->phase = new_cube();
	set_diff(PLA->phase, cube.fullset, cube.var_mask[cube.num_vars-1]);
  } else if (PLA->phase != NULL) {
	(void) set_phase(PLA);
  }

  /* Setup minimization for two-bit decoders */
  if (PLA->pair != (ppair) NULL) {
	set_pair(PLA);
  }

  if (PLA->symbolic != NIL(symbolic_t)) {
	EXEC(map_symbolic(PLA), "MAP-INPUT  ", PLA->F);
  }
  if (PLA->symbolic_output != NIL(symbolic_t)) {
	EXEC(map_output_symbolic(PLA), "MAP-OUTPUT ", PLA->F);
	if (needs_offset) {
	  free_cover(PLA->R);
	  EXECUTE(PLA->R=complement(cube2list(PLA->F,PLA->D)), COMPL_TIME, PLA->R, cost);
	}
  }
  
  return 1;
}

pPLA new_PLA(void)
{
  pPLA PLA;

  PLA = ALLOC(PLA_t, 1);
  PLA->F = PLA->D = PLA->R = (pcover) NULL;
  PLA->phase = (pcube) NULL;
  PLA->pair = (ppair) NULL;
  PLA->label = (char **) NULL;
  PLA->filename = (char *) NULL;
  PLA->pla_type = 0;
  PLA->symbolic = NIL(symbolic_t);
  PLA->symbolic_output = NIL(symbolic_t);
  return PLA;
}


void PLA_labels(pPLA PLA)
{
  int i;

  PLA->label = ALLOC(char *, cube.size);
  for(i = 0; i < cube.size; i++)
	PLA->label[i] = (char *) NULL;
}


void free_PLA(pPLA PLA)
{
  symbolic_list_t *p2, *p2next;
  symbolic_t *p1, *p1next;
  int i;

  if (PLA->F != (pcover) NULL)
	free_cover(PLA->F);
  if (PLA->R != (pcover) NULL)
	free_cover(PLA->R);
  if (PLA->D != (pcover) NULL)
	free_cover(PLA->D);
  if (PLA->phase != (pcube) NULL)
	free_cube(PLA->phase);
  if (PLA->pair != (ppair) NULL) {
	FREE(PLA->pair->var1);
	FREE(PLA->pair->var2);
	FREE(PLA->pair);
  }
  if (PLA->label != NULL) {
	for(i = 0; i < cube.size; i++)
	  if (PLA->label[i] != NULL)
		FREE(PLA->label[i]);
	FREE(PLA->label);
  }
  if (PLA->filename != NULL) {
	FREE(PLA->filename);
  }
  for(p1 = PLA->symbolic; p1 != NIL(symbolic_t); p1 = p1next) {
	for(p2 = p1->symbolic_list; p2 != NIL(symbolic_list_t); p2 = p2next) {
	  p2next = p2->next;
	  FREE(p2);
	}
	p1next = p1->next;
	FREE(p1);
  }
  PLA->symbolic = NIL(symbolic_t);
  for(p1 = PLA->symbolic_output; p1 != NIL(symbolic_t); p1 = p1next) {
	for(p2 = p1->symbolic_list; p2 != NIL(symbolic_list_t); p2 = p2next) {
	  p2next = p2->next;
	  FREE(p2);
	}
	p1next = p1->next;
	FREE(p1);
  }
  PLA->symbolic_output = NIL(symbolic_t);
  FREE(PLA);
}


int label_index(pPLA PLA, char *word, int *varp, int *ip)
{
  int var, i;

  if (PLA->label == NIL(char *) || PLA->label[0] == NIL(char)) {
	if (sscanf(word, "%d", varp) == 1) {
	  *ip = *varp;
	  return TRUE;
	}
  } else {
	for(var = 0; var < cube.num_vars; var++) {
	  for(i = 0; i < cube.part_size[var]; i++) {
		if (equal(PLA->label[cube.first_part[var]+i], word)) {
		  *varp = var;
		  *ip = i;
		  return TRUE;
		}
	  }
	}
  }
  return FALSE;
}
