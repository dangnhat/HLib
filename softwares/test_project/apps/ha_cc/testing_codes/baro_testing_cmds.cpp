/*
 * baro_testing_cmds.cpp
 *
 *  Created on: Aug 28, 2015
 *      Author: nvhien1992
 */
#include "baro_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

#define BMP180_ADDRESS (0x77) //7-bits address
#define BMP180_I2C_Speed (100000) //100 kHz standard mode

using namespace testing_ns;

static void baro_testing_init(void);

static void baro_testing_deinit(void);

static void baro_write_read_test(void);

static void baro_full_test(void);

static i2c *MB1_i2c_p = &MB1_I2C2;

static i2c_ns::i2c_params_t i2c_params = {
BMP180_I2C_Speed,
i2c_ns::mode_i2c,
i2c_ns::dc_2,
i2c_ns::ack_enable,
i2c_ns::acked_address_7bit,
BMP180_ADDRESS << 1, // BMP180 7-bit adress = 0x77, 8-bit address = 0xEE;
};

/* Shell command usages */
static const char baro_test_usage[] = "Usage:\n"
        "baro_test -i, initialize hardware and data for the test.\n"
        "baro_test -d, de-initialize hardware and data for the test.\n"
        "baro_test -b, write/read register of barometer sensor test.\n"
        "baro_test -f, perform full test = baro_test -i -b.\n"
        "baro_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

void baro_test(int argc, char** argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", baro_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    baro_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    baro_testing_deinit();
                    break;

                case 'g':
                    /* Write/Read register test*/
                    baro_write_read_test();
                    break;

                case 'f':
                    /* Full test */
                    baro_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", baro_test_usage);
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

static void baro_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for BMP180 tests ***\n"
            "\tInitialize I2C%u.\n", MB1_i2c_p->get_used_i2c());

    MB1_i2c_p->init(&i2c_params);
}

static void baro_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "\tI2C%u will be shut down.\n", MB1_i2c_p->get_used_i2c());

    MB1_i2c_p->deinit();
}

static void baro_write_read_test(void)
{
    uint8_t baro_eeprom_reg = 0xF6;
    HA_NOTIFY("\n*** Test read register at address: 0x%x ***\n",
            baro_eeprom_reg);

    uint8_t reg_readback[2] = { 0, 0 };
    MB1_i2c_p->master_receive(BMP180_ADDRESS, baro_eeprom_reg,
            reg_readback, 2);
    HA_NOTIFY("\tData readback at 0xF6: 0x%x, at 0xF7: 0x%x\n", reg_readback[0],
            reg_readback[1]);
}

static void baro_full_test(void)
{
    baro_testing_init();
    baro_write_read_test();
}
