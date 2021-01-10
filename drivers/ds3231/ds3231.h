/*
 * ds3231.h
 *
 *  Created on: 4 janv. 2021
 *      Author: Quent
 */

#ifndef __DS3231_H__
#define __DS3231_H__


/* ------------------------------------------------------------- --
   Includes
-- ------------------------------------------------------------- */
#include "main.h"
#include <stdlib.h>
#include "stdarg.h"
#include <stdbool.h>


/* ------------------------------------------------------------- --
   Define
-- ------------------------------------------------------------- */
/* DS3231 address */
#define DS3231_ADDR  (0x68 << 1)

/* DS3231 registers */
#define DS3231_REG_TIME         0x00
#define DS3231_REG_ALARM1       0x07
#define DS3231_REG_ALARM2       0x0B
#define DS3231_REG_CONTROL      0x0E
#define DS3231_REG_STATUS       0x0F
#define DS3231_REG_TEMP         0x11

#define DS3231_CON_EOSC         0x80
#define DS3231_CON_BBSQW        0x40
#define DS3231_CON_CONV         0x20
#define DS3231_CON_RS2          0x10
#define DS3231_CON_RS1          0x08
#define DS3231_CON_INTCN        0x04
#define DS3231_CON_A2IE         0x02
#define DS3231_CON_A1IE         0x01

#define DS3231_STA_OSF          0x80
#define DS3231_STA_32KHZ        0x08
#define DS3231_STA_BSY          0x04
#define DS3231_STA_A2F          0x02
#define DS3231_STA_A1F          0x01


/* ------------------------------------------------------------- --
   Enumerates
-- ------------------------------------------------------------- */
typedef enum
{
  ALARM_MODE_ALL_MATCHED = 0,
  ALARM_MODE_HOUR_MIN_SEC_MATCHED,
  ALARM_MODE_MIN_SEC_MATCHED,
  ALARM_MODE_SEC_MATCHED,
  ALARM_MODE_ONCE_PER_SECOND
} DS3231_AlarmMode;

typedef enum
{
  SUNDAY = 1,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
} DS3231_DaysOfWeek;


/* ------------------------------------------------------------- --
   Structures
-- ------------------------------------------------------------- */
typedef struct
{
   uint8_t Year;
   uint8_t Month;
   uint8_t Date;
   uint8_t DaysOfWeek;
   uint8_t Hour;
   uint8_t Min;
   uint8_t Sec;

   float temperature;

   I2C_HandleTypeDef i2c;
}DS3231_t;


/* ------------------------------------------------------------- --
   Functions
-- ------------------------------------------------------------- */
uint8_t DS3231_Init(DS3231_t *DS3231);

uint8_t DS3231_Read_All(DS3231_t *DS3231);
uint8_t DS3231_Read_Time(DS3231_t *DS3231);
uint8_t DS3231_Read_Temperature(DS3231_t *DS3231);

uint8_t DS3231_SetTime(DS3231_t *DS3231);


#endif /* __DS3231_H__ */


/* ------------------------------------------------------------- --
   End of files
-- ------------------------------------------------------------- */
