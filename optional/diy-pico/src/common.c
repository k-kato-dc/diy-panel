// ---- portrait frame buffer of ST7789V controller

//#define LCDC_W				240
//#define LCDC_H				320

#define VRAM_SIZE			FLASH_PLANE_SIZE	// 160kb
#define VramPlane(num)		(11+(num))			// 11:0x1A0000 12:1C8000

static uint8_t	VramBuf[VRAM_SIZE];

/**
uint8_t *GetVramBuf()	// use LCD:BmpTest
{
	return( VramBuf );
}
**/

/**
uint8_t *GetVram0()
{
	return( FlashAddr(VramPlane(0)) );
}
/**/

//[EOF]
