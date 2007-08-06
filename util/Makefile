TOP = ../../..
ifdef EPICS_HOST_ARCH
include $(TOP)/configure/CONFIG

EDMTOP = $(TOP)/src/edm

SHARED_LIBRARIES = YES

USR_CFLAGS += -D__osf__=1


INSTALL_INCLUDE = $(INSTALL_LOCATION)/include/edm

SRC_DIRS += $(EDMTOP)/util/sys
INC += os.h
INC += os_objs.h
INC += sys_types.h
INC += os_priv.h


SRC_DIRS += $(EDMTOP)/util/net
INC += ipncl.h
INC += ipncl_priv.h
INC += ipnsv.h
INC += ipnsv_priv.h
INC += ipport.h
INC += ipport_priv.h
INC += ipnis.h
INC += ipnis_priv.h

SRC_DIRS += $(EDMTOP)/util/msg
INC += msg.h
INC += msg_priv.h


SRC_DIRS += $(EDMTOP)/util/thread
INC += thread.h
INC += thread_priv.h


SRC_DIRS += $(EDMTOP)/util/iprpc/osf
INC += client_iprpc.h
INC += iprpc.h
INC += iprpc_priv.h
INC += server_iprpc_priv.h
INC += client_iprpc_priv.h
INC += server_iprpc.h

SRC_DIRS += $(EDMTOP)/util/avl
INC += avl.h
INC += avl_priv.h


SRC_DIRS += $(EDMTOP)/util/crc
INC += crc.h

LIBRARY += 114135a4-6f6c-11d3-95bc-00104b8742df

LIB_SRCS += ipncl_pkg.c
LIB_SRCS += ipnsv_pkg.c
LIB_SRCS += ipnis_pkg.c

LIB_SRCS += sys_pkg.c
LIB_SRCS += os_pkg.c

LIB_SRCS += msg_pkg.c

LIB_SRCS += thread_pkg.c

LIB_SRCS += server_iprpc_pkg.c
LIB_SRCS += client_iprpc_pkg.c

LIB_SRCS += avl.c
LIB_SRCS += crc.c

include $(TOP)/configure/RULES


else
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
   
endif

