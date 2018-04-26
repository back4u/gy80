#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "HMC5883L.h"
#include "gy80.h"

static float mgPerDigit = 0.0;
static int xOffset = 0;
static int yOffset = 0;
static struct Vector sVector = {0.0, 0.0, 0.0};

static int setRange(int iFd, hmc5883l_range_t range)
{
	unsigned char ucaBuf[2];

	switch(range)
    {
		case HMC5883L_RANGE_0_88GA:
			mgPerDigit = 0.073f;
			break;

		case HMC5883L_RANGE_1_3GA:
			mgPerDigit = 0.92f;
			break;

		case HMC5883L_RANGE_1_9GA:
			mgPerDigit = 1.22f;
			break;

		case HMC5883L_RANGE_2_5GA:
			mgPerDigit = 1.52f;
			break;

		case HMC5883L_RANGE_4GA:
			mgPerDigit = 2.27f;
			break;

		case HMC5883L_RANGE_4_7GA:
			mgPerDigit = 2.56f;
			break;

		case HMC5883L_RANGE_5_6GA:
			mgPerDigit = 3.03f;
			break;

		case HMC5883L_RANGE_8_1GA:
			mgPerDigit = 4.35f;
			break;

		default:
			break;
    }

	ucaBuf[0] = HMC5883L_REG_CONFIG_B;
	ucaBuf[1] = (range << 5);

	if (write(iFd, ucaBuf, 2) != 2)
	{
		fprintf(stderr, "Configuration Register B write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static hmc5883l_range_t getRange(int iFd)
{
	unsigned char ucaBuf[1];

	ucaBuf[0] = HMC5883L_REG_CONFIG_B;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register B write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register B read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return (hmc5883l_range_t)(ucaBuf[0] >> 5);
}

static void printRange(hmc5883l_range_t range)
{
	switch(range)
    {
		case HMC5883L_RANGE_0_88GA:
			printf("Range: 0.88 Ga\n");
			break;

		case HMC5883L_RANGE_1_3GA:
			printf("Range: 1.3 Ga\n");
			break;

		case HMC5883L_RANGE_1_9GA:
			printf("Range: 1.9 Ga\n");
			break;

		case HMC5883L_RANGE_2_5GA:
			printf("Range: 2.5 Ga\n");
			break;

		case HMC5883L_RANGE_4GA:
			printf("Range: 4.0 Ga\n");
			break;

		case HMC5883L_RANGE_4_7GA:
			printf("Range: 4.7 Ga\n");
			break;

		case HMC5883L_RANGE_5_6GA:
			printf("Range: 5.6 Ga\n");
			break;

		case HMC5883L_RANGE_8_1GA:
			printf("Range: 8.1 Ga\n");
			break;

		default:
			printf("Range: ERROR\n");
			break;
    }
}

static int setMeasurementMode(int iFd, hmc5883l_mode_t mode)
{

	unsigned char ucaBuf[2];

	ucaBuf[0] = HMC5883L_REG_MODE;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Mode Register write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Mode Register read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ucaBuf[1]  = ucaBuf[0];
	ucaBuf[1] &= 0b11111100;
	ucaBuf[1] |= mode;

	ucaBuf[0] = HMC5883L_REG_MODE;

	if (write(iFd, ucaBuf, 2) != 2)
	{
		fprintf(stderr, "Mode Register write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static hmc5883l_mode_t getMeasurementMode(int iFd)
{
	unsigned char ucaBuf[1];

	ucaBuf[0] = HMC5883L_REG_MODE;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Mode Register write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Mode Register read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

    ucaBuf[0] &= 0b00000011;

    return (hmc5883l_mode_t)ucaBuf[0];
}

static void printMeasurementMode(hmc5883l_mode_t mode)
{
	switch(mode)
    {
		case HMC5883L_IDLE:
			printf("Mode: Idle\n");
			break;

		case HMC5883L_SINGLE:
			printf("Mode: Single\n");
			break;

		case HMC5883L_CONTINOUS:
			printf("Mode: Continuous\n");
			break;

		default:
			printf("Mode: ERROR\n");
			break;
    }
}

static int setDataRate(int iFd, hmc5883l_dataRate_t dataRate)
{
	unsigned char ucaBuf[2];

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ucaBuf[1]  = ucaBuf[0];
	ucaBuf[1] &= 0b11100011;
	ucaBuf[1] |= (dataRate << 2);

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 2) != 2)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static hmc5883l_dataRate_t getDataRate(int iFd)
{
	unsigned char ucaBuf[1];

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ucaBuf[0] &= 0b00011100;

    return (hmc5883l_dataRate_t)(ucaBuf[0] >> 2);
}

static void printDataRate(hmc5883l_dataRate_t dataRate)
{
	switch(dataRate)
    {
		case HMC5883L_DATARATE_75HZ:
			printf("Data Rate: 75 Hz\n");
			break;

		case HMC5883L_DATARATE_30HZ:
			printf("Data Rate: 30 Hz\n");
			break;

		case HMC5883L_DATARATE_15HZ:
			printf("Data Rate: 15 Hz\n");
			break;

		case HMC5883L_DATARATE_7_5HZ:
			printf("Data Rate: 7.5 Hz\n");
			break;

		case HMC5883L_DATARATE_3HZ:
			printf("Data Rate: 3 Hz\n");
			break;

		case HMC5883L_DATARATE_1_5HZ:
			printf("Data Rate: 1.5 Hz\n");
			break;

		case HMC5883L_DATARATE_0_75HZ:
			printf("Data Rate: 0.75 Hz\n");
			break;

		default:
			printf("Data Rate: ERROR\n");
			break;
    }
}

static int setSamples(int iFd, hmc5883l_samples_t samples)
{
	unsigned char ucaBuf[2];

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ucaBuf[1]  = ucaBuf[0];
	ucaBuf[1] &= 0b10011111;
	ucaBuf[1] |= (samples << 5);

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 2) != 2)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static hmc5883l_samples_t getSamples(int iFd)
{
	unsigned char ucaBuf[1];

	ucaBuf[0] = HMC5883L_REG_CONFIG_A;

	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (read(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Configuration Register A read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ucaBuf[0] &= 0b01100000;

    return (hmc5883l_samples_t)(ucaBuf[0] >> 5);
}

static void printSamples(hmc5883l_samples_t samples)
{
	switch(samples)
    {
		case HMC5883L_SAMPLES_8:
			printf("Sample: 8\n");
			break;

		case HMC5883L_SAMPLES_4:
			printf("Sample: 4\n");
			break;

		case HMC5883L_SAMPLES_2:
			printf("Sample: 2\n");
			break;

		case HMC5883L_SAMPLES_1:
			printf("Sample: 1\n");
			break;

		default:
			printf("Sample: ERROR\n");
			break;
    }
}

//static int getOutputRaw(int iFd)
//{
//	unsigned char ucaBuf[6];
//	unsigned char ucaData[2];
//
//	ucaBuf[0] = HMC5883L_REG_OUT_X_M;
//
//	// move the address pointer to register 03
//	if (write(iFd, ucaBuf, 1) != 1)
//	{
//		fprintf(stderr, "Data Output X MSB Register write error: %s\n", strerror(errno));
//		return EXIT_FAILURE;
//	}
//
//	usleep(WAIT_100MS);
//
//	// read MSB and LSB of X, Y, Z
//	if (read(iFd, ucaBuf, 6) != 6)
//	{
//		fprintf(stderr, "Data Output Registers read error: %s\n", strerror(errno));
//		return EXIT_FAILURE;
//	}
//	else
//	{
//		ucaData[0] = ucaBuf[1];
//		ucaData[1] = ucaBuf[0];
//		sVector.XAxis = *((short *)ucaData) - xOffset;
//
//		ucaData[0] = ucaBuf[3];
//		ucaData[1] = ucaBuf[2];
//		sVector.YAxis = *((short *)ucaData) - yOffset;
//
//		ucaData[0] = ucaBuf[5];
//		ucaData[1] = ucaBuf[4];
//		sVector.ZAxis = *((short *)ucaData);
//	}
//
//	return EXIT_SUCCESS;
//}

static int getOutputNormalized(int iFd)
{
	unsigned char ucaBuf[6];
	unsigned char ucaData[2];

	ucaBuf[0] = HMC5883L_REG_OUT_X_M;

	// move the address pointer to register 03
	if (write(iFd, ucaBuf, 1) != 1)
	{
		fprintf(stderr, "Data Output X MSB Register write error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	usleep(WAIT_100MS);

	// read MSB and LSB of X, Y, Z
	if (read(iFd, ucaBuf, 6) != 6)
	{
		fprintf(stderr, "Data Output Registers read error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	else
	{
		ucaData[0] = ucaBuf[1];
		ucaData[1] = ucaBuf[0];
		sVector.XAxis = (*((short *)ucaData) - xOffset) * mgPerDigit;

		ucaData[0] = ucaBuf[3];
		ucaData[1] = ucaBuf[2];
		sVector.YAxis = (*((short *)ucaData) - yOffset) * mgPerDigit;

		ucaData[0] = ucaBuf[5];
		ucaData[1] = ucaBuf[4];
		sVector.ZAxis = (*((short *)ucaData)) * mgPerDigit;
	}

	return EXIT_SUCCESS;
}

static float calcHeading(void)
{
	return atan2(sVector.YAxis * mgPerDigit, sVector.XAxis * mgPerDigit)*180.0/PI;
}

static void printOutput(float dHeading_deg)
{
	printf("X-Axis: %f, Y-Axis: %f, Z-Axis: %f, Heading: %f, ", sVector.XAxis, sVector.YAxis, sVector.ZAxis, dHeading_deg);
}

void initHMC5883L(int iFd)
{
	setRange(iFd, HMC5883L_RANGE_1_3GA);
	setMeasurementMode(iFd, HMC5883L_CONTINOUS);
	setDataRate(iFd, HMC5883L_DATARATE_15HZ);
	setSamples(iFd, HMC5883L_SAMPLES_1);

	printf("HMC5883L sensor is initializing...\n");

	printRange(getRange(iFd));
	printMeasurementMode(getMeasurementMode(iFd));
	printDataRate(getDataRate(iFd));
	printSamples(getSamples(iFd));
}

void readHMC5883L(int iFd)
{
	getOutputNormalized(iFd);
	printOutput(calcHeading());
}
