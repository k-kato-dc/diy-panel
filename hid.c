
//---- information about S1 TFT LCD ----//

#define S1RPT_VID		0x04D9
#define S1RPT_PID		0xFD01
#define S1RPT_INTERFACE		0x01
#define S1RPT_ENDPOINT		0x02
#define S1RPT_TIMEOUT		10000

#define	S1RPT_SIZE		4104	//8+4096
#define S1RPT_HEAD_NUM		40
#define S1RPT_HEAD_SIZE		8
#define S1RPT_DATA_SIZE		4096

const BYTE	S1RptHeader[S1RPT_HEAD_NUM][S1RPT_HEAD_SIZE] = {
	{ 0x55, 0xA1, 0xF1, 0x01,  0x00, 0x00, 0x00, 0x00 },	//landscape
	{ 0x55, 0xA1, 0xF1, 0x02,  0x00, 0x00, 0x00, 0x00 },	//portrait

	{ 0x55, 0xA1, 0xF2, 0x01,  0x02, 0x03, 0x00, 0x00 },	//heart beat(h,m,s)
	{ 0x55, 0xA1, 0xF3, 0x01,  0x02, 0x03, 0x00, 0x00 },	//set time(h,m,s)

	{ 0x55, 0xA2, 0x01, 0x01,  0x02, 0x02, 0x03, 0x04 },	//update(x,y,w,h)
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
//8
	{ 0x55, 0xA3, 0xF0, 0x01,  0x00, 0x00, 0x00, 0x10 },	//redraw
	{ 0x55, 0xA3, 0xF1, 0x02,  0x00, 0x10, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x03,  0x00, 0x20, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x04,  0x00, 0x30, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x05,  0x00, 0x40, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x06,  0x00, 0x50, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x07,  0x00, 0x60, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x08,  0x00, 0x70, 0x00, 0x10 },
//16
	{ 0x55, 0xA3, 0xF1, 0x09,  0x00, 0x80, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0A,  0x00, 0x90, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0B,  0x00, 0xA0, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0C,  0x00, 0xB0, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0D,  0x00, 0xC0, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0E,  0x00, 0xD0, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x0F,  0x00, 0xE0, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x10,  0x00, 0xF0, 0x00, 0x10 },
//24
	{ 0x55, 0xA3, 0xF1, 0x11,  0x00, 0x00, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x12,  0x00, 0x10, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x13,  0x00, 0x20, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x14,  0x00, 0x30, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x15,  0x00, 0x40, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x16,  0x00, 0x50, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x17,  0x00, 0x60, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x18,  0x00, 0x70, 0x00, 0x10 },
//32
	{ 0x55, 0xA3, 0xF1, 0x19,  0x00, 0x80, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF1, 0x1A,  0x00, 0x90, 0x00, 0x10 },
	{ 0x55, 0xA3, 0xF2, 0x1B,  0x00, 0xA0, 0x00, 0x09 },

	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00 }
//40
};

//---- hid func ----//

static BYTE			S1RptBuf[S1RPT_SIZE + 2];	// 0x0 buf 0x0

void GetTime3Byte(BYTE *buf);	// thread.c

void SetOutputReport(BYTE header_num, BYTE *buf)
{
	BYTE *dst, *src;

	// clear report
	dst = S1RptBuf;
	memset(dst, 0, sizeof(S1RptBuf));
	dst++;    // skip hid id (1byte)

	// set header
	memcpy(dst, &(S1RptHeader[header_num]), S1RPT_HEAD_SIZE);
	src = NULL;
	if ((header_num == 2) || (header_num == 3)) {
		// heatbeat, set time
		GetTime3Byte(dst+3);
	} else if (header_num == 4) {
		// update TBD
		src = buf;
	} else if ((header_num >= 8) && (header_num <= 34)) {
		// redraw
		src = buf + ((header_num - 8) * S1RPT_DATA_SIZE);
	} 
	dst += S1RPT_HEAD_SIZE;

	// set data
	if (src != NULL) {
		memcpy(dst, src, S1RPT_DATA_SIZE);
	}
}


#ifdef WINDOWS

static wchar_t		S1DevicePath[512] = L"\\\\?\\HID#VID_04D9&PID_FD01&MI_01#7&16a3c4a9&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}\0";
static HANDLE		hDev;

void InitHidEnv()
{
	// hid open
	hDev = INVALID_HANDLE_VALUE;
	hDev = CreateFile(S1DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
	printf("Open hDev = %016lX\n", hDev);

	if (hDev == INVALID_HANDLE_VALUE) {
		printf("hid device open error.\n");
		return;
	}

	return;

}

void FreeHidEnv()
{
	if (hDev == NULL) {
		printf("hDev is not opend.\n");
		return;
	}

	if (hDev != INVALID_HANDLE_VALUE) {
		CloseHandle( hDev );
		hDev = INVALID_HANDLE_VALUE;

		printf("hid: close hDev.\n");
	}

	return;
}

void SendOutputReport(int num, BYTE *buf)
{
	BYTE 	*p;
	DWORD	len;
	int		sent;
	int			ret, err, i;
	BOOL		done, stop;
	OVERLAPPED	eventWrite = {0};

	// no device
	if (hDev == INVALID_HANDLE_VALUE) {
		return;
	}

	// data set
	SetOutputReport(num, buf);

	// send
	//p = S1RptBuf + 1;
	//len = S1RPT_SIZE;
	p = S1RptBuf;
	len = S1RPT_SIZE + 1;

	//printf("hid: output len=%d / %d\n", sent, len);

	eventWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	i=0;
	ret = WriteFile(hDev, p, len, &sent, &eventWrite);
	if (ret == 0) {
		err = GetLastError();
		if (err == ERROR_IO_PENDING) {
			done = FALSE;
			stop = FALSE;

			do {
				// yes. Wait till pending state has gone
				ret = WaitForSingleObject(eventWrite.hEvent, 25);

				if (ret == WAIT_OBJECT_0) {
					GetOverlappedResult(hDev, &eventWrite, &sent, FALSE);
					printf("sent = %d\n", sent);
					done = TRUE;
				} else if (ret == WAIT_TIMEOUT) {
					// Need to try again.
				} else {
					stop = TRUE;
					done = TRUE;
				}
			} while (!done && !stop);
		}
	}

	return;
}

#else

#include <libusb-1.0/libusb.h>

static libusb_device_handle	*hDev;
static int			ret_claim, ret_detach;

void InitHidEnv()
{
	// start hid action
	libusb_init(NULL);

	// hid open
	hDev = libusb_open_device_with_vid_pid(NULL, S1RPT_VID, S1RPT_PID);
	if (hDev == NULL) {
		printf("hid device open error.\n");
		return;
	}

	// get device info number
	printf("hid: open hDev=%016lX(dev=%016lX, bus=%X)", (QWORD)hDev, (QWORD)(libusb_get_device(hDev)), libusb_get_bus_number(libusb_get_device(hDev)));

	// detach kernel driver
	ret_detach = libusb_detach_kernel_driver(hDev, S1RPT_INTERFACE);
	printf(", detach kernel driver ret=%d", ret_detach);

	// claim interface
	ret_claim = libusb_claim_interface(hDev, S1RPT_INTERFACE);
	printf(", claim interface ret=%d\n", ret_claim);
}

void FreeHidEnv()
{
	int	ret;

	if (hDev == NULL) {
		printf("hDev is not opend.\n");
		return;
	}

	printf("hid:");
	// release interface, if claimed
	if (ret_claim == 0) {
		ret = libusb_release_interface(hDev, S1RPT_INTERFACE);
		printf(" release interface ret=%d", ret);
	}

	// re-attach driver, if detached.
	if (ret_detach == 0) {
		ret = libusb_attach_kernel_driver(hDev, S1RPT_INTERFACE);
		printf(", attach kernel driver ret=%d", ret);
	}
	printf("\n");

	// hid close
	libusb_close(hDev);

	// stop hid action
	libusb_exit(NULL);
}

void SendOutputReport(int num, BYTE *buf)
{
	BYTE 	*p;
	DWORD	len;
	int	sent;
	int	ret, ret_detach;

	// no device
	if (hDev == NULL) {
		return;
	}

	// claim interface
 	if (ret_claim != 0) {
		printf(" usb interface does not claimed yet.(ret = %d)\n", ret_claim);
		ret_claim = libusb_claim_interface(hDev, S1RPT_INTERFACE);
		if (ret_claim != 0) {
			printf("abort sending: cannot claim usb interface(ret = %d)\n", ret_claim);

			return;
		}
	}

	// data set
	SetOutputReport(num, buf);

	// send
	p = S1RptBuf + 1;
	len = S1RPT_SIZE;
	ret = libusb_bulk_transfer(hDev, S1RPT_ENDPOINT, p, len, &sent, S1RPT_TIMEOUT);

	//printf("hid: output ret=%d, len=%d / %d\n", ret, sent, len);

	return;
}

#endif 

// ---- thread interface  ----

void HidAction(BYTE comnum, BYTE *pix)
{
	int	i;
	//BYTE	*p;

	printf("---- HidAction(%d, %08X)\n", comnum, pix);

	if ( comnum == 8) {

		// create bitmap
		//p = GetPanelPtr();
	
		// send full screen
		for (i=0; i < 27; i++) {
			//SendOutputReport(i+8, p);	// redraw
			SendOutputReport(i+8, pix);	// redraw
		}

	} else if (comnum == 3) {

		// send heart beat.\n");
		SendOutputReport(3, NULL);		// set time

	}

	return;
}


//[EOF]

