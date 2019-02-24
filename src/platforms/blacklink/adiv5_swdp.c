/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2019 Uwe Bonnes (bon@elektron.ikp.physik.tu-darmstadt.de)
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

/* This file implements the SW-DP specific functions of the
 * ARM Debug Interface v5 Architecure Specification, ARM doc IHI0031A.
 */

#include "general.h"
#include "exception.h"
#include "adiv5.h"
#include "swdptap.h"
#include "target.h"
#include "target_internal.h"
#include "stlinkv2.h"

static uint32_t adiv5_swdp_read(ADIv5_DP_t *dp, uint16_t addr);

static uint32_t adiv5_swdp_low_access(ADIv5_DP_t *dp, uint8_t RnW,
				      uint16_t addr, uint32_t value);

int adiv5_swdp_scan(void)
{
	target_list_free();
	ADIv5_DP_t *dp = (void*)calloc(1, sizeof(*dp));
	stlink_enter_debug_swd();
	dp->idcode = stlink_read_coreid();
	dp->dp_read = adiv5_swdp_read;
//	dp->error = adiv5_swdp_error;
	dp->low_access = adiv5_swdp_low_access;
//	dp->abort = adiv5_swdp_abort;

//	adiv5_swdp_error(dp);
	adiv5_dp_init(dp);

//	return target_list?1:0;
	return 0;
}
static uint32_t adiv5_swdp_read(ADIv5_DP_t *dp, uint16_t addr)
{
	if (addr & ADIV5_APnDP) {
		DEBUG ("AP read addr 0x%04" PRIx16 "\n", addr);
		adiv5_swdp_low_access(dp, ADIV5_LOW_READ, addr, 0);
		return adiv5_swdp_low_access(dp, ADIV5_LOW_READ,
		                           ADIV5_DP_RDBUFF, 0);
	} else {
		DEBUG ("DP read addr 0x%04" PRIx16 "\n", addr);
		return adiv5_swdp_low_access(dp, ADIV5_LOW_READ, addr, 0);
	}
}

static uint32_t adiv5_swdp_low_access(ADIv5_DP_t *dp, uint8_t RnW,
				      uint16_t addr, uint32_t value)
{
	uint32_t response = 0;
	int res;
	if (RnW) {
		res = stlink_read_dp_register(addr, &response);
		DEBUG("SWD read addr %04" PRIx16 ": %08" PRIx32 "\n", addr, response);
	} else {
		DEBUG("SWD write addr %04" PRIx16 ": %08" PRIx32 "\n", addr, value);
		res = stlink_write_dp_register(addr, value);
	}
	if (res == STLINK_ERROR_WAIT)
		raise_exception(EXCEPTION_TIMEOUT, "SWDP ACK timeout");

	if(res == STLINK_ERROR_FAIL) {
		dp->fault = 1;
		return 0;
	}
	return response;
}

#if 0

static uint32_t adiv5_swdp_error(ADIv5_DP_t *dp)
{
	uint32_t err, clr = 0;

	err = adiv5_swdp_read(dp, ADIV5_DP_CTRLSTAT) &
		(ADIV5_DP_CTRLSTAT_STICKYORUN | ADIV5_DP_CTRLSTAT_STICKYCMP |
		ADIV5_DP_CTRLSTAT_STICKYERR | ADIV5_DP_CTRLSTAT_WDATAERR);

	if(err & ADIV5_DP_CTRLSTAT_STICKYORUN)
		clr |= ADIV5_DP_ABORT_ORUNERRCLR;
	if(err & ADIV5_DP_CTRLSTAT_STICKYCMP)
		clr |= ADIV5_DP_ABORT_STKCMPCLR;
	if(err & ADIV5_DP_CTRLSTAT_STICKYERR)
		clr |= ADIV5_DP_ABORT_STKERRCLR;
	if(err & ADIV5_DP_CTRLSTAT_WDATAERR)
		clr |= ADIV5_DP_ABORT_WDERRCLR;

	adiv5_dp_write(dp, ADIV5_DP_ABORT, clr);
	dp->fault = 0;

	return err;
}

static void adiv5_swdp_abort(ADIv5_DP_t *dp, uint32_t abort)
{
	adiv5_dp_write(dp, ADIV5_DP_ABORT, abort);
}
#endif
