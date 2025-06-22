
//---- status file ----//

#define STATFILE_NUM		8
#define STATFILE_SIGNAL		0
#define STATFILE_PSTAT		1
#define STATFILE_DSTAT		2
#define STATFILE_PICO_CONFIG	6
#define STATFILE_PANEL_CONFIG	7

#define STATFILE_NAMELEN	256
#define STATFILE_BUFSIZE	80
#define STATFILE_VALNUM		16

static BYTE	STATFNAME[STATFILE_NUM][STATFILE_NAMELEN] = {
	SCRFILEPATHEX("pstat", ".signal"),
	SCRFILEPATHEX("pstat", ".current"),
	SCRFILEPATHEX("dstat", ".current"),
	"\0",
	"\0",
	"\0",
	MODFILEPATH("panel", "pico.config"),
	MODFILEPATH("panel", "panel.config") };

typedef struct tagSTAT_FILE {
	BYTE		fname[STATFILE_NAMELEN];
	struct stat	st;
	int			fd;
	int			size;
	int			read;
	BYTE		buf[STATFILE_BUFSIZE];

	DWORD		time;
	BYTE		val[STATFILE_VALNUM];
} STAT_FILE;

static STAT_FILE	StatFile[STATFILE_NUM];

#define GetPaneOrientation()	(StatFile[7].val[0] & 0x0001)
#define GetPaneOrientationRaw()	(StatFile[7].val[0])
#define GetPaneColor(n)			(StatFile[7].val[1 + (n)])

void InitStatFile()
{
	STAT_FILE	*sf;
	DWORD		i;

	for (i=0; i < STATFILE_NUM; i++) {
		sf = &(StatFile[i]);

		memset((BYTE *)sf, 0, sizeof(STAT_FILE));

		if (STATFNAME[i][0] == '\0') {
			continue;
		}

		memcpy(sf->fname, STATFNAME[i], strlen(STATFNAME[i]));
		if (stat(sf->fname, &(sf->st)) != 0) {
			printf("stat file not exist.(%s)\n", sf->fname);
			sf->fd = -1;
			continue;
		}

		sf->fd = open(sf->fname, O_RDONLY);
		if (sf->fd < 0) {
			printf("stat file open error.(%s)\n", sf->fname);
			continue;
		}

		sf->st.st_mtime = 0;	// clear time for 1st read
	}

	return;
}

void FreeStatFile()
{
	STAT_FILE	*sf;
	DWORD		i;

	for (i=0; i < STATFILE_NUM; i++) {
		sf = &(StatFile[i]);

		if (sf->fd > 0) {
			close(sf->fd);
			sf->fd = -1;
		}
		sf->fname[0] = '\0';
	}

	return;
}

void PrintStatFileVal()
{
	STAT_FILE	*sf;
	DWORD		i, j;

	for (j=0; j < STATFILE_NUM; j++) {
		sf = &(StatFile[j]);

		if (sf->fname[0] != '\0') {
			printf("status val[%d]: %d", j, sf->time);
			for (i=0; i < 16; i++) {
				printf(" %03d", sf->val[i]);
			}
			printf("\n");
		} 
	}

	return;
}

BYTE ClipStatVal(BYTE filenum, BYTE valnum, BYTE min, BYTE max)
{
	BYTE	val;

	val = StatFile[filenum].val[valnum];
	if (min > 0) {
		if (val < min) {
			val = min;
		}
	} else if (max < 255) {
		if (val > max) {
			val = max;
		}
	}

	return(val);
}

void RotatePaneColor()
{
	STAT_FILE	*sf;

	sf = &(StatFile[STATFILE_PANEL_CONFIG]);
	if (sf->val[3] != 1) {
		return;		// not rotate mode
	}

	if (sf->val[1] == 7) {
		sf->val[1] = 0;				// return to black
	} else {
		sf->val[1]++;
	}
	sf->val[2] = sf->val[1];			// same color, left and right pane

	printf("demo: rotate color = %d\n", sf->val[1]);
	return;
}

void C16PixPlotUpdate(void);

void GetStatFileVal1()
{
	struct stat	st_prev;
	STAT_FILE	*sf;
	DWORD		i, j, k, val;
	BYTE		*p;

	for(k=0; k < STATFILE_NUM; k++) {
		sf = &(StatFile[k]);

		if (sf->fd < 0) {		// file not exist
			continue;
		}

		memcpy(&st_prev, &(sf->st), sizeof(struct stat)); 
		stat(sf->fname, &(sf->st));
		if (sf->st.st_mtime != st_prev.st_mtime) {
/**/
			if (sf->fd > 0) {
				close(sf->fd);
			}
			sf->fd = open(sf->fname, O_RDONLY);
			if (sf->fd < 0) {
				printf("stat file open error.(%s)\n", sf->fname);
				continue;
			}
/**/
			lseek(sf->fd, 0, SEEK_SET);
			sf->read = read(sf->fd, sf->buf, STATFILE_BUFSIZE);
			//printf("read stat[%d] %d byte\n", k, sf->read);
		} else {
			continue;
		}

		// process numeric value
		p = sf->buf;
		val = 0;
		for(j=0; j < 10; j++) {
			val *= 10;
			val += p[j] - '0';
		}
		sf->time = (DWORD)val;
		p += 11;
 
		for (j=0; j < STATFILE_VALNUM; j++) {
			val = 0;
			for(i=0; i < 3; i++) {
				val *= 10;
				val += p[i] - '0';
			}
			sf->val[j] = (BYTE)val;

			p += 4;			// 3 digit + 1 space

			if (sf->buf + sf->read <= p) {
				break;
			}
		}

		switch (k) {
		case STATFILE_DSTAT:
			C16PixPlotUpdate();
			break;
		} 
	}

	PrintStatFileVal();
	return;
}


//[EOF]

