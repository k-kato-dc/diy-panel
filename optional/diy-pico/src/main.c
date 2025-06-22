typedef struct _SOFTWARE_INFO {
	char		name[76];
	char		ver[3];
} SOFTWARE_INFO;

  const SOFTWARE_INFO SWINFO= { "diy-panel pico", {1, 0, 3} };

// ---- diy-panel for picolcd2 ----

#include <stdio.h>
#include <string.h>		// memcpy

#include <pico/stdlib.h>
#include "picoenv.c"

#include "common.c"
#include "bitmap.c"
#include "vram.c"


void dumpmem(uint8_t *buf, DWORD size)
{
	uint32_t	i;
	uint8_t		*p;

	p = buf;
	for (i=0; i < size; i++) {
		if (i % 16 == 0) {
			if (i != 0) {
				printf("\n");
			}
			printf("%08lX:", p);
		}
		printf(" %02X", *p);
		p++;
	}
	printf("\n");

	return;
}

void LogoSet(VRAMCLIP *vc)
{
	SaveVramClip(vc, VramPlane(1));
	return;
}

void LogoDisp(VRAMCLIP *vc)
{
	// logo from vram1
	LoadVramClip(vc, VramPlane(1));
	//RotateBmp2(1, 1);
	LCDC_Redraw(VramBuf+0x42);
	return;
}

typedef struct _PERF_COUNTER {
	uint32_t	cnt;
	uint32_t	sum;
	float		ave;
} PERF_COUNTER;

void InitCnt(PERF_COUNTER *pc)
{
	pc->cnt	= 0;
	pc->sum = 0;
	pc->ave	= 0;

	return;
}

void UpdateCnt(PERF_COUNTER *pc, uint32_t len)
{
	pc->sum += len;
	pc->cnt++;

	return;
}

void PrintCnt(PERF_COUNTER *pc)
{
	pc->ave = (float)(pc->sum);
	pc->ave /= (float)(pc->cnt);

	printf("Received %ld bytes ( %1.2f bytes * %ld times )\n", pc->sum, pc->ave, pc->cnt);

	return;
}

//#define REBOOT_AFTER_IDLE	300000		// 5 min
#define REBOOT_AFTER_IDLE	60000		// 1 min
#define CDC_BURST_UNIT		4096		// 4096 byte
#define CDC_BURST_TIMEOUT	2000		// 2 sec

int main()
{
	VRAMCLIP	vc;
	uint8_t		*p, buf[CDC_BURST_UNIT];
	uint32_t	dwReadUnit, dwIdle;
	int 		c, len, ret;

	 PERF_COUNTER	pc1;

	init_picoenv();
	LogoDisp(&vc);
	ClearVramClip(&vc);

	dwIdle = 0;
	dwReadUnit = 1;		// command mode
    while (true) {

		len = stdio_usb_in_chars(buf, dwReadUnit);
		if (len > 0) {
			if (dwIdle > 25) {
				printf("idle %d ms -> receive char 0x%02X.\n", dwIdle, buf[0]);
			}
			dwIdle = 0;

			if (isReceiveBmp(&vc)) {
				AppendVramClip(&vc, buf, len);
				UpdateCnt(&pc1, len);
				continue;
			}

			switch (buf[0]) {
			case 0x00:	// NUL(^@ ^2) Reserved
				break;
			case 0x01:	// SOH(^A)
			case 0x02:	// STX(^B)
				//lcdc_test_16color();
				printf("%s ver%d.%d.%d\n", SWINFO.name, SWINFO.ver[0], SWINFO.ver[1], SWINFO.ver[2]);
				break;
			case 0x03:	// ETX(^C) Reserved
				break;
			case 0x04:	// EOT(^D)
				SoftwareReset(0);
				break;
			case 0x05:	// ENQ(^E)
				break;
			case 0x06:	// ACK(^F)
				break;
			case 0x07:	// BEL(^G)
				printf("---- VramBuf\n");
				dumpmem(VramBuf, 128);
				printf("---- Vram[0]\n");
				dumpmem(FlashAddr(VramPlane(0)), 128);
				printf("---- Vram[1]\n");
				dumpmem(FlashAddr(VramPlane(1)), 128);
				break;
			case 0x08:	// BS(^H)
				break;
			case 0x09:	// HT(^I)
				SaveVramClip(&vc, VramPlane(0));
				break;
			case 0x0A:	// LF(^J) Reserved
				break;
			case 0x0B:	// VT(^K)
				LoadRotateBmp(0, 0);
				break;
			case 0x0C:	// FF(^L)
				LoadRotateBmp(1, 0);
				break;
			case 0x0D:	// CR(^M) Reserved
				break;
			case 0x0E:	// SO(^N)
				break;
			case 0x0F:	// SI(^O)
				LoadVramClip(&vc, VramPlane(0));
				break;
			case 0x10:	// DLE(^P)
				LCDC_Redraw(VramBuf+0x42);
				break;
			case 0x11:	// DC1(^Q)
				break;
			case 0x12:	// DC2(^R)
				LogoDisp(&vc);
				break;
			case 0x13:	// DC3(^S)
				break;
			case 0x14:	// DC4(^T)
				LogoSet(&vc);
				break;
			case 0x15:	// NAK(^U)
				break;
			case 0x16:	// SYN(^V) Reserved
				break;
			case 0x18:	// CAN(^X)
				break;
			case 0x1A:	// SUB(^Z) Reserved for entering BOOTSEL
				SoftwareReset(1);
				break;

			case 0x3C:	// <(SHIFT-,)
				printf("Backlight level-%d\n", SetBacklightLevel(0xFF));	// level down
				break;
			case 0x3E:	// >(SHIFT-.)
				printf("Backlight level-%d\n", SetBacklightLevel(0xFE));	// level up
				break;

			case 0x42:	// B
				ClearVramClip(&vc);
				InitCnt(&pc1);

				AppendVramClip(&vc, buf, len);	// start
				UpdateCnt(&pc1, len);

				//printf("Start binary transfer mode. start len = %d\n", len);
				dwReadUnit = CDC_BURST_UNIT;	// transfer mode
				break;

			case 0xB0:	// Backlight off
			case 0xB1:	// Backlight level 1 dark
			case 0xB2:	// Backlight level 2
			case 0xB3:	// Backlight level 3
			case 0xB4:	// Backlight level 4
			case 0xB5:	// Backlight level 5
			case 0xB6:	// Backlight level 6
			case 0xB7:	// Backlight level 7
			case 0xB8:	// Backlight level 8
			case 0xB9:	// Backlight level 9
			case 0xBA:	// Backlight level 10
			case 0xBB:	// Backlight level 11
			case 0xBC:	// Backlight level 12
			case 0xBD:	// Backlight level 13
			case 0xBE:	// Backlight level 14
			case 0xBF:	// Backlight level 15 bright
				printf("Backlight level-%d\n", SetBacklightLevel( buf[0] - 0xB0 ));	// level down
				break;

			default:
				break;
			}

		} else {

			dwIdle++;

			// reboot and logo disp if missing heartbeat
			if (dwIdle > REBOOT_AFTER_IDLE) {
				break;
			}

			if (isReceiveBmp(&vc)) {
				if (dwIdle > CDC_BURST_TIMEOUT) {
					// timeout

				//} else if (dwIdle % 1000 == 999) {
				} else {
					switch (dwIdle) {
					case  (CDC_BURST_TIMEOUT >> 7):		// 15ms
					//case  (CDC_BURST_TIMEOUT >> 6):		// 31ms
					case  (CDC_BURST_TIMEOUT >> 5):		// 62ms
					//case  (CDC_BURST_TIMEOUT >> 4):		// 125ms
					case  (CDC_BURST_TIMEOUT >> 3):		// 250ms
					//case  (CDC_BURST_TIMEOUT >> 2):		// 0.5s
					case  (CDC_BURST_TIMEOUT >> 1):		// 1s
						ret = isCompleteBmp(&vc);
						if (ret == -1) {		// reading invalid data.
							printf("x");
						} else if (ret == 0) {	// reading valid data.
							printf("o");
						} else {				// complete
							PrintCnt(&pc1);
							if (ret == 2){		// not vram compatible bitmap
								SaveVramClip(&vc, VramPlane(0));
								LoadRotateBmp(1, 0);
							}
							LCDC_Redraw(VramBuf+0x42);

							dwReadUnit = 1;		// command mode
						}
						break;

					case CDC_BURST_TIMEOUT:				// 2s
						printf("Burst recieve timed out.(Idle=%ld ms, Read=%ld bytes)\n", dwIdle, vc.len);
						Stop_ReceiveBmp(&vc);
						dwReadUnit = 1;			// command mode
						break;

					default:
						break;
					}
				}
			}

			sleep_ms(1);	// unit of dwIdle is 1 msec.
		}
    }

	SoftwareReset(0);
	return;
}
