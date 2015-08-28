/**
 * @file adceb_testing_cmds.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 28-Aug-2015
 * @brief Source file for ADC EBoard testing shell commands.
 */

#include "adceb_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"
#include "math.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;
using namespace adc_ns;
using namespace gpio_ns;

/* Configuration data */

const uint16_t v_ref = 3300; /* mV */
const uint16_t adc_resolution = 4096; /* 12 bit adc */

/* Photo-resistor ADC and GPIO */
static const adc_params_t photores_adc_params = {
    independent,
    adc1,
    5,
    continuous_mode,
    regular_channel,
    no_option,
    ADC_SampleTime_28Cycles5,
    poll,
};

static const gpio_ns::gpio_params_t photores_gpio_params = {
        port_A,
        5,
        in_analog,
        speed_50MHz,
};
static gpio photores_gpio;

/* NTC ADC and GPIO */
static const adc_params_t ntc_adc_params = {
    independent,
    adc1,
    6,
    continuous_mode,
    regular_channel,
    no_option,
    ADC_SampleTime_28Cycles5,
    poll,
};

static const gpio_ns::gpio_params_t ntc_gpio_params = {
        port_A,
        6,
        in_analog,
        speed_50MHz,
};
static gpio ntc_gpio;

/* TSL12T light sensor ADC and GPIO */
static const adc_params_t lightic_adc_params = {
    independent,
    adc1,
    4,
    continuous_mode,
    regular_channel,
    no_option,
    ADC_SampleTime_28Cycles5,
    poll,
};

static const gpio_ns::gpio_params_t lightic_gpio_params = {
        port_A,
        4,
        in_analog,
        speed_50MHz,
};
static gpio lightic_gpio;

/* LMT86 temperature sensor ADC and GPIO */
static const adc_params_t tmpic_adc_params = {
    independent,
    adc1,
    7,
    continuous_mode,
    regular_channel,
    no_option,
    ADC_SampleTime_28Cycles5,
    poll,
};

static const gpio_ns::gpio_params_t tmpic_gpio_params = {
        port_A,
        7,
        in_analog,
        speed_50MHz,
};
static gpio tmpic_gpio;

/* End configuration data */

/* Shell command usages */
static const char adceb_test_usage[] = "Usage:\n"
        "adceb_test -i, initialize hardware and data for the test.\n"
        "adceb_test -d, deinitialize hardware and data.\n"
        "adceb_test -p, Photo-resistor test.\n"
        "adceb_test -l, TSL12T light sensor IC test.\n"
        "adceb_test -t, LMT8x temperature IC test.\n"
        "adceb_test -n, NTC test.\n"
        "adceb_test -f, perform full test = adceb_test -i -p -l -n -t -d.\n"
        "adceb_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for ADC EBoard.
 */
static void adceb_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for ADC EBoard.
 */
static void adceb_testing_deinit(void);

/**
 * @brief   MBoard-1 and Photo-resistor test.
 *          Tester should observe the measured light intensity (lux).
 */
static void adceb_photores_test(void);

/**
 * @brief   MBoard-1 and TSL12T light sensor IC test.
 *          Tester should observe the measured light intensity (lux).
 */
static void adceb_lightic_test(void);

/**
 * @brief   MBoard-1 and NTC test.
 *          Tester should observe the measured temperature (oC).
 */
static void adceb_ntc_test(void);

/**
 * @brief   MBoard-1 and LMT8x temperature IC test.
 *          Tester should observe the measured temperature (oC).
 */
static void adceb_tmpic_test(void);

/**
 * @brief   Full test.
 */
static void adceb_full_test(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void adceb_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", adceb_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    adceb_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    adceb_testing_deinit();
                    break;

                case 'p':
                    /* Photo-resistor test*/
                    adceb_photores_test();
                    break;

                case 'l':
                    /* TSL12T light sensor IC test */
                    adceb_lightic_test();
                    break;

                case 'n':
                    /* NTC test */
                    adceb_ntc_test();
                    break;

                case 't':
                    /* LMT86 temperature sensor IC test */
                    adceb_tmpic_test();
                    break;

                case 'f':
                    /* Full test */
                    adceb_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", adceb_test_usage);
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
static void adceb_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for ADC EBoard tests ***\n"
            "Photo-res pin: port: %u (port A = 0,...), pin: %u\n"
            "TSL12T light sensor ic pin: port: %u (port A = 0,...), pin: %u\n"
            "NTC pin: port: %u (port A = 0,...), pin: %u\n"
            "LMT8x temperature sensor ic pin: port: %u (port A = 0,...), pin: %u\n",
            photores_gpio_params.port, photores_gpio_params.pin,
            lightic_gpio_params.port, lightic_gpio_params.pin,
            ntc_gpio_params.port, ntc_gpio_params.pin,
            tmpic_gpio_params.port, tmpic_gpio_params.pin);

    /* Init GPIOs */
    photores_gpio.gpio_init(&photores_gpio_params);
    lightic_gpio.gpio_init(&lightic_gpio_params);
    ntc_gpio.gpio_init(&ntc_gpio_params);
    tmpic_gpio.gpio_init(&tmpic_gpio_params);
}

/*----------------------------------------------------------------------------*/
static void adceb_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "All IO pins will be reset to IN_FLOATING\n");

    /* Shutdown USART and GPIOs */
    photores_gpio.gpio_shutdown();
    lightic_gpio.gpio_shutdown();
    ntc_gpio.gpio_shutdown();
    tmpic_gpio.gpio_shutdown();
}

/*----------------------------------------------------------------------------*/
static void adceb_photores_test(void)
{
    adc photores;
    uint16_t adc_value;
    double r, lumi;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** PHOTO-RESISTOR TEST ***\n"
                "(press ESC to quit).\n");

    photores.adc_init(&photores_adc_params);
    photores.adc_start();

    HA_NOTIFY("\nADC:\tLumi(lux):\n");

    while (1) {
        adc_value = photores.adc_convert();

        /* Convert to lux */
        r = (double)adc_resolution/(double)adc_value - 1;
        lumi = 562.3413 * pow(r, -1.25);

        HA_NOTIFY("\r%-4u\t%-5u", adc_value, (uint16_t) lumi);
        HA_FLUSH_STDOUT();
        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    photores.adc_stop();

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void adceb_lightic_test(void)
{
    adc lightic;
    uint16_t adc_value;
    double v, lumi;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** TSL12T light sensor IC TEST ***\n"
                "(press ESC to quit).\n");

    lightic.adc_init(&lightic_adc_params);
    lightic.adc_start();

    HA_NOTIFY("\nADC:\tLumi(uW/cm2):\n");

    while (1) {
        adc_value = lightic.adc_convert();

        /* Convert to lux */
        v = (double)adc_value/(double)adc_resolution * v_ref;
        lumi = v / 96;

        HA_NOTIFY("\r%-4u\t%-5u", adc_value, (uint16_t) lumi);
        HA_FLUSH_STDOUT();
        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    lightic.adc_stop();

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void adceb_ntc_test(void)
{
    adc ntc;
    uint16_t adc_value;
    double r, tmp;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** NTC thermistor test  ***\n"
                "(press ESC to quit).\n");

    ntc.adc_init(&ntc_adc_params);
    ntc.adc_start();

    HA_NOTIFY("\nADC:\tTemperature(oC):\n");

    while (1) {
        adc_value = ntc.adc_convert();

        /* Convert to lux */
        r = ((double)adc_resolution * 100)/(double)adc_value - 100;
        tmp = 3990 / (8.7852 + log(r)) - 273;

        HA_NOTIFY("\r%-4u\t%-5u", adc_value, (uint16_t) tmp);
        HA_FLUSH_STDOUT();
        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    ntc.adc_stop();

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}

/*----------------------------------------------------------------------------*/
static void adceb_tmpic_test(void)
{
    adc tmpic;
    uint16_t adc_value;
    double v, tmp;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** LMT86 temperature sensor IC TEST ***\n"
                "(press ESC to quit).\n");

    tmpic.adc_init(&tmpic_adc_params);
    tmpic.adc_start();

    HA_NOTIFY("\nADC:\tTemperature(oC):\n");

    while (1) {
        adc_value = tmpic.adc_convert();

        /* Convert to lux */
        v = (double)adc_value/(double)adc_resolution * v_ref;
        tmp = (2103 - v)/10.9;

        HA_NOTIFY("\r%-4u\t%-5u", adc_value, (uint16_t) tmp);
        HA_FLUSH_STDOUT();
        testing_delay_us(500000);

        /* poll the esc_pressed */
        if (esc_pressed == true) {
            break;
        }
    }

    tmpic.adc_stop();

    stop_waiting_esc_character();
    HA_NOTIFY("\nTest stopped.\n");
}


/*----------------------------------------------------------------------------*/
static void adceb_full_test(void)
{
    adceb_testing_init();

    adceb_photores_test();

    adceb_lightic_test();

    adceb_ntc_test();

    adceb_tmpic_test();

    adceb_testing_deinit();
}
