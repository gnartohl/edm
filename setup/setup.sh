# Setup edm variables so that it uses shared libs and config files
# from this source tree.
#
# edm has to be built before this is run

#HOST_ARCH=Linux
if [ x$EPICS = x ]
then
    EPICS=/cs/epics/R3.13.3/base
fi

export EDMBASE=`(cd ..;pwd)`

echo Setting up EDM base directory: $EDMBASE
echo ""

export EDMFILES=$EDMBASE/setup
export EDMOBJECTS=$EDMBASE/setup
export EDMPVOBJECTS=$EDMBASE/setup

LD_LIBRARY_PATH=$EPICS/lib/$HOST_ARCH
for libdir in baselib imagelib lib pvlib util
do
    LD_LIBRARY_PATH=$EDMBASE/$libdir/O.$HOST_ARCH:$LD_LIBRARY_PATH
done
export LD_LIBRARY_PATH

export EDM=$EDMBASE/edmMain/O.$HOST_ARCH/edm

if [ -f $EDMPVOBJECTS/edmPvObjects ]
then
    echo There is already a file $EDMOBJECTS/edmPvObjects
    echo To have it recreated, remove that file and restart.
    echo ""
else
    echo "1"     > $EDMPVOBJECTS/edmPvObjects
    echo "epicsPv $EDMBASE/pvlib/O.$HOST_ARCH/libEpics.so epics" \
     >>$EDMPVOBJECTS/edmPvObjects
fi

if [ -f $EDMOBJECTS/edmObjects ]
then
    echo There is already a file $EDMOBJECTS/edmObjects
    echo To have it recreated, remove that file and restart.
    echo ""
else
    $EDM -add $EDMBASE/baselib/O.$HOST_ARCH/libEdmBase.so
    $EDM -add $EDMBASE/imagelib/O.$HOST_ARCH/lib605432d2-f29d-11d2-973b-00104b8742df.so
    $EDM -add $EDMBASE/pvFactory/O.$HOST_ARCH/libPV.so
fi












