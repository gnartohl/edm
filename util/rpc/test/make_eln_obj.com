$ on error then goto abort
$ set ver
$ cc/noopt/debug/obj=client_test_eln.obj client_test.c -
   + [-]elninc/lib + eln$:vaxelnc/lib
$ cc/noopt/debug/obj=rpc_app_eln.obj rpc_app.c -
   + [-]elninc/lib + eln$:vaxelnc/lib
$ cc/noopt/debug/obj=server_test_eln.obj server_test.c -
   + [-]elninc/lib + eln$:vaxelnc/lib
$ abort:
$ set nover
