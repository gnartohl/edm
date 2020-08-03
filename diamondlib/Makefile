TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   SHARED_LIBRARIES = YES
   
   USR_CXXFLAGS += -std=c++0x -fpermissive -Wno-narrowing -Wno-write-strings -Wno-unused-but-set-variable
   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1 -Wno-long-long
   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1

   EdmDiamond_LIBS += EdmBase cfcaa62e-8199-11d3-a77f-00104b8742df
   EdmDiamond_LIBS += 114135a4-6f6c-11d3-95bc-00104b8742df
   EdmDiamond_LIBS += Com

   EdmDiamond_LIBS_Linux += Xm Xt Xtst X11
   EdmDiamond_SYS_LIBS_Linux += pthread dl

   EdmDiamond_LIBS_Darwin += Xm Xt Xtst X11
   EdmDiamond_SYS_LIBS_Darwin += pthread dl

   EdmDiamond_LIBS_solaris +=  Xm Xt Xmu X11 Xext
   EdmDiamond_SYS_LIBS_solaris +=  pthread dl rt

   USR_LIBS_hpux11_11_mt = Xm Xt X11 Xext
   USR_SYS_LIBS_hpux11_11_mt = pthread

   LIBRARY = EdmDiamond 

   INSTALL_INCLUDE = $(INSTALL_LOCATION)/include/edm
   USR_INCLUDES += -I../../util -I../../lib

   USR_INCLUDES += -I$(MOTIF_INC) -I$(X11_INC)


   Xm_DIR =   $(MOTIF_LIB)
   Xt_DIR =   $(X11_LIB)
   X11_DIR =  $(X11_LIB)
   Xmu_DIR =  $(X11_LIB)
   Xtst_DIR = $(X11_LIB)

   # ELW added following two lines for
   # Red Hat 7.0 LINUX
   Xext_DIR = $(X11_LIB)
   #
   XlibGUI_DIR = $(X11_LIB)
   XmStatic_DIR = $(MOTIF_LIB)
   XmStatXt_DIR = $(MOTIF_LIB)
   HCLXt_DIR = $(X11_LIB)
   Xlib_DIR = $(X11_LIB)
   HCLXmu_DIR = $(X11_LIB)
   
   LIB_SRCS += multiLineTextUpdate.cc
   LIB_SRCS += diamondWidgets.cc
   
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

