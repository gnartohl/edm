TOP = ../../..
ifneq ($(wildcard $(TOP)/config)x,x)
  # New Makefile.Host config file location
  include $(TOP)/config/CONFIG_EXTENSIONS
  include $(TOP)/config/RULES_ARCHS
else
  # Old Makefile.Unix config file location
  EPICS=../../../..
  include $(EPICS)/config/CONFIG_EXTENSIONS
  include $(EPICS)/config/RULES_ARCHS
endif

   USR_CXXFLAGS += -std=c++0x -fpermissive -Wno-narrowing -Wno-write-strings -Wno-unused-but-set-variable
