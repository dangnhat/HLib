/**
 * @file MB1_rtc.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 28-Oct-2014
 * @brief Header files for internal rtc module of MBoard-1.
 *  - RTC clock source:
 *      + HSE / 128
 *      + LSI
 *      + LSE (32.768 KHz on MBoard-1), should be used when 32.768 KHz and 3V battery
 *      are available. (default)
 *  - Time/Date:
 *      + Prescaler will be set to 7FFF by default to generate 1s period.
 *      + Time/Date base will be 00:00:00 1-Jan-1980, Tue. (default)
 *      + 1 unit of 32-bit RTC counter is 1s (default). Thus, the upper bound for
 *      time/date will be around 6-Feb-2116.
 *  - NOTE: Once clock source for RTC is selected in init, it can't be changed until
 *  BKP domain is reset. (by software (BKP_DeInit) or remove VBAT and VDD).
 */

#ifndef MB1_RTC_H_
#define MB1_RTC_H_

#include "MB1_Glb.h"

namespace rtc_ns {

enum:
uint8_t {
    SUN = 0,
    MON = 1,
    TUE = 2,
    WED = 3,
    THU = 4,
    FRI = 5,
    SAT = 6,
};

typedef struct rtc_params_s {
    uint32_t clock_src; /* RCC_RTCCLKSource_LSE, RCC_RTCCLKSource_LSI or
                        RCC_RTCCLKSource_HSE_Div128 */
    uint32_t prescaler;
} rtc_params_t;

typedef struct time_s {
    uint16_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t min;
    uint8_t sec;

    uint8_t dayow;
} time_t;

}

class rtc {
public:
    rtc(void);

    /**
     * @brief   Init the rtc module with default configurations.
     *
     * @details Default configurations:
     *          + LSE clock source.
     *          + Prescaler load value: 0x7FFF for 32.768 KHz.
     *          + Time period: 1s
     *          + Turn on clock for power and backup interface.
     *          + Enable access to RTC and backup registers. (set BDP bit in PWR_CR)
     *          + Enable second interrupt.
     *          + Time base: 00:00:00 1-Jan-1980, Tue.
     */
    void init(void);

    /**
     * @brief   Init the rtc module with given configurations.
     *
     * @details Configurations:
     *          + clock source: params.clock_src.
     *          + Prescaler load value: params.prescaler.
     *          + Time period: fTR_CLK = fRTCCLK/(PRL[19:0]+1)
     *          + Turn on clock for power and backup interface.
     *          + Enable access to RTC and backup registers. (set BDP bit in PWR_CR)
     *
     * @param[in]   &params, struct which is holding configuration values.
     *              + clock_src, can be RCC_RTCCLKSource_LSE, RCC_RTCCLKSource_LSI or
     *              RCC_RTCCLKSource_HSE_Div128.
     *              + prescaler,
     * @param[in]   &timebase, struct holding the base (year, month, day, day of week,
     *              hour, min, sec) to calculate current time, date.
     *
     * @param[in]   sec_int, true to enable Second interrupt.
     *              Prio will be set to lowest value by default.
     */
    void init(rtc_ns::rtc_params_t &params, rtc_ns::time_t &timebase, bool sec_int);

    /**
     * @brief   Get current time and date.
     *
     * @param[out]  time, struct holding year, month, day, hour, min, sec, dayow.
     */
    void get_time(rtc_ns::time_t &time);

    /**
     * @brief   Set current time and date.
     *
     * @param[in]  time, struct holding year, month (1-12), day (1-28/29/30/31),
     *              hour (0-23), min (0-59), sec (0-59), dayow (0-6, 0 is Sunday, 6 is Saturday).
     */
    void set_time(rtc_ns::time_t &time);

    /**
     * @brief   Get packed time in 32 bit. Packed time format:
     *          bit 0-4: second / 2.
     *          bit 5-10: minute.
     *          bit 11-15: hour.
     *          bit 16-20: day in month.
     *          bit 21-24: month.
     *          bit 25-31: number of years from timebase.year.
     */
    uint32_t get_time_packed(void);

    /**
     * @brief   Set packed time in 32 bit to system time. Packed time format:
     *          bit 0-4: second / 2.
     *          bit 5-10: minute.
     *          bit 11-15: hour.
     *          bit 16-20: day in month.
     *          bit 21-24: month.
     *          bit 25-31: number of years from timebase.year.
     */
    void set_time_packed(uint32_t packed_time);

    /**
     * @brief   Get the raw value of rtc's counter.
     *
     * @return  Raw value.
     */
    uint32_t get_time_raw(void);

    /**
     * @brief   Set the raw value of rtc's counter.
     *
     * @param[in]   value, value of rtc's counter to be set.
     */
    void set_time_raw(uint32_t value);

    /**
     * @brief   Check whether the given year is a leap year or not.
     *
     * @param[in]   year.
     *
     * @return  true or false.
     */
    inline bool is_leap_year(uint16_t year)
    {
        return ((year%4 == 0) ? ((year%100 == 0) ? ((year%400 == 0) ? true : false) : true): false);
    }

    /**
     * @brief   Convert raw rtc counter value to time based on timebase.
     *
     * @param[in]   time_raw, raw value of rtc counter.
     * @param[out]  time, struct holding year, month, day, dayow, hour, min, sec.
     */
    void raw_to_time(uint32_t time_raw, rtc_ns::time_t &time);

    /**
     * @brief   Convert time to raw counter value based on timebase.
     *
     * @param[in]  time, struct holding year, month (1-12), day (1-31), dayow (0-6),
     *             hour (0-59), min (0-59), sec (0-59).
     *
     * @return  raw value for rtc counter (> 0), 0 if error.
     */
    uint32_t time_to_raw(rtc_ns::time_t &time);

    /**
     * @brief   Convert packed time to time_t with timebase (year).
     *
     * @param[in]   packed_time, packed_time format:
     *              bit 0-4: second / 2.
     *              bit 5-10: minute.
     *              bit 11-15: hour.
     *              bit 16-20: day in month.
     *              bit 21-24: month.
     *              bit 25-31: number of years from timebase.year.
     * @param[out]  time, struct holding year, month (1-12), day (1-31), dayow (0-6),
     *             hour (0-59), min (0-59), sec (0-59).
     */
    void packed_to_time(uint32_t packed_time, rtc_ns::time_t &time);

    /**
     * @brief   Convert time_t to packed_time with timebase (year).
     *
     * @param[in]   &time, struct holding year, month (1-12), day (1-31), dayow (0-6),
     *              hour (0-59), min (0-59), sec (0-59).
     *
     * @return      packed_time, packed_time format:
     *              bit 0-4: second / 2.
     *              bit 5-10: minute.
     *              bit 11-15: hour.
     *              bit 16-20: day in month.
     *              bit 21-24: month.
     *              bit 25-31: number of years from timebase.year.
     */
    uint32_t time_to_packed(rtc_ns::time_t &time);

private:
    rtc_ns::time_t timebase;
};




#endif /* MB1_RTC_H_ */
