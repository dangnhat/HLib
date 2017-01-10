/**
 * @file ble_testing_cmds.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 5-Aug-2015
 * @brief Source file for BLE testing shell commands.
 */

#include "ble_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

/* BGLib */
#include "apitypes.h"
#include "cmd_def.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* USART */
static serial_t* MB1_usart_p = &MB1_USART3;
static const uint16_t baudrate = 9600;
static const ISRMgr_ns::ISR_t isr_handlers = ISRMgr_ns::ISRMgr_USART3;

/* For BGLib */
static uint8_t usart_buffer[256];
static uint16_t idxBuf = 0;

/* RESET pin */
static const gpio_ns::gpio_params_t rst_params = {
        gpio_ns::port_B,
        3,
        gpio_ns::out_push_pull,
        gpio_ns::speed_2MHz,
};
static const bool rst_active_value = 0;
static gpio MB1_rst;
/* End configuration data */

/* Shell command usages */
static const char ble_test_usage[] = "Usage:\n"
        "ble_test -i, initialize hardware and data for the test.\n"
        "ble_test -d, deinitialize hardware and data for the test.\n"
        "ble_test -w, warm reset test.\n"
        "ble_test -c, BLE and MBoard communication test.\n"
        "ble_test -f, perform full test = ble_test -i -c -w.\n"
        "ble_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for BLE module.
 */
static void ble_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for BLE module.
 */
static void ble_testing_deinit(void);

/**
 * @brief   Warm reset test.
 */
static void ble_warm_reset_test(void);

/**
 * @brief   MBoard-1 and BLE module communication test. 2 following commands will
 *          be used to perform the test.
 *          hello, BLE will say Hello after receiving this command.
 *          get_info, Read software and hardware versions of BLE module.
 */
static void ble_communication_test(void);

/**
 * @brief   Full test. This includes initialization, communication test and warm reset test.
 */
static void ble_full_test(void);

/* BGLib functions */
static void bglib_send_message(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2);
static void bglib_parse_message(void);
static void bglib_usart_isr(void);

/* Public function implementations */
/*----------------------------------------------------------------------------*/
void ble_test(int argc, char **argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", ble_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    ble_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    ble_testing_deinit();
                    break;

                case 'w':
                    /* Warm reset test */
                    ble_warm_reset_test();
                    break;

                case 'c':
                    /* Communication test*/
                    ble_communication_test();
                    break;

                case 'f':
                    /* Full test */
                    ble_full_test();
                    break;

                case 'h':
                    HA_NOTIFY("%s", ble_test_usage);
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
void ble_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for BLE tests ***\n"
            "USART: %u, Buadrate: %u,\n"
            "RST pin: port: %u (port A = 0,...), pin: %u, active value: %u.\n",
            MB1_usart_p->Get_usedUart(), baudrate,
            rst_params.port, rst_params.pin, rst_active_value);

    MB1_usart_p->Restart(baudrate);
    MB1_usart_p->it_enable(0x2, 0x2);
    MB1_usart_p->it_config(serial_ns::it_rxne, true);
    MB1_rst.gpio_init(&rst_params);

    /* Hold rst for 100ms and release */
    MB1_rst.gpio_assign_value(rst_active_value);
    testing_delay_us(100000);
    MB1_rst.gpio_assign_value(!rst_active_value);

    /* BGLib init */
    bglib_output = bglib_send_message;
    MB1_ISRs.subISR_assign(isr_handlers, bglib_usart_isr);
}

/*----------------------------------------------------------------------------*/
void ble_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "USART: %u, will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n",
            MB1_usart_p->Get_usedUart());

    /* Shutdown USART and GPIOs */
    MB1_usart_p->it_disable();
    MB1_usart_p->Shutdown();
    MB1_rst.gpio_shutdown();

    /* Deinit BGLib */
    bglib_output = NULL;
    MB1_ISRs.subISR_remove(isr_handlers, bglib_usart_isr);
}

/*----------------------------------------------------------------------------*/
void ble_communication_test(void)
{
    start_waiting_esc_character();

    HA_NOTIFY("\n*** BLE & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    /* Periodically send hello and get_info commands */
    while (1) {
        /* Hello command */
        HA_NOTIFY("Sending Hello command\n");
        ble_cmd_system_hello();
        /* time out to receive data */
        testing_delay_us(1000000);

        /* Get_info command */
        HA_NOTIFY("Sending get_info command.\n");
        ble_cmd_system_get_info();
        /* time out to receive data */
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
void ble_warm_reset_test(void)
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

/*----------------------------------------------------------------------------*/
void ble_full_test(void)
{
    ble_testing_init();

    ble_communication_test();

    ble_warm_reset_test();

    ble_testing_deinit();
}

/* BGLib functions */
/*----------------------------------------------------------------------------*/
void bglib_send_message(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2)
{
    //this line assumes the BLE module is in packet mode, meaning the
    //length of the packet must be specified immediately before sending
    //the packet; this line does that

    uint8_t len = len1 + (uint8_t) len2;
    MB1_usart_p->Out(len);

    //this loop sends the header of the BLE Message
    for (uint8_t i = 0; i < len1; i++) {
        MB1_usart_p->Out(data1[i]);
    }

    //this loop sends the payload of the BLE Message
    for (uint8_t i = 0; i < len2; i++) {
        MB1_usart_p->Out(data2[i]);
    }
}

/*----------------------------------------------------------------------------*/
void bglib_parse_message(void)
{
    const struct ble_msg *BTMessage;         //holds BLE message
    struct ble_header BTHeader;              //holds header of message
    uint8_t data[256] = "\0";                //holds payload of message

    //read BLE message header
    BTHeader.type_hilen = usart_buffer[0];
    BTHeader.lolen = usart_buffer[1];
    BTHeader.cls = usart_buffer[2];
    BTHeader.command = usart_buffer[3];

    //read the payload of the BLE Message

    for (uint8_t i = 0; i < BTHeader.lolen; i++) {
        data[i] = usart_buffer[i + 4];
    }

    //find the appropriate message based on the header, which allows
    //the ble112 library to call the appropriate handler
    BTMessage = ble_get_msg_hdr(BTHeader);

    //print error if the header doesn't match any known message header
    if (!BTMessage) {
        //handle error here
        return;
    }
    //call the handler for the received message, passing in the received payload data
    BTMessage->handler(data);
}

/*----------------------------------------------------------------------------*/
void bglib_usart_isr(void) {
    /* Check interrupt flags */
    if (MB1_usart_p->it_get_status(serial_ns::it_rxne)) {
        usart_buffer[idxBuf++] = MB1_usart_p->Get_ISR();

        if ((idxBuf > 1) && (idxBuf == (usart_buffer[1] + 4))) {  //END of packet
            idxBuf = 0;
            bglib_parse_message();                             // Parse data from packet
        }
    }
}

/* BGLib Event callbacks */
/*----------------------------------------------------------------------------*/
void ble_evt_system_boot(const struct ble_msg_system_boot_evt_t *msg)
{
    HA_NOTIFY("\nBGLib callback: SYSTEM BOOT.\n");
//    HA_NOTIFY("- Software version:\n"
//            "\tmajor(%hu), minor(%hu), \n"
//            "\tpatch(%hu), build(%hu),\n"
//            "\tll version(%hu), protocol version(%hu),\n"
//            "\thw version(%hu).\n", msg->major, msg->minor, msg->patch, msg->build,
//            msg->ll_version, msg->protocol_version, msg->hw);
}

/*----------------------------------------------------------------------------*/
void ble_rsp_system_reset(const void *nul)
{
    HA_NOTIFY("\nBGLib callback: SYSTEM RESET.\n");
}

/*----------------------------------------------------------------------------*/
void ble_rsp_system_hello(const void *nul)
{
    HA_NOTIFY("\nBGLib callback: HELLO.\n");
}

/*----------------------------------------------------------------------------*/
void ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t *msg)
{
    HA_NOTIFY("\nBGLib callback: SYSTEM GET INFO.\n");
//    HA_NOTIFY("- Software version: "
//            "\tmajor(%hu), minor(%hu), \n"
//            "\tpatch(%hu), build(%hu),\n"
//            "\tll version(%hu), protocol version(%hu),\n"
//            "\thw version(%hu).\n", msg->major, msg->minor, msg->patch, msg->build,
//            msg->ll_version, msg->protocol_version, msg->hw);
}

/*----------------------------------------------------------------------------*/
void ble_rsp_hardware_set_soft_timer(
        const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{
    HA_DEBUG("-- set soft timer --\n");

}

/*----------------------------------------------------------------------------*/
void ble_rsp_sm_set_bondable_mode(const void *nul)
{
    HA_DEBUG("-- bondable --\n");
}

/*----------------------------------------------------------------------------*/
void ble_rsp_gap_set_mode(const struct ble_msg_gap_set_mode_rsp_t *msg)
{
    HA_DEBUG("-- discoverable --\n");
}

/*----------------------------------------------------------------------------*/
void ble_evt_connection_disconnected(
        const struct ble_msg_connection_disconnected_evt_t *msg)
{
    HA_DEBUG("-- remote device disconnected --\n");
}

/*----------------------------------------------------------------------------*/
void ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg)
{
    HA_DEBUG("client write \n");
}

/*----------------------------------------------------------------------------*/
void ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t *msg)
{
    HA_DEBUG("-- write local--\n");
}

/*----------------------------------------------------------------------------*/
void ble_evt_connection_status(
        const struct ble_msg_connection_status_evt_t *msg)
{
    HA_DEBUG("-- client connected --\n");
}
