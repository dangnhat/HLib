/**
 * @file ha_system.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Oct-2014
 * @brief This is implement the init function for HA System.
 */

#include "ha_system.h"

/******************** Config interface ****************************************/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/*------------------- Functions ----------------------------------------------*/
void ha_system_init(void)
{
    /* Init MB1_system */
    MB1_system_init();
    HA_NOTIFY("MB1_system initialized.\n");
}
