# -*- makefile -*-
include ../make.setup

# Nothing to configure here, try to resolve problems in make.setup
LIBNAME = $(ODIR)/libPV.so
OBJS    = $(ODIR)/pv_factory.o
OBJS   += $(ODIR)/epics_pv_factory.o
OBJS   += $(ODIR)/textupdate.o
OBJS   += $(ODIR)/SciPlot.o
OBJS   += $(ODIR)/strip_data.o
OBJS   += $(ODIR)/strip.o
OBJS   += $(ODIR)/pv_register.o

all: 	$(LIBNAME)

buildInstall: all

$(LIBNAME): $(OBJS)
	$(SL_LINK) -o $(LIBNAME) $(OBJS) $(SL_FLAGS)

# Dependencies
$(ODIR)/pv_factory.o: pv_factory.cc pv_factory.h

$(ODIR)/epics_pv_factory.o: epics_pv_factory.cc epics_pv_factory.h

$(ODIR)/textupdate.o: textupdate.cc

$(ODIR)/SciPlot.o: SciPlot.c SciPlot.h

$(ODIR)/strip_data.o: strip_data.cc strip_data.h

$(ODIR)/strip.o: strip.cc strip.h strip_data.h

$(ODIR)/pv_register.o: pv_register.cc

