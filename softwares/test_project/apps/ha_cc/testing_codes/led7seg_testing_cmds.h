/*
 * led7seg_testing_cmds.h
 *
 *  Created on: Aug 9, 2015
 *      Author: Minh
 */

#ifndef TESTING_CODES_LED7SEG_TESTING_CMDS_H_
#define TESTING_CODES_LED7SEG_TESTING_CMDS_H_

/**
 * @brief   LED7SEG module testing cmd.
 *
 * @details Usage:
 *      led7seg_test -i, initialize hardware and data for the test.
 *      led7seg_test -d, deinitialize hardware and data for the test.
 *      led7seg_test -a, test all led.
 *      led7seg_test -s, test led7seg scan.
 *      led7seg_test -f, perform full test = led7seg_test -i -a -s -d.
 *      led7seg_test -h, print the usage.
 *      Press ESC to stop the test.;
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void led7seg_test(int argc, char** argv);



#endif /* TESTING_CODES_LED7SEG_TESTING_CMDS_H_ */
