/**
 * @file sim900_testing_cmds.cpp
 * @author  Nguyen Dinh Trung Truc  <truc.ndtt@gmail.com>.
 * @version 1.1
 * @date 15-Feb-2016
 * @brief Source file for Sim900 testing shell commands.
 */

#include "sim900_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"
#include "string.h"

using namespace testing_ns;
//using namespace ISRMgr_ns;

/* Configuration data */
/* USART */
static const uint8_t uart_num = 3;
static const uint32_t baudrate = 115200;
static serial_t MB1_usart(uart_num);

static ISRMgr MB1_int;
static const ISRMgr_ns::ISR_t MB1_int_type = ISRMgr_ns::ISRMgr_USART3;

/* RI pin */
static gpio_ns::gpio_params_t RI_params = {
        gpio_ns::port_B,
        3,
        gpio_ns::in_pull_up,
        gpio_ns::speed_50MHz,
};
static gpio RI_pin;

/* Ring buffer*/
#define MAX_SIZE_RB 128

typedef struct rb_handle_t
{
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    uint8_t max_size;

    uint8_t buffer[MAX_SIZE_RB];
} rb_handle_t;

static rb_handle_t rx_buffer;

/* End configuration data */

/* Shell command usages */
static const char sim900_test_usage[] = "Usage:\n"
        "sim900_test -i, initialize hardware and data for the test.\n"
        "sim900_test -d, deinitialize hardware and data.\n"
        "sim900_test -f, perform full test.\n"
        "sim900_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for CAN EBoard.
 */
static void sim900_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for CAN EBoard.
 */
static void sim900_testing_deinit(void);

/**
 * @brief   Perform full test.
 */
static void sim900_testing_full(void);

/**
 * @brief   Initialize sim900
 */
static void sim900_init(void);

/**
 * @brief   Initialize ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[in]   size, max size of the ring buffer
 *
 * @return
 *      1: success
 *      0: failure
 */
static uint8_t rb_init(rb_handle_t *rb_data, uint16_t size);

/**
 * @brief   add data to the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[in]   buf, data to add
 * @param[in]   size, size of data
 *
 * @return
 *      1: success
 *      0: failure
 */
static uint8_t rb_add_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size);

/**
 * @brief   get data from the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[out]   data, data received
 * @param[in]   size, size of data to receive
 *
 * @return  real size of received data
 */
static uint16_t rb_get_data(rb_handle_t *rb_data, uint8_t *data, uint16_t size);

/**
 * @brief   clear the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 */
static void rb_clear(rb_handle_t *rb_data);

/**
 * @brief   Check communication b/w MB and SIM900
 */
static void sim900_uart_test(void);

/**
 * @brief   Check account money
 */
static void sim900_gsm_test(void);

/**
 * @brief   usart3 interrupt callback function
 */
static void usart3_irq(void);


/* Public function implementations */
/*----------------------------------------------------------------------------*/
void sim900_test(int argc, char** argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", sim900_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    sim900_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    sim900_testing_deinit();
                    break;

                case 'f':
                    /* Perform full test */
                    sim900_testing_full();
                    break;

                case 'h':
                    /* Print usage */
                    HA_NOTIFY("%s", sim900_test_usage);
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
static void sim900_testing_init(void)
{
    HA_NOTIFY("\nInitializing hardware for sim900 tests\n"
            "USART: %u, Baudrate: %lu\n"
            "RI pin: port: %u (port A = 0,...), pin: %u\n",
            uart_num, baudrate,
            RI_params.port, RI_params.pin);

    /* Initialize UART */
    MB1_usart.Restart(baudrate);
    MB1_usart.it_enable(0,1);
    MB1_usart.it_config(USART_IT_RXNE, ENABLE);
    MB1_int.subISR_assign(MB1_int_type, usart3_irq);

    /* Init RI pin */
    RI_pin.gpio_init(&RI_params);

    HA_NOTIFY("\nRI: %d (should be 1)\n", RI_pin.gpio_read());

    /* Create ring buffer */
    rb_init(&rx_buffer, 128);
}

/*----------------------------------------------------------------------------*/
static void sim900_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "USART: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            uart_num);

    /*Deinit USART and INT*/
    MB1_int.subISR_remove(ISRMgr_ns::ISRMgr_USART3, usart3_irq);
    MB1_usart.it_disable();
    MB1_usart.Shutdown();

    /* Deinit RI pin */
    RI_pin.gpio_shutdown();

    rb_clear(&rx_buffer);
}

/*----------------------------------------------------------------------------*/
static uint8_t rb_init(rb_handle_t *rb_data, uint16_t size)
{
    uint16_t i;

    if (size > MAX_SIZE_RB) return 0;

    for (i = 0; i < size; i++)
    {
        rb_data->buffer[i] = 0;
    }

    rb_data->head = 0;
    rb_data->tail = 0;
    rb_data->size = 0;
    rb_data->max_size = size;

    return 1;

}

/*----------------------------------------------------------------------------*/
static uint8_t rb_add_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size)
{
    uint16_t i;

    if (rb_data->size + size <= rb_data->max_size)
    {
        for (i = 0; i < size; i++)
        {
            rb_data->buffer[rb_data->tail] = buf[i];
            rb_data->tail = (rb_data->tail + 1) % rb_data->max_size;
            rb_data->size++;
        }
        return 1;
    }
    else return 0;
}

/*----------------------------------------------------------------------------*/
static uint16_t rb_get_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size)
{
    uint16_t i;

    size = (size < rb_data->size)? size : rb_data->size;

    for (i = 0; i < size; i++)
    {
        buf[i] = rb_data->buffer[rb_data->head];
        rb_data->head = (rb_data->head + 1) % rb_data->max_size;
        rb_data->size--;
    }

    return size;
}

/*----------------------------------------------------------------------------*/
static void rb_clear(rb_handle_t *rb_data)
{
    rb_data->size = 0;
    rb_data->head = 0;
    rb_data->tail = 0;
}

/*----------------------------------------------------------------------------*/
static void sim900_init(void)
{
    HA_NOTIFY("\nInitializing SIM900 (about 5 seconds) ...\n");

    /* Turn echo off */
    MB1_usart.Print("ATE0\r");
    testing_delay_us(1000000);

    /* SMS in text mode */
    MB1_usart.Print("AT+CMGF=1\r");
    testing_delay_us(1000000);

    /* Set how the modem will response when a SMS is received */
    MB1_usart.Print("AT+CNMI=1,2,0,0,0\r");
    testing_delay_us(1000000);

    /* Show caller number */
    MB1_usart.Print("AT+CLIP=1\r");
    testing_delay_us(1000000);

    /* report feature */
    MB1_usart.Print("AT+CSMP=17,167,0,240\r");
    testing_delay_us(1000000);

    /* Clear RX buffer */
    rb_clear(&rx_buffer);

    HA_NOTIFY("DONE !\n");
}

/*----------------------------------------------------------------------------*/
static void sim900_uart_test(void)
{
    uint8_t state = 0, data;

    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #1: UART TEST\n"
            "This test makes sure Sim900 can communicate with MB\n"
            "(Press ESC to continue)\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    /* Clear buffer before testing */
    rb_clear(&rx_buffer);

    HA_NOTIFY("\nSending \"AT\" ...\n");
    MB1_usart.Print("AT\r");

    start_waiting_esc_character();

    while (1)
    {
        /* Receive OK */
        if (rb_get_data(&rx_buffer,&data,1) > 0)
        {
            switch (state)
            {
            case 0:
                if (data == 'O') state = 1;
                break;

            case 1:
                if (data == 'K') state = 2;
                break;

            case 2:
                if (data == '\r')
                {
                    state = 3;
                    HA_NOTIFY("\nReceive \"OK\", Successful !\n");
                }
                break;
            }
        }

        if (esc_pressed == true)
        {
            break;
        }
    }

    stop_waiting_esc_character();

    /* Fail to receive "OK" */
    if (state != 3)
    {
        HA_NOTIFY("\nFail\n");
    }
}

static void sim900_gsm_test(void)
{
    uint8_t state = 0, data, ri = 0;

    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #2: GSM TEST\n"
            "This test calls *101# to check money account\n"
            "(Press ESC to continue)\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    /* Clear buffer brfore testing */
    rb_clear(&rx_buffer);

    HA_NOTIFY("\nCalling *101# ... \n");

    /* Call *101# */
    MB1_usart.Print("ATD*101#;\r");

    start_waiting_esc_character();

    while(1)
    {
        /* Receiving message */
        if (rb_get_data(&rx_buffer,&data,1) > 0)
        {
            switch (state)
            {
            /* Print content between two " " */
            case 0:
                if (data == '"')
                {
                    state = 1;
                    HA_NOTIFY("\nReceived Message: \n");

                    /*check RI pin*/
                    if (RI_pin.gpio_read() == 0) ri = 1;
                }
                break;

            case 1:
                if (data == '"')
                {
                    state = 2;
                    HA_NOTIFY("\n");
                }
                else
                {
                    HA_NOTIFY("%c", (char)data);
                }
                break;

            /* End of message */
            case 2:
                if (data == '\r')
                {
                    state = 3;
                    HA_NOTIFY("\nSuccessful !\n");
                }
                break;
            }

        }

        if (esc_pressed == true)
        {
            break;
        }
    }

    stop_waiting_esc_character();

    /* Fail to receive message */
    if (state != 3)
    {
        HA_NOTIFY("\nFail\n");
    }
    else
    {
        if (ri == 1)
        {
            HA_NOTIFY("\nRI pin works\n");
        }
        else
        {
            HA_NOTIFY("\nRI pin does not work\n");
        }
    }
}

/*----------------------------------------------------------------------------*/
static void sim900_testing_full(void)
{
    sim900_testing_init();

    HA_NOTIFY("\n*** SIM900 EBOARD & MBOARD COMMUNICAION TEST ***\n");
    HA_NOTIFY("\nPlease make sure SIM900 is turned on\n"
            "(Press ESC to continue)\n");

    start_waiting_esc_character();
    while (esc_pressed == false);
    stop_waiting_esc_character();

    sim900_init();

    sim900_uart_test();

    sim900_gsm_test();

    sim900_testing_deinit();
}

/*----------------------------------------------------------------------------*/
static void usart3_irq(void)
{
    uint8_t buf = USART_ReceiveData(USART3);

    /* add received byte to ring buffer */
    rb_add_data(&rx_buffer, &buf, 1);
}

