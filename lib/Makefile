TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG
  #HOST_OPT=NO
  # If you want support accessing files via http and curl
  # then set USE_CURL=YES
  USE_CURL=NO

  ifeq ($(USE_CURL), YES)
        USR_CXXFLAGS += -std=c++0x -fpermissive -Wno-narrowing -Wno-write-strings -Wno-unused-but-set-variable
        USR_CXXFLAGS_Darwin += -DUSECURL -DSINGLE_THREADED=1 -D__epics__=1 -pedantic -Wno-long-long
        USR_CXXFLAGS_Linux += -DUSECURL -DSINGLE_THREADED=1 -D__epics__=1 -pedantic
	USR_CXXFLAGS_solaris += -D__solaris__ -DUSECURL -DSINGLE_THREADED=1 -D__epics__=1 -pedantic
  else
        USR_CXXFLAGS += -std=c++0x -fpermissive -Wno-narrowing -Wno-write-strings -Wno-unused-but-set-variable
        USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1 -pedantic -Wno-long-long
        USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
        USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
  endif

  SHARED_LIBRARIES = YES

  # Add debugging
  #USR_CXXFLAGS_Linux += -g -O0

  USR_LIBS_Darwin += Xm Xt Xtst X11
  USR_SYS_LIBS_Darwin += pthread dl

  USR_LIBS_Linux += Xm Xt Xtst X11
  USR_SYS_LIBS_Linux += pthread dl

  USR_LIBS_solaris += Xm Xt Xmu X11 Xext
  USR_SYS_LIBS_solaris += pthread dl rt

  ifeq ($(USE_CURL), YES)
        USR_SYS_LIBS_Darwin += curl
        USR_SYS_LIBS_Linux +=  curl 
	USR_SYS_LIBS_solaris += curl
  endif

  INSTALL_INCLUDE = $(INSTALL_LOCATION)/include/edm

  INC += act_grf.h
  INC += act_win.h
  INC += app_pkg.h
  INC += bindings.h
  INC += btnActionListType.h
  INC += calpad.h
  INC += clipbd.h
  INC += color_button.h
  INC += color_helper.h
  INC += color_list.h
  INC += color_pkg.h
  INC += confirm_dialog.h
  INC += cursor.h
  INC += dl_list.h
  INC += dynSymbol.h
  INC += edmPrint.h
  INC += entry_form.h
  INC += expString.h
  INC += fileSelect.h
  INC += font_menu.h
  INC += font_pkg.h
  INC += gc_pkg.h
  INC += group.h
  INC += hashtable.h
  INC += keypad.h
  INC += msg_dialog.h
  INC += process.h
  INC += pvColor.h
  INC += pvConnection.h
  INC += remFileOpen.h
  INC += scheme.h
  INC += scrolled_list.h
  INC += scrolled_text.h
  INC += symbol.h
  INC += asymbol.h
  INC += tableObj.h
  INC += tag_pkg.h
  INC += ulBindings.h
  INC += undo.h
  INC += utility.h
  INC += path_list.h
  INC += crawler.h
  INC += lookup.h
  INC += pv_action.h
  INC += pv_factory.h
  INC += pvBindings.h
  INC += dimDialog.h

  USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
  USR_LIBS_hpux11_11_mt = Xm Xt X11 Xext pthread

  # Build Shared library
  LIBRARY = cfcaa62e-8199-11d3-a77f-00104b8742df

  USR_LIBS += 114135a4-6f6c-11d3-95bc-00104b8742df
  LIB_LIBS_DEFAULT += ca Com
  LIB_LIBS_Darwin += -nil-

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
  LIB_SRCS += pvConnection.cc
  LIB_SRCS += group.cc
  LIB_SRCS += symbol.cc
  LIB_SRCS += asymbol.cc
  LIB_SRCS += dynSymbol.cc
  LIB_SRCS += keypad.cc
  LIB_SRCS += calpad.cc
  LIB_SRCS += fileSelect.cc
  LIB_SRCS += undo.cc
  LIB_SRCS += color_list.cc
  LIB_SRCS += msg_dialog.cc
  LIB_SRCS += edmPrint.cc
  LIB_SRCS += tag_pkg.cc
  LIB_SRCS += remFileOpen.cc
  LIB_SRCS += tableObj.cc
  LIB_SRCS += scrolledWAcceleratorSupport.c
  LIB_SRCS += path_list.cc
  LIB_SRCS += crawler.cc
  LIB_SRCS += lookup.cc
  LIB_SRCS += pv_action.cc
  LIB_SRCS += pv_factory.cc
  LIB_SRCS += pvBindings.cc
  LIB_SRCS += dimDialog.cc

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

