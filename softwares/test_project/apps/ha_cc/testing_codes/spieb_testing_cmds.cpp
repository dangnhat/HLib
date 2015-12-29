/**
 * @file spieb_testing_cmds.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 24-Aug-2015
 * @brief Source file for SPI EBoard testing shell commands.
 */

#include "spieb_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* SPI */
static SPI* MB1_spi_p = &MB1_SPI1;
static const SPI_ns::SPI_params_t spi_params = {
        SPI_ns::bp16,
        SPI_ns::cpha_1edge,
        SPI_ns::cpol_low,
        0,
        SPI_ns::d8b,
        SPI_ns::dr_2lines_fullduplex,
        SPI_ns::msb_first,
        SPI_ns::master,
        SPI_ns::soft_nss,
};

/* SPI SS Decoding table */
static const uint8_t num_of_ss_lines = 2;

enum ss_line_table: uint8_t {
    mb1_ss0_pin_linenum = 0,
    mb1_ss1_pin_linenum = 1,
};

enum decoding_table: uint16_t {
    tmp = SPI_ns::last_of_initial_table,
    dpot,
    eeprom,
};

enum decoding_values_table: uint16_t {
    all_free_value = 0,
    tmp_value = 1,
    dpot_value = 2,
    eeprom_value = 3
};

/* SS pin */
static const gpio_ns::gpio_params_t ss0_params = {
        gpio_ns::port_A,
        4,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_ss0_pin;

static const gpio_ns::gpio_params_t ss1_params = {
        gpio_ns::port_C,
        11,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_ss1_pin;

/* CAT25256 EEPROM data */
enum eeprom_instruction_e: uint8_t {
    eeprom_read_ins = 0x03,
    eeprom_write_ins = 0x02,
    eeprom_write_enable_ins = 0x06,
    eeprom_write_disable_ins = 0x04,
    eeprom_read_status_ins = 0x05,
    eeprom_write_status_ins = 0x01,
};

/* Digital Potentiometer (MCP41XXX) data */
enum dpot_command_e: uint8_t {
    dpot_write_cmd = 0x10,
    dpot_shutdowm_cmd = 0x20,
    dpot_nop_cmd = 0x00,
};

enum dpot_pot_select_e: uint8_t {
    dpot_pot0_select = 0x01,
    dpot_pot1_select = 0x02, /* Don't care in case of MCP41XXX */
};

static const adc_ns::adc_params_t dpot_adc_params = {
    adc_ns::independent,
    adc_ns::adc1,
    10,
    adc_ns::continuous_mode,
    adc_ns::regular_channel,
    adc_ns::no_option,
    ADC_SampleTime_28Cycles5,
    adc_ns::poll,
};

static const gpio_ns::gpio_params_t dpot_adc_gpio_params = {
    gpio_ns::port_C,
    0,
    gpio_ns::in_analog,
    gpio_ns::speed_50MHz,
};

/* Temperature sensor (TMP121/TMP122) data */
const float resolution_step = 0.0625; /* oC */

/* End configuration data */

/* Shell command usages */
static const char spieb_test_usage[] = "Usage:\n"
        "spieb_test -i, initialize hardware and data for the test.\n"
        "spieb_test -d, deinitialize hardware and data.\n"
        "spieb_test -e, SPI EEPROM communication test.\n"
        "spieb_test -r, Digital potentiometer communication test.\n"
        "spieb_test -t, Temperature IC communication test.\n"
        "spieb_test -f, perform full test = spieb_test -i -e -r -t -d.\n"
        "spieb_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for SPI EBoard.
 */
static void spieb_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for SPI EBoard.
 */
static void spieb_testing_deinit(void);

/**
 * @brief   MBoard-1 and EEPROM communication test.
 *          First, make sure that nWP jumper is not connected.
 *          Data will be written to STATUS and read back.
 *          Written data and read data should be the same.
 *
 *          Then, connect nWP jumper.
 *          Data will also be written to STATUS and read back.
 *          Read data will stay the same regardless of written data.
 */
static void spieb_eeprom_communication_test(void);

/**
 * @brief   MBoard-1 and Digital potentiometer communication test.
 *          First, connect A to 3V3 and B to GND.
 *          Data for potentiometer will be looped in a cycle of 0 and 255.
 *          Use VOM to measure changes of voltage on pin W.
 */
static void spieb_digital_potentiometer_test(void);

/**
 * @brief   MBoard-1 and TMP121/TMP122 communication test.
 */
static void spieb_tmpic_test(void);

/**
 * @brief   Full test. This includes initialization, communication test and warm reset test.
 */
static void spieb_full_test(void);

/**
 * @brief   Enable write latch of EEPROM.
 *
 * @param[in]   spi_p, pointer to a initialized SPI object.
 * @param[in]   spi_dev_id, device slave id.
 */
static void eeprom_enable_write_latch(SPI *spi_p, const uint16_t spi_dev_id);

/**
 * @brief   Disable write latch of EEPROM.
 *
 * @param[in]   spi_p, pointer to a initialized SPI object.
 * @param[in]   spi_dev_id, device slave id.
 */
static void eeprom_disable_write_latch(SPI *spi_p, const uint16_t spi_dev_id);

/**
 * @brief   Read status of EEPROM.
 *
 * @param[in]   spi_p, pointer to a initialized SPI object.
 * @param[in]   spi_dev_id, device slave id.
 *
 * @return      STATUS value.
 */
static uint8_t eeprom_read_status(SPI *spi_p, const uint16_t spi_dev_id);

/**
 * @brief   Write status of EEPROM.
 *
 * @param[in]   spi_p, pointer to a initialized SPI object.
 * @param[in]   spi_dev_id, device slave id.
 * @param[in]   value, new status value.
 */
static void eeprom_write_status(SPI *spi_p, const uint16_t spi_dev_id, const uint8_t value);

/**
 * @brief   Write command to MCP41XXX.
 *
 * @param[in]   spi_p, pointer to a initialized SPI object.
 * @param[in]   spi_dev_id, device slave id.
 * @param[in]   cmd, command. (dpot_write_cmd, dpot_shutdown_cmd, or dpot_nop_cmd)
 * @param[in]   pot, selected potentiometer (only dpot_pot0_select in case of MCP41XXX)
 * @param[in]   data.
 */
static void dpot_send_command_and_data(SPI *spi_p, const uint16_t spi_dev_id,
        const uint8_t cmd, const uint8_t pot, const uint8_t data);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void spieb_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", spieb_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    spieb_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    spieb_testing_deinit();
                    break;

                case 'e':
                    /* EEPROM Communication test*/
                    spieb_eeprom_communication_test();
                    break;

                case 'r':
                    /* Digital potentiometer communication test */
                    spieb_digital_potentiometer_test();
                    break;

                case 't':
                    /* Temperature communication test */
                    spieb_tmpic_test();
                    break;

                case 'f':
                    /* Full test */
                    spieb_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", spieb_test_usage);
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
static void spieb_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for ETH tests ***\n"
            "SPI: %u, Buadrate prescaler: %u,\n"
            "SS0 pin: port: %u (port A = 0,...), pin: %u\n"
            "SS1 pin: port: %u (port A = 0,...), pin: %u\n",
            MB1_spi_p->get_usedSPI(), 1 << (spi_params.baudRatePrescaler/8 + 1),
            ss0_params.port, ss0_params.pin,
            ss1_params.port, ss1_params.pin);

    /* Init SPI and GPIOs */
    MB1_spi_p->init(&spi_params);
    MB1_ss0_pin.gpio_init(&ss0_params);
    MB1_ss1_pin.gpio_init(&ss1_params);

    /* Setup decoding table */
    MB1_spi_p->SM_numOfSSLines_set(num_of_ss_lines);
    MB1_spi_p->SM_GPIO_set(&MB1_ss0_pin, mb1_ss0_pin_linenum);
    MB1_spi_p->SM_GPIO_set(&MB1_ss1_pin, mb1_ss1_pin_linenum);
    MB1_spi_p->SM_deviceToDecoder_set(SPI_ns::all_free, all_free_value);
    MB1_spi_p->SM_deviceToDecoder_set(tmp, tmp_value);
    MB1_spi_p->SM_deviceToDecoder_set(dpot, dpot_value);
    MB1_spi_p->SM_deviceToDecoder_set(eeprom, eeprom_value);
}

/*----------------------------------------------------------------------------*/
static void spieb_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "SPI: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            MB1_spi_p->get_usedSPI());

    /* Shutdown USART and GPIOs */
    MB1_spi_p->deinit();
    MB1_spi_p->SM_deinit();
}

/*----------------------------------------------------------------------------*/
static void spieb_eeprom_communication_test(void)
{
    uint8_t temp = 0;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** EEPROM CAT25256 & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Attach */
    MB1_spi_p->SM_device_attach(eeprom);

    HA_NOTIFY("Make sure nWP jumper on the board is not shorted.\n"
            "Press ESC to continue.\n");
    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    while (1) {
        /* Set BP1 = 1, BP0 = 0, WPEN = 1 */
        HA_NOTIFY("\nWRITE: WPEN: 1, BP1: 1, BP0: 0.\n");
        eeprom_enable_write_latch(MB1_spi_p, eeprom);
        eeprom_write_status(MB1_spi_p, eeprom, 0x88);

        /* Read back */
        temp = eeprom_read_status(MB1_spi_p, eeprom);
        HA_NOTIFY("READ : WPEN: %u, BP1: %u, BP0: %u, WEL: %u, RDY: %u. (0x%x)\n",
                temp >> 7, (temp >> 3) & 0x01, (temp >> 2) & 0x01,
                (temp >> 1) & 0x01, temp & 0x01, temp);
        testing_delay_us(1000000);

        /* Set BP1 = 0, BP0 = 1, WPEN = 1 */
        HA_NOTIFY("\nWRITE: WPEN: 1, BP1: 0, BP0: 1.\n");
        eeprom_enable_write_latch(MB1_spi_p, eeprom);
        eeprom_write_status(MB1_spi_p, eeprom, 0x84);

        /* Read back */
        temp = eeprom_read_status(MB1_spi_p, eeprom);
        HA_NOTIFY("READ : WPEN: %u, BP1: %u, BP0: %u, WEL: %u, RDY: %u. (0x%x)\n",
                temp >> 7, (temp >> 3) & 0x01, (temp >> 2) & 0x01,
                (temp >> 1) & 0x01, temp & 0x01, temp);
        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }
    stop_waiting_esc_character();
    start_waiting_esc_character();

    HA_NOTIFY("\nConnect nWP jumper on board, STATUS value will not change.\n"
            "Press ESC to continue.\n");
    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    while (1) {
        /* Set BP1 = 1, BP0 = 0, WPEN = 1 */
        HA_NOTIFY("\nWRITE: WPEN: 1, BP1: 1, BP0: 0.\n");
        eeprom_enable_write_latch(MB1_spi_p, eeprom);
        eeprom_write_status(MB1_spi_p, eeprom, 0x88);

        /* Read back */
        temp = eeprom_read_status(MB1_spi_p, eeprom);
        HA_NOTIFY("READ : WPEN: %u, BP1: %u, BP0: %u, WEL: %u, WIP: %u. (0x%x)\n",
                temp >> 7, (temp >> 3) & 0x01, (temp >> 2) & 0x01,
                (temp >> 1) & 0x01, temp & 0x01, temp);
        testing_delay_us(1000000);

        /* Set BP1 = 0, BP0 = 1, WPEN = 1 */
        HA_NOTIFY("\nWRITE: WPEN: 1, BP1: 0, BP0: 1.\n");
        eeprom_enable_write_latch(MB1_spi_p, eeprom);
        eeprom_write_status(MB1_spi_p, eeprom, 0x84);

        /* Read back */
        temp = eeprom_read_status(MB1_spi_p, eeprom);
        HA_NOTIFY("READ : WPEN: %u, BP1: %u, BP0: %u, WEL: %u, WIP: %u. (0x%x)\n",
                temp >> 7, (temp >> 3) & 0x01, (temp >> 2) & 0x01,
                (temp >> 1) & 0x01, temp & 0x01, temp);
        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    /* Detach */
    MB1_spi_p->SM_device_release(eeprom);

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void eeprom_enable_write_latch(SPI *spi_p, const uint16_t spi_dev_id)
{
    /* Chip select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send instruction */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, eeprom_write_enable_ins);

    /* Chip deselect */
    spi_p->SM_device_deselect(spi_dev_id);
}

/*----------------------------------------------------------------------------*/
static void eeprom_disable_write_latch(SPI *spi_p, const uint16_t spi_dev_id)
{
    /* Chip select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send instruction */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, eeprom_write_disable_ins);

    /* Chip deselect */
    spi_p->SM_device_deselect(spi_dev_id);
}

/*----------------------------------------------------------------------------*/
static uint8_t eeprom_read_status(SPI *spi_p, const uint16_t spi_dev_id)
{
    uint8_t retval = 0;

    /* Chip select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send instruction */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, eeprom_read_status_ins);

    /* Read data */
    retval = spi_p->M2F_sendAndGet_blocking(spi_dev_id, 0);

    /* Chip deselect */
    spi_p->SM_device_deselect(spi_dev_id);

    return retval;
}

/*----------------------------------------------------------------------------*/
static void eeprom_write_status(SPI *spi_p, const uint16_t spi_dev_id, const uint8_t value)
{
    /* Chip select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send instruction */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, eeprom_write_status_ins);

    /* Read data */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, value);

    /* Chip deselect */
    spi_p->SM_device_deselect(spi_dev_id);
}

/*----------------------------------------------------------------------------*/
static void spieb_digital_potentiometer_test(void)
{
    uint16_t d = 0;
    uint16_t vdd = 3300;
    adc w_adc;
    gpio adc_gpio;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** Digital Potentiometer MCP41XXX & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Attach */
    MB1_spi_p->SM_device_attach(dpot);

    /* Init ADC and GPIO for ADC */
    w_adc.adc_init(&dpot_adc_params);
    w_adc.adc_start();
    adc_gpio.gpio_init(&dpot_adc_gpio_params);

    HA_NOTIFY("Connect A to 3V3, B to GND, and W to ADC in10 on P1.\n"
            "Press ESC to continue.\n");
    while (esc_pressed == false);
    stop_waiting_esc_character();
    start_waiting_esc_character();

    HA_NOTIFY("\nDn:\tVw(mV):\n");

    while (1) {
        HA_NOTIFY("\r%-4u\t%-4u", d, (unsigned int)( (float)w_adc.adc_convert()/4096 * vdd ));
        HA_FLUSH_STDOUT();

        dpot_send_command_and_data(MB1_spi_p, dpot, dpot_write_cmd, dpot_pot0_select, d);

        d += 5;
        if (d > 255) {
            d = 0;
        }

        testing_delay_us(200000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }
    /* Detach */
    MB1_spi_p->SM_device_release(dpot);

    /* Deinit ADC and GPIO for ADC */
    w_adc.adc_stop();
    adc_gpio.gpio_shutdown();

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void dpot_send_command_and_data(SPI *spi_p, const uint16_t spi_dev_id,
        const uint8_t cmd, const uint8_t pot, const uint8_t data)
{
    /* Chip select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send command */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, cmd | pot);

    /* Send data */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, data);

    /* Chip deselect */
    spi_p->SM_device_deselect(spi_dev_id);
}

/*----------------------------------------------------------------------------*/
static void spieb_tmpic_test(void)
{
    int16_t d = 0;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** SPI temperature sensor (TMP121/122) & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Attach */
    MB1_spi_p->SM_device_attach(tmp);

    HA_NOTIFY("\n16b data(hex):\tT(oC):\n");

    while (1) {
        /* Get temperature */
        MB1_spi_p->SM_device_select(tmp);
        d = MB1_spi_p->M2F_sendAndGet_blocking(tmp, 0x00);
        d = (d << 8) | MB1_spi_p->M2F_sendAndGet_blocking(tmp, 0x00);
        MB1_spi_p->SM_device_deselect(tmp);

        HA_NOTIFY("\r%-4x\t\t%-4u", d, (uint16_t)((float)(d >> 3) * 0.0625 ) );
        HA_FLUSH_STDOUT();

        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }
    /* Detach */
    MB1_spi_p->SM_device_release(tmp);

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void spieb_full_test(void)
{
    spieb_testing_init();

    spieb_eeprom_communication_test();

    spieb_digital_potentiometer_test();

    spieb_tmpic_test();

    spieb_testing_deinit();
}
