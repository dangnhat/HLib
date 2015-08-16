/**
 * @file eth_testing_cmds.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 13-Aug-2015
 * @brief Source file for ETH testing shell commands.
 */

#include "eth_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* SPI */
static SPI* MB1_spi_p = &MB1_SPI2;
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
static const uint8_t num_of_ss_lines = 1;

enum ss_line_table: uint8_t {
    mb1_nss_pin_linenum = 0,
};

enum decoding_table: uint16_t {
    w5100z = SPI_ns::last_of_initial_table,
};

enum decoding_values_table: uint16_t {
    w5100z_value = 0,
    all_free_value = 1,
};

/* SS pin */
static const gpio_ns::gpio_params_t nss_params = {
        gpio_ns::port_B,
        12,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static gpio MB1_nss_pin;

/* INT pin */
static const gpio_ns::gpio_params_t int_params = {
        gpio_ns::port_C,
        6,
        gpio_ns::in_floating,
        gpio_ns::speed_2MHz,
};
static const bool int_active_value = 0;
static gpio MB1_int_pin;

/* W5100 data */
enum rw_opcode_e: uint8_t {
    read_op = 0x0F,
    write_op = 0xF0,
};

enum reg_address_e: uint16_t {
    mode_register = 0x0000, /* R/W */

    gateway_address_3 = 0x0001, /* R/W */
    gateway_address_2 = 0x0002, /* R/W */
    gateway_address_1 = 0x0003, /* R/W */
    gateway_address_0 = 0x0004, /* R/W */

    subnet_mask_3 = 0x0005, /* R/W */
    subnet_mask_2 = 0x0006, /* R/W */
    subnet_mask_1 = 0x0007, /* R/W */
    subnet_mask_0 = 0x0008, /* R/W */

    hardware_address_5 = 0x0009, /* R/W */
    hardware_address_4 = 0x000A, /* R/W */
    hardware_address_3 = 0x000B, /* R/W */
    hardware_address_2 = 0x000C, /* R/W */
    hardware_address_1 = 0x000D, /* R/W */
    hardware_address_0 = 0x000E, /* R/W */

    source_ip_address_3 = 0x000F, /* R/W */
    source_ip_address_2 = 0x0010, /* R/W */
    source_ip_address_1 = 0x0011, /* R/W */
    source_ip_address_0 = 0x0012, /* R/W */

    retry_time_value_1 = 0x0017, /* R/W */
    retry_time_value_0 = 0x0018, /* R/W */
    retry_count = 0x0019, /* R/W */
    rx_memory_size = 0x001A, /* R/W */
    tx_memory_size = 0x001B, /* R/W */
};

static const uint8_t w5100_mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static const uint8_t w5100_source_ip[] = {192, 168, 111, 3};
static const uint8_t w5100_subnet_mask[] = {255, 255, 255, 0};
static const uint8_t w5100_gateway[] = {192, 168, 111, 1};

/* End configuration data */

/* Shell command usages */
static const char eth_test_usage[] = "Usage:\n"
        "eth_test -i, initialize hardware and data for the test.\n"
        "eth_test -d, deinitialize hardware and data for the test.\n"
        "eth_test -c, BLE and MBoard communication test.\n"
        "eth_test -j, communication through RJ45 test.\n"
        "eth_test -f, perform full test = ble_test -i -c -w.\n"
        "eth_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for BLE module.
 */
static void eth_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for BLE module.
 */
static void eth_testing_deinit(void);

/**
 * @brief   MBoard-1 and BLE module communication test. 2 following commands will
 *          be used to perform the test.
 *          hello, BLE will say Hello after receiving this command.
 *          get_info, Read software and hardware versions of BLE module.
 */
static void eth_communication_test(void);

/**
 * @brief   Communication through RJ45 test.
 */
static void eth_rj45_communication_test(void);

/**
 * @brief   Full test. This includes initialization, communication test and warm reset test.
 */
static void eth_full_test(void);

/**
 * @brief   Read a register value of ETH IC (w5100z).
 *
 * @param[in]   spi_p, pointer to an initialized SPI object.
 * @param[in]   spi_dev_id, device id to be used with SPI object.
 * @param[in]   addr, 16bit address of a register in W5100 memory.
 *
 * @return  8bit value of the register.
 */
static uint8_t eth_read_register(SPI *spi_p, uint16_t spi_dev_id, uint16_t addr);

/**
 * @brief   Write a register value of ETH IC (w5100z).
 *
 * @param[in]   spi_p, pointer to an initialized SPI object.
 * @param[in]   spi_dev_id, device id to be used with SPI object.
 * @param[in]   addr, 16bit address of a register in W5100 memory.
 * @param[in]   8bit value of the register to be written.
 */
static void eth_write_register(SPI *spi_p, uint16_t spi_dev_id, uint16_t addr, uint8_t value);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void eth_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", eth_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    eth_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    eth_testing_deinit();
                    break;

                case 'c':
                    /* Communication test*/
                    eth_communication_test();
                    break;

                case 'j':
                    /* RJ45 Communication test */
                    eth_rj45_communication_test();
                    break;

                case 'f':
                    /* Full test */
                    eth_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", eth_test_usage);
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
void eth_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for ETH tests ***\n"
            "SPI: %u, Buadrate prescaler: %u,\n"
            "NSS pin: port: %u (port A = 0,...), pin: %u,\n"
            "INT pin: port: %u (port A = 0,...), pin: %u, active value: %u.\n",
            MB1_spi_p->get_usedSPI(), 1 << (spi_params.baudRatePrescaler/8 + 1),
            nss_params.port, nss_params.pin,
            int_params.port, int_params.pin, int_active_value);

    /* Init SPI and GPIOs */
    MB1_spi_p->init(&spi_params);
    MB1_nss_pin.gpio_init(&nss_params);
    MB1_int_pin.gpio_init(&int_params);

    /* Setup decoding table */
    MB1_spi_p->SM_numOfSSLines_set(num_of_ss_lines);
    MB1_spi_p->SM_GPIO_set(&MB1_nss_pin, mb1_nss_pin_linenum);
    MB1_spi_p->SM_deviceToDecoder_set(w5100z, w5100z_value);
    MB1_spi_p->SM_deviceToDecoder_set(SPI_ns::all_free, all_free_value);

    /* Attach */
    MB1_spi_p->SM_device_attach(w5100z);
}

/*----------------------------------------------------------------------------*/
void eth_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "SPI: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            MB1_spi_p->get_usedSPI());

    /* Release */
    MB1_spi_p->SM_device_release(w5100z);

    /* Shutdown USART and GPIOs */
    MB1_spi_p->deinit();
    MB1_spi_p->SM_deinit();
    MB1_int_pin.gpio_shutdown();
}

/*----------------------------------------------------------------------------*/
void eth_communication_test(void)
{
    uint16_t temp = 0;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** ETH IC & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Periodically read information from 5 registers from w5100 */
    while (1) {
        /* Read retry_time_value registers */
        HA_NOTIFY("\nRetry time-value:\n");
        temp = eth_read_register(MB1_spi_p, w5100z, retry_time_value_1);
        temp = (temp << 8) | (uint16_t)eth_read_register(MB1_spi_p, w5100z, retry_time_value_0);
        HA_NOTIFY("%u ms (default: 200ms).\n", temp/10);
        testing_delay_us(1000000);

        /* Read retry count registers */
        HA_NOTIFY("\nRetry count:\n");
        temp = eth_read_register(MB1_spi_p, w5100z, retry_count);
        HA_NOTIFY("%u (default: 8).\n", temp);
        testing_delay_us(1000000);

        /* Read rx memory size registers */
        HA_NOTIFY("\nRX memory sizes:\n");
        temp = eth_read_register(MB1_spi_p, w5100z, rx_memory_size);
        HA_NOTIFY("Sk0: %u KB, Sk1: %u KB, Sk2: %u KB, Sk3: %u KB.\n"
                "(default: 2,2,2,2).\n",
                1 << (temp & 0x0003),
                1 << ((temp & 0x000C) >> 2),
                1 << ((temp & 0x0030) >> 4),
                1 << ((temp & 0x00C0) >> 6));
        testing_delay_us(1000000);

        /* Read tx memory size registers */
        HA_NOTIFY("\nTX memory sizes:\n");
        temp = eth_read_register(MB1_spi_p, w5100z, tx_memory_size);
        HA_NOTIFY("Sk0: %u KB, Sk1: %u KB, Sk2: %u KB, Sk3: %u KB.\n"
                "(default: 2,2,2,2).\n",
                1 << (temp & 0x0003),
                1 << ((temp & 0x000C) >> 2),
                1 << ((temp & 0x0030) >> 4),
                1 << ((temp & 0x00C0) >> 6));
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
void eth_rj45_communication_test(void)
{
    start_waiting_esc_character();
    HA_NOTIFY("\n*** RJ45 COMMUNICATION TEST ***\n"
                "(press ESC to quit).\n");

    /* Set mode */
    HA_NOTIFY("\nSetting mode register...\n");
    eth_write_register(MB1_spi_p, w5100z, mode_register, 0);
    HA_NOTIFY("Mode register: %u (should be 0)\n",
            eth_read_register(MB1_spi_p, w5100z, mode_register));
    testing_delay_us(1000000);

    /* Set MAC address */
    HA_NOTIFY("\nSetting MAC address...\n");
    eth_write_register(MB1_spi_p, w5100z, hardware_address_5, w5100_mac[0]);
    eth_write_register(MB1_spi_p, w5100z, hardware_address_4, w5100_mac[1]);
    eth_write_register(MB1_spi_p, w5100z, hardware_address_3, w5100_mac[2]);
    eth_write_register(MB1_spi_p, w5100z, hardware_address_2, w5100_mac[3]);
    eth_write_register(MB1_spi_p, w5100z, hardware_address_1, w5100_mac[4]);
    eth_write_register(MB1_spi_p, w5100z, hardware_address_0, w5100_mac[5]);

    /* Read back MAC address */
    HA_NOTIFY("MAC address: %x:%x:%x:%x:%x:%x.\n",
            eth_read_register(MB1_spi_p, w5100z, hardware_address_5),
            eth_read_register(MB1_spi_p, w5100z, hardware_address_4),
            eth_read_register(MB1_spi_p, w5100z, hardware_address_3),
            eth_read_register(MB1_spi_p, w5100z, hardware_address_2),
            eth_read_register(MB1_spi_p, w5100z, hardware_address_1),
            eth_read_register(MB1_spi_p, w5100z, hardware_address_0));

    testing_delay_us(1000000);

    /* Set IP */
    HA_NOTIFY("\nSetting IP address...\n");
            eth_write_register(MB1_spi_p, w5100z, source_ip_address_3, w5100_source_ip[0]);
            eth_write_register(MB1_spi_p, w5100z, source_ip_address_2, w5100_source_ip[1]);
            eth_write_register(MB1_spi_p, w5100z, source_ip_address_1, w5100_source_ip[2]);
            eth_write_register(MB1_spi_p, w5100z, source_ip_address_0, w5100_source_ip[3]);

    /* Read back IP address */
    HA_NOTIFY("IP address: %u.%u.%u.%u.\n",
            eth_read_register(MB1_spi_p, w5100z, source_ip_address_3),
            eth_read_register(MB1_spi_p, w5100z, source_ip_address_2),
            eth_read_register(MB1_spi_p, w5100z, source_ip_address_1),
            eth_read_register(MB1_spi_p, w5100z, source_ip_address_0));

    testing_delay_us(1000000);

    /* Set subnet mask */
    HA_NOTIFY("\nSetting subnet mask...\n");
            eth_write_register(MB1_spi_p, w5100z, subnet_mask_3, w5100_subnet_mask[0]);
            eth_write_register(MB1_spi_p, w5100z, subnet_mask_2, w5100_subnet_mask[1]);
            eth_write_register(MB1_spi_p, w5100z, subnet_mask_1, w5100_subnet_mask[2]);
            eth_write_register(MB1_spi_p, w5100z, subnet_mask_0, w5100_subnet_mask[3]);

    /* Read back subnet mask */
    HA_NOTIFY("Subnet mask: %u.%u.%u.%u.\n",
            eth_read_register(MB1_spi_p, w5100z, subnet_mask_3),
            eth_read_register(MB1_spi_p, w5100z, subnet_mask_2),
            eth_read_register(MB1_spi_p, w5100z, subnet_mask_1),
            eth_read_register(MB1_spi_p, w5100z, subnet_mask_0));

    testing_delay_us(1000000);

    /* Set default gateway */
    HA_NOTIFY("\nSetting default gateway...\n");
            eth_write_register(MB1_spi_p, w5100z, gateway_address_3, w5100_gateway[0]);
            eth_write_register(MB1_spi_p, w5100z, gateway_address_2, w5100_gateway[1]);
            eth_write_register(MB1_spi_p, w5100z, gateway_address_1, w5100_gateway[2]);
            eth_write_register(MB1_spi_p, w5100z, gateway_address_0, w5100_gateway[3]);

    /* Read back default gateway */
    HA_NOTIFY("Default gateway: %u.%u.%u.%u.\n",
            eth_read_register(MB1_spi_p, w5100z, gateway_address_3),
            eth_read_register(MB1_spi_p, w5100z, gateway_address_2),
            eth_read_register(MB1_spi_p, w5100z, gateway_address_1),
            eth_read_register(MB1_spi_p, w5100z, gateway_address_0));

    testing_delay_us(1000000);

    /* Wait for ping or esc */
    HA_NOTIFY("\nConnect Ethernet EB to a computer and try to ping the EB from computer.\n"
            "Press ESC to escape.\n");

    /* poll the esc_pressed */
    while (esc_pressed != true) {
        ;
    }

    stop_waiting_esc_character();
    HA_NOTIFY("Test stopped.\n");
}

/*----------------------------------------------------------------------------*/
void eth_full_test(void)
{
    eth_testing_init();

    eth_communication_test();

    eth_rj45_communication_test();

    eth_testing_deinit();
}

/*----------------------------------------------------------------------------*/
uint8_t eth_read_register(SPI *spi_p, uint16_t spi_dev_id, uint16_t addr)
{
    uint8_t retval;

    /* Select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send data to W5100 */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, read_op);
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, (uint8_t)(addr >> 8));
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, (uint8_t)addr);
    retval = spi_p->M2F_sendAndGet_blocking(spi_dev_id, 0x00);

    /* Deselect */
    spi_p->SM_device_deselect(spi_dev_id);

    return retval;
}

/*----------------------------------------------------------------------------*/
void eth_write_register(SPI *spi_p, uint16_t spi_dev_id, uint16_t addr, uint8_t value)
{
    /* Select */
    spi_p->SM_device_select(spi_dev_id);

    /* Send data to W5100 */
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, write_op);
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, (uint8_t)(addr >> 8));
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, (uint8_t)addr);
    spi_p->M2F_sendAndGet_blocking(spi_dev_id, value);

    /* Deselect */
    spi_p->SM_device_deselect(spi_dev_id);
}
