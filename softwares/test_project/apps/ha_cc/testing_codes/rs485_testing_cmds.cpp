/**
 * @file rs485_testing_cmds.h
 * @author  Huynh Van Minh  <huynhminh009@gmail.com>.
 * @version 1.0
 * @date 29-July-2015
 * @brief Source file for RS485 testing shell commands.
 * (Need initialized MB1_rtc object to implement the timeout code)
 */

#include "rs485_testing_cmds.h"
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

/* nRE = PA0 */
static const gpio_ns::gpio_params_t nRE_params = {
        gpio_ns::port_A,
        0,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static const bool nRE_enable_value = 0;
static gpio MB1_nRE;
/* DE = PA1 */
static const gpio_ns::gpio_params_t DE_params = {
        gpio_ns::port_A,
        1,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static const bool DE_enable_value = 1;
static gpio MB1_DE;
/* End configuration data */

/* Shell command usages */
const char rs485_erx_test_usage[] = "Usage:\n"
        "rs485_test -i, initialize hardware and data for the test.\n"
		"rs485_test -d, deinitialize hardware and data for the test.\n"
        "rs485_test -c, RS485 and MBoard communication test.\n"
		"rs485_test -f, perform full test = rs485_test -i -c.\n"
        "rs485_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for RS485 module.
 */
void rs485_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for RS485 module.
 */
static void rs485_testing_deinit(void);

/**
 * @brief   MBoard-1 and RS485 module communication test. 5 following AT commands will be
 *          used to perform the test:
 *          +++, Put RS485 module into command mode.
 *          ATVR, Read the firmware version.(Loop)
 *          ATHV, Read the hardware version.(Loop)
 *          ATBD, Read current baudrate.    (Loop)
 *          ATCN, Exit command mode when ESC is pressed.
 */
void rs485_communication_test(void);

/**
 * @brief   Full test. This includes initialization, communication test.
 */
static void rs485_full_test(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void rs485_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", rs485_erx_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    rs485_testing_init();
                    break;
                case 'd':
                    /* Deinit */
                    rs485_testing_deinit();
                    break;
                case 'c':
                    /* Communication test*/
                	rs485_communication_test();
                    break;
                case 'f':
                    /* Communication test*/
                    rs485_full_test();
                    break;
                case 'h':
                    HA_NOTIFY("%s", rs485_erx_test_usage);
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
        rs485_testing_deinit();
    }
}


/* Private function implementations */
/*----------------------------------------------------------------------------*/
void rs485_testing_init(void)
{
    HA_NOTIFY("Initializing hardware for RS485 tests\n"
            "USART: %u, Buadrate: %u,\n"
            "nRE pin: port: %u (port A = 0,...), pin: %u, enable value: %u.\n"
    		"DE pin: port: %u (port A = 0,...), pin: %u, enable value: %u.\n",
            uart_num, baudrate, nRE_params.port, nRE_params.pin, nRE_enable_value,
			DE_params.port, DE_params.pin, DE_enable_value);

    MB1_usart.Restart(baudrate);
    MB1_nRE.gpio_init(&nRE_params);
    MB1_DE.gpio_init(&DE_params);

    /* Enable nRE and DE */
    MB1_nRE.gpio_assign_value(nRE_enable_value);
    MB1_DE.gpio_assign_value(DE_enable_value);
    HA_NOTIFY("Initializing success\n");
}

/*----------------------------------------------------------------------------*/
void rs485_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "USART: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            MB1_usart.Get_usedUart());

    /* Shutdown USART and GPIOs */
    MB1_usart.it_disable();
    MB1_usart.Shutdown();
    MB1_nRE.gpio_shutdown();
    MB1_DE.gpio_shutdown();
    HA_NOTIFY("Deinitializing hardware finish.\n");
}
/*----------------------------------------------------------------------------*/
void rs485_communication_test(void)
{
    char buffer[256];
    start_waiting_esc_character();

    HA_NOTIFY("\n*** RS485 & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");
    HA_NOTIFY("Switch to command mode. Send +++ ...\n");
    testing_delay_us(1000000);
    MB1_usart.Print("+++");
    wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
    HA_NOTIFY("%s\n", buffer);
    HA_NOTIFY("\n");

    /* Periodically send 3 AT commands in a loop */
    while (1) {
        HA_NOTIFY("Sending HELLO.\n");
        MB1_usart.Print("HELLO\r");
        /* time out to receive data*/
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        HA_NOTIFY("Sending RS485 testing.\n");
        MB1_usart.Print("RS485 testing\r");
        /* time out to receive data*/
        wait_uart_with_timeout(&MB1_usart, 2, buffer, 256);
        HA_NOTIFY("%s\n", buffer);

        HA_NOTIFY("Sending OK.\n");
        MB1_usart.Print("OK");
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
void rs485_full_test(void)
{
	rs485_testing_init();

	rs485_communication_test();

	rs485_testing_deinit();
}

