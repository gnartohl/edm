$ on error then goto abort
$ set ver
$ cc/noopt/debug SYS_PKG.C +[--]vmsinc/lib
$ abort:
$ set nover
