#!/bin/bash

# Script to setup startup configuration for Remote Raspberry
# Or disable this configuration


ROOT="/home/pi/spidal"

# Check privileges
if [ "$EUID" -ne 0 ]
then
	echo "Please run as root" >&2
	exit -1
fi

# Check number of args
if [ $# -ne 1 ]
then 
	echo "Illegal number of parameters. Format : confGUI.sh <enable|disable>" >&2
	exit -1
fi

if [ "$1" = "enable" ]
then
	# Enable configuration
	echo "Backing up rc.local file"
	cp /etc/rc.local /etc/rc.local.back
	cp /etc/rc.local $ROOT/confs/remoteStartConfs/bak/rc.local.bak
	echo "Changing rc.local file"
	rm /etc/rc.local
	cp $ROOT/confs/remoteStartConfs/rc.local /etc/rc.local

elif [ "$1" = "disable" ]
then
	# Disable configuration	
	echo "Restoring rc.local file"
	rm /etc/rc.local
	cp $ROOT/confs/remoteStartConfs/bak/rc.local.bak /etc/rc.local
else
	echo "Illegal parameter value : enable or disable" >&2
	exit -1
fi

