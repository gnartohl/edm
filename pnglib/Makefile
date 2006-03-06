TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG
   
   IMAGELIB=/usr/local
   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   
   57d79238-2924-420b-ba67-dfbecdf03fcd_LIBS_Linux += EdmBase cfcaa62e-8199-11d3-a77f-00104b8742df
   57d79238-2924-420b-ba67-dfbecdf03fcd_SYS_LIBS_Linux += Xm Xt Xp Xtst X11 pthread dl png
   
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1
   
   57d79238-2924-420b-ba67-dfbecdf03fcd_LIBS_Darwin += EdmBase cfcaa62e-8199-11d3-a77f-00104b8742df
   57d79238-2924-420b-ba67-dfbecdf03fcd_SYS_LIBS_Darwin += Xm Xt Xp Xtst X11 pthread dl png
   
   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1

   USR_LIBS_solaris = EdmBase cfcaa62e-8199-11d3-a77f-00104b8742df Xm Xt Xmu X11 Xext pthread dl rt png

   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1

   USR_LIBS_hpux11_11_mt = EdmBase cfcaa62e-8199-11d3-a77f-00104b8742df Xm Xt X11 Xext pthread png
   
   
   
   USR_LDFLAGS += -L$(INSTALL_LIB) -L$(IMAGELIB)
   57d79238-2924-420b-ba67-dfbecdf03fcd_LDFLAGS += -L$(MOTIF_LIB) -L$(X11_LIB)
   
   LIBRARY = 57d79238-2924-420b-ba67-dfbecdf03fcd

   PROD_LIBS += ca Com

    USR_INCLUDES = -I../
    USR_INCLUDES += -I$(TOP)/include/edm -I../../util -I../../lib -I../../pvlib

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
   
  
   LIB_SRCS += png.cc
   LIB_SRCS += reg_lib57d79238-2924-420b-ba67-dfbecdf03fcd.cc
   
   

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

