$ ! $Revision$
$
$ set noon
$ set ver
$ cc/noopt/deb/noinclude/def=VAXELN []fixvic.c -
  +[--]util/lib + eln$:vaxelnc/lib
$ link/nosyslib/nosysshr/nouserlib/deb -
  fixvic, eln$:vme300/lib, -
  eln$:crtlshare/lib, eln$:rtlshare/lib, eln$:rtl/lib
$ del *.obj.*
$ ren fixvic.exe [--]*/log
$ set nover
