
// ---- sram function ----

typedef struct _VRAMCLIP {
	DWORD		szBuf;
	BYTE		*lpBuf;

	DWORD		len;
	BYTE		*p;		// temporary used
	BITMAPFILE	bm;

	BYTE		bPlane;		// flush plane to read/write
	BYTE		stat;
} VRAMCLIP;

static VRAMCLIP	VramClip;

DWORD ClearVramClip(VRAMCLIP *vc)
{
	// allocate info
	vc->lpBuf = VramBuf;
	vc->szBuf = VRAM_SIZE;
	memset(vc->lpBuf, 0, vc->szBuf);

	// clear variable
	vc->p	= vc->lpBuf;
	vc->len = 0;
	memset(&(vc->bm), 0, sizeof(BITMAPFILE));

	vc->bPlane = VramPlane(0);		// default

	vc->stat  = 0;					// 0:clear 1:reading 2:complete 3:flash stored

	return(vc->len);				// used by dwRead
}

DWORD AppendVramClip(VRAMCLIP *vc, BYTE *buf, DWORD size)
{
	vc->stat = 1;		// reading

	if (size > 0) {		// only set stat=1, if size=0.
		if (vc->len + size < vc->szBuf) {		// drop size-over data
			memcpy(vc->p, buf, size);
			vc->len += size;
			vc->p   += size;
		}
	}

	return( vc->len );
}

DWORD LoadVramClip(VRAMCLIP *vc, BYTE flash_plane)
{
	ClearVramClip(vc);

	FlashRead(flash_plane, vc->lpBuf);
	vc->stat = 3;	// complete

	return( VRAM_SIZE );
}

DWORD SaveVramClip(VRAMCLIP *vc, BYTE flash_plane)
{
	FlashWrite(flash_plane, vc->lpBuf);

	return( VRAM_SIZE );
}

DWORD Start_ReceiveBmp(VRAMCLIP *vc)
{
	vc->stat = 1;	// reading
}

DWORD Stop_ReceiveBmp(VRAMCLIP *vc)
{
	vc->stat = 2;	// complete
}

int isReceiveBmp(VRAMCLIP *vc)
{
	if (vc->stat == 1) {
		return(1);
	}

	return(0);
}

int isCompleteBmp(VRAMCLIP *vc)
{
	if (vc->len < sizeof(BITMAPFILEHEADER)) {
		return(0);
	}

	if ((vc->lpBuf[0] != 'B') || (vc->lpBuf[1] != 'M')) {
		printf("Not bitmap data.(%02X %02X); stored %ld bytes\n", vc->lpBuf[0], vc->lpBuf[1], vc->len);
		return(-1);
	}

	vc->bm.f = vc->lpBuf;
	//printf("Received %ld / %ld bytes.\n", vc->len, vc->bm.f->bfSize);

	if (vc->bm.f->bfSize > vc->szBuf) {
		printf("bmp size %ld exceeds the buffer size %ld; invalidate the bitmap header.\n", vc->bm.f->bfSize, vc->szBuf);
		vc->lpBuf[1] = 'm';			// BM -> Bm
	}

	if (vc->len < vc->bm.f->bfSize) {
		vc->stat = 1;		// reading
		return(0);			// not complete
	}
	vc->stat = 2;			// complete

	AttachBitmapStruct(&(vc->bm), vc->lpBuf);
	if ((vc->bm.i->biWidth == LCDC_W) && (vc->bm.i->biHeight == LCDC_H)) {
		return(1);	// complete vram compatible bitmap
	}
	return(2);		// complete other bitmap
}

//[EOF]
