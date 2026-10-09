#!/bin/bash
# Flash the signed PX4 image into MCUboot slot0 and start it on the M7.
#
# This mirrors exactly what "west flash" does for the Zephyr
# mr_navq95b_mimx9596_m7_flash board. Capture it with "west -v flash":
#
#   pyocd flash -e sector -a 0x28020000 -t mimx95_cm7_mx25um -f 20M \
#         -O vtor=0x28020800 <signed.bin>
#
# Two details matter:
#
#  -a 0x28020000  slot0_partition. pyocd would otherwise default to the
#                 start of the flash region (0x28000000), which is
#                 MCUboot's own boot_partition.
#
#  -O vtor=...    slot0 + CONFIG_ROM_START_OFFSET (0x800) = the application
#                 vector table. For this SoC pyocd's CM7 "reset" is an
#                 emulated, core-only restart: it writes VTOR, then loads
#                 MSP/PC from that table and resumes. That launches the
#                 application directly and does not go through MCUboot.
#
# Do not use "pyocd reset -t mimx95_cm33" here. That resets the whole chip,
# so the M7 comes up in MCUboot instead of being launched at the app VTOR.
# On such a cold boot this MCUboot stays in its serial-recovery loop
# ("Unable to find bootable image") rather than chain-loading slot0, so the
# application never runs even though the image in slot0 is valid.
set -e

PYOCD=${PYOCD:-pyocd}
IMG=${1:-./build/nxp_imx95_default/nxp_imx95_default.signed.bin}

SLOT0=0x28020000
VTOR=0x28020800
TARGET=mimx95_cm7_mx25um
FREQ=20M

"$PYOCD" flash -e sector -a "$SLOT0" -t "$TARGET" -f "$FREQ" \
	-O vtor="$VTOR" "$IMG"

# Core-only restart of the M7 straight into the application vector table.
"$PYOCD" reset -t mimx95_cm33 -f "$FREQ"
