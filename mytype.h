#include <stdio.h>
#include <stdlib.h>	// exit
#include <memory.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
	#define WINDOWS
	//#define _CRT_SECURE_NO_WARNINGS
	__pragma(warning(push))
	__pragma(warning(disable:4996))		// unsafe: sprintf _open
	__pragma(warning(disable:4244))		// data lost: DWORD to BYTE
#endif

#ifdef WINDOWS

#include <windows.h>
#include <io.h>	 // open close

//#include <fcntl.h>
#define open	    _open
#undef O_RDONLY	 // fctrl.h
#define O_RDONLY	_O_RDONLY | _O_BINARY
#undef O_WRONLY	 // fctrl.h
#define O_WRONLY	_O_WRONLY | _O_BINARY
#define O_APPEND	_O_APPEND
#define O_CREAT	 _O_CREAT
#define O_TRUNC	 _O_TRUNC
#define close	   _close
#define lseek	   _lseek
#define read	    _read
#define write	   _write

//#include <sys/stat.h>
#define S_IRUSR	 _S_IREAD
#define S_IWUSR	 _S_IWRITE

#include <conio.h>	// _kbhit, _getch

//#include <time.h>
#define localtime_r(pt, ptm)	_localtime32_s((ptm), ((__time32_t *)pt))

typedef unsigned long long	QWORD;

#else

#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>	// clock_gettime
#include <termios.h>
#include <sys/ioctl.h>	// ICANON
#include <string.h>	// memcpy
#include <sys/select.h>

#define _stat   stat	// avoid "struct stat"

//#define Sleep(ms)	usleep(((ms) * 1000))
void Sleep(int ms)
{
	//printf("sleep %d ms\n", ms);
	usleep(ms * 1000);
	return;
}

typedef uint8_t		BYTE;
typedef uint16_t	WORD;
typedef uint32_t	DWORD;
typedef int32_t		LONG;
typedef uint64_t	QWORD;

#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD   biSize;
	LONG    biWidth;
	LONG    biHeight;
	WORD    biPlanes;
	WORD    biBitCount;
	DWORD   biCompression;
	DWORD   biSizeImage;
	LONG    biXPelsPerMeter;
	LONG    biYPelsPerMeter;
	DWORD   biClrUsed;
	DWORD   biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD		 bmiColors[1];
} BITMAPINFO;
//#pragma pack(8) //default
#pragma pack(pop)

#endif


//[EOF]
