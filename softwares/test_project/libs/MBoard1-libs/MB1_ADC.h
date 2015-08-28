/**
 * @file MB1_ADC.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 14-10-2014
 * @brief This is header file for ADC conversion for MBoard-1.
 * This lib is partial support for ADC:
 *  _independent ADC.
 *  _continuous mode.
 *  _poll to get converted data.
 * @How to use:
 *  -Declare an adc-class instance.
 *  -Initialize GPIO as analog input for ADC channel.
 *  -Enter value to an "adc_params_t"
 *  -Pass above struct into adc_init method to initialize ADC.
 *  -Call adc_convert method to get converted data.
 *  Note: DMA can only serve ADC1 and ADC3. ADCCLK: 0.6->14MHz.
 */
#ifndef __MB1_ADC_H_
#define __MB1_ADC_H_

#include "MB1_Glb.h"

namespace adc_ns {
typedef enum {
    adc1 = 0,
    adc2 = 1,
    adc3 = 2
} adc_t;

typedef enum {
    independent,
    dual_adc
} adc_mode_t;

typedef enum {
    single_mode,
    continuous_mode,
    discontinuous_mode,
    scan_mode
} adc_conv_mode_t;

typedef enum {
    ext_trigger,
    analog_watchdog,
    no_option
} adc_option_t;

typedef enum {
    dma_request,
    adc_irq,
    poll
} adc_access_t;

typedef enum {
    regular_channel,
    injected_channel
} adc_chn_t;

typedef struct {
    adc_mode_t adc_mode;
    adc_t adc;
    uint8_t adc_channel;
    adc_conv_mode_t conv_mode;
    adc_chn_t channel_type;
    adc_option_t option;
    uint8_t adc_sample_time;
    adc_access_t data_access;
} adc_params_t;
}

class adc {
public:
    /**
     * @brief Constructor of adc class.
     */
    adc(void);

    /**
     * @brief Initialize ADC.
     *
     * @param[in] adc_params A structure contains all user info about ADC.
     */
    void adc_init(const adc_ns::adc_params_t *adc_params);

    /**
     * @brief Start ADC.
     */
    void adc_start(void);

    /**
     * @brief Stop ADC. Shouldn't use if another ADC channel is still running.
     */
    void adc_stop(void);

    /**
     * @brief Get converted data after ADC-ing.
     *
     * @return The converted data.
     */
    uint16_t adc_convert(void);
private:
    bool poll_data;
    uint8_t adc_num;

    /**
     * @brief The private common method used to en/dis an ADC.
     *
     * @param[in] new_state ENABLE or DISABLE.
     */
    void adc_en_dis(FunctionalState new_state);

    void adc_calibration(void);
};

#endif //__MB1_ADC_H_
