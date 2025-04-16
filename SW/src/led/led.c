#include "led.h"
/* ARM */
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <stdbool.h>

#define ON  true
#define OFF false
// Para ver traza de los leds que se quieren encender
#define BYTE_TO_LEDS_PATTERN "%s%s%s"
#define BYTE_TO_LEDS(byte) \
                     ((byte) & 0x01 ? "[LD1]" : ""), \
                     ((byte) & 0x02 ? "[LD2]" : ""), \
                     ((byte) & 0x04 ? "[LD3]" : "")

/* Public */
osThreadId_t        e_ledThreadId;
osMessageQueueId_t  e_ledInputMessageId;

/* Private */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t      pin;
  ELedId        id;
	bool          on;
} ledInfo_t;

static  ledInfo_t  g_ledInfo[] = {
  { GPIOB, GPIO_PIN_0,  LD1, OFF },
  { GPIOB, GPIO_PIN_7,  LD2, OFF },
  { GPIOB, GPIO_PIN_14, LD3, OFF }
};

static  void  Run(void *argument);
static  void  InitLeds(void);
static  void  ProcessMessage(const ledMessage_t ledMsg);
static  void  HandleLedOn(const ledMessage_t ledMsg);
static  void  HandleLedOff(const ledMessage_t ledMsg);
static  void  HandleLedToggle(const ledMessage_t ledMsg);
static  void  HandleLedSequence(const ledMessage_t ledMsg);
static  void  Error_Handler(void);

/**************************************/

void LED_Initialize(void) 
{
	osMessageQueueAttr_t inputMessageAttr = {
		.name = "LED INPUT"
	};
  e_ledThreadId       = osThreadNew(Run, NULL, NULL);
  e_ledInputMessageId = osMessageQueueNew(LED_MAX_MSGS, sizeof(ledMessage_t), &inputMessageAttr);

  if ((e_ledThreadId == NULL) || (e_ledInputMessageId == NULL)) 
  {
    printf("[led::%s] ERROR! osThreadNew [%d] osMessageQueueNew [%d]\n", __func__, 
                                                          (e_ledThreadId == NULL), 
                                                   (e_ledInputMessageId == NULL));
    
    Error_Handler();
  }
}

static void Run(void *argument) 
{
  InitLeds();
	
	osStatus_t status;
  while(1) 
  {
    ledMessage_t ledMsg;
    printf("[led::%s] Waiting for message...\n", __func__);
		
    status = osMessageQueueGet(e_ledInputMessageId, &ledMsg, NULL, osWaitForever);
    if (osOK == status) 
    {
      ProcessMessage(ledMsg);
    }
    else
    {
      printf("[led::%s] osMessageQueueGet ERROR!\n", __func__);
      
      Error_Handler();
    }
  }
}

static void InitLeds(void)
{
  printf("[led::%s] Initializing mbed LEDs...\n", __func__);
	
  GPIO_InitTypeDef GPIO_InitStruct;
	__GPIOB_CLK_ENABLE();
	
	/* Configure GPIO pins: PB0 PB7 PB14 */
  GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  printf("[led::%s] OK\n", __func__);
	
}

static void ProcessMessage(const ledMessage_t ledMsg)
{
	switch(ledMsg.mode)
	{
		case LED_ON:
			HandleLedOn(ledMsg);
		  break;
		
		case LED_OFF:
			HandleLedOff(ledMsg);
		  break;
		
		case LED_TOGGLE:
			HandleLedToggle(ledMsg);
		  break;
		
		case LED_SEQUENCE:
			HandleLedSequence(ledMsg);
      break;

    default:
      break;
	}
}

static void HandleLedOn(const ledMessage_t ledMsg)
{
	ELedId ledsOn = ledMsg.ledsOn.leds;
	printf("[led::%s] LEDs to be turned on: " BYTE_TO_LEDS_PATTERN "\n", __func__, BYTE_TO_LEDS(ledsOn));
	
	
	int numLeds = sizeof(g_ledInfo)/sizeof(g_ledInfo[0]);
	
  for (int i = 0; i < numLeds; i++)
  {
    if (g_ledInfo[i].id & ledsOn)
    {
			if (g_ledInfo[i].on) 
      {
				continue;
      }
      HAL_GPIO_WritePin(g_ledInfo[i].port, g_ledInfo[i].pin, GPIO_PIN_SET);
			g_ledInfo[i].on = ON;
    }
  }
}

static void HandleLedOff(const ledMessage_t ledMsg)
{
  ELedId ledsOff = ledMsg.ledsOff.leds;
	printf("[led::%s] LEDs to be turned off: " BYTE_TO_LEDS_PATTERN "\n", __func__, BYTE_TO_LEDS(ledsOff));
	
	
	int numLeds = sizeof(g_ledInfo)/sizeof(g_ledInfo[0]);
	
  for (int i = 0; i < numLeds; i++)
  {
    if (g_ledInfo[i].id & ledsOff)
    {
			if (!g_ledInfo[i].on) 
      {
				continue;
      }
      HAL_GPIO_WritePin(g_ledInfo[i].port, g_ledInfo[i].pin, GPIO_PIN_RESET);
			g_ledInfo[i].on = OFF;
    }
  }
}

static void HandleLedToggle(const ledMessage_t ledMsg)
{
  ELedId ledsToggle = ledMsg.ledsToggle.leds;
	printf("[led::%s] LEDs to be toggled: " BYTE_TO_LEDS_PATTERN "\n", __func__, BYTE_TO_LEDS(ledsToggle));
	
	
	int numLeds = sizeof(g_ledInfo)/sizeof(g_ledInfo[0]);
	
  for (int i = 0; i < numLeds; i++)
  {
    if (g_ledInfo[i].id & ledsToggle)
    {
			bool wasOn = g_ledInfo[i].on;
			const char* previousStateStr = wasOn ? "ON" : "OFF";
			const char* newStateStr      = wasOn ? "OFF" : "ON";
			
			printf("[led::%s] LD%d [%s]->[%s]\n", __func__, i, previousStateStr, newStateStr);
			
      HAL_GPIO_TogglePin(g_ledInfo[i].port, g_ledInfo[i].pin);
			g_ledInfo[i].on = !wasOn;
    }
  }
}

static void HandleLedSequence(const ledMessage_t ledMsg)
{
//  ELedId ledsSequence[LED_MAX_STATES] = ledMsg.ledSequence.ledsStates;

//  for (int i = 0; i < LED_MAX_STATES)
//  {

//  }

//	printf("[led::%s] LEDs to be toggled: " BYTE_TO_LEDS_PATTERN "\n", __func__, BYTE_TO_LEDS(ledsToggle));
//	
//	
//	int numLeds = sizeof(g_ledInfo)/sizeof(g_ledInfo[0]);
//	
//  for (int i = 0; i < numLeds; i++)
//  {
//    if (g_ledInfo[i].id & ledsToggle)
//    {
//			bool wasOn = g_ledInfo[i].on;
//			const char* previousStateStr = wasOn ? "ON" : "OFF";
//			const char* newStateStr      = wasOn ? "OFF" : "ON";
//			
//			printf("[led::%s] LD%d [%s]->[%s]\n", __func__, i, previousStateStr, newStateStr);
//			
//      HAL_GPIO_TogglePin(g_ledInfo[i].port, g_ledInfo[i].pin);
//			g_ledInfo[i].on = !wasOn;
//    }
//  }
}

static void Error_Handler(void)
{
  /* Turn LED3 on */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	printf("[led::%s] ERROR! LEDs won't answer to messages from now on...\n", __func__);
	
  while (1)
  {
  }
}
