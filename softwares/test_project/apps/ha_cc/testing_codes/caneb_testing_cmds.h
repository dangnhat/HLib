/**
 * @file spieb_testing_cmds.h
 * @author  Nguyen Dinh Trung Truc  <truc.ndtt@gmail.com>.
 * @version 1.0
 * @date 11-Jan-2016
 * @brief Header file for CAN EBoard testing shell commands.
 */

#ifndef CANEB_TESTING_CMDS_H_
#define CANEB_TESTING_CMDS_H_

/**
 * @brief   CAN EBoard testing cmd.
 *
 * @details Usage:
 *       caneb_test -i, initialize hardware and data for the test.
 *       caneb_test -d, deinitialize hardware and data.
 *       caneb_test -s, full test as a slave.
 *       caneb_test -m, full test as a master.
 *       caneb_test -h, print the usage.
 *       Press ESC to stop the test.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void caneb_test(int argc, char** argv);



#endif /* CANEB_TESTING_CMDS_H_ */
