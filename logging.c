
//---- first stting; debug logging

static int	FD_PRINTF;
static char	StdoutFname[256];
static char	StdoutBuf[4096];

#define printf(...)	{ int ret; if (FD_PRINTF > 0) { memset(StdoutBuf, 0, 4096); sprintf(StdoutBuf, __VA_ARGS__); ret = write(FD_PRINTF, StdoutBuf, strlen(StdoutBuf)); } }

void ChangePrintfToFile(char *argv0, int not_close)
{
	if ( argv0 == NULL) {
		//printf("---- stop logging\n");
		if ((FD_PRINTF > 0) && (not_close == 0)) {
			close(FD_PRINTF);
		}
		FD_PRINTF = -1;
		return;
	}

	sprintf(StdoutFname, "%s.stdout", argv0); 
	FD_PRINTF = open(StdoutFname, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);

	printf("---- Start logging %s.stdout at %ld\n", argv0, time(NULL));

	return;
}


//[EOF]
