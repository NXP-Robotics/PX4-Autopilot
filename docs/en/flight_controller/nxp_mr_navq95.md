# NXP MR-NavQ95

<Badge type="tip" text="PX4 main" />

::: warning
PX4 does not manufacture this (or any) autopilot.
Contact the [manufacturer](https://www.nxp.com) for hardware support (https://community.nxp.com/) or compliance issues.
:::

The _MR-NavQ95_ is NXP's i.MX 95 based vehicle computer.
Linux runs on the Cortex-A55 cores, and PX4 runs on the Cortex-M7 real-time core, started by MCUboot from the on-board NOR flash.
The two sides talk over RPMsg: PX4 keeps its parameters, dataman and logs on a filesystem served by Linux, and exposes its shell and a MAVLink link as ttys on the Linux side.

::: info
This flight controller is [manufacturer supported](../flight_controller/autopilot_manufacturer_supported.md).
:::

## Key Features

- **SoC:** NXP i.MX 95, PX4 on the Arm® Cortex®-M7 core (800 MHz)
- **IMU:** TDK InvenSense ICM-45686 and Bosch BMI088, on separate SPI buses
- **Magnetometer:** Bosch BMM350
- **Barometer:** Bosch BMP581
- **Interfaces:**
  - 3x UARTs for PX4 (GPS, telemetry with flow control, RC)
  - 3x CAN FD (FlexCAN): two for PX4, DroneCAN capable, and one for Linux
  - External I2C
  - 7x PWM outputs, all [DShot](../peripherals/dshot.md) capable
  - RPMsg links to Linux: shell, MAVLink, storage

Hardware details, schematics and the pinout of every connector are in the [MR-NAVQ95 repository](https://github.com/NXP-Robotics/MR-NAVQ95).

## Where to Buy

See the [MR-NAVQ95 repository](https://github.com/NXP-Robotics/MR-NAVQ95) for availability.

## Serial Port Mapping

| UART    | Device     | PX4 Port                   |
| ------- | ---------- | -------------------------- |
| LPUART2 | /dev/ttyS1 | RC                         |
| LPUART5 | /dev/ttyS4 | GPS1                       |
| LPUART7 | /dev/ttyS6 | TEL1 (hardware flow control) |

### PWM Outputs

The 7 outputs are in 3 groups:

- Outputs 1-3 in group 1 (TPM3)
- Outputs 4-5 in group 2 (TPM4)
- Outputs 6-7 in group 3 (TPM5)

All outputs support PWM and [DShot](../peripherals/dshot.md).
Each group's protocol is set with the corresponding `PWM_MAIN_TIMx` parameter.

## Linux Side

The Linux side must run the NavQ95 BSP built from [imx-manifest-navq95](https://github.com/NXP-Robotics/imx-manifest-navq95).
It provides the RPMsg links PX4 depends on:

| Linux device   | PX4 side                   | Purpose                                   |
| -------------- | -------------------------- | ----------------------------------------- |
| `/dev/ttynsh`  | `nshterm`                  | PX4 shell (NSH console)                   |
| `/dev/ttyproxy` | `mavlink` in onboard mode | MAVLink at 921600 baud for the companion |

PX4 has no local storage.
`/fs/rpmsg` on the PX4 side is an RPMsg filesystem (rpmsgfs) served by Linux, and it holds the parameters, dataman and the flight logs.

## Building Firmware

To [build PX4](../dev_setup/building_px4.md) for this target:

```sh
make nxp_mr-navq95_default
```

The build writes two images:

- `build/nxp_mr-navq95_default/nxp_mr-navq95_default.bin`, the plain binary.
- `build/nxp_mr-navq95_default/nxp_mr-navq95_default.signed.bin`, the plain binary wrapped with the MCUboot header and SHA256 trailer by `imgtool`.
  This is the image MCUboot boots from slot0.

The MCUboot header size, slot size and alignment are set in `default.px4board` (`CONFIG_BOARD_MCUBOOT_IMGTOOL_ARGS`) and match the on-board MCUboot, which is built with `CONFIG_BOOT_SIGNATURE_TYPE_NONE`.
`imgtool` is installed by the PX4 setup scripts (`Tools/setup/requirements.txt`); if it is missing, the build warns and skips the signed image.

The `ddr` label (`make nxp_mr-navq95_ddr`) runs from SDRAM and is loaded by Linux instead of MCUboot, so it has no signed image.

### Signing With a Key

The default image carries only a SHA256 and boots on an MCUboot built with `CONFIG_BOOT_SIGNATURE_TYPE_NONE`.
For an MCUboot that verifies signatures, generate a key, pass it in the environment when configuring the build, and bake the matching public key into MCUboot:

```sh
python3 -m imgtool.main keygen -k navq95-signing.pem -t ecdsa-p256
BOARD_MCUBOOT_KEY=/path/to/navq95-signing.pem make nxp_mr-navq95_default
python3 -m imgtool.main getpub -k navq95-signing.pem   # for the MCUboot build
```

The key type must match the MCUboot signature type (`ecdsa-p256`, `rsa-2048`, `rsa-3072` or `ed25519`).
The key is read when CMake configures, so changing it needs a fresh build directory.
Keep the private key out of the repository.

## Installing PX4 Firmware

The firmware is flashed into MCUboot slot0 over SWD with pyOCD.
Mainline pyOCD has no i.MX 95 support; use the NXP-Robotics fork, branch `pr-imx95`, which adds the `mimx95_*` targets and the NOR flash algorithm:

```sh
git clone https://github.com/NXP-Robotics/pyOCD -b pr-imx95
cd pyOCD
python3 -m pip install .
```

Set the boot switches and power the board as described in the [NavQ95 manifest README](https://github.com/NXP-Robotics/imx-manifest-navq95#flash-nor-flash-image), then:

```sh
pyocd flash -e sector -a 0x28020000 -t mimx95_cm7_mx25um -f 20M \
  -O vtor=0x28020800 build/nxp_mr-navq95_default/nxp_mr-navq95_default.signed.bin
pyocd reset -t mimx95_cm33 -f 20M
```

- `-a 0x28020000` is slot0. pyOCD would otherwise default to the start of the flash region (`0x28000000`), which is MCUboot's own partition.
- `-O vtor=0x28020800` is slot0 plus the 0x800 byte header, i.e. the application vector table.
  pyOCD's CM7 reset on this SoC is a core-only restart: it writes VTOR, loads MSP/PC from that table and resumes, so the application starts directly without going through MCUboot.
- Do not expect a cold boot (`pyocd reset -t mimx95_cm33`) to chain-load slot0: on this board MCUboot stays in its serial-recovery loop ("Unable to find bootable image") after a full chip reset even though the image in slot0 is valid.

## Debug Port

The PX4 [system console](../debug/system_console.md) is `/dev/ttynsh` on the Linux side; there is no dedicated console UART.
The M7 core is reachable over the on-board SWD/JTAG probe through pyOCD (`pyocd gdb -t mimx95_cm7_mx25um`).

## Further info

- [MR-NAVQ95 hardware repository](https://github.com/NXP-Robotics/MR-NAVQ95)
- [NavQ95 Linux BSP manifest](https://github.com/NXP-Robotics/imx-manifest-navq95)
