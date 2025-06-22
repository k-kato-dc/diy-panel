//---- modules

// common envronment
#include "mytype.h"
#include "logging.c"

// input status
#include "myconst.h"
#include "stat.c"

// screen image
#include "bitmap.c"
#include "pix_color.c"
#include "snap.c"
#include "pix_parts.c"
#include "pix_plot.c"

// output device
static int fDEBUG = 0;	// for pico.c
#include "pico.c"
#include "hid.c"

// service process
#include "thread.c"
#include "timer.c"
#include "proc.c"

//---- initialize

void InitEnv()
{
	printf("---- InitEnv()\n");

	printf("  PanelParts\n");
	InitPanelParts();
	InitSnapFile();

	printf("  Hid\n");
	InitHidEnv();

	printf("  Timer\n");
	TimerControl(1);
	printf("  Thread\n");
	ThreadControl(1);

	InitSignal();

	return;
}

void FreeEnv()
{
	printf("---- FreeEnv()\n");

	printf("  Timer\n");
	TimerControl(0);
	printf("  Thread\n");
	ThreadControl(0);

	printf("  Pico\n");
	Sleep(100);		// wait for sending ^D
	PicoCloseCom();

	printf("  Hid\n");
	FreeHidEnv();

	printf("  PanelParts\n");
	FreeSnapFile();
	FreePanelParts();

	LoggingStopInfo();

	printf("...end service.\n");
	ReleaseLock();
	ChangePrintfToFile(NULL,0);

	return;
}

//---- main loop

int main(int argc, char *argv[], char *envp[])
{
	THREAD_DATA	*th;

	// check other instance	
	if (isDuplicateRun(argv[0])) {
		return(1);
	}

	// check stop request
	sprintf(StopFname, "%s.STOP", argv[0]);
	if ( isStopRequest() ) {
		return(2);
	}

	// logging stdout to file
	ChangePrintfToFile(NULL,1);
	if (argc == 2) {
		if(strcmp(argv[1], "debug") == 0) {
			ChangePrintfToFile(argv[0],1);
			fDEBUG = 1;		// for pico.c
		}
	}

	printf("start service...  stop request file is %s\n", StopFname);
	printf("main: pid=%d, ppid=%d, tid=%ld\n", getpid(), getppid(), syscall(SYS_gettid));

	InitEnv();

	// main loop
	th = &(ThList[0]);
	th->counter = COUNTER_PER_SEC * 3600 * 24;	// 1 day, process life
	th->interval = 125000;			// 125ms, wakeup cycle

	th->alarm = 0;
	while (th->counter > 0) {

		if ( isStopRequest() ) {	// user create file named $0.STOP
			break;
		}

		if (th->alarm) {			// may be 250ms
			th->alarm = 0;

			// date time
			th->time = time(NULL);	
			localtime_r(&(th->time), &(th->tm));
		}

		usleep(th->interval);
	}

	FreeEnv();
	return(0);
}


//[EPF]
