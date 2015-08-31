/*
 * led7seg_testing_cmds.cpp
 *
 *  Created on: Aug 9, 2015
 *      Author: Minh
 */

#include "led7seg_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */

/* L1 pin */
static const gpio_ns::gpio_params_t l1_params = {
        gpio_ns::port_B,
        0,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_L1;

/* L2 pin */
static const gpio_ns::gpio_params_t l2_params = {
        gpio_ns::port_B,
        1,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_L2;

/* L3 pin */
static const gpio_ns::gpio_params_t l3_params = {
        gpio_ns::port_B,
        4,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_L3;

/* L4 pin */
static const gpio_ns::gpio_params_t l4_params = {
        gpio_ns::port_B,
        5,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_L4;
static const bool l_enable_value = 0;

/* A pin */
static const gpio_ns::gpio_params_t a_params = {
        gpio_ns::port_A,
        4,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_A;

/* B pin */
static const gpio_ns::gpio_params_t b_params = {
        gpio_ns::port_A,
        5,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_B;

/* C pin */
static const gpio_ns::gpio_params_t c_params = {
        gpio_ns::port_A,
        6,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_C;

/* D pin */
static const gpio_ns::gpio_params_t d_params = {
        gpio_ns::port_A,
        7,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_D;

/* h pin */
static const gpio_ns::gpio_params_t h_params = {
        gpio_ns::port_C,
        11,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_h;

static const bool data_set_value = 1;
/* End configuration data */

/*--------------------------------------------------------------------------------------*/
/* Shell command usages */
const char led7seg_test_usage[] = "Usage:\n"
        "led7seg_test -i, initialize hardware and data for the test.\n"
        "led7seg_test -d, deinitialize hardware and data for the test.\n"
        "led7seg_test -a, test all led.\n"
        "led7seg_test -s, test led7seg scan.\n"
        "led7seg_test -f, perform full test = led7seg_test -i -a -s.\n"
        "led7seg_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";
/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for LED7SEG module.
 */
static void led7seg_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for LED7SEG module.
 */
static void led7seg_testing_deinit(void);

/*
 * Test LED7SEG one led.
 * led_number = 1 ~ 4.
 */
static void led7seg_testing_all(void);

/*
 * Test Scan LED7SEG
 */
static void led7seg_testing_scan(void);

/**
 * @brief   Full test.
 */
static void led7seg_full_test(void);

/*----------------------------------------------------------------------------*/
/* LED7SEG function */
/*
 * Function led_data() display data 0-9 and data h.
 */
static void led_data(uint8_t number, bool h);

/*
 * Function led_number() choose LED7SEG.
 * led_number : 1 ~ 4.
 */
#define all 0
static void led_number(uint8_t lednumber);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void led7seg_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", led7seg_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    led7seg_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    led7seg_testing_deinit();
                    break;

                case 'a':
                    /* Test led7seg 1 */
                    led7seg_testing_all();
                    break;

                case 's':
                    /* Test led7seg scan */
                    led7seg_testing_scan();
                    break;

                case 'f':
                    /* Full test */
                    led7seg_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", led7seg_test_usage);
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
void led7seg_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for LED7SEG tests ***\n"
            "L1 pin: port: %u (port A = 0,...), pin: %u\n"
            "L2 pin: port: %u (port A = 0,...), pin: %u\n"
            "L3 pin: port: %u (port A = 0,...), pin: %u\n"
            "L4 pin: port: %u (port A = 0,...), pin: %u\n"
            "A pin: port: %u (port A = 0,...), pin: %u\n"
            "B pin: port: %u (port A = 0,...), pin: %u\n"
            "C pin: port: %u (port A = 0,...), pin: %u\n"
            "D pin: port: %u (port A = 0,...), pin: %u\n"
            "h pin: port: %u (port A = 0,...), pin: %u\n",
            l1_params.port, l1_params.pin,
            l2_params.port, l2_params.pin,
            l3_params.port, l3_params.pin,
            l4_params.port, l4_params.pin,
            a_params.port, a_params.pin,
            b_params.port, b_params.pin,
            c_params.port, c_params.pin,
            d_params.port, d_params.pin,
            h_params.port, h_params.pin);

    MB1_L1.gpio_init(&l1_params);
    MB1_L2.gpio_init(&l2_params);
    MB1_L3.gpio_init(&l3_params);
    MB1_L4.gpio_init(&l4_params);

    MB1_A.gpio_init(&a_params);
    MB1_B.gpio_init(&b_params);
    MB1_C.gpio_init(&c_params);
    MB1_D.gpio_init(&d_params);
    MB1_h.gpio_init(&h_params);

}

/*----------------------------------------------------------------------------*/
void led7seg_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "All IO pins will be reset to IN_FLOATING\n");

    /* Shutdown GPIOs */
    MB1_L1.gpio_shutdown();
    MB1_L2.gpio_shutdown();
    MB1_L3.gpio_shutdown();
    MB1_L4.gpio_shutdown();
    MB1_A.gpio_shutdown();
    MB1_B.gpio_shutdown();
    MB1_C.gpio_shutdown();
    MB1_D.gpio_shutdown();
    MB1_h.gpio_shutdown();
}

/*----------------------------------------------------------------------------*/
void led7seg_testing_all(void)
{
    start_waiting_esc_character();

    /* Configuration */
    HA_NOTIFY("\n*** TEST LED7SEG ***\n"
              "(press ESC to quit).\n");
    led_number(all);
    led_data(8,0);
    while(1)
    {
        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    } //End while()

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void led7seg_testing_scan(void)
{
    start_waiting_esc_character();
    HA_NOTIFY("\n*** TEST LED7SEG SCAN  ***\n"
                  "(press ESC to quit).\n");
    while(1)
    {
        for(int led_index=1;led_index<=4;led_index++)
        {
            led_number(led_index);
            led_data(led_index,1);
            testing_delay_us(1000);
        }
        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void led7seg_full_test(void)
{
    led7seg_testing_init();
    led7seg_testing_all();
    led7seg_testing_scan();
    led7seg_testing_deinit();
}

/*----------------------------------------------------------------------------*/
/* LED7SEG function */
void led_data(uint8_t number, bool h)
{
    switch (number){
    /* Number 0 */
    case 0:
        MB1_A.gpio_assign_value(!data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 1 */
    case 1:
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 2 */
    case 2:
        MB1_A.gpio_assign_value(!data_set_value);
        MB1_B.gpio_assign_value(data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 3 */
    case 3:
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 4 */
    case 4:
        MB1_A.gpio_assign_value(!data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 5 */
    case 5:
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 6 */
    case 6:
        MB1_A.gpio_assign_value(!data_set_value);
        MB1_B.gpio_assign_value(data_set_value);
        MB1_C.gpio_assign_value(data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 7 */
    case 7:
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(data_set_value);
        MB1_C.gpio_assign_value(data_set_value);
        MB1_D.gpio_assign_value(!data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 8 */
    case 8:
        MB1_A.gpio_assign_value(!data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    /* Number 9 */
    case 9:
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(!data_set_value);
        MB1_C.gpio_assign_value(!data_set_value);
        MB1_D.gpio_assign_value(data_set_value);
        MB1_h.gpio_assign_value(h);
        break;

    default:
        HA_NOTIFY("Data must be 0 ~ 9.\n");
        MB1_A.gpio_assign_value(data_set_value);
        MB1_B.gpio_assign_value(data_set_value);
        MB1_C.gpio_assign_value(data_set_value);
        MB1_D.gpio_assign_value(data_set_value);
        MB1_h.gpio_assign_value(h);
        break;
    }

}
void led_number(uint8_t lednumber)
{
    switch (lednumber){
    case all:
        MB1_L1.gpio_assign_value(l_enable_value);
        MB1_L2.gpio_assign_value(l_enable_value);
        MB1_L3.gpio_assign_value(l_enable_value);
        MB1_L4.gpio_assign_value(l_enable_value);
        break;
    case 1:
        MB1_L1.gpio_assign_value(l_enable_value);
        MB1_L2.gpio_assign_value(!l_enable_value);
        MB1_L3.gpio_assign_value(!l_enable_value);
        MB1_L4.gpio_assign_value(!l_enable_value);
        break;
    case 2:
        MB1_L1.gpio_assign_value(!l_enable_value);
        MB1_L2.gpio_assign_value(l_enable_value);
        MB1_L3.gpio_assign_value(!l_enable_value);
        MB1_L4.gpio_assign_value(!l_enable_value);
        break;
    case 3:
        MB1_L1.gpio_assign_value(!l_enable_value);
        MB1_L2.gpio_assign_value(!l_enable_value);
        MB1_L3.gpio_assign_value(l_enable_value);
        MB1_L4.gpio_assign_value(!l_enable_value);
        break;
    case 4:
        MB1_L1.gpio_assign_value(!l_enable_value);
        MB1_L2.gpio_assign_value(!l_enable_value);
        MB1_L3.gpio_assign_value(!l_enable_value);
        MB1_L4.gpio_assign_value(l_enable_value);
        break;
    default:
        HA_NOTIFY("You can choose led_number 1 ~ 4.\n");
        MB1_L1.gpio_assign_value(!l_enable_value);
        MB1_L2.gpio_assign_value(!l_enable_value);
        MB1_L3.gpio_assign_value(!l_enable_value);
        MB1_L4.gpio_assign_value(!l_enable_value);
        break;
    }//End switch case
}
