#!/bin/bash

# Script to setup GUI
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
	echo "Backing up xinitrc file"
	cp /etc/X11/xinit/xinitrc /etc/X11/xinit/xinitrc.bak
	cp /etc/X11/xinit/xinitrc $ROOT/confs/GUIConfs/bak/xinitrc.bak
	echo "Changing xinitrc file"
	rm /etc/X11/xinit/xinitrc
	cp $ROOT/confs/GUIConfs/xinitrc /etc/X11/xinit/xinitrc

	# Splash screen
	echo "Backing up splash screen image"
	cp /usr/share/plymouth/themes/pix/splash.png /usr/share/plymouth/themes/pix/splash.png.bak
	cp /usr/share/plymouth/themes/pix/splash.png $ROOT/confs/GUIConfs/bak/splash.png.bak
	echo "Changing splash screen"
	rm /usr/share/plymouth/themes/pix/splash.png
	cp $ROOT/confs/GUIConfs/splash.png /usr/share/plymouth/themes/pix/splash.png


elif [ "$1" = "disable" ]
then
	# Disable configuration	
	echo "Restoring xinitrc file"
	rm /etc/X11/xinit/xinitrc
	cp $ROOT/confs/GUIConfs/bak/xinitrc.bak /etc/X11/xinit/xinitrc

	# Splash screen
	echo "Restoring splash screen image"
	rm /usr/share/plymouth/themes/pix/splash.png
	cp $ROOT/confs/GUIConfs/bak/splash.png.bak /usr/share/plymouth/themes/pix/splash.png 
else
	echo "Illegal parameter value : enable or disable" >&2
	exit -1
fi

