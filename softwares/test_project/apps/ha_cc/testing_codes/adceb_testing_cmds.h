/**
 * @file adceb_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 28-Aug-2015
 * @brief Header file for ADC EBoard testing shell commands.
 */

#ifndef ADCEB_TESTING_CMDS_H_
#define ADCEB_TESTING_CMDS_H_

/**
 * @brief   ADC EBoard testing cmd.
 *
 * @details Usage:
 *      adceb_test -i, initialize hardware and data for the test.
 *      adceb_test -d, deinitialize hardware and data.
 *      adceb_test -p, Photo-resistor test.
 *      adceb_test -l, TSL12T light sensor IC test.
 *      adceb_test -n, NTC test.
 *      adceb_test -t, LMT8x temperature IC test.
 *      adceb_test -f, perform full test = adceb_test -i -p -l -n -t -d.
 *      adceb_test -h, print the usage.
 *      Press ESC to stop the test.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void adceb_test(int argc, char** argv);

#endif /* SPIEB_TESTING_CMDS_H_ */
