/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2008  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
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

/* Low level JTAG implementation using FT2232 with libftdi.
 *
 * Issues:
 * Should share interface with swdptap.c or at least clean up...
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <assert.h>

#include "general.h"
#include "jtagtap.h"

int jtagtap_init(void)
{
	return 0;
}

void jtagtap_reset(void)
{
	jtagtap_soft_reset();
}

void
jtagtap_tms_seq(uint32_t MS, int ticks)
{
	(void) MS;
	(void) ticks;
}

void
jtagtap_tdi_tdo_seq(uint8_t *DO, const uint8_t final_tms, const uint8_t *DI, int ticks)
{
	(void) DO;
	(void) final_tms;
	(void) DI;
	(void) ticks;
}

void jtagtap_tdi_seq(const uint8_t final_tms, const uint8_t *DI, int ticks)
{
	return jtagtap_tdi_tdo_seq(NULL, final_tms, DI, ticks);
}

uint8_t jtagtap_next(uint8_t dTMS, uint8_t dTDI)
{
	(void) dTMS;
	(void) dTDI;
	return true;
}

