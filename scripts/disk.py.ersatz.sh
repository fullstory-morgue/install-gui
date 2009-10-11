#!/bin/sh
#
# (C) 2008 Joaquim Boura <x-un-i@sidux.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this package; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301, USA.
#
# On Debian GNU/Linux systems, the text of the GPL license can be
# found in /usr/share/common-licenses/GPL.
#
#--------------------------------------------------------------------------
# Shell replacement for disk.py
#
# We emulate the different calls for disk.py
# Only option -u needs a parameter.
#
#
#
exec 2>/dev/null

partition_or_volume_info()
{
	local myVar=$1
	udevadm info  --query=all --name=$myVar |\
		fgrep -w -e DEVNAME -e ID_FS_TYPE -e ID_FS_USAGE -e ID_FS_UUID |\
		sed 's#E: ##' |\
		awk -F"=" '{printf "%s,", $2}END{printf "norm\n"}'| \
		awk -F"," '{printf  "%s,%s,%s,%s,%s\n",$1,$3,$4,$2,$5}' |\
		fgrep -v -e "LVM2_member"|fgrep "filesystem"

}

parameter_p()
{
	disks=`LANG=C fdisk -l | grep "Disk /dev" | cut -d: -f1 | cut -d" " -f2`
	# parts=`fdisk -l $disk | fgrep -v "Extended" | fgrep '^/dev/' | cut -d " " -f1 | cut -d "/" -f 3`

	for disk in ${disks}
	do
		parts=$(fdisk -l $disk | grep -v "Extended" | \
			grep '^/dev/' | cut -d " " -f1 | cut -d "/" -f 3)
		for part in $parts
		do
			partition_or_volume_info $part
		done
	done

	volumes=`ls /dev/mapper/* | fgrep -v "CONTROL"`
	for vol in $volumes
		do
			partition_or_volume_info $vol
		done
}

parameter_d()
{
	LANG=C fdisk -l | grep "Disk /dev" | \
		cut -d: -f1 | cut -d" " -f2 | \
		fgrep -v "/dev/dm"
	ls  /dev/mapper/* | grep -v control
}

parameter_n()
{
	disks=`LANG=C fdisk -l | grep "Disk /dev" |\
	grep -v "/dev/dm" | cut -d: -f1 | cut -d" " -f2`
	for disk in ${disks}; do
		readlink -f /sys/block/${disk##/dev/}/device |\
			grep -q usb || printf "%s\n" ${disk}
	done
}

parameter_u()
{
	local mydisk=$1
	readlink -f /sys/block/${mydisk}/device | \
		grep -q usb && printf "/dev/%s\n" ${mydisk}
}


# parameter_u $1
echo "Par p"
parameter_p
echo "Par d"
parameter_d
echo "Par n"
parameter_n

