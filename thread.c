
//---- thread

#ifdef WINDOWS

//typedef unsigned long	pthread_t;
typedef HANDLE					pthread_t;
#define pthread_create(hTh, pAttr, pFunc, pArg)		(int)(*(hTh) = CreateThread((pAttr), 0, (LPTHREAD_START_ROUTINE)(pFunc), (pArg), 0, NULL))
#define pthread_join(hTh, ret)						(WaitForSingleObject(hTh, 10000))

#define getpid()				GetCurrentProcessId()
#define getppid()				(0xFFFFFFFF)
#define syscall(SYS_gettid)		GetCurrentThreadId()

#define usleep(us)				Sleep((us)/1000)

#else

#include <pthread.h>
#include <sys/syscall.h>

#endif

#define COUNTER_PER_SEC		64				
#define TIME_WAIT_SEC		1
#define TIME_WAIT_NANO		0
#define TIME_INTERVAL_SEC	0
#define TIME_INTERVAL_NANO	(1000000000 / COUNTER_PER_SEC)

typedef struct tagTHREAD_DATA {
	pthread_t	pthread;
	void		*(*func)(void *arg);
	int		thread_arg;
	int		ret;

	int		alarm;
	int		runmode;
	int		interval;
	int		counter;

	time_t		time;
	time_t		prev_time;
	struct tm	tm;
} THREAD_DATA;

static THREAD_DATA	ThList[16];

void GetTime3Byte(BYTE *buf)	// hid.c
{
	buf[0] = ThList[0].tm.tm_hour & 0xFF;
	buf[1] = ThList[0].tm.tm_min & 0xFF;
	buf[2] = ThList[0].tm.tm_sec & 0xFF;
}

// ---- screen update  ----

void *thread_func1(void *arg)
{
	THREAD_DATA	*td;
	int		thread_arg, myid, sec;
	BYTE	*pix;

	thread_arg = *((int *)arg);
	myid = 1;
	td = &(ThList[myid]);
	printf("thread[%d]: start arg=%d pid=%d, ppid=%d, tid=%ld\n", myid, thread_arg, getpid(), getppid(), syscall(SYS_gettid));

	sec = 5;

	td->interval = 50000;	// 50ms
	td->counter = 0;
	td->alarm = 0;
	while (td->runmode) {

		switch (td->alarm) {
		case 1: td->alarm = 0;
			td->prev_time = ThList[0].counter;

			if (td->counter % sec == 0) {
				// create bitmap
				pix = GetPanelPtr();

				HidAction(8, pix);	// send vram
				PicoAction(0, pix);	// send vram
			} else {
				HidAction(3, NULL);	// heart beat
				PicoAction(1, NULL);	// mode change
			}

			td->time = ThList[0].counter;

			if (td->time != td->prev_time) {
				printf("thread-%d: took %ld ms", myid, (td->prev_time - td->time) * (TIME_INTERVAL_NANO / 1000000));
				//printf(" (%d - %d = %d count)", td->prev_time, td->time, (td->prev_time - td->time));
				printf("\n");
			}

			td->counter++;
			break;
		}

		// clear verbose alarm
		td->alarm = 0;

		usleep(td->interval);		// 50ms
	}

	printf("thread[%d]: end.\n", myid);
	return(0);
}

// ---- status file polling  ----

void *thread_func2(void *arg)
{
	THREAD_DATA	*td;
	int     	thread_arg, myid;

	thread_arg = *((int *)arg);
	myid = 2;
	td = &(ThList[myid]);
	printf("thread[%d]: start arg=%d pid=%d, ppid=%d, tid=%ld\n", myid, thread_arg, getpid(), getppid(), syscall(SYS_gettid));

	InitStatFile();

	td->alarm = 0;
	td->interval = 50000;
	td->counter = 0;
	while (td->runmode) {
		switch (td->alarm) {
		case 1:
			td->alarm = 0;
			td->prev_time = ThList[0].counter;

			if (td->counter % 3 == 0) {
				GetStatFileVal1();
			}

			td->time = ThList[0].counter;

			if (td->time != td->prev_time) {
				printf("thread-%d: took %ld ms", myid, (td->prev_time - td->time) * (TIME_INTERVAL_NANO / 1000000));
				//printf(" (%d - %d = %d count)", td->prev_time, td->time, (td->prev_time - td->time));
				printf("\n");
			}

			td->counter++;
			break;
		}
		
		usleep(td->interval);          // 50ms
	}

	FreeStatFile();

	printf("thread[%d]: end.\n", myid);
	return(0);
}

// ---- create and join  ----

int ThreadControl(int isStart)
{
	int	i;

	if (isStart) {
		memset((BYTE *)ThList, 0, sizeof(THREAD_DATA) * 16);
		ThList[1].func = &thread_func1;
		ThList[2].func = &thread_func2;

		for (i=0; i < 16; i++) {
			if (ThList[i].func != NULL) {
				// start thread
				printf("main: create thread %d.\n", i);
				ThList[i].thread_arg = i;
				ThList[i].runmode = 1;
				ThList[i].ret = pthread_create(&(ThList[i].pthread), NULL, (ThList[i].func), &(ThList[i].thread_arg));
			}
		}
	} else {

		for (i=0; i < 16; i++) {
			if (ThList[i].func != NULL) {
				// stop thread 
				ThList[i].runmode = 0;
				ThList[i].ret = pthread_join(ThList[i].pthread, NULL);
				printf("main: join thread %d complete.\n", i);
			}
		}
	}

	return(0);
}

//[EOF]
