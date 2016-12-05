#!/bin/sh
module="timer"
device="timer"

#run rmmod
/sbin/rmmod $module || exit 1

#remove old device nodes
rm -f /dev/${device}r
rm -f /dev/${device}f