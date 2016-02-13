/**
 * @file sim900_testing_cmds.h
 * @author  Nguyen Dinh Trung Truc  <truc.ndtt@gmail.com>.
 * @version 1.0
 * @date 12-Feb-2016
 * @brief Header file for Sim900 EBoard testing shell commands.
 */

#ifndef SIM900_TESTING_CMDS_H_
#define SIM900_TESTING_CMDS_H_

/**
 * @brief   Sim900 EBoard testing cmd.
 *
 * @details Usage:
 *       sim900_test -i, initialize hardware and data for the test.
 *       sim900_test -d, deinitialize hardware and data.
 *       sim900_test -f, perform full test.
 *       sim900_test -h, print the usage.
 *       Press ESC to stop the test.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void sim900_test(int argc, char** argv);



#endif /* SIM900_TESTING_CMDS_H_ */
