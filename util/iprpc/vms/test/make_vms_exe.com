$ on error then goto abort
$ set ver
$ link/debug/exe=CLIENT_TEST_vms.exe -
  CLIENT_TEST_vms.OBJ, RPC_APP_vms.OBJ, [--]VMS_UTIL.OLB/lib, -
  sys$input:/opt
vsys_share:vsysvmsrtl.exe/share
sys$share:vaxcrtl.exe/share
sys$share:cma$open_lib_shr.exe/share
sys$share:cma$open_rtl.exe/share
MULTINET_COMMON_ROOT:[MULTINET]MULTINET_SOCKET_LIBRARY.EXE/share
$ link/debug/exe=SERVER_TEST_vms.exe -
  SERVER_TEST_vms.OBJ, [--]VMS_UTIL.OLB/lib, sys$input:/opt
vsys_share:vsysvmsrtl.exe/share
sys$share:vaxcrtl.exe/share
sys$share:cma$open_lib_shr.exe/share
sys$share:cma$open_rtl.exe/share
MULTINET_COMMON_ROOT:[MULTINET]MULTINET_SOCKET_LIBRARY.EXE/share
$ abort:
$ set nover
