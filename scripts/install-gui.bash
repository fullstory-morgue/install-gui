#!/bin/bash

# check: Detected driver that requires firmware to operate
export SSFT_FRONTEND="text"
export FLL_FIRMWARE=$(fw-detect <<< $'\n' | grep -v "ENTER")

exec install-gui
