/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2018 Uwe Bonnes (bon@elektron.ikp.physik.tu-darmstadt.de)
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

/* MPSSE bit-banging SW-DP interface over FTDI with loop unrolled.
 * Speed is sensible.
 */

#include <stdio.h>
#include <assert.h>

#include "general.h"
#include "swdptap.h"

int swdptap_init(void)
{
	return 0;
}

bool swdptap_bit_in(void)
{
	return true;
}

void swdptap_bit_out(bool val)
{
	(void) val;
}

bool swdptap_seq_in_parity(uint32_t *res, int ticks)
{
	(void) res;
	(void) ticks;
	return true;
}

uint32_t swdptap_seq_in(int ticks)
{
	(void) ticks;
	return 0;
}

void swdptap_seq_out(uint32_t MS, int ticks)
{
	(void) ticks;
	(void) MS;
}

void swdptap_seq_out_parity(uint32_t MS, int ticks)
{
	(void) ticks;
	(void) MS;
}
