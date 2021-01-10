/*
 * bmp280.h
 *
 *  Created on: Dec 19, 2020
 *      Author: Quentin Bakrim
 */


/* ------------------------------------------------------------- --
   Includes
-- ------------------------------------------------------------- */
#include "bmp280.h"


/* ------------------------------------------------------------- --
   Variables
-- ------------------------------------------------------------- */
static const uint16_t i2c_timeout = 1000;


/* ------------------------------------------------------------- --
   Private prototypes
-- ------------------------------------------------------------- */
static uint8_t bmp280_read_fixed(BMP280_t* BMP280, int32_t *temperature, uint32_t *pressure);
static uint8_t bmp280_read_register16(BMP280_t *BMP280, uint8_t addr, uint16_t *value);
static uint8_t bmp280_read_calibration_data(BMP280_t *BMP280);

static int32_t bmp280_compensate_temperature(BMP280_t *BMP280, int32_t adc_temp, int32_t *fine_temp);
static uint32_t bmp280_compensate_pressure(BMP280_t *BMP280, int32_t adc_press, int32_t fine_temp);


/* ============================================================= ==
   Functions
== ============================================================= */
/* ************************************************************* *
 * @name		BMP280_Init
 * @brief
 *
 * @args
 * ************************************************************* */
uint8_t BMP280_Init(BMP280_t *BMP280)
{
    uint8_t check;
    uint8_t status;
    uint8_t data;

    /* initialize the temperature and the pressure to 0 */
    BMP280->pressure = 0.0;
    BMP280->temperature = 0.0;

    /* Structure to configure the BMP280 */
    BMP280_config_t config;
	config.mode = 						BMP280_MODE_NORMAL;
	config.filter = 					BMP280_FILTER_OFF;
	config.oversampling_pressure = 		BMP280_STANDARD;
	config.oversampling_temperature = 	BMP280_STANDARD;
	config.standby = 					BMP280_STANDBY_250;
	BMP280->config = config;

	/* check device ID WHO_AM_I */
	if(HAL_I2C_Mem_Read(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_ID, 1, &check, 1, i2c_timeout)) return HAL_ERROR;
	if(check != BMP280_CHIP_ID) return false;

	/* reset the chip */
	if(HAL_I2C_Mem_Write(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_RESET, 1, (uint8_t*)BMP280_RESET_VALUE, 1, i2c_timeout)) return HAL_ERROR;

	/* Wait until finished copying over the NVP data */
	while (1)
	{
		if(!HAL_I2C_Mem_Read(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_STATUS, 1, &status, 1, i2c_timeout) && (status & 1) == 0) break;
	}

	/* read calibration data */
	bmp280_read_calibration_data(BMP280);

	/* Config register */
	data = (BMP280->config.standby << 5 | BMP280->config.filter << 2);
	if(HAL_I2C_Mem_Write(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_CONFIG, 1, &data, 1, i2c_timeout)) return HAL_ERROR;

	/* Control register */
	data = (BMP280->config.oversampling_temperature << 5 | BMP280->config.oversampling_pressure << 2 | BMP280->config.mode);
	if(HAL_I2C_Mem_Write(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_CTRL, 1, &data, 1, i2c_timeout)) return HAL_ERROR;

	return HAL_OK;
}


/* ************************************************************* *
 * @name		BMP280_Read_All
 * @brief		read the temperature and pressure data
 *
 * @args
 * ************************************************************* */
uint8_t BMP280_Read_All(BMP280_t* BMP280)
{
	int32_t fixed_temperature;
	uint32_t fixed_pressure;

	if(!bmp280_read_fixed(BMP280, &fixed_temperature, &fixed_pressure))
	{
		BMP280->temperature = (float) fixed_temperature / 100;
		BMP280->pressure = (float) fixed_pressure / 256;
		return HAL_OK;
	}
	return HAL_ERROR;
}


/* ************************************************************* *
 * @name		bmp280_read_fixed
 * @brief		read the data fixed with the calibrated data
 *
 * @args
 * ************************************************************* */
uint8_t bmp280_read_fixed(BMP280_t* BMP280, int32_t *temperature, uint32_t *pressure)
{
	int32_t adc_pressure;
	int32_t adc_temp;
	int32_t fine_temp;
	uint8_t data[6];

	if(HAL_I2C_Mem_Read(&BMP280->i2c, BMP280_ADDR<<1, BMP280_REG_PRESS_MSB, 1, data, sizeof(data), i2c_timeout)) return HAL_ERROR;

	adc_pressure = data[0] << 12 | data[1] << 4 | data[2] >> 4;
	adc_temp = data[3] << 12 | data[4] << 4 | data[5] >> 4;

	*temperature = bmp280_compensate_temperature(BMP280, adc_temp, &fine_temp);
	*pressure = bmp280_compensate_pressure(BMP280, adc_pressure, fine_temp);

	return HAL_OK;
}


/* ************************************************************* *
 * @name		bmp280_compensate_temperature
 * @brief		calculate the right temperature with the calibration
 * 				data.
 *
 * @args
 * ************************************************************* */
static int32_t bmp280_compensate_temperature(BMP280_t *BMP280, int32_t adc_temp, int32_t *fine_temp)
{
	int32_t var1, var2;

	var1 = ((((adc_temp >> 3) - ((int32_t) BMP280->calib.dig_T1 << 1))) * (int32_t) BMP280->calib.dig_T2) >> 11;
	var2 = (((((adc_temp >> 4) - (int32_t) BMP280->calib.dig_T1) * ((adc_temp >> 4) - (int32_t) BMP280->calib.dig_T1)) >> 12) * (int32_t) BMP280->calib.dig_T3) >> 14;

	*fine_temp = var1 + var2;
	return (*fine_temp * 5 + 128) >> 8;
}


/* ************************************************************* *
 * @name		bmp280_compensate_pressure
 * @brief		calculate the right pressure with the calibration
 * 				data.
 *
 * @args
 * ************************************************************* */
static uint32_t bmp280_compensate_pressure(BMP280_t *BMP280, int32_t adc_press, int32_t fine_temp)
{
	int64_t var1, var2, p;

	var1 = (int64_t) fine_temp - 128000;
	var2 = var1 * var1 * (int64_t)  BMP280->calib.dig_P6;
	var2 = var2 + ((var1 * (int64_t)  BMP280->calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)  BMP280->calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)  BMP280->calib.dig_P3) >> 8) + ((var1 * (int64_t)  BMP280->calib.dig_P2) << 12);
	var1 = (((int64_t) 1 << 47) + var1) * ((int64_t)  BMP280->calib.dig_P1) >> 33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = ((int64_t)  BMP280->calib.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
	var2 = ((int64_t)  BMP280->calib.dig_P8 * p) >> 19;

	p = ((p + var1 + var2) >> 8) + ((int64_t)  BMP280->calib.dig_P7 << 4);
	return p;
}


/* ************************************************************* *
 * @name		bmp280_read_calibration_data
 * @brief		read the calibration registers of the bmp280
 *
 * @args
 * ************************************************************* */
static uint8_t bmp280_read_calibration_data(BMP280_t *BMP280) {

	if(bmp280_read_register16(BMP280, 0x88, &BMP280->calib.dig_T1)
	&& bmp280_read_register16(BMP280, 0x8a, (uint16_t *) &BMP280->calib.dig_T2)
	&& bmp280_read_register16(BMP280, 0x8c, (uint16_t *) &BMP280->calib.dig_T3)
	&& bmp280_read_register16(BMP280, 0x8e, &BMP280->calib.dig_P1)
	&& bmp280_read_register16(BMP280, 0x90, (uint16_t *) &BMP280->calib.dig_P2)
	&& bmp280_read_register16(BMP280, 0x92, (uint16_t *) &BMP280->calib.dig_P3)
	&& bmp280_read_register16(BMP280, 0x94, (uint16_t *) &BMP280->calib.dig_P4)
	&& bmp280_read_register16(BMP280, 0x96, (uint16_t *) &BMP280->calib.dig_P5)
	&& bmp280_read_register16(BMP280, 0x98, (uint16_t *) &BMP280->calib.dig_P6)
	&& bmp280_read_register16(BMP280, 0x9a, (uint16_t *) &BMP280->calib.dig_P7)
	&& bmp280_read_register16(BMP280, 0x9c, (uint16_t *) &BMP280->calib.dig_P8)
	&& bmp280_read_register16(BMP280, 0x9e, (uint16_t *) &BMP280->calib.dig_P9))
	{
		return HAL_OK;
	}
	return HAL_ERROR;
}


/* ************************************************************* *
 * @name		bmp280_read_register16
 * @brief		Function used to read 2 bytes of data and return
 * 				it.
 *
 * @args
 * ************************************************************* */
static uint8_t bmp280_read_register16(BMP280_t *BMP280, uint8_t addr, uint16_t *value)
{
	uint8_t rx_buff[2];

	if (HAL_I2C_Mem_Read(&BMP280->i2c, BMP280_ADDR<<1, addr, 1, rx_buff, 2, 5000) == HAL_OK)
	{
		*value = (uint16_t) ((rx_buff[1] << 8) | rx_buff[0]);
		return true;
	}
	else
	{
		return false;
	}
}


/* ------------------------------------------------------------- --
   End of file
-- ------------------------------------------------------------- */
