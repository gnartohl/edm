TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   SHARED_LIBRARIES = YES

   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_Darwin += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1


   INSTALL_INCLUDE = $(INSTALL_LOCATION)/include/edm
   INC += pv.h
   INC += epicsPv.h

   Epics_LIBS += cfcaa62e-8199-11d3-a77f-00104b8742df

   Epics_SYS_LIBS_Linux += pthread dl
   Epics_SYS_LIBS_Darwin += pthread dl
   Epics_SYS_LIBS_solaris += pthread dl 
   Epics_SYS_LIBS_hpux11_11_mt += pthread

   LIBRARY = Epics

   USR_LIBS_DEFAULT = ca Com
   USR_LIBS_Darwin = -nil-  

   USR_INCLUDES += -I../../lib


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

