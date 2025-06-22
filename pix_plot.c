
//---- C16 pixel func ----//

void C16PixOverlay(C16PIXRECT *dst, C16PIXRECT *src)
{
	DWORD   i, col, line;
	WORD    *dp, *sp;

	//printf("---- C16PixOverlay\n");

	line = dst->h - dst->y;
	if (src->h < line) {
		line = src->h;
	}

	col = dst->w - dst->x;
	if (src->w < col) {
		col = src->w;
	}
	col *= sizeof(WORD);

	sp = src->pix + src->y * src->w + src->x;
	dp = dst->pix + dst->y * dst->w + dst->x;

	for (i=0; i < line; i++) {
		memcpy(dp, sp, col);

		sp += src->w;
		dp += dst->w;
	}

	return;
}

/**
void C16PixOverlayiWithTransparency(C16PIXRECT *dst, C16PIXRECT *src, WORD bgcol)
{
	DWORD   i, j, col, line;
	WORD    *dp, *sp;

	line = dst->h - dst->y;
	if (src->h < line) {
		line = src->h;
	}

	col = dst->w - dst->x;
	if (src->w < col) {
		col = src->w;
	}
	col *= sizeof(WORD);

	sp = src->pix + src->y * src->w + src->x;
	dp = dst->pix + dst->y * dst->w + dst->x;

	for (j=0; j < line; j++) {
		for (i=0; i < col; i++) {
			if (sp[i] != bgcol) {
				dp[i] = sp[i];
			}
			//memcpy(dp, sp, col);
		}
		sp += src->w;
		dp += dst->w;
	}

	return;
}
**/


// ---- pix plot func ---- //

void C16PixScrollLeft(C16PIXRECT *dst, DWORD width)
{
	DWORD	i, j, count;
	WORD	*dp;

	//printf("---- C16PixScrollLeft\n");

	count = dst->w - width;

	dp = dst->pix;
	for (j=0; j < dst->h; j++) {
		// move left
		for (i=0; i < count; i++) {
			dp[i] = dp[i + width];
		}

		// black padding
		for (i=count; i < dst->w; i++) {
			dp[i] = 0x0000;
		}

		dp += dst->w;
	}

	return;
}

void C16PixPlotRight(C16PIXRECT *dst, BYTE col, BYTE level, DWORD dot, BYTE *bg)
{
	DWORD	i, j, k, lv, lvmax;
	WORD	*dp, *sp;

	//printf("---- C16PixPlotRight\n");

	lv = (DWORD)level;
	lvmax = (dst->h / dot) - 1;

	for (j=0; j <= lvmax; j++) {
		if (j <= lv) {
			sp = (WORD *)(pPixCol(col, 3 + j));
		} else if (bg == NULL) {
			break;
		} else {
			sp = (WORD *)(bg);
		}

		for (i=0; i < dot; i++) {
			dp = dst->pix + ((lvmax - j) * dot + i) * dst->w + (dst->w - dot);
			for (k=0; k < dot; k++) {
				memcpy(dp + k, sp, sizeof(WORD));
			}
		}
	}

	return;
}

void C16PixPlotUpdate()
{
	static BYTE	col[8] = { 4, 5, 2, 2, 2, 3, 3, 3 };

	C16PIXRECT	dst;
	DWORD		i, rect, dot;
	BYTE		*bg;

	printf("---- C16PixPlotUpdate\n");

	rect = 40;
	dot  = 4;
	bg   = pPixCol(0, 3);
	//bg   = pPixCol(0, 5);
	//bg   = NULL;

	dst.x = 0;
	dst.y = 0;
	dst.w = rect;
	dst.h = rect;

	for (i=0; i < 8; i++) {
		dst.pix = (WORD *)(pPixPlot40(i));
		C16PixScrollLeft(&dst, dot);
		C16PixPlotRight(&dst, col[i], (StatFile[STATFILE_DSTAT].val[i] / 10), dot, bg);
	}

	return;
}

void C16PlotOverlay(BYTE *dstpix)
{
	C16PIXRECT	src, dst;
	DWORD		startx, starty, stepy, stepx;

	startx = 166;
	starty = 20;
	stepx  = 54;
	stepy  = 53;

	src.x = 0;
	src.y = 0;
	src.w = 40;
	src.h = 40;
	src.pix = (WORD *)(pPixPlot40(0));

	dst.x = startx;
	dst.y = starty;
	dst.w = 320;
	dst.h = 170;
	dst.pix = (WORD *)dstpix;

	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(1));
	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(2));
	dst.x = startx;
	dst.y += stepy;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(3));
	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(4));
	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(5));
	dst.x = startx;
	dst.y += stepy;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(6));
	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	src.pix = (WORD *)(pPixPlot40(7));
	dst.x += stepx;
	C16PixOverlay(&dst, &src);

	return;
}


//[EOF]
