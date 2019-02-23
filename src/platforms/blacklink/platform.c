/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.	 If not, see <http://www.gnu.org/licenses/>.
 */
#include "general.h"
#include "gdb_if.h"
#include "version.h"
#include "platform.h"

#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define VENDOR_ID_STLINK		0x483
#define PRODUCT_ID_STLINK_MASK	0xfff0
#define PRODUCT_ID_STLINK_GROUP 0x3740
#define PRODUCT_ID_STLINKV1		0x3744
#define PRODUCT_ID_STLINKV2		0x3748
#define PRODUCT_ID_STLINKV21	0x374b
#define PRODUCT_ID_STLINKV21_MSD 0x3752
#define PRODUCT_ID_STLINKV3		0x374f

#define STLINK_SWIM_ERR_OK             0x00
#define STLINK_SWIM_BUSY               0x01
#define STLINK_DEBUG_ERR_OK            0x80
#define STLINK_DEBUG_ERR_FAULT         0x81
#define STLINK_SWD_AP_WAIT             0x10
#define STLINK_SWD_AP_FAULT            0x11
#define STLINK_SWD_AP_ERROR            0x12
#define STLINK_SWD_AP_PARITY_ERROR     0x13
#define STLINK_JTAG_WRITE_ERROR        0x0c
#define STLINK_JTAG_WRITE_VERIF_ERROR  0x0d
#define STLINK_SWD_DP_WAIT             0x14
#define STLINK_SWD_DP_FAULT            0x15
#define STLINK_SWD_DP_ERROR            0x16
#define STLINK_SWD_DP_PARITY_ERROR     0x17

#define STLINK_SWD_AP_WDATA_ERROR      0x18
#define STLINK_SWD_AP_STICKY_ERROR     0x19
#define STLINK_SWD_AP_STICKYORUN_ERROR 0x1a

#define STLINK_CORE_RUNNING            0x80
#define STLINK_CORE_HALTED             0x81
#define STLINK_CORE_STAT_UNKNOWN       -1

#define STLINK_GET_VERSION             0xF1
#define STLINK_DEBUG_COMMAND           0xF2
#define STLINK_DFU_COMMAND             0xF3
#define STLINK_SWIM_COMMAND            0xF4
#define STLINK_GET_CURRENT_MODE        0xF5
#define STLINK_GET_TARGET_VOLTAGE      0xF7

#define STLINK_DEV_DFU_MODE            0x00
#define STLINK_DEV_MASS_MODE           0x01
#define STLINK_DEV_DEBUG_MODE          0x02
#define STLINK_DEV_SWIM_MODE           0x03
#define STLINK_DEV_BOOTLOADER_MODE     0x04
#define STLINK_DEV_UNKNOWN_MODE        -1

#define STLINK_DFU_EXIT                0x07

/*
	STLINK_SWIM_ENTER_SEQ
	1.3ms low then 750Hz then 1.5kHz

	STLINK_SWIM_GEN_RST
	STM8 DM pulls reset pin low 50us

	STLINK_SWIM_SPEED
	uint8_t (0=low|1=high)

	STLINK_SWIM_WRITEMEM
	uint16_t length
	uint32_t address

	STLINK_SWIM_RESET
	send syncronization seq (16us low, response 64 clocks low)
*/
#define STLINK_SWIM_ENTER                  0x00
#define STLINK_SWIM_EXIT                   0x01
#define STLINK_SWIM_READ_CAP               0x02
#define STLINK_SWIM_SPEED                  0x03
#define STLINK_SWIM_ENTER_SEQ              0x04
#define STLINK_SWIM_GEN_RST                0x05
#define STLINK_SWIM_RESET                  0x06
#define STLINK_SWIM_ASSERT_RESET           0x07
#define STLINK_SWIM_DEASSERT_RESET         0x08
#define STLINK_SWIM_READSTATUS             0x09
#define STLINK_SWIM_WRITEMEM               0x0a
#define STLINK_SWIM_READMEM                0x0b
#define STLINK_SWIM_READBUF                0x0c

#define STLINK_DEBUG_GETSTATUS             0x01
#define STLINK_DEBUG_FORCEDEBUG            0x02
#define STLINK_DEBUG_APIV1_RESETSYS        0x03
#define STLINK_DEBUG_APIV1_READALLREGS     0x04
#define STLINK_DEBUG_APIV1_READREG         0x05
#define STLINK_DEBUG_APIV1_WRITEREG        0x06
#define STLINK_DEBUG_READMEM_32BIT         0x07
#define STLINK_DEBUG_WRITEMEM_32BIT        0x08
#define STLINK_DEBUG_RUNCORE               0x09
#define STLINK_DEBUG_STEPCORE              0x0a
#define STLINK_DEBUG_APIV1_SETFP           0x0b
#define STLINK_DEBUG_READMEM_8BIT          0x0c
#define STLINK_DEBUG_WRITEMEM_8BIT         0x0d
#define STLINK_DEBUG_APIV1_CLEARFP         0x0e
#define STLINK_DEBUG_APIV1_WRITEDEBUGREG   0x0f
#define STLINK_DEBUG_APIV1_SETWATCHPOINT   0x10

#define STLINK_DEBUG_ENTER_JTAG_RESET      0x00
#define STLINK_DEBUG_ENTER_SWD_NO_RESET    0xa3
#define STLINK_DEBUG_ENTER_JTAG_NO_RESET   0xa4

#define STLINK_DEBUG_APIV1_ENTER           0x20
#define STLINK_DEBUG_EXIT                  0x21
#define STLINK_DEBUG_READCOREID            0x22

#define STLINK_DEBUG_APIV2_ENTER           0x30
#define STLINK_DEBUG_APIV2_READ_IDCODES    0x31
#define STLINK_DEBUG_APIV2_RESETSYS        0x32
#define STLINK_DEBUG_APIV2_READREG         0x33
#define STLINK_DEBUG_APIV2_WRITEREG        0x34
#define STLINK_DEBUG_APIV2_WRITEDEBUGREG   0x35
#define STLINK_DEBUG_APIV2_READDEBUGREG    0x36

#define STLINK_DEBUG_APIV2_READALLREGS     0x3A
#define STLINK_DEBUG_APIV2_GETLASTRWSTATUS 0x3B
#define STLINK_DEBUG_APIV2_DRIVE_NRST      0x3C

#define STLINK_DEBUG_APIV2_GETLASTRWSTATUS2 0x3E

#define STLINK_DEBUG_APIV2_START_TRACE_RX  0x40
#define STLINK_DEBUG_APIV2_STOP_TRACE_RX   0x41
#define STLINK_DEBUG_APIV2_GET_TRACE_NB    0x42
#define STLINK_DEBUG_APIV2_SWD_SET_FREQ    0x43
#define STLINK_DEBUG_APIV2_JTAG_SET_FREQ   0x44
#define STLINK_DEBUG_APIV2_READ_DAP_REG    0x45
#define STLINK_DEBUG_APIV2_WRITE_DAP_REG   0x46
#define STLINK_DEBUG_APIV2_READMEM_16BIT   0x47
#define STLINK_DEBUG_APIV2_WRITEMEM_16BIT  0x48

#define STLINK_DEBUG_APIV2_INIT_AP         0x4B
#define STLINK_DEBUG_APIV2_CLOSE_AP_DBG    0x4C

#define STLINK_APIV3_SET_COM_FREQ           0x61
#define STLINK_APIV3_GET_COM_FREQ           0x62

#define STLINK_APIV3_GET_VERSION_EX         0xFB

#define STLINK_DEBUG_APIV2_DRIVE_NRST_LOW   0x00
#define STLINK_DEBUG_APIV2_DRIVE_NRST_HIGH  0x01
#define STLINK_DEBUG_APIV2_DRIVE_NRST_PULSE 0x02

#define STLINK_DEBUG_PORT_ACCESS            0xffff

#define STLINK_TRACE_SIZE               4096
#define STLINK_TRACE_MAX_HZ             2000000

#define STLINK_V3_MAX_FREQ_NB               10

/** */
enum stlink_mode {
	STLINK_MODE_UNKNOWN = 0,
	STLINK_MODE_DFU,
	STLINK_MODE_MASS,
	STLINK_MODE_DEBUG_JTAG,
	STLINK_MODE_DEBUG_SWD,
	STLINK_MODE_DEBUG_SWIM
};

typedef struct {
	libusb_context* libusb_ctx;
	uint16_t     vid;
	uint16_t     pid;
	uint8_t      serial[32];
	uint8_t      ver_stlink;
	uint8_t      ver_api;
	uint8_t      ver_jtag;
	uint8_t      ver_mass;
	uint8_t      ver_swim;
	uint8_t      ver_bridge;
	libusb_device_handle *handle;
	struct libusb_transfer* req_trans;
	struct libusb_transfer* rep_trans;
} stlink;

stlink Stlink;

static void exit_function(void)
{
	libusb_exit(NULL);
	DEBUG("Normal termination\n");
}

/* SIGTERM handler. */
static void sigterm_handler(int sig)
{
	(void)sig;
	exit(0);
}

struct trans_ctx {
#define TRANS_FLAGS_IS_DONE (1 << 0)
#define TRANS_FLAGS_HAS_ERROR (1 << 1)
    volatile unsigned long flags;
};

static void on_trans_done(struct libusb_transfer * trans)
{
    struct trans_ctx * const ctx = trans->user_data;

    if (trans->status != LIBUSB_TRANSFER_COMPLETED)
    {
        if(trans->status == LIBUSB_TRANSFER_TIMED_OUT)
        {
            DEBUG("Timeout\n");
        }
        else if (trans->status == LIBUSB_TRANSFER_CANCELLED)
            DEBUG("cancelled\n");
        else if (trans->status == LIBUSB_TRANSFER_NO_DEVICE)
            DEBUG("no device\n");
        else
            DEBUG("unknown\n");
        ctx->flags |= TRANS_FLAGS_HAS_ERROR;
    }
    ctx->flags |= TRANS_FLAGS_IS_DONE;
}

static int submit_wait(struct libusb_transfer * trans) {
	struct timeval start;
	struct timeval now;
	struct timeval diff;
	struct trans_ctx trans_ctx;
	enum libusb_error error;

	trans_ctx.flags = 0;

	/* brief intrusion inside the libusb interface */
	trans->callback = on_trans_done;
	trans->user_data = &trans_ctx;

	if ((error = libusb_submit_transfer(trans))) {
		DEBUG("libusb_submit_transfer(%d): %s\n", error,
			  libusb_strerror(error));
		return -1;
	}

	gettimeofday(&start, NULL);

	while (trans_ctx.flags == 0) {
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		if (libusb_handle_events_timeout(Stlink.libusb_ctx, &timeout)) {
			DEBUG("libusb_handle_events()\n");
			return -1;
		}

		gettimeofday(&now, NULL);
		timersub(&now, &start, &diff);
		if (diff.tv_sec >= 1) {
			libusb_cancel_transfer(trans);
			DEBUG("libusb_handle_events() timeout\n");
			return -1;
		}
	}

	if (trans_ctx.flags & TRANS_FLAGS_HAS_ERROR) {
		DEBUG("libusb_handle_events() | has_error\n");
		return -1;
	}

	return 0;
}

static int send_recv(uint8_t *txbuf, size_t txsize,
					 uint8_t *rxbuf, size_t rxsize)
{
	int res = 0;
	int ep_tx = 1;
	if( txsize) {
		libusb_fill_bulk_transfer(Stlink.req_trans, Stlink.handle,
								  ep_tx | LIBUSB_ENDPOINT_OUT,
								  txbuf, txsize,
								  NULL, NULL,
								  0
			);

		if (submit_wait(Stlink.req_trans)) {
			DEBUG("clear 2\n");
			libusb_clear_halt(Stlink.handle,2);
			return -1;
		}
	}
	/* send_only */
	if (rxsize != 0) {
		/* read the response */
		libusb_fill_bulk_transfer(Stlink.rep_trans, Stlink.handle,
								  0x01| LIBUSB_ENDPOINT_IN,
								  rxbuf, rxsize, NULL, NULL, 0);

		if (submit_wait(Stlink.rep_trans)) {
			DEBUG("clear 1\n");
			libusb_clear_halt(Stlink.handle,1);
			return -1;
		}
		res = Stlink.rep_trans->actual_length;
		if (res >0) {
			int i;
			uint8_t *p = rxbuf;
			DEBUG("Transfer ");
			for (i=0; i< res && i <32 ; i++)
				DEBUG("%02x", p[i]);
			DEBUG("\n");
		}
	}
	return res;
}

#define ERROR_FAIL -1
#define ERROR_OK 0
#define ERROR_WAIT 1
/**
    Converts an STLINK status code held in the first byte of a response to
	readable error
*/
static int stlink_usb_error_check(uint8_t *data)
{
	switch (data[0]) {
		case STLINK_DEBUG_ERR_OK:
			return ERROR_OK;
		case STLINK_DEBUG_ERR_FAULT:
			DEBUG("SWD fault response (0x%x)", STLINK_DEBUG_ERR_FAULT);
			return ERROR_FAIL;
		case STLINK_SWD_AP_WAIT:
			DEBUG("wait status SWD_AP_WAIT (0x%x)", STLINK_SWD_AP_WAIT);
			return ERROR_WAIT;
		case STLINK_SWD_DP_WAIT:
			DEBUG("wait status SWD_DP_WAIT (0x%x)", STLINK_SWD_DP_WAIT);
			return ERROR_WAIT;
		case STLINK_JTAG_WRITE_ERROR:
			DEBUG("Write error");
			return ERROR_FAIL;
		case STLINK_JTAG_WRITE_VERIF_ERROR:
			DEBUG("Write verify error, ignoring");
			return ERROR_OK;
		case STLINK_SWD_AP_FAULT:
			/* git://git.ac6.fr/openocd commit 657e3e885b9ee10
			 * returns ERROR_OK with the comment:
			 * Change in error status when reading outside RAM.
			 * This fix allows CDT plugin to visualize memory.
			 */
			DEBUG("STLINK_SWD_AP_FAULT");
			return ERROR_FAIL;
		case STLINK_SWD_AP_ERROR:
			DEBUG("STLINK_SWD_AP_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_AP_PARITY_ERROR:
			DEBUG("STLINK_SWD_AP_PARITY_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_DP_FAULT:
			DEBUG("STLINK_SWD_DP_FAULT");
			return ERROR_FAIL;
		case STLINK_SWD_DP_ERROR:
			DEBUG("STLINK_SWD_DP_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_DP_PARITY_ERROR:
			DEBUG("STLINK_SWD_DP_PARITY_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_AP_WDATA_ERROR:
			DEBUG("STLINK_SWD_AP_WDATA_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_AP_STICKY_ERROR:
			DEBUG("STLINK_SWD_AP_STICKY_ERROR");
			return ERROR_FAIL;
		case STLINK_SWD_AP_STICKYORUN_ERROR:
			DEBUG("STLINK_SWD_AP_STICKYORUN_ERROR");
			return ERROR_FAIL;
		default:
			DEBUG("unknown/unexpected STLINK status code 0x%x", data[0]);
			return ERROR_FAIL;
	}
}
static void platform_version(void)
{
	uint8_t cmd[1] = {STLINK_GET_VERSION};
	uint8_t data[16];
    int size = send_recv(cmd, 1, data, 6);
    if (size == -1) {
        printf("[!] send_recv STLINK_GET_VERSION\n");
    }
	Stlink.vid = data[3] << 8 | data[2];
	Stlink.pid = data[5] << 8 | data[4];
	int  version = data[0] << 8 | data[1];
	Stlink.ver_stlink = (version >> 12) & 0x0f;
	if (Stlink.ver_stlink == 3) {
		cmd[0] = STLINK_APIV3_GET_VERSION_EX;
		int size = send_recv(cmd, 1, data, 16);
		if (size == -1) {
			printf("[!] send_recv STLINK_APIV3_GET_VERSION_EX\n");
		}
		Stlink.ver_swim  =  data[1];
		Stlink.ver_jtag  =  data[2];
		Stlink.ver_mass  =  data[3];
		Stlink.ver_bridge = data[4];
	} else {
		Stlink.ver_jtag  =  (version >>  6) & 0x3f;
		if ((Stlink.pid == PRODUCT_ID_STLINKV21_MSD) ||
			(Stlink.pid == PRODUCT_ID_STLINKV21)) {
			Stlink.ver_mass  =  (version >>  0) & 0x3f;
		}
	}
	DEBUG("V%dJ%d",Stlink.ver_stlink, Stlink.ver_jtag);
	if (Stlink.ver_api == 30)
		DEBUG("M%dB%dS%d", Stlink.ver_mass, Stlink.ver_bridge, Stlink.ver_swim);
	else if (Stlink.ver_api == 20)
 		DEBUG("S%d", Stlink.ver_swim);
	else if (Stlink.ver_api == 21)
 		DEBUG("M%d", Stlink.ver_mass);
}

static void stlink_leave_state(void)
{
	uint8_t cmd[2] = {STLINK_GET_CURRENT_MODE};
	uint8_t data[2];
	send_recv(cmd, 1, data, 2);
	if (data[0] == STLINK_DEV_DFU_MODE) {
		uint8_t dfu_cmd[2] = {STLINK_DFU_COMMAND, STLINK_DFU_EXIT};
		DEBUG("Leaving DFU Mode\n");
		send_recv(dfu_cmd, 2, NULL, 0);
	} else if (data[0] == STLINK_DEV_SWIM_MODE) {
		uint8_t swim_cmd[2] = {STLINK_SWIM_COMMAND, STLINK_SWIM_EXIT};
		DEBUG("Leaving SWIM Mode\n");
		send_recv(swim_cmd, 2, NULL, 0);
	} else if (data[0] == STLINK_DEV_DEBUG_MODE) {
		uint8_t dbg_cmd[2] = {STLINK_DEBUG_COMMAND, STLINK_DEBUG_EXIT};
		DEBUG("Leaving DEBUG Mode\n");
		send_recv(dbg_cmd, 2, NULL, 0);
	} else if (data[0] == STLINK_DEV_BOOTLOADER_MODE) {
		DEBUG("BOOTLOADER Mode\n");
	} else if (data[0] == STLINK_DEV_MASS_MODE) {
		DEBUG("MASS Mode\n");
	} else {
		DEBUG("Unknown Mode\n");
	}
}

const char *platform_target_voltage(void)
{
	uint8_t cmd[2] = {STLINK_GET_TARGET_VOLTAGE};
	uint8_t data[8];
	send_recv(cmd, 1, data, 8);
	uint16_t adc[2];
	adc[0] = data[0] | data[1] << 8; /* Calibration value? */
	adc[1] = data[4] | data[5] << 8; /* Measured value?*/
	float result  = 0.0;
	if (adc[0])
		result = 2.0 * adc[1] * 1.2 / adc[0];
	static char res[6];
	sprintf(res, "%4.2fV", result);
	return res;
}

void platform_init(int argc, char **argv)
{
	libusb_device **devs, *dev;
	int r;
	atexit(exit_function);
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigterm_handler);
	libusb_init(&Stlink.libusb_ctx);
	char *serial = NULL;
	int c;
	while((c = getopt(argc, argv, "s:")) != -1) {
		switch(c) {
		case 's':
			serial = optarg;
			break;
		}
	}
	r = libusb_init(NULL);
	if (r < 0)
		DEBUG("Failed: %s", libusb_strerror(r));
	ssize_t cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0) {
		libusb_exit(NULL);
		DEBUG("Failed: %s", libusb_strerror(r));
		goto error;
	}
	int i = 0;
	bool multiple_devices = false;
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "libusb_get_device_descriptor failed %s",
					libusb_strerror(r));
			goto error;
		}
		if ((desc.idVendor == VENDOR_ID_STLINK) &&
			((desc.idProduct & PRODUCT_ID_STLINK_MASK) ==
			 PRODUCT_ID_STLINK_GROUP)) {
			if (Stlink.handle) {
				libusb_close(Stlink.handle);
				multiple_devices = (serial)? false : true;
			}
			r = libusb_open(dev, &Stlink.handle);
			if (r == LIBUSB_SUCCESS) {
				if (desc.iSerialNumber) {
					r = libusb_get_string_descriptor_ascii
						(Stlink.handle,desc.iSerialNumber, Stlink.serial,
						 sizeof(Stlink.serial));
				} else {
					DEBUG("No serial number\n");
				}
				if (serial && (!strcmp((char*)Stlink.serial, serial)) &&
					(desc.idProduct == PRODUCT_ID_STLINKV1))
					DEBUG("Found ");
				if (((!serial) || (!strcmp((char*)Stlink.serial, serial))) &&
					desc.idProduct == PRODUCT_ID_STLINKV2) {
					DEBUG("STLINKV2	 serial %s\n", Stlink.serial);
					Stlink.ver_api = 20;
				} else if (((!serial) ||
						  (!strcmp((char*)Stlink.serial, serial))) &&
						 desc.idProduct == PRODUCT_ID_STLINKV21) {
					Stlink.ver_api = 21;
					DEBUG("STLINKV21 serial %s\n", Stlink.serial);
				} else if (((!serial) ||
							(!strcmp((char*)Stlink.serial, serial))) &&
						   desc.idProduct == PRODUCT_ID_STLINKV3) {
					DEBUG("STLINKV3	serial %s\n", Stlink.serial);
					Stlink.ver_api = 21;
				} else if (((!serial) ||
							(!strcmp((char*)Stlink.serial, serial))) &&
						   desc.idProduct == PRODUCT_ID_STLINKV1) {
					DEBUG("STLINKV1 serial %s not supported\n", Stlink.serial);
				}
				if (serial && (!strcmp((char*)Stlink.serial, serial)))
					break;
			} else {
				DEBUG("Open failed %s\n", libusb_strerror(r));
			}
		}
	}
	if (multiple_devices) {
		DEBUG("Multiple Stlinks. Please sepecify serial number\n");
		goto error_1;
	}
	if (!Stlink.handle) {
		DEBUG("No Stlink device found!\n");
		goto error;
	}
	int config;
	r = libusb_get_configuration(Stlink.handle, &config);
	if (r) {
		DEBUG("libusb_get_configuration failed %d: %s", r, libusb_strerror(r));
		goto error_1;
	}
	DEBUG("Config %d\n", config);
	if (config != 1) {
		r = libusb_set_configuration(Stlink.handle, 0);
		if (r) {
			DEBUG("libusb_set_configuration failed %d: %s",
				  r, libusb_strerror(r));
			goto error_1;
		}
	}
	r = libusb_claim_interface(Stlink.handle, 0);
	if (r)
	{
		DEBUG("libusb_claim_interface failed %s", libusb_strerror(r));
		goto error_1;
	}
	Stlink.req_trans = libusb_alloc_transfer(0);
	Stlink.rep_trans = libusb_alloc_transfer(0);
 	platform_version();
	if (Stlink.ver_api < 30 && Stlink.ver_jtag < 32) {
		DEBUG("Please update Firmware\n");
		goto error_1;
	}
	stlink_leave_state();
 	putchar('\n');
	assert(gdb_if_init() == 0);
	return;
  error_1:
	libusb_close(Stlink.handle);
  error:
	libusb_free_device_list(devs, 1);
	exit(-1);
}

void platform_srst_set_val(bool assert)
{
	uint8_t cmd[3] = {STLINK_DEBUG_COMMAND,
					  STLINK_DEBUG_APIV2_DRIVE_NRST,
					  (assert)? STLINK_DEBUG_APIV2_DRIVE_NRST_LOW
					  : STLINK_DEBUG_APIV2_DRIVE_NRST_HIGH};
	uint8_t data[2];
	send_recv(cmd, 3, data, 2);
	stlink_usb_error_check(data);
}

bool platform_srst_get_val(void) { return false; }

void platform_buffer_flush(void)
{
}

int platform_buffer_write(const uint8_t *data, int size)
{
	(void) data;
	(void) size;
	return size;
}

int platform_buffer_read(uint8_t *data, int size)
{
	(void) data;
	return size;
}

#if defined(_WIN32) && !defined(__MINGW32__)
#warning "This vasprintf() is dubious!"
int vasprintf(char **strp, const char *fmt, va_list ap)
{
	int size = 128, ret = 0;

	*strp = malloc(size);
	while(*strp && ((ret = vsnprintf(*strp, size, fmt, ap)) == size))
		*strp = realloc(*strp, size <<= 1);

	return ret;
}
#endif

void platform_delay(uint32_t ms)
{
	usleep(ms * 1000);
}

uint32_t platform_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
