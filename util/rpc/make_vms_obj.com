$ on error then goto abort
$ set ver
$ cc/noopt/debug/obj=client_rpc_pkg_vms.obj client_rpc_pkg.c + [-]vmsinc/lib
$ cc/noopt/debug/obj=server_rpc_pkg_vms.obj server_rpc_pkg.c + [-]vmsinc/lib
$ abort:
$ set nover
