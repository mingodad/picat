CCC=$(CC) -c
CPPC=$(CPP) -c

ESPRESSO_OBJ = black_white.o canonical.o cofactor.o cols.o compl.o contain.o cpu_time.o cubestr.o \
               cvrin.o cvrm.o cvrmisc.o cvrout.o dominate.o equiv.o espresso.o espresso_expand.o \
               essen.o essentiality.o exact.o gasp.o gimpel.o globals.o hack.o indep.o irred.o \
               map.o matrix.o mincov.o opo.o pair.o part.o primes.o prtime.o reduce.o rows.o \
               set.o setc.o sharp.o sigma.o signature.o signature_exact.o sminterf.o solution.o \
               sparse.o unate.o util_signature.o verify.o

OBJ = dis.o init.o init_sym.o loader.o inst_inf.o main.o toam.o unify.o \
	file.o domain.o cfd.o float1.o arith.o token.o global.o \
	builtins.o mic.o numbervars.o cpreds.o univ.o assert.o findall.o clause.o \
    delay.o clpfd.o clpfd_libs.o event.o toamprofile.o \
    kapi.o getline.o table.o gcstack.o gcheap.o gcqueue.o debug.o \
	mSolver.o mOptions.o mSystem.o mSimpSolver.o \
    expand.o bigint.o sapi.o sat_bp.o maple_interface.o espresso_bp.o \
	picat_utilities.o fann.o fann_cascade.o fann_error.o fann_io.o fann_train.o fann_train_data.o fann_interface.o

picat$(EXT) : $(OBJ) $(ESPRESSO_OBJ)
	$(CPP) -o picat$(EXT) $(CFLAGS) $(OBJ) $(ESPRESSO_OBJ) $(LFLAGS)
clean :
	rm -f *.o picat$(EXT)
dis.o   : dis.c term.h inst.h basic.h
	$(CCC) $(CFLAGS) dis.c
init.o  : init.c term.h inst.h basic.h
	$(CCC) $(CFLAGS) init.c
init_sym.o  : init_sym.c term.h inst.h basic.h
	$(CCC) $(CFLAGS) init_sym.c
loader.o : loader.c term.h basic.h inst.h picat_bc.h
	$(CCC) $(CFLAGS) loader.c
inst_inf.o  : inst_inf.c term.h inst.h basic.h
	$(CCC) $(CFLAGS) inst_inf.c
main.o  : main.c term.h inst.h basic.h
	$(CCC) $(CFLAGS) main.c
toam.o  : toam.c term.h inst.h basicd.h basic.h toam.h event.h frame.h emu_inst.h
	$(CCC) $(CFLAGS) toam.c
toamprofile.o  : toamprofile.c term.h inst.h basicd.h basic.h toam.h event.h frame.h
	$(CCC) $(CFLAGS) toamprofile.c
unify.o : unify.c term.h basic.h event.h bapi.h frame.h
	$(CCC) $(CFLAGS) unify.c
file.o	: file.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) file.c
domain.o	: domain.c term.h basic.h bapi.h event.h frame.h
	$(CCC) $(CFLAGS) domain.c
cfd.o	: cfd.c term.h basic.h bapi.h event.h frame.h
	$(CCC) $(CFLAGS) cfd.c
float1.o	: float1.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) float1.c
arith.o	: arith.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) arith.c
bigint.o	: bigint.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) bigint.c
token.o	: token.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) token.c
global.o	: global.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) global.c
builtins.o	: builtins.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) builtins.c
mic.o	: mic.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) mic.c
numbervars.o	: numbervars.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) numbervars.c
cpreds.o	: cpreds.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) cpreds.c
univ.o	: univ.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) univ.c
assert.o	: assert.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) assert.c
findall.o	: findall.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) findall.c
clause.o	: clause.c term.h basic.h bapi.h dynamic.h
	$(CCC) $(CFLAGS) clause.c
delay.o	: delay.c term.h basic.h bapi.h
	$(CCC) $(CFLAGS) delay.c
clpfd.o : clpfd.c term.h basic.h bapi.h event.h frame.h
	$(CCC) $(CFLAGS) clpfd.c
clpfd_libs.o : clpfd_libs.c term.h basic.h bapi.h event.h frame.h
	$(CCC) $(CFLAGS) clpfd_libs.c
kapi.o : kapi.c term.h basic.h bapi.h kapi.h
	$(CCC) $(CFLAGS) kapi.c
event.o : event.c term.h basic.h bapi.h kapi.h event.h
	$(CCC) $(CFLAGS) event.c
table.o : table.c term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) table.c
debug.o : debug.c term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) debug.c
gcstack.o : gcstack.c gc.h term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) gcstack.c
gcheap.o : gcheap.c gc.h term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) gcheap.c
gQueue.o : gcqueue.c gc.h term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) gcqueue.c
expand.o : expand.c gc.h term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) expand.c
espresso_bp.o : espresso_bp.c term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) -Iespresso espresso_bp.c
sapi.o : sapi.c sapi.h term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) sapi.c
sat_bp.o : sat_bp.c term.h basic.h bapi.h frame.h
	$(CCC) $(CFLAGS) sat_bp.c
mSolver.o : maple/core/Solver.cc
	$(CPPC) $(CFLAGS) -Imaple -o mSolver.o maple/core/Solver.cc
mOptions.o : maple/utils/Options.cc
	$(CPPC) $(CFLAGS) -Imaple -o mOptions.o maple/utils/Options.cc
mSystem.o : maple/utils/System.cc
	$(CPPC) $(CFLAGS) -Imaple -o mSystem.o maple/utils/System.cc
mSimpSolver.o : maple/simp/SimpSolver.cc
	$(CPPC) $(CFLAGS) -Imaple -o mSimpSolver.o maple/simp/SimpSolver.cc
maple_interface.o : maple_interface.cpp
	$(CPPC) $(CFLAGS) -Imaple maple_interface.cpp
black_white.o : espresso/black_white.c
	$(CCC) $(ESPRESSO_FLAGS)  -o black_white.o espresso/black_white.c
canonical.o : espresso/canonical.c
	$(CCC) $(ESPRESSO_FLAGS) -o canonical.o espresso/canonical.c
cofactor.o : espresso/cofactor.c
	$(CCC) $(ESPRESSO_FLAGS) -o cofactor.o espresso/cofactor.c
cols.o : espresso/cols.c
	$(CCC) $(ESPRESSO_FLAGS) -o cols.o espresso/cols.c
compl.o : espresso/compl.c
	$(CCC) $(ESPRESSO_FLAGS) -o compl.o espresso/compl.c
contain.o : espresso/contain.c
	$(CCC) $(ESPRESSO_FLAGS) -o contain.o espresso/contain.c
cpu_time.o : espresso/cpu_time.c
	$(CCC) $(ESPRESSO_FLAGS) -o cpu_time.o espresso/cpu_time.c
cubestr.o : espresso/cubestr.c
	$(CCC) $(ESPRESSO_FLAGS) -o cubestr.o espresso/cubestr.c
cvrin.o : espresso/cvrin.c
	$(CCC) $(ESPRESSO_FLAGS) -o cvrin.o espresso/cvrin.c
cvrm.o : espresso/cvrm.c
	$(CCC) $(ESPRESSO_FLAGS) -o cvrm.o espresso/cvrm.c
cvrmisc.o : espresso/cvrmisc.c
	$(CCC) $(ESPRESSO_FLAGS) -o cvrmisc.o espresso/cvrmisc.c
cvrout.o : espresso/cvrout.c
	$(CCC) $(ESPRESSO_FLAGS) -o cvrout.o espresso/cvrout.c
dominate.o : espresso/dominate.c
	$(CCC) $(ESPRESSO_FLAGS) -o dominate.o espresso/dominate.c
equiv.o : espresso/equiv.c
	$(CCC) $(ESPRESSO_FLAGS) -o equiv.o espresso/equiv.c
espresso.o : espresso/espresso.c
	$(CCC) $(ESPRESSO_FLAGS) -o espresso.o espresso/espresso.c
espresso_expand.o : espresso/espresso_expand.c
	$(CCC) $(ESPRESSO_FLAGS) -o espresso_expand.o espresso/espresso_expand.c
espresso_main.o : espresso/espresso_main.c
	$(CCC) $(ESPRESSO_FLAGS) -o espresso_main.o espresso/espresso_main.c
essen.o : espresso/essen.c
	$(CCC) $(ESPRESSO_FLAGS) -o essen.o espresso/essen.c
essentiality.o : espresso/essentiality.c
	$(CCC) $(ESPRESSO_FLAGS) -o essentiality.o espresso/essentiality.c
exact.o : espresso/exact.c
	$(CCC) $(ESPRESSO_FLAGS) -o exact.o espresso/exact.c
gasp.o : espresso/gasp.c
	$(CCC) $(ESPRESSO_FLAGS) -o gasp.o espresso/gasp.c
gimpel.o : espresso/gimpel.c
	$(CCC) $(ESPRESSO_FLAGS) -o gimpel.o espresso/gimpel.c
globals.o : espresso/globals.c
	$(CCC) $(ESPRESSO_FLAGS) -o globals.o espresso/globals.c
hack.o : espresso/hack.c
	$(CCC) $(ESPRESSO_FLAGS) -o hack.o espresso/hack.c
indep.o : espresso/indep.c
	$(CCC) $(ESPRESSO_FLAGS) -o indep.o espresso/indep.c
irred.o : espresso/irred.c
	$(CCC) $(ESPRESSO_FLAGS) -o irred.o espresso/irred.c
map.o : espresso/map.c
	$(CCC) $(ESPRESSO_FLAGS) -o map.o espresso/map.c
matrix.o : espresso/matrix.c
	$(CCC) $(ESPRESSO_FLAGS) -o matrix.o espresso/matrix.c
mincov.o : espresso/mincov.c
	$(CCC) $(ESPRESSO_FLAGS) -o mincov.o espresso/mincov.c
opo.o : espresso/opo.c
	$(CCC) $(ESPRESSO_FLAGS) -o opo.o espresso/opo.c
pair.o : espresso/pair.c
	$(CCC) $(ESPRESSO_FLAGS) -o pair.o espresso/pair.c
part.o : espresso/part.c
	$(CCC) $(ESPRESSO_FLAGS) -o part.o espresso/part.c
primes.o : espresso/primes.c
	$(CCC) $(ESPRESSO_FLAGS) -o primes.o espresso/primes.c
prtime.o : espresso/prtime.c
	$(CCC) $(ESPRESSO_FLAGS) -o prtime.o espresso/prtime.c
reduce.o : espresso/reduce.c
	$(CCC) $(ESPRESSO_FLAGS) -o reduce.o espresso/reduce.c
rows.o : espresso/rows.c
	$(CCC) $(ESPRESSO_FLAGS) -o rows.o espresso/rows.c
set.o : espresso/set.c
	$(CCC) $(ESPRESSO_FLAGS) -o set.o espresso/set.c
setc.o : espresso/setc.c
	$(CCC) $(ESPRESSO_FLAGS) -o setc.o espresso/setc.c
sharp.o : espresso/sharp.c
	$(CCC) $(ESPRESSO_FLAGS) -o sharp.o espresso/sharp.c
sigma.o : espresso/sigma.c
	$(CCC) $(ESPRESSO_FLAGS) -o sigma.o espresso/sigma.c
signature.o : espresso/signature.c
	$(CCC) $(ESPRESSO_FLAGS) -o signature.o espresso/signature.c
signature_exact.o : espresso/signature_exact.c
	$(CCC) $(ESPRESSO_FLAGS) -o signature_exact.o espresso/signature_exact.c
sminterf.o : espresso/sminterf.c
	$(CCC) $(ESPRESSO_FLAGS) -o sminterf.o espresso/sminterf.c
solution.o : espresso/solution.c
	$(CCC) $(ESPRESSO_FLAGS) -o solution.o espresso/solution.c
sparse.o : espresso/sparse.c
	$(CCC) $(ESPRESSO_FLAGS) -o sparse.o espresso/sparse.c
unate.o : espresso/unate.c
	$(CCC) $(ESPRESSO_FLAGS) -o unate.o espresso/unate.c
util_signature.o : espresso/util_signature.c
	$(CCC) $(ESPRESSO_FLAGS) -o util_signature.o espresso/util_signature.c
verify.o : espresso/verify.c
	$(CCC) $(ESPRESSO_FLAGS) -o verify.o espresso/verify.c
picat_utilities.o : picat_utilities.c
	$(CCC) $(CFLAGS) picat_utilities.c
fann.o : fann/src/fann.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann.c
fann_cascade.o : fann/src/fann_cascade.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann_cascade.c
fann_error.o : fann/src/fann_error.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann_error.c
fann_io.o : fann/src/fann_io.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann_io.c
fann_train.o : fann/src/fann_train.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann_train.c
fann_train_data.o : fann/src/fann_train_data.c
	$(CCC) $(CFLAGS) -Ifann/src/include fann/src/fann_train_data.c
fann_interface.o : fann/fann_interface.cpp
	$(CPPC) $(CFLAGS) -Ifann/src/include fann/fann_interface.cpp

