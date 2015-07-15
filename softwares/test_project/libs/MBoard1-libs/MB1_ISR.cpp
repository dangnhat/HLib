/**
 * @file MB1_ISR.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 21-10-2013
 * @brief This is source file for interrupt handlers for MBoard-1.
 *
 */

/* Includes */
#include "MB1_ISR.h"
#include "thread.h" /* RIOT's header */

using namespace ISRMgr_ns;

typedef struct {
    callback_t callback;
    void *arg;
} ISR_cb_t;

/**<------------------- Sub ISR tables ---------------------*/

/**< SysTick sub ISR table */
void (*SysTick_subISR_table[numOfSubISR_max])(void);
/**< SysTick sub ISR table */

/**< SysTick sub ISR table */
void (*RTC_subISR_table[numOfSubISR_max])(void);
/**< SysTick sub ISR table */

/**< TIM6 sub ISR table */
void (*TIM6_subISR_table[numOfSubISR_max])(void);
/**< TIM6 sub ISR table */

/**< USART1 sub ISR table */
void (*USART1_subISR_table[numOfSubISR_max])(void);
/**< USART1 sub ISR table */

/**< USART3 sub ISR table */
void (*USART3_subISR_table[numOfSubISR_max])(void);
/**< USART3 sub ISR table */

/**< EXTI sub ISR table */
ISR_cb_t EXTI_subISR_table[16][numOfSubISR_max];
/**< EXTI sub ISR table */

/**<------------------- Sub ISR tables ---------------------*/

/**< class ISRMgr */
ISRMgr::ISRMgr(void)
{
    /**< Init sub ISR table of SysTick */
    SysTick_subISR_table_init();
    RTC_subISR_table_init();
    TIM6_subISR_table_init();
    USART1_subISR_table_init();
    EXTI_subISR_table_init();
    USART3_subISR_table_init();
    return;
}

/**
 * @brief subISR_assign. Assign a sub ISR function ptr to the sub ISR table of ISR_type
 * @param ISR_t ISR_type
 * @param void (* subISR_p)(void) (not a NULL ptr)
 * @return None.
 */
status_t ISRMgr::subISR_assign(ISR_t ISR_type, void (*subISR_p)(void))
{
    status_t retval = failed;

    if (subISR_p == NULL)
        return failed;

    switch (ISR_type) {
    case ISRMgr_SysTick:
        retval = subISR_SysTick_assign(subISR_p);
        break;
    case ISRMgr_RTC:
        retval = subISR_RTC_assign(subISR_p);
        break;
    case ISRMgr_TIM6:
        retval = subISR_TIM6_assign(subISR_p);
        break;
    case ISRMgr_USART1:
        retval = subISR_USART1_assign(subISR_p);
        break;
    case ISRMgr_USART3:
        retval = subISR_USART3_assign(subISR_p);
        break;
//    case ISRMgr_EXTI0:
//    case ISRMgr_EXTI1:
//    case ISRMgr_EXTI2:
//    case ISRMgr_EXTI3:
//    case ISRMgr_EXTI4:
//    case ISRMgr_EXTI5:
//    case ISRMgr_EXTI6:
//    case ISRMgr_EXTI7:
//    case ISRMgr_EXTI8:
//    case ISRMgr_EXTI9:
//    case ISRMgr_EXTI10:
//    case ISRMgr_EXTI11:
//    case ISRMgr_EXTI12:
//    case ISRMgr_EXTI13:
//    case ISRMgr_EXTI14:
//    case ISRMgr_EXTI15:
//        retval = subISR_EXTI_assign((uint8_t) ISR_type, subISR_p);
//        break;
    default:
        break;
    }

    return retval;
}

/**
 * @brief subISR_remove. remove a sub ISR function ptr to the sub ISR table of ISR_type
 * @param ISR_t ISR_type
 * @param void (* subISR_p)(void) (not a NULL ptr)
 * @return None.
 */
status_t ISRMgr::subISR_remove(ISR_t ISR_type, void (*subISR_p)(void))
{
    status_t retval = failed;

    if (subISR_p == NULL)
        return failed;

    switch (ISR_type) {
    case ISRMgr_SysTick:
        retval = subISR_SysTick_remove(subISR_p);
        break;
    case ISRMgr_RTC:
        retval = subISR_RTC_remove(subISR_p);
        break;
    case ISRMgr_TIM6:
        retval = subISR_TIM6_remove(subISR_p);
        break;
    case ISRMgr_USART1:
        retval = subISR_USART1_remove(subISR_p);
        break;
    case ISRMgr_USART3:
        retval = subISR_USART3_remove(subISR_p);
        break;
//    case ISRMgr_EXTI0:
//    case ISRMgr_EXTI1:
//    case ISRMgr_EXTI2:
//    case ISRMgr_EXTI3:
//    case ISRMgr_EXTI4:
//    case ISRMgr_EXTI5:
//    case ISRMgr_EXTI6:
//    case ISRMgr_EXTI7:
//    case ISRMgr_EXTI8:
//    case ISRMgr_EXTI9:
//    case ISRMgr_EXTI10:
//    case ISRMgr_EXTI11:
//    case ISRMgr_EXTI12:
//    case ISRMgr_EXTI13:
//    case ISRMgr_EXTI14:
//    case ISRMgr_EXTI15:
//        retval = subISR_EXTI_remove((uint8_t) ISR_type, subISR_p);
//        break;
    default:
        break;
    }

    return retval;
}

/**< SysTick private */

/**
 * @brief subISR_SysTick_assign. assign a sub ISR func ptr to SysTick_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_SysTick_assign(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (SysTick_subISR_table[i] == NULL) {
            found = true;
            SysTick_subISR_table[i] = subISR_p;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief subISR_SysTick_remove. Remove a sub ISR func ptr to SysTick_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_SysTick_remove(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (SysTick_subISR_table[i] == subISR_p) {
            found = true;
            SysTick_subISR_table[i] = NULL;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief SysTick_subISR_table_init. Init all value in SysTick_subISR_table.
 * @return None.
 */
void ISRMgr::SysTick_subISR_table_init(void)
{
    uint8_t a_count;
    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        SysTick_subISR_table[a_count] = NULL;
    }
}
/**< SysTick private */

/**< RTC private */

/**
 * @brief subISR_RTC_assign. assign a sub ISR func ptr to RTC_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_RTC_assign(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (RTC_subISR_table[i] == NULL) {
            found = true;
            RTC_subISR_table[i] = subISR_p;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief subISR_RTC_remove. Remove a sub ISR func ptr to RTC_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_RTC_remove(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (RTC_subISR_table[i] == subISR_p) {
            found = true;
            RTC_subISR_table[i] = NULL;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief RTC_subISR_table_init. Init all value in RTC_subISR_table.
 * @return None.
 */
void ISRMgr::RTC_subISR_table_init(void)
{
    uint8_t a_count;
    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        RTC_subISR_table[a_count] = NULL;
    }
}
/**< SysTick private */

/**< TIM6 private */

/**
 * @brief subISR_TIM6_assign. assign a sub ISR func ptr to TIM6_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_TIM6_assign(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (TIM6_subISR_table[i] == NULL) {
            found = true;
            TIM6_subISR_table[i] = subISR_p;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief subISR_TIM6_remove. Remove a sub ISR func ptr to TIM6_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_TIM6_remove(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (TIM6_subISR_table[i] == subISR_p) {
            found = true;
            TIM6_subISR_table[i] = NULL;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief TIM6_subISR_table_init. Init all value in TIM6_subISR_table.
 * @return None.
 */
void ISRMgr::TIM6_subISR_table_init(void)
{
    uint8_t a_count;
    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        TIM6_subISR_table[a_count] = NULL;
    }
}
/**< TIM6 private */

/**< USART1 private */
/**
 * @brief subISR_USART1_assign. assign a sub ISR func ptr to USART1_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_USART1_assign(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (USART1_subISR_table[i] == NULL) {
            found = true;
            USART1_subISR_table[i] = subISR_p;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief subISR_USART1_remove. Remove a sub ISR func ptr to USART1_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_USART1_remove(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (USART1_subISR_table[i] == subISR_p) {
            found = true;
            USART1_subISR_table[i] = NULL;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief USART1_subISR_table_init. Init all value in USART1_subISR_table.
 * @return None.
 */
void ISRMgr::USART1_subISR_table_init(void)
{
    uint8_t a_count;
    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        USART1_subISR_table[a_count] = NULL;
    }
}
/**< USART1 private */

/**< USART3 private */
/**
 * @brief subISR_USART3_assign. assign a sub ISR func ptr to USART3_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_USART3_assign(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (USART3_subISR_table[i] == NULL) {
            found = true;
            USART3_subISR_table[i] = subISR_p;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief subISR_USART3_remove. Remove a sub ISR func ptr to USART3_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_USART3_remove(void (*subISR_p)(void))
{
    uint8_t i;
    bool found = false;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (USART3_subISR_table[i] == subISR_p) {
            found = true;
            USART3_subISR_table[i] = NULL;

            break;
        }
    }

    if (found)
        return successful;

    return failed;
}

/**
 * @brief USART3_subISR_table_init. Init all value in USART3_subISR_table.
 * @return None.
 */
void ISRMgr::USART3_subISR_table_init(void)
{
    uint8_t a_count;
    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        USART1_subISR_table[a_count] = NULL;
    }
}
/**< USART3 private */

/**< EXTI private */
/**
 * @brief subISR_EXTI_assign. assign a sub ISR func ptr to EXTI_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_EXTI_assign(uint8_t exti_line, callback_t subISR_p,
        void *arg)
{
    uint8_t i;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (EXTI_subISR_table[exti_line][i].callback == NULL) {
            EXTI_subISR_table[exti_line][i].callback = subISR_p;
            EXTI_subISR_table[exti_line][i].arg = arg;

            return successful;
        }
    }

    return failed;
}

/**
 * @brief subISR_EXTI_remove. Remove a sub ISR func ptr to EXTI_subISR_table.
 * @param void (* subISR_p)(void)
 * @return None.
 */
status_t ISRMgr::subISR_EXTI_remove(uint8_t exti_line, callback_t subISR_p)
{
    uint8_t i;

    for (i = 0; i < numOfSubISR_max; i++) {
        if (EXTI_subISR_table[exti_line][i].callback == subISR_p) {
            EXTI_subISR_table[exti_line][i].callback = NULL;

            return successful;
        }
    }

    return failed;
}

/**
 * @brief EXTI_subISR_table_init. Init all value in EXTI_subISR_table.
 * @return None.
 */
void ISRMgr::EXTI_subISR_table_init(void)
{
    uint8_t a_count, line;
    for (line = 0; line < 16; line++) {
        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            EXTI_subISR_table[line][a_count].callback = NULL;
        }
    }
}
/**< EXTI private */

/* ISRs */
/* Changed from Systick_Handler to isr_systick to comply with RIOT */
void isr_systick(void)
{
    uint8_t a_count;

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (SysTick_subISR_table[a_count] != NULL) {
            SysTick_subISR_table[a_count]();
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */

    return;
}

void isr_rtc(void)
{
    uint8_t a_count;

    /* Clear the RTC Second interrupt */
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW | RTC_IT_ALR);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (RTC_subISR_table[a_count] != NULL) {
            RTC_subISR_table[a_count]();
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */

    return;
}

/* Changed from TIM6_IRQHandler to isr_tim6 to comply with RIOT */
void isr_tim6(void)
{
    uint8_t a_count;
    /**< clear IT flag */
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (TIM6_subISR_table[a_count] != NULL) {
            TIM6_subISR_table[a_count]();
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */

    return;
}

void isr_exti0(void)
{
    uint8_t a_count;

    EXTI_ClearITPendingBit(EXTI_Line0);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (EXTI_subISR_table[0][a_count].callback != NULL) {
            EXTI_subISR_table[0][a_count].callback(
                    EXTI_subISR_table[0][a_count].arg);
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti1(void)
{
    uint8_t a_count;

    EXTI_ClearITPendingBit(EXTI_Line1);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (EXTI_subISR_table[1][a_count].callback != NULL) {
            EXTI_subISR_table[1][a_count].callback(
                    EXTI_subISR_table[1][a_count].arg);
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti2(void)
{
    uint8_t a_count;

    EXTI_ClearITPendingBit(EXTI_Line2);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (EXTI_subISR_table[2][a_count].callback != NULL) {
            EXTI_subISR_table[2][a_count].callback(
                    EXTI_subISR_table[2][a_count].arg);
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti3(void)
{
    uint8_t a_count;

    EXTI_ClearITPendingBit(EXTI_Line3);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (EXTI_subISR_table[3][a_count].callback != NULL) {
            EXTI_subISR_table[3][a_count].callback(
                    EXTI_subISR_table[3][a_count].arg);
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti4(void)
{
    uint8_t a_count;

    EXTI_ClearITPendingBit(EXTI_Line4);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (EXTI_subISR_table[4][a_count].callback != NULL) {
            EXTI_subISR_table[4][a_count].callback(
                    EXTI_subISR_table[4][a_count].arg);
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti9_5(void)
{
    uint8_t a_count;

    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line5);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[5][a_count].callback != NULL) {
                EXTI_subISR_table[5][a_count].callback(
                        EXTI_subISR_table[5][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line6) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line6);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[6][a_count].callback != NULL) {
                EXTI_subISR_table[6][a_count].callback(
                        EXTI_subISR_table[6][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line7) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line7);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[7][a_count].callback != NULL) {
                EXTI_subISR_table[7][a_count].callback(
                        EXTI_subISR_table[7][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line8) != RESET) {

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[8][a_count].callback != NULL) {
                EXTI_subISR_table[8][a_count].callback(
                        EXTI_subISR_table[8][a_count].arg);
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line8);

    } else if (EXTI_GetITStatus(EXTI_Line9) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line9);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[9][a_count].callback != NULL) {
                EXTI_subISR_table[9][a_count].callback(
                        EXTI_subISR_table[9][a_count].arg);
            }
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_exti15_10(void)
{
    uint8_t a_count;

    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line10);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[10][a_count].callback != NULL) {
                EXTI_subISR_table[10][a_count].callback(
                        EXTI_subISR_table[10][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line11) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line11);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[11][a_count].callback != NULL) {
                EXTI_subISR_table[11][a_count].callback(
                        EXTI_subISR_table[11][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line12) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line12);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[12][a_count].callback != NULL) {
                EXTI_subISR_table[12][a_count].callback(
                        EXTI_subISR_table[12][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line13) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line13);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[13][a_count].callback != NULL) {
                EXTI_subISR_table[13][a_count].callback(
                        EXTI_subISR_table[13][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line14) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line14);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[14][a_count].callback != NULL) {
                EXTI_subISR_table[14][a_count].callback(
                        EXTI_subISR_table[14][a_count].arg);
            }
        }

    } else if (EXTI_GetITStatus(EXTI_Line15) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line15);

        for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
            if (EXTI_subISR_table[15][a_count].callback != NULL) {
                EXTI_subISR_table[15][a_count].callback(
                        EXTI_subISR_table[15][a_count].arg);
            }
        }

    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */
}

void isr_usart3(void)
{
    uint8_t a_count;

    /**< clear IT flag */
    USART_ClearFlag(USART3, USART_FLAG_RXNE);

    for (a_count = 0; a_count < numOfSubISR_max; a_count++) {
        if (USART3_subISR_table[a_count] != NULL) {
            USART3_subISR_table[a_count]();
        }
    }

    /* RIOT specific code */
    if (sched_context_switch_request) {
        thread_yield();
    }
    /* RIOT specific code */

    return;
}

/*
 void USART1_IRQHandler (void){
 uint8_t a_count;

 for (a_count = 0; a_count < numOfSubISR_max; a_count++){
 if (USART1_subISR_table[a_count] != NULL){
 USART1_subISR_table[a_count] ();
 }
 }
 */
/**< clear IT flag */
/*    USART_ClearITPendingBit  (USART1, USART_IT_RXNE);
 USART_ClearITPendingBit  (USART1, USART_IT_CTS);
 USART_ClearITPendingBit  (USART1, USART_IT_LBD);
 USART_ClearITPendingBit  (USART1, USART_IT_TC);

 return;
 }
 */

