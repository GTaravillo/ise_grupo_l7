#include "server.h"
/* ARM */
#include "stm32f4xx_hal.h"
#include "rl_net.h"
/* std */
#include <stdio.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_MAIN
#include PATH_LED
#include PATH_BUTTONS
#include PATH_ADC
#include PATH_LCD

// Main stack size must be multiple of 8 Bytes
#define SERVER_STK_SZ (1024U)

/* Public */
osThreadId_t        e_serverThreadId;
osMessageQueueId_t  e_serverInputMessageId;
osMessageQueueId_t  e_serverOutputMessageId;

/* Private */
static  uint64_t g_serverStk[SERVER_STK_SZ / 8];
static  char     g_lcdText[2][LCD_STR_MAX_LEN];
static  char     g_time[9];
static  char     g_date[9];

static const short c_timeLen = sizeof(g_time)/sizeof(g_time[0]);
static const short c_dateLen = sizeof(g_date)/sizeof(g_date[0]);

static  const osThreadAttr_t g_serverAttr = {
	.name = "Server Thread",
  .stack_mem  = &g_serverStk[0],
  .stack_size = sizeof(g_serverStk)
};

static  void  Run(void *argument);
static  void  LoadDefaultValues(void);
static  void  Error_Handler(void);


void Server_Initialize(void) 
{
  e_serverThreadId        = osThreadNew(Run, NULL, &g_serverAttr);
  // e_serverInputMessageId  = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);
  // e_serverOutputMessageId = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);

  if ((e_serverThreadId == NULL) /*|| (e_serverInputMessageId == NULL) || (e_serverOutputMessageId == NULL)*/) 
  {
    printf("[server::%s] osThreadNew ERROR!\n", __func__);
    
    Error_Handler();
  }
}

static void Run(void *argument) 
{
  LoadDefaultValues();
  //Buttons_Initialize();
  //ADC_Initialize();

  netInitialize();

  //TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  //TID_Display = osThreadNew (Display,  NULL, NULL);

  //osThreadExit();
}

static void LoadDefaultValues(void)
{
	osStatus_t osStatus;
  lcdMessage_t lcdMsg;
  const char* string;

  lcdMsg.mode = PRINT_NORMAL;

  lcdMsg.printMsg.printLine = PRINT_LINE_1;
  string = "      MDK-MW";
  strncpy(lcdMsg.printMsg.msg, string, LCD_STR_MAX_LEN - 1);
  osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
  
  lcdMsg.printMsg.printLine = PRINT_LINE_2;
  string = "HTTP Server example ";
  strncpy(lcdMsg.printMsg.msg, string, LCD_STR_MAX_LEN - 1);
  osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
  
  string = "LCD line 1";
  strncpy(g_lcdText[0], string, LCD_STR_MAX_LEN - 1);

  string = "LCD line 2";
  strncpy(g_lcdText[1], string, LCD_STR_MAX_LEN - 1);

  string = "00:00:00";
  strncpy(g_time, string, c_timeLen - 1);

  string = "01/01/25";
  strncpy(g_date, string, c_dateLen - 1);
}



uint16_t ADC_in(uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return ((uint16_t)val);
}

/* Read digital inputs */
uint8_t get_button (void) {
  return ((uint8_t)Buttons_GetState ());
}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

//  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
//    /* IP address change, trigger LCD update */
//    osThreadFlagsSet (TID_Display, 0x01);
//  }
}

const char* GetLcdTextInput(ELcdLine line)
{
  switch(line)
  {
    case LCD_LINE_1:
      return g_lcdText[LCD_LINE_1];
		
    case LCD_LINE_2:
      return g_lcdText[LCD_LINE_2];

    default:
      return "";
  }
}

void SetWebTime(const char* time)
{
  if (!IsTimeCorrect(time))
  {
    return;
  }

  strncpy(g_time, time, c_timeLen - 1);
  printf("[server::%s] Time set to [%s]", __func__, g_time);
}

void SetWebDate(const char* date)
{
  if (!IsDateCorrect(date))
  {
    return;
  }

  strncpy(g_date, date, c_dateLen - 1);
  printf("[server::%s] Time set to [%s]\n", __func__, g_date);
}

const char* GetTime(void)
{
  printf("[server::%s] Returning time [%s]\n", __func__, g_time);
  return g_time;
}

const char* GetDate(void)
{
  printf("[server::%s] Returning date [%s]\n", __func__, g_date);
  return g_date;
}

bool IsTimeCorrect(const char* time)
{
  int hh, mm, ss;
  bool correct = false;

  if (time == NULL)
  {
    printf("[server::%s] Time input pointer is NULL", __func__);
    return correct;
  }

  if (time[8] != '\0') 
  {
    printf("[server::%s] Incorrect time input format", __func__);
    return correct;
  }

  if (sscanf(time, "%2d:%2d:%2d", &hh, &mm, &ss) == 3)
  {
    if ((hh >= 0) && (hh < 24) && (mm >= 0) && (mm < 60) && (ss >= 0) && (ss < 60)) 
    {
      correct = true;
    }
  }

  return correct;
}

bool IsDateCorrect(const char* date)
{
  int dd, mm, yy;
  bool correct = false;

  // Check for NULL pointer
  if (date == NULL) 
  {
    printf("[server::%s] Time input pointer is NULL", __func__);
    return 0;
  }

  if (date[8] != '\0') 
  {
    printf("[server::%s] Incorrect time input format", __func__);
    return 0;
  }

  if (sscanf(date, "%2d/%2d/%2d", &dd, &mm, &yy) == 3) 
  {
    if (mm >= 1 && mm <= 12 && dd >= 1 && dd <= 31) 
    {
      correct = true;
    }
  }

  return correct;
}

static void Error_Handler(void) 
{
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
