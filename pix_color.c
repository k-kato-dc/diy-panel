
//---- run level color func ----//

BYTE RunLevelColor(BYTE level)
{
	static BYTE	SwitchColMap[17] = {
			// sw--color-----attribute-----------------------description-------
		10,	// on  green	std.normal.running		0: Normally running
		 2,	// off green	std.normal.stopped		1: Normally stopped
		11,	// on  yellow	std.warning.running		2: Anomaly detected
		 3,	// off yellow	std.warning.stopped		3: Suspended
		 9,	// on  red	std.error.running		4: Hazardous behavior
		 1,	// off red	std.error.stopped		5: Error halt
		 8,	// on  black	std.undetectable.running	6: Monitoring failed
		 0,	// off black	std.undetectable.stopped	7: uninstalled
		12,	// on  blue	opt.normal.running
		 4,	// off blue	opt.normal.stopped
		14,	// on  cyan	opt.warning.running
		 6,	// off cyan	opt.warning.stopped
		13,	// on  magenta	opt,error.running
		 5,	// off magenta	opt.error.stopped
		15,	// on  white	opt.other.running
		 7,	// off white	opt.other.stopped
		16  };	// (not disp)

	if (level > 16) {
		level = 16;
	}

	return( SwitchColMap[level] );
}

// ---- Color table func ---- //

#define COLOR4_NUM		8

static BYTE Color4[COLOR4_NUM][12] = {
	{ 26,26,26, 55,55,55, 96,96,96, 127,127,127 },	  // black
	{ 0,26,112, 0,61,188, 13,104,236, 109,163,244 },	// brawn
	{ 0,61,22, 18,109,54, 49,199,127, 122,202,155 },	// green
	{ 0,65,101, 0,111,170, 49,183,255, 121,208,255 },       // gold
	{ 102,48,0, 170,95,10, 206,139,62, 234,191,144 },       // blue
	{ 74,0,98, 255,16,143, 255,78,140, 255,200,202 },       // purple
	{ 77,73,0, 136,130,0, 208,198,0, 255,249,119 },	 // cyan
	{ 96,96,96, 128,128,128, 160,160,160, 200,200,200 } };  // white


void BGRInterpolation(BYTE *dst, BYTE *src, BYTE dst_num, BYTE isSrc4) 
{
	BYTE	i, j, BGR;
	BYTE	*p;
	//BYTE	*srcl, *srcr;
	float	val, l_dif[3], r_dif[3], l_val[3], r_val[3], ratio;


	// set src 3 color bytes to float value
	BGR = 3;
	for (i=0; i < BGR; i++) {
		if (isSrc4) {
			// src is 4 pixels(l-1, l, r, r+1) x 3 color bytes
			l_dif[i] = (float)(src[i + BGR] - src[i]);
    		r_dif[i] = (float)(src[i + 2 * BGR] - src[i + 3 * BGR]);
			l_val[i] = src[i + BGR];
			r_val[i] = src[i + 2 * BGR];
		} else {
			// src is 2 pixels(l, r) x 3 color bytes
			l_dif[i] = 0;			// same as src[0] = src[1] in 4 pixel
			r_dif[i] = 0;			// same as src[2] = src[3] in 4 pixel
			l_val[i] = src[i];
			r_val[i] = src[i + BGR];
		}
	}

	// set dst color sequence
	p = dst;
	for (j = 0; j <= (dst_num - 1); j++) {		// pixel counter
		ratio = (float)j / (float)(dst_num -1);

		for (i=0; i < BGR; i++) {		// color counter(3)
			// extimate from left
			val = (l_val[i] + l_dif[i] * ratio) * (1 - ratio);
			//val = l_val[i] * (1 - ratio) + (l_dif[i] / 4) * ratio;

			// extimate from right
			val += (r_val[i] + r_dif[i] * (1 - ratio)) * ratio;
			//val += r_val[i] * ratio + (r_dif[i] / 4) * (1 - ratio);

			// range 0 to 255
			if (val < 0) {
				val = 0;
			} else if (val > 255) {
				val = 255;
			}

			// to byte value
			*p = (BYTE)val;
			p++; 
		}
	}

	return;
}

void PrintBGRInterpolation(BYTE *dst, BYTE *src, BYTE dst_num, BYTE isSrc4)
{
	BYTE i, j, src_num, *p;

	BGRInterpolation(dst, src, dst_num, isSrc4);

	src_num = 2;
	if (isSrc4) {
		src_num = 4;
	}

	printf("---- src ----\n");
	for (i=0; i < src_num; i++) {
		p = src + i * 3;
		printf("%2d: %02X %02X %02X\n", i, p[0], p[1], p[2]);
	}
	printf("---- dst ----\n");
	for (j=0; j < dst_num; j++) {
		p = dst + j * 3;
		printf("%2d: %02X %02X %02X", j, p[0], p[1], p[2]);
		for (i=0; i < 68; i++) {
			if (p[0] /4== i) {
				printf("1");
			} else if (p[1]/4 == i) {
				printf("2");
			} else if (p[2]/4 == i) {
				printf("3");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}

//---- color convert func ----//

void GetBGRVal(BYTE *bgr, BYTE *pix)
{
	DWORD	dw;

	dw = (pix[0] << 8) | pix[1];

	bgr[0] = (BYTE)((dw & 0xF800) >> 9);
	bgr[1] = (BYTE)((dw & 0x07E0) >> 3);
	bgr[2] = (BYTE)((dw & 0x001F) << 3);
	return;
}

void SetBGRVal(BYTE *pix, BYTE *bgr)
{
	DWORD	dw, r, g, b;

	b = (DWORD)bgr[0];
	g = (DWORD)bgr[1];
	r = (DWORD)bgr[2];

	dw = ((b & 0x00F8) >> 3) | ((g & 0x00FC) << 3) | ((r & 0x00F8) << 8);

	pix[0] = (BYTE)(dw >> 8);
	pix[1] = (BYTE)(dw & 0xFF);

	return;
}


// ---- 7seg digit color func  ---- //

void ConvertColorFromCyanInit(BYTE *pix, BYTE num, BYTE width, BYTE height)
{
	WORD	    w, w2, *p, *dst, *src;
	DWORD	   n, x, y, col, colofs;

	// color convert; loaded cyan bitmap -> 8 colors
	p = (WORD *)(pix);
	colofs = width * height * num;		// maybe 12 digit

	col = 6;	// cyan011
	src = &(p[col * colofs]); 
	col = 7;	// white111
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w = ((*src >> 8) & 0x001F) << 3;	// R = B, bigendian RGB
				*dst = (*src & 0xFF07) | w;
				src++;
				dst++;
			}
		}
	}

	col = 6;	// cyan011
	src = &(p[col * colofs]);
	col = 4;	// blue001
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w = ((*src >> 3) & 0x001F);
				w2 = (w & 0x0003) << 14;
				w = ((w & 0x001C) >> 2) | w2;		// G = R, bigendian RGB
				*dst = (*src & 0x1FF8) | w;
				src++;
				dst++;
			}
		}
	}

	col = 6;	// cyan011
	src = &(p[col * colofs]);
	col = 2;	// green010
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w = ((*src >> 3) & 0x001F) << 8;	// B = R, bigendian RGB
				*dst = (*src & 0xE0FF) | w;
				src++;
				dst++;
			}
		}
	}

	col = 6;	// cyan011
	src = &(p[col * colofs]);
	col = 3;	// yellow110
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w2 = ((*src >> 3) & 0x001F) << 8;	// R
				w = (((*src >> 8) & 0x001F) << 3) | w2;	// swap(B, R), bigendian RGB
				*dst = (*src & 0xE007) | w;
				src++;
				dst++;
			}
		}
	}

	col = 4;	// blue001
	src = &(p[col * colofs]);
	col = 1;	// red100
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w2 = ((*src >> 3) & 0x001F) << 8;	// R
				w = (((*src >> 8) & 0x001F) << 3) | w2;	// swap(B, R), bigendian RGB
				*dst = (*src & 0xE007) | w;
				src++;
				dst++;
			}
		}
	}

	col = 4;	// blue001
	src = &(p[col * colofs]); 
	col = 5;	// magenta101
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w = ((*src >> 8) & 0x001F) << 3;	// R = B, bigendian RGB
				*dst = (*src & 0xFF07) | w;
				src++;
				dst++;
			}
		}
	}

	col = 4;	// blue001
	src = &(p[col * colofs]); 
	col = 0;	// black000
	dst = &(p[col * colofs]);
	for (n=0; n < num; n++) {
		for (y=0; y < height; y++) {
			for (x=0; x < width; x++) {
				w = ((*src >> 3) & 0x001F) << 8;	// B = R, bigendian RGB
				*dst = (*src & 0xE0FF) | w;
				src++;
				dst++;
			}
		}
	}

	return;
}


//[EOF]

