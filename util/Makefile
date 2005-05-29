TOP = ../../..
ifdef EPICS_HOST_ARCH
include $(TOP)/configure/CONFIG

USR_CFLAGS += -D__osf__=1

USR_INCLUDES += -I$(TOP)/include/sys/linux
USR_INCLUDES += -I$(TOP)/include/net/linux
USR_INCLUDES += -I$(TOP)/include/msg
USR_INCLUDES += -I$(TOP)/include/thread/linux
USR_INCLUDES += -I$(TOP)/include/iprpc/osf
USR_INCLUDES += -I$(TOP)/include/avl
USR_INCLUDES += -I$(TOP)/include/crc

#INSTALL_INCLUDE = $(TOP)/src/edm/include
INC += sys/linux/os.h
INC += sys/linux/os_objs.h
INC += sys/linux/sys_types.h
INC += sys/linux/os_priv.h

INC += net/linux/ipncl.h
INC += net/linux/ipncl_priv.h
INC += net/linux/ipnsv.h
INC += net/linux/ipnsv_priv.h
INC += net/linux/ipport.h
INC += net/linux/ipport_priv.h
INC += net/linux/ipnis.h
INC += net/linux/ipnis_priv.h

INC += msg/msg.h
INC += msg/msg_priv.h

INC += thread/linux/thread.h
INC += thread/linux/thread_priv.h

INC += iprpc/osf/client_iprpc.h
INC += iprpc/osf/iprpc.h
INC += iprpc/osf/iprpc_priv.h
INC += iprpc/osf/server_iprpc_priv.h
INC += iprpc/osf/client_iprpc_priv.h
INC += iprpc/osf/server_iprpc.h

INC += avl/avl.h
INC += avl/avl_priv.h

INC += crc/crc.h




LIBRARY_HOST += 114135a4-6f6c-11d3-95bc-00104b8742df
LIB_LIB += 114135a4-6f6c-11d3-95bc-00104b8742df

SRC_DIRS += ../net/linux
SRC_DIRS += ../sys/linux
SRC_DIRS += ../msg
SRC_DIRS += ../thread/linux
SRC_DIRS += ../iprpc/osf
SRC_DIRS += ../avl
SRC_DIRS += ../crc

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

