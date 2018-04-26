#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include "i2c-dev-user.h"
#include "BMP085.h"
#include "gy80.h"

static int16_t ac1 = 0;
static int16_t ac2 = 0;
static int16_t ac3 = 0;
static int16_t b1 = 0;
static int16_t b2 = 0;
static int16_t mb = 0;
static int16_t mc = 0;
static int16_t md = 0;
static u_int16_t ac4 = 0;
static u_int16_t ac5 = 0;
static u_int16_t ac6 = 0;

static int32_t b5 = 0;

//Read calibration data from the E2PROM of the BMP085
static int loadCalibration(int iFd)
{
    u_int8_t uiaBuf[22];

    if (i2c_smbus_read_i2c_block_data(iFd, BMP085_RA_AC1_H, 22, uiaBuf) < 0)
    {
    	fprintf(stderr, "BMP085_RA registers read error: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

    ac1 = ((int16_t)  uiaBuf[0]  << 8) + uiaBuf[1];
    ac2 = ((int16_t)  uiaBuf[2]  << 8) + uiaBuf[3];
    ac3 = ((int16_t)  uiaBuf[4]  << 8) + uiaBuf[5];
    ac4 = ((u_int16_t)uiaBuf[6]  << 8) + uiaBuf[7];
    ac5 = ((u_int16_t)uiaBuf[8]  << 8) + uiaBuf[9];
    ac6 = ((u_int16_t)uiaBuf[10] << 8) + uiaBuf[11];
    b1  = ((int16_t)  uiaBuf[12] << 8) + uiaBuf[13];
    b2  = ((int16_t)  uiaBuf[14] << 8) + uiaBuf[15];
    mb  = ((int16_t)  uiaBuf[16] << 8) + uiaBuf[17];
    mc  = ((int16_t)  uiaBuf[18] << 8) + uiaBuf[19];
    md  = ((int16_t)  uiaBuf[20] << 8) + uiaBuf[21];

    return EXIT_SUCCESS;
}

//read uncompensated temperature value
static u_int32_t readUT(int iFd)
{
	u_int32_t iUTemp = 0;

    if (i2c_smbus_write_byte_data(iFd, BMP085_RA_CONTROL, BMP085_MODE_TEMPERATURE) < 0)
    {
    	fprintf(stderr, "BMP085_MODE_TEMPERATURE register write error: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

    // wait at least 4.5ms
    usleep(WAIT_50MS);


    if ((iUTemp = i2c_smbus_read_word_data(iFd, BMP085_RA_MSB)) < 0)
    {
    	fprintf(stderr, "BMP085_RA_MSB_LSB register read error: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

	// Convert result to 16 bits and swap bytes
    return ((iUTemp<<8) & 0xFF00) | ((iUTemp>>8) & 0xFF);
}

//read uncompensated pressure value
static u_int32_t readUP(int iFd)
{
	unsigned char ucaVal[3] = {0};

    if (i2c_smbus_write_byte_data(iFd, BMP085_RA_CONTROL, (BMP085_MODE_PRESSURE_0 + (BMP085_OVERSAMPLING_SETTING << 6))) < 0)
    {
    	fprintf(stderr, "BMP085_MODE_PRESSURE_0 register write error: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

    // wait at least 4.5ms
    usleep(WAIT_50MS);

    if (i2c_smbus_read_i2c_block_data(iFd, BMP085_RA_MSB, 3, ucaVal) < 0)
    {
    	fprintf(stderr, "BMP085_RA_MSB_LSB register read error: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }

    return ((u_int32_t)ucaVal[0] << 16 | (u_int32_t)ucaVal[1] << 8 | (u_int32_t)ucaVal[2]) >> (8-BMP085_OVERSAMPLING_SETTING);
}

// Calculate temperature given uncalibrated temperature
// Value returned will be in units of 0.1 deg C
static u_int32_t getTemperature(u_int32_t iUT)
{
	int x1, x2;

	x1 = (((int)iUT - (int)ac6)*(int)ac5) >> 15;
	x2 = ((int)mc << 11)/(x1 + md);
	b5 = x1 + x2;

	unsigned int result = ((b5 + 8)>>4);

	return result;
}

// Calculate pressure given uncalibrated pressure
// Value returned will be in units of XXXXX
static u_int32_t getPressure(u_int32_t uiUP)
{
	int x1, x2, x3, b3, b6, p;
	unsigned int b4, b7;

	b6 = b5 - 4000;
	// Calculate B3
	x1 = (b2 * (b6 * b6)>>12)>>11;
	x2 = (ac2 * b6)>>11;
	x3 = x1 + x2;
	b3 = (((((int)ac1)*4 + x3)<<BMP085_OVERSAMPLING_SETTING) + 2)>>2;

	// Calculate B4
	x1 = (ac3 * b6)>>13;
	x2 = (b1 * ((b6 * b6)>>12))>>16;
	x3 = ((x1 + x2) + 2)>>2;
	b4 = (ac4 * (unsigned int)(x3 + 32768))>>15;

	b7 = ((unsigned int)(uiUP - b3) * (50000>>BMP085_OVERSAMPLING_SETTING));
	if (b7 < 0x80000000)
		p = (b7<<1)/b4;
	else
		p = (b7/b4)<<1;

	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038)>>16;
	x2 = (-7357 * p)>>16;
	p += (x1 + x2 + 3791)>>4;

	return p;
}

// This Altitude part is stolen from some unknown Arduino library.
// The number divided into pressure for float A is derived from
// the local pressure as explained at
// http://learn.adafruit.com/bmp085/using-the-bmp085.
static u_int32_t getAltitude(float pressure)
{
	float A = pressure/101794.58;
	float B = 1/5.25588;
	float C = pow(A,B);
	C = 1 - C;
	C = C / 0.0000225577;

	return C;
}

void initBMP085(int iFd)
{
	printf("BMP085 sensor is initializing...\n");
	loadCalibration(iFd);
}
void readBMP085(int iFd)
{
	u_int32_t uiUP = readUP(iFd);
	u_int32_t uiPress = getPressure(uiUP);

	printf("Temperature: %d C, Pressure: %d mbar, Altitude: %d m\n", getTemperature(readUT(iFd)), uiPress, getAltitude(uiPress));
}
