$ on error then goto abort
$ set ver
$ cc/debug/noopt simple.c
$ link/debug simple, sys$sysdevice:[sinclair.util]vms_util/lib, sys$input/opt
sys$share:decw$xlibshr/share
sys$share:vaxcrtl/share
$ abort:
$ pur
$ set nover
