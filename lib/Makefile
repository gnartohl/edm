TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG
  #HOST_OPT=NO
  # If you want support accessing files via http and curl
  # then set USE_CURL=YES
  USE_CURL=NO

  ifeq ($(USE_CURL), YES)
        USR_CXXFLAGS_Linux += -DUSECURL -DSINGLE_THREADED=1 -D__epics__=1 -pedantic
  else
        USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
  endif

  # Add debugging
  #USR_CXXFLAGS_Linux += -g -O0

  ifeq ($(USE_CURL), YES)
        cfcaa62e-8199-11d3-a77f-00104b8742df_SYS_LIBS_Linux = Xm Xt Xp Xtst X11 pthread dl curl
  else
        cfcaa62e-8199-11d3-a77f-00104b8742df_SYS_LIBS_Linux = Xm Xt Xp Xtst X11 pthread dl
  endif


  USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
  USR_LIBS_solaris = Xm Xt Xmu X11 Xext pthread dl rt

  USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
  USR_LIBS_hpux11_11_mt = Xm Xt X11 Xext pthread

  USR_LDFLAGS += -L$(INSTALL_LIB)
  cfcaa62e-8199-11d3-a77f-00104b8742df_LDFLAGS += -L$(MOTIF_LIB) -L$(X11_LIB)

  # Add Debugging
  #USR_LDFLAGS += -g

  # Build Shared library
  LIBRARY = cfcaa62e-8199-11d3-a77f-00104b8742df

  PROD_LIBS += ca Com

  USR_INCLUDES = -I../ -I../../util -I../../pvlib
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

  LIB_SRCS += clipbd.cc
  LIB_SRCS += process.cc
  LIB_SRCS += app_pkg.cc
  LIB_SRCS += act_win.cc
  LIB_SRCS += act_grf.cc
  LIB_SRCS += font_pkg.cc
  LIB_SRCS += color_pkg.cc
  LIB_SRCS += font_menu.cc
  LIB_SRCS += utility.cc
  LIB_SRCS += pvColor.cc
  LIB_SRCS += expString.cc
  LIB_SRCS += cursor.cc
  LIB_SRCS += color_button.cc
  LIB_SRCS += gc_pkg.cc
  LIB_SRCS += entry_form.cc
  LIB_SRCS += confirm_dialog.cc
  LIB_SRCS += bindings.cc
  LIB_SRCS += scheme.cc
  LIB_SRCS += scrolled_text.cc
  LIB_SRCS += pvs.cc
  LIB_SRCS += scrolled_list.cc
  LIB_SRCS += ulBindings.cc
  LIB_SRCS += pvBindings.cc
  LIB_SRCS += pvConnection.cc
  LIB_SRCS += group.cc
  LIB_SRCS += symbol.cc
  LIB_SRCS += dynSymbol.cc
  LIB_SRCS += keypad.cc
  LIB_SRCS += calpad.cc
  LIB_SRCS += fileSelect.cc
  LIB_SRCS += undo.cc
  LIB_SRCS += color_list.cc
  LIB_SRCS += msg_dialog.cc
  LIB_SRCS += loc_pv_factory.cc
  LIB_SRCS += calc_pv_factory.cc
  LIB_SRCS += epics_pv_factory.cc
  LIB_SRCS += pv_factory.cc
  LIB_SRCS += edmPrint.cc
  LIB_SRCS += tag_pkg.cc
  LIB_SRCS += remFileOpen.cc
  LIB_SRCS += tableObj.cc
  LIB_SRCS += scrolledWAcceleratorSupport.c

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

