#!/bin/bash


source /etc/default/distro


user_home=$(getent passwd ${FLL_LIVE_USER} | cut -d : -f6)
# kde
user_kde_config_dir="/home/${FLL_LIVE_USER}/.kde/share/config"
user_kde_medianotifierrc="${user_kde_config_dir}/medianotifierrc"
# xfce
user_xfce_volmanrc="${user_home}/.config/Thunar/volmanrc"
# gnome ??


#--------------------------------------------------------
# disable automount in xfce or medianotifier popup in KDE
function disable()
{
	if [ ! -d "${user_kde_config_dir}" ]; then
		if [ -e "${user_xfce_volmanrc}" ]; then 
			sed -i -e 's/AutomountDrives=TRUE/AutomountDrives=FALSE/' \
				 -e 's/AutomountMedia=TRUE/AutomountMedia=FALSE/' \
				 -e 's/Autobrowse=TRUE/Autobrowse=FALSE/' \
				"${user_xfce_volmanrc}"
		fi
	else
		cat <<EOF > "${user_kde_medianotifierrc}"
[Auto Actions]
media/cdwriter_unmounted=#NothinAction
media/hdd_unmounted=#NothinAction
media/removable_unmounted=#NothinAction
EOF
	fi
}

#---------------------------------------------------------
# reenable automount in xfce or medianotifier popup in KDE
function reenable()
{
	udevadm trigger --subsystem-match=block
	udevsettle --timeout=30
	if [ ! -d "${user_kde_config_dir}" ]; then
		if [ -e "${user_xfce_volmanrc}" ]; then 
			sed -i -e 's/AutomountDrives=FALSE/AutomountDrives=TRUE/' \
				 -e 's/AutomountMedia=FALSE/AutomountMedia=TRUE/' \
				 -e 's/Autobrowse=FALSE/Autobrowse=TRUE/' \
				"${user_xfce_volmanrc}"
		fi
	else
		 rm -f "${user_kde_medianotifierrc}"
	fi
	/etc/init.d/fll-fstab start
	# also reactivate swap devices
	swapon -a
}


# called with one parameter enable or disable

if [ $# -ne 1 ] ; then
	echo "usage: $0 enable|disable"
else
	if [ "$1" = "enable" ]; then
		reenable
	elif [ "$1" = "disable" ]; then
		disable
	else
		echo "usage: $0 enable|disable"
		exit 1
	fi
fi

exit 0
