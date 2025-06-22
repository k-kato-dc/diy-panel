
//---- Bitmap function ----//

typedef struct tagBITMAPFILE {
	BITMAPFILEHEADER	*f;
	BITMAPINFOHEADER	*i;
	RGBQUAD			*c;
	BYTE			*b;
} BITMAPFILE;

const BYTE      BITFIELD565[12] = { 0x00, 0xF8, 0x00, 0x00,  0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 };

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
	printf("FileHeader      = %04lX\n",				(QWORD)(bm->f));
	printf("InfoHeader      = %04lX (+%04lX)\n",	(QWORD)(bm->i), (QWORD)((BYTE *)(bm->i) - (BYTE *)(bm->f)));
	printf("BmiColors       = %04lX (+%04lX)\n",	(QWORD)(bm->c), (QWORD)((BYTE *)(bm->c) - (BYTE *)(bm->f)));
	printf("Bitmap          = %04lX (+%04lX)\n",	(QWORD)(bm->b), (QWORD)((BYTE *)(bm->b) - (BYTE *)(bm->f)));

	printf("---- BITMAP FILE HEADER ----\n");
	if (bm->f != NULL) {
		p = (BYTE *)(bm->f);
		printf("bfType          = 0x%04X (%c%c)\n",	bm->f->bfType, p[0], p[1]);
		printf("bfSize          = 0x%08X (%d)\n",	bm->f->bfSize, bm->f->bfSize);
		printf("bfReserved1     = 0x%04X (%d)\n",	bm->f->bfReserved1, bm->f->bfReserved1);
		printf("bfReserved2     = 0x%04X (%d)\n",	bm->f->bfReserved2, bm->f->bfReserved2);
		printf("bfOffBits       = 0x%08X (%d)\n",	bm->f->bfOffBits, bm->f->bfOffBits);
	}

	printf("---- BITMAP INFO HEADER ----\n");
	if (bm->i != NULL) {
		printf("biSize          = %d (0x%02X)\n",	bm->i->biSize, bm->i->biSize);
		printf("biWidth         = %d\n",			bm->i->biWidth);
		printf("biHeight        = %d\n",			bm->i->biHeight);
		printf("biPlanes        = %d\n",			bm->i->biPlanes);
		printf("biBitCount      = %d\n",			bm->i->biBitCount);
		printf("biCompression   = %d\n",			bm->i->biCompression);
		printf("biSizeImage     = %d\n",			bm->i->biSizeImage);
		printf("biXPelsPerMeter = %d (%ddpi)\n",	bm->i->biXPelsPerMeter, bm->i->biXPelsPerMeter*254/10000);
		printf("biYPelsPerMeter = %d (%ddpi)\n",	bm->i->biYPelsPerMeter, bm->i->biYPelsPerMeter*254/10000);
		printf("biClrUsed       = %d\n",			bm->i->biClrUsed);
		printf("biClrImportant  = %d\n",			bm->i->biClrImportant);
	}

	printf("---- RGB QUAD ----\n");
	printf("item bytes  = %ld (0x%01lX)\n",			sizeof(RGBQUAD), sizeof(RGBQUAD));
	if (bm->i != NULL) {
		printf("Array bytes = %ld (0x%04lX)\n",		bm->i->biClrUsed * sizeof(RGBQUAD), bm->i->biClrUsed * sizeof(RGBQUAD));
	}

	printf("---- BITMAP BIT ----\n");
	if (bm->i != NULL) {
		printf("item bytes  = %d (0x%01X)\n",		(bm->i->biBitCount / 8), (bm->i->biBitCount / 8));
		if (bm->i->biHeight ==0) {
			padding = 0;
		} else {
			padding = (bm->i->biSizeImage / bm->i->biHeight) - (bm->i->biWidth * bm->i->biBitCount / 8);
		}
		printf("Array[y=%d][x=%d + padding=%d]\n",	bm->i->biHeight, bm->i->biWidth, padding);
	}

	return;
}

//---- BGR to C16 function ----//

BYTE *GetBmpBit(BYTE *p, RGBQUAD *col)
{
	if (col != NULL) {
		return( (BYTE *)(&(col[*p])) ); // 256(8-bit) color
	}

	return( p );
}

void BGR24toRGB565(BYTE *dst, BYTE *src, BYTE isBE)
{
	DWORD   dw;

	dw = (src[2] >> 3) & 0x1F;	      // R 5bit
	dw <<= 6;
	dw |= (src[1] >> 2) & 0x3F;	     // G 6bit
	dw <<= 5;
	dw |= (src[0] >> 3) & 0x1F;	     // B 5bit

	// little endian to big endian
	if (isBE) {
		dst[1] = dw & 0xFF;
		dst[0] = (dw >> 8) & 0xFF;
	} else {
		dst[0] = dw & 0xFF;
		dst[1] = (dw >> 8) & 0xFF;
	}

	return;
}

void BmpBitToC16Blt(BYTE *dst, BYTE *src, RGBQUAD *col, BYTE isBE)
{
	BYTE    *p;

	p = GetBmpBit(src, col);

	BGR24toRGB565(dst, p, isBE);

	return;
}

BYTE *BMPtoC16(BYTE *pix, BITMAPFILE *bm, BYTE isBE)
{
	BYTE    *p, *dst, *src;
	RGBQUAD *col;
	DWORD   x, y, padding, srcunit, dstunit;

	//printf("---- BMPtoC16()\n");

	dstunit = 2;	// RGB565

	// bitmap info
	if (bm->i == NULL) {
		printf("No bitmap info header.\n");
		return(NULL);
	}

	// color bit
	col = NULL;				     // 24-bit no color table
	if (bm->i->biBitCount == 8) {
		col = bm->c;	     // 8-bit 256 color table
	} else if (bm->i->biBitCount != 24) {
		printf("24-bit or 8-bit bitmap is required.\n");
		return(NULL);
	}

	// bit field
	if (bm->b == NULL) {
		printf("Bitmap buffer is NULL.\n");
		return(NULL);
	}
	src = bm->b;

	srcunit = bm->i->biBitCount >> 3;	// 24bit -> 3byte, 8bit -> 1byte
	padding = (bm->i->biWidth * srcunit) % 4;
	if (padding != 0) {
		padding = 4 - padding;
	}
	//printf("padding = %d\n", padding);

	// check dst buffer
	dst = pix;
	if (dst == NULL) {
		printf("Destination buffer is NULL.\n");
		return(NULL);
	}

	for(y=0; y < (DWORD)(bm->i->biHeight); y++) {
		if (isBE) {
			// for S1; change endian; upside down
			p = dst + (bm->i->biHeight - y - 1) * bm->i->biWidth * dstunit;
		} else {
			p = dst + y * bm->i->biWidth * dstunit;
		}
		for(x=0; x < (DWORD)(bm->i->biWidth); x++) {
			BmpBitToC16Blt(p, src, col, isBE);

			src += srcunit;
			p += dstunit;
		}
		src += padding;
	}

	return(dst);
}


//---- C16 pixel func ----//

typedef struct tagC16PIXRECT {
	DWORD	x;
	DWORD	y;
	DWORD	w;
	DWORD	h;
	WORD	*pix;
} C16PIXRECT;


void SetPixInfo(C16PIXRECT *pi, DWORD x, DWORD y, DWORD w, DWORD h, WORD *p)
{
	pi->x = x;
	pi->y = y;
	pi->w = w;
	pi->h = h;
	pi->pix = p;

	//printf(" SetPixInfo: (%d, %d) - (W%d, H%d) %08X\n", pi->x, pi->y, pi->w, pi->h, pi->pix);

	return;
}

void UpsidedownPix(C16PIXRECT *pi)
{
	DWORD	x, y, half;
	WORD	d, *src, *dst;

	//printf(" ---- UpsidedownPix()\n");

	half = pi->h / 2;

	for (y=0; y < half; y++) {
		src = pi->pix + y * pi->w;
		dst = pi->pix + (pi->h - 1 - y) * pi->w;
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

void Rotate180Pix(C16PIXRECT *pi)
{
	DWORD	x, y, half;
	WORD	d, *src, *dst;

	//printf(" ---- UpsidedownPix()\n");

	half = pi->h / 2;

	for (y=0; y < half; y++) {
		src = pi->pix + y * pi->w;
		dst = pi->pix + (pi->h - y) * pi->w - 1;
		for (x=0; x < pi->w; x++) {
			d = *src;
			*src = *dst;
			*dst = d;
			
			src++;
			dst--;
		}
	}

	return;
}

void Rotate270Pix(C16PIXRECT *dstpi, C16PIXRECT *srcpi, BYTE isBE)
{
	DWORD	x, y;
	WORD	*src, *dst;

	//printf(" ---- Rotate270Pix()\n");

	for (y=0; y < srcpi->h; y++) {
		src = srcpi->pix + y * srcpi->w;
		dst = dstpi->pix + (srcpi->w - 1) * srcpi->h + y;
		for (x=0; x < srcpi->w; x++) {
			if (isBE) {
				// endian change
				*dst = ((*src >> 8) & 0x00FF) | ((*src << 8) & 0xFF00);
			} else {
				*dst = *src;
			}

			src++;
			dst -= srcpi->h;
		}
	}

	dstpi->x = 0;
	dstpi->y = 0;
	dstpi->w = srcpi->h;
	dstpi->h = srcpi->w;

	return;
}

void Rotate180Vram(BYTE *buf)
{
	C16PIXRECT	pl;


	SetPixInfo(&pl, 0, 0, 320, 170, (WORD *)(buf));
	Rotate180Pix(&pl);

	return;
}

void Line170to240Vram(BYTE *buf)
{
	C16PIXRECT	pl;

	// top to bottom, align center; 170 + 35 = 205
	SetPixInfo(&pl, 0, 0, 320, 205, (WORD *)(buf));
	UpsidedownPix(&pl);

	return;
}


//[EOF]