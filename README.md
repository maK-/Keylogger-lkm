Keylogger-lkm
========================

This is a very simple Keylogger implementation, it doesnt hide itself 
and is part of a college project http://blogs.computing.dcu.ie/wordpress/mak0/

To run simply run "make" in the folder with the Makefile.

There is also the option of running scripts/lets_mak_it.sh
This takes 2 arguments, the name of the device to appear in /dev/DEVICE_NAME
and a major number for the device. By default it used a device name of
maK_it and a major number of 33.

To view the logged keys cat /dev/maK_it

install with 
insmod maK_it.ko

Remove with 
rmmod maK_it

Building towards development of a rootkit...

