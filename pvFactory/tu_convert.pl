# Tries to convert Textupdate fields that used
# - widget background
# - alarm sensitive text
# to
# - alt. widget background
# - alarm sens. border
# - black text
use English;

$ARGC=$#ARGV + 1;

die "Usage: $PROGRAM_NAME <old edl> <new edl>" unless ($ARGC==2);

$old = $ARGV[0];
$new = $ARGV[1];

print "Converting $old to $new\n";

$otextcol="15\n";
$ntextcol="112\n";
$nfillcol="5\n";

open o, "$old" or die "Cannot open $old\n";
open n, ">$new" or die "Cannot create $new\n";

while (<o>)
{
    $line = $_;
    if ($line =~ "TextupdateClass")
    {
	$convert = 0;
	print n $line;
	# Version
	$line = <o>;
	die "Cannot convert from version $line" unless ($line =~ "6 0 0");
	print n "7 0 0\n";
	# X, Y, W, H
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	# PV
	$line = <o>; print n $line;
	# Mode, precision
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	# Text Color
	$line = <o>; print n $line;
	die "Missing index for text color\n" unless ($line =~ "index");
	$line = <o>;
	if ($line eq $otextcol)
	{
	    print n $ntextcol;
	    $convert = 1;
	}
	else
	{
	    print n $line;
	}
	# Alarm Sensitive
	$line = <o>;
	if ($convert)
	{
	    print n "0\n";
	}
	else
	{
	    print n $line;
	}
	# Fill Color
	$line = <o>; print n $line;
	die "Missing index for fill color\n" unless ($line =~ "index");
	$line = <o>;
	if ($convert)
	{
	    print n $nfillcol;
	}
	else
	{
	    print n $line;
	}
	# Color PV
	$line = <o>; print n $line;
	# fill mode, fonts
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	$line = <o>; print n $line;
	# Line width
	$line = <o>;
	if ($convert)
	{
	    print n "2 0\n";
	}
	else
	{
	    print n $line;
	}
	# NEW: Alarm sensitive!!
	print n "1\n";
	
	$line = <o>;
	print n "$line";
	die "Didn't find end\n" unless ($line =~ "<<<E~O~D");
    }
    else
    {
	print n $line;
    }
}
