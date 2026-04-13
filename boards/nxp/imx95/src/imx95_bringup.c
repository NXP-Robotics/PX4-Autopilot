/****************************************************************************
 * boards/arm/imx95/imx95-evk/src/imx95_bringup.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <syslog.h>

#include <nuttx/fs/fs.h>

#include "arm_internal.h"
#include "imx95-evk.h"
#include "imx9_gpio.h"

#ifdef CONFIG_RPTUN
#  include <imx9_rptun.h>
#endif

#ifdef CONFIG_IMX9_FLEXCAN
#  include "imx9_flexcan.h"
#endif

#ifdef CONFIG_RPMSG_UART
#  include <nuttx/serial/uart_rpmsg.h>
#endif

#include <hardware/imx95/imx95_pinmux.h>
#include <imx9_iomuxc.h>
#include <imx9_clockconfig.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_RPMSG_UART
void rpmsg_serialinit(void)
{
	uart_rpmsg_init("netcore", "proxy", 1024, false);
	uart_rpmsg_init("netcore", "mavmin", 1024, false);
	uart_rpmsg_init("netcore", "ros", 1024, false);
	uart_rpmsg_init("netcore", "nsh", 1024, true);
}
#endif

void pwm_iomux_initialize(void)
{
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO04_TPM3_CH0, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO20_TPM3_CH1, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO24_TPM3_CH3, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO05_TPM4_CH0, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO21_TPM4_CH1, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO06_TPM5_CH0, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));
	imx9_iomux_configure(IOMUX_CFG(IOMUXC_PAD_GPIO_IO22_TPM5_CH1, (IOMUXC_PAD_FSEL_SLOW | IOMUXC_PAD_DSE_X6 | IOMUXC_PAD_PD_ON),
				       IOMUXC_MUX_SION_ON));

	imx9_configure_clock(TPM4_CLK_ROOT_OSC_24M_CLK | CLOCK_DIV(24), true);
	imx9_configure_clock(TPM5_CLK_ROOT_OSC_24M_CLK | CLOCK_DIV(24), true);
	imx9_configure_clock(TPM6_CLK_ROOT_OSC_24M_CLK | CLOCK_DIV(24), true);

}

/****************************************************************************
 * Name: imx_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int imx95_bringup(void)
{
	int ret = OK;

#ifdef CONFIG_RPTUN
	imx9_rptun_init("imx9-shmem", "netcore");
#endif

	pwm_iomux_initialize();

#ifdef CONFIG_PWM
	/* Configure PWM outputs */

	ret = imx95_pwm_setup();

	if (ret < 0) {
		syslog(LOG_ERR, "ERROR: Failed initialize PWM outputs: %d\n", ret);
	}

#endif

#if defined(CONFIG_IMX9_LPI2C)

	/* Configure I2C peripheral interfaces */
	if (ret == 0) {
		ret = imx95_i2c_initialize();

		if (ret < 0) {
			syslog(LOG_ERR, "Failed to initialize I2C driver: %d\n", ret);
		}
	}

#endif

#if defined(CONFIG_IMX9_LPSPI1)

	/* Configure SPI peripheral interfaces */
	if (ret == 0) {
		ret = imx95_spi_initialize();

		if (ret < 0) {
			syslog(LOG_ERR, "Failed to initialize SPI driver: %d\n", ret);
		}
	}

#endif

	if (ret == 0) {
		ret = nx_mount(NULL, "/fs/rpmsg", "rpmsgfs", 0, "cpu=netcore,fs=/px4");

		if (ret < 0) {
			syslog(LOG_ERR, "Failed to mount rpmsgfs: %d\n", ret);
		}
	}

#ifdef CONFIG_IMX9_FLEXCAN1
	imx9_caninitialize(1);
#endif

#ifdef CONFIG_IMX9_FLEXCAN2
	imx9_caninitialize(2);
#endif

#ifdef CONFIG_IMX9_FLEXCAN3
	imx9_caninitialize(3);
#endif

#ifdef CONFIG_IMX9_FLEXCAN4
	imx9_caninitialize(4);
#endif

#ifdef CONFIG_IMX9_FLEXCAN4
	imx9_caninitialize(5);
#endif

	return ret;
}
