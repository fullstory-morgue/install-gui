#!/bin/dash
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
#Simple script to umount all partitions before the installer starts his work
#--------------------------------------------------------------------------
 
function umount_all_drives
{
	local ok=0
	local TempFile=`mktemp -p /tmp/ .XXXXXXXXXX`

	awk '/\/dev/{print $1":"$3}'  /etc/fstab > $TempFile

	while IFS=: read device typ; do 
		case "$typ" in 
		swap) swapoff "$device" ;; 
		ext2|ext3|reiserfs|vfat|jfs|xfs|ntfs) umount "$device" 
			ok=$? 
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

if umount_all_drives ; then
	exit 0	
else
	exit 1 
fi
