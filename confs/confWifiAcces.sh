#!/bin/bash

# Script to setup Raspberry as a wifi access point 
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
	echo "Illegal number of parameters. Format : confWifiAccess.sh <enable|disable>" >&2
	exit -1
fi

if [ "$1" = "enable" ]
then
	# Enable configuration
	echo "Backing up configuration files"
	cp /etc/dhcpcd.conf /etc/dhcpcd.conf.bak
	cp /etc/dhcpcd.conf $ROOT/confs/wifiConfs/bak/dhcpcd.conf.bak
	
	cp /etc/network/interfaces /etc/network/interfaces.bak
	#cp /etc/network/interfaces $ROOT/confs/wifiConfs/bak/interfaces.bak
	rm /etc/network/interfaces 
	
	cp /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf.bak
	#cp /etc/hostapd/hostapd.conf $ROOT/confs/wifiConfs/bak/hostapd.conf.bak
	rm /etc/hostapd/hostapd.conf
	
	if [ -f "/etc/default/hostapd" ]
	then
		cp /etc/default/hostapd /etc/default/hostapd.bak
		#cp /etc/default/hostapd $ROOT/confs/wifiConfs/bak/hostapd.txt.bak
		rm /etc/default/hostapd 
	fi
	
	cp /etc/dnsmasq.conf /etc/dnsmasq.conf.bak
	#cp /etc/dnsmasq.conf $ROOT/confs/wifiConfs/bak/dnsmasq.conf.bak
	rm /etc/dnsmasq.conf
	
	echo "Installing new configuration files"
	echo "denyinterfaces wlan0" >> /etc/dhcpcd.conf
	cp $ROOT/confs/wifiConfs/interfaces.txt /etc/network/interfaces
	cp $ROOT/confs/wifiConfs/hostapd.conf /etc/hostapd/hostapd.conf
	cp $ROOT/confs/wifiConfs/hostapd.txt /etc/default/hostapd
	cp $ROOT/confs/wifiConfs/dnsmasq.conf /etc/dnsmasq.conf
	
	systemctl unmask hostapd.service
	systemctl enable hostapd.service

elif [ "$1" = "disable" ]
then
	# Disable configuration
	echo "Restoring configuration files"
	
	rm /etc/dhcpd.conf 
	rm /etc/network/interfaces 
	rm /etc/hostapd/hostapd.conf
	rm /etc/default/hostapd 
	rm /etc/dnsmasq.conf
	
	cp etc/dhcpcd.conf.bak /etc/dhcpcd.conf
	cp /etc/network/interfaces.bak /etc/network/interfaces
	cp /etc/hostapd/hostapd.conf.bak /etc/hostapd/hostapd.conf
	if [ -f "/etc/default/hostapd.bak" ]
	then
		cp /etc/default/hostapd.bak /etc/default/hostapd
	fi
	cp /etc/dnsmasq.conf.bak /etc/dnsmasq.conf
	
	systemctl disable hostapd.service
	systemctl mask hostapd.service

else
	echo "Illegal parameter value : enable or disable" >&2
	exit -1
fi

