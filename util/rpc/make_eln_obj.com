$ on error then goto abort
$ set ver
$ cc/noopt/debug/obj=client_rpc_pkg_eln.obj -
   client_rpc_pkg.c +[-]elninc/lib + eln$:vaxelnc/lib
$ cc/noopt/debug/obj=server_rpc_pkg_eln.obj -
   server_rpc_pkg.c +[-]elninc/lib + eln$:vaxelnc/lib
$ abort:
$ set nover
