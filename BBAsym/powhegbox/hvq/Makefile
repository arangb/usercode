#-*- Makefile -*-
ifeq ($(shell uname),Linux)
## -fbounds-check causes a weird error due to non-lazy evaluation
## of boolean in gfortran.
## As of gfortran 4.4.1 optimizing with -O3 yields erroneous results
## Use -O2
#F77= gfortran -Wall -fno-automatic #-ffpe-trap=invalid,zero,overflow
#OPT=-O2
## For g77 comment the above and uncomment below
F77= g77 -Wall -fno-automatic -ffortran-bounds-check
OPT=-O3
# For debugging uncomment the following
#DEBUG= -ggdb -pg
#Do not uncomment this with gfortran
#SYSOBJ=trapfpe.o
endif

FASTJET_CONFIG=fastjet-config

CXXFLAGS += $(shell $(FASTJET_CONFIG) --cxxflags)
LIBSFASTJET += $(shell $(FASTJET_CONFIG) --libs --plugins ) -lstdc++
FJCXXFLAGS+= $(shell $(FASTJET_CONFIG) --cxxflags)

PWD=$(shell pwd)

WDNAME=$(shell basename $(PWD))

VPATH= ./:../:obj/

FF=$(F77) $(OPT) $(DEBUG)


INCLUDE =$(shell echo ../include/*.h *.h)

LHAPDF_CONFIG=lhapdf-config
PDFPACK=lhapdfif.o
LIBSLHAPDF=-L$(shell $(LHAPDF_CONFIG) --prefix)//lib -lLHAPDF
LIBS=$(LIBSLHAPDF)
# In order to use mlmpdf uncomment the following and comment previous lines
#LIBS=
# PDFPACK=mlmpdfif.o hvqpdfpho.o

# Uncomment the following with the appropriate location of libstdc++.a for static link
# STATIC= -lstdc++ -static


%.o: %.f $(INCLUDE)
	ln -sf $(WDNAME)/nlegborn.h ../
	$(FF) -c -o obj/$@ $<

%.o: %.c
	$(CC) $(DEBUG) -c -o obj/$@ $^ 

%.o: %.cc
	$(CC) $(DEBUG) -c -o obj/$@ $^ $(FJCXXFLAGS)


USER=init_couplings.o init_processes.o Born_phsp.o Born.o virtual.o \
		      real.o ttdec.o

PWHG=pwhg_main.o pwhg_init.o bbinit.o btilde.o lhefwrite.o LesHouches.o \
	LesHouchesreg.o gen_Born_phsp.o find_regions.o test_Sudakov.o \
        pt2maxreg.o \
	sigborn.o gen_real_phsp.o maxrat.o gen_index.o gen_radiation.o \
	Bornzerodamp.o sigremnants.o random.o boostrot.o \
	bra_ket_subroutines.o cernroutines.o init_phys.o powheginput.o \
        pdfcalls.o  sigreal.o sigcollremn.o \
        pwhg_bookhist.o pwhg_analysis_driver.o checkmomzero.o \
	setstrongcoupl.o integrator.o newunit.o mwarn.o \
	sigsoftvirt.o sigcollsoft.o sigvirtual.o \
	$(PDFPACK) $(USER) $(SYSOBJ)

# target to generate LHEF output; top analysis
pwhg_main-top:$(PWHG) pwhg_analysis-top.o  fastjetfortran.o
	$(FF) $(patsubst %,obj/%,$(PWHG) pwhg_analysis-top.o fastjetfortran.o) $(LIBS) $(LIBSFASTJET) -o $@ $(STATIC)

# target to generate LHEF output; dummy analysis (good for any heavy quark)
pwhg_main:$(PWHG) pwhg_analysis-top.o
	$(FF) $(patsubst %,obj/%,$(PWHG) pwhg_analysis-top.o fastjetfortran.o) $(LIBS) $(LIBSFASTJET) -o $@ $(STATIC)




LHEFANAL=lhef_analysis.o boostrot.o random.o cernroutines.o  opencount.o \
	powheginput.o pwhg_bookhist.o lhefread.o newunit.o \
	fastjetsisconewrap.o fastjetktwrap.o pwhg_analysis_driver.o \
	$(SYSOBJ)

# target to analyze LHEF output
lhef_analysis-top:$(LHEFANAL)  pwhg_analysis-top.o fastjetfortran.o
	$(FF) $(patsubst %,obj/%,$(LHEFANAL)  pwhg_analysis-top.o fastjetfortran.o) -o $@ $(LIBS) $(LIBSFASTJET)

# target to read event file, shower events with HERWIG + analysis
HERWIG=main-HERWIG-lhef.o herwig6510.o \
	boostrot.o powheginput.o \
        pwhg_bookhist.o lhefread.o newunit.o \
	fastjetsisconewrap.o fastjetktwrap.o pwhg_analysis_driver.o \
	random.o cernroutines.o opencount.o  $(SYSOBJ)

main-HERWIG-lhef: $(HERWIG) pwhg_analysis-top.o fastjetfortran.o setup_herwig.o
	$(FF) $(patsubst %,obj/%,$(HERWIG) pwhg_analysis-top.o) -o $@ $(LIBSFASTJET)

# target to read event file, shower events with PYTHIA + analysis
PYTHIA=main-PYTHIA-lhef.o pythia-6.4.21.o \
	boostrot.o powheginput.o \
        pwhg_bookhist.o lhefread.o newunit.o \
	fastjetsisconewrap.o fastjetktwrap.o pwhg_analysis_driver.o \
	random.o cernroutines.o opencount.o  $(SYSOBJ)

main-PYTHIA-lhef-top: $(PYTHIA) pdfdummies.o  pwhg_analysis-top.o fastjetfortran.o setup_pythia.o
	$(FF) $(patsubst %,obj/%,$(PYTHIA) pwhg_analysis-top.o fastjetfortran.o setup_pythia.o) -o $@ $(LIBSFASTJET) pdfdummies.o

clean:
	rm -f obj/*.o pwhg_main lhef_analysis main-HERWIG-lhef \
	main-PYTHIA-lhef
