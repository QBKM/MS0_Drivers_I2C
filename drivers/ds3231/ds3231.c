/*
 * ds3231.c
 *
 *  Created on: 4 janv. 2021
 *      Author: Quent
 */


/* ------------------------------------------------------------- --
   Includes
-- ------------------------------------------------------------- */
#include "ds3231.h"


/* ------------------------------------------------------------- --
   Variables
-- ------------------------------------------------------------- */
static const uint16_t i2c_timeout = 100;


/* ------------------------------------------------------------- --
   private prototypes
-- ------------------------------------------------------------- */
static uint8_t B2D(uint8_t bcd);
static uint8_t D2B(uint8_t decimal);


/* ============================================================= ==
   Functions
== ============================================================= */
/* ************************************************************* *
 * @name		DS3231_Init
 * @brief   init the time of the ds3231
 *
 * @args    DS3231_t
 * ************************************************************* */
uint8_t DS3231_Init(DS3231_t *DS3231)
{
  DS3231->Sec =         0;  /* 0 sec */
  DS3231->Min =         0;  /* 0 mins */
  DS3231->Hour =        0;  /* 0 hours */
  DS3231->DaysOfWeek =  1;  /* 1 = sunday*/
  DS3231->Date =        1;  /* 1st */
  DS3231->Month =       1;  /* January */
  DS3231->Year =        0;  /* 0000 */

  if(DS3231_SetTime(DS3231)) return HAL_ERROR;
	return HAL_OK;
}

/* ************************************************************* *
 * @name		DS3231_Read_All
 * @brief   Read the time and the temperature
 *
 * @args    DS3231_t
 * ************************************************************* */
uint8_t DS3231_Read_All(DS3231_t *DS3231)
{
  if(DS3231_Read_Time(DS3231)) return HAL_ERROR;
  if(DS3231_Read_Temperature(DS3231)) return HAL_ERROR;

  return HAL_OK;
}

/* ************************************************************* *
 * @name		DS3231_Read_Time
 * @brief   Read the time
 *
 * @args    DS3231_t
 * ************************************************************* */
uint8_t DS3231_Read_Time(DS3231_t *DS3231)
{
  uint8_t data[7];

  if(HAL_I2C_Mem_Read(&DS3231->i2c, DS3231_ADDR, DS3231_REG_TIME, 1, data, sizeof(data), i2c_timeout )) return HAL_ERROR;

  DS3231->Sec = B2D(data[0] & 0x7F);
  DS3231->Min = B2D(data[1] & 0x7F);
  DS3231->Hour = B2D(data[2] & 0x3F);
  DS3231->DaysOfWeek = data[3] & 0x07;
  DS3231->Date = B2D(data[4] & 0x3F);
  DS3231->Month = B2D(data[5] & 0x1F);
  DS3231->Year = B2D(data[6]);

  return HAL_OK;
}


/* ************************************************************* *
 * @name		DS3231_Read_Temperature
 * @brief   Read the temperature
 *
 * @args    DS3231_t
 * ************************************************************* */
uint8_t DS3231_Read_Temperature(DS3231_t *DS3231)
{
  uint8_t data[2];

  if(HAL_I2C_Mem_Read(&DS3231->i2c, DS3231_ADDR, DS3231_REG_TEMP, 1, data, sizeof(data), i2c_timeout )) return HAL_ERROR;
  
  int16_t value = (data[0] << 8) | (data[1]);
  value = (value >> 6);

  DS3231->temperature = (value / 4.0f);
  return HAL_OK;
}


/* ************************************************************* *
 * @name		DS3231_SetTime
 * @brief   Set the time
 *
 * @args    DS3231_t
 * ************************************************************* */
uint8_t DS3231_SetTime(DS3231_t *DS3231)
{
  uint8_t startAddr = DS3231_REG_TIME;
  uint8_t data[8] = {startAddr, D2B(DS3231->Sec), D2B(DS3231->Min), D2B(DS3231->Hour), DS3231->DaysOfWeek, D2B(DS3231->Date), D2B(DS3231->Month), D2B(DS3231->Year)};
  
  if(HAL_I2C_Master_Transmit(&DS3231->i2c, DS3231_ADDR, data, sizeof(data), HAL_MAX_DELAY)) return HAL_ERROR;

  return HAL_OK;
}


/* ************************************************************* *
 * @name		B2D
 * @brief   Convert the bcd to decimal
 *
 * @args    bcd
 * ************************************************************* */
static uint8_t B2D(uint8_t bcd)
{
  return (bcd >> 4) * 10 + (bcd & 0x0F);
}


/* ************************************************************* *
 * @name		D2B
 * @brief   Convert the decimal to bcd
 *
 * @args    decimal
 * ************************************************************* */
static uint8_t D2B(uint8_t decimal)
{
  return (((decimal / 10) << 4) | (decimal % 10));
}
