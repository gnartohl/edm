$ on error then goto abort
$ set ver
$ cc/noopt/debug/obj=client_iprpc_pkg_vms.obj client_iprpc_pkg.c + [--]vmsinc/lib
$ cc/noopt/debug/obj=server_iprpc_pkg_vms.obj server_iprpc_pkg.c + [--]vmsinc/lib
$ abort:
$ set nover
