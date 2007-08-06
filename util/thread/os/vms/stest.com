$ set ver
$ cc/noopt/deb stest.c
$ link/deb stest, stest.opt/opt
$ set nover
