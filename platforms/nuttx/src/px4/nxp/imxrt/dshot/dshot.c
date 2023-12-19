/****************************************************************************
 *
 * Copyright (C) 2023 PX4 Development Team. All rights reserved.
 * Author: Peter van der Perk <peter.vanderperk@nxp.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *	used to endorse or promote products derived from this software
 *	without specific prior written permission.
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
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/micro_hal.h>
#include <px4_platform_common/log.h>
#include <imxrt_flexio.h>
#include <hardware/imxrt_flexio.h>
#include <imxrt_periphclks.h>
#include <px4_arch/dshot.h>
#include <px4_arch/io_timer.h>
#include <drivers/drv_dshot.h>
#include <stdio.h>
#include "barriers.h"

#include "arm_internal.h"

#define FLEXIO_BASE			IMXRT_FLEXIO1_BASE
#define FLEXIO_PREQ			120000000
#define DSHOT_TIMERS			FLEXIO_SHIFTBUFNIS_COUNT
#define DSHOT_THROTTLE_POSITION		5u
#define DSHOT_TELEMETRY_POSITION	4u
#define NIBBLES_SIZE 			4u
#define DSHOT_NUMBER_OF_NIBBLES		3u

typedef struct dshot_handler_t {
	bool			init;
	uint32_t 		data_seg1;
	uint32_t 		irq_data;
} dshot_handler_t;

static dshot_handler_t dshot_inst[DSHOT_TIMERS] = {};

struct flexio_dev_s *flexio_s;

static int flexio_irq_handler(int irq, void *context, void *arg)
{

	uint32_t flags = get_shifter_status_flags();
	uint32_t channel;

	for (channel = 0; flags && channel < DSHOT_TIMERS; channel++) {
		if (flags & (1 << channel)) {
			disable_shifter_status_interrupts(1 << channel);

			if (dshot_inst[channel].irq_data != 0) {
				flexio_putreg32(dshot_inst[channel].irq_data, IMXRT_FLEXIO_SHIFTBUF0_OFFSET + channel * 0x4);
				dshot_inst[channel].irq_data = 0;

			} else if (dshot_inst[channel].irq_data == 0 && dshot_inst[channel].state == BDSHOT_RECEIVE) {
				dshot_inst[channel].state = BDSHOT_RECEIVE_COMPLETE;
				dshot_inst[channel].raw_response = flexio_getreg32(IMXRT_FLEXIO_SHIFTBUFBIS0_OFFSET + channel * 0x4);

				bdshot_recv_mask |= (1 << channel);

				if (bdshot_recv_mask == dshot_mask) {
					// Received telemetry on all channels
					// Schedule workqueue?
				}
			}
		}
	}

	return OK;
}


int up_dshot_init(uint32_t channel_mask, unsigned dshot_pwm_freq, bool enable_bidirectional_dshot)
{
	uint32_t timer_compare = 0x2F00 | (((BOARD_FLEXIO_PREQ / (dshot_pwm_freq * 3) / 2) - 1) & 0xFF);


	/* Init FlexIO peripheral */

	flexio_s = imxrt_flexio_initialize(1);
	up_enable_irq(IMXRT_IRQ_FLEXIO1);
	irq_attach(IMXRT_IRQ_FLEXIO1, flexio_irq_handler, 0);

	dshot_mask = 0x0;

	for (unsigned channel = 0; (channel_mask != 0) && (channel < DSHOT_TIMERS); channel++) {
		if (channel_mask & (1 << channel)) {
			uint8_t timer = timer_io_channels[channel].timer_index;

			if (io_timers[timer].dshot.pinmux == 0) { // board does not configure dshot on this pin
				continue;
			}

			imxrt_config_gpio(io_timers[timer].dshot.pinmux | IOMUX_PULL_UP);

			struct flexio_shifter_config_s shft_cfg;
			shft_cfg.timer_select = channel;
			shft_cfg.timer_polarity = FLEXIO_SHIFTER_TIMER_POLARITY_ON_POSITIVE;
			shft_cfg.pin_config = FLEXIO_PIN_CONFIG_OUTPUT;
			shft_cfg.pin_select = io_timers[timer].dshot.flexio_pin;
			shft_cfg.pin_polarity = FLEXIO_PIN_ACTIVE_HIGH;
			shft_cfg.shifter_mode = FLEXIO_SHIFTER_MODE_TRANSMIT;
			shft_cfg.parallel_width = 0;
			shft_cfg.input_source = FLEXIO_SHIFTER_INPUT_FROM_PIN;
			shft_cfg.shifter_stop = FLEXIO_SHIFTER_STOP_BIT_LOW;
			shft_cfg.shifter_start = FLEXIO_SHIFTER_START_BIT_DISABLED_LOAD_DATA_ON_ENABLE;

			flexio_dshot_output(channel, io_timers[timer].dshot.flexio_pin, dshot_tcmp, dshot_inst[channel].bdshot);

			dshot_inst[channel].init = true;

			// Mask channel to be active on dshot
			dshot_mask |= (1 << channel);
		}
	}

	flexio_modifyreg32(IMXRT_FLEXIO_CTRL_OFFSET, 0,
			   FLEXIO_CTRL_FLEXEN_MASK);

	return channel_mask;
}

void up_dshot_trigger(void)
{
	clear_timer_status_flags(0xFF);

	for (uint8_t channel = 0; (channel < DSHOT_TIMERS); channel++) {
		if ((bdshot_recv_mask & (1 << channel)) == 0) {
			dshot_inst[channel].no_response_cnt++;
		}

		if (dshot_inst[channel].init && dshot_inst[channel].data_seg1 != 0) {
			flexio_putreg32(dshot_inst[channel].data_seg1, IMXRT_FLEXIO_SHIFTBUF0_OFFSET + channel * 0x4);
		}
	}

	bdshot_recv_mask = 0x0;

	clear_timer_status_flags(0xFF);
	enable_shifter_status_interrupts(0xFF);
	enable_timer_status_interrupts(0xFF);
}

/* Expand packet from 16 bits 48 to get T0H and T1H timing */
uint64_t dshot_expand_data(uint16_t packet)
{
	unsigned int mask;
	unsigned int index = 0;
	uint64_t expanded = 0x0;

	for (mask = 0x8000; mask != 0; mask >>= 1) {
		if (packet & mask) {
			expanded = expanded | ((uint64_t)0x3 << index);

		} else {
			expanded = expanded | ((uint64_t)0x1 << index);
		}

		index = index + 3;
	}

	return expanded;
}

/**
* bits 	1-11	- throttle value (0-47 are reserved, 48-2047 give 2000 steps of throttle resolution)
* bit 	12		- dshot telemetry enable/disable
* bits 	13-16	- XOR checksum
**/
void dshot_motor_data_set(unsigned channel, uint16_t throttle, bool telemetry)
{
	uint8_t timer = timer_io_channels[channel].timer_index;

	if (channel < DSHOT_TIMERS && dshot_inst[channel].init) {
		uint16_t csum_data;
		uint16_t packet = 0;
		uint16_t checksum = 0;

		packet |= throttle << DSHOT_THROTTLE_POSITION;
		packet |= ((uint16_t)telemetry & 0x01) << DSHOT_TELEMETRY_POSITION;

		if (dshot_inst[channel].bdshot) {
			csum_data = ~packet;

		} else {
			csum_data = packet;
		}

		/* XOR checksum calculation */
		csum_data >>= NIBBLES_SIZE;

		for (unsigned i = 0; i < DSHOT_NUMBER_OF_NIBBLES; i++) {
			checksum ^= (csum_data & 0x0F); // XOR data by nibbles
			csum_data >>= NIBBLES_SIZE;
		}

		packet |= (checksum & 0x0F);

		uint64_t dshot_expanded = dshot_expand_data(packet);
		dshot_inst[channel].data_seg1 = (uint32_t)(dshot_expanded & 0xFFFFFF);
		dshot_inst[channel].irq_data = (uint32_t)(dshot_expanded >> 24);
		dshot_inst[channel].state = DSHOT_START;

		if (dshot_inst[channel].bdshot) {

			flexio_putreg32(0x0, IMXRT_FLEXIO_TIMCTL0_OFFSET + channel * 0x4);
			disable_shifter_status_interrupts(1 << channel);

			flexio_dshot_output(channel, io_timers[timer].dshot.flexio_pin, dshot_tcmp, dshot_inst[channel].bdshot);

			clear_timer_status_flags(0xFF);
		}
	}
}

int up_dshot_arm(bool armed)
{
	return io_timer_set_enable(armed, IOTimerChanMode_Dshot, IO_TIMER_ALL_MODES_CHANNELS);
}
