$ ! $Revision$
$
$ set noon
$ set ver
$ macro eln$:kerneldef+[]access_prot/obj=[]access_prot
$
$ cc/noopt/deb/noinclude/def=VAXELN []get_crate_id-
  +[--]elninc/lib+eln$:vaxelnc/lib
$
$ cc/noopt/deb/noinclude/def=VAXELN []get_tunable-
  +[--]elninc/lib+eln$:vaxelnc/lib
$
$ cc/noopt/deb/noinclude/def=VAXELN []map_sm-
  +eln$:vaxelnc/lib
$
$ cc/noopt/deb/noinclude/def=VAXELN []halt-
  +eln$:vaxelnc/lib
$
$ set nover
