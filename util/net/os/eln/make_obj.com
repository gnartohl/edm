$ on error then goto abort
$ set ver
$ cc/noopt/debug/noinclude/def=VAXELN NCL_PKG.C -
  + [--]elninc/lib + eln$:vaxelnc/lib
$  cc/noopt/debug/noinclude/def=VAXELN NIS_PKG.C -
  + [--]elninc/lib + eln$:vaxelnc/lib
$ cc/noopt/debug/noinclude/def=VAXELN NSV_PKG.C -
  + [--]elninc/lib + eln$:vaxelnc/lib
$ abort:
$ set nover
