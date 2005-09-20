TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG
  #HOST_OPT=NO
  
        USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1 -Wno-deprecated
  
# System Libraries to link in:
  TwoDProfileMonitor_SYS_LIBS_Linux = Xpm
 
  USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1 -Wno-deprecated
  USR_LIBS_solaris =  Xpm

  USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1 -Wno-deprecated
  USR_LIBS_hpux11_11_mt = Xpm

  USR_LDFLAGS += -L$(INSTALL_LIB)
  TwoDProfileMonitor_LDFLAGS += -L$(MOTIF_LIB) -L$(X11_LIB)

 

  # Build Shared library
  LIBRARY = TwoDProfileMonitor

  
  USR_INCLUDES = -I../ -I../../util -I../../lib -I../../pvlib
  USR_INCLUDES += -I$(TOP)/include/sys/linux
  USR_INCLUDES += -I$(TOP)/include/net/linux
  USR_INCLUDES += -I$(TOP)/include/msg
  USR_INCLUDES += -I$(TOP)/include/thread/linux
  USR_INCLUDES += -I$(TOP)/include/iprpc/osf
  USR_INCLUDES += -I$(TOP)/include/avl
  USR_INCLUDES += -I$(TOP)/include/crc
  
  USR_INCLUDES += -I$(MOTIF_INC) -I$(X11_INC)

  Xm_DIR =   $(MOTIF_LIB)
  Xt_DIR =   $(X11_LIB)
  X11_DIR =  $(X11_LIB)
  Xmu_DIR =  $(X11_LIB)
  Xtst_DIR = $(X11_LIB)
  
  # ELW added following two lines for
  # Red Hat 7.0 LINUX
  Xp_DIR =   $(X11_LIB)
  Xext_DIR = $(X11_LIB)
  #
  XlibGUI_DIR = $(X11_LIB)
  XmStatic_DIR = $(MOTIF_LIB)
  XmStatXt_DIR = $(MOTIF_LIB)
  HCLXt_DIR = $(X11_LIB)
  Xlib_DIR = $(X11_LIB)
  HCLXmu_DIR = $(X11_LIB)

  LIB_SRCS += TwoDProfileMonitor.cc
  LIB_SRCS += widget.cc
  

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
