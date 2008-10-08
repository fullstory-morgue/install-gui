#!/bin/bash
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
# Simple script to umount all partitions before the installer starts his work
# or to return 1 if any partition is mounted and could not be umounted
# called with one parameter ( value does not matter) just inquiries if there 
# is at least one partition mounted returning 1 in this case.
#--------------------------------------------------------------------------
 
function umount_all_drives()
{
	local ok=0
	local do_it=$1
	local TempFile=`mktemp -p /tmp/ .XXXXXXXXXX`

	awk '/\/dev/{print $1":"$3}'  /etc/fstab > $TempFile

	while IFS=: read device typ; do 
		case "$typ" in 
		swap) 
			if swapon -s | grep -q "^$device "  ; then
				if [ "$do_it" = "check" ]; then 
					ok=1
				else
					swapoff "$device" 
					ok=$? 
				fi
			fi
			;;	
		ext2|ext3|reiserfs|vfat|jfs|xfs|ntfs) 
			if mount | grep -q "^$device "; then
				if [ "$do_it" = "check" ]; then 
					ok=1
				else
					umount "$device" 
					ok=$? 
				fi
			fi
			;;	
		 auto|udf*)  
			ok=0
			;;# nothing to do skip
		*)
			ok=255 # unknow reason
		esac
		if [ "$ok" -eq 0 ] || [ "$ok" -eq 2 ]; then
			ok=0 # ignore device is not mounted or was sucessfuly umounted
		elif [ "$ok" -eq 1 ]; then  # device busy
			break
		else
			break  # some other errorcode
		fi
	done< $TempFile
	test -e ${TempFile} && rm -f ${TempFile} 
	return $ok
}

if [ $# -eq 1 ] ; then
	par="check"
else
	par="doit"
fi
if umount_all_drives $par; then
	exit 0	
else
	exit 1 
fi
