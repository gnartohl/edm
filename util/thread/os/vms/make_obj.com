$ on error then goto abort
$ set ver
 cc/noopt/debug THREAD_PKG.C +[--]vmsinc/lib
$ abort:
$ set nover
