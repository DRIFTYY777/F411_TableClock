/*
 * rtcmanager.h
 *
 *  Created on: 24-Apr-2026
 *      Author: dhima
 */

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ── Backup Register Map ─────────────────────────────────────── */
#define BKP_REG_MAGIC        RTC_BKP_DR0   // 0xDEADBEEF = RTC valid
#define BKP_REG_CALIB_SIGN   RTC_BKP_DR1   // 0 = positive, 1 = negative
#define BKP_REG_CALIB_VALUE  RTC_BKP_DR2   // calibration magnitude
#define BKP_REG_USER_DATA0   RTC_BKP_DR3   // user non-volatile slot 0
#define BKP_REG_USER_DATA1   RTC_BKP_DR4   // user non-volatile slot 1
#define BKP_REG_USER_DATA2   RTC_BKP_DR5   // user non-volatile slot 2
#define BKP_REG_RESET_COUNT  RTC_BKP_DR6   // counts power cycles

#define RTC_MAGIC_VALUE      0xDEADBEEF

/* ── Calibration limits (STM32F4 RM0383) ────────────────────── */
// CALM: 0..511 (adds pulses, slows clock = positive correction)
// CALP: adds 512 pulses per 32s window (speeds clock)
// Net range: -487.1 ppm to +488.5 ppm

typedef struct {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
} DateTime_t;

typedef struct {
    int32_t  ppm;        // desired correction in ppm*100 (e.g. 150 = +1.50 ppm)
    bool     is_fast;    // true = RTC runs fast (we need to slow it)
} CalibConfig_t;

/* ── Public API ──────────────────────────────────────────────── */
bool     RTC_Manager_Init(RTC_HandleTypeDef *hrtc);
bool     RTC_IsTimeValid(RTC_HandleTypeDef *hrtc);
void     RTC_SetDateTime(RTC_HandleTypeDef *hrtc, DateTime_t *dt);
void     RTC_GetDateTime(RTC_HandleTypeDef *hrtc, DateTime_t *dt);
void     RTC_PrintDateTime(RTC_HandleTypeDef *hrtc);

/* Calibration */
void     RTC_ApplyCalibration(RTC_HandleTypeDef *hrtc, int32_t ppm_x100);
int32_t  RTC_GetStoredCalibration(RTC_HandleTypeDef *hrtc);

/* Backup registers (survive power loss via VBAT) */
void     RTC_BkpWrite(RTC_HandleTypeDef *hrtc, uint32_t reg, uint32_t value);
uint32_t RTC_BkpRead(RTC_HandleTypeDef *hrtc, uint32_t reg);

/* Reset counter */
uint32_t RTC_GetResetCount(RTC_HandleTypeDef *hrtc);

#endif /* RTC_MANAGER_H */
