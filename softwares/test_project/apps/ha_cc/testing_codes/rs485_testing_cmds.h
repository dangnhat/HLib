/**
 * @file rs485_testing_cmds.h
 * @author  Huynh Van Minh  <huynhminh009@gmail.com>.
 * @version 1.0
 * @date 29-July-2015
 * @brief Header file for RS485 testing shell commands.
 */

#ifndef RS485_TESTING_CMDS_H_
#define RS485_TESTING_CMDS_H_

/**
 * @brief   RS485 module testing cmd.
 *
 * @details Usage:  rs485_test -i, initialize hardware and data for the test.
 * 					rs485_test -d, deinitialize hardware and data for the test.
 *                  rs485_test -c, RS485 and MBoard communication test.
 *                  rs485_test -f, perform full test = ble_test -i -c.
 *                  rs485_test -h, print the usage.
 *                  Press ESC to stop.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void rs485_test(int argc, char** argv);

#endif /* RS485_TESTING_CMDS_H_ */
