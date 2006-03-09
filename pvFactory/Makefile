TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CFLAGS_Linux = $(USR_CXXFLAGS_Linux)

   #USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CFLAGS_Darwin = $(USR_CXXFLAGS_Darwin)
   
   
   # Add Debugging
   #USR_CXXFLAGS_Linux += -g -O0
   

   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_solaris = cfcaa62e-8199-11d3-a77f-00104b8742df pthread dl

   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_hpux11_11_mt = cfcaa62e-8199-11d3-a77f-00104b8742df pthread

   USR_LDFLAGS += -L$(INSTALL_LIB)

   #USR_INCLUDES += -I$(MOTIF_INC) -I$(X11_INC)
   USR_INCLUDES += -I/usr/X11R6/include/Xm -I/usr/X11R6/include


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
   
  
   LIBRARY = PV

   PROD_LIBS += ca Com

   USR_INCLUDES = -I../
   USR_INCLUDES += -I$(TOP)/include/edm -I../../util -I../../pvlib -I../../lib

   LIB_SRCS   += textupdate.cc
   LIB_SRCS   += SciPlot.cc
   LIB_SRCS   += strip_data.cc
   LIB_SRCS   += strip.cc
   LIB_SRCS   += regTextupdate.cc
   LIB_SRCS   += byte.cc
   LIB_SRCS   += pv_register.cc
   
   
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

