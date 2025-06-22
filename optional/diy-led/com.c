//---- S1LED

#define S1COM_BAUDRATE		10000
#define S1COM_DATA_SIZE		5
  static BYTE				S1ComBuf[S1COM_DATA_SIZE + 1];


#ifdef WINDOWS

#define S1COM_NAME			L"COM3"
  static HANDLE				hS1Com = INVALID_HANDLE_VALUE;

int S1OpenCom()
{
	DCB		dcb;
	BOOL	fSuccess;

	hS1Com = INVALID_HANDLE_VALUE;
	hS1Com = CreateFile(S1COM_NAME, GENERIC_READ|GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
	if (hS1Com == INVALID_HANDLE_VALUE) {
		//printf("open error: %s\n", S1COM_NAME);
		return(1);
	}
    //printf("open %s, hS1Com = %ld\n", S1COM_NAME, hS1Com);

	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	fSuccess = GetCommState(hS1Com, &dcb);

	dcb.BaudRate = S1COM_BAUDRATE;	// Ch340 com3
	dcb.ByteSize = 8;
	dcb.Parity   = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	fSuccess = SetCommState(hS1Com, &dcb);
	if (!fSuccess) {
		//printf("SetComState failed.(%04X)\n", GetLastError());
		return(2);
	}

	return(0);
}

void S1CloseCom()
{
	if (hS1Com != INVALID_HANDLE_VALUE) {
		CloseHandle(hS1Com);

		hS1Com = INVALID_HANDLE_VALUE;
		//printf("Close hS1Com = %ld\n", hS1Com);
	}

	return;
}

void S1SendCom()
{
	DWORD	i, dwLen, dwLenTotal;

	dwLenTotal = 0;
	for(i=0; i < S1COM_DATA_SIZE; i++) {
		dwLen = 0;
		WriteFile(hS1Com, S1ComBuf+i, 1, &dwLen, 0);

		dwLenTotal += dwLen;
		Sleep(5);	// send wait
	}

	return;
}

#else

#include <termios.h>
#define S1COM_NAME			"/dev/ttyUSB0"
  static int				hS1Com = -1;

int S1OpenCom(void)
{
	struct termios	tio;

    	hS1Com = open(S1COM_NAME, O_RDWR);
    	if (hS1Com < 0) {
		//printf("open error: %s\n", S1COM_NAME);
        	return -1;
    	}
	//printf("open %s, hS1Com = %ld\n", S1COM_NAME, hS1Com);

	//tio.c_cflag += CREAD;	// enable read
	tio.c_cflag += CLOCAL;	// no modem control
	//tio.c_cflag += CS8;		// 8bit
	//tio.c_cflag += 0;		// 1 stop bit
	//tio.c_cflag += 0;		// parity None

	//cfsetspeed( &tio, B9600 );
	cfsetispeed( &tio, S1COM_BAUDRATE );
	cfsetospeed( &tio, S1COM_BAUDRATE );

	cfmakeraw(&tio);					// RAW mode
	tcsetattr(hS1Com, TCSANOW, &tio);	// set
	ioctl(hS1Com, TCSETS, &tio);		// set

	return 0;
}

void S1CloseCom()
{
	if (hS1Com > 0) {
		close(hS1Com);

		hS1Com = -1;
		printf("Close hS1Com = %d\n", hS1Com);
	}

	return;
}

void S1SendCom()
{
	DWORD	i, dwLen, dwLenTotal;
	int	ret;

	if (S1OpenCom() != 0) {
		return;
	}

	dwLenTotal = 0;
	for(i=0; i < S1COM_DATA_SIZE; i++) {
		dwLen = 0;
        	dwLen = write(hS1Com, S1ComBuf+i, 1);

		dwLenTotal += dwLen;
		usleep(5000);	// send wait
	}
	printf("Com send dwLenTotal = %ld\n", dwLenTotal);

	S1CloseCom();

}

#endif


//NOTE:	mode:		1:rainbow 2:breathing 3:color-cycle 4:off 5:automatic
//		luminance:	1:bright - 5:dark
//		speed:		1:fast   - 5:slow

#define CheckLedParam(val)	( (((val) < 1) || ((val) > 5)) ? 1 : 0 )

int SetLedLevel(BYTE mode, BYTE luminance, BYTE speed)
{
	int ret;

	ret = 0;
	ret += CheckLedParam(mode);
	ret += CheckLedParam(luminance);
	ret += CheckLedParam(speed);

	if (ret	== 0) {
		if (S1OpenCom() == 0) {

			S1ComBuf[0] = 0xfa;
			S1ComBuf[1] = mode;	//mode
			S1ComBuf[2] = luminance;	//light;
			S1ComBuf[3] = speed;	//speed;
			S1ComBuf[4] = (S1ComBuf[0] + S1ComBuf[1] + S1ComBuf[2] + S1ComBuf[3]) & 0xFF;
			S1ComBuf[5] = 0x0;

			S1SendCom();
			S1CloseCom();
		} else {
			ret += 4;
		}
	}

	return(ret);
}

//[EOF]
