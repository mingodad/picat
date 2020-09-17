

#include "reasonls_interface.h"


#include "reasonls/core/Solver.h"


#include "reasonls/utils/System.h"
#include "reasonls/utils/ParseUtils.h"
#include "reasonls/utils/Options.h"
#include "reasonls/core/Dimacs.h"
#include "reasonls/simp/SimpSolver.h"



#include <vector>
#include <iostream>

#include <ctime>

using namespace Minisat;


static SimpSolver* maple_s;

static vec<Lit> lits;


extern "C" {
	void maple_init() {
		if (maple_s) {
			delete maple_s;
		}
		
		maple_s = new SimpSolver();

		maple_s->parsing = 1;
		maple_s->verbosity = -1;

		//maple_s->use_simplification = true;
		//maple_s->certifiedUNSAT = false;
		//maple_s->vbyte = false;
		
	}

	void maple_add_lit(int lit0) {	
		if (lit0 == 0) {
			maple_s->addClause(lits);
			lits.clear();
		}
		else {
			int var = abs(lit0) - 1;
			while (var >= maple_s->nVars())
				maple_s->newVar();
			lits.push((lit0 > 0) ? mkLit(var) : ~mkLit(var));
		}
	}


	int maple_start_solver() {
	  /*
		printf("solve, time: ");
		
		std::clock_t    start;
		start = std::clock();
	  */

		maple_s->parsing = 0;
		maple_s->eliminate(true);

		vec<Lit> dummy;
		lbool ret = maple_s->solveLimited(dummy);
		/*
		printf("%d ms, ", (int)((std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)));

		if (ret == l_True)
			printf("SAT\n");
		else
			printf("UNSAT\n");
		*/
		return ret == l_True ? 1 : 0;

		//maple_s->parsing = 0;
		//return maple_s->solve();
	}

	int maple_get_binding(int varNum) {
		if (varNum > maple_s->model.size()) {
			return 0;
		}
		return maple_s->model[varNum - 1] == l_True ? 1 : 0;
	}

}
