$ on error then goto abort
$ set ver
$ cc/debug/noopt [-]avl.c + [--]vmsinc/lib
$ abort:
$ set nover
