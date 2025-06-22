//#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

// ---- Cable connection of PicoLCD2 ----
//		name			GPn
								// GND
//#define GPO_LCDC_SDA	6		// I2C1 SDA
//#define GPO_LCDC_SCL	7		// I2C1 SCL
#define GPO_LCDC_DC		8		// (SPI1 RX)
#define GPO_LCDC_CS		9		// SPI1 CSn
								// GND
#define GPO_LCDC_CLK	10		// SPI1 SCK
#define GPO_LCDC_MOSI	11		// SPI1 TX
#define GPO_LCDC_RST	12		// (SPI1 RX)
#define GPO_LCDC_BL		13		// (SPI1 CSn)
								// GND

// ---- GPIO function ----

void InitGPIO(void)
{
    // GPIO Config
    gpio_init(GPO_LCDC_DC);
    gpio_init(GPO_LCDC_CS);
    gpio_init(GPO_LCDC_RST);
    gpio_init(GPO_LCDC_BL);

	gpio_set_dir(GPO_LCDC_DC, GPIO_OUT);
	gpio_set_dir(GPO_LCDC_CS, GPIO_OUT);
	gpio_set_dir(GPO_LCDC_RST, GPIO_OUT);
	gpio_set_dir(GPO_LCDC_BL, GPIO_OUT);

    gpio_put(GPO_LCDC_CS, 1);	// cs = no
    gpio_put(GPO_LCDC_DC, 0);	// dc = lcdc command
    gpio_put(GPO_LCDC_RST, 1);	// reset = high
    //gpio_put(GPO_LCDC_BL, 1);	// backlight = on
    gpio_put(GPO_LCDC_BL, 0);	// backlight = off

	return;
}

#define LCDC_HardwareReset()		HardwareReset(GPO_LCDC_RST)

void HardwareReset(uint reset_gpio)
{
    gpio_put(reset_gpio, 1);
    sleep_ms(100);
    gpio_put(reset_gpio, 0);
    sleep_ms(100);
    gpio_put(reset_gpio, 1);
    sleep_ms(100);

	return;
}

// ---- PWM function ----

static uint bl_slice_num;

#define EnableBacklightControl()	InitPWM(GPO_LCDC_BL, &bl_slice_num)
#define BacklightControl(value)		pwm_set_chan_level(bl_slice_num, PWM_CHAN_B, (value))

void InitPWM(uint gpio, uint *slice_num)
{
    // PWM Config
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    *slice_num = pwm_gpio_to_slice_num(gpio);

    pwm_set_clkdiv(*slice_num, 50);						// clock / 50
    pwm_set_wrap(*slice_num, 100);						// range = 0:off - 100:max
    pwm_set_chan_level(*slice_num, PWM_CHAN_B, 0);		// set on = 0%; backlight off

    pwm_set_enabled(*slice_num, true);

    return;
}

// ---- SPI function ----

#define SPI_PORT spi1

void InitSPI(void)
{
    // SPI Config
    spi_init(SPI_PORT, 10000 * 1000);
    gpio_set_function(GPO_LCDC_CLK, GPIO_FUNC_SPI);
    gpio_set_function(GPO_LCDC_MOSI, GPIO_FUNC_SPI);

    return;
}

void SendSPI(const uint8_t *buf)
{
	uint32_t	len;
	uint8_t		*p;

	p = buf;
	len = *p - 1;

	p++;
	// command
	gpio_put(GPO_LCDC_DC, 0);
	gpio_put(GPO_LCDC_CS, 0);
	spi_write_blocking(SPI_PORT, p, 1);

	p++;
	// param
	gpio_put(GPO_LCDC_DC, 1);
	spi_write_blocking(SPI_PORT, p, len);
	gpio_put(GPO_LCDC_CS, 1);

	return;
}

// ---- LCD controller ST7789V ----

#define SLEEP_OUT			0
#define DISPLAY_INV			1
#define DISPLAY_ON			2
#define ADDRESS_X			3
#define ADDRESS_Y			4
#define MEMORY_WRITE		5
#define MEMORY_READ			6
#define MEMORY_CONTROL		7
#define COLOR_RGB565		8
#define PORCH_CONTROL		9
#define GATE_CONTROL		10
#define VCOMS_SETTING		11
#define LCM_CONTROL			12
#define VDV_VRH_COMMAND		13
#define VRH_SET				14
#define VDV_SET				15
#define FRAMERATE_CONTROL	16
#define POWER_CONTROL		17
#define POSITIVE_VOLTAGE	18
#define NEGATIVE_VOLTAGE	19

const uint8_t CMD_ST7789V[][16] = {
//	command sequence is,
//	{ Length, Command, Palam1, Param2, ... };

	// 0: Sleep OUT; Turn off sleep mode; default=sleep mode
	{ 0x01, 0x11 },
	// 1: Display Inversion ON;  invert color is normal
	{ 0x01, 0x21 },
	// 2: Display ON; default=display off
	{ 0x01, 0x29 },

	// 3: Column Address Set = default(0x00EF, MV=0)
	//    from 0x0000 to 0x00EF; (*, 0) - (*, 239)
	{ 0x05, 0x2A, 0x00, 0x00, 0x00, 0xEF },
	// 4: Row Address Set = default(0x013F, MV=0)
	//    from 0x0000 to 0x013F; (0, *) - (319, 0)
	{ 0x05, 0x2B, 0x00, 0x00, 0x01, 0x3F },
	// 5: Memory Write; have to send 240x320 words after send this.
	{ 0x01, 0x2C },
	// 6: Memory Read; have to get 240x320 words after send this; not WRX but RDX=0.
	{ 0x01, 0x2E },

	// 7: Memory Data Access Control = default
	//    MY=TopToBottom, MX=LeftToRight, MV=NormalMode, ML=RefreshTopToBottom, RGB=RGB, MH=RefreshLeftToRight
	{ 0x02, 0x36, 0x00 },
	// 8: Interface Pixel Format != default(18bit)
	//    262KofRGB, 16bit
	{ 0x02, 0x3A, 0x65 },

	// 9: Porch Setting = default
	//    Front/BackPorch Normal=0x0C, ,separate = disable, Idle = 0x33
	{ 0x06, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33 },
	// 10: Gate Control = default
	//     VGH=13.26V, VGL=-10.43V
	{ 0x02, 0xB7, 0x35 },
	// 11: VCOMS Setting != default(0x20)
	//     VCOMS = 0.875V
	{ 0x02, 0xBB, 0x1F },
	// 12: LCM Control = default
	//     !XMY36h, XBGR36h, !XINV21h, XMX36h, XMH, !XMV36h, !XGSe4h
	{ 0x02, 0xC0, 0x2C },
	// 13: VDV and VRH Command Enable = default?(0x01FF)
	//     VDV/VRHCommandWrite = Enable
	{ 0x03, 0xC2, 0x01, 0xFF },
	// 14: VRH Set != default(0x0B)
	//     VAP(GVDD)=4.45+(vcom+vcom offset+0.5vdv)V, VAN(GVCL)=-4.45+(vcom+vcom offset-0.5vdv)V
	{ 0x02, 0xC3, 0x12 },
	// 15: VDV Set = default
	//     VdV=0V
	{ 0x02, 0xC4, 0x20 },
	// 16: Frame Rate Control in Normal Mode = default
	//     FR in normal mode = 60Hz
	{ 0x02, 0xC6, 0x0F },
	// 17: Power Control 1 != default(0xA481)
	//     AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V
	{ 0x03, 0xD0, 0xA4, 0xA1 },
	// 18: Positive Voltage Gamma Control != default
	//     default = E0 D0 00 02  07 0B 1A 31  54 40 29 12  12 12 17
	{ 0x0F, 0xE0, 0xD0, 0x08, 0x11,  0x08, 0x0C, 0x15, 0x39,  0x33, 0x50, 0x36, 0x13,  0x14, 0x29, 0x2D },
	// 19: Negative Voltage Gamma Control
	//     default = E1 D0 00 02  07 05 05 2D  44 44 0C 18  16 1C 1D
	{ 0x0F, 0xE1, 0xD0, 0x08, 0x10,  0x08, 0x06, 0x06, 0x39,  0x44, 0x51, 0x0B, 0x16,  0x14, 0x2F, 0x31 }
};

void InitLCDC(void)
{
    // Hardware reset
	LCDC_HardwareReset();

	// ----  x-y address settings ----
	//SendSPI( CMD_ST7789V[ ADDRESS_X ] );
	//SendSPI( CMD_ST7789V[ ADDRESS_Y ] );

	// ---- memory settings ----
	SendSPI( CMD_ST7789V[ MEMORY_CONTROL ] );
	SendSPI( CMD_ST7789V[ COLOR_RGB565 ] );

	//return;		// skip detail settings

	// ---- hardware settings; waveshare example values ----
	//SendSPI( CMD_ST7789V[ PORCH_CONTROL ] );
	SendSPI( CMD_ST7789V[ VCOMS_SETTING ] );
	//SendSPI( CMD_ST7789V[ LCM_CONTROL ] );
	SendSPI( CMD_ST7789V[ VDV_VRH_COMMAND ] );
	SendSPI( CMD_ST7789V[ VRH_SET ] );
	//SendSPI( CMD_ST7789V[ VDV_SET ] );
	//SendSPI( CMD_ST7789V[ FRAMERATE_CONTROL ] );
	SendSPI( CMD_ST7789V[ POWER_CONTROL ] );

	SendSPI( CMD_ST7789V[ POSITIVE_VOLTAGE ] );
	SendSPI( CMD_ST7789V[ NEGATIVE_VOLTAGE ] );

	return;
}

void LCDC_StartDisplay(void)
{
	// ---- start display ----
	SendSPI( CMD_ST7789V[ DISPLAY_INV ] );
	SendSPI( CMD_ST7789V[ SLEEP_OUT ] );
	SendSPI( CMD_ST7789V[ DISPLAY_ON ] );

	return;
}

// ---- LCDC API function ----

#define LCDC_W		240
#define LCDC_H		320

void LCDC_SetWindow(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
	uint8_t	seq[7];

	seq[0] = 0x05;			// command len
	seq[1] = 0x2A;			// X
	seq[2] = X >> 8;		// big endian
	seq[3] = X & 0xff;
	seq[4] = (Width - 1) >> 8;
	seq[5] = (Width - 1) & 0xff;
	SendSPI( seq );

	seq[1] = 0x2B;			// Y
	seq[2] = Y >> 8;		// big endian
	seq[3] = Y & 0xff;
	seq[4] = (Height - 1) >> 8;
	seq[5] = (Height - 1) & 0xff;
	SendSPI( seq );

	return;
}

void LCDC_ClearScreen(uint16_t color)
{
    uint32_t	i, num;
	uint8_t		*p;

	LCDC_SetWindow(0,0,LCDC_W,LCDC_H);	// 240x320
	//SendSPI( CMD_ST7789V[ ADDRESS_X ] );
	//SendSPI( CMD_ST7789V[ ADDRESS_Y ] );

	SendSPI( CMD_ST7789V[ MEMORY_WRITE ] );
    gpio_put(GPO_LCDC_DC, 1);
    gpio_put(GPO_LCDC_CS, 0);

	num = LCDC_W * LCDC_H;
	p = &color;
    for (i=0; i < num; i++) {
	    spi_write_blocking(SPI_PORT, p + 1, 1);		// little to big endian
    	spi_write_blocking(SPI_PORT, p, 1);
	}

    gpio_put(GPO_LCDC_CS, 1);

	SendSPI( CMD_ST7789V[ DISPLAY_ON ] );
	return;
}

void LCDC_Redraw(uint8_t *vram)
{
    int			x, y;
	uint8_t		*p;

	SendSPI( CMD_ST7789V[ MEMORY_WRITE ] );

    gpio_put(GPO_LCDC_DC, 1);
    gpio_put(GPO_LCDC_CS, 0);

    for (y=0; y < LCDC_W; y++) {
		p = vram + (LCDC_H * 2) * y;
 		for (x=0; x < LCDC_H; x++) {
		    spi_write_blocking(SPI_PORT, p + 1, 1);		// little to big endian
	    	spi_write_blocking(SPI_PORT, p, 1);

			p += 2;
		}
    }

    gpio_put(GPO_LCDC_CS, 1);

    return;
}

uint8_t SetBacklightLevel(uint8_t new_level)
{
	const uint16_t	bl_step16[16] = { 0, 1, 2, 3,  4, 6, 8, 12,  16, 24, 32, 40,  48, 64, 80, 100 };
	static uint8_t	bl_level_now;

	switch (new_level) {
	case 0xFF:
		if (bl_level_now > 0) bl_level_now--;
		break;
	case 0xFE:
		if (bl_level_now < 15) bl_level_now++;
		break;
	default:
		bl_level_now = new_level;
		if (bl_level_now > 15) bl_level_now = 15;
	}

    BacklightControl(bl_step16[bl_level_now]);		// 0:off - 100:brite

	return(bl_level_now);
}

void lcdc_test_16color(void)
{
	const uint16_t COL16[16] = {	0x0000, 0xF800, 0x07E0, 0xFFE0, 0x001F, 0xF81F, 0x07FF, 0xFFFF,
									0x39E7, 0x7800, 0x03E0, 0x7BE0, 0x000F, 0x780F, 0x03EF, 0x7BEF };
	int		i;

	for (i=0; i < 16; i++) {
		LCDC_ClearScreen(COL16[i]);
		sleep_ms(1000);
	}

	return;
}

void lcdc_init(uint32_t *lcdc_w, uint32_t *lcdc_h)
{

    //stdio_init_all();

	InitGPIO();
	EnableBacklightControl();

	InitSPI();
    InitLCDC();
	LCDC_ClearScreen(0);
	LCDC_StartDisplay();
	SetBacklightLevel(12);	// level 12/16, half power

	*lcdc_w = LCDC_W;
	*lcdc_h = LCDC_H;

	return;
}

//[EOF]
