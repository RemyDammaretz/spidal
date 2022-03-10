#!/bin/bash

# Script to setup Raspberry as a NTP server
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
	echo "Illegal number of parameters. Format : confNtpServer.sh <enable|disable>" >&2
	exit -1
fi

if [ "$1" = "enable" ]
then
	# Enable configuration
	echo "Disabling default time configuration"
	timedatectl set-ntp false
	echo "Backing up NTP configuration file"
	cp /etc/ntp.conf /etc/ntp.server.conf.bak
	#cp /etc/ntp.conf $ROOT/confs/ntpConfs/bak/ntp.server.conf.bak
	echo "Changing NTP configuration file"
	rm /etc/ntp.conf
	cp $ROOT/confs/ntpConfs/ntp.server.conf /etc/ntp.conf
	echo "Restarting NTP service"
	service ntp restart

elif [ "$1" = "disable" ]
then
	# Disable configuration	
	echo "Restoring NTP configuration file"
	rm /etc/ntp.conf
	cp /etc/ntp.server.conf.bak /etc/ntp.conf
	echo "Stoping NTP service"
	service ntp stop
	echo "Enabling default time configuration"
	timedatectl set-ntp true
else
	echo "Illegal parameter value : enable or disable" >&2
	exit -1
fi

