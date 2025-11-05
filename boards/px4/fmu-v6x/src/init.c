/****************************************************************************
 *
 *   Copyright (c) 2012-2022 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file init.c
 *
 * PX4FMU-specific early startup code.  This file implements the
 * board_app_initialize() function that is called early by nsh during startup.
 *
 * Code here is run before the rcS script is invoked; it should start required
 * subsystems and perform board-specific initialization.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "board_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <nuttx/spi/spi.h>
#include <nuttx/sdio.h>
#include <nuttx/mmcsd.h>
#include <nuttx/analog/adc.h>
#include <nuttx/mm/gran.h>

#include "arm_internal.h"
#include "imxrt_flexspi_nor_boot.h"
#include <px4_arch/imxrt_flexspi_nor_flash.h>
#include "imxrt_iomuxc.h"
#include "imxrt_flexcan.h"
#include "imxrt_enet.h"
#include <chip.h>
#include <stm32_uart.h>
#include <arch/board/board.h>
#include "arm_internal.h"

#include <drivers/drv_hrt.h>
#include <drivers/drv_board_led.h>
#include <systemlib/px4_macros.h>
#include <px4_arch/io_timer.h>
#include <px4_arch/imxrt_romapi.h>
#include <px4_platform_common/init.h>
#include <px4_platform_common/px4_manifest.h>
#include <px4_platform/gpio.h>
#include <px4_platform/board_determine_hw_info.h>
#include <px4_platform/board_dma_alloc.h>

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/*
 * Ideally we'd be able to get these from arm_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */
__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
__END_DECLS


/************************************************************************************
 * Name: board_peripheral_reset
 *
 * Description:
 *
 ************************************************************************************/
__EXPORT void board_peripheral_reset(int ms)
{
	/* set the peripheral rails off */

	VDD_5V_PERIPH_EN(false);
	board_control_spi_sensors_power(false, 0xffff);
	VDD_3V3_SENSORS4_EN(false);

	bool last = READ_VDD_3V3_SPEKTRUM_POWER_EN();
	/* Keep Spektum on to discharge rail*/
	VDD_3V3_SPEKTRUM_POWER_EN(false);

	/* wait for the peripheral rail to reach GND */
	usleep(ms * 1000);
	syslog(LOG_DEBUG, "reset done, %d ms\n", ms);

	/* re-enable power */

	/* switch the peripheral rail back on */
	VDD_3V3_SPEKTRUM_POWER_EN(last);
	board_control_spi_sensors_power(true, 0xffff);
	VDD_3V3_SENSORS4_EN(true);
	VDD_5V_PERIPH_EN(true);

}

/************************************************************************************
 * Name: board_on_reset
 *
 * Description:
 * Optionally provided function called on entry to board_system_reset
 * It should perform any house keeping prior to the rest.
 *
 * status - 1 if resetting to boot loader
 *          0 if just resetting
 *
 ************************************************************************************/
__EXPORT void board_on_reset(int status)
{
	for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
		px4_arch_configgpio(io_timer_channel_get_gpio_output(i));
	}

	if (status >= 0) {
		up_mdelay(6);
	}
}

/****************************************************************************
 * Name: imxrt_flash_romapi_initialize
 *
 * Description:
 *
 ****************************************************************************/
struct flexspi_nor_config_s g_bootConfig;

locate_code(".ramfunc")
void imxrt_flash_romapi_initialize(void)
{
	memcpy((struct flexspi_nor_config_s *)&g_bootConfig, &g_flash_config,
	       sizeof(struct flexspi_nor_config_s));
	g_bootConfig.memConfig.tag = FLEXSPI_CFG_BLK_TAG;

	ROM_API_Init();

	ROM_FLEXSPI_NorFlash_Init(NOR_INSTANCE, (struct flexspi_nor_config_s *)&g_bootConfig);
	ROM_FLEXSPI_NorFlash_ClearCache(NOR_INSTANCE);

	ARM_DSB();
	ARM_ISB();
	ARM_DMB();
}

locate_code(".ramfunc")
void imxrt_flash_setup_prefetch_partition(void)
{
//Prefetch tuning to be determined
#if 0
	putreg32((uint32_t)&_srodata, IMXRT_FLEXSPI1_AHBBUFREGIONSTART0);
	putreg32((uint32_t)&_erodata, IMXRT_FLEXSPI1_AHBBUFREGIONEND0);
	/*putreg32((uint32_t)&_stext, IMXRT_FLEXSPI1_AHBBUFREGIONSTART1);
	putreg32((uint32_t)&_etext, IMXRT_FLEXSPI1_AHBBUFREGIONEND1);
	putreg32((uint32_t)&_stext, IMXRT_FLEXSPI1_AHBBUFREGIONSTART2);
	putreg32((uint32_t)&_etext, IMXRT_FLEXSPI1_AHBBUFREGIONEND2);
	putreg32((uint32_t)&_stext, IMXRT_FLEXSPI1_AHBBUFREGIONSTART3);
	putreg32((uint32_t)&_etext, IMXRT_FLEXSPI1_AHBBUFREGIONEND3);*/
#endif
#if 0
	struct flexspi_type_s *g_flexspi = (struct flexspi_type_s *)IMXRT_FLEXSPIC_BASE;
	/* RODATA */
	g_flexspi->AHBRXBUFCR0[0] = FLEXSPI_AHBRXBUFCR0_BUFSZ(48) |
				    FLEXSPI_AHBRXBUFCR0_MSTRID(0) |
				    FLEXSPI_AHBRXBUFCR0_PREFETCHEN(1) |
				    FLEXSPI_AHBRXBUFCR0_REGIONEN(1);


	/* All Text */
	g_flexspi->AHBRXBUFCR0[1] = FLEXSPI_AHBRXBUFCR0_BUFSZ(80) |
				    FLEXSPI_AHBRXBUFCR0_MSTRID(0) |
				    FLEXSPI_AHBRXBUFCR0_PREFETCHEN(1) |
				    FLEXSPI_AHBRXBUFCR0_REGIONEN(1);

	/* Disable 2 */
	g_flexspi->AHBRXBUFCR0[2] = FLEXSPI_AHBRXBUFCR0_BUFSZ(0) |
				    FLEXSPI_AHBRXBUFCR0_MSTRID(0) |
				    FLEXSPI_AHBRXBUFCR0_PREFETCHEN(1) |
				    FLEXSPI_AHBRXBUFCR0_REGIONEN(0);

	/* Disable 3 */
	g_flexspi->AHBRXBUFCR0[3] = FLEXSPI_AHBRXBUFCR0_BUFSZ(0) |
				    FLEXSPI_AHBRXBUFCR0_MSTRID(0) |
				    FLEXSPI_AHBRXBUFCR0_PREFETCHEN(1) |
				    FLEXSPI_AHBRXBUFCR0_REGIONEN(0);
#endif

	ARM_DSB();
	ARM_ISB();
	ARM_DMB();
}

/****************************************************************************
 * Name: imxrt_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point.  This entry point
 *   is called early in the initialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

__EXPORT void
stm32_boardinitialize(void)
{
	imxrt_flash_setup_prefetch_partition();

	imxrt_flash_romapi_initialize();

	board_on_reset(-1); /* Reset PWM first thing */

	/* configure LEDs */

	board_autoled_initialize();

	/* configure pins */

	const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
	px4_gpio_init(gpio, arraySize(gpio));

	imxrt_usb_initialize();

}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization.  This function is never
 *   called directly from application code, but only indirectly via the
 *   (non-standard) boardctl() interface using the command BOARDIOC_INIT.
 *
 * Input Parameters:
 *   arg - The boardctl() argument is passed to the board_app_initialize()
 *         implementation without modification.  The argument has no
 *         meaning to NuttX; the meaning of the argument is a contract
 *         between the board-specific initalization logic and the the
 *         matching application logic.  The value cold be such things as a
 *         mode enumeration value, a set of DIP switch switch settings, a
 *         pointer to configuration data read from a file or serial FLASH,
 *         or whatever you would like to do with it.  Every implementation
 *         should accept zero/NULL as a default configuration.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
	int ret = OK;

#if !defined(BOOTLOADER)

	imxrt_flexio_clocking();

	/* Power on Interfaces */
	VDD_3V3_SD_CARD_EN(true);
	VDD_5V_PERIPH_EN(true);
	VDD_5V_HIPOWER_EN(true);
	VDD_3V3_SENSORS4_EN(true);
	VDD_3V3_SPEKTRUM_POWER_EN(true);

	px4_platform_init();

	imxrt_spiinitialize();

	/* configure the DMA allocator */

	if (board_dma_alloc_init() < 0) {
		syslog(LOG_ERR, "[boot] DMA alloc FAILED\n");
	}

#  if defined(SERIAL_HAVE_RXDMA)
	// set up the serial DMA polling at 1ms intervals for received bytes that have not triggered a DMA event.
	static struct hrt_call serial_dma_call;
	hrt_call_every(&serial_dma_call, 1000, 1000, (hrt_callout)stm32_serial_dma_poll, NULL);
#  endif

#if defined(CONFIG_IMXRT_USDHC)
	ret = fmurt1062_usdhc_initialize();
#endif

	imxrt_spiinitialize();

	board_spi_reset(10, 0xffff);

#ifdef CONFIG_IMXRT_ENET
	imxrt_netinitialize(0);
#endif

#ifdef CONFIG_IMXRT_FLEXCAN3
	imxrt_caninitialize(3);
#endif

	ret = imxrt_flexspi_storage_initialize();

	if (ret < 0) {
		syslog(LOG_ERR,
		       "ERROR: imxrt_flexspi_nor_initialize failed: %d\n", ret);
	}

#endif /* !defined(BOOTLOADER) */

#  ifdef CONFIG_MMCSD
	int ret = stm32_sdio_initialize();

	if (ret != OK) {
		led_on(LED_RED);
		return ret;
	}

#  endif /* CONFIG_MMCSD */

#endif /* !defined(BOOTLOADER) */

	return OK;
}
