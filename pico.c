
//---- Pico USB Serial

#define PICOUSB_BAUDRATE	9600		// no effect for USB CDC(1.8Mbps)
//#define PICOUSB_BAUDRATE	921600
#define PICOUSB_DATASIZE	153666		// 66 + 320*240*2

  static BYTE	PicoPortNum = NUM_CDCPORT;
  static BYTE	PicoBacklightLevel = 0;
  static BYTE	PicoUsbName[256];
  static BYTE	PicoUSBBuf[PICOUSB_DATASIZE + 2];	// hid_id[1] + data + \0


void SetPicoPort(BYTE portnum)
{
	memset(PicoUsbName, 0x0, sizeof(PicoUsbName));
	sprintf(PicoUsbName, "%s%1d", STR_CDCDEV, portnum);

	return;
}

#define CDCRX_TIMEOUT_MS	1
//#define CDCTX_TIMEOUT_MS	100	// failed to send ^D	
#define CDCTX_TIMEOUT_MS	200	
//#define CDCTX_TIMEOUT_MS	500
//#define CDCTX_TIMEOUT_MS	5000

#ifndef WAIT_CHAR
	#define WAIT_CHAR	0x95
#endif

#ifdef WINDOWS

#define CdcClose(h)			{ CloseHandle(h); }
#define CdcFlushRx(h)			{ PurgeComm((h), PURGE_RXABORT | PURGE_RXCLEAR ); }
#define CdcFlushTx(h)			{ PurgeComm((h), PURGE_TXABORT | PURGE_TXCLEAR ); }
#define CdcRead(h, c, siz, len)		{ ReadFile((h), (c), (siz), (len), 0); }
#define CdcWrite(h, c, siz, len)	{ WriteFile((h), (c), (siz), (len), 0); }
#define CDC_CLOSED		INVALID_HANDLE_VALUE

  static HANDLE			hPicoUSB = CDC_CLOSED;

/**
void PrintDCB(DCB *dcb)
{
	printf("BaudRate = %04X\n", dcb->BaudRate);
	printf("ByteSize = %04X\n", dcb->ByteSize);
	printf("DCBlength = %04X\n", dcb->DCBlength);
	printf("EofChar = %04X\n", dcb->EofChar);
	printf("ErrorChar = %04X\n", dcb->ErrorChar);
	printf("EvtChar = %04X\n", dcb->EvtChar);
	printf("Parity = %04X\n", dcb->Parity);
	printf("StopBits = %04X\n", dcb->StopBits);
	printf("XoffChar = %04X\n", dcb->XoffChar);
	printf("XoffLim = %04X\n", dcb->XoffLim);
	printf("XonChar = %04X\n", dcb->XonChar);
	printf("XonLim = %04X\n", dcb->XonLim);
	printf("fAbortOnError = %04X\n", dcb->fAbortOnError);
	printf("fBinary = %04X\n", dcb->fBinary);
	printf("fDsrSensitivity = %04X\n", dcb->fDsrSensitivity);
	printf("fDtrControl = %04X\n", dcb->fDtrControl);
	printf("fDummy2 = %04X\n", dcb->fDummy2);
	printf("fErrorChar = %04X\n", dcb->fErrorChar);
	printf("fInX = %04X\n", dcb->fInX);
	printf("fNull = %04X\n", dcb->fNull);
	printf("fOutX = %04X\n", dcb->fOutX);
	printf("fOutxCtsFlow = %04X\n", dcb->fOutxCtsFlow);
	printf("fOutxDsrFlow = %04X\n", dcb->fOutxDsrFlow);
	printf("fParity = %04X\n", dcb->fParity);
	printf("fRtsControl = %04X\n", dcb->fRtsControl);
	printf("fTXContinueOnXoff = %04X\n", dcb->fTXContinueOnXoff);
	printf("wReserved = %04X\n", dcb->wReserved);
	printf("wReserved1 = %04X\n", dcb->wReserved1);

	return;
}
**/

// ---- pico USB CDC

int PicoOpenCom(void)
{
	wchar_t			wcPicoUsbName[512];
	COMMTIMEOUTS	timeout;
	DCB				dcb;
	BOOL			fSuccess;


	// open
	wsprintf(wcPicoUsbName, L"%S\0", PicoUsbName);
	hPicoUSB = CreateFile(wcPicoUsbName, GENERIC_READ|GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
	if (fDEBUG) {
	    printf("open %s, hPicoUSB = %d\n", PicoUsbName, hPicoUSB);
	}
	if (hPicoUSB == CDC_CLOSED) {
		return(1);
	}

	// clear Tx/Rx buffer
	PurgeComm(hPicoUSB, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	// set dcb
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	fSuccess = GetCommState(hPicoUSB, &dcb);
	//PrintDCB(&dcb);

	dcb.BaudRate		= PICOUSB_BAUDRATE;
	dcb.ByteSize		= 8;
	dcb.Parity			= NOPARITY;
	dcb.StopBits		= ONESTOPBIT;
	// disable RTS/CTS
	//dcb.fRtsControl		= RTS_CONTROL_DISABLE;
	dcb.fRtsControl		= RTS_CONTROL_ENABLE;
	dcb.fOutxCtsFlow	= FALSE;
	// disable DTR/DSR
	//dcb.fDtrControl		= DTR_CONTROL_DISABLE;
	dcb.fDtrControl		= DTR_CONTROL_ENABLE;
	dcb.fOutxDsrFlow	= FALSE;
	// disable XON/XOFF
	dcb.fInX			= FALSE;
	dcb.fOutX			= FALSE;

	dcb.EofChar			= 0xC1;
	dcb.ErrorChar		= 0x20;
	dcb.EvtChar			= 0x74;
	dcb.XonChar			= 0x03;


	fSuccess = SetCommState(hPicoUSB, &dcb);
	if (!fSuccess) {
		/**/printf("SetComState failed.(%04X)\n", GetLastError());
		return(3);
	}

	// set timeout
	timeout.ReadIntervalTimeout			= 0;
	timeout.ReadTotalTimeoutMultiplier	= 0;
	timeout.ReadTotalTimeoutConstant	= 10;
	timeout.WriteTotalTimeoutMultiplier	= 0;
	timeout.WriteTotalTimeoutConstant	= CDCTX_TIMEOUT_MS;		// WriteFile() aborts after 5 seconds even if the send buffer is not empty.

	fSuccess = SetCommTimeouts(hPicoUSB, &timeout);
	if (!fSuccess) {
		/**/printf("SetComTimeout failed.(%04X)\n", GetLastError());
		return(2);
	}

	return(0);
}

int isCdcEnable(int isWrite)
{
	return(1);	// always ok
}

void PicoEnterBOOTSEL(void)
{
	DCB		dcb;

	GetCommState(hPicoUSB, &dcb);
	dcb.BaudRate		= CBR_1200;	// magic baud rate for Pico BOOTSEL reboot

	if (SetCommState(hPicoUSB, &dcb) == 0) {
		/**/printf("SetComState failed.(%04X)\n", GetLastError());
	}
	return;
}

#else

#include <termios.h>

#define CdcClose(h)			{ close(h); }
#define CdcFlushRx(h)			{ ioctl((h), TCFLSH, 0); }
#define CdcFlushTx(h)			{ ioctl((h), TCFLSH, 1); }
#define CdcRead(h, c, siz, len)		{ *(len) = read((h), (c),(siz)); }
#define CdcWrite(h, c, siz, len)	{ do { *(len) = write((h), (c), (siz)); } while (*(len) == -1); }
#define CDC_CLOSED		-1

  static int		hPicoUSB = CDC_CLOSED;

int PicoOpenCom(void)
{
	struct termios	tio;

	hPicoUSB = open(PicoUsbName, O_RDWR);
	if (fDEBUG) {
	    printf("open %s, hPicoUSB = %d\n", PicoUsbName, hPicoUSB);
	}
	if (hPicoUSB < 0) {
		return(1);
	}

	tcgetattr(hPicoUSB, &tio);
	tio.c_cflag += CREAD;	// enable read
	tio.c_cflag += CLOCAL;	// no modem control
	cfmakeraw(&tio);	// RAW mode
	//tio.c_cflag += CS8;	// 8bit
	//tio.c_cflag += 0;	// 1 stop bit
	//tio.c_cflag += 0;	// parity None
	//tio.c_lflag &= ~ICANON;	// not canonical
	//tio.c_lflag &= ~ECHO;	// no local echo
	//tio.c_lflag &= ~ISIG;	// no signal proc
	tio.c_cc[VMIN] = 0;	// no wait Rx char
	tio.c_cc[VTIME] = 1;	// Rx timeout 1ms
	cfsetispeed( &tio, PICOUSB_BAUDRATE );	// dummy for cdc
	cfsetospeed( &tio, PICOUSB_BAUDRATE );	// dummy for cdc
	tcsetattr(hPicoUSB, TCSANOW, &tio);	// set
	ioctl(hPicoUSB, TCSETS, &tio);		// set

	//CdcFlushRx(hPicoUSB);	// purge Rx buffer
	CdcFlushTx(hPicoUSB);	// purge Tx buffer

	// Enter Non-blocking mode
	fcntl(hPicoUSB, F_SETFL, O_NONBLOCK);

	return(0);
}

void PicoEnterBOOTSEL()
{
	struct termios  tio;

	tcgetattr(hPicoUSB, &tio);
	cfsetispeed( &tio, B1200 );	// magic baud rate for Pico BOOTSEL reboot
	cfsetospeed( &tio, B1200 );

	tcsetattr(hPicoUSB, TCSANOW, &tio);     // set
	ioctl(hPicoUSB, TCSETS, &tio);          // set
    return;
}

int isCdcExist(char *path)
{
	struct stat	ss;

	if (_stat(PicoUsbName, &ss) != 0) {
		return(0);
	}
	return(1);
}

void PicoCloseCom(void);

int isCdcEnable(int isWrite)
{
	fd_set		fds;
	struct timeval	fdtv;

	if (hPicoUSB != CDC_CLOSED) {
		if (isCdcExist(PicoUsbName) == 0) {
			PicoCloseCom();
			return(0);
		}

		FD_ZERO(&fds);
		FD_SET(hPicoUSB, &fds);

		if (isWrite) {
			fdtv.tv_sec = (CDCTX_TIMEOUT_MS / 1000);
			fdtv.tv_usec = (CDCTX_TIMEOUT_MS % 1000) * 1000;
			return( select(hPicoUSB+1, NULL, &fds, NULL, &fdtv) );	// writable
		} else {
			fdtv.tv_sec = (CDCRX_TIMEOUT_MS / 1000);
			fdtv.tv_usec = (CDCRX_TIMEOUT_MS % 1000) * 1000;
			return( select(hPicoUSB+1, &fds, NULL, NULL, &fdtv) );	// readable
		}
	}

	return(0);
}

#endif

int isPicoOpened(void)
{
	if (hPicoUSB == CDC_CLOSED) {
		return(1);
	}

	return(0);
}

void PicoCloseCom(void)
{
	if (hPicoUSB != CDC_CLOSED) {
		CdcFlushTx(hPicoUSB);	// purge Tx buffer
		Sleep(1);
		CdcClose(hPicoUSB);

		hPicoUSB = CDC_CLOSED;
		if (fDEBUG) {
			printf("Close hPicoUSB = %d\n", hPicoUSB);
		}
	}

	return;
}

BYTE PicoReadChar()
{
	BYTE	c;
	DWORD dwRead;

	if (isCdcEnable(0) == 0) {
		return(WAIT_CHAR);	// simply wait if timeout
	}

	dwRead = 0;
	CdcRead(hPicoUSB, &c, 1, &dwRead);
	if (dwRead != 1) {
		c = WAIT_CHAR;		// no data to read
	}

	return(c);
}

DWORD PicoSendChar(BYTE c)
{
	DWORD dwSent;

	dwSent = 0;
	if (c != WAIT_CHAR) {
		if (isCdcEnable(1) == 0) {
			return(0);	// error return
		}

		CdcWrite(hPicoUSB, &c, 1, &dwSent);
	}

	return(dwSent);
}

DWORD PicoBurstSend()
{
	BYTE	*p;
	DWORD	dwSent, dwLeft, dwUnit;
	int	Len;

	//dwUnit = 256;
	dwUnit = 1024;				// about 1.7Mbps for pico usb cdc
	//dwUnit = 4096;

	dwSent = 0;
	dwLeft = PICOUSB_DATASIZE;
	p = PicoUSBBuf + 1;			// skip hid id

	while (dwLeft > 0) {
		if (dwUnit > dwLeft) {
			dwUnit = dwLeft;	// last bytes
		}

		Len = 0;
		if (isCdcEnable(1) == 0) {
			Len = 0;		// tx timeout
		} else {
			CdcWrite(hPicoUSB, p, dwUnit, &Len);
		}

		if (Len > 0) {
			// sending
			dwSent += Len;
			dwLeft -= Len;
			p += Len;
		//} else if (Len == -1) {
		//	; 	// nop for EAGAIN
		} else {
			// timeout with no data sent.
			if (fDEBUG) {
				printf("Timeout occurred in burst transmission; purge %d bytes.\n", dwLeft);
			}
			PicoCloseCom();
			break;
		}
	}

	return(dwSent);
}

// ---- pico screen ----

#define PICOVRAM_SIZE	0x28000		// 64kb * 2.5 = 320 * 256 * 2

  static BYTE		VramBuf[2][PICOVRAM_SIZE];

int LoadVramBmp(BYTE *fname)
{
	int			fd;
	DWORD		dwRead;

	fd = open(fname, O_RDONLY);
	if (fd > 0) {
		dwRead = read(fd, VramBuf[0], PICOVRAM_SIZE);
		close(fd);

		//printf(" ---- Loaded %d bytes: %s\n", dwRead, fname);
		return(0);
	}

	printf("File open error: %s\n", fname);
	return(1);
}

int SaveVramBmp(BYTE *fname)
{
	int			fd;
	DWORD		dwWrite;
	BITMAPFILE	bm;

	fd = open(fname, O_WRONLY | O_TRUNC);
	if (fd > 0) {
		AttachBitmapStruct(&bm, VramBuf[0]);
		dwWrite = write(fd, VramBuf[0], bm.f->bfSize);
		close(fd);

		/**/printf(" ---- Saved %d / %d bytes: %s\n", dwWrite, bm.f->bfSize, fname);
		return(0);
	}

	printf("File open error: %s\n", fname);
	return(1);
}

int GetVramSrc_fromFile(BYTE *fname)
{
	BITMAPFILE	bm;

	if (LoadVramBmp(fname) != 0) {
		return(2);
	}

	AttachBitmapStruct(&bm, VramBuf[0]);
	if ((bm.i->biWidth != 320) || (bm.i->biHeight != 170) || (bm.i->biBitCount != 16)) {
		printf("Error: Only support 320x170 16bit bitmap.\n");
		PrintBitmapHeader(&bm);
		return(1);
	}

	// size change; top and bottom 35 lines are blank.
	Line170to240Vram(bm.b);
	bm.i->biHeight = 240;
	bm.i->biSizeImage = 320 * 240 * 2;
	bm.f->bfSize = bm.i->biSizeImage + bm.f->bfOffBits;

	return(0);
}

int GetVramSrc_fromVram(BYTE *pix)
{
	BITMAPFILE	bm;
	DWORD		src_h, pixbyte;
	BYTE		*src, *dst;

	// bitmap file header
	bm.f = (BITMAPFILEHEADER *)(VramBuf[0]);

	bm.f->bfType          = 0x4D42;
	//bm.f->bfSize          = bm.f->bfOffBits + bm.i->biSizeImage;	// set later
	bm.f->bfReserved1     = 0;
	bm.f->bfReserved2     = 0;
	bm.f->bfOffBits       = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(BITFIELD565);

	// bitmap info header
	bm.i = (BITMAPINFOHEADER *)((BYTE *)(bm.f) + sizeof(BITMAPFILEHEADER));

	bm.i->biSize          = sizeof(BITMAPINFOHEADER);
	bm.i->biWidth         = 320;
	bm.i->biHeight        = 240;
	bm.i->biPlanes        = 1;
	bm.i->biBitCount      = 16;
	bm.i->biCompression   = 3;			// 0:BIRGB(555), 3:BI_BITFIELD(565)
	bm.i->biXPelsPerMeter = 3780;		// 96dpi
	bm.i->biYPelsPerMeter = 3780;		// 96dpi
	bm.i->biClrUsed       = 0;
	bm.i->biClrImportant  = 0;

	// relative value
	pixbyte = bm.i->biBitCount / 8;
	bm.i->biSizeImage     = bm.i->biWidth * bm.i->biHeight * pixbyte;
	bm.f->bfSize          = bm.f->bfOffBits + bm.i->biSizeImage;

	// color data
	bm.c = (RGBQUAD *)((BYTE *)(bm.i) + bm.i->biSize);

	memcpy((BYTE *)(bm.c), BITFIELD565, sizeof(BITFIELD565));

	// bitmap bit
	bm.b = (BYTE *)(bm.c) + sizeof(BITFIELD565);

	if (pix == NULL) {
		src = (BYTE *)(SnapFile[SNAPFILE_DUALPANE].vram[GetPaneOrientation()]);	// landscape or portrait
	} else {
		src = pix;
	}
	dst = (BYTE *)(bm.b);
	memset(dst, 0, bm.i->biSizeImage);										// black

	src_h = 170;
	dst += ((bm.i->biHeight - src_h) / 2) * bm.i->biWidth * pixbyte;		// center alignment
	memcpy(dst, src, (bm.i->biWidth * src_h * pixbyte));					// panel pix size, keep top to bottom, big endian

	return(0);
}

void ConvertPicoPix(BYTE *lcd_bmp, BYTE *pix_bmp, BYTE isBE)
{
	C16PIXRECT		src, dst;

	//printf(" ---- ConvertPicoPix()\n");

	SetPixInfo(&src, 0, 0, 320, 240, (WORD *)(pix_bmp + 0x42));
	SetPixInfo(&dst, 0, 0, 320, 240, (WORD *)(lcd_bmp + 0x42));

	// rotate 270 from s1 landscape to pico
	Rotate270Pix(&dst, &src, isBE);

	if (GetPaneOrientation() == 1) {		// 0:landscape 1:portrait
		// rotate180 from s1 portrait to pico 
		Rotate180Pix(&dst);
	}

	memcpy(lcd_bmp, pix_bmp, 0x42);	// copy bitmap header

	return;
}

int PicoSetData(BYTE *src, DWORD size)
{
	memset(PicoUSBBuf, 0, PICOUSB_DATASIZE + 1);
	memcpy((PicoUSBBuf + 1), src, size);    // skip hid id (1byte)

	return(0);
}

DWORD PicoSendVram(BYTE *fname, BYTE *pix)
{
	BITMAPFILE	bm;
	DWORD		temp, dwSent;
	int			ret, isBE;

	dwSent = 0;

	if (isPicoOpened() == 0) {		// opened

		memset(VramBuf[0], 0, PICOVRAM_SIZE);
		if (fname == NULL) {
			ret = GetVramSrc_fromVram(pix);	// panel pix to vram0
			isBE = 1;
		} else {
			ret = GetVramSrc_fromFile(fname);
			isBE = 0;
		}
		if (ret != 0) {
			return(0);	// failed
		}

		memset(VramBuf[1], 0, PICOVRAM_SIZE);	// bgcolor = 0:black or transparent
		ConvertPicoPix(VramBuf[1], VramBuf[0], isBE);

		// set width hight info
		AttachBitmapStruct(&bm, VramBuf[1]);
		temp = bm.i->biWidth;
		bm.i->biWidth = bm.i->biHeight;
		bm.i->biHeight = temp;

		// copy to buffer and burst Tx
		PicoSetData(VramBuf[1], bm.f->bfSize);
		dwSent = PicoBurstSend();
	}

	return(dwSent);
}

// ---- pico run mode

DWORD PicoModeChange()
{
	static BYTE prev_PicoBacklightLevel = 0;
	BYTE	portnum, backlight;

	portnum = ClipStatVal(STATFILE_PICO_CONFIG, 0, 0, 15);
	if (portnum != PicoPortNum) {
		if (isPicoOpened() == 0) {
			PicoCloseCom();
		}
		PicoPortNum = portnum;
		return(2);

	} else {

		backlight = ClipStatVal(STATFILE_PICO_CONFIG, 1, 0, 15);
		if (backlight != PicoBacklightLevel) {
			if (isPicoOpened() == 0) {
				PicoBacklightLevel = backlight;
				return ( PicoSendChar(0xB0 + PicoBacklightLevel) );		// backlight set command
			}
		}
	}

	return(0);
}

DWORD PicoLogoChange(BYTE *fname)
{
	BYTE		new_fname[256];
	BITMAPFILE	bm;
	DWORD		pixbyte, dwSent;

	memset(VramBuf[0], 0, PICOVRAM_SIZE);
	if (LoadVramBmp(fname) != 0) {
		return(0);
	}

	AttachBitmapStruct(&bm, VramBuf[0]);
	if (bm.i->biBitCount != 16) {

		// create 16bit image
		BMPtoC16(VramBuf[1], &bm, 0);	// 0:for pico

		// bitmap file header
		bm.f->bfOffBits       = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(BITFIELD565);

		// bitmap info header
		bm.i->biBitCount      = 16;
		bm.i->biCompression   = 3;			// 0:BIRGB(555), 3:BI_BITFIELD(565)
		bm.i->biClrUsed       = 0;
		bm.i->biClrImportant  = 0;

		// relative value
		pixbyte = bm.i->biBitCount / 8;
		bm.i->biSizeImage     = bm.i->biWidth * bm.i->biHeight * pixbyte;
		bm.f->bfSize          = bm.f->bfOffBits + bm.i->biSizeImage;

		// color data
		bm.c = (RGBQUAD *)((BYTE *)(bm.i) + bm.i->biSize);
		memcpy((BYTE *)(bm.c), BITFIELD565, sizeof(BITFIELD565));

		// bitmap bit
		bm.b = (BYTE *)(bm.c) + sizeof(BITFIELD565);
		memcpy((BYTE *)(bm.b), VramBuf[1], bm.i->biSizeImage);

	}

	//debug
	SaveVramBmp(VRAMFNAME[7]);	// factory1.bmp

	dwSent = 0;
	PicoSetData(VramBuf[0], bm.f->bfSize);
	dwSent = PicoBurstSend();
	Sleep(100);
	dwSent += PicoSendChar(0x14);	// 0x14(^)T Logo Set
	Sleep(100);

	// rename logo file to prevent duplicate send.
	sprintf(new_fname, "%s.sended", fname);
	remove(new_fname);
	rename(fname, new_fname);
	/**/printf(" ---- Renamed to %s\n", new_fname);

	return(dwSent);
}

// ---- pico init ----

int isPicoResponse(void)
{
	const char PicoAckStr[] = "diy-panel pico";
	DWORD	dwTimeout;
	BYTE	buf[80];
	int	i, j;

	// send SOH(0x01, ^A)
	if (PicoSendChar(0x01) != 1) {
		/**/printf(" ---- Failed to send start char(0x01) to pico.\n");
		return(2);		// not pico; error
	}

	// check response string
	for (j=0; j < 2; j++) {		// check 2 lines
		memset(buf, 0x0, sizeof(buf));
		dwTimeout = 0;
		for (i=0; i < 76; i++) {	// software info max
			buf[i] = PicoReadChar();
			if (buf[i] == WAIT_CHAR) {
				dwTimeout++;
				if (dwTimeout >= 1000) {	// abort after waiting 1 sec.
					printf("Timed out for get response\n");
					break;
				}
				Sleep(1);	// unit of dwTimeout is 1 msec.
				i--;
				continue;
			}
			dwTimeout = 0;

			if (buf[i] == 0x0A) {	// end of line
				i++;
				break;
			}
		}
		buf[i] = 0x00;	// end of string
		if (fDEBUG) {
			printf("PicoAckStr[%ld] = %s", strlen(buf), buf);
		}

		// check response string
		if (dwTimeout == 0) {
			if (memcmp(buf, PicoAckStr, strlen(PicoAckStr)) == 0) {
				//printf(" ---- valid pico response.\n");
				return(0);			// is diy-panel pico
			}
		}
	}

	//printf(" ---- Not pico response.\n");
	return(1);	// not diy-panel pico
}

int InitPico(BYTE *fname)
{
	if (isPicoOpened() != 0) {
		SetPicoPort(PicoPortNum);

		if (PicoOpenCom() != 0) {
			return(2);		// open error
		}

		if (isPicoResponse() != 0) {
			PicoCloseCom();
			return(1);		// not pico
		}

		if (fDEBUG) {
			printf(" ---- Found pico at %s.\n", PicoUsbName);
		}

		PicoLogoChange(fname);
	}

	return(0);
}

// ---- thread interface  ----

DWORD PicoAction(BYTE comnum, BYTE *pix)
{
	printf("---- PicoAction(%d, %08X)\n", comnum, pix);

	if (comnum == 0) {
		if (InitPico(VRAMFNAME[6]) != 0) {
			return(0);
		}

		return( PicoSendVram(NULL, pix) );
		//return( PicoSendVram(VRAMFNAME[0], NULL) );
	}

	return( PicoModeChange() );
}


//[EOF]
