
//---- panel parts file func ----//

typedef struct tagPARTS_BMP {
	BYTE			fname[256];
	struct stat		st;
	int			fd;
	DWORD			buf_read;
	BYTE			buf[1048576];	// 1MB > 320x170x3 x2 + 160x160x3 x8

	BITMAPFILE		bm;

	BYTE			pix[1048576];	// 1MB > 320x170x2 x9
} PARTS_BMP;

void LoadPartsFile(PARTS_BMP *pf)
{
	// stat file
	memset((BYTE *)&(pf->st), 0, sizeof(struct stat));
	if (stat(pf->fname, &(pf->st)) != 0) {
		printf("parts file stat error.\n");
		return;
	}
	if (pf->st.st_size > sizeof(pf->buf)) {
		printf("parts file too large.");
		return;
	}

	// read file
	pf->fd = open(pf->fname, O_RDONLY);
	if (pf->fd < 0) {
		printf("parts file open error.\n");
		return;
	}
	pf->buf_read = read(pf->fd, pf->buf, sizeof(pf->buf));
	close(pf->fd);
	pf->fd = -1;
	printf("Loaded parts bitmap: %s (%d byte)\n", pf->fname, pf->buf_read);

	// attach bitmap structure
	AttachBitmapStruct(&(pf->bm), pf->buf);

	return;
}

// ---- panel parts func ---- //

#define pPixParts(pnum, inum)	(PixParts[pnum].pix + (inum) * PixParts[pnum].part_byte)

typedef struct tagPANEL_PARTS {
	DWORD	part_num;
	DWORD	width;
	DWORD	height;
	DWORD	part_byte;
	//BYTE	*pix;
	BYTE	pix[1048576];
} PANEL_PARTS;

#define PARTS_NUM		8

#define	PARTS_SINGLE_PANE	0
#define	PARTS_DUAL_PANE		1
#define	PARTS_SIGNAL_LAMP	2
#define	PARTS_SLIDE_SWITCH	3
#define	PARTS_PANE_LABEL	4
#define	PARTS_CLOCK_LETTER	5
#define	PARTS_COLOR_TABLE	6
#define	PARTS_PLOT_40		7

static PANEL_PARTS	PixParts[PARTS_NUM];

void InitSinglePane(BYTE *parts_pix)
{
	PANEL_PARTS     *pp;
	WORD	    *p, *dst, *src;
	DWORD	   n, x, y, src_x, src_y, src_h, src_w, size;

	printf("---- InitSinglePane()\n");

	pp = &(PixParts[PARTS_SINGLE_PANE]);

	// alloc parts buffer
	pp->part_num    = 16;
	pp->width       = 160;
	pp->height      = 160;
	pp->part_byte   = pp->width * pp->height * 2;
	size = pp->part_num * pp->part_byte;
	dst = (WORD *)(pp->pix);

	// set src location
	src_x = 0;
	src_y = 0;
	src_w = 160;
	src_h = 1280;
	p = (WORD *)(parts_pix);				// image: 320x170x2 bg=#000000 landscape
	src = &(p[src_y * src_w + src_x]);      // entry: 40x10x2 x 8 colors

	// clop image for left pane
	for (n=0; n < (pp->part_num / 2); n++) {
		for (y=0; y < pp->height; y++) {
			for (x=0; x < pp->width; x++) {
				*dst = src[x];
				dst++;
			}
			src += src_w;
		}
	}

	// rotate 180 image for right pane
	p = (WORD *)(pp->pix);
	for (; n < pp->part_num; n++) {
		for (y=0; y < pp->height; y++) {
			src = &(p[(pp->height - 1 - y) * pp->width + (160*160 * (n - 8))]);	// reverse line
			for (x=0; x < pp->width; x++) {
				*dst = src[pp->width - 1 - x];				// reverse column
				dst++;
			}
		}
	}

	return;
}

void InitPaneLabel(BYTE *parts_pix)
{
	PANEL_PARTS     *pp;
	WORD	    *p, *dst, *src;
	//DWORD	   n; 
		DWORD			x, y, src_x, src_y, src_h, src_w, offset;

	printf("---- InitPaneLabel()\n");

	pp = &(PixParts[PARTS_PANE_LABEL]);

	// alloc parts buffer
	//pp->part_num    = 16;
	pp->part_num    = 2;
	pp->width       = 160;
	pp->height      = 160;
	pp->part_byte   = pp->width * pp->height * 2;
	dst = (WORD *)(pp->pix);
	offset = (pp->part_num / 2) * pp->width * pp->height;

	// set src location
	src_x = 0;
	src_y = 10;
	src_w = 320;
	src_h = 170;
	p = (WORD *)(parts_pix);		// image: 320x170x2 bg=#000000 landscape
	src = &(p[src_y * src_w + src_x]);      // entry: 160x160x2 x 2 left-right

	// clop image for left pane(n=0-159) and right pane(n=160-319)
	for (y=0; y < pp->height; y++) {
		for (x=0; x < pp->width; x++) {
			// left pane
			if ((x < 32) && (y > 16)) {
				// erase switch image
				*dst = 0x0000;
			} else {
			       	*dst = src[x];
			}

			// right pane
			dst[offset] = src[pp->width + x];

			dst++;
		}
		src += src_w;
	}

	return;
}

void InitSignalLamp(BYTE *parts_pix)
{
	PANEL_PARTS     *pp;
	WORD	    *p, *dst, *src;
	DWORD	   n, x, y, src_x, src_y, src_h, src_w;

	printf("---- InitSignalLamp()\n");

	pp = &(PixParts[PARTS_SIGNAL_LAMP]);

	// alloc parts buffer
	pp->part_num    = 16;
	pp->width       = 40;
	pp->height      = 10;
	pp->part_byte   = pp->width * pp->height * 2;
	dst = (WORD *)(pp->pix);

	// set src location
	src_x = 0;
	src_y = 0;
	src_w = 320;
	src_h = 170;
	p = (WORD *)(parts_pix);		// image: 320x170x2 bg=#000000 landscape

	// clop image
	for (n=0; n < (pp->part_num / 2); n++) {
		src = &(p[src_y * src_w + src_x + (pp->width * n)]);	// left to right 
		for (y=0; y < pp->height; y++) {
			for (x=0; x < pp->width; x++) {
				*dst = src[x];
				dst++;
			}
			src += src_w;
		}
	}

	// additional image for blink
	src = (WORD *)(pp->pix);
	for (; n < pp->part_num; n++) {
		for (y=0; y < pp->height; y++) {
			for (x=0; x < pp->width; x++) {
				*dst = src[x];		// lamp off
				dst++;
			}
		}
	}

	return;
}

void InitSlideSwitch(BYTE *parts_pix)
{
	PANEL_PARTS	*pp;
	WORD		*p, *dst, *src;
	DWORD		n, x, y, src_x, src_y, src_h, src_w;

	printf("---- InitSlideSwitch()\n");

	pp = &(PixParts[PARTS_SLIDE_SWITCH]);

	// alloc parts buffer
	pp->part_num	= 16;
	pp->width	= 32;
	pp->height	= 18;
	pp->part_byte	= pp->width * pp->height * 2;
	dst = (WORD *)(pp->pix);

	// set src location
	src_x = 0;
	src_y = 27;
	src_w = 320;
	src_h = 170;
	p = (WORD *)(parts_pix);			// image: 320x170x2 bg=#000000 landscape
	src = &(p[src_y * src_w + src_x]);	// entry: 16 + 18x32x2 x 8 colors

	// clop image
	for (n=0; n < (pp->part_num / 2); n++) {
		for (y=0; y < pp->height; y++) {
			for (x=0; x < pp->width; x++) {
				*dst = src[x];
				dst++;
			}
			src += src_w;
		}
	}

	// mirror image for switch-on
	src = (WORD *)(pp->pix);
	for (; n < pp->part_num; n++) {
		for (y=0; y < pp->height; y++) {
			for (x=0; x < pp->width; x++) {
				*dst = src[pp->width - 1 - x];

				dst++;
			}
			src += pp->width;
		}
	}

	return;
}

void InitClockDisp(BYTE *parts_pix)
{
	PANEL_PARTS     *pp;
	WORD			*p, *dst, *src;
	DWORD			n, x, y, src_x, src_y, src_h, src_w, offset, col, colofs;

	printf("---- InitClockDisp()\n");

	pp = &(PixParts[PARTS_CLOCK_LETTER]);

	// alloc parts buffer
	pp->part_num	= 12 * 8;	// 12 digit x 8 color
	pp->width	= 11;
	pp->height	= 22;
	pp->part_byte	= pp->width * pp->height * 2;
	offset = pp->width * pp->height;	// 1 digit
	colofs = offset * 12;				// 12 digit

	// set src location
	src_x = 0;
	src_y = 0;
	src_w = 132;
	src_h = 22;
	p = (WORD *)(parts_pix);		// image: 132x22x2 bg=#000000
	src = &(p[src_y * src_w + src_x]);	// entry: 0 + 11x22x2 x 12 digit(0-9 - :)

	// clop image
	col = 6;	// cyan
	p = (WORD *)(pp->pix);
	dst = &(p[col * colofs]); 

	for (y=0; y < pp->height; y++) {
		for (x=0; x < pp->width; x++) {
			for (n=0; n < (pp->part_num / 8); n++) {
				dst[offset * n + x] = src[n * pp->width + x];
			}
		}
		src += src_w;
		dst += pp->width;
	}

	// color convert; loaded cyan bitmap -> 8 colors
	ConvertColorFromCyanInit(pp->pix, (pp->part_num / 8), pp->width, pp->height);

	return;
}


#define pPixCol(table, level)   ((PixParts[PARTS_COLOR_TABLE].pix) + (table) * (PixParts[PARTS_COLOR_TABLE].part_byte) + (level) * 2)

void InitColorTable()
{
	PANEL_PARTS	*pp;
	BYTE    	buf[48], *src, *dst;
	BYTE    	i, j, BGR;

	printf("---- InitColorTable\n");

	pp = &(PixParts[PARTS_COLOR_TABLE]);

	// alloc parts buffer
	pp->part_num	= 8;		// 8 color
	pp->width	= 1;
	pp->height	= 16;		// 16 level
	pp->part_byte	= pp->width * pp->height * 2;
	memset(pp->pix, 0, pp->part_byte * pp->part_num);

	BGR = 3;
	for (j=0; j < pp->part_num; j++)
	{
		// GRB 4 color to 10 color
		src = Color4[j];
		memset(buf, 0 , 10 * BGR);
		for (i=0; i < 3; i++) {
			BGRInterpolation(buf + i * (3 * BGR), src + i * BGR, 4, 0);
		}

		// convert GRB888 to RGB565 
		src = buf;
		dst = pPixCol(j, 3);    // 3-12: 10 level for plot
		for (i=0; i < 10; i++) {
			SetBGRVal(dst, src);

			dst += 2;
			src += 3;
		}
	}

	return;

}

#define pPixPlot40(num)   ((PixParts[PARTS_PLOT_40].pix) + (num) * (PixParts[PARTS_PLOT_40].part_byte))

void C16PixPlotInitValue()
{
	static BYTE     data1[10] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
	DWORD   i, j;

	//printf("---- C16PixPlotInitValue\n");

	for (j=0; j < 10; j++) {
		for (i=0; i < 8; i++) {
			StatFile[STATFILE_DSTAT].val[i] = data1[j];
		}
		C16PixPlotUpdate();
	}

	return;
}

void InitPlot40()
{
	PANEL_PARTS	*pp;

	printf("---- InitPlot40\n");

	pp = &(PixParts[PARTS_PLOT_40]);

	// alloc parts buffer
	pp->part_num	= 8;		// 8 plots in right pane
	pp->width	= 40;
	pp->height	= 40;
	pp->part_byte	= pp->width * pp->height * 2;
	memset(pp->pix, 0, pp->part_byte * pp->part_num);

	C16PixPlotInitValue();
	
	return;

}

void InitPanelParts()
{
	const BYTE fname[3][256] = {
			SUBFILEPATH("bitmap", "pane_background.bmp"),
			SUBFILEPATH("bitmap", "pane_overlay.bmp"),
			SUBFILEPATH("bitmap", "pane_7segc.bmp") };
	static PARTS_BMP	pf;

	printf("---- InitPanelParts() Start...\n");

	memset(PixParts, 0, sizeof(PANEL_PARTS) * PARTS_NUM);

	// color table
	InitColorTable();

	// plot40
	InitPlot40();

	// pane background
	memcpy(pf.fname, fname[0], strlen(fname[0])+1);
	LoadPartsFile(&pf);

	BMPtoC16(pf.pix, &(pf.bm), 1);

	if (pf.buf_read >= 614400) {	// 160x160x3 x8
		InitSinglePane(pf.pix);
	}

	// pane overlay
	memcpy(pf.fname, fname[1], strlen(fname[1])+1);
	LoadPartsFile(&pf);

	BMPtoC16(pf.pix, &(pf.bm), 1);

	if (pf.buf_read >= 163200) {	// 320x170x3
		InitSignalLamp(pf.pix);
		InitSlideSwitch(pf.pix);
		InitPaneLabel(pf.pix);
	}

	// 7seg
	memcpy(pf.fname, fname[2], strlen(fname[2])+1);
	LoadPartsFile(&pf);

	BMPtoC16(pf.pix, &(pf.bm), 1);

	if (pf.buf_read >= 8712) {	// 22x11x12
		InitClockDisp(pf.pix);
	}

	printf("---- InitPanelParts() ...end\n");

	return;
}

void FreePanelParts()
{
	int	i;
	//BYTE	*p;
	DWORD	used_bytes;

	printf("---- FreePanelParts()\n");

	used_bytes = sizeof(PANEL_PARTS) * PARTS_NUM;
	for (i=0; i < PARTS_NUM; i++) {
		used_bytes += PixParts[i].part_num * PixParts[i].part_byte;
		printf("pix[%d]:%dx%d ", i, PixParts[i].part_num, PixParts[i].part_byte);
	}

	memset(PixParts, 0, sizeof(PANEL_PARTS) * PARTS_NUM);
	printf("\n...cleared %d bytes.\n", used_bytes);

	return;	
}


void C16PaneUpdate(BYTE *pix)
{
	WORD 	w, *dst, *src_l, *src_r;
	//DWORD	n;
	DWORD	x, y;

	//printf("---- C16PaneUpdate\n");

	src_l = (WORD *)(pPixParts(PARTS_SINGLE_PANE, GetPaneColor(0)));	// left pane
	src_r = (WORD *)(pPixParts(PARTS_SINGLE_PANE, (GetPaneColor(1) + 8)));	// right pane (rot 180)
	dst   = (WORD *)(pix);

	// background
	for (y=0; y < 160; y++) {
		for (x=0; x < 160; x++) {
			dst[(10 + y) * 320 + x] = src_l[y * 160 + x];
			dst[(10 + y) * 320 + x + 160] = src_r[y * 160 + x];
		}
	}

	// label
	src_l = (WORD *)(pPixParts(PARTS_PANE_LABEL, 0));		// 0: left pane (pstat)
	src_r = (WORD *)(pPixParts(PARTS_PANE_LABEL, 1));		// 1: right pane (dstat)
	dst   = (WORD *)(pix);

	for (y=0; y < 160; y++) {
		for (x=0; x < 160; x++) {
			w = src_l[y * 160 + x];
			if (w != 0x0000) {
				dst[(10 + y) * 320 + x] = w;
			}
			w = src_r[y * 160 + x];
			if (w != 0x0000) {
				dst[(10 + y) * 320 + x + 160] = w;
			}
		}
	}

	return;
}

void C16Digit11x22(C16PIXRECT *dp, BYTE val, BYTE col8, BYTE half_mode)
{
	WORD	w, *src, *dst;
	DWORD	x, y, src_w, src_h, src_n, src_w2, src_h2;
	//DWORD	n, dst_x, dst_y, col;

	// clock
	dst = dp->pix;
	dst += dp->y * dp->w + dp->x;

	// src digit parts
	src_w = 11;
	src_h = 22;
	src_n = 12;	// number of digit
	src = (WORD *)(pPixParts(PARTS_CLOCK_LETTER, val));
	src += col8 * src_w * src_h * src_n;

	if (half_mode == 3) {

		// 4 x 11
		src_w2 = 4;
		src_h2 = 11;
		for(y=0; y < src_h2; y++) {
			for(x=0; x < src_w2; x++) {
				w = src[x * 2 + 2];
				if (w != 0x0000) {
					dst[x] = w; 
				}
			}
			src += src_w * 2;
			dst += dp->w;
		}

	} else if (half_mode == 2) {

		// 6 x 11
		src_w2 = 6;
		src_h2 = 11;
		for(y=0; y < src_h2; y++) {
			for(x=0; x < src_w2; x++) {
				w = src[x * 2];
				if (w != 0x0000) {
					dst[x] = w; 
				}
			}
			src += src_w * 2;
			dst += dp->w;
		}

	} else if (half_mode == 1) {

		// 5 x 22
		src_w2 = 5;
		src_h2 = 22;
		for(y=0; y < src_h2; y++) {
			for(x=0; x < src_w2; x++) {
				w = src[x + 2];
				if (w != 0x0000) {
					dst[x] = w; 
				}
				//src++;
			}
			src += src_w;
			dst += dp->w;
		}

	} else {

		// 11 x 22
		for(y=0; y < src_h; y++) {
			for(x=0; x < src_w; x++) {
				if (*src != 0x0000) {
					dst[x] = *src; 
				}
				src++;
			}
			dst += dp->w;
		}
	}

	return;
}


void C16ClockUpdate(BYTE *pix, DWORD x, DWORD y, DWORD dstat_time, BYTE dstat_ok_col, BYTE dstat_ng_col)
{
	struct tm	tm;
	time_t		t;
	C16PIXRECT	dp;
	DWORD		val, col;

	//printf("---- C16ClockUpdate\n");

	// get current time
	t = time(NULL);
	localtime_r(&t, &tm);

	// set color; waning if dstat stopped 
	col = dstat_ok_col;		// maybe cyan
	if (t - dstat_time > 20) {
		col = dstat_ng_col;	// maybe yellow
	}

	// date overlay

	dp.pix = (WORD *)pix;
	dp.x = x + 3;
	dp.y = y;
	dp.w = 320;
	dp.h = 170;
	val = (tm.tm_year - 100) / 10;
	C16Digit11x22(&dp, val, col, 2);
	dp.x += 6;
	val = (tm.tm_year - 100) % 10;
	C16Digit11x22(&dp, val, col, 2);

	dp.x += 6;
	C16Digit11x22(&dp, 10, col, 3);

	dp.x += 4;
	val = (tm.tm_mon + 1) / 10;
	if (val != 0) {
		C16Digit11x22(&dp, val, col, 2);
	}
	dp.x += 6;
	val = (tm.tm_mon + 1) % 10;
	C16Digit11x22(&dp, val, col, 2);

	dp.x += 6;
	C16Digit11x22(&dp, 10, col, 3);

	dp.x += 4;
	val = tm.tm_mday / 10;
	if (val != 0) {
		C16Digit11x22(&dp, val, col, 2);
	}
	dp.x += 6;
	val = tm.tm_mday % 10;
	C16Digit11x22(&dp, val, col, 2);

	// time overlay

	dp.x = x;
	dp.y += 18;
	val = tm.tm_hour / 10;
	if (val != 0) {
		C16Digit11x22(&dp, val, col, 0);
	}
	dp.x +=11;
	val = tm.tm_hour % 10;
	C16Digit11x22(&dp, val, col, 0);

	dp.x +=11;
	C16Digit11x22(&dp, 11, col, 0);

	dp.x +=5;
	val = tm.tm_min / 10;
	C16Digit11x22(&dp, val, col, 0);
	dp.x +=11;
	val = tm.tm_min % 10;
	C16Digit11x22(&dp, val, col, 0);

	return;
}

void C16PartsUpdate(BYTE *pix, BYTE *val8, BYTE *sig8)
{
	WORD 	*dst, *src;
	DWORD	n, x, y;

	//printf("---- C16PartsUpdate\n");

	// slide switch
	dst = (WORD *)(pix);
	for (n=0; n < 8; n++) {
		src = (WORD *)(pPixParts(PARTS_SLIDE_SWITCH, RunLevelColor(StatFile[STATFILE_PSTAT].val[n])));	// 0-7:off 8-15: on
		for (y=0; y < 18; y++) {
			for (x=0; x < 32; x++) {
				if (src[y * 32 + x] != 0x0000) {
					dst[(27 + y + 18 * n) * 320 + x] = src[y * 32 + x];
				}
			}
		}
	}

	// signal lamp
	dst = (WORD *)(pix);
	for (n=0; n < 8; n++) {
		src = (WORD *)(pPixParts(PARTS_SIGNAL_LAMP, StatFile[STATFILE_SIGNAL].val[n]));

		for (y=0; y < 10; y++) {
			for (x=0; x < 40; x++) {
				dst[y * 320 + x + (40 * n)] = src[y * 40 + x];
			}
		}
	}

	// clock
	C16ClockUpdate(pix, 162, 16, StatFile[STATFILE_DSTAT].time, 6, 3);	// ok:cyan6 ng:yellow3

	return;
}


//[EOF]
