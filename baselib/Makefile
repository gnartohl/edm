TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG

   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   # Add Debugging
   #USR_CXXFLAGS_Linux += -g -O0
   
   USR_CXXFLAGS_Darwin += -DSINGLE_THREADED=1 -D__epics__=1

   EdmBase_LIBS_Linux += cfcaa62e-8199-11d3-a77f-00104b8742df 
   EdmBase_SYS_LIBS_Linux += Xm Xt Xp Xtst X11 pthread dl

   EdmBase_LIBS_Darwin += cfcaa62e-8199-11d3-a77f-00104b8742df 
   EdmBase_SYS_LIBS_Darwin += Xm Xt Xp Xtst X11 pthread dl

   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_solaris = cfcaa62e-8199-11d3-a77f-00104b8742df Xm Xt Xmu X11 Xext pthread dl rt

   USR_CXXFLAGS_hpux11_11_mt += +W652 -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_hpux11_11_mt = cfcaa62e-8199-11d3-a77f-00104b8742df Xm Xt Xp Xtst X11 pthread

   USR_LDFLAGS += -L$(INSTALL_LIB) -L$(INSTALL_BIN) 
   EdmBase_LDFLAGS += -L$(MOTIF_LIB) -L$(X11_LIB)
   
   LIBRARY = EdmBase

   PROD_LIBS += ca Com
   
    USR_INCLUDES = -I../ 
    USR_INCLUDES += -I$(TOP)/include/edm -I../../util -I../../pvlib -I../../lib

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
   
   LIB_SRCS += line_obj.cc
   LIB_SRCS += rectangle_obj.cc
   LIB_SRCS += circle_obj.cc
   LIB_SRCS += arc_obj.cc
   LIB_SRCS += x_text_obj.cc
   LIB_SRCS += x_regtext_obj.cc
   LIB_SRCS += meter.cc
   LIB_SRCS += bar.cc
   LIB_SRCS += x_text_dsp_obj.cc
   LIB_SRCS += button.cc
   LIB_SRCS += menu_button.cc
   LIB_SRCS += message_button.cc
   LIB_SRCS += exit_button.cc
   LIB_SRCS += menu_mux.cc
   LIB_SRCS += related_display.cc
   LIB_SRCS += message_box.cc
   LIB_SRCS += slider.cc
   LIB_SRCS += shell_cmd.cc
   LIB_SRCS += updownButton.cc
   LIB_SRCS += reg_libEdmBase.cc
   LIB_SRCS += motifSlider.cc
   LIB_SRCS += radio_button.cc
   LIB_SRCS += xygraph.cc
   LIB_SRCS += pip.cc
   LIB_SRCS += pv_inspector.cc
   LIB_SRCS += table.cc
   
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
