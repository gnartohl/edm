# -*- makefile -*-
include ../make.setup

.phony : inc
inc :;

# Nothing to configure here, try to resolve problems in make.setup
LIBNAME = $(ODIR)/libPV.so
OBJS   += $(ODIR)/textupdate.o
OBJS   += $(ODIR)/SciPlot.o
OBJS   += $(ODIR)/strip_data.o
OBJS   += $(ODIR)/strip.o
OBJS   += $(ODIR)/regTextupdate.o
OBJS   += $(ODIR)/byte.o
OBJS   += $(ODIR)/pv_register.o

all: 	$(LIBNAME)

buildInstall: $(ODIR) all

$(LIBNAME): $(OBJS)
	$(SL_LINK) -o $(LIBNAME) $(OBJS) $(SL_FLAGS)
	cp $(LIBNAME) $(EPICS_EXTENSIONS)/lib/$(HOST_ARCH)
#	cp $(LIBNAME) /home/csg/devel/ADE/R3.13.3/support/extensions/1-1/lib/solaris

# Dependencies
$(ODIR)/textupdate.o: textupdate.cc

$(ODIR)/SciPlot.o: SciPlot.c SciPlot.h

$(ODIR)/strip_data.o: strip_data.cc strip_data.h

$(ODIR)/strip.o: strip.cc strip.h strip_data.h

$(ODIR)/regTextupdate.o: regTextupdate.cc

$(ODIR)/byte.o: byte.cc byte.h

$(ODIR)/pv_register.o: pv_register.cc

