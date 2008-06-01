#!/bin/bash

# check: Detected driver that requires firmware to operate
export SSFT_FRONTEND="text"
export FLL_FIRMWARE=$(fw-detect <<< $'\n' | grep -v "ENTER")
export FLL_FIRMWARE_INSTALL=$(fw-detect -y)

exec /usr/bin/hal-lock --interface org.freedesktop.Hal.Device.Storage --exclusive --run install-gui
