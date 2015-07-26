/**
 * @file xbee_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-July-2015
 * @brief Source file for XBee testing shell commands.
 */

#include "xbee_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* USART */
static const uint8_t uart_num = 2;
static const uint16_t baudrate = 9600;
static serial_t MB1_usart(uart_num);

/* RESET pin */
static const gpio_ns::gpio_params_t rst_params = {
        gpio_ns::port_A,
        1,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static const bool rst_active_value = 0;
static gpio MB1_rst;
/* End configuration data */

/* Shell command usages */
const char xbee_erx_test_usage[] = "Usage:\n"
        "xbee_test -i, initialize hardware and data for the test.\n"
        "xbee_test -r, ERx LED test, a 8 bit data (b'01010101)\n"
        "\twill be sent to XBee module every 500ms.\n"
        "xbee_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for XBee module.
 */
void xbee_testing_init(void);

/**
 * @brief   ERx LED test.
 */
void xbee_erx_test(void);

/* Public function implementations */
void xbee_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", xbee_erx_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    xbee_testing_init();
                    break;

                case 'r':
                    /* ERx LED test */
                    xbee_erx_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", xbee_erx_test_usage);
                    return;

                default:
                    HA_NOTIFY("Err: unknow option.\n");
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
void xbee_testing_init(void)
{
    HA_NOTIFY("Initializing hardware for XBee tests\n"
            "USART: %u, Buadrate: %u,\n"
            "RST pin: port: %u (port A = 0,...), pin: %u, active value: %u.\n",
            uart_num, baudrate, rst_params.port, rst_params.pin, rst_active_value);

    MB1_usart.Restart(baudrate);
    MB1_rst.gpio_init(&rst_params);

    /* Hold rst for 100ms and release */
    MB1_rst.gpio_assign_value(rst_active_value);
    testing_delay_us(100000);
    MB1_rst.gpio_assign_value(!rst_active_value);
}

/*----------------------------------------------------------------------------*/
void xbee_erx_test(void)
{
    /* Do the test */
    start_waiting_esc_character();

    HA_NOTIFY("Starting ERx LED test ..., press ESC to quit.\n");

    HA_NOTIFY("Holding XBee module in reset.\n");
    MB1_rst.gpio_assign_value(rst_active_value);

    while (1) {
        MB1_usart.Out((uint8_t)0x55);
        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    /* Cleanup */
    stop_waiting_esc_character();

    HA_NOTIFY("Releasing reset...\n");
    MB1_rst.gpio_assign_value(!rst_active_value);

    HA_NOTIFY("Test stopped.\n");

    return;
}

