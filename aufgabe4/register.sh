#!/bin/sh
module="timer"
device="timer"
mode="666"

#run insmod
/sbin/insmod ./$module.ko $* || exit 1

#remove old device nodes
rm -f /dev/${device}r
rm -f /dev/${device}f

# get major number from /proc/devices using awk
major=$(awk "\$2 == \"$module\" {print \$1}" /proc/devices)

# create device nodes
echo Creating device nodes for major number $major
mknod /dev/${device}f c $major 0
mknod /dev/${device}r c $major 1

# change group and permission
chgrp 'wheel' /dev/${device}f
chgrp 'wheel' /dev/${device}r

chmod $mode /dev/${device}f
chmod $mode /dev/${device}r