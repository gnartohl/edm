$ on error then goto abort
$ set ver
$ cc/noopt/debug/obj=CLIENT_TEST_vms.obj CLIENT_TEST.C +[--]vmsinc/lib
$ cc/noopt/debug/obj=RPC_APP_vms.obj RPC_APP.C +[--]vmsinc/lib
$ cc/noopt/debug/obj=SERVER_TEST_vms.obj SERVER_TEST.C +[--]vmsinc/lib
$ abort:
$ set nover
