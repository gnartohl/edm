$ ! $Revision$
$
$ set noon
$ set ver
$ cc/opt/nodeb [-]compute_sm_params.c
$ link/nodeb compute_sm_params
$ del *.obj.*
$ pur compute_sm_params.exe
$ set nover
