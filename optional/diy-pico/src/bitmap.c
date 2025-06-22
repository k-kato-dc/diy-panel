
typedef uint8_t		BYTE;
typedef uint16_t	WORD;
typedef uint32_t	DWORD;
typedef int32_t		LONG;
typedef uint64_t	QWORD;

//---- bitmap struct ----

#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
        DWORD   biSize;
        LONG    biWidth;
        LONG    biHeight;
        WORD    biPlanes;
        WORD    biBitCount;
        DWORD   biCompression;
        DWORD   biSizeImage;
        LONG    biXPelsPerMeter;
        LONG    biYPelsPerMeter;
        DWORD   biClrUsed;
        DWORD   biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
        BITMAPINFOHEADER        bmiHeader;
        RGBQUAD                 bmiColors[1];
} BITMAPINFO;
//#pragma pack(8) //default
#pragma pack(pop)

typedef struct tagBITMAPFILE {
	BITMAPFILEHEADER	*f;
	BITMAPINFOHEADER	*i;
	RGBQUAD			*c;
	BYTE			*b;
} BITMAPFILE;


void AttachBitmapStruct(BITMAPFILE *bm, BYTE *buf)
{
        BYTE *p;

        if ( memcmp(buf, "BM", 2) != 0 ) {
                printf("Not bitmap file header.(%02X %02X)\n", buf[0], buf[1]);
                return;
        }

        p = buf;
        bm->f = (BITMAPFILEHEADER *)p;

        p += sizeof(BITMAPFILEHEADER);
        bm->i = (BITMAPINFOHEADER *)p;

        p += bm->i->biSize;
        bm->c = (RGBQUAD *)p;

        p = buf;
        p += bm->f->bfOffBits;
        bm->b = (BYTE *)p;

        return;
}

void PrintBitmapHeader(BITMAPFILE *bm)
{
        BYTE    *p;
        DWORD   padding;

        printf("---- BITMAP ADDRESS ----\n");
        printf("FileHeader      = %04lX\n",               (QWORD)(bm->f));
        printf("InfoHeader      = %04lX (+%04lX)\n",      (QWORD)(bm->i), (QWORD)((BYTE *)(bm->i) - (BYTE *)(bm->f)));
        printf("BmiColors       = %04lX (+%04lX)\n",      (QWORD)(bm->c), (QWORD)((BYTE *)(bm->c) - (BYTE *)(bm->f)));
        printf("Bitmap          = %04lX (+%04lX)\n",      (QWORD)(bm->b), (QWORD)((BYTE *)(bm->b) - (BYTE *)(bm->f)));

        printf("---- BITMAP FILE HEADER ----\n");
        if (bm->f != NULL) {
                p = (BYTE *)(bm->f);
                printf("bfType          = 0x%04X (%c%c)\n",     bm->f->bfType, p[0], p[1]);
                printf("bfSize          = 0x%08X (%d)\n" ,      bm->f->bfSize, bm->f->bfSize);
                printf("bfReserved1     = 0x%04X (%d)\n",       bm->f->bfReserved1, bm->f->bfReserved1);
                printf("bfReserved2     = 0x%04X (%d)\n",       bm->f->bfReserved2, bm->f->bfReserved2);
                printf("bfOffBits       = 0x%08X (%d)\n",       bm->f->bfOffBits, bm->f->bfOffBits);
        }

        printf("---- BITMAP INFO HEADER ----\n");
        if (bm->i != NULL) {
                printf("biSize          = %d (0x%02X)\n",       bm->i->biSize, bm->i->biSize);
                printf("biWidth         = %d\n",                        bm->i->biWidth);
                printf("biHeight        = %d\n",                        bm->i->biHeight);
                printf("biPlanes        = %d\n",                        bm->i->biPlanes);
                printf("biBitCount      = %d\n",                        bm->i->biBitCount);
                printf("biCompression   = %d\n",                        bm->i->biCompression);
                printf("biSizeImage     = %d\n",                        bm->i->biSizeImage);
                printf("biXPelsPerMeter = %d (%ddpi)\n",        bm->i->biXPelsPerMeter, bm->i->biXPelsPerMeter*254/10000);
                printf("biYPelsPerMeter = %d (%ddpi)\n",        bm->i->biYPelsPerMeter, bm->i->biYPelsPerMeter*254/10000);
                printf("biClrUsed       = %d\n",                        bm->i->biClrUsed);
                printf("biClrImportant  = %d\n",                        bm->i->biClrImportant);
        }

        printf("---- RGB QUAD ----\n");
        printf("item bytes  = %ld (0x%01lX)\n",           sizeof(RGBQUAD), sizeof(RGBQUAD));
        if (bm->i != NULL) {
                printf("Array bytes = %ld (0x%04lX)\n",           bm->i->biClrUsed * sizeof(RGBQUAD), bm->i->biClrUsed * sizeof(RGBQUAD));
        }

        printf("---- BITMAP BIT ----\n");
        if (bm->i != NULL) {
                printf("item bytes  = %d (0x%01X)\n",           (bm->i->biBitCount / 8), (bm->i->biBitCount / 8));
                if (bm->i->biHeight ==0) {
                        padding = 0;
                } else {
                        padding = (bm->i->biSizeImage / bm->i->biHeight) - (bm->i->biWidth * bm->i->biBitCount / 8);
                }
                printf("Array[y=%d][x=%d + padding=%d]\n",      bm->i->biHeight, bm->i->biWidth, padding);
        }

        return;
}

//---- 16bit pixel functions ----

typedef struct _PIXINFO {
	uint32_t	x;
	uint32_t	y;
	uint32_t	w;
	uint32_t	h;
	uint16_t	*p;
} PIXINFO;


void SetPixInfo(PIXINFO *pi, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t *p)
{
	pi->x = x;
	pi->y = y;
	pi->w = w;
	pi->h = h;
	pi->p = p;

	return;
}

void UpsidedownPix(PIXINFO *pi)
{
	uint32_t	x, y, half;
	uint16_t	d, *src, *dst;

	printf(" ---- UpsidedownPix()\n");

	half = pi->h / 2;

	for (y=0; y < half; y++) {
		src = pi->p + y * pi->w;
		dst = pi->p + (pi->h - 1 - y) * pi->w;
		for (x=0; x < pi->w; x++) {
			d = *src;
			*src = *dst;
			*dst = d;
			
			src++;
			dst++;
		}
	}

	return;
}

void Rotate90Pix(PIXINFO *dstpi, PIXINFO *srcpi, uint8_t mode)
{
	uint32_t	x, y, xblank, yblank, srcinc, srcofs;
	uint16_t	*src, *dst;

	printf(" ---- Rotate90Pix(%d)\n", mode);


	srcinc = 1;
	xblank = (LCDC_H - srcpi->w) / 2;	// center align
	yblank = (LCDC_W - srcpi->h) / 2;
	if (mode) {
		srcinc = -1;
		yblank = LCDC_W - 1 - yblank;
	}
	printf("xblank = %ld, yblank = %ld\n", xblank, yblank);

	for (y=0; y < srcpi->h; y++) {
		src = srcpi->p + y * srcpi->w + srcpi->w - 1;
		dst = dstpi->p + (LCDC_H - 1 - xblank) * LCDC_W + yblank + (srcinc * y);
		for (x=0; x < srcpi->w; x++) {
		//for (x=0; x < srcpi->w - srcpi->x; x++) {
			*dst = *src;
			
			src += srcinc;
			dst -= LCDC_W;
		}
	}

	dstpi->x = 0;
	dstpi->y = 0;
	dstpi->w = LCDC_W;
	dstpi->h = LCDC_H;

	return;
}

void LoadRotateBmp(uint8_t mode, BYTE plane)
{
	BITMAPFILE	bm;
	PIXINFO		src, dst;

	//printf(" ---- RotateBmp()\n");

	AttachBitmapStruct(&bm, FlashAddr(VramPlane(plane)));
	//PrintBitmapHeader(&bm);
	SetPixInfo(&src, 0, 0, bm.i->biWidth, bm.i->biHeight, bm.b);

	memset(VramBuf, 0, VRAM_SIZE);
	SetPixInfo(&dst, 0, 0, 320, 240, (uint16_t *)(VramBuf + 0x42));

	Rotate90Pix(&dst, &src, mode);

	UpsidedownPix(&dst);

	return;
}

//[EOF]
