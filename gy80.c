#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "gy80.h"
#include "HMC5883L.h"
#include "BMP085.h"

static int openI2CDev(int iDevNo)
{

	int iFd = -1;
	char acDevName[20];
	snprintf(acDevName, 19, "/dev/i2c-%d", iDevNo);

	iFd = open(acDevName, O_RDWR);

	if (iFd < 0)
	{
		fprintf(stderr, "unable to open device: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return iFd;
}

static int setI2CAddr(int iFd, int iAddr)
{
	if (ioctl(iFd, I2C_SLAVE, iAddr) < 0)
	{
		fprintf(stderr, "unable to set slave address: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	int iFd = -1;
	iFd = openI2CDev(DEV_NO);
	setI2CAddr(iFd, HMC5883L_ADDRESS);
	initHMC5883L(iFd);
	setI2CAddr(iFd, BMP085_ADDRESS);
	initBMP085(iFd);

	while(1)
	{
		setI2CAddr(iFd, HMC5883L_ADDRESS);
		readHMC5883L(iFd);
		sleep(1);
		setI2CAddr(iFd, BMP085_ADDRESS);
		readBMP085(iFd);
		sleep(1);
	}
}
