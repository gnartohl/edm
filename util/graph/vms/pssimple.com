$ on error then goto abort
$ set ver
$ cc/debug/noopt pssimple.c
$ link/debug pssimple, sys$sysdevice:[sinclair.util]vms_util/lib, sys$input/opt
sys$share:decw$xlibshr/share
sys$share:vaxcrtl/share
$ abort:
$ pur
$ set nover
