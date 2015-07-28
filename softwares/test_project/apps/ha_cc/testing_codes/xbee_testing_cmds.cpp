/**
 * @file xbee_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-July-2015
 * @brief Source file for XBee testing shell commands.
 * (Need initialized MB1_rtc object to implement the timeout code)
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
        "xbee_test -w, warm reset test.\n"
        "xbee_test -r, ERx LED test, a 8 bit data (b'01010101)\n"
        "\twill be sent to XBee module every 500ms.\n"
        "xbee_test -t, ETx LED test.\n"
        "xbee_test -c, XBee and MBoard communication test.\n"
        "xbee_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for XBee module.
 */
void xbee_testing_init(void);

/**
 * @brief   Warm reset test.
 */
void xbee_warm_reset_test(void);

/**
 * @brief   ERx LED test.
 */
void xbee_erx_test(void);

/**
 * @brief   ETx LED test.
 */
void xbee_etx_test(void);

/**
 * @brief   MBoard-1 and XBee module communication test. 5 following AT commands will be
 *          used to perform the test:
 *          +++, Put XBee module into command mode.
 *          ATVR, Read the firmware version.(Loop)
 *          ATHV, Read the hardware version.(Loop)
 *          ATBD, Read current baudrate.    (Loop)
 *          ATCN, Exit command mode when ESC is pressed.
 */
void xbee_communication_test(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
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

                case 'w':
                    /* Warm reset test */
                    xbee_warm_reset_test();
                    break;

                case 'r':
                    /* ERx LED test */
                    xbee_erx_test();
                    break;

                case 't':
                    /* ETx LED test */
                    xbee_etx_test();
                    break;

                case 'c':
                    /* Communication test*/
                    xbee_communication_test();
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

    HA_NOTIFY("\n*** ERx LED TEST ***\n"
            "(press ESC to quit).\n");

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

/*----------------------------------------------------------------------------*/
void xbee_communication_test(void)
{
    char buffer[256];

    start_waiting_esc_character();

    HA_NOTIFY("\n*** XBEE & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    HA_NOTIFY("Switch to command mode. Send +++ ...\n");
    testing_delay_us(1000000);
    MB1_usart.Print("+++");
    wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
    HA_NOTIFY("%s\n", buffer);
    HA_NOTIFY("\n");

    /* Periodically send 3 AT commands in a loop */
    while (1) {
        /* Read firmware version */
        HA_NOTIFY("Sending ATVR (read firmware version).\n");
        MB1_usart.Print("ATVR\r");
        /* time out to receive data */
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        /* Read hardware version */
        HA_NOTIFY("Sending ATHV (read hardware version).\n");
        MB1_usart.Print("ATHV\r");
        /* time out to receive data */
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        /* Read buadrate */
        HA_NOTIFY("Sending ATBD (read current buadrate).\n");
        MB1_usart.Print("ATBD\r");
        /* time out to receive data */
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    /* Exit command mode and stop the test */
    HA_NOTIFY("\nExit command mode. Send ATCN ...\n");
    MB1_usart.Print("ATCN\r");
    wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
    HA_NOTIFY("%s\n", buffer);

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void xbee_etx_test(void)
{
    char buffer[256];

    start_waiting_esc_character();
    HA_NOTIFY("\n*** ETx LED TEST ***\n"
                "(press ESC to quit).\n");

    while (1) {
        testing_delay_us(1000000);
        HA_NOTIFY("Switch to command mode. Send +++ ...\n");
        MB1_usart.Print("+++");
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        HA_NOTIFY("Exit to command mode. Send ATCN ...\n");
        MB1_usart.Print("ATCN\r");
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void xbee_warm_reset_test(void)
{
    start_waiting_esc_character();
    HA_NOTIFY("\n*** WARM RESET TEST ***\n"
                "(press ESC to quit).\n");

    while (1) {
        HA_NOTIFY("Holding module in reset.\n");
        MB1_rst.gpio_assign_value(rst_active_value);

        testing_delay_us(500000);

        HA_NOTIFY("Releasing reset...\n");
        MB1_rst.gpio_assign_value(!rst_active_value);

        testing_delay_us(1000000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}
