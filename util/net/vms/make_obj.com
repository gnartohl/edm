$ on error then goto abort
$ set ver
$ cc/noopt/debug NSV_PKG.C +[--]vmsinc/lib
$ cc/noopt/debug IPNSV_PKG.C +[--]vmsinc/lib
$ cc/noopt/debug NCL_PKG.C +[--]vmsinc/lib
$ cc/noopt/debug IPNCL_PKG.C +[--]vmsinc/lib
$ cc/noopt/debug NIS_PKG.C +[--]vmsinc/lib
$ cc/noopt/debug IPNIS_PKG.C +[--]vmsinc/lib
$ abort:
$ set nover
