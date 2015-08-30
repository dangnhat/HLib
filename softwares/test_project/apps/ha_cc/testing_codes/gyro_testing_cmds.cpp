/*
 * gyro_testing_cmds.cpp
 *
 *  Created on: Aug 26, 2015
 *      Author: nvhien1992
 */
#include "gyro_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

#define MPU6050_DEFAULT_ADDRESS (0x68) //7-bits address
#define MPU6050__ADDRESS_AD0_HIGH (0x69)
#define MPU6050_I2C_Speed (100000) //100 kHz standard mode

using namespace testing_ns;

static void gyro_testing_init(void);

static void gyro_testing_deinit(void);

static void gyro_testing_AD0(void);

static void gyro_write_read_test(uint8_t addr);

static void gyro_full_test(void);

static I2C *MB1_i2c_p = &MB1_I2C2;

static I2C_InitTypeDef i2c_init_structure = {
MPU6050_I2C_Speed,
I2C_Mode_I2C,
I2C_DutyCycle_2,
MPU6050_DEFAULT_ADDRESS << 1, // MPU6050 7-bit address = 0x68, 8-bit address = 0xD0;
I2C_Ack_Enable,
I2C_AcknowledgedAddress_7bit };

static gpio addr_ctrl;
static const gpio_ns::gpio_params_t addr_ctrl_params = { gpio_ns::port_A, 15,
        gpio_ns::out_push_pull, gpio_ns::speed_2MHz, };

static gpio gyro_int;
static const gpio_ns::gpio_params_t gyro_int_params = { gpio_ns::port_B, 3,
        gpio_ns::in_pull_down, gpio_ns::speed_2MHz, };

/* Shell command usages */
static const char gyro_test_usage[] = "Usage:\n"
        "gyro_test -i, initialize hardware and data for the test.\n"
        "gyro_test -d, de-initialize hardware and data for the test.\n"
        "gyro_test -a, choose slave address using AD0 pin.\n"
        "gyro_test -g, write/read register of gyroscope sensor with default address.\n"
        "gyro_test -f, perform full test = gyro_test -i -a.\n"
        "gyro_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

void gyro_test(int argc, char** argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", gyro_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    gyro_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    gyro_testing_deinit();
                    break;

                case 'a':
                    gyro_testing_AD0();
                    break;

                case 'g':
                    addr_ctrl.gpio_reset();
                    /* Write/Read register test*/
                    gyro_write_read_test(MPU6050_DEFAULT_ADDRESS);
                    break;

                case 'f':
                    /* Full test */
                    gyro_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", gyro_test_usage);
                    return;

                default:
                    HA_NOTIFY("Err: unknown option.\n");
                    return;
                }
            } else { /* path */
                break;
            }
        } /* end for */
    }
}

static void gyro_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for MPU6050 tests ***\n"
            "\tInitialize: I2C%u, AD0 pin.\n", MB1_i2c_p->get_used_i2c());

    MB1_i2c_p->reinit(&i2c_init_structure);
    addr_ctrl.gpio_init(&addr_ctrl_params);
    gyro_int.gpio_init(&gyro_int_params);
}

static void gyro_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "\tI2C%u, AD0 pin will be shut down.\n", MB1_i2c_p->get_used_i2c());

    MB1_i2c_p->deinit();
    addr_ctrl.gpio_shutdown();
    gyro_int.gpio_shutdown();
}

static void gyro_testing_AD0(void)
{
    HA_NOTIFY("\n*** Testing AD0 pin (set address) ***\n");

    HA_NOTIFY("\n[1] Choose address 0x68 using AD0 pin and test write/read:\n");
    addr_ctrl.gpio_reset();
    gyro_write_read_test(MPU6050_DEFAULT_ADDRESS);

    HA_NOTIFY("\n[2] Choose address 0x69 using AD0 pin and test write/read:\n");
    addr_ctrl.gpio_set();
    gyro_write_read_test(MPU6050__ADDRESS_AD0_HIGH);
}

static void gyro_write_read_test(uint8_t addr)
{
    uint8_t gyro_config[2] = { 0x1B, 0x18 };
    HA_NOTIFY("\n*** Test wr/rd register at address: 0x%x with data 0x%x ***\n",
            gyro_config[0], gyro_config[1]);
    if (!MB1_i2c_p->master_send_to(addr, gyro_config, 2,
            true)) {
        HA_NOTIFY("\t[ERR] Write fail!\n");
        return;
    }

    uint8_t reg_readback = 0;
    if (!MB1_i2c_p->master_receive_from(addr, gyro_config[0],
            &reg_readback, 1)) {
        HA_NOTIFY("\t[ERR] Read fail!\n");
        return;
    }
    HA_NOTIFY("\tRegister readback: 0x%x\n", reg_readback);
}

static void gyro_full_test(void)
{
    gyro_testing_init();
    gyro_testing_AD0();
}
