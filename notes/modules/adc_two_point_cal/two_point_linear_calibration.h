/******************************************************************************
 * @file two_point_linear_calibration.h
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
#ifndef __TWO_POINT_LINEAR_CALIBRATION_H__
#define __TWO_POINT_LINEAR_CALIBRATION_H__
//******************************** Includes *********************************//
#include "stdint.h"
//******************************** Includes *********************************//

uint16_t adc_raw_to_twopoint_calibrated_u16(uint16_t raw, uint8_t channel, uint8_t cal_mode);

#endif

