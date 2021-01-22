/*
 * ssd1306.c
 *
 *  Created on: Jan 21, 2021
 *      Author: Quent
 */

/* ------------------------------------------------------------- --
   Includes
-- ------------------------------------------------------------- */
#include "ssd1306.h"


/* ------------------------------------------------------------- --
   Global variables
-- ------------------------------------------------------------- */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static HAL_StatusTypeDef SSD1306_check;


/* ------------------------------------------------------------- --
   Private prototypes
-- ------------------------------------------------------------- */
uint8_t SSD1306_WriteCommand(SSD1306_t *SSD1306, uint8_t cmd);


/* ------------------------------------------------------------- --
   Private functions
-- ------------------------------------------------------------- */
/* ************************************************************* *
 * @name		SSD1306_WriteCommand
 * @brief		send the commands
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_WriteCommand(SSD1306_t *SSD1306, uint8_t cmd)
{
	uint8_t data[2] = {SSD1306_REG_CMD, cmd};

	if(HAL_I2C_Master_Transmit(&SSD1306->i2c, SSD1306_ADDR, data, 2, HAL_MAX_DELAY)) return HAL_ERROR;

	return HAL_OK;
}


/* ------------------------------------------------------------- --
   Functions
-- ------------------------------------------------------------- */
/* ************************************************************* *
 * @name		SSD1306_Init
 * @brief		initialize the mpu6050
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Init(SSD1306_t *SSD1306)
{
	/* Init LCD */
	SSD1306_WriteCommand(SSD1306, 0xAE); //display off
	SSD1306_WriteCommand(SSD1306, 0x20); //Set Memory Addressing Mode   
	SSD1306_WriteCommand(SSD1306, 0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	SSD1306_WriteCommand(SSD1306, 0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SSD1306_WriteCommand(SSD1306, 0xC8); //Set COM Output Scan Direction
	SSD1306_WriteCommand(SSD1306, 0x00); //---set low column address
	SSD1306_WriteCommand(SSD1306, 0x10); //---set high column address
	SSD1306_WriteCommand(SSD1306, 0x40); //--set start line address
	SSD1306_WriteCommand(SSD1306, 0x81); //--set contrast control register
	SSD1306_WriteCommand(SSD1306, 0xFF);
	SSD1306_WriteCommand(SSD1306, 0xA1); //--set segment re-map 0 to 127
	SSD1306_WriteCommand(SSD1306, 0xA6); //--set normal display
	SSD1306_WriteCommand(SSD1306, 0xA8); //--set multiplex ratio(1 to 64)
	SSD1306_WriteCommand(SSD1306, 0x3F); //
	SSD1306_WriteCommand(SSD1306, 0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SSD1306_WriteCommand(SSD1306, 0xD3); //-set display offset
	SSD1306_WriteCommand(SSD1306, 0x00); //-not offset
	SSD1306_WriteCommand(SSD1306, 0xD5); //--set display clock divide ratio/oscillator frequency
	SSD1306_WriteCommand(SSD1306, 0xF0); //--set divide ratio
	SSD1306_WriteCommand(SSD1306, 0xD9); //--set pre-charge period
	SSD1306_WriteCommand(SSD1306, 0x22); //
	SSD1306_WriteCommand(SSD1306, 0xDA); //--set com pins hardware configuration
	SSD1306_WriteCommand(SSD1306, 0x12);
	SSD1306_WriteCommand(SSD1306, 0xDB); //--set vcomh
	SSD1306_WriteCommand(SSD1306, 0x20); //0x20,0.77xVcc
	SSD1306_WriteCommand(SSD1306, 0x8D); //--set DC-DC enable
	SSD1306_WriteCommand(SSD1306, 0x14); //
	SSD1306_WriteCommand(SSD1306, 0xAF); //--turn on SSD1306 panel
	SSD1306_WriteCommand(SSD1306, SSD1306_DEACTIVATE_SCROLL);

	/* Clear screen */
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	
	/* Update screen */
	SSD1306_UpdateScreen(SSD1306);
	
	/* Set default values */
	SSD1306->CurrentX = 0;
	SSD1306->CurrentY = 0;
	
	/* Initialized OK */
	SSD1306->Initialized = 1;
	
	/* Return OK */
	return HAL_OK;
}

/* ************************************************************* *
 * @name		SSD1306_Fill
 * @brief		fill the screen with the arg color
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Fill(SSD1306_COLOR_t color)
{
	/* Set memory */
	memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}


/* ************************************************************* *
 * @name		SSD1306_UpdateScreen
 * @brief		update the screen with the screen buffer
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_UpdateScreen(SSD1306_t *SSD1306) {
	uint8_t m;

	uint8_t data[256];
	data[0] = 0x40;
	uint8_t i;
	
	for (m = 0; m < 8; m++) {
		SSD1306_check = SSD1306_WriteCommand(SSD1306, 0xB0 + m);
		SSD1306_check = SSD1306_WriteCommand(SSD1306, 0x00);
		SSD1306_check = SSD1306_WriteCommand(SSD1306, 0x10);
		
		SSD1306_I2C_WriteMulti(SSD1306, SSD1306_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}


/* ************************************************************* *
 * @name		SSD1306_I2C_WriteMulti
 * @brief		send multi message on i2c
 *
 * @args
 * ************************************************************* */
void SSD1306_I2C_WriteMulti(SSD1306_t *SSD1306, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) 
{
	uint8_t dt[256];
	dt[0] = reg;
	uint8_t i;
		for(i = 0; i < count; i++) dt[i+1] = data[i];

	HAL_I2C_Master_Transmit(&SSD1306->i2c, address, dt, count+1, 10);
}


/* ************************************************************* *
 * @name		SSD1306_Clear
 * @brief		clear the screeb
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Clear(SSD1306_t *SSD1306)
{
	SSD1306_Fill (SSD1306_COLOR_BLACK);
    SSD1306_UpdateScreen(SSD1306);
}


/* ************************************************************* *
 * @name		SSD1306_GotoXY
 * @brief		move the cursor on the screen
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_GotoXY(SSD1306_t *SSD1306, uint16_t x, uint16_t y)
{
	/* Set write pointers */
	SSD1306->CurrentX = x;
	SSD1306->CurrentY = y;
}


/* ************************************************************* *
 * @name		SSD1306_DrawPixel
 * @brief		draw a pixel at arg
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) /* Error */ return;
	
	/* Set color */
	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}


/* ************************************************************* *
 * @name		SSD1306_DrawBitmap
 * @brief		draw a bitmap
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color)
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) SSD1306_DrawPixel(x+i, y, color);
        }
    }
}


/* ************************************************************* *
 * @name		SSD1306_DrawLine
 * @brief		draw a line
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	
	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			SSD1306_DrawPixel(x0, i, c);
		}
		
		/* Return from function */
		return;
	}
	
	if (dy == 0) 
	{
		if (y1 < y0) 
		{
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) 
		{
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			SSD1306_DrawPixel(i, y0, c);
		}
		
		/* Return from function */
		return;
	}
	
	while (1) {
		SSD1306_DrawPixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}


/* ************************************************************* *
 * @name		SSD1306_Putc
 * @brief		print a character
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Putc(SSD1306_t *SSD1306, uint8_t ch, FontDef_t* Font, SSD1306_COLOR_t color) {
	uint32_t i, b, j;
	
	/* Check available space in LCD */
	if (
		SSD1306_WIDTH <= (SSD1306->CurrentX + Font->FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306->CurrentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}
	
	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_DrawPixel(SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t) color);
			} else {
				SSD1306_DrawPixel(SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	
	/* Increase pointer */
	SSD1306->CurrentX += Font->FontWidth;
	
	/* Return character written */
	return ch;
}


/* ************************************************************* *
 * @name		SSD1306_Puts
 * @brief		print a string
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Puts(SSD1306_t *SSD1306, uint8_t* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_Putc(SSD1306, *str, Font, color) != *str) {
			/* Return error */
			return *str;
		}
		
		/* Increase string pointer */
		str++;
	}
	
	/* Everything OK, zero should be returned */
	return *str;
}


/* ************************************************************* *
 * @name		SSD1306_Puts_Num16bits
 * @brief		print a number
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_Puts_Num16bits(SSD1306_t *SSD1306, int16_t number, FontDef_t* Font, SSD1306_COLOR_t color)
{
	uint8_t buffer[10];

	sprintf(buffer, "%d", number);
	SSD1306_Puts(SSD1306, buffer, Font, color);

	return *buffer;
}


/* ************************************************************* *
 * @name		SSD1306_ScrollRight
 * @brief		scroll text to the right
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_ScrollRight(SSD1306_t *SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCommand(SSD1306, SSD1306_RIGHT_HORIZONTAL_SCROLL);  // send 0x26
  SSD1306_WriteCommand(SSD1306, 0x00);  // send dummy
  SSD1306_WriteCommand(SSD1306, start_row);  // start page address
  SSD1306_WriteCommand(SSD1306, 0X00);  // time interval 5 frames
  SSD1306_WriteCommand(SSD1306, end_row);  // end page address
  SSD1306_WriteCommand(SSD1306, 0X00);
  SSD1306_WriteCommand(SSD1306, 0XFF);
  SSD1306_WriteCommand(SSD1306, SSD1306_ACTIVATE_SCROLL); // start scroll

  return HAL_OK;
}


/* ************************************************************* *
 * @name		SSD1306_ScrollLeft
 * @brief		scroll text to the left
 *
 * @args
 * ************************************************************* */
uint8_t SSD1306_ScrollLeft(SSD1306_t *SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCommand(SSD1306, SSD1306_LEFT_HORIZONTAL_SCROLL);  // send 0x26
  SSD1306_WriteCommand(SSD1306, 0x00);  // send dummy
  SSD1306_WriteCommand(SSD1306, start_row);  // start page address
  SSD1306_WriteCommand(SSD1306, 0X00);  // time interval 5 frames
  SSD1306_WriteCommand(SSD1306, end_row);  // end page address
  SSD1306_WriteCommand(SSD1306, 0X00);
  SSD1306_WriteCommand(SSD1306, 0XFF);
  SSD1306_WriteCommand(SSD1306, SSD1306_ACTIVATE_SCROLL); // start scroll

  return HAL_OK;
}


/* ************************************************************* *
 * @name		SSD1306_Stopscroll
 * @brief		stop the scroll
 *
 * @args
 * ************************************************************* */
void SSD1306_Stopscroll(SSD1306_t *SSD1306)
{
	 SSD1306_WriteCommand(SSD1306, SSD1306_DEACTIVATE_SCROLL);
}