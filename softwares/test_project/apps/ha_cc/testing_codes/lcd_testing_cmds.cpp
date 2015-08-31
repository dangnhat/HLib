/*
 * lcd_testing_cmds.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: minh
 */

#include "lcd_testing_cmds.h"
#include "clcd16x2.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* End configuration data */

/*--------------------------------------------------------------------------------------*/
/* Shell command usages */
const char lcd_test_usage[] = "Usage:\n"
        "lcd_test -i, initialize hardware and data for the test.\n"
		"lcd_test -d, deinitialize hardware and data for the test.\n"
		"lcd_test -f, perform full.\n"
		"lcd_test -h, print the usage.\n"
		"Press ESC to stop the test.\n";
/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for LED7SEG module.
 */

/**
 * @brief   Deinitialize hardware and data for LED7SEG module.
 */
//static void lcd_testing_deinit(void);

/**
 * @brief   Full test.
 */
static void lcd_testing_full(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void lcd_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", lcd_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    break;

                case 'f':
                	lcd_testing_full();
                	break;

                case 'h':
                    HA_NOTIFY("%s", lcd_test_usage);
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

/*----------------------------------------------------------------------------*/

void lcd_testing_full(void)
{
	HA_NOTIFY("LCD Test start.\n"
			"Press ESC to stop the test.\n");
	clcd16x2 lcd;
	lcd.clear();
	lcd.set_backlight(1);
	lcd.printf("\tTEST LCD16x2\n\tHELLO MINH");
    start_waiting_esc_character();
    while(1)
    {
    	/* poll the esc_pressed */
    	if (esc_pressed == true) {
    	    break;
    	}
    }

	lcd.clear();
    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}
