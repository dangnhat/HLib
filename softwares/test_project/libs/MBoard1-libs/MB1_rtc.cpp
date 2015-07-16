/**
 * @file MB1_rtc.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 29-Oct-2014
 * @brief Implementation for MBoard-1's rtc module.
 */

#include <string.h>

#include "MB1_rtc.h"

using namespace rtc_ns;

/*----------------------------------------------------------------------------*/
rtc::rtc(void)
{
    /* init private vars */
    memset((void *)&timebase, 0, sizeof(timebase));
}

/*----------------------------------------------------------------------------*/
void rtc::init(rtc_ns::rtc_params_t &params, rtc_ns::time_t &timebase, bool sec_int)
{
    NVIC_InitTypeDef nvicStruct;

    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Enable access to BKP domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Enable clock source */
    switch (params.clock_src) {
    case RCC_RTCCLKSource_LSE:
        /* Enable LSE clock */
        RCC_LSEConfig(RCC_LSE_ON);

        /* Wait for LSE is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET) {
        }

        /* Config RTC clock source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);

        /* Wait for sync */
        RTC_WaitForSynchro();

        /* Set prescaler value */
        RTC_SetPrescaler(params.prescaler);
        RTC_WaitForLastTask();

        break;
    case RCC_RTCCLKSource_LSI:
        /* TODO: implement LSI when needed */
        while (1);
        break;
    case RCC_RTCCLKSource_HSE_Div128:
        /* TODO: implement HSE/128 when needed */
        while (1);
        break;
    }

    if (sec_int) {
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        RTC_WaitForLastTask();

        nvicStruct.NVIC_IRQChannel = RTC_IRQn;
        nvicStruct.NVIC_IRQChannelCmd = ENABLE;
        nvicStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
        nvicStruct.NVIC_IRQChannelSubPriority = 0x0F;

        NVIC_Init (&nvicStruct);
    }

    memcpy((void *)&this->timebase, (void *)&timebase, sizeof(time_t));
}

/*----------------------------------------------------------------------------*/
void rtc::init(void)
{
    rtc_params_t params;
    time_t timebase;

    params.clock_src = RCC_RTCCLKSource_LSE;
    params.prescaler = 0x7FFF;

    timebase.year = 1980;
    timebase.month = 1;
    timebase.day = 1;
    timebase.dayow = TUE;
    timebase.hour = 0;
    timebase.min = 0;
    timebase.sec = 0;

    init(params, timebase, true);
}

/*----------------------------------------------------------------------------*/
void rtc::get_time(rtc_ns::time_t &time)
{
    uint32_t counter_val;

    /* get raw value of rtc counter */
    counter_val = get_time_raw();

    /* convert raw value to time */
    raw_to_time(counter_val, time);
}

/*----------------------------------------------------------------------------*/
void rtc::set_time(rtc_ns::time_t &time)
{
    uint32_t counter_val;

    /* convert time to raw value */
    counter_val = time_to_raw(time);

    /* set counter value */
    set_time_raw(counter_val);
}

/*----------------------------------------------------------------------------*/
uint32_t rtc::get_time_raw(void)
{
    /* wait for sync (just to be sure rtc has been sync-ed with APB1) */
    RTC_WaitForSynchro();

    return RTC_GetCounter();
}

/*----------------------------------------------------------------------------*/
void rtc::set_time_raw(uint32_t value)
{
    /* Wait for RTOFF = 1 (last write completed) */
    RTC_WaitForLastTask();

    /* Set to config mode */
    RTC_EnterConfigMode();

    /* Set counter */
    RTC_SetCounter(value);

    /* Exit config mode */
    RTC_ExitConfigMode();

    /* Wait for write op is completed */
    RTC_WaitForLastTask();
}

/*----------------------------------------------------------------------------*/
uint32_t rtc::get_time_packed(void)
{
    time_t a_time;

    get_time(a_time);
    return time_to_packed(a_time);
}

/*----------------------------------------------------------------------------*/
void rtc::set_time_packed(uint32_t packed_time)
{
    time_t a_time;

    packed_to_time(packed_time, a_time);
    set_time(a_time);
}

/*------------------- Conversion methods -------------------------------------*/

void rtc::raw_to_time(uint32_t time_raw, rtc_ns::time_t &time)
{
    uint32_t total_sec = time_raw + timebase.sec;
    uint32_t total_min = total_sec / 60;
    uint32_t total_hour = total_min / 60;
    uint32_t total_day = total_hour / 24;

    uint16_t year;
    uint8_t month, day;
    bool inc_month;
    uint8_t numofdays;

    /* Calculate day, hour, min, sec */
    time.sec = total_sec % 60;
    time.min = total_min %  60;
    time.hour = total_hour % 24;

    /* calculate day of weeks */
    time.dayow = (total_day + timebase.dayow) % 7;

    /* Calculate day in month, month, year */
    day = timebase.day;
    month = timebase.month;
    year = timebase.year;

    while (1) {
        inc_month = false;

        /* inc day */
        if (month == 1 || month == 3 || month == 5 || month == 7
                || month == 8 || month == 10 || month == 12) { /* 31-days months */
            numofdays = 31;
        }/* end 31-days months */
        else if (month == 2) {
            if (is_leap_year(year)){ /* leap year */
                numofdays = 29;
            }/* end leap year */
            else {
                numofdays = 28;
            }
        }/* end month 2 */
        else { /* 30-days months */
            numofdays = 30;
        }/* end 30-days months */

        if ((total_day + day) > numofdays) {
            inc_month = true;
            day = 1;
            total_day -= (numofdays - day + 1);
        }
        else {
            day = total_day + day;
            inc_month = false;
        }

        if (inc_month) {
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
        else {
            break;
        }
    }

    time.day = day;
    time.month = month;
    time.year = year;
}

/*----------------------------------------------------------------------------*/
uint32_t rtc::time_to_raw(rtc_ns::time_t &time)
{
    uint32_t raw_value;
    bool borrow_flag;

    uint32_t total_day;
    uint16_t year_count;
    uint8_t month_count;

    /* check if time > timebase */
    if (time.year < timebase.year) {
        return 0;
    }
    else if (time.year == timebase.year) { /* same year */
        if (time.month < timebase.month) {
            return 0;
        }
        else if (time.month == timebase.month) { /* same month */
            if (time.day < timebase.day) {
                return 0;
            }
            else if (time.day == timebase.day) { /* same day */
                if (time.hour < timebase.hour) {
                    return 0;
                }
                else if (time.hour == timebase.hour) { /* same hour */
                    if (time.min < timebase.min) {
                        return 0;
                    }
                    else if (time.min == timebase.min) { /* same min */
                        if (time.sec < timebase.sec) {
                            return 0;
                        }
                    } /* end same min */
                } /* end same hour */
            }/* end same day */
        }/* end same month */
    }/* end same year */

    /* time - timebase */
    raw_value = 0;
    borrow_flag = false;

    /* time.sec - timebase.sec */
    if (time.sec < timebase.sec) {
        borrow_flag = true;
        raw_value += time.sec + 60 - timebase.sec;
    }
    else {
        raw_value += time.sec - timebase.sec;
        borrow_flag = false;
    }

    /* time.min - timbase.min */
    if (time.min < (timebase.min + (borrow_flag ? 1 : 0)) ) {
        raw_value += (time.min + 60 - (timebase.min + (borrow_flag ? 1 : 0))) * 60;
        borrow_flag = true;
    }
    else {
        raw_value += (time.min - (timebase.min + (borrow_flag ? 1 : 0))) * 60;
        borrow_flag = false;
    }

    /* time.hour - timebase.hour */
    if (time.hour < (timebase.hour + (borrow_flag ? 1 : 0))) {
        raw_value += (time.hour + 24 - (timebase.hour + (borrow_flag ? 1 : 0))) * 3600;
        borrow_flag = true;
    }
    else {
        raw_value += (time.hour - (timebase.hour + (borrow_flag ? 1 : 0))) * 3600;
        borrow_flag = false;
    }

    /* time's date - timebase's date */
    total_day = borrow_flag ? 1 : 0;

    month_count = timebase.month;
    year_count = timebase.year;

    while ((month_count != time.month) || (year_count != time.year)) {
        if (month_count == 1 || month_count == 3 || month_count == 5 || month_count == 7
            || month_count == 8 || month_count == 10 || month_count == 12) {
            total_day += 31;
        }
        else if (month_count == 2) {
            if (is_leap_year(year_count)) {
                total_day += 29;
            }
            else {
                total_day += 28;
            }
        }
        else {
            total_day += 30;
        }

        month_count++;
        if (month_count > 12) {
            month_count = 1;
            year_count++;
        }
    }

    total_day = total_day + time.day - timebase.day;

    raw_value += total_day * 24 * 3600;

    return raw_value;
}

/*----------------------------------------------------------------------------*/
void rtc::packed_to_time(uint32_t packed_time, rtc_ns::time_t &time)
{
    time.year = (packed_time >> 25) + timebase.year;
    time.month = (packed_time >> 21) & 0xF;
    time.day = (packed_time >> 16) & 0x1F;
    time.hour = (packed_time >> 11) & 0x1F;
    time.min = (packed_time >> 5) & 0x3F;
    time.sec = (packed_time & 0x1F) * 2;
}

/*----------------------------------------------------------------------------*/
uint32_t rtc::time_to_packed(rtc_ns::time_t &time)
{
    uint32_t packed_time = 0;

    packed_time = ((uint32_t)(time.year - timebase.year)) << 25;
    packed_time = packed_time | (((uint32_t) time.month) << 21);
    packed_time = packed_time | (((uint32_t)time.day) << 16);
    packed_time = packed_time | (((uint32_t)time.hour) << 11);
    packed_time = packed_time | (((uint32_t)time.min) << 5);
    packed_time = packed_time | (uint32_t)(time.sec / 2);

    return packed_time;
}
