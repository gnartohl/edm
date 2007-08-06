$ on error then goto abort
$ set ver
$ cc/noopt/debug/noinclude/def=VAXELN SYS_PKG.C -
  + [--]elninc/lib + eln$:vaxelnc/lib
$ abort:
$ set nover
