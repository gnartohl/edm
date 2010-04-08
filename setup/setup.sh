# Setup edm variables so that it uses shared libs and config files
# from this source tree.
#
# This is meant to get you started, this is not the ultimate EDM
# setup tool.
#
# edm has to be built before this is run
#
# "source" this from any shell (other than csh) and hopefully
# you can then run EDM as "$EDM".
#
# If that works, checkout the result of
#
# "set | grep EDM"
#
# and try to understand what all the EDM-related variables
# mean. Include them in your login script or /etc/profile or ...
#

# These must be prepared:
echo "*"
if [ x$HOST_ARCH = x ]
then
   echo You need to set HOST_ARCH!
   exit 1
fi
if [ x$EPICS_HOST_ARCH = x ]
then
   echo If you are under EPICS base R3.14, you need to set EPICS_HOST_ARCH!
fi
echo "* Your LD_LIBRARY_PATH must be set and contain"
echo "* the path to your EPICS base's"
echo "* lib/HOST_ARCH resp. lib/EPICS_HOST_ARCH directory"
echo "* so that we can use the CA, Com, ... libraries!"
echo "* It might also need to include the Motif libraries."
echo "*"

export EDMBASE=`(cd ..;pwd)`

echo Setting up EDM base directory: $EDMBASE
echo ""

if [ x$EPICS_HOST_ARCH = x ]
then
   ODIR=$HOST_ARCH
else
   ODIR=$EPICS_HOST_ARCH
fi
export EDMFILES=$EDMBASE/setup
export EDMOBJECTS=$EDMBASE/setup
export EDMPVOBJECTS=$EDMBASE/setup

for libdir in baselib imagelib lib epicsPv locPv calcPv util choiceButton
do
    LD_LIBRARY_PATH=$EDMBASE/$libdir/O.$ODIR:$LD_LIBRARY_PATH
done
export LD_LIBRARY_PATH

export EDM=$EDMBASE/edmMain/O.$ODIR/edm

if [ -f $EDMPVOBJECTS/edmPvObjects ]
then
    echo There is already a file $EDMOBJECTS/edmPvObjects
    echo To have it recreated, remove that file and restart.
    echo ""
else
    $EDM -addpv $EDMBASE/epicsPv/O.$ODIR/libEpics.so
    $EDM -addpv $EDMBASE/locPv/O.$ODIR/libLoc.so
    $EDM -addpv $EDMBASE/calcPv/O.$ODIR/libCalc.so
fi

if [ -f $EDMOBJECTS/edmObjects ]
then
    echo There is already a file $EDMOBJECTS/edmObjects
    echo To have it recreated, remove that file and restart.
    echo ""
else
    $EDM -add $EDMBASE/baselib/O.$ODIR/libEdmBase.so
    $EDM -add $EDMBASE/imagelib/O.$ODIR/lib605432d2-f29d-11d2-973b-00104b8742df.so
    $EDM -add $EDMBASE/choiceButton/O.$ODIR/lib7e1b4e6f-239d-4650-95e6-a040d41ba633.so
    $EDM -add $EDMBASE/pvFactory/O.$ODIR/libPV.so
fi












