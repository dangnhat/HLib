/**
 * @file xbee_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-July-2015
 * @brief Header file for XBee testing shell commands.
 */

#ifndef XBEE_TESTING_CMDS_H_
#define XBEE_TESTING_CMDS_H_

/**
 * @brief   XBee module testing cmd.
 *
 * @details Usage:  xbee_test -i, initialize hardware and data for the test.
 *                  xbee_test -r, ERx LED test, a 8 bit data (b'01010101)
 *                  will be sent to XBee module every 500ms.
 *                  xbee_test -h, print the usage.
 *                  Press ESC to stop.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void xbee_test(int argc, char** argv);

#endif /* XBEE_TESTING_CMDS_H_ */
