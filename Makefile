# Makefile for edm top level
TOP = ../..
ifdef EPICS_HOST_ARCH
 include $(TOP)/configure/CONFIG
 DIRS += util 
 DIRS += lib 
 DIRS += epicsPv
 DIRS += logPv
 DIRS += proxyPv
 DIRS += calcPv
 DIRS += locPv
 DIRS += baselib 
 DIRS += edmMain 
 DIRS += giflib 
 DIRS += pnglib 
 DIRS += pvFactory 
 DIRS += choiceButton
 DIRS += videowidget
 DIRS += triumflib
 DIRS += diamondlib
 DIRS += indicator
 DIRS += multiSegRampButton
 DIRS += slaclib
 include $(TOP)/configure/RULES_DIRS

else
    TOP = ../..
    ifneq ($(wildcard $(TOP)/config)x,x)
      # New Makefile.Host config file location
      include $(TOP)/config/CONFIG_EXTENSIONS
      DIRS = util lib epicsPv calcPv locPv baselib edmMain giflib pnglib pvFactory choiceButton
      include $(TOP)/config/RULES_DIRS
    else
      # Old Makefile.Unix config file location
      EPICS=../../..
      include $(EPICS)/config/CONFIG_EXTENSIONS
      DIRS = util lib epicsPv calcPv locPv baselib edmMain giflib pnglib pvFactory choiceButton
      include $(EPICS)/config/RULES_DIRS
    endif
endif
