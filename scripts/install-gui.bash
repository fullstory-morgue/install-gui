#!/bin/bash

# initialize ssft
SSFTSH="$(which ssft.sh)" || SSFTSH="/usr/bin/ssft.sh"
if [ -r "$SSFTSH" ]; then
	. "$SSFTSH"
	[ -n "$SSFT_FRONTEND" ] ||  SSFT_FRONTEND="$(ssft_choose_frontend)"

	# The BIOS used in Intel's D945GCLF2 board requires a bootflag, to
	# consider a disk bootable
	if [ -r /sys/devices/virtual/dmi/id/board_vendor ] && \
	   [ -r /sys/devices/virtual/dmi/id/board_name ] && \
	   [ "x$(cat /sys/devices/virtual/dmi/id/board_vendor)" = "xIntel Corporation" ] && \
	   [ "x$(cat /sys/devices/virtual/dmi/id/board_name)" = "xD945GCLF2" ]; then
		if [ -n "$DISPLAY" ]; then
			ssft_display_message "ATTENTION" "install-gui has detected an Intel D945GCLF2 mainboard, this mainboard requires a bootflag to consider a harddisk bootable.\n\nPlease make sure to set it for one single partition on your dedicated bootdisk inside your partitioning program."
		fi
	fi
fi

# check: Detected driver that requires firmware to operate
export SSFT_FRONTEND="text"
export FLL_FIRMWARE=$(fw-detect <<< $'\n' | grep -v "ENTER")
export FLL_FIRMWARE_INSTALL=$(fw-detect -y)

exec install-gui
