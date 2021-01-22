/*
 * ssd1306.h
 *
 *  Created on: Jan 21, 2021
 *      Author: Quent
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__


/* ------------------------------------------------------------- --
   Includes
-- ------------------------------------------------------------- */
#include "fonts.h"
#include "main.h"

#include "stdlib.h"
#include "string.h"


/* ------------------------------------------------------------- --
   Defines
-- ------------------------------------------------------------- */
/* I2C address */
#define SSD1306_ADDR         				0x78  /* or 0x7A */     

/* SSD1306 size in pixels */
#define SSD1306_WIDTH            			128
#define SSD1306_HEIGHT           			64

/* i2c timeout */
#define ssd1306_I2C_TIMEOUT					2000

/* register addresses*/
#define SSD1306_REG_CMD						0x00

/* list of commands */
#define SSD1306_RIGHT_HORIZONTAL_SCROLL     0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL      0x27
#define SSD1306_DEACTIVATE_SCROLL           0x2E 
#define SSD1306_ACTIVATE_SCROLL             0x2F 
#define SSD1306_SET_VERTICAL_SCROLL_AREA    0xA3 


/* ------------------------------------------------------------- --
   Enumerates
-- ------------------------------------------------------------- */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /* Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01  /* White color,  1 pixel */
} SSD1306_COLOR_t;


/* ------------------------------------------------------------- --
   Structures
-- ------------------------------------------------------------- */
typedef struct
{
	uint16_t CurrentX;
	uint16_t CurrentY;

	uint8_t Initialized;

	I2C_HandleTypeDef i2c;
}SSD1306_t;


/* ------------------------------------------------------------- --
   Functions
-- ------------------------------------------------------------- */
/* init */
uint8_t SSD1306_Init(SSD1306_t *SSD1306);

/* screen action */
uint8_t SSD1306_UpdateScreen(SSD1306_t *SSD1306);
uint8_t SSD1306_Fill(SSD1306_COLOR_t Color);
uint8_t SSD1306_Clear (SSD1306_t *SSD1306);

/* cursor */
uint8_t SSD1306_GotoXY(SSD1306_t *SSD1306, uint16_t x, uint16_t y);

/* draw  */
uint8_t SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
uint8_t SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c);
uint8_t SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color);

/* print characters */
uint8_t SSD1306_Putc(SSD1306_t *SSD1306, uint8_t ch, FontDef_t* Font, SSD1306_COLOR_t color);
uint8_t SSD1306_Puts(SSD1306_t *SSD1306, uint8_t* str, FontDef_t* Font, SSD1306_COLOR_t color);
uint8_t SSD1306_Puts_Num16bits(SSD1306_t *SSD1306, int16_t number, FontDef_t* Font, SSD1306_COLOR_t color);

/* scroll */
uint8_t SSD1306_ScrollRight(SSD1306_t *SSD1306, uint8_t start_row, uint8_t end_row);
uint8_t SSD1306_ScrollLeft(SSD1306_t *SSD1306, uint8_t start_row, uint8_t end_row);
uint8_t SSD1306_Stopscroll(void);

/* i2c */
void SSD1306_I2C_WriteMulti(SSD1306_t *SSD1306, uint8_t address, uint8_t reg, uint8_t *data, uint16_t count);


/* ------------------------------------------------------------- --
   End of file
-- ------------------------------------------------------------- */

#endif /* __SSD1306_H__ */
