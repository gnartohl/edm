$ on error then goto abort
$ set ver
$ cc/debug/noopt xgraph_pkg.c + [--]vmsinc/lib
$ cc/debug/noopt psgraph_pkg.c + [--]vmsinc/lib
$ abort:
$ set nover
