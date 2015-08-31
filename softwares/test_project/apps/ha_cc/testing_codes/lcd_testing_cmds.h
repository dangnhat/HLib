/*
 * lcd_testing_cmds.h
 *
 *  Created on: Aug 11, 2015
 *      Author: minh
 */

#ifndef TESTING_CODES_LCD_TESTING_CMDS_H_
#define TESTING_CODES_LCD_TESTING_CMDS_H_

/**
 * @brief   LCD module testing cmd.
 *
 * @details Usage:
 *      lcd_test -i, initialize hardware and data for the test.
 *      lcd_test -d, deinitialize hardware and data for the test.
 *      lcd_test -f, perform full test.
 *      lcd_test -h, print the usage.
 *      Press ESC to stop the test.;
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */

void lcd_test(int argc, char** argv);

#endif /* TESTING_CODES_LCD_TESTING_CMDS_H_ */
