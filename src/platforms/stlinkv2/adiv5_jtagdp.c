/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2019  Uwe Bonnes(bon@elektron.ikp.physik.tu-darmstadt.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements a subset of JTAG-DP specific functions of the
 * ARM Debug Interface v5 Architecure Specification, ARM doc IHI0031A
 * used in BMP.
 */

#include "general.h"
//#include "exception.h"
#include "target.h"
#include "adiv5.h"
//#include "jtag_scan.h"
//#include "jtagtap.h"
#include "jtag_devs.h"

#define JTAGDP_ACK_OK	0x02
#define JTAGDP_ACK_WAIT	0x01

/* 35-bit registers that control the ADIv5 DP */
#define IR_ABORT	0x8
#define IR_DPACC	0xA
#define IR_APACC	0xB

static uint32_t adiv5_jtagdp_read(ADIv5_DP_t *dp, uint16_t addr);

static uint32_t adiv5_jtagdp_error(ADIv5_DP_t *dp);

static uint32_t adiv5_jtagdp_low_access(ADIv5_DP_t *dp, uint8_t RnW,
					uint16_t addr, uint32_t value);

static void adiv5_jtagdp_abort(ADIv5_DP_t *dp, uint32_t abort);

struct jtag_dev_s jtag_devs[JTAG_MAX_DEVS+1];
int jtag_dev_count;

int jtag_scan(const uint8_t *irlens)
{
	uint32_t idcodes[JTAG_MAX_DEVS+1];
	(void) *irlens;
	target_list_free();

	jtag_dev_count = 0;
	memset(&jtag_devs, 0, sizeof(jtag_devs));
	if (stlink_enter_debug_jtag())
		return 0;
	jtag_dev_count = stlink_read_idcodes(idcodes);
	/* Check for known devices and handle accordingly */
	for(int i = 0; i < jtag_dev_count; i++)
		jtag_devs[i].idcode = idcodes[i];
	for(int i = 0; i < jtag_dev_count; i++)
		for(int j = 0; dev_descr[j].idcode; j++)
			if((jtag_devs[i].idcode & dev_descr[j].idmask) ==
			   dev_descr[j].idcode) {
				if(dev_descr[j].handler)
					dev_descr[j].handler(&jtag_devs[i]);
				break;
			}

	return jtag_dev_count;
}

void adiv5_jtag_dp_handler(jtag_dev_t *dev)
{
	ADIv5_DP_t *dp = (void*)calloc(1, sizeof(*dp));

	dp->dev = dev;
	dp->idcode = dev->idcode;

	dp->dp_read = adiv5_jtagdp_read;
	dp->error = adiv5_jtagdp_error;
	dp->low_access = adiv5_jtagdp_low_access;
	dp->abort = adiv5_jtagdp_abort;

	adiv5_dp_init(dp);
}

static uint32_t adiv5_jtagdp_read(ADIv5_DP_t *dp, uint16_t addr)
{
	(void) dp;
	(void) addr;
	return 0;
}

static uint32_t adiv5_jtagdp_error(ADIv5_DP_t *dp)
{
	(void) dp;
	return 0;
}

static uint32_t adiv5_jtagdp_low_access(ADIv5_DP_t *dp, uint8_t RnW,
					uint16_t addr, uint32_t value)
{
	(void) dp;
	(void) RnW;
	(void) addr;
	(void) value;
	return 0;
}

static void adiv5_jtagdp_abort(ADIv5_DP_t *dp, uint32_t abort)
{
	(void) dp;
	(void) abort;
}

