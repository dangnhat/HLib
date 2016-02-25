/*
 * cLCD.h
 *
 *  Created on: Feb 2, 2015
 *      Author: Nguyen Van Hien <nvhien1992@gmail.com>
 *  How to use:
 *  _All pins connecting to LCD must be initialized auto-ly.
 *  _Declare a var had lcd_pins_t type.
 *  _Assign all functions pointer of var with functions respectively.
 *  _Call init_lcd function to initialize.
 */

#ifndef CLCD_H_
#define CLCD_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "MB1_GPIO.h"

extern gpio lcdRS;
extern gpio lcdEN;
extern gpio lcdBL;
extern gpio lcdD4;
extern gpio lcdD5;
extern gpio lcdD6;
extern gpio lcdD7;

/**
 * @brief init lcd, 4 bit mode, increase counter.
 * 
 * @return false if lcd_pins is null, true otherwise.
 */
bool cLCD_Init(void);

/**
 * @brief set position of cursor.
 *
 * @param[in] line, should be between 1 and 4.
 * @param[in] pos, should be between 1 and 20.
 *
 * @return success=true or failed=false.
 */bool cLCD_SetCursor(uint8_t line, uint8_t pos);

/**
 * @brief print a character to LCD.
 *
 * @param[in] a_char, a character.
 */
void cLCD_Putc(char aChar);

/**
 * @brief print a string to LCD.
 *
 * @param[in] a_string, pointer to a string of character.
 *
 * @return number of character has been print to LCD.
 */
int16_t cLCD_Puts(char *aString);

/**
 * @brief lcd's printf (only for 20x4-1 characters)
 *
 * @param[in] format, in printf's style.
 *
 * @return number of characters has been printed, negative value if error.
 */
int16_t cLCD_Printf(const char *format, ...);

/**
 * @brief clear screen and set cursor to 1-1.
 */
void cLCD_Clear(void);

/**
 * @brief Set cursor to 1-1. (no clearing)
 */
void cLCD_ReturnHome(void);

/**
 * @brief Set cursor to 1-1 without shifting. (no clearing)
 */
void cLCD_ReturnHomeWithoutShifting(void);

/**
 * @brief store a custom code generated pattern to CGRAM.
 *
 * @param[in] pattern, an 8-elements array.
 * @param[in] addr, DDRAM address for generated pattern; should be between 0 and 7.
 * (CGRAM address = addr << 3).
 */
void cLCD_GenPattern(const uint8_t *pattern, uint8_t addr);

#endif /* CLCD_H_ */
