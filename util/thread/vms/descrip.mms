test.exe : test.obj
  link/deb test, jwsutils:vms_util/lib, vms/opt
  pur

test.obj : test.c
  cc/deb/noopt test.c + jwsutils:vmsinc/lib
