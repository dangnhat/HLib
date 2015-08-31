/**
 * @file rs485_testing_cmds.h
 * @author  Huynh Van Minh  <huynhminh009@gmail.com>.
 * @version 1.0
 * @date 29-July-2015
 * @brief Source file for RS485 testing shell commands.
 * (Need initialized MB1_rtc object to implement the timeout code)
 */

#include "button_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* UP =  */
static const gpio_ns::gpio_params_t up_params = {
        gpio_ns::port_B,
        8,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static gpio MB1_up;

/* DOWN =  */
static const gpio_ns::gpio_params_t down_params = {
        gpio_ns::port_B,
        9,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static gpio MB1_down;

/* LEFT =  */
static const gpio_ns::gpio_params_t left_params = {
        gpio_ns::port_B,
        7,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static gpio MB1_left;

/* RIGHT =  */
static const gpio_ns::gpio_params_t right_params = {
        gpio_ns::port_B,
        6,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static gpio MB1_right;

/* SELECT =  */
static const gpio_ns::gpio_params_t select_params = {
        gpio_ns::port_C,
        13,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static gpio MB1_select;
/* End configuration data */

/* Shell command usages */
const char button_erx_test_usage[] = "Usage:\n"
        "button_test -i, initialize hardware and data for the test.\n"
		"button_test -d, Deinitialize hardware and data for the test.\n"
		"button_test -f,perform full test.\n"
        "button_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for BUTTON module.
 */
static void button_testing_init(void);

/* Private function prototypes */
/**
 * @brief   Deinitialize hardware and data for BUTTON module.
 */
static void button_testing_deinit(void);

static void button_testing_full(void);

static void button_testing(void);

/* BUTTON functions */
static bool is_press_button(bool button);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void button_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", button_erx_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                	button_testing_init();
                    break;

                case 'f':
                	button_testing_full();
                	break;

                case 'h':
                    HA_NOTIFY("%s", button_erx_test_usage);
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
void button_testing_init(void)
{
    HA_NOTIFY("up pin: port: %u (port A = 0,...), pin: %u.\n"
    		"down pin: port: %u (port A = 0,...), pin: %u.\n"
    		"left pin: port: %u (port A = 0,...), pin: %u.\n"
    		"right pin: port: %u (port A = 0,...), pin: %u.\n"
    		"select pin: port: %u (port A = 0,...), pin: %u.\n",
            up_params.port, up_params.pin,
			down_params.port, down_params.pin,
			left_params.port, left_params.pin,
			right_params.port, right_params.pin,
			select_params.port, select_params.pin);

    MB1_up.gpio_init(&up_params);
    MB1_down.gpio_init(&down_params);
    MB1_left.gpio_init(&left_params);
    MB1_right.gpio_init(&right_params);
    MB1_select.gpio_init(&select_params);
    testing_delay_us(100000);
}

/*----------------------------------------------------------------------------*/
void button_testing_deinit(void)
{
	HA_NOTIFY("\n*** Deinitializing hardware ***\n"
	            "All IO pins will be reset to IN_FLOATING\n");
	MB1_up.gpio_shutdown();
	MB1_down.gpio_shutdown();
	MB1_left.gpio_shutdown();
	MB1_right.gpio_shutdown();
	MB1_select.gpio_shutdown();
}

/*----------------------------------------------------------------------------*/
void button_testing(void)
{
	start_waiting_esc_character();

	HA_NOTIFY("\n*** BUTTON & MBOARD TEST ***\n"
	                "(press ESC to quit).\n");

	while(1)
	{
		if(is_press_button(MB1_up.gpio_read()))
		{
			testing_delay_us(10000);
			while(is_press_button(MB1_up.gpio_read()));
			HA_NOTIFY("Press button UP.\n");
		}
		if(is_press_button(MB1_down.gpio_read()))
		{
			testing_delay_us(10000);
			while(is_press_button(MB1_down.gpio_read()));
			HA_NOTIFY("Press button DOWN.\n");
		}
		if(is_press_button(MB1_left.gpio_read()))
		{
			testing_delay_us(10000);
			while(is_press_button(MB1_left.gpio_read()));
			HA_NOTIFY("Press button LEFT.\n");
		}
		if(is_press_button(MB1_right.gpio_read()))
		{
			testing_delay_us(10000);
			while(is_press_button(MB1_right.gpio_read()));
			HA_NOTIFY("Press button RIGHT.\n");
		}
		if(is_press_button(MB1_select.gpio_read()))
		{
			testing_delay_us(10000);
			while(is_press_button(MB1_select.gpio_read()));
			HA_NOTIFY("Press button SELECT.\n");
		}

		/* poll the esc_pressed */
		if (esc_pressed == true) {
		    break;
		}
	}//End while().

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void button_testing_full(void)
{
	button_testing_init();
	button_testing();
	button_testing_deinit();
}


/*----------------------------------------------------------------------------*/
bool is_press_button(bool button)
{
	if(button == 1) return 0;
	else return 1;
}
