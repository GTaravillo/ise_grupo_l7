#include "LedStripManager.h"
#include "led.h"
/* ARM */
#include "driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
/* Interfaces */
#include "../config/Paths.h"

/*  System Core Clock
  HSE_VALUE        = 8MHz
  PLLM             = 8    [0,63]      ->  8MHz/4    = 1MHz
  PLLN             = 336 [50,432]     ->  1MHz*336  = 336MHz
  PLLP             = RCC_PLLP_DIV2    ->  336MHz/2  = 168MHz  = SystemCoreClock
  AHBCLKDivider    = RCC_SYSCLK_DIV1  ->  168MHz
    APB1CLKDivider = RCC_HCLK_DIV4    ->  168MHz/4  = 42MHz   = APB1 peripheral clocks (max 45MHz)
                                          42MHz*2   = 84MHz   = APB1 Timer clocks  <-- TIM7
    APB2CLKDivider = RCC_HCLK_DIV2    ->  168MHz/2  = 84MHz   = APB2 peripheral clocks (max 90MHz)
                                          84MHz*2   = 168MHz  = APB2 Timer clocks
*/

/* Registros */
#define  SPICLK                      10000000
#define  START                       0x00  // 4
#define  STOP                        0xFF  // 4
#define  BRIGHTNESS_MASK             0xE0  // [D7:D5]  // Brightness (0-31)

/* Flags */
#define LCD_TRANSFER_COMPLETE        0xFF  // Used to signal the end of SPI transfer
/* Config */
//#define  TIM7_PERIOD     83    // 84MHz/84 = 1MHz = 1us
//#define  TIM7_PRESCALER  0

/* Public */
typedef struct{
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_color_t;

osThreadId_t        e_ledStripManagerThreadId;

extern ARM_DRIVER_SPI   Driver_SPI2;
       ARM_DRIVER_SPI*  SPIdrv2 = &Driver_SPI2;

/* Private */
static  void  Run(void *argument);
static  void  Led_wr_cmd(unsigned char cmd);
static  void  Led_wr_data(unsigned char data);
void  ARM_LedSPI_SignalEvent(uint32_t event);

/**************************************/

void LedStripManagerInitialize(void)
{
  e_ledStripManagerThreadId = osThreadNew(Run, NULL, NULL);

  if ((e_ledStripManagerThreadId == NULL))
  {
    printf("[lcd::%s] ERROR! osThreadNew [%d]\n", __func__, (e_ledStripManagerThreadId == NULL));
  }
}

static void Run(void *argument)
{
  ledMessage_t ledMsg = {
        .mode = LED_ON,
        .ledsOn.leds = LD1
    };
  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);

  const short brightness = 15;
  /** Inicialización y configuración del driver SPI **/
  /* Initialize the SPI driver */
  SPIdrv2->Initialize(ARM_LedSPI_SignalEvent);
  /* Power up the SPI peripheral */
  SPIdrv2->PowerControl(ARM_POWER_FULL);
  /* Configure the SPI to Master, 8-bit mode @20000 kBits/sec */
  SPIdrv2->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), SPICLK);

  Led_wr_cmd(START);
  //osDelay(100);
  Led_wr_cmd(START);
  //osDelay(100);
  Led_wr_cmd(START);
  //osDelay(100);
  Led_wr_cmd(START);
  //osDelay(100);
  // -------- LED 1 --------
  Led_wr_data(BRIGHTNESS_MASK | brightness);
  //osDelay(100);
  Led_wr_data(0xFF);  // Blue
  //osDelay(100);
  Led_wr_data(0x00);  // Green
  //osDelay(100);
  Led_wr_data(0x00);  // Red
  //osDelay(100);
  // -------- LED 2 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 3 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 4 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 5 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 6 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 7 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 8 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 9 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  // -------- LED 10 --------
  Led_wr_data(BRIGHTNESS_MASK | 0x08);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0x00);
  //osDelay(1);
  Led_wr_cmd(0xFF);
  //osDelay(1);

  Led_wr_cmd(STOP);
  //osDelay(100);
  Led_wr_cmd(STOP);
  //osDelay(100);
  Led_wr_cmd(STOP);
  //osDelay(100);
  Led_wr_cmd(STOP);
  //osDelay(100);

  ledMessage_t ledMsg2 = {
        .mode = LED_ON,
        .ledsOn.leds = LD3
    };
  osStatus_t osStatus2 = osMessageQueuePut(e_ledInputMessageId, &ledMsg2, 1, 0);

  while(1)
  {



  }
}

static void Led_wr_cmd(unsigned char cmd)
{

  /* Data send */
  SPIdrv2->Send(&cmd, sizeof(cmd));
  osThreadFlagsWait(LCD_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

}

static void Led_wr_data(unsigned char data)
{
  /* Data send */
  SPIdrv2->Send(&data, sizeof(data));
  osThreadFlagsWait(LCD_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

void ARM_LedSPI_SignalEvent(uint32_t event)
{
  ledMessage_t ledMsg = {
        .mode = LED_ON,
        .ledsOn.leds = LD2
    };
  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);

  if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
  {
    // Data Transfer completed
    osThreadFlagsSet(e_ledStripManagerThreadId, LCD_TRANSFER_COMPLETE);
  }
  if (event & ARM_SPI_EVENT_MODE_FAULT)
  {
    // Master Mode Fault (SS deactivated when Master)
  }
  if (event & ARM_SPI_EVENT_DATA_LOST)
  {
    // Data lost: Receive overflow / Transmit underflow
  }
}
