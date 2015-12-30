/**
 * @file i2ceb_testing_cmds.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Aug-2015
 * @brief Source file for I2C EBoard testing shell commands.
 */

#include "i2ceb_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"
#include "string.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;
using namespace i2c_ns;

/* Configuration data */
/* I2C */
static i2c* MB1_i2c_ptr = &MB1_I2C2;
static const i2c_params_t i2c_params = {
        100000,
        mode_i2c,
        dc_2,
        ack_enable,
        acked_address_7bit,
        0xFF,
};

/* I2C address */
enum i2c_device_address_e: uint8_t {
    eeprom_addr = 0x50, /* Only 4 MSB is used */
    tmp_addr = 0x48,
    light_addr = 0x39,
};

/* AT24C128 EEPROM data */

const uint8_t eeprom_max_datasize = 8;
const uint16_t eeprom_start_addr = 0;

/* Light sensor data */
enum lightic_cmd_e: uint8_t {
    cmd_prefix = 0x80,
    int_clear = 0x40, /* bit 6 = 1 means clearing pending INT */
    word_protocol = 0x20,

    control_reg_addr = 0x00,
    timming_reg_addr = 0x01,
    threslow_low_reg_addr = 0x02,
    threslow_high_reg_addr = 0x03,
    threshigh_low_reg_addr = 0x04,
    threshigh_high_reg_addr = 0x05,
    int_reg_addr = 0x06,
    id_reg_addr = 0x0A,
    data0_low_reg_addr = 0x0C,
    data0_high_reg_addr = 0x0D,
    data1_low_reg_addr = 0x0E,
    data1_high_reg_addr = 0x0F,
};

/* INT pin */
static const gpio_ns::gpio_params_t lint_params = {
        gpio_ns::port_C,
        10,
        gpio_ns::in_pull_up,
        gpio_ns::speed_10MHz,
};
static gpio MB1_lint_pin;

/* Temperature sensor (TMP121/TMP122) data */
enum tmpic_address_e: uint8_t {
    tmp_register_pointer_addr = 0x00, /* Read only */
    conf_register_pointer_addr = 0x01, /* R/W */
    tlow_register_pointer_addr = 0x02, /* R/W */
    thigh_register_pointer_addr = 0x03, /* R/W */
};

static const float resolution_step = 0.0625;

/* ALERT pin */
static const gpio_ns::gpio_params_t tarlt_params = {
        gpio_ns::port_A,
        15,
        gpio_ns::in_pull_up,
        gpio_ns::speed_10MHz,
};
static gpio MB1_tarlt0_pin;

/* DS18B20 data */
enum ds18b20_ins_e: uint8_t {
    ds18b20_12bit_res = 3,
    ds18b20_rom_skip = 0xCC,
    ds18b20_convert = 0x44,
    ds18b20_write_scr = 0x4E,
    ds18b20_read_scr = 0xBE,
};

/* 1-wired pin */
static gpio_ns::gpio_params_t onewired_params = {
        gpio_ns::port_B,
        3,
        gpio_ns::out_push_pull,
        gpio_ns::speed_50MHz,
};
static gpio MB1_1wired_pin;

/* End configuration data */

/* Shell command usages */
static const char i2ceb_test_usage[] = "Usage:\n"
        "i2ceb_test -i, initialize hardware and data for the test.\n"
        "i2ceb_test -d, deinitialize hardware and data.\n"
        "i2ceb_test -e, I2C EEPROM communication test.\n"
        "i2ceb_test -w, 1-wired IC communication test.\n"
        "i2ceb_test -l, Light sensor IC communication test.\n"
        "i2ceb_test -t, Temperature IC communication test.\n"
        "i2ceb_test -f, perform full test = spieb_test -i -e -w -l -t -d.\n"
        "i2ceb_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for SPI EBoard.
 */
static void i2ceb_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for SPI EBoard.
 */
static void i2ceb_testing_deinit(void);

/**
 * @brief   MBoard-1 and EEPROM communication test.
 *          First, make sure that nWP jumper is not connected.
 *          Random data will be written from address 0x00 to 0x07 and read back.
 *          Written data and read data should be the same.
 *
 *          Then, connect nWP jumper.
 *          Random data will be written from address 0x00 to 0x07 and read back.
 *          Read data will stay the same regardless of written data.
 */
static void i2ceb_eeprom_communication_test(void);

/**
 * @brief
 */
static void i2c_lightic_test(void);

/**
 * @brief   MBoard-1 and TMP(2)75 communication test.
 */
static void i2ceb_tmpic_test(void);

/**
 * @brief   MBoard and DS18B20 1-wired communication test
 */
static void i2ceb_1wired_test(void);

/**
 * @brief   Master MBoard creates a reset pulse to start a command
 */
static void ds18b20_init(void);

/**
 * @brief   Write a byte to IC DS18B20
 *
 * @param[in]   data, the byte value to be written
 */
static void ds18b20_write_byte(uint8_t data);

/**
 * @brief   Write a command to IC DS18B20
 *
 * @param[in]   data, the command to be written
 */
static void ds18b20_write_cmd(uint8_t data);

/**
 * @brief   Read a byte from IC DS18B20
 *
 * @return  the read value
 */
static uint8_t ds18b20_read_byte(void);

/**
 * @brief   Full test.
 */
static void i2ceb_full_test(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void i2ceb_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", i2ceb_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    i2ceb_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    i2ceb_testing_deinit();
                    break;

                case 'e':
                    /* EEPROM Communication test*/
                    i2ceb_eeprom_communication_test();
                    break;

                case 'l':
                    /* Light sensor communication test */
                    i2c_lightic_test();
                    break;

                case 't':
                    /* Temperature communication test */
                    i2ceb_tmpic_test();
                    break;

                case 'w':
                    /* 1-wired communication test */
                    i2ceb_1wired_test();
                    break;

                case 'f':
                    /* Full test */
                    i2ceb_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", i2ceb_test_usage);
                    return;

                default:
                    HA_NOTIFY("Err: unknown option.\n");
                    return;
                }
            }
            else { /* path */
                break;
            }
        } /* end for */
    }
}

/* Private function implementations */
/*----------------------------------------------------------------------------*/
static void i2ceb_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for I2C EB tests ***\n"
            "I2C: %u, Speed(Hz): %lu,\n"
            "Temperature ALERT pin: port: %u (port A = 0,...), pin: %u\n"
            "Light INT pin: port: %u (port A = 0,...), pin: %u\n"
            "1-Wired pin: port: %u (port A = 0,...), pin: %u\n",
            MB1_i2c_ptr->get_used_i2c(), i2c_params.clock_speed,
            tarlt_params.port, tarlt_params.pin,
            lint_params.port, lint_params.pin,
            onewired_params.port, onewired_params.pin);

    /* Init SPI and GPIOs */
    MB1_i2c_ptr->init(&i2c_params);
    MB1_tarlt0_pin.gpio_init(&tarlt_params);
    MB1_lint_pin.gpio_init(&lint_params);
    MB1_1wired_pin.gpio_init(&onewired_params);
}

/*----------------------------------------------------------------------------*/
static void i2ceb_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "I2C: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            MB1_i2c_ptr->get_used_i2c());

    /* Shutdown USART and GPIOs */
    MB1_i2c_ptr->deinit();
    MB1_tarlt0_pin.gpio_shutdown();
    MB1_lint_pin.gpio_shutdown();
    MB1_1wired_pin.gpio_shutdown();
}

/*----------------------------------------------------------------------------*/
static void i2ceb_eeprom_communication_test(void)
{
    uint8_t temp = 0, count;
    uint8_t buffer[eeprom_max_datasize + 2];

    start_waiting_esc_character();

    HA_NOTIFY("\n*** EEPROM AT24C128 & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Write */
    HA_NOTIFY("Make sure nWP jumper on the board is not shorted.\n"
            "Press ESC to continue.\n");
    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    while (1) {
        /* Set up address (MSB first) */
        buffer[0] = eeprom_start_addr >> 8;
        buffer[1] = eeprom_start_addr & 0xFF;
        HA_NOTIFY("\nWRITE(0x%04x): ", eeprom_start_addr);

        for (count = 0; count < eeprom_max_datasize; count++) {
            buffer[count + 2] = temp;
            HA_NOTIFY("%x ", buffer[count + 2]);
            temp++;
        }
        HA_NOTIFY("\n");

        MB1_i2c_ptr->master_send(eeprom_addr, buffer, eeprom_max_datasize + 2, true);

        testing_delay_us(100000); /* 100ms */

        /* Read back */
        memset(buffer, 0, eeprom_max_datasize + 2);
        buffer[0] = eeprom_start_addr >> 8;
        buffer[1] = eeprom_start_addr & 0xFF;

        HA_NOTIFY("\nREAD(0x%04x): ", eeprom_start_addr);
        MB1_i2c_ptr->master_send(eeprom_addr, buffer, 2, false);
        MB1_i2c_ptr->master_receive_bare(eeprom_addr, &buffer[2], eeprom_max_datasize);
        for (count = 0; count < eeprom_max_datasize; count++) {
            HA_NOTIFY("%x ", buffer[count + 2]);
        }
        HA_NOTIFY("\n");

        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }
    stop_waiting_esc_character();
    start_waiting_esc_character();

    HA_NOTIFY("\nConnect nWP jumper on board, READ value will not change.\n"
            "Press ESC to continue.\n");
    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    while (1) {
        /* Set up address (MSB first) */
        buffer[0] = eeprom_start_addr >> 8;
        buffer[1] = eeprom_start_addr & 0xFF;
        HA_NOTIFY("\nWRITE(0x%04x): ", eeprom_start_addr);

        for (count = 0; count < eeprom_max_datasize; count++) {
            buffer[count + 2] = temp;
            HA_NOTIFY("%x ", buffer[count + 2]);
            temp++;
        }
        HA_NOTIFY("\n");

        MB1_i2c_ptr->master_send(eeprom_addr, buffer, eeprom_max_datasize + 2, true);

        testing_delay_us(100000); /* 100ms */

        /* Read back */
        memset(buffer, 0, eeprom_max_datasize + 2);
        buffer[0] = eeprom_start_addr >> 8;
        buffer[1] = eeprom_start_addr & 0xFF;

        HA_NOTIFY("\nREAD(0x%04x): ", eeprom_start_addr);
        MB1_i2c_ptr->master_send(eeprom_addr, buffer, 2, false);
        MB1_i2c_ptr->master_receive_bare(eeprom_addr, &buffer[2], eeprom_max_datasize);
        for (count = 0; count < eeprom_max_datasize; count++) {
            HA_NOTIFY("%x ", buffer[count + 2]);
        }
        HA_NOTIFY("\n");

        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}


/*----------------------------------------------------------------------------*/
static void i2c_lightic_test(void)
{
    uint8_t buffer[16], data;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** APDS-9301 & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Power up */
    HA_NOTIFY("\nPower up device.\n");
    buffer[0] = cmd_prefix | int_clear | control_reg_addr;
    buffer[1] = 0x03;
    MB1_i2c_ptr->master_send(light_addr, buffer, 2, true);

    /* Read Power register */
    buffer[0] = cmd_prefix | int_clear | control_reg_addr;
    MB1_i2c_ptr->master_receive(light_addr, buffer[0],
                        &buffer[1], 1);
    HA_NOTIFY("Control Read(hex): %x.\n", buffer[1]);

    while (1) {
        data = 0;

        /* Read ID register */
        buffer[0] = cmd_prefix | int_clear | id_reg_addr;
        MB1_i2c_ptr->master_receive(light_addr, buffer[0],
                            &buffer[1], 1);
        data = buffer[1];

        HA_NOTIFY("\nID Read(hex): %x. Partnum: %x(should be 5). Revno: %x\n",
                data, data >> 4, data & 0x0F);

        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    /* Power down */
    HA_NOTIFY("\nPower down device.\n");
    buffer[0] = cmd_prefix | int_clear | control_reg_addr;
    buffer[1] = 0x00;
    MB1_i2c_ptr->master_send(light_addr, buffer, 2, true);

    /* Read Power register */
    buffer[0] = cmd_prefix | int_clear | control_reg_addr;
    MB1_i2c_ptr->master_receive(light_addr, buffer[0],
                        &buffer[1], 1);
    HA_NOTIFY("Control Read(hex): %x.\n", buffer[1]);

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void i2ceb_tmpic_test(void)
{
    uint8_t buffer[16], conf;
    uint16_t thigh, tlow, tmp;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** TMP275 & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Prompt user */
    HA_NOTIFY("TMP275 configurations:\n"
            "Resolution: 9 bit, Alert active 0.\n"
            "THigh: 45oC, TLow: 40oC.\n");

    /* Write configurations */
    /* Config register */
    buffer[0] = conf_register_pointer_addr;
    buffer[1] = 0x00;
    MB1_i2c_ptr->master_send(tmp_addr, buffer, 2, true);

    /* THigh register */
    buffer[0] = thigh_register_pointer_addr;
    thigh = (uint16_t) (45/resolution_step) << 4;
    buffer[1] = (uint8_t) (thigh >> 8);
    buffer[2] = (uint8_t) thigh;
    MB1_i2c_ptr->master_send(tmp_addr, buffer, 3, true);

    /* THigh register */
    buffer[0] = tlow_register_pointer_addr;
    tlow = (uint16_t) (40/resolution_step) << 4;
    buffer[1] = (uint8_t) (tlow >> 8);
    buffer[2] = (uint8_t) tlow;
    MB1_i2c_ptr->master_send(tmp_addr, buffer, 3, true);

    HA_NOTIFY("WRITE: CONF: %x, THigh: %x, TLow: %x.\n", 0, thigh, tlow);

    /* Read back */
    conf = 0xFF;
    buffer[0] = conf_register_pointer_addr;
    MB1_i2c_ptr->master_receive(tmp_addr, buffer[0],
                    &buffer[1], 1);
    conf = buffer[1];

    /* THigh register */
    thigh = 0;
    buffer[0] = thigh_register_pointer_addr;
    MB1_i2c_ptr->master_receive(tmp_addr, buffer[0],
                        &buffer[1], 2);
    thigh = ((uint16_t)buffer[1] << 8) | (buffer[2]);

    /* THigh register */
    tlow = 0;
    buffer[0] = tlow_register_pointer_addr;
    MB1_i2c_ptr->master_receive(tmp_addr, buffer[0],
                        &buffer[1], 2);
    tlow = ((uint16_t)buffer[1] << 8) | (buffer[2]);

    HA_NOTIFY("READ : CONF: %x, THigh: %x, TLow: %x.\n", conf, thigh, tlow);
    HA_NOTIFY("Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    HA_NOTIFY("D(hex):\tT(oC):\tAlert:\n");
    while (1) {
        /* Read tmp register */
        tmp = 0;
        buffer[0] = tmp_register_pointer_addr;
        MB1_i2c_ptr->master_receive(tmp_addr, buffer[0],
                            &buffer[1], 2);
        tmp = ((uint16_t)buffer[1] << 8) | (buffer[2]);

        HA_NOTIFY("\r%-4x\t%u\t%u", tmp, (uint16_t)((float)(tmp >> 4)*resolution_step),
                MB1_tarlt0_pin.gpio_read());
        HA_FLUSH_STDOUT();

        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void i2ceb_1wired_test(void)
{
    uint32_t val;
    uint16_t temp;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** DS18B20 & MBOARD 1-WIRED COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* write the resolution to configuration register */
    ds18b20_write_cmd(ds18b20_write_scr);
    ds18b20_write_cmd(45);
    ds18b20_write_cmd(10);
    ds18b20_write_cmd((ds18b20_12bit_res << 5) | 0x1F);

    HA_NOTIFY("\n16b data(hex):\tT(oC):\n");
    while(1)
    {
        /* Get temperature */
        ds18b20_write_cmd(ds18b20_convert);
        testing_delay_us(750000);
        ds18b20_write_cmd(ds18b20_read_scr);

        temp = ds18b20_read_byte();
        temp = (temp) | (ds18b20_read_byte() << 8);

        val = (float)(temp & 0x07FF) * 0.0625 * 10000;

        HA_NOTIFY("\r0x%04X\t\t%ld.%04ld", temp, val / 10000,
                val - (val / 10000)*10000);
        HA_FLUSH_STDOUT();

        testing_delay_us(250000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void ds18b20_init()
{
    /* change to output mode */
    MB1_1wired_pin.gpio_shutdown();
    onewired_params.mode = gpio_ns::out_push_pull;
    MB1_1wired_pin.gpio_init(&onewired_params);

    /* reset pulse */
    MB1_1wired_pin.gpio_reset();

    testing_delay_us(480);

    /* release the pin */
    MB1_1wired_pin.gpio_set();

    /* change to input mode */
    MB1_1wired_pin.gpio_shutdown();
    onewired_params.mode = gpio_ns::in_pull_up;
    MB1_1wired_pin.gpio_init(&onewired_params);

    testing_delay_us(480);
}

/*----------------------------------------------------------------------------*/
static void ds18b20_write_byte(uint8_t data)
{
    uint8_t i = 0;

    /* change to output mode */
    MB1_1wired_pin.gpio_shutdown();
    onewired_params.mode = gpio_ns::out_push_pull;
    MB1_1wired_pin.gpio_init(&onewired_params);

    for (i = 0; i < 8; i++)
    {
        MB1_1wired_pin.gpio_reset();
        if (data & 0x01)
        {
            MB1_1wired_pin.gpio_set();
            testing_delay_us(114);
        }
        else
        {
            testing_delay_us(120);
            MB1_1wired_pin.gpio_set();
        }

        data >>= 1;
    }
}

/*----------------------------------------------------------------------------*/
static uint8_t ds18b20_read_byte(void)
{
    uint8_t i=0, data=0;

    for (i=0; i<8; i++)
    {
        /* change to output mode */
        MB1_1wired_pin.gpio_shutdown();
        onewired_params.mode = gpio_ns::out_push_pull;
        MB1_1wired_pin.gpio_init(&onewired_params);

        /* Pull down then release the pin */
        MB1_1wired_pin.gpio_reset();
        data >>= 1;
        MB1_1wired_pin.gpio_set();

        /* change to input mode */
        MB1_1wired_pin.gpio_shutdown();
        onewired_params.mode = gpio_ns::in_pull_up;
        MB1_1wired_pin.gpio_init(&onewired_params);

        if (MB1_1wired_pin.gpio_read()) data|=0x80;

        testing_delay_us(120);
    }

    return data;
}

/*----------------------------------------------------------------------------*/
static void ds18b20_write_cmd(uint8_t data)
{
    ds18b20_init();

    ds18b20_write_byte(ds18b20_rom_skip);

    ds18b20_write_byte(data);
}

/*----------------------------------------------------------------------------*/
static void i2ceb_full_test(void)
{
    i2ceb_testing_init();

    i2ceb_eeprom_communication_test();

    i2ceb_1wired_test();

    i2c_lightic_test();

    i2ceb_tmpic_test();

    i2ceb_testing_deinit();
}
