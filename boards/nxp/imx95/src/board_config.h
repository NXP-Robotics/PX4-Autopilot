/****************************************************************************
 *
 *   Copyright (c) 2018-2019 PX4 Development Team. All rights reserved.
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
 * @file board_config.h
 *
 * PX4 fmu-v6xrt internal definitions
 */

#pragma once

/****************************************************************************************************
 * Included Files
 ****************************************************************************************************/

#include <nuttx/config.h>

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

#include "imx9_gpio.h"
#include "imx9_iomuxc.h"
#include "hardware/imx9_pinmux.h"

#include <arch/board/board.h>

/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/


/*
 * From the radion souce code
 * // Serial flow control
 * #define SERIAL_RTS PIN_ENABLE  // always an input
 * #define SERIAL_CTS PIN_CONFIG  // input in bootloader, output in app
 *
 * RTS is an out from FMU
 * CTS is in input to the FMU but the booloader on the radion will treat it as an input, and the
 * radion APP as output.
 *
 * To ensure radios do not go into bootloader mode because our CTS is configured with Pull downs
 * We init with pull ups, then enable power, then initalize the CTS will pull downs
 */

#define GPIO_LPUART4_CTS_INIT     PX4_MAKE_GPIO_PULLED_INPUT(GPIO_LPUART4_CTS, IOMUXC_PAD_PU_ON)
#define GPIO_LPUART8_CTS_INIT     PX4_MAKE_GPIO_PULLED_INPUT(GPIO_LPUART8_CTS, IOMUXC_PAD_PU_ON)
#define GPIO_LPUART10_CTS_INIT    PX4_MAKE_GPIO_PULLED_INPUT(GPIO_LPUART10_CTS,IOMUXC_PAD_PU_ON)

/*
 *  Define the ability to shut off off the sensor signals
 *  by changing the signals to inputs
 */

#define _PIN_OFF(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT | IOMUXC_PAD_PD_ON))

/*  Define the Chip Selects, Data Ready and Control signals per SPI bus */

#define CS_IOMUX  0 //FIXME
#define OUT_IOMUX 0 //FIXME


/* SPI1 off */

#define GPIO_LPSPI1_SCK   /* SAI1_TXD0 */  (GPIO_PORT1 | GPIO_PIN13 | CS_IOMUX)
#define GPIO_LPSPI1_MISO  /* SAI1_TXC  */  (GPIO_PORT1 | GPIO_PIN12 | CS_IOMUX)
#define GPIO_LPSPI1_MOSI  /* SAI1_RXD0 */  (GPIO_PORT1 | GPIO_PIN14 | CS_IOMUX)

#define GPIO_SPI1_SCK_OFF   _PIN_OFF(_GPIO_LPSPI1_SCK)
#define GPIO_SPI1_MISO_OFF  _PIN_OFF(_GPIO_LPSPI1_MISO)
#define GPIO_SPI1_MOSI_OFF  _PIN_OFF(_GPIO_LPSPI1_MOSI)

/* SPI2 off */

#define _GPIO_LPSPI2_SCK   /* GPIO_AD_24  GPIO3_IO23 */  (GPIO_PORT3 | GPIO_PIN23 | CS_IOMUX)
#define _GPIO_LPSPI2_MISO  /* GPIO_AD_27  GPIO3_IO26 */  (GPIO_PORT3 | GPIO_PIN26 | CS_IOMUX)
#define _GPIO_LPSPI2_MOSI  /* GPIO_AD_26  GPIO3_IO25 */  (GPIO_PORT3 | GPIO_PIN25 | CS_IOMUX)

#define GPIO_SPI2_SCK_OFF   _PIN_OFF(_GPIO_LPSPI2_SCK)
#define GPIO_SPI2_MISO_OFF  _PIN_OFF(_GPIO_LPSPI2_MISO)
#define GPIO_SPI2_MOSI_OFF  _PIN_OFF(_GPIO_LPSPI2_MOSI)

/* SPI3 off */

#define _GPIO_LPSPI3_SCK   /* GPIO_EMC_B2_04 GPIO2_IO14 */  (GPIO_PORT2 | GPIO_PIN14 | CS_IOMUX)
#define _GPIO_LPSPI3_MISO  /* GPIO_EMC_B2_07 GPIO2_IO17 */  (GPIO_PORT2 | GPIO_PIN17 | CS_IOMUX)
#define _GPIO_LPSPI3_MOSI  /* GPIO_EMC_B2_06 GPIO2_IO16 */  (GPIO_PORT2 | GPIO_PIN16 | CS_IOMUX)

#define GPIO_SPI3_SCK_OFF   _PIN_OFF(_GPIO_LPSPI3_SCK)
#define GPIO_SPI3_MISO_OFF  _PIN_OFF(_GPIO_LPSPI3_MISO)
#define GPIO_SPI3_MOSI_OFF  _PIN_OFF(_GPIO_LPSPI3_MOSI)

/* SPI4 off */

#define GPIO_LPSPI4_SCK   /* GPIO_IO37 */  (GPIO_PORT5 | GPIO_PIN17 | CS_IOMUX)
#define GPIO_LPSPI4_MISO  /* GPIO_IO19 */  (GPIO_PORT2 | GPIO_PIN19 | CS_IOMUX)
#define GPIO_LPSPI4_MOSI  /* GPIO_IO36 */  (GPIO_PORT5 | GPIO_PIN16 | CS_IOMUX)

#define GPIO_SPI4_SCK_OFF   _PIN_OFF(_GPIO_LPSPI4_SCK)
#define GPIO_SPI4_MISO_OFF  _PIN_OFF(_GPIO_LPSPI4_MISO)
#define GPIO_SPI4_MOSI_OFF  _PIN_OFF(_GPIO_LPSPI4_MOSI)

/* SPI6 off */

#define _GPIO_LPSPI6_SCK   /* GPIO_LPSR_10 GPIO6_IO10 */  (GPIO_PORT6 | GPIO_PIN10 | CS_IOMUX)
#define _GPIO_LPSPI6_MISO  /* GPIO_LPSR_12 GPIO6_IO12 */  (GPIO_PORT6 | GPIO_PIN12 | CS_IOMUX)
#define _GPIO_LPSPI6_MOSI  /* GPIO_LPSR_11 GPIO6_IO11 */  (GPIO_PORT6 | GPIO_PIN11 | CS_IOMUX)

#define GPIO_SPI6_SCK_OFF   _PIN_OFF(_GPIO_LPSPI6_SCK)
#define GPIO_SPI6_MISO_OFF  _PIN_OFF(_GPIO_LPSPI6_MISO)
#define GPIO_SPI6_MOSI_OFF  _PIN_OFF(_GPIO_LPSPI6_MOSI)


/*  Define the SPI Data Ready and Control signals */
#define DRDY_IOMUX (IOMUXC_PAD_PU_ON)


/*  SPI1 */

#define GPIO_SPI1_DRDY1_SENSOR1   /* GPIO_AD_20      GPIO3_IO19 */ (GPIO_PORT3 | GPIO_PIN19  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI2_DRDY1_SENSOR2   /* GPIO_EMC_B1_39  GPIO2_IO07 */ (GPIO_PORT2 | GPIO_PIN07  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI3_DRDY1_SENSOR3   /* GPIO_AD_21      GPIO3_IO20 */ (GPIO_PORT3 | GPIO_PIN20  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI3_DRDY2_SENSOR3   /* GPIO_EMC_B2_09  GPIO2_IO19 */ (GPIO_PORT2 | GPIO_PIN19  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI4_DRDY1_SENSOR4   /* GPIO_EMC_B1_16  GPIO1_IO16 */ (GPIO_PORT1 | GPIO_PIN16  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI6_DRDY1_EXTERNAL1 /* GPIO_EMC_B1_05  GPIO1_IO05 */ (GPIO_PORT1 | GPIO_PIN05  | GPIO_INPUT  | DRDY_IOMUX)
#define GPIO_SPI6_DRDY2_EXTERNAL1 /* GPIO_EMC_B1_07  GPIO1_IO07 */ (GPIO_PORT1 | GPIO_PIN07  | GPIO_INPUT  | DRDY_IOMUX)


#define GPIO_SPI6_nRESET_EXTERNAL1  /* GPIO_EMC_B1_11 GPIO1_IO11 */ (GPIO_PORT1 | GPIO_PIN11 | GPIO_OUTPUT | GPIO_OUTPUT_ONE | OUT_IOMUX)
#define GPIO_SPIX_SYNC              /* GPIO_EMC_B1_18 GPIO1_IO18 */ (GPIO_PORT1 | GPIO_PIN18  | GPIO_OUTPUT | GPIO_OUTPUT_ONE | OUT_IOMUX)

#define GPIO_DRDY_OFF_SPI6_DRDY2_EXTERNAL1   _PIN_OFF(GPIO_SPI6_DRDY2_EXTERNAL1)
#define GPIO_SPI6_nRESET_EXTERNAL1_OFF       _PIN_OFF(GPIO_SPI6_nRESET_EXTERNAL1)
#define GPIO_SPIX_SYNC_OFF                   _PIN_OFF(GPIO_SPIX_SYNC)

/* Define Channel numbers must match above GPIO pin IN(n)*/

#define ADC_BATTERY_VOLTAGE_CHANNEL         /* VIN          ADC_IN0 */  0
#define ADC_BATTERY_CURRENT_CHANNEL         /* Not available        */ -1
#define ADC_5V_RAIL_SENSE                   /* VDD_SYS_5V0  ADC_IN1 */  1
#define ADC_SCALED_VDD_3V3_SENSORS1_CHANNEL /* VDD_SYS_3V3  ADC_IN2 */  2
#define ADC_SCALED_VDD_3V3_SENSORS2_CHANNEL /* PF09 AMUX    ADC_IN3 */  3
#define ADC_ADC_6V6_CHANNEL                 /* VDD_CON_PWM  ADC_IN6 */  6
#define ADC_ADC_3V3_CHANNEL                 /* VDD_USB_UART ADC_IN7 */  7

#define ADC_V5_V_FULL_SCALE                 (13.2f)  // 5 volt, divided by 4

#define ADC_CHANNELS \
	((1 << ADC_BATTERY_VOLTAGE_CHANNEL)  | \
	 (1 << ADC_5V_RAIL_SENSE)  | \
	 (1 << ADC_SCALED_VDD_3V3_SENSORS1_CHANNEL)  | \
	 (1 << ADC_SCALED_VDD_3V3_SENSORS2_CHANNEL)                | \
	 (1 << ADC_ADC_6V6_CHANNEL)                  | \
	 (1 << ADC_ADC_3V3_CHANNEL))

// The ADC is used in SCALED mode.
// The V that is converted to a DN is 30/64 of Vin of the pin.
// The DN is therfore 30/64 of the real voltage

#define BOARD_ADC_POS_REF_V (1.825f * 64.0f / 30.0f)

#define SYSTEM_ADC_BASE     IMX9_ADC_BASE

#define BOARD_I2C_LATEINIT 1 /* See Note about SE550 Eanable */

/* nARMED GPIO1_IO17
 *  The GPIO will be set as input while not armed HW will have external HW Pull UP.
 *  While armed it shall be configured at a GPIO OUT set LOW
 */
#define nARMED_INPUT_IOMUX  (/*IOMUXC_PAD_PU_ON | */0)
#define nARMED_OUTPUT_IOMUX (/*IOMUX_PULL_KEEP | IOMUX_SLEW_FAST | */0)

#define GPIO_nARMED_INIT     /* GPIO1_IO17 */ (GPIO_PORT1 | GPIO_PIN17 | GPIO_INPUT | nARMED_INPUT_IOMUX)
#define GPIO_nARMED          /* GPIO1_IO17 */ (GPIO_PORT1 | GPIO_PIN17 | GPIO_OUTPUT | GPIO_OUTPUT_ZERO | nARMED_OUTPUT_IOMUX)

#define BOARD_INDICATE_EXTERNAL_LOCKOUT_STATE(enabled)  px4_arch_configgpio((enabled) ? GPIO_nARMED : GPIO_nARMED_INIT)
#define BOARD_GET_EXTERNAL_LOCKOUT_STATE() px4_arch_gpioread(GPIO_nARMED)

/* PWM Capture
 *
 * 2  PWM Capture inputs are supported
 */
#define DIRECT_PWM_CAPTURE_CHANNELS  1
#define CAP_IOMUX (IOMUX_PULL_NONE | IOMUX_SLEW_FAST)
#define GPIO_FMU_CAP1 /* GPIO_EMC_B1_20 TMR4_TIMER0 */  (GPIO_QTIMER4_TIMER0_1 | CAP_IOMUX)

/* PWM
 */

#define DIRECT_PWM_OUTPUT_CHANNELS  7

// Input Capture not supported on MVP

#define BOARD_HAS_NO_CAPTURE

/* Power supply control and monitoring GPIOs */

#define GENERAL_INPUT_IOMUX  (IOMUXC_PAD_PU_ON)
#define GENERAL_OUTPUT_IOMUX (/*IOMUX_PULL_KEEP | IOMUX_SLEW_FAST */0)

#define BOARD_NUMBER_BRICKS             1
#define BOARD_ADC_BRICK_VALID           1

/* Tone alarm output */

#define TONE_ALARM_TIMER        3  /* GPT 3 */
#define TONE_ALARM_CHANNEL      2  /* GPIO_EMC_B2_09 GPT3_COMPARE2 */

#define GPIO_BUZZER_1           /* GPIO_EMC_B2_09  GPIO2_IO19  */ (GPIO_PORT2 | GPIO_PIN19  | GPIO_OUTPUT | GPIO_OUTPUT_ZERO | GENERAL_OUTPUT_IOMUX)

#define GPIO_TONE_ALARM_IDLE    GPIO_BUZZER_1
#define GPIO_TONE_ALARM         (GPIO_GPT3_COMPARE2_1 | GENERAL_OUTPUT_IOMUX)

#define RC_SERIAL_PORT                  "/dev/ttyS1"

/* Safety Switch is HW version dependent on having an PX4IO
 * So we init to a benign state with the _INIT definition
 * and provide the the non _INIT one for the driver to make a run time
 * decision to use it.
 */
#define SAFETY_INIT_IOMUX (/*IOMUX_PULL_NONE */ 0)
#define SAFETY_IOMUX      (/* IOMUX_PULL_NONE | IOMUX_SLEW_SLOW*/ 0)
#define SAFETY_SW_IOMUX   (/* IOMUXC_PAD_PU_ON */0)

#define GPIO_nSAFETY_SWITCH_LED_OUT_INIT   /* GPIO_EMC_B1_03 GPIO1_IO03 */ (GPIO_PORT1 | GPIO_PIN3 | GPIO_INPUT  | SAFETY_INIT_IOMUX)
#define GPIO_nSAFETY_SWITCH_LED_OUT        /* GPIO_EMC_B1_03 GPIO1_IO03 */ (GPIO_PORT1 | GPIO_PIN3 | GPIO_OUTPUT | GPIO_OUTPUT_ONE | SAFETY_IOMUX)

/* Enable the FMU to control it if there is no px4io fixme:This should be BOARD_SAFETY_LED(__ontrue) */
#define GPIO_LED_SAFETY GPIO_nSAFETY_SWITCH_LED_OUT
#define GPIO_SAFETY_SWITCH_IN              /* GPIO_EMC_B1_24 GPIO1_IO24 */ (GPIO_PORT1 | GPIO_PIN24 | GPIO_INPUT | SAFETY_SW_IOMUX)
/* Enable the FMU to use the switch it if there is no px4io fixme:This should be BOARD_SAFTY_BUTTON() */
#define GPIO_BTN_SAFETY GPIO_SAFETY_SWITCH_IN /* Enable the FMU to control it if there is no px4io */

#define SDIO_SLOTNO                    0  /* Only one slot */
#define SDIO_MINOR                     0

/* SD card bringup does not work if performed on the IDLE thread because it
 * will cause waiting.  Use either:
 *
 *  CONFIG_BOARDCTL=y, OR
 *  CONFIG_BOARD_INITIALIZE=y && CONFIG_BOARD_INITTHREAD=y
 */

#if defined(CONFIG_BOARD_INITIALIZE) && !defined(CONFIG_LIB_BOARDCTL) && \
   !defined(CONFIG_BOARD_INITTHREAD)
#  warning SDIO initialization cannot be perfomed on the IDLE thread
#endif

/* By Providing BOARD_ADC_USB_CONNECTED (using the px4_arch abstraction)
 * this board support the ADC system_power interface, and therefore
 * provides the true logic GPIO BOARD_ADC_xxxx macros.
 */

#define BOARD_ADC_USB_VALID     (1)
#define BOARD_ADC_USB_CONNECTED (1)

/* FMUv5 never powers odd the Servo rail */

#define BOARD_ADC_SERVO_VALID     (1)

/* This board provides a DMA pool and APIs */
#define BOARD_DMA_ALLOC_POOL_SIZE 5120

#define BOARD_HAS_ON_RESET 1

#define PX4_GPIO_INIT_LIST { \
	}

#define BOARD_ENABLE_CONSOLE_BUFFER
#define PX4_I2C_BUS_MTD      1

#define LPTMR2_CLK		   (LPTMR2_CLK_ROOT_OSC_24M_CLK | CLOCK_DIV(24))

__BEGIN_DECLS

/****************************************************************************************************
 * Public Types
 ****************************************************************************************************/

/****************************************************************************************************
 * Public data
 ****************************************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************************************
 * Public Functions
 ****************************************************************************************************/

extern void imx9_spiinitialize(void);
extern void imx9_lpspi1select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected);
extern void imx9_lpspi4select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected);
extern void imx9_lpspi8select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
