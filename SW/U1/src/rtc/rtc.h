#ifndef __RTC_H
#define __RTC_H

/* ARM */
#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"

#define RTC_ASYNCH_PREDIV    0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV     0x00FF /* LSE as RTC clock */
#define BACKUP_REGISTER_MN   0X5555 /* Magic number stored in register, which holds RTC configuration */
// Thread flags
// #define WAKE_UP_FLAG_1s      0x01U

// #define WAKE_UP_ALARM_A      0x02U
// #define WAKE_UP_ALARM_B      0x04U
// #define WAKE_UP_ALARM        WAKE_UP_ALARM_A + WAKE_UP_ALARM_B

// #define TIME_SET_WEB        0x08U
// #define DATE_SET_WEB        0x10U

typedef enum {
    FORMATO_24H = 0,
    FORMATO_12H
} EFormatoHora;

typedef enum {
    ENERO      = 0x01U,
    FEBRERO    = 0x02U,
    MARZO      = 0x03U,
    ABRIL      = 0x04U,
    MAYO       = 0x05U,
    JUNIO      = 0x06U,
    JULIO      = 0x07U,
    AGOSTO     = 0x08U,
    SEPTIEMBRE = 0x09U,
    OCTUBRE    = 0x10U,
    NOVIEMBRE  = 0x11U,
    DICIEMBRE  = 0x12U
} EMes;

typedef enum {
    LUNES     = 0x01U,
    MARTES    = 0x02U,
    MIERCOLES = 0x03U,
    JUEVES    = 0x04U,
    VIERNES   = 0x05U,
    SABADO    = 0x06U,
    DOMINGO   = 0x07U
} EDiaSemana;

typedef enum {
    TIME_SET_WEB  = 0x08U,
    DATE_SET_WEB  = 0x10U,
    BUTTON_ACTION = 0x20U
} ERtcFlag;

extern  osThreadId_t     e_rtcThreadId;
//extern  osSemaphoreId_t  e_writeSemaphoreId;

void  RTC_Initialize(void);
void  SetRtcTime(const int hh, const int mm, const int ss, EFormatoHora formato_24_12h);
void  SetRtcDate(const int8_t dd, const EMes mes, const int8_t yyyy, EDiaSemana diaSemana);
void  SetTimerOnce(const int segundos);
const char* GetRtcTime();
const char* GetRtcDate();

#endif
