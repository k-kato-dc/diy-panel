// ---- on-board led ---

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Initialize the GPIO for the LED
void pico_led_init(void) {
#ifdef PICO_DEFAULT_LED_PIN
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
}

// Turn the LED on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#endif
}

// ---- flash memory ---

#include <hardware/flash.h>
#include <hardware/sync.h>


#define FLASH_PLANE_SIZE	0x28000		// W320 x H256 x 2byte = 160kb(163840)
#define FlashAddr(plane)	(XIP_BASE + FlashOfs[plane])
#define FLASH_PLANE_MIN		3			// 0-3 reserved
#define FLASH_PLANE_MAX		12			// 0-3 reserved

const uint32_t	FlashOfs[14] = {
	0x000000,	// 0: reserved 64K
	0x010000,	// 1: plane 1, reserved 160K
	0x038000,	// 2: plane 2, reserved 160K
	0x060000,	// 3: plane 3
	0x088000,	// 4: plane 4
	0x0B0000,	// 5: plane 5
	0x0D8000,	// 6: plane 6
	0x100000,	// 7: plane 7
	0x128000,	// 8: plane 8
	0x150000,	// 9: plane 9
	0x178000,	// 10: plane 10
	0x1A0000,	// 11: vram 0
	0x1C8000,	// 12: vram 1
	0x1F0000	// 13: reserved 64K
};

uint8_t *FlashRead(uint8_t plane, uint8_t *dst)
{
	uint8_t		*src;

	if ((plane < FLASH_PLANE_MIN) || (plane > FLASH_PLANE_MAX)) {
		return(NULL);	// illegal plane
	}

	src = FlashAddr(plane);
	memcpy(dst, src, FLASH_PLANE_SIZE);
	printf("Loaded from Flash Plane %d (%08lX).\n", plane, src);

	return( src );
}

uint8_t *FlashWrite(uint8_t plane, uint8_t *src)
{
	uint32_t	Intrpt;
	uint32_t	Ofs;

	if ((plane < FLASH_PLANE_MIN) || (plane > FLASH_PLANE_MAX)) {
		return(NULL);	// illegal plane
	}

	Ofs  = FlashOfs[plane];
	printf("Flash Plane %d (%08lX)", plane, Ofs);
	Intrpt = save_and_disable_interrupts();

	printf(", erasing...");
	flash_range_erase(Ofs, FLASH_PLANE_SIZE);			// 4096 byte aline

	printf("writing...");
	flash_range_program(Ofs, src, FLASH_PLANE_SIZE);	// 256 byte aline

	restore_interrupts(Intrpt);
	printf(" done.\n");

	return( FlashAddr(plane) );
}

// ---- reset ---

void SoftwareReset(uint8_t isBOOTSEL)
{
	if (isBOOTSEL) {
		printf("enter BOOTSEL mode...\n");
		sleep_ms(1000);

		rom_reset_usb_boot_extra(-1, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK, false); 

		// As for entering BOOTSEL mode, open com port with 1200 baud rate from host.

	} else {
		printf("rebooting...\n");
		sleep_ms(1000);

		// NOTE: USB CDC does not available after following reset procedure.
		//*((volatile uint32_t*)(PPB_BASE + 0xED0C)) = 0x5FA0004;

		// reset for entering current uf2
		watchdog_enable(0,0);
		while(1);
	}

	return;
}

// ---- initialize ---

static uint32_t		LCDC_W, LCDC_H;

int init_picoenv() {
	stdio_init_all();
    pico_led_init();
	lcdc_init(&LCDC_W, &LCDC_H);
}

//[EOF]
