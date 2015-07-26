/**
 * @file ha_system.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Oct-2014
 * @brief This is implement the init function for HA System.
 */

#include "ha_system.h"

#include "cc110x_reconfig.h" /* to re-init CC1101 module to use with 433MHz */
#include "diskio.h" /* for FAT FS initialization */
#include "ha_sixlowpan.h"

/******************** Config interface ****************************************/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/* 10ms or 1ms timer is required for FAT FS */
const ISRMgr_ns::ISR_t timer_1ms = ISRMgr_ns::ISRMgr_TIM6;

const char default_drive_path[] = "0:/";
FATFS fatfs;

/* 6LoWPAN restart */
Button* stop_resart_slp_btn_p = &MB1_usrBtn1;

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(void)
{
    /* Init MB1_system */
    MB1_system_init();
    HA_NOTIFY("MB1_system initialized.\n");
}
