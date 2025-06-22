#include <stdio.h>

#ifdef _WIN32
  #define WINDOWS
#endif

#ifdef WINDOWS
  #include <windows.h>
#else
  #include <stdlib.h>	// atoi
  #include <unistd.h>	// write usleep
  #include <sys/ioctl.h>	// TCSETS
  #include <fcntl.h>	// open 

  typedef unsigned char	BYTE;
  typedef unsigned long	DWORD;
#endif

#include "com.c"

void PrintUsage(char *argv[])
{
	printf("Usage: %s [param]\n", argv[0]);
	printf("  param = mode(1-5) * 100 + luminance(1-5) * 10 + speed(1-5)\n");
	printf("  ex.) %s 133\n", argv[0]);
	printf("\n");
	printf("  mode:         1:rainbow 2:breathing 3:color-cycle 4:off 5:automatic\n");
	printf("  luminance:    1:bright - 5:dark\n");
	printf("  speed:        1:fast   - 5:slow\n");

	return;
}

int main(int argc, char *argv[], char *envp[])
{
	DWORD	level;
	BYTE	mode, luminance, speed;
	int		ret;	

	if (argc < 2) {
		PrintUsage(argv);
		return(0);
	}

	ret = 0;
	level = atoi(argv[1]);

	if ((level >= 111) && (level <= 555)) {
		speed = (BYTE)(level % 10);
		level /= 10;
		luminance = (BYTE)(level % 10);
		level /= 10;
		mode = (BYTE)level;

		ret = SetLedLevel(mode, luminance, speed);

		if (ret == 0) {
			printf("set LED mode=%d, luminance=%d, speed=%d\n", mode, luminance, speed);
		} else if (ret < 4) {
			printf("illegal parameter: %ld\n", level);
		} else {
			printf("handle serial port failed.\n");
		}
	} else {
		printf("illegal parameter: %s\n", argv[1]);
	}

	return(ret);
}
