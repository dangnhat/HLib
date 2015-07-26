#include "ha_system.h"
#include "os_dependent_code.h"

int main(void)
{
    /* Init HA System */
    ha_system_init();

    /* Start esc_waiting_thread*/
    esc_waiting_thread_start();

    /* Run shell */
    ha_shell_irun(NULL);

    return 0;
}
