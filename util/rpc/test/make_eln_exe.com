$ on error then goto abort
$ set ver
$ link/debug/nosyslib/nosysshr/exe=client_test_eln.exe -
   client_test_eln.obj, rpc_app_eln.obj, -
   [-]eln_rpc_util.olb/lib, -
   eln$:crtlshare/lib+frtlobject/lib+rtlshare/lib+rtl/lib
$ link/debug/nosyslib/nosysshr/exe=server_test_eln.exe -
   server_test_eln.obj, [-]eln_rpc_util.olb/lib, -
   eln$:crtlshare/lib+frtlobject/lib+rtlshare/lib+rtl/lib
$ abort:
$ set nover
