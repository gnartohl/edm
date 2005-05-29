TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   # Add Debugging
   #USR_CXXFLAGS_Linux += -g -O0
   Epics_LIBS_Linux += cfcaa62e-8199-11d3-a77f-00104b8742df
   Epics_SYS_LIBS_Linux += pthread dl

   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_solaris = cfcaa62e-8199-11d3-a77f-00104b8742df pthread dl

   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_hpux11_11_mt = cfcaa62e-8199-11d3-a77f-00104b8742df pthread

   USR_LDFLAGS += -L$(INSTALL_LIB)

   LIBRARY = Epics

   PROD_LIBS += ca Com

   USR_INCLUDES = -I../ -I../../lib

   LIB_SRCS += reg_pvs.cc
   LIB_SRCS += epicsPv.cc
   LIB_SRCS += pv.cc
   
  include $(TOP)/configure/RULES
   
else
   ifneq ($(wildcard $(TOP)/config)x,x)
     # New Makefile.Host config file location
     include $(TOP)/config/CONFIG_EXTENSIONS
     include $(TOP)/config/RULES_ARCHS
   else
     # Old Makefile.Unix config file location
     EPICS=../../..
     include $(EPICS)/config/CONFIG_EXTENSIONS
     include $(EPICS)/config/RULES_ARCHS
   endif
endif

