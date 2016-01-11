/**
 * @file caneb_testing_cmds.cpp
 * @author  Nguyen Dinh Trung Truc  <truc.ndtt@gmail.com>.
 * @version 1.0
 * @date 11-Jan-2015
 * @brief Source file for CAN EBoard testing shell commands.
 */

#include "caneb_testing_cmds.h"
#include "os_dependent_code.h"
#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace testing_ns;

/* Configuration data */
/* CAN TX pin */
static const gpio_ns::gpio_params_t cantx_params = {
        gpio_ns::port_B,
        9,
        gpio_ns::af_push_pull,
        gpio_ns::speed_50MHz,
};
static gpio MB_cantx_pin;

/* CAN RX pin */
static const gpio_ns::gpio_params_t canrx_params = {
        gpio_ns::port_B,
        8,
        gpio_ns::in_pull_up,
        gpio_ns::speed_50MHz,
};
static gpio MB_canrx_pin;

/* End configuration data */

/* Shell command usages */
static const char caneb_test_usage[] = "Usage:\n"
        "caneb_test -i, initialize hardware and data for the test.\n"
        "caneb_test -d, deinitialize hardware and data.\n"
        "caneb_test -s, full test as a slave.\n"
        "caneb_test -m, full test as a master.\n"
        "caneb_test -h, print the usage.\n"
        "Press ESC to stop the test.\n";

/* Private function prototypes */
/**
 * @brief   Initialize hardware and data for CAN EBoard.
 */
static void caneb_testing_init(void);

/**
 * @brief   Deinitialize hardware and data for CAN EBoard.
 */
static void caneb_testing_deinit(void);

/**
 * @brief   Configure CAN to function properly.
 */
static void can_config(void);

/**
 * @brief   Initialize parameters for a receive message.
 * @param[in]   RxMsg, pointer to a receive message.
 */
static void can_rxmsg_init(CanRxMsg *RxMsg);

/**
 * @brief   Initialize parameters for a transmit message.
 * @param[in]   RxMsg, pointer to a transmit message.
 */
static void can_txmsg_init(CanTxMsg *TxMsg);

/**
 * @brief   Perform a full test as a master (including init and deinit).
 */
static void caneb_testing_full_master(void);

/**
 * @brief   Perform a full test as a slave (including init and deinit).
 */
static void caneb_testing_full_slave(void);

/**
 * @brief   Perform 4 tests as a master.
 */
static void caneb_master_test(void);

/**
 * @brief   Transmit messages to a slave and receive them back.
 */
static void caneb_transmit_data(CanTxMsg *TxMsg);

/**
 * @brief   Receive messages from a master and transmit them back.
 */
static void caneb_slave_test(void);


/* Public function implementations */
/*----------------------------------------------------------------------------*/
void caneb_test(int argc, char** argv)
{
    uint8_t count;

    if (argc == 1) {
        HA_NOTIFY("Err: no option selected.\n");
        HA_NOTIFY("%s\n", caneb_test_usage);
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'i':
                    /* Init */
                    caneb_testing_init();
                    break;

                case 'd':
                    /* Deinit */
                    caneb_testing_deinit();
                    break;

                case 'm':
                    /* Perform full test as a master */
                    caneb_testing_full_master();
                    break;

                case 's':
                    /* Perform full test as a slave */
                    caneb_testing_full_slave();
                    break;

                case 'h':
                    /* Print usage */
                    HA_NOTIFY("%s", caneb_test_usage);
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
static void caneb_testing_init(void)
{
    HA_NOTIFY("\n*** Initializing hardware for CAN tests ***\n"
            "CAN TX pin: port: %u (port A = 0,...), pin: %u\n"
            "CAN RX pin: port: %u (port A = 0,...), pin: %u\n",
            cantx_params.port, cantx_params.pin,
            canrx_params.port, canrx_params.pin);

    /* CAN clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* Configure CAN */
    can_config();

    /* Initialize CAN TX, CAN RX */
    MB_canrx_pin.gpio_init(&canrx_params);
    MB_cantx_pin.gpio_init(&cantx_params);

    /* Remap CAN1 to PB8 (RX), PB9 (TX) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
}

/*----------------------------------------------------------------------------*/
static void caneb_testing_deinit(void)
{
    HA_NOTIFY("\n*** Deinitializing hardware ***\n"
            "CAN will be shut down\n"
            "All IO pins will be reset to IN_FLOATING\n");

    /* Deinitialize CAN */
    CAN_DeInit(CAN1);

    /* CAN clock disable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);

    /* Deinitialize CAN TX, CAN RX */
    MB_canrx_pin.gpio_shutdown();
    MB_cantx_pin.gpio_shutdown();

    /* Disable remaping CAN1 to PB8 (RX), PB9 (TX) */
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
}

/*----------------------------------------------------------------------------*/
static void can_config(void)
{
    CAN_InitTypeDef can_params;

    CAN_DeInit(CAN1);
    CAN_StructInit(&can_params);

    can_params.CAN_TTCM = DISABLE;
    can_params.CAN_ABOM = DISABLE;
    can_params.CAN_AWUM = DISABLE;
    can_params.CAN_NART = DISABLE;
    can_params.CAN_RFLM = DISABLE;
    can_params.CAN_TXFP = DISABLE;
    can_params.CAN_Mode = CAN_Mode_Normal;
    can_params.CAN_SJW = CAN_SJW_1tq;
    can_params.CAN_BS1 = CAN_BS1_8tq;
    can_params.CAN_BS2 = CAN_BS2_7tq;

    /* Baudrate */
    can_params.CAN_Prescaler = 600;

    CAN_Init(CAN1, &can_params);

    CAN_FilterInitTypeDef  CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
}

/*----------------------------------------------------------------------------*/
static void can_rxmsg_init(CanRxMsg *RxMsg)
{
    RxMsg->StdId = 0x00;
    RxMsg->IDE = CAN_ID_STD;
    RxMsg->RTR = CAN_RTR_DATA;
    RxMsg->DLC = 0;
    RxMsg->Data[0] = 0x00;
}

/*----------------------------------------------------------------------------*/
static void can_txmsg_init(CanTxMsg *TxMsg)
{
    TxMsg->StdId = 0x11;
    TxMsg->RTR = CAN_RTR_DATA;
    TxMsg->IDE = CAN_ID_STD;
    TxMsg->DLC = 1;
    TxMsg->Data[0] = 0;
}

/*----------------------------------------------------------------------------*/
static void caneb_transmit_data(CanTxMsg *TxMsg)
{
    CanRxMsg RxMsg;

    /* Configure receive message */
    can_rxmsg_init(&RxMsg);

    start_waiting_esc_character();

    while(1)
    {
        TxMsg->Data[0]++;

        /* Transmit a message */
        CAN_Transmit(CAN1, TxMsg);

        HA_NOTIFY("\nTransmitted: %d, ", TxMsg->Data[0]);

        HA_FLUSH_STDOUT();

        testing_delay_us(1000000);

        /* If there is an inbox then print it */
        if (CAN_MessagePending(CAN1, CAN_FIFO0) > 0)
        {
            CAN_Receive(CAN1, CAN_FIFO0, &RxMsg);
            HA_NOTIFY("Received: %d\n", RxMsg.Data[0]);
        }
        else
        {
            HA_NOTIFY("Received: NULL (timeout).\n");
        }

        if (esc_pressed == true)
        {
            break;
        }
    }

    stop_waiting_esc_character();
}

/*----------------------------------------------------------------------------*/
static void caneb_master_test(void)
{
    CanTxMsg TxMsg;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** CAN EBOARD & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    HA_NOTIFY("\nTesting in master mode. Make sure the slave is in slave mode."
            "\n4 tests in total. Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    /* Configure transmit message */
    can_txmsg_init(&TxMsg);

    /* TEST #1 */
    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #1: Connect H1 of the master to H1 of the slave.\n"
            "Disconnect H3 on both boards.\n"
            "The slave should transmit back what has been sent by the master.\n"
            "Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    caneb_transmit_data(&TxMsg);

    /* TEST #2 */
    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #2: Disconnect H1, connect COM1 of the master to COM1 "
            "of the slave.\n"
            "The rest must not change.\n"
            "The slave should transmit back what has been sent by the master.\n"
            "Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    caneb_transmit_data(&TxMsg);

    /* TEST #3 */
    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #3: Connect pin 1 & 2 of jumper H3 on the slave.\n"
            "The rest must not change.\n"
            "The slave should transmit back what has been sent by the master.\n"
            "Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    caneb_transmit_data(&TxMsg);

    /* TEST #4 */
    start_waiting_esc_character();

    HA_NOTIFY("\nTEST #4: Connect pin 2 & 3 of jumper H3 on the slave.\n"
            "The rest must not change.\n"
            "The slave should NOT receive what has been sent by the master.\n"
            "Press ESC to continue.\n");

    while (esc_pressed == false);
    stop_waiting_esc_character();

    caneb_transmit_data(&TxMsg);

    HA_NOTIFY("\nTest stopped\n");
}

/*----------------------------------------------------------------------------*/
static void caneb_slave_test(void)
{
    CanRxMsg RxMsg;
    CanTxMsg TxMsg;

    start_waiting_esc_character();

    HA_NOTIFY("\n*** CAN EBOARD & MBOARD COMMUNICAION TEST ***\n"
                "(press ESC to quit).\n");

    HA_NOTIFY("\nTesting in slave mode. Make sure the master is in master mode."
            "\nFollow the instructions on the master's terminal"
            "\nThis displays what has been received and transmit it back.\n");

    /* Configure receive message */
    can_rxmsg_init(&RxMsg);

    /* Configure transmit message */
    can_txmsg_init(&TxMsg);

    while(1)
    {
        /* If there is an inbox, get it and transmit it back */
        if (CAN_MessagePending(CAN1, CAN_FIFO0) > 0)
        {
            CAN_Receive(CAN1, CAN_FIFO0, &RxMsg);

            /* Configure and transmit the message */
            TxMsg.Data[0] = RxMsg.Data[0];
            CAN_Transmit(CAN1, &TxMsg);

            HA_NOTIFY("\nReceived: %d, Transmitted: %d\n", RxMsg.Data[0],
                    TxMsg.Data[0]);
        }

        if (esc_pressed == true)
        {
            break;
        }
    }

    stop_waiting_esc_character();
}

/*----------------------------------------------------------------------------*/
static void caneb_testing_full_slave(void)
{
    caneb_testing_init();
    caneb_slave_test();
    caneb_testing_deinit();
}

/*----------------------------------------------------------------------------*/
static void caneb_testing_full_master(void)
{
    caneb_testing_init();
    caneb_master_test();
    caneb_testing_deinit();
}

