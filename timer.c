//---- timer and signal

void FreeEnv(void);

#ifdef WINDOWS

static UINT			uDelay;
static MMRESULT		uTimerID;

void CALLBACK BaseTimerProc(UINT uid, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (ThList[0].counter <= 0) {
		return;
	} else {

		ThList[0].counter--;

		switch (ThList[0].counter % COUNTER_PER_SEC) {
		case  0:
		case 16: 
		case 32:
		case 48: ThList[0].alarm = 1;	// wakeup main
			break;
		case 24: ThList[1].alarm = 1;	// wakeup disp
			break;
		case  8: ThList[2].alarm = 1;	// wakeup stat
			break;
		}
	}

	return;	
}

BOOL WINAPI SigIntProc(DWORD dwCtrlType)
{
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
		printf("\n catch CTRL_C.\n");
		break;
	case CTRL_SHUTDOWN_EVENT:
		printf("\n catch CTRL_SHUTDOWN.\n");
		break;

	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	default:
		return(FALSE);
	}

	ThList[0].counter = 0;	// try normal end
	return(TRUE);
}

void InitSignal()
{
	SetConsoleCtrlHandler(SigIntProc, TRUE);
}

int TimerControl(int isStart)
{
	if (isStart) {
		uDelay = TIME_INTERVAL_SEC * 1000 + TIME_INTERVAL_NANO / 1000000;
		uTimerID = timeSetEvent(uDelay, 1, BaseTimerProc, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		if (uTimerID == 0) {
			printf("timeSetEvent() failed.\n");
			return(3);
		}

       	printf("timer interval = %3.3f ms\n", ((float)(TIME_INTERVAL_NANO) / 1000000));
		ThList[0].time = time(NULL);

		printf("Main clock start at %ld, %s", ThList[0].time, ctime(&(ThList[0].time)));

	} else {

		printf("Main clock end at %ld, %s", ThList[0].time, ctime(&(ThList[0].time)));

		// stop timer
		timeKillEvent(uTimerID);
	}

	return(0);
}

#else

void BaseTimerProc(int signum)
{
	if (ThList[0].counter <= 0) {
		return;
	} else {

		switch(signum) {
		case SIGALRM:
			ThList[0].counter--;

			switch (ThList[0].counter % COUNTER_PER_SEC) {
			case  0:
			case 16: 
			case 32:
			case 48: ThList[0].alarm = 1;	// wakeup main
				break;
			case 24: ThList[1].alarm = 1;	// wakeup disp
				break;
			case  8: ThList[2].alarm = 1;	// wakeup stat
				break;
			}

			break;
		default:
			break;
		}
	}

	return;	
}

static struct itimerspec       tim, old_tim;
static struct sigaction        act, old_act;
static timer_t                 tid;

void SigIntProc(int sinnum)
{
	printf("\n catch SIGINT.\n");
	FreeEnv();

	exit(0);
}

int InitSignal()
{
	// handle SIGINT to call FreeEnv()
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = SigIntProc;
	if (sigaction(SIGINT, &act, NULL) == -1) {
		printf("sigaction() failed.\n");
		return(1);
	}

	return(0);
}

int TimerControl(int isStart)
{
	if (isStart) {
		// set alarm proc
		memset(&act, 0, sizeof(struct sigaction));
		act.sa_handler = BaseTimerProc;
		act.sa_flags = SA_RESTART;
		memset(&old_act, 0, sizeof(struct sigaction));
		if (sigaction(SIGALRM, &act, &old_act) == -1) {
			printf("sigaction() failed.\n");
			return(1);
		}

		// start timer
		if (timer_create(CLOCK_REALTIME, NULL, &tid) == -1) {
			printf("timer_create() failed.\n");
			return(2);
		}
		tim.it_value.tv_sec     = TIME_WAIT_SEC;
		tim.it_value.tv_nsec    = TIME_WAIT_NANO;
		tim.it_interval.tv_sec  = TIME_INTERVAL_SEC;
		tim.it_interval.tv_nsec = TIME_INTERVAL_NANO;
		if (timer_settime(tid, 0, &tim, &old_tim) == -1) {
			printf("timer_settime() failed.\n");
			return(3);
		}
		printf("timer interval = %3.3f ms\n", ((float)(tim.it_interval.tv_nsec) / 1000000));
		ThList[0].time = time(NULL);
		printf("Main clock start at %ld, %s", ThList[0].time, ctime(&(ThList[0].time)));

	} else {

		printf("Main clock end at %ld, %s", ThList[0].time, ctime(&(ThList[0].time)));

		// stop timer
		timer_delete(tid);
		sigaction(SIGALRM, &old_act, NULL);

	}

	return(0);
}

#endif


//[EOF]