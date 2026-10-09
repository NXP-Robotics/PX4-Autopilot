#!/bin/bash
# Sign the PX4 i.MX95 M7 image for MCUboot slot0.
#
# These settings mirror what Zephyr's "west sign" produces for the
# mr_navq95b_mimx9596_m7_flash board, which is the configuration the
# on-target MCUboot was built against:
#
#   header-size 0x800   MCUboot image header; the linker places
#                       .interrupts at 0x28020800, i.e. slot0 + 0x800,
#                       so the header occupies the first 2 KB.
#   slot-size   7340032 0x700000, the size of slot0_partition
#                       (and slot1_partition) in the board devicetree.
#   align       2       matches the mx25um write-block-size of 2 bytes.
#   pad-header          PX4's .bin starts directly with the vector table
#                       and does not reserve header space, so imgtool has
#                       to prepend the 0x800 header itself. Zephyr instead
#                       reserves it via CONFIG_ROM_START_OFFSET and omits
#                       this flag; both yield the same layout.
#
# No signing key is used: the on-target MCUboot is built with
# CONFIG_BOOT_SIGNATURE_TYPE_NONE, so a plain SHA256 TLV is what it expects.
set -e

BUILD_DIR=${BUILD_DIR:-build/nxp_imx95_default}
IN=${1:-$BUILD_DIR/nxp_imx95_default.bin}
OUT=${2:-$BUILD_DIR/nxp_imx95_default.signed.bin}
VERSION=${VERSION:-1.0.0}

make nxp_imx95

python3 scripts/imgtool.py sign \
	--version "$VERSION" \
	--header-size 0x800 \
	--slot-size 7340032 \
	--align 2 \
	--pad-header \
	"$IN" "$OUT"

echo "signed: $OUT"
python3 scripts/imgtool.py verify "$OUT"
