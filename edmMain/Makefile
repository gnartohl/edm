TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   USR_CXXFLAGS += -std=c++0x -fpermissive -Wno-narrowing -Wno-write-strings -Wno-unused-but-set-variable
   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1 -Wno-long-long
   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1

   USR_LDFLAGS_Linux += -Wl,--no-as-needed

   # Add Debugging
   #USR_CXXFLAGS_Linux += -g -O0

   edm_LIBS += cfcaa62e-8199-11d3-a77f-00104b8742df 
   edm_LIBS += 114135a4-6f6c-11d3-95bc-00104b8742df

   edm_LIBS_Linux += Xm Xmu Xt Xtst X11
   edm_SYS_LIBS_Linux += pthread dl
   
   edm_LIBS_Darwin += Xm Xmu Xt Xtst X11
   edm_SYS_LIBS_Darwin += pthread dl

   edm_LIBS_solaris += Xm Xt Xmu X11 Xext
   edm_SYS_LIBS_solaris += pthread dl rt socket nsl

   USR_LIBS_hpux11_11_mt = Xm Xt Xmu X11 Xext
   USR_SYS_LIBS_hpux11_11_mt = pthread

   PROD = edm

#  AJF: For solaris builds
#  OP_SYS_LDFLAGS = -z ignore -z combreloc -z lazyload
#  This is set in "base/configure/os/CONFIG.solaris-sparc-gnu.solaris-sparc-gnu
#  However, the presence of the "-z ignore" breaks the "edm" build and you end up
#  with the following message when trying to run "edm": Error: No realize class procedure defined.
#
#  From the man page:
#  -z ignore | record
#
#  Ignores, or records, dynamic dependencies that are not referenced as part of the link-edit. 
#  Ignores, or records, unreferenced ELF sections from the relocatable objects that are read as 
#  part of the link-edit. By default, -z record is in effect.
#
#  If an ELF section is ignored, the section is eliminated from the output file being generated. 
#  A section is ignored when three conditions are true. The eliminated section must contribute to 
#  an allocatable segment. The eliminated section must provide no global symbols. No other section 
#  from any object that contributes to the link-edit, must reference an eliminated section. 
#
#  Therefore, to build for "solaris", set OP_SYS_LDFLAGS as follows:
   OP_SYS_LDFLAGS = -z combreloc -z lazyload 

   PROD_LIBS += ca Com
   
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
   
   edm_SRCS += main.cc

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

