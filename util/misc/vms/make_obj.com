$ on error then goto abort
$ set ver
$ cc/noopt/debug PARAM_PKG.C +[--]vmsinc/lib
$ abort:
$ set nover
