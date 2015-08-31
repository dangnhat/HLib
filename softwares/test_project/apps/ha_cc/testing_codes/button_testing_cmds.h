/**
 * @file button_testing_cmds.h
 * @author  Huynh Van Minh  <huynhminh009@gmail.com>.
 * @version 1.0
 * @date 29-July-2015
 * @brief Header file for BUTTON testing shell commands.
 */

#ifndef BUTTON_TESTING_CMDS_H_
#define BUTTON_TESTING_CMDS_H_

/**
 * @brief   BUTTON module testing cmd.
 *
 * @details Usage:  button_test -i, initialize hardware and data for the test.
 *                  button_test -f,perform full test.
 *                  button_test -h, print the usage.
 *                  Press ESC to stop.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void button_test(int argc, char** argv);

#endif /* BUTTON_TESTING_CMDS_H_ */
