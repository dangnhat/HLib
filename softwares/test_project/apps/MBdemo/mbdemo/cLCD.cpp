/*
 * cLCD.c
 *
 *  Created on: Feb 2, 2015
 *      Author: nvhie_000
 */
#include <stdarg.h>
#include "cLCD.h"
#include "MB1_GPIO.h"

#define SYS_CLK 72 //MHz
//#define LCD20x4
#define LCD16x2

#ifdef LCD20x4
#define MAX_CHAR_IN_LINE 20
#define MAX_LINE 4
#endif

#ifdef LCD16x2
#define MAX_CHAR_IN_LINE 16
#define MAX_LINE 2
#endif

/*--------------------- LCD-specific data, definitions -----------------------*/
/** register select **/
#define RS_CMD()    lcdRS.gpio_reset()//LCD_RS_ClrVal(NULL)
#define RS_DATA()   lcdRS.gpio_set()//LCD_RS_SetVal(NULL)
/** lcd EN **/
#define RW_EN()     lcdEN.gpio_set()//LCD_EN_SetVal(NULL)
#define RW_DIS()    lcdEN.gpio_reset()//LCD_EN_ClrVal(NULL)
/** data pins **/
#define SET_DB7()   lcdD7.gpio_set()//LCD_DB7_SetVal(NULL)
#define SET_DB6()   lcdD6.gpio_set()//LCD_DB6_SetVal(NULL)
#define SET_DB5()   lcdD5.gpio_set()//LCD_DB5_SetVal(NULL)
#define SET_DB4()   lcdD4.gpio_set()//LCD_DB4_SetVal(NULL)
#define CLR_DB7()   lcdD7.gpio_reset()//LCD_DB7_ClrVal(NULL)
#define CLR_DB6()   lcdD6.gpio_reset()//LCD_DB6_ClrVal(NULL)
#define CLR_DB5()   lcdD5.gpio_reset()//LCD_DB5_ClrVal(NULL)
#define CLR_DB4()   lcdD4.gpio_reset()//LCD_DB4_ClrVal(NULL)
/** backlight pin **/
#define BL_ON()     lcdBL.gpio_set()//LCD_BL_SetVal(NULL)
#define BL_OFF()    lcdBL.gpio_reset()//LCD_BL_ClrVal(NULL)
/** rw en/dis **/
//#define READ()      LCD_RW_SetVal(NULL)
//#define WRITE()     LCD_RW_ClrVal(NULL)

/* define cLCD's pin state */
enum RS_state_e {
    cmd = 0, data = 1
};
/* end define pin state */

/** define cLCD's instructions **/
enum instruction_e {
    /* clear Display :
     * Write 20H to DDRAM and set DDRAM address to 00h from AC.
     * RS = 0, RW = 0.
     * 1.5 ms.
     */
    cLCD_cmd_ClearDisplay = 0x01,
    /* return Home :
     * set DDRAM address to 00h from AC, return cursor to it's original location if shifted (contents of DDRAM not changed).
     * RS = 0, RW = 0.
     * 1.5 ms.
     */
    cLCD_cmd_ReturnHome = 0x02,
    /* Entry mode set :
     * Set cursor move direction and specifies display shift.
     * RS = 0, RW = 0.
     * 37 us.
     */
    cLCD_cmd_EntryMode_inc = 0x06,
    cLCD_cmd_EntryMode_dec = 0x04,
    cLCD_cmd_EntryMode_SL = 0x07,
    cLCD_cmd_EntryMode_SR = 0x05,
    /* Display ON/OFF :
     * RS = 0, RW = 0, 0 0 0 0 1 D C B
     * D : display on (1) / off (0)
     * C : cursor on (1) / off (0)
     * B : cursor blink on (1) / off (0)
     * 37 us
     */
    cLCD_cmd_DCB_off = 0x08,
    cLCD_cmd_D_on = 0x0C,
    cLCD_cmd_C_on = 0x0A,
    cLCD_cmd_B_on = 0x09,
    /* Cursor or display shift :
     * shift cursor/display left/right
     * RS = 0, RW = 0, 0 0 0 1 S/C R/L x x.
     * 37 us.
     */
    cLCD_cmd_cursorSL = 0x10, // AC-1
    cLCD_cmd_cursorSR = 0x14, // AC+1
    cLCD_cmd_displaySL = 0x18, // AC = AC
    cLCD_cmd_displaySR = 0x1C, // AC = AC
    /* Function set :
     * RS = 0, RW = 0, 0 0 1 DL N F x x
     * DL : 1 - 8bit, 0 - 4bit.
     * N : 1 - 2 line, 0 - 1 line.
     * F : 1 - 5x11 dots, 0 - 5x8 dots.
     * 37 us.
     */
    cLCD_cmd_FS_4bit_2line = 0x28,
    /* Set CGRAM Address :
     * RS = 0, RW = 0, 0 1 AC5 AC4 AC3 AC2 AC1 AC0.
     * Set CGRAM address to Ac.
     * 37us.
     */
    //TODO
    /* Set DDRAM Address :
     * RS = 0, RW = 0, 1 AC6 AC5 AC4 AC3 AC2 AC1 AC0
     * Set DDRAM address to Ac.
     * 1 line mode: ?
     * 2 line mode:
     * _20x4:
     *      00 - 13 (line 1, pos 1-20)
     *      40 - 53 (line 2)
     *      14 - 27 (line 3)
     *      54 - 67 (line 4)
     * _16x2:
     *      00 - 0F (line 1, pos 1-16)
     *      40 - 4F (line 2)
     * 37 us.
     */
    cLCD_DDRAM_line1 = 0x00,
    cLCD_DDRAM_line2 = 0x40,
    //LCD20x4
    cLCD_DDRAM_line3 = 0x14,
    cLCD_DDRAM_line4 = 0x54,
    /* Read busy flag and AC
     * RS = 0, RW = 1, 0us, output BF AC6 - AC0
     * BF = 0 : not busy.
     */
    //TODO
    /* Write data to RAM :
     * RS = 1, RW = 1, D7-D0 : data.
     * After write AC +/- 1.
     */
    //TODO
    /* Read data from RAM 
     */
    //TODO
};

gpio lcdRS;
gpio lcdEN;
gpio lcdBL;
gpio lcdD4;
gpio lcdD5;
gpio lcdD6;
gpio lcdD7;

/* private variables */
static uint8_t currentLine = 1;
static uint8_t currentPos = 1;

/* declare private functions */
static void WaitForLcd(void);
static void Send4bits(uint8_t RS, uint8_t D7D4);
static void SendByteWithoutWaiting(uint8_t RS, uint8_t D7D0);
static void SendByte(uint8_t RS, uint8_t D7D0);
static void delay_us(uint32_t usec);

/* implement private functions */
static void WaitForLcd(void) {
    /* waiting for lcd in 0.12ms */
    delay_us(120);
}

static void Send4bits(uint8_t RS, uint8_t D7D4) {
    /* cmd = 0; data = 1 */
    RS ? RS_DATA() : RS_CMD();

    /* addition delay */
    delay_us(1);

    /* EN = 1 */RW_EN();

    /* Send data */
    ((D7D4 & 0x80) >> 7) ? SET_DB7() : CLR_DB7();
    ((D7D4 & 0x40) >> 6) ? SET_DB6() : CLR_DB6();
    ((D7D4 & 0x20) >> 5) ? SET_DB5() : CLR_DB5();
    ((D7D4 & 0x10) >> 4) ? SET_DB4() : CLR_DB4();

    /* addition delay */
    delay_us(1);

    /* EN = 0 */
    RW_DIS();
}

static void SendByteWithoutWaiting(uint8_t RS, uint8_t D7D0) {
    RS ? RS_DATA() : RS_CMD();

    /* addition delay */
    delay_us(1);

    /* Send 4 MSB bits D7-D4 */
    /* EN = 1 */RW_EN();

    ((D7D0 & 0x80) >> 7) ? SET_DB7() : CLR_DB7();
    ((D7D0 & 0x40) >> 6) ? SET_DB6() : CLR_DB6();
    ((D7D0 & 0x20) >> 5) ? SET_DB5() : CLR_DB5();
    ((D7D0 & 0x10) >> 4) ? SET_DB4() : CLR_DB4();

    /* addition delay */
    delay_us(1);

    /* EN = 0 */RW_DIS();

    /* tEC > 1.2 us */
    delay_us(2);

    /* Send next 4 bits D3-D0 */
    /* EN = 1 */RW_EN();

    ((D7D0 & 0x08) >> 3) ? SET_DB7() : CLR_DB7();
    ((D7D0 & 0x04) >> 2) ? SET_DB6() : CLR_DB6();
    ((D7D0 & 0x02) >> 1) ? SET_DB5() : CLR_DB5();
    (D7D0 & 0x01) ? SET_DB4() : CLR_DB4();

    /* addition delay */
    delay_us(1);

    /* EN = 0 */
    RW_DIS();
}

static void SendByte(uint8_t RS, uint8_t D7D0) {
    /* wait for lcd is not busy */
    WaitForLcd();

    /* send data */
    SendByteWithoutWaiting(RS, D7D0);
}

static void delay_us(uint32_t usec) {
    uint32_t num_clocks = SYS_CLK * usec;
    uint32_t count = 0;

    for (count = 0; count < num_clocks; count++) {
        __asm__ __volatile__("nop");
    }
}
/* end implementing private functions */

/* implement public functions */
bool cLCD_Init(void) {
    /* only write mode */
    //WRITE();

    /* Set back light BL = 1 */
    BL_ON();

    /* wait for >40ms */
    delay_us(100000);

    /* send function set 1 */
    Send4bits(cmd, 0x30);
    /* wait for >37us */
    delay_us(50);

    /* send function set 2 */
    SendByteWithoutWaiting(cmd, cLCD_cmd_FS_4bit_2line);
    /* wait for >37us */
    delay_us(50);

    /* send function set 3 */
    SendByteWithoutWaiting(cmd, cLCD_cmd_FS_4bit_2line);
    /* wait for >37us */
    delay_us(50);

    /* send display control, Display/Blink_cursor on */
    SendByte(cmd, cLCD_cmd_D_on | cLCD_cmd_B_on);

    /* send display clear, clear all DDRAM to 20H, set DDRAM address to 0 from AC */
    SendByte(cmd, cLCD_cmd_ClearDisplay);

    /* send Entry mode set, AC++ */
    SendByte(cmd, cLCD_cmd_EntryMode_inc);

    /* additional delay */
    delay_us(1000);

    return true ;
}

bool cLCD_SetCursor(uint8_t line, uint8_t pos) {
    uint8_t ddram_addr = 0;

    /* check line and pos */
    if ((line < 1) || (line > MAX_LINE)) {
        return false ;
    }
    if ((pos < 1) || (pos > MAX_CHAR_IN_LINE)) {
        return false ;
    }

    /* calculate DDRAM address */
    switch (line) {
    case 1:
        ddram_addr = cLCD_DDRAM_line1 + pos - 1;
        break;
    case 2:
        ddram_addr = cLCD_DDRAM_line2 + pos - 1;
        break;
        /* LCD 20x4 */
    case 3:
        ddram_addr = cLCD_DDRAM_line3 + pos - 1;
        break;
    case 4:
        ddram_addr = cLCD_DDRAM_line4 + pos - 1;
        break;
    default:
        return false ;
    }
    /* send set DDRAM address command */
    SendByte(cmd, ddram_addr | 0x80);

    /* save state */
    currentLine = line;
    currentPos = pos;

    return false ;
}

void cLCD_Putc(char aChar) {
    uint8_t tab_pos;

    /* special characters */
    switch (aChar) {
    case '\n':
        if (currentLine < MAX_LINE) {
            cLCD_SetCursor(++currentLine, 1);
        }
        break;
    case '\t':
        if (currentPos % 2 == 0) {
            tab_pos = currentPos + 2;
        } else {
            tab_pos = (currentPos / 2 + 1) * 2;
        }
        if (tab_pos < MAX_CHAR_IN_LINE) {
            cLCD_SetCursor(currentLine, tab_pos);
        }
        break;
    case '\r':
        cLCD_SetCursor(currentLine, 1);
        break;
    default:
        /* send data */
        SendByte(data, aChar);

        /* save state */
        currentPos++;
        if (currentPos > MAX_CHAR_IN_LINE) {
            currentPos = 1;
            currentLine++;
            if (currentLine > MAX_LINE) {
                currentLine = 1;
            }
            cLCD_SetCursor(currentLine, currentPos);
        }
        break;
    }
}

int16_t cLCD_Puts(char *aString) {
    int16_t numChar = 0;

    while (*aString != '\0') {
        cLCD_Putc(*aString++);
        numChar++;
    }

    return numChar;
}

int16_t cLCD_Printf(const char *format, ...) {
    char buffer[MAX_CHAR_IN_LINE * MAX_LINE + 1];
    int16_t retval;
    va_list args;

    va_start(args, format);
    vsnprintf((char*) buffer, MAX_CHAR_IN_LINE * MAX_LINE + 1, format, args);
    va_end(args);

    /* print buffer to lcd */
    retval = cLCD_Puts(buffer);

    return retval;
}

void cLCD_Clear(void) {
    /* send clear command */
    SendByte(cmd, cLCD_cmd_ClearDisplay);

    /* save state */
    currentLine = 1;
    currentPos = 1;

    /* additional delay */
    delay_us(50);
}

void cLCD_ReturnHome(void) {
    /* send clear command */
    SendByte(cmd, cLCD_cmd_ReturnHome);

    /* save state */
    currentLine = 1;
    currentPos = 1;
}

void cLCD_ReturnHomeWithoutShifting(void) {
    cLCD_SetCursor(1, 1);
}

void cLCD_GenPattern(const uint8_t *pattern, uint8_t addr) {
    uint8_t count;

    /* check addr */
    if (addr > 7) {
        return;
    }

    /* send set CGRAM address command */
    SendByte(cmd, (addr << 3) | 0x40);

    /* send pattern */
    for (count = 0; count < 8; count++) {
        SendByte(data, pattern[count]);
    }

    /* set cursor to current state */
    cLCD_SetCursor(currentLine, currentPos);
}
