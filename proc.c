//---- service process control

static char	LockFname[256];
static char	WakeupLogName[256];

void LoggingStopInfo()
{
	char	buf[256];
	int	fd_log;
	size_t	len;

	// logging wakeup history
	fd_log = open(WakeupLogName, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd_log > 0) {
		sprintf(buf, "%ld %d %d STOP\n", time(NULL), getpid(), getppid()); 
		len = write(fd_log, buf, strlen(buf));

		close(fd_log);
	}

	return;
}

#ifdef WINDOWS

static HANDLE	hMutex;
const wchar_t	*MutexName = L"diy-panel_running";

void ReleaseLock(void)
{
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return;
}

int isDuplicateRun(char *argv0)
{
	BYTE	buf[256];
	size_t	len;
	int		fd_log;
	int		ret;

	ret = 0;
	// logging wakeup history
	sprintf(WakeupLogName, "%s.wakeup", argv0);
	fd_log = open(WakeupLogName, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);

	hMutex = CreateMutex(NULL, FALSE, MutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		ret = 1;
	}

	if (ret == 0) {
		sprintf(buf, "%ld %d %d runnung\n", time(NULL), getpid(), getppid()); 
		len = write(fd_log, buf, strlen(buf));
	}

	close(fd_log);
	fd_log = -1;

	return(ret);
}

#else

void ReleaseLock(void)
{
	remove(LockFname);
	LockFname[0] = '\0';

	return;
}

int isDuplicateRun(char *argv0)
{
	BYTE		buf[256];
	size_t		len;
	int		fd_pid, fd_log;
	int		pid, ret;

	// open lock file
	ret = 0;
	sprintf(LockFname, "%s.pid", argv0);
	fd_pid = open(LockFname, O_RDONLY);
	if (fd_pid > 0) {
		// read pid from lock file
		len = read(fd_pid, buf, 256);
		close(fd_pid);
		fd_pid = -1;

		buf[len] = '\0';
		pid = atoi(buf);
		
		// check pid is valid(0) or not
		if ( kill(pid, 0) == 0 ) {
			ret = 1;		// service already running
		}
	}

	if (ret == 0) {				// 1st service
		// write own pid to lock file
		sprintf(buf, "%d", getpid());
		fd_pid = open(LockFname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd_pid > 0) {
			len = write(fd_pid, buf, (strlen(buf) + 1));
			close(fd_pid);
			fd_pid = -1;
		}

		// logging wakeup history
		sprintf(WakeupLogName, "%s.wakeup", argv0);
		fd_log = open(WakeupLogName, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd_log > 0) {
			sprintf(buf, "%ld %d %d runnung\n", time(NULL), getpid(), getppid()); 
			len = write(fd_log, buf, strlen(buf));
			close(fd_log);
			fd_log = -1;
		}
	}

	return(ret);
}

#endif

static BYTE StopFname[256];

int isStopRequest()
{
	struct stat	st;

	if (stat(StopFname, &st) == 0) {
		printf("detect stop request.\n");
		return(1);
	}

	return(0);
}


//[EOF]
