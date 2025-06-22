
//---- snapshot file ----//

#define SNAPFILE_NUM		2
#define SNAPFILE_DUALPANE	0
#define SNAPFILE_STILLFRAME	1

#define SNAPFILE_NAMELEN	256
#define SNAPFILE_BUFSIZE	196608	// 192KB 320x204x3
#define SNAPFILE_PIXSIZE	131072	// 128KB 320x204x2
#define SNAPFILE_PIXNUM		8

typedef struct tagSNAP_FILE {
	BYTE		fname[SNAPFILE_NAMELEN];
	struct stat	st;
	int		fd;
	int		size;
	int		read;
	BYTE		buf[SNAPFILE_BUFSIZE];

	BITMAPFILE	bm;

	BYTE		*vram[SNAPFILE_PIXNUM];
	BYTE		pix[SNAPFILE_PIXNUM][SNAPFILE_PIXSIZE];
} SNAP_FILE;

static BYTE	VRAMFNAME[SNAPFILE_PIXNUM][SNAPFILE_NAMELEN] = {
	//SUBFILEPATH("bitmap", "panel0.bmp"),
	MODFILEPATHEX("html", "status", "panel0.bmp"),
	SUBFILEPATH("bitmap", "panel1.bmp"),
	SUBFILEPATH("bitmap", "prev-panel0.bmp"),
	SUBFILEPATH("bitmap", "prev-panel1.bmp"),
	SUBFILEPATH("bitmap", "new-panel0.bmp"),
	SUBFILEPATH("bitmap", "new-panel1.bmp"),
	SUBFILEPATH("bitmap", "factory0.bmp"),
	SUBFILEPATH("bitmap", "factory1.bmp") };

static BYTE	STILLFNAME[SNAPFILE_PIXNUM][SNAPFILE_NAMELEN] = {
	SUBFILEPATH("bitmap", "snapshot0.bmp"),
	SUBFILEPATH("bitmap", "snapshot1.bmp"),
	SUBFILEPATH("bitmap", "snapshot2.bmp"),
	SUBFILEPATH("bitmap", "snapshot3.bmp"),
	SUBFILEPATH("bitmap", "snapshot4.bmp"),
	SUBFILEPATH("bitmap", "snapshot5.bmp"),
	SUBFILEPATH("bitmap", "snapshot6.bmp"),
	SUBFILEPATH("bitmap", "snapshot7.bmp") };


static SNAP_FILE	SnapFile[SNAPFILE_NUM];

void InitSnapFile()
{
	SNAP_FILE	*sf;
	DWORD		i, j;
	WORD		*p, col;

	printf("---- InitSnapFile()\n");

	// init dual pane vram
	sf = &(SnapFile[SNAPFILE_DUALPANE]);	// 0
	memset((BYTE *)sf, 0, sizeof(SNAP_FILE));

	// check output file existance... cannot write if not exist
	for (i=0; i < SNAPFILE_PIXNUM; i++) {
		sf->vram[i] = sf->pix[i];

		if (VRAMFNAME[i][0] == '\0') {
			continue;
		}

		memset(sf->fname, 0, SNAPFILE_NAMELEN);
		memcpy(sf->fname, VRAMFNAME[i], strlen(VRAMFNAME[i]));
		if (stat(sf->fname, &(sf->st)) != 0) {
			//printf("panel image file not exist.(%s)\n", sf->fname);
			sf->fd = -1;
			continue;
		}
		printf("PanelFile[%d] will be used.(%s)\n", i, sf->fname);
	}

	// init still frame buffer
	sf = &(SnapFile[SNAPFILE_STILLFRAME]);	// 1
	memset((BYTE *)sf, 0, sizeof(SNAP_FILE));

	// load still image, if exist.
	for (i=0; i < SNAPFILE_PIXNUM; i++) {
		sf->vram[i] = sf->pix[i];
		BGR24toRGB565((BYTE *)(&col),Color4[i], 1);
		p = (WORD *)(sf->pix[i]);
		for (j=0; j < 320 * 170; j++) {
			p[j] = col;
		}

		if (STILLFNAME[i][0] == '\0') {
			continue;
		}

		memset(sf->fname, 0, SNAPFILE_NAMELEN);
		memcpy(sf->fname, STILLFNAME[i], strlen(STILLFNAME[i]));
		if (stat(sf->fname, &(sf->st)) != 0) {
			//printf("still image file not exist.(%s)\n", sf->fname);
			sf->fd = -1;
			continue;
		}

		sf->fd = open(sf->fname, O_RDONLY);
		if (sf->fd < 0) {
			printf("still image file open error.(%s)\n", sf->fname);
			continue;
		}

		// load bitmap
		sf->read = read(sf->fd, sf->buf, SNAPFILE_BUFSIZE);
		printf("StillImage[%d] has been loaded %d byte.(%s)\n", i, sf->read, sf->fname);
		close(sf->fd);
		sf->fd = -1;

		// convert 24bit bitmap to RGB565 image
		AttachBitmapStruct(&(sf->bm), sf->buf);
		BMPtoC16(sf->pix[i], &(sf->bm), 1);

	}

	return;
}

void FreeSnapFile()
{
	SNAP_FILE	*sf;
	DWORD		i;

	printf("---- FreeSnapFile()\n");

	sf = &(SnapFile[SNAPFILE_DUALPANE]);	// 0

	for (i=0; i < SNAPFILE_PIXNUM; i++) {
		if (sf->fd > 0) {
			close(sf->fd);
			sf->fd = -1;
		}
		sf->fname[0] = '\0';
	}

	return;
}

//---- screen buffer ----//

#define VRAM_LANDSCAPE	0
#define VRAM_PORTRAIT	1
#define VRAM_PREV0	2
#define VRAM_PREV1	3
#define VRAM_NEW0	4
#define VRAM_NEW1	5
#define VRAM_BACKGROUND	6
#define VRAM_OVERLAY	7
#define pPanelPtr(n)	(SnapFile[SNAPFILE_DUALPANE].vram[(n)])
#define pStillPtr(n)	(SnapFile[SNAPFILE_STILLFRAME].vram[(n)])

void SavePanelFile()
{
	SNAP_FILE	*sf;
	DWORD           x, y;
	BYTE            *p, *src, *dst;

	//printf("---- SavePanelFile\n");

	sf = &(SnapFile[SNAPFILE_DUALPANE]);

	sf->bm.f = (BITMAPFILEHEADER *)(sf->buf);
	sf->bm.i = (BITMAPINFOHEADER *)((BYTE *)(sf->bm.f) + sizeof(BITMAPFILEHEADER));
	sf->bm.c = (RGBQUAD *) ((BYTE *)(sf->bm.i) + sizeof(BITMAPINFOHEADER));
	sf->bm.b = (BYTE *)(sf->bm.c) + 12;

	sf->bm.f->bfType          = 0x4D42;
	sf->bm.f->bfSize          = 14 + 40 + 12 + (320 * 170 * 2);
	sf->bm.f->bfReserved1     = 0;
	sf->bm.f->bfReserved2     = 0;
	sf->bm.f->bfOffBits       = 14 + 40 + 12;

	sf->bm.i->biSize          = 40;
	sf->bm.i->biWidth         = 320;
	sf->bm.i->biHeight        = 170;
	sf->bm.i->biPlanes        = 1;
	sf->bm.i->biBitCount      = 16;
	sf->bm.i->biCompression   = 3;    // 0:BIRGB(555), 3:BI_BITFIELD(565)
	sf->bm.i->biSizeImage     = 320 * 170 * 2;
	sf->bm.i->biXPelsPerMeter = 3780;       // 96dpi
	sf->bm.i->biYPelsPerMeter = 3780;       // 96dpi
	sf->bm.i->biClrUsed       = 0;
	sf->bm.i->biClrImportant  = 0;

	memcpy((BYTE *)(sf->bm.c), BITFIELD565, 12);

	if (StatFile[STATFILE_PANEL_CONFIG].val[4] == 2) {
		src = (BYTE *)(sf->vram[GetPaneOrientation()]);					// landscape or portrait
	} else {
		src = (BYTE *)(sf->vram[(StatFile[STATFILE_PANEL_CONFIG].val[4] % 2)]);		// 0:landscape 1:portrait
	}

	p = (BYTE *)(sf->bm.b);
	for (y=0; y < 170; y++) {
		dst = p + (169 - y) * 320 * 2;
		for (x=0; x < 320; x++) {
			dst[0] = src[1];
			dst[1] = src[0];
			dst += 2;
			src += 2;
		}
	}

	memset(sf->fname, 0, SNAPFILE_NAMELEN);
	memcpy(sf->fname, VRAMFNAME[0], strlen(VRAMFNAME[0]));	// vram 0
	sf->fd = open(sf->fname, O_WRONLY);
	if (sf->fd < 0) {
		printf("save bmp file open error.(%s)\n", sf->fname);
		return;
	}
	sf->read = write(sf->fd, sf->buf, sf->bm.f->bfSize);
	close(sf->fd);
	sf->fd = -1;

	printf("saved %s: (%d bytes)\n", sf->fname, sf->read);

	return;
}

void AddPortraitPanel(BYTE *dst, BYTE *src)
{
	DWORD   src_width, src_height, pixbytes;
	DWORD   src_x, src_y, dst_x, dst_y, dst_width, dst_height, dst_linebytes;
	BYTE    *src_p, *dst_p;

	// copy signal area
	dst_width  = 320;
	dst_height = 10;                        // signal hight = 10
	pixbytes   = 2;
	dst_linebytes = dst_width * dst_height * pixbytes;
	memcpy(dst, src, dst_linebytes);

	dst += dst_linebytes;
	src += dst_linebytes;

	dst_width       = 320;
	dst_height      = 160;			// 160 = 170 - 10
	dst_linebytes   = dst_width * pixbytes;
	dst_y           = dst_height - 1;       // 159
	dst             += dst_y * dst_linebytes;

	src_width       = dst_width / 2;        // 160 = 320 / 2, two square area
	src_height      = src_width;            // 160 = 170 - 10
	src_p           = src;
	for (src_y=0; src_y < src_height; src_y++) {
		// rotate left rectangle
		dst_x = src_y;
		dst_p = dst + (dst_x * pixbytes);
		for (src_x=0; src_x < src_width; src_x++) {
			memcpy(dst_p, src_p, pixbytes);

			src_p += pixbytes;
			dst_p -= dst_linebytes;
		}

		// rotate right rectangle
		dst_x += src_width;
		dst_p = dst + (dst_x * pixbytes);
		for (src_x=0; src_x < src_width; src_x++) {
			memcpy(dst_p, src_p, pixbytes);

			src_p += pixbytes;
			dst_p -= dst_linebytes;
		}
	}

	return;
}

BYTE *GetStillPtr()
{
	BYTE	*p, *val;

	val = &(StatFile[STATFILE_PANEL_CONFIG].val[5]);

	p = NULL;
	switch (val[0]) {
	case 0:
		break;
	case 1:
		// for still image
		if (StatFile[STATFILE_PANEL_CONFIG].val[3] == 1) {

			// same color as rotated oanel background color
			p = SnapFile[SNAPFILE_STILLFRAME].pix[StatFile[STATFILE_PANEL_CONFIG].val[1]];

		} else if (val[0] == 1) {

			// local color cycle
			p = SnapFile[SNAPFILE_STILLFRAME].pix[val[1]];

		}
	case 2:
		val[0] = (val[0] % 2) + 1;	// for next time, panel disp
		break;
	}

	return(p);
}

void C16PaneUpdate(BYTE *pix);	// pix_parts.c
void C16PartsUpdate(BYTE *pix, BYTE *val8, BYTE *sig8);	// pix_parts.c
void C16PlotOverlay(BYTE *dstpix);	// pix_plot.c

BYTE *GetPanelPtr()
{
	BYTE	*ret, *p1, *p2;

	//printf("---- GetPanelPtr()\n");

	p1 = GetStillPtr();
	if (p1 != NULL) {
		return(p1);
	}

	// make landscape image
	p1 = pPanelPtr(VRAM_LANDSCAPE);

	C16PaneUpdate(p1);
	C16PartsUpdate(p1, NULL, NULL);
	C16PlotOverlay(p1);

	// make portrait image
	p2 = pPanelPtr(VRAM_PORTRAIT);
	AddPortraitPanel(p2, p1);

	switch ( GetPaneOrientationRaw() ) {		// 0:landscape 1:portrait 2:landscape(rot180) 3:portrait(rot180)
	case 1:
		//AddPortraitPanel(p2, p1);
		ret = p2;
		break;
	case 2:
		Rotate180Vram(p1);
		ret = p1;
		break;
	case 3:
		Rotate180Vram(p2);
		ret = p2;
		break;
	default:	// 0:landscape
		ret = p1;
		break;
	}

	SavePanelFile();

	RotatePaneColor();

	return(ret);
}


//[EOF]
