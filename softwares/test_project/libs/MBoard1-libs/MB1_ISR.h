/**
 * @file MB1_ISR.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 21-10-2013
 * @brief This is header file for interrupt handlers for MBoard-1.
 *
 */

#ifndef __MB1_ISR_H_
#define __MB1_ISR_H_

/* Includes */
#include "MB1_Glb.h"
#include "MB1_Misc.h"

typedef void (*callback_t)(void *arg);

namespace ISRMgr_ns {

const uint8_t numOfSubISR_max = 8;

typedef enum {
    successful,
    failed
} status_t;

typedef enum {
    ISRMgr_EXTI0 = 0,
    ISRMgr_EXTI1 = 1,
    ISRMgr_EXTI2 = 2,
    ISRMgr_EXTI3 = 3,
    ISRMgr_EXTI4 = 4,
    ISRMgr_EXTI5 = 5,
    ISRMgr_EXTI6 = 6,
    ISRMgr_EXTI7 = 7,
    ISRMgr_EXTI8 = 8,
    ISRMgr_EXTI9 = 9,
    ISRMgr_EXTI10 = 10,
    ISRMgr_EXTI11 = 11,
    ISRMgr_EXTI12 = 12,
    ISRMgr_EXTI13 = 13,
    ISRMgr_EXTI14 = 14,
    ISRMgr_EXTI15 = 15,
    ISRMgr_SysTick,
    ISRMgr_RTC,
    ISRMgr_TIM6,
    ISRMgr_USART1,
    ISRMgr_USART2,
    ISRMgr_USART3
} ISR_t;

}

class ISRMgr {
public:
    ISRMgr (void);
    ISRMgr_ns::status_t subISR_assign (ISRMgr_ns::ISR_t ISR_type, void (* subISR_p)(void) );
    ISRMgr_ns::status_t subISR_remove (ISRMgr_ns::ISR_t ISR_type, void (* subISR_p)(void) );

private:
    /**< SysTick */
    void SysTick_subISR_table_init (void);
    ISRMgr_ns::status_t subISR_SysTick_assign (void (* subISR_p)(void) );
    ISRMgr_ns::status_t subISR_SysTick_remove (void (* subISR_p)(void) );
    /**< SysTick */

    /**< SysTick */
    void RTC_subISR_table_init (void);
    ISRMgr_ns::status_t subISR_RTC_assign (void (* subISR_p)(void) );
    ISRMgr_ns::status_t subISR_RTC_remove (void (* subISR_p)(void) );
    /**< SysTick */

    /**< TIM6 */
    void TIM6_subISR_table_init (void);
    ISRMgr_ns::status_t subISR_TIM6_assign ( void (* subISR_p) (void) );
    ISRMgr_ns::status_t subISR_TIM6_remove ( void (* subISR_p) (void) );
    /**< TIM6 */

    /**< USART1 */
    void USART1_subISR_table_init (void);
    ISRMgr_ns::status_t subISR_USART1_assign ( void (* subISR_p) (void) );
    ISRMgr_ns::status_t subISR_USART1_remove ( void (* subISR_p) (void) );
    /**< USART1 */

    /**< USART2 */
    void USART2_subISR_table_init (void);
    ISRMgr_ns::status_t subISR_USART2_assign ( void (* subISR_p) (void) );
    ISRMgr_ns::status_t subISR_USART2_remove ( void (* subISR_p) (void) );
    /**< USART2 */

    /**< USART3 */
	void USART3_subISR_table_init (void);
	ISRMgr_ns::status_t subISR_USART3_assign ( void (* subISR_p) (void) );
	ISRMgr_ns::status_t subISR_USART3_remove ( void (* subISR_p) (void) );
	/**< USART3 */

    /**< EXTI */
    void EXTI_subISR_table_init(void);
public:
    ISRMgr_ns::status_t subISR_EXTI_assign(uint8_t exti_line, callback_t subISR_p, void *arg);
    ISRMgr_ns::status_t subISR_EXTI_remove(uint8_t exti_line, callback_t subISR_p);
    /**< EXTI */
};

#ifdef __cplusplus
extern "C" {
#endif

/* ISRs */
void isr_tim6(void);
void isr_exti0(void);
void isr_exti1(void);
void isr_exti2(void);
void isr_exti3(void);
void isr_exti4(void);
void isr_exti9_5(void);
void isr_exti15_10(void);
void isr_usart3(void);
void isr_usart2(void);
void isr_rtc(void);
#ifdef __cplusplus
}
#endif

#endif // __MB1_IRQ_H_
