/**
 * @file eth_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 13-Aug-2015
 * @brief Header file for ETH testing shell commands.
 */

#ifndef ETH_TESTING_CMDS_H_
#define ETH_TESTING_CMDS_H_

/**
 * @brief   ETH module testing cmd.
 *
 * @details Usage:
 *      eth_test -i, initialize hardware and data for the test.
 *      eth_test -d, deinitialize hardware and data for the test.
 *      eth_test -c, ETH IC and MBoard communication test.
 *      eth_test -j, communication through RJ45 test.
 *      eth_test -f, perform full test = ble_test -i -c -j.
 *      eth_test -h, print the usage.
 *      Press ESC to stop the test.;
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void eth_test(int argc, char** argv);

#endif /* ETH_TESTING_CMDS_H_ */
