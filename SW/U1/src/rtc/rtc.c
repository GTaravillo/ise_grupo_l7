#include "rtc.h"
/* ARM */
#include "rl_net.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED
#include PATH_IRQ
#include PATH_LCD

/* Public */
osThreadId_t     e_rtcThreadId;
// osSemaphoreId_t  e_writeSemaphoreId;
		
void  RTC_Initialize(void);
void  SetRtcTime(const int hh, const int mm, const int ss, EFormatoHora formato_24_12h);
void  SetRtcDate(const int8_t dd, const EMes mes, const int8_t yy, EDiaSemana diaSemana);
void  RemoveTimer(uint32_t alarm);
void  SetTimerOnce(const int segundos);
/* Override */
				//void  RTC_Alarm_IRQHandler();
/* Private */
typedef enum {
    WAKE_UP_1s      = 0x01U,
    WAKE_UP_ALARM_A = 0x02U,
    WAKE_UP_ALARM_B = 0x04U,
    TIME_SET_WEB_P  = TIME_SET_WEB,
    DATE_SET_WEB_P  = DATE_SET_WEB,
    ANY             = 0x1F
} ERtcPrivateFlag;

static  osStatus_t        g_threadStatus;
static  RTC_HandleTypeDef g_rtcHandle;   // RTC handler declaration
static	RTC_TimeTypeDef   g_timeConfig;
static  RTC_DateTypeDef   g_dateConfig;
static  RTC_AlarmTypeDef  g_alarmConfig;
static  char              g_time[9];     // Time buffer (HH:MM:SS)
static  char              g_date[11];    // Date buffer (DD/MM/AAAA)
static  bool              g_waitForSntp;
static  bool              g_dstOn = true;
static  int               g_utcOffset = +1;

static  void  Run(void *argument);
static  void  RTC_Config(void);
static  void  WakeUpConfig(void);

static  bool  HasRTCLostPower(void);

static  void  UpdateTime(void);
static  void  UpdateDate(void);
static  void  PrintTimeLCD(void);
static  void  PrintDateLCD(void);

static  void  GetAlarmTime(int *hours, int *min, int *sec, int secToAdd);
static  bool  RequestSntpTime(const char* serverIp);
static  void  SntpCB(uint32_t seconds, uint32_t fractionSeconds);
static  void  EventCB(ERtcPrivateFlag event);
static  void  CheckFlags(void);
static  void  InitUserButton(void);
static  void  Error_Handler(void);

/**************************************/

void RTC_Initialize(void)
{
	osMessageQueueAttr_t inputMessageAttr = {
		.name = "RTC INPUT"
	};
  e_rtcThreadId      = osThreadNew(Run, NULL, NULL);
  // e_writeSemaphoreId = osSemaphoreNew(1, 1, NULL);

  if ((e_rtcThreadId == NULL) /*|| (e_writeSemaphoreId == NULL)*/) 
  {
    printf("[rtc::%s] ERROR! osThreadNew [%d]\n", __func__, 
                                                       (e_rtcThreadId == NULL)/*, 
                                                 (e_writeSemaphoreId == NULL)*/);
    
    Error_Handler();
  }
}

void SetRtcTime(const int hh, const int mm, const int ss, EFormatoHora formato_24_12h)
{
  // osSemaphoreAcquire(e_writeSemaphoreId, 0);
  ////printf("[rtc::%s] Want to set time to [%02d:%02d:%02d] DST [%s] UTC+%d\n", __func__, hh, mm, ss, g_dstOn ? "ON" : "OFF", g_utcOffset);

  g_timeConfig.Hours          = (uint8_t)hh;
  g_timeConfig.Minutes        = (uint8_t)mm;
  g_timeConfig.Seconds        = (uint8_t)ss;
  g_timeConfig.TimeFormat     = (formato_24_12h == FORMATO_24H) ? RTC_HOURFORMAT_24
                                                                : RTC_HOURFORMAT_12;
  g_timeConfig.DayLightSaving = g_dstOn ? RTC_DAYLIGHTSAVING_ADD1H 
                                        : RTC_DAYLIGHTSAVING_NONE;
  g_timeConfig.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&g_rtcHandle, &g_timeConfig, RTC_FORMAT_BIN) != HAL_OK)
  {
    ////printf("[rtc::%s] HAL_RTC_SetTime ERROR!\n", __func__);
    
    Error_Handler();
  }

  //printf("[rtc::%s] Time set to [%02d:%02d:%02d]\n", __func__, hh, mm, ss);
  

  // osSemaphoreRelease(e_writeSemaphoreId);
}

void SetSntpTime(const int hh, const int mm, const int ss, EFormatoHora formato_24_12h)
{
  // osSemaphoreAcquire(e_writeSemaphoreId, 0);

  g_timeConfig.Hours          = (uint8_t)hh;
  g_timeConfig.Minutes        = (uint8_t)mm;
  g_timeConfig.Seconds        = (uint8_t)ss;
  g_timeConfig.TimeFormat     = (formato_24_12h == FORMATO_24H) ? RTC_HOURFORMAT_24
                                                                : RTC_HOURFORMAT_12;
  g_timeConfig.DayLightSaving = g_dstOn ? RTC_DAYLIGHTSAVING_ADD1H 
                                        : RTC_DAYLIGHTSAVING_NONE;
  g_timeConfig.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&g_rtcHandle, &g_timeConfig, RTC_FORMAT_BIN) != HAL_OK)
  {
    //printf("[rtc::%s] HAL_RTC_SetTime ERROR!\n", __func__);
    
    Error_Handler();
  }

  //printf("[rtc::%s] Time set to [%02d:%02d:%02d]\n", __func__, hh, mm, ss);
  

  // osSemaphoreRelease(e_writeSemaphoreId);
}

void SetRtcDate(const int8_t dd, const EMes mes, const int8_t yy, EDiaSemana diaSemana)
{
  //osSemaphoreAcquire(e_writeSemaphoreId, 0);
  g_dateConfig.Date    = (uint8_t)dd;
  g_dateConfig.Month   = (uint8_t)mes;
  g_dateConfig.Year    = (uint8_t)yy;
  g_dateConfig.WeekDay = diaSemana;

  if(HAL_RTC_SetDate(&g_rtcHandle, &g_dateConfig, RTC_FORMAT_BIN) != HAL_OK)
  {
    //printf("[rtc::%s] HAL_RTC_SetDate ERROR!\n", __func__);
    
    Error_Handler();
  }

  //printf("[rtc::%s] Date set to [%02d/%02d/%02d]\n", __func__, dd, mes, yy);
  
	
  //osSemaphoreRelease(e_writeSemaphoreId);
}

void RemoveTimer(uint32_t alarm)
{
  RTC_AlarmTypeDef alarmConfig;
  if ((alarm != RTC_ALARM_A) && (alarm != RTC_ALARM_B))
  {
    //printf("[rtc::%s] Specified alarm [%d] isn't RTC_ALARM_A nor RTC_ALARM_B\n", __func__, alarm);
    return;
  }
	
	__HAL_RTC_ALARM_CLEAR_FLAG(&g_rtcHandle, (alarm == RTC_ALARM_A) ? RTC_FLAG_ALRAF : RTC_FLAG_ALRBF);
	HAL_RTC_DeactivateAlarm(&g_rtcHandle, alarm);
	
  //printf("[rtc::%s] Alarm [%c] removed\n", __func__, (alarm == RTC_ALARM_A) ? 'A' : 'B');
}

void SetTimerOnce(const int segundos) 
{
  HAL_StatusTypeDef status;
  // char timeBuffer[9];
  int hh, mm, ss;
  

  const bool alarmAisActive = g_rtcHandle.Instance->CR & RTC_CR_ALRAE;
  const bool alarmBisActive = g_rtcHandle.Instance->CR & RTC_CR_ALRBE;

  if (alarmAisActive && alarmBisActive) 
  {
    //printf("[rtc::%s] No alarms left\n", __func__);
    
    return;
  }
	
  if (!alarmAisActive)
  {
    //printf("[rtc::%s] Setting alarm A...\n", __func__);
    
    g_alarmConfig.Alarm = RTC_ALARM_A;
  }
  else
  {
    //printf("[rtc::%s] Setting alarm B...\n", __func__);
    
    g_alarmConfig.Alarm = RTC_ALARM_B;
  }
	
  // Turns specified seconds into appropiate time for alarm
  GetAlarmTime(&hh, &mm, &ss, segundos);

	//printf("[%02d:%02d:%02d]\n", hh, mm, ss);
	
	hh = RTC_ByteToBcd2(hh);
	mm = RTC_ByteToBcd2(mm);
	ss = RTC_ByteToBcd2(ss);

  //printf("[%02X:%02X:%02X]\n", hh, mm, ss);
	
  g_alarmConfig.AlarmTime.Hours    = hh;
  g_alarmConfig.AlarmTime.Minutes  = mm;
  g_alarmConfig.AlarmTime.Seconds  = ss;
  g_alarmConfig.AlarmMask          = RTC_ALARMMASK_DATEWEEKDAY;
  g_alarmConfig.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	
  // s//printf((char *)timeBuffer, "%02d:%02d:%02d", alarmConfig.AlarmTime.Hours, 
  //                                               alarmConfig.AlarmTime.Minutes, 
  //                                               alarmConfig.AlarmTime.Seconds);

  // //printf("[rtc::%s] Alarm [%c] set to [%s]\n", __func__, (alarmConfig.Alarm == RTC_ALARM_A) ? 'A' : 'B', timeBuffer);
  
	//printf("[%02X:%02X:%02X]\n", g_alarmConfig.AlarmTime.Hours, g_alarmConfig.AlarmTime.Minutes, g_alarmConfig.AlarmTime.Seconds);


  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  // Set Alarm
  status = HAL_RTC_SetAlarm_IT(&g_rtcHandle, &g_alarmConfig, RTC_FORMAT_BCD);
	//printf("[rtc::%s] HAL_RTC_SetAlarm_IT [%d]\n", __func__, status);
  if (status != HAL_OK)
  {
    //printf("[rtc::%s] HAL_RTC_SetAlarm_IT ERROR!\n", __func__);
    
    Error_Handler();
  }
}

static void Run(void *argument) 
{
	uint32_t flag = 0;
	bool waitForSntp;
	RTC_Config();
	CheckFlags();
  
	if (!RequestSntpTime("193.147.107.33") && HasRTCLostPower())
	{
		// Hora y fecha por defecto
		SetRtcTime(00, 00, 00, FORMATO_24H);
		SetRtcDate(01, ENERO, 99, VIERNES);
	}
	
	while (g_waitForSntp)
	{
		//printf("[rtc::%s] Waiting for SNTP server to respond...\n", __func__);
		osDelay(100);
	}
	
	WakeUpConfig();
	UpdateTime();
	UpdateDate();
	RemoveTimer(RTC_ALARM_A);
	RemoveTimer(RTC_ALARM_B);
	// SetTimerOnce(10);
	InitUserButton();
	while(1)
	{
		flag = osThreadFlagsWait(ANY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
    //printf("[rtc::%s] received [%d]\n", __func__, flag);
		ERtcPrivateFlag event = (ERtcPrivateFlag)flag;
		EventCB(event);
	}
}

static void RTC_Config(void) 
{
  HAL_StatusTypeDef   halStatus;

  g_rtcHandle.Instance            = RTC; 
  g_rtcHandle.Init.HourFormat     = RTC_HOURFORMAT_24;
  g_rtcHandle.Init.AsynchPrediv   = RTC_ASYNCH_PREDIV;
  g_rtcHandle.Init.SynchPrediv    = RTC_SYNCH_PREDIV;
  g_rtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
  g_rtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  g_rtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

  __HAL_RTC_RESET_HANDLE_STATE(&g_rtcHandle);
  halStatus = HAL_RTC_Init(&g_rtcHandle);

  if (halStatus != HAL_OK)
  {
    //printf("[rtc::%s] HAL_RTC_Init ERROR!\n", __func__);
    
    Error_Handler();
  }

  /* Turn LED1 on */
  ledMessage_t ledMsg = {
    .mode        = LED_ON,
    .ledsOn.leds = LD1
  };

  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
}

static void WakeUpConfig(void)
{
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  // Configura wakeup timer cada 1s
  HAL_RTCEx_SetWakeUpTimer_IT(&g_rtcHandle, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);  // ck_spre = 1Hz
}

static bool HasRTCLostPower(void)
{
  if (HAL_RTCEx_BKUPRead(&g_rtcHandle, RTC_BKP_DR1) == BACKUP_REGISTER_MN)	// re-initialize the clock
  {
    // Hora guardada
    return false;
  }
  // Hora no guardada. Cargar datos por defecto

  /* Save Configuration in backup register */
  HAL_RTCEx_BKUPWrite(&g_rtcHandle, RTC_BKP_DR1, BACKUP_REGISTER_MN);
  return true;
}

static void GetAlarmTime(int *hours, int *min, int *sec, int secToAdd) 
{
	// Obtener hora actual en segundos
	RTC_TimeTypeDef rtcTime;
	HAL_RTC_GetTime(&g_rtcHandle, &rtcTime, RTC_FORMAT_BIN);
	
	int actualTime = (rtcTime.Hours * 3600) + (rtcTime.Minutes * 60) + (rtcTime.Seconds);
	// Añadir tiempo de espera (24h == 86400s)
	int totalTimeInSec = (actualTime + secToAdd) % 86400;
	// Calcular tiempo de alarma
	*hours = totalTimeInSec / 3600;
	*min   = (totalTimeInSec % 3600) / 60;
	*sec   = totalTimeInSec % 60;
}

static void UpdateTime(void)
{
  RTC_TimeTypeDef rtcTime;

  HAL_RTC_GetTime(&g_rtcHandle, &rtcTime, RTC_FORMAT_BIN);
  g_timeConfig.Hours   = rtcTime.Hours;
  g_timeConfig.Minutes = rtcTime.Minutes;
  g_timeConfig.Seconds = rtcTime.Seconds;

  sprintf((char *)g_time, "%02d:%02d:%02d", g_timeConfig.Hours, g_timeConfig.Minutes, g_timeConfig.Seconds);
  //printf("[rtc::%s] Time for LCD set to [%s]\n", __func__, g_time);
  
}

static void UpdateDate(void)
{
	RTC_DateTypeDef rtcDate;

 	HAL_RTC_GetDate(&g_rtcHandle, &rtcDate, RTC_FORMAT_BIN);
	g_dateConfig.Date   = rtcDate.Date;
	g_dateConfig.Month  = rtcDate.Month;
	g_dateConfig.Year   = rtcDate.Year;
	
  sprintf((char *)g_date, "%02d/%02d/%4d", g_dateConfig.Date, g_dateConfig.Month, (2000 + g_dateConfig.Year));
}

static void PrintTimeLCD(void)
{
	osStatus_t osStatus;
	lcdMessage_t lcdMsg;
	printableMsg_t printMsg;
	
  lcdMsg.mode        = PRINT_NORMAL;
	printMsg.printLine = PRINT_LINE_1;
	strcpy(printMsg.msg, g_time);
  lcdMsg.printMsg = printMsg;
	
	osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
}

static void PrintDateLCD(void)
{
	osStatus_t osStatus;
	lcdMessage_t lcdMsg;
	printableMsg_t printMsg;
	
  lcdMsg.mode        = PRINT_NORMAL;
	printMsg.printLine = PRINT_LINE_2;
	strcpy(printMsg.msg, g_date);
  lcdMsg.printMsg = printMsg;
	
	osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
}

const char* GetRtcTime()
{
  return g_time;
}

const char* GetRtcDate()
{
  return g_date;
}

static bool RequestSntpTime(const char* serverIp)
{
	netStatus status;
	uint8_t ip[NET_ADDR_IP4_LEN];
	bool correct = false;
	
	correct = netIP_aton(serverIp, NET_ADDR_IP4, &ip[0]);
	//printf("[rtc::%s] IP [%s]\n", __func__, serverIp);
	
	if (!correct)
	{
		//printf("[rtc::%s] netIP_aton ERROR!\n", __func__);
		return correct;
	}
	
	for (int i = 0; i < NET_ADDR_IP4_LEN; i++)
	{
		//printf("ip[%d] = %d ", i, ip[i]);
	}
	//printf("\n");
	
  NET_ADDR ntpServer = { 
    .addr = { ip[0], ip[1], ip[2], ip[3] },
		.addr_type = NET_ADDR_IP4
  };
	
	while (!netHTTPs_Running())
	{
		//printf("[rtc::%s] Waiting for HTTP server to start...\n", __func__);
		osDelay(10);
	}
	
  osDelay(5000);
	
	status = netSNTPc_GetTime(&ntpServer, SntpCB);
	
	if (status == osOK)
	{
		correct = true;
	}

  g_waitForSntp = true;
	return correct;
}

void SntpCB(uint32_t seconds, uint32_t fractionSeconds)
{
  struct tm localTime = *localtime((time_t*)&seconds);
  //printf("[rtc::%s] seconds [%d]\n", __func__, seconds);
	
	if (seconds == 0)
	{
		//printf("[rtc::%s] No connection with SNTP server!\n", __func__);
		g_waitForSntp = false;
		return;
	}

  int day     = localTime.tm_mday;
	int month   = 1 + localTime.tm_mon;
	int year    = localTime.tm_year - 100;
	int weekDay = localTime.tm_wday;
	
  int hour = localTime.tm_hour + g_utcOffset;
	int min  = localTime.tm_min;
	int sec  = localTime.tm_sec;
	
	//printf("[rtc::%s] day[%d] month[%d] year[%d] wday[%d] hh[%d] mm[%d] ss[%d]\n", __func__, day, month, year, weekDay, hour, min, sec);
	
	weekDay = (weekDay == 0) ? DOMINGO : weekDay;
	
	SetRtcDate(day, month, year, weekDay);
	SetRtcTime(hour, min, sec, FORMATO_24H);
	
	g_waitForSntp = false;
	
//	ledMessage_t ledMsg = 
//	{
//    .ledSequence = { LD1 |  }
//	};
}

static void EventCB(ERtcPrivateFlag event)
{
  //printf("[rtc::%s] event [%d]\n", __func__, event);

  bool wakeUpEv = event & WAKE_UP_1s;
  bool alarmAEv = event & WAKE_UP_ALARM_A;
  bool buttonEv = event & BUTTON_ACTION;

  if (wakeUpEv)
  {
    osThreadFlagsClear(WAKE_UP_1s);
    UpdateTime();
    UpdateDate();
    //PrintTimeLCD();
    //PrintDateLCD();
  }

  if (alarmAEv)
  {
    osThreadFlagsClear(WAKE_UP_ALARM_A);
    ledMessage_t ledMsg = 
    {
      .mode            = LED_TOGGLE,
      .ledsToggle.leds = LD2
    };
    
    osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
    RemoveTimer(RTC_ALARM_A);
    SetTimerOnce(10);
  }

  if (buttonEv)
  {
    osThreadFlagsClear(BUTTON_ACTION);
		SetRtcTime(00, 00, 00, FORMATO_24H);
    SetRtcDate(01, ENERO, 00, LUNES);
  }
}
	
static void CheckFlags(void)
{	
  /* Check if the Power On Reset flag is set */
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
	{
		/* Turn on LED2: Power on reset occurred */
		ledMessage_t ledMsg = {
    .mode        = LED_ON,
    .ledsOn.leds = LD2
	  };
	
    osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
	}
	/* Check if Pin Reset flag is set */
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
	{
		/* Turn on LED1: External reset occurred */
		ledMessage_t ledMsg = {
    .mode        = LED_ON,
    .ledsOn.leds = LD1
	  };
	
    osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
	}
	/* Clear source Reset Flag */
	__HAL_RCC_CLEAR_RESET_FLAGS();
}

void RTC_WKUP_IRQHandler(void)
{
    HAL_RTCEx_WakeUpTimerIRQHandler(&g_rtcHandle);
}

void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&g_rtcHandle);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *rtcHandle)
{
  osThreadFlagsSet (e_rtcThreadId, WAKE_UP_1s);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{  
  osThreadFlagsSet (e_rtcThreadId, WAKE_UP_ALARM_A);
}

void HAL_RTC_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{  
  __HAL_RTC_ALARM_CLEAR_FLAG(&g_rtcHandle, RTC_FLAG_ALRBF);
  
	ledMessage_t ledMsg = {
    .mode        = LED_TOGGLE,
    .ledsToggle.leds = LD2
  };
	
  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
	
  osThreadFlagsSet (e_rtcThreadId, WAKE_UP_ALARM_B);
}

static void InitUserButton(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct_B1;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);		// Interrupt lines 15:10 (B1)

	GPIO_InitStruct_B1.Pin 		= GPIO_PIN_13;
	GPIO_InitStruct_B1.Mode 	= GPIO_MODE_IT_RISING; 					// External Interrupt Mode with Rising edge trigger detection
	GPIO_InitStruct_B1.Pull 	= GPIO_PULLDOWN; 								// When B1 is pushed: 0 -> 1. Otherwise, 0 (Pull-Down)
	GPIO_InitStruct_B1.Speed 	= GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_B1);
}

static void Error_Handler(void) {
  /* Turn LED3 on */
  ledMessage_t ledMsg = {
    .mode = LED_ON,
    .ledsOn.leds = LD3
  };

  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
  while (1)
  {
  }
}
