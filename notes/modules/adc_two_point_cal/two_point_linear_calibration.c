/******************************************************************************
 * @file two_point_linear_calibration.c
 *
 * @par dependencies
 * - two_point_linear_calibration.h
 *
 * @author Alan 
 *
 * @brief Provide the HAL APIs of the adc value calibration.
 *
 * Processing flow:
 * call directly.
 *
 * @version V1.0        2026-05-28  *
 * @note    
 *       1 tab == 4 spaces!
 *       encoding == UTF-8
 *
 *****************************************************************************/


//******************************** Includes *********************************//
#include "two_point_linear_calibration.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define ADC_VALUE_MIN       0.0f
#define ADC_VALUE_MAX       65535.0f   
#define ADC_CAL_MODE_COUNT  2u
#define ADC_CHANNEL_COUNT   4u
//******************************** Defines **********************************//

typedef struct {
    float       input_value;
    uint16_t    ideal_code;
    uint16_t    measured_code;
} adc_calibration_point_t;

typedef struct {
    adc_calibration_point_t low;
    adc_calibration_point_t high;
} adc_calibration_pair_t;


static adc_calibration_pair_t s_adc_cal_points[ADC_CAL_MODE_COUNT][ADC_CHANNEL_COUNT] = {
    {
        {{-9.98f, 66u, 212u},   {9.98f, 65468u, 65325u}},
        {{-9.98f, 66u, 209u},   {9.98f, 65468u, 65320u}},
        {{-9.98f, 66u, 210u},   {9.98f, 65468u, 65340u}},
        {{-9.98f, 66u, 341u},   {9.98f, 65468u, 65270u}}
    },
    {
		{{-19.98f, 33u, 347u}, 	{19.98f, 65501u, 65198u}},
		{{-19.99f, 16u, 342u},	{20.00f, 65534u, 65194u}},
		{{-19.53f, 770u, 1128u},{19.87f, 65321u, 64956u}},
		{{-19.94f, 98u, 444u}, 	{19.97f, 65485u, 65150u}}
    }
};

static inline uint16_t adc_raw_to_customer_u16(uint16_t raw)
{
    return (uint16_t)(raw ^ 0x8000u);
}

static uint16_t adc_clamp_customer_u16(float code)
{
    if (code <= ADC_VALUE_MIN) {
        return 0u;
    }
    if (code >= ADC_VALUE_MAX) {
        return 65535u;
    }

    return (uint16_t)(code + 0.5f);
}

static uint16_t adc_apply_two_point_linear_calibration(uint16_t customer_code, uint8_t channel, uint8_t cal_mode)
{
    const adc_calibration_pair_t *cal_pair;
    float measured_span;
    float gain;
    float offset;
    float corrected_code;

    if (channel >= ADC_CHANNEL_COUNT){
        return customer_code; 
    }
    if (cal_mode > 1){
        cal_mode = 1;
    }

    cal_pair        = &s_adc_cal_points[cal_mode][channel];
    measured_span   = (float)cal_pair->high.measured_code - (float)cal_pair->low.measured_code;

    if (measured_span == 0.0f){
        return customer_code;
    }
    gain            = ((float)cal_pair->high.ideal_code - (float)cal_pair->low.ideal_code) /  measured_span;
    offset          = (float)cal_pair->low.ideal_code - gain * (float)cal_pair->low.measured_code;
    corrected_code  = gain * (float)customer_code + offset;
    
    return adc_clamp_customer_u16(corrected_code);
}

uint16_t adc_raw_to_twopoint_calibrated_u16(uint16_t raw, uint8_t channel, uint8_t cal_mode)
{
    uint16_t customer_code;

    customer_code = adc_raw_to_customer_u16(raw);
    return adc_apply_two_point_linear_calibration(customer_code, channel, cal_mode);
}

