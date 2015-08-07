/**
 * @file ble_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 5-Aug-2015
 * @brief Header file for BLE testing shell commands.
 */

#ifndef BLE_TESTING_CMDS_H_
#define BLE_TESTING_CMDS_H_

/**
 * @brief   BLE module testing cmd.
 *
 * @details Usage:
 *      ble_test -i, initialize hardware and data for the test.
 *      ble_test -d, deinitialize hardware and data for the test.
 *      ble_test -w, warm reset test.\n"
 *      ble_test -c, BLE and MBoard communication test.
 *      ble_test -f, perform full test = ble_test -i -c -w.
 *      ble_test -h, print the usage.
 *      Press ESC to stop the test.;
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void ble_test(int argc, char** argv);

#endif /* BLE_TESTING_CMDS_H_ */
