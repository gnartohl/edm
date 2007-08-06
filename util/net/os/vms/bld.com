$ on error then goto done
$ set ver
$ cc/noopt/deb client_test.c + jwsutils:vmsinc/lib
$ cc/noopt/deb server_test.c + jwsutils:vmsinc/lib
$ link/deb client_test, jwsutils:vms_util/lib, vsys_lib:vdb_shrlib/lib, -
   sys$disk:[]vms/opt
$ link/deb server_test, jwsutils:vms_util/lib, vsys_lib:vdb_shrlib/lib, -
   sys$disk:[]vms/opt
$ pur
$ done:
$ set nover
