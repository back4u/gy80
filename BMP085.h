#ifndef BMP085_H
#define BMP085_H

#define BMP085_ADDRESS     0x77
#define BMP085_RA_AC1_H    0xAA    /* AC1_H */
#define BMP085_RA_AC1_L    0xAB    /* AC1_L */
#define BMP085_RA_AC2_H    0xAC    /* AC2_H */
#define BMP085_RA_AC2_L    0xAD    /* AC2_L */
#define BMP085_RA_AC3_H    0xAE    /* AC3_H */
#define BMP085_RA_AC3_L    0xAF    /* AC3_L */
#define BMP085_RA_AC4_H    0xB0    /* AC4_H */
#define BMP085_RA_AC4_L    0xB1    /* AC4_L */
#define BMP085_RA_AC5_H    0xB2    /* AC5_H */
#define BMP085_RA_AC5_L    0xB3    /* AC5_L */
#define BMP085_RA_AC6_H    0xB4    /* AC6_H */
#define BMP085_RA_AC6_L    0xB5    /* AC6_L */
#define BMP085_RA_B1_H     0xB6    /* B1_H */
#define BMP085_RA_B1_L     0xB7    /* B1_L */
#define BMP085_RA_B2_H     0xB8    /* B2_H */
#define BMP085_RA_B2_L     0xB9    /* B2_L */
#define BMP085_RA_MB_H     0xBA    /* MB_H */
#define BMP085_RA_MB_L     0xBB    /* MB_L */
#define BMP085_RA_MC_H     0xBC    /* MC_H */
#define BMP085_RA_MC_L     0xBD    /* MC_L */
#define BMP085_RA_MD_H     0xBE    /* MD_H */
#define BMP085_RA_MD_L     0xBF    /* MD_L */
#define BMP085_RA_CONTROL  0xF4    /* CONTROL */
#define BMP085_RA_MSB      0xF6    /* MSB */
#define BMP085_RA_LSB      0xF7    /* LSB */
#define BMP085_RA_XLSB     0xF8    /* XLSB */

#define BMP085_MODE_TEMPERATURE  0x2E
#define BMP085_MODE_PRESSURE_0   0x34
#define BMP085_MODE_PRESSURE_1   0x74
#define BMP085_MODE_PRESSURE_2   0xB4
#define BMP085_MODE_PRESSURE_3   0xF4

#define BMP085_OVERSAMPLING_SETTING 3

void initBMP085(int iFd);
void readBMP085(int iFd);

#endif /* BMP085_H */
