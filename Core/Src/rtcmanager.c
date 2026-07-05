/*
 * rtcmanager.c
 *
 *  Created on: 24-Apr-2026
 *      Author: dhima
 */

#include "rtcmanager.h"
#include <stdio.h>

/*
   INIT
   Returns true  = RTC was already valid (kept time through power loss)
   Returns false = RTC was reset, needs time to be set
*/
bool RTC_Manager_Init(RTC_HandleTypeDef *hrtc)
{
    /* Enable backup domain access — MUST be called before touching
       RTC or backup registers. PWR clock must already be enabled.   */
    HAL_PWR_EnableBkUpAccess();

    bool was_valid = RTC_IsTimeValid(hrtc);

    if (!was_valid) {
        /* First boot or VBAT was lost — stamp magic and reset counter */
        RTC_BkpWrite(hrtc, BKP_REG_MAGIC, RTC_MAGIC_VALUE);
        RTC_BkpWrite(hrtc, BKP_REG_RESET_COUNT, 1);
        RTC_BkpWrite(hrtc, BKP_REG_CALIB_SIGN,  0);
        RTC_BkpWrite(hrtc, BKP_REG_CALIB_VALUE, 0);

        /* Set a default compile-time date/time */
        DateTime_t dt = {0};
        dt.time.Hours   = 0;
        dt.time.Minutes = 0;
        dt.time.Seconds = 0;
        dt.date.Year    = 25;   // 2025
        dt.date.Month   = RTC_MONTH_JANUARY;
        dt.date.Date    = 1;
        dt.date.WeekDay = RTC_WEEKDAY_WEDNESDAY;
        RTC_SetDateTime(hrtc, &dt);

        printf("[RTC] First boot – default time set\r\n");
    } else {
        /* Increment reset/power-cycle counter */
        uint32_t cnt = RTC_BkpRead(hrtc, BKP_REG_RESET_COUNT);
        RTC_BkpWrite(hrtc, BKP_REG_RESET_COUNT, cnt + 1);

        /* Re-apply stored calibration (survives reset but hardware
           calibration register resets on full power loss)            */
        int32_t stored_ppm = RTC_GetStoredCalibration(hrtc);
        if (stored_ppm != 0) {
            RTC_ApplyCalibration(hrtc, stored_ppm);
        }
        printf("[RTC] Valid – time retained. Boot #%lu\r\n",
               (unsigned long)(RTC_BkpRead(hrtc, BKP_REG_RESET_COUNT)));
    }

    return was_valid;
}

/*
   Check if RTC has valid time (magic cookie in backup register)
*/
bool RTC_IsTimeValid(RTC_HandleTypeDef *hrtc)
{
    return (HAL_RTCEx_BKUPRead(hrtc, BKP_REG_MAGIC) == RTC_MAGIC_VALUE);
}

/*
   SET date/time
*/
void RTC_SetDateTime(RTC_HandleTypeDef *hrtc, DateTime_t *dt)
{
    dt->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    dt->time.StoreOperation  = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(hrtc, &dt->time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(hrtc, &dt->date, RTC_FORMAT_BIN);

    /* Re-stamp magic in case this is called after a time-sync */
    RTC_BkpWrite(hrtc, BKP_REG_MAGIC, RTC_MAGIC_VALUE);
}

/*
   GET date/time
*/
void RTC_GetDateTime(RTC_HandleTypeDef *hrtc, DateTime_t *dt)
{
    /* IMPORTANT: Read TIME first, then DATE — this is required by
       STM32 hardware to unlock the shadow register correctly.       */
    HAL_RTC_GetTime(hrtc, &dt->time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &dt->date, RTC_FORMAT_BIN);
}

/*
   PRINT date/time over UART (assumes printf retargeted to USART)
*/
void RTC_PrintDateTime(RTC_HandleTypeDef *hrtc)
{
    DateTime_t dt;
    RTC_GetDateTime(hrtc, &dt);

    printf("20%02d-%02d-%02d  %02d:%02d:%02d  SubSec=%lu\r\n",
           dt.date.Year,
           dt.date.Month,
           dt.date.Date,
           dt.time.Hours,
           dt.time.Minutes,
           dt.time.Seconds,
           (unsigned long)dt.time.SubSeconds);
}

/*
   CALIBRATION
   ppm_x100: correction in units of 0.01 ppm
     positive = RTC is running SLOW  → add pulses (CALP)
     negative = RTC is running FAST  → remove pulses (CALM)

   STM32F4 smooth calibration:
     CALP=1, CALM=0   → +488.5 ppm  (max speed-up)
     CALP=0, CALM=511 → -487.1 ppm  (max slow-down)
     Resolution: ~0.954 ppm per CALM step

   Formula:
     If fast (ppm < 0):  CALM = |ppm| / 0.954,  CALP = 0
     If slow (ppm > 0):  CALM = (488.5 - ppm) / 0.954, CALP = 1
*/
void RTC_ApplyCalibration(RTC_HandleTypeDef *hrtc, int32_t ppm_x100)
{
    uint32_t calp, calm;
    int32_t  ppm = ppm_x100;   // working in 0.01 ppm units

    /* Resolution = 0.954 ppm = 95 in our 0.01ppm units */
    const int32_t RESOLUTION = 95;
    const int32_t MAX_CALP_PPM = 48850; /* +488.50 ppm * 100 */

    if (ppm >= 0) {
        /* RTC is slow → speed it up: CALP=1, reduce CALM */
        calp = RTC_SMOOTHCALIB_PLUSPULSES_SET;
        int32_t net = MAX_CALP_PPM - ppm;
        if (net < 0) net = 0;
        calm = (uint32_t)(net / RESOLUTION);
        if (calm > 511) calm = 511;
    } else {
        /* RTC is fast → slow it down: CALP=0, increase CALM */
        calp = RTC_SMOOTHCALIB_PLUSPULSES_RESET;
        calm = (uint32_t)((-ppm) / RESOLUTION);
        if (calm > 511) calm = 511;
    }

    HAL_RTCEx_SetSmoothCalib(hrtc,
        RTC_SMOOTHCALIB_PERIOD_32SEC,
        calp,
        calm);

    /* Persist to backup registers so it survives full power-off */
    uint32_t sign  = (ppm_x100 < 0) ? 1u : 0u;
    uint32_t value = (uint32_t)(ppm_x100 < 0 ? -ppm_x100 : ppm_x100);
    RTC_BkpWrite(hrtc, BKP_REG_CALIB_SIGN,  sign);
    RTC_BkpWrite(hrtc, BKP_REG_CALIB_VALUE, value);

    printf("[RTC] Calibration applied: %s%ld.%02ld ppm  "
           "CALP=%lu CALM=%lu\r\n",
           (ppm_x100 < 0) ? "-" : "+",
           (long)( (ppm_x100 < 0 ? -ppm_x100 : ppm_x100) / 100 ),
           (long)( (ppm_x100 < 0 ? -ppm_x100 : ppm_x100) % 100 ),
           (unsigned long)calp,
           (unsigned long)calm);
}

/* Read calibration from backup registers */
int32_t RTC_GetStoredCalibration(RTC_HandleTypeDef *hrtc)
{
    uint32_t sign  = RTC_BkpRead(hrtc, BKP_REG_CALIB_SIGN);
    uint32_t value = RTC_BkpRead(hrtc, BKP_REG_CALIB_VALUE);
    int32_t  ppm   = (int32_t)value;
    return (sign == 1) ? -ppm : ppm;
}

/*
   BACKUP REGISTER helpers (20 x 32-bit, survive VBAT)
*/
void RTC_BkpWrite(RTC_HandleTypeDef *hrtc, uint32_t reg, uint32_t value)
{
    HAL_RTCEx_BKUPWrite(hrtc, reg, value);
}

uint32_t RTC_BkpRead(RTC_HandleTypeDef *hrtc, uint32_t reg)
{
    return HAL_RTCEx_BKUPRead(hrtc, reg);
}

uint32_t RTC_GetResetCount(RTC_HandleTypeDef *hrtc)
{
    return RTC_BkpRead(hrtc, BKP_REG_RESET_COUNT);
}
