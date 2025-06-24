#include "lcd.h"
#include "Arial12x12.h"
/* ARM */
#include "driver_SPI.h"
#include "stm32f4xx_hal.h"
/* std */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED

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
#define  SPICLK                      20000000
#define  DISPLAY_OFF                 0xAE  // Const 
#define  DISPLAY_ON                  0xAF  // Const
#define  START_LINE_ADDRESS          0x40  // [D5:D0]  Sets the display RAM display start line address
#define  PAGE_ADDRESS                0xB0  // [D3:D0]  Sets the display RAM page address
#define  MSB_COLUMN_ADDRESS          0x10  // [D3:D0]  Sets the most significant 4 bits of the display RAM column address
#define  LSB_COLUMN_ADDRESS          0x00  // [D3:D0]  Sets the least significant 4 bits of the display RAM column address
#define  STATUS_READ                 0x00  // [D7:D4]  Reads the status data
#define  ADC_SELECT                  0xA0  // Const    Sets the display RAM address SEG output correspondance
#define  DISPLAY_NORMAL              0xA6  // Const    Sets the LCD display normal
#define  DISPLAY_REVERSE             0xA7  // Const    Sets the LCD display reverse
#define  DISPLAY_ALL_POINTS_OFF      0xA4  // Const    Display all points: normal display
#define  DISPLAY_ALL_POINTS_ON       0xA5  // Const    Display all points: ON
#define  BIAS_SET_1_9                0xA2  // Const    Sets the LCD drive voltage bias ratio: 1/9 bias
#define  COLUMN_ADDRESS_INCREMENT    0xE0  // Const    Read/modify/write: Column address increment
#define  END                         0xEE  // Const    Clear read/modify/write
#define  RESET                       0xE2  // Const    Internal reset
#define  COMMON_OUTPUT_NORMAL        0xC8  // Const    Select COM output scan direction: normal
#define  BOOSTER_VR_VF_POWER_SUPPLY  0x2F  // Const
#define  VR_VF_POWER_SUPPLY          0x2B  // Const
#define	 VF_POWER_SUPPLY             0x29  // Const
#define  RESISTOR_RATIO              0x20  // [D2:D0]  Vo voltage regulator internal resistor ratio (Rb/Ra)
#define  CONTRAST_MODE_SET           0x81  // Const    Electronic volume mode set: Set the Vo output voltage electronic volume register
#define  CONTRAST_VALUE              0x00  // [D5:D0]  Electronic volume value
#define  SLEEP_MODE                  0xAC  // Const    Sleep mode
#define  NORMAL_MODE                 0xAD  // Const    Normal mode
/* Flags */
#define LCD_TRANSFER_COMPLETE 0x01		// Used to signal the end of SPI transfer
/* Config */
#define  TIM7_PERIOD     83    // 84MHz/84 = 1MHz = 1us
#define  TIM7_PRESCALER  0

/* Public */
osThreadId_t        e_lcdThreadId;
osMessageQueueId_t  e_lcdInputMessageId;

extern ARM_DRIVER_SPI   Driver_SPI1;
       ARM_DRIVER_SPI*  SPIdrv = &Driver_SPI1;

/* Private */
static  TIM_HandleTypeDef  g_Tim7Handle;
static  unsigned char      g_Buffer[512];
static  uint8_t            g_PositionL1;
static  uint8_t            g_PositionL2;

static  void  Run(void *argument);
static  void  ProcessMessage(const lcdMessage_t lcdMsg);
static  void  HandlePrintableMsg(const lcdMessage_t lcdMsg);
static  void  HandleColumnPrintMsg(const lcdMessage_t lcdMsg);
static  void  HandleMoveLines(const lcdMessage_t lcdMsg);
static  void  InitGpios(void);
static  void  InitTim7(void);
static  void  StartLcd(void);
static  void  ResetLcd(void);
static  void  Delay(const uint32_t n_microseconds);
static  void  EraseLine1(void);
static  void  EraseLine2(void);
static  void  EraseDisplay(void);
static  void  LCD_wr_data(const unsigned char data);
static  void  LCD_wr_cmd(const unsigned char cmd);
static  void  PrintBuffer(void);
static  void  LCD_SymbolToLocalBuffer_L1(const uint8_t symbol);
static  void  LCD_SymbolToLocalBuffer_L2(const uint8_t symbol);
static  void  SymbolToLocalBuffer(const EPrintLine line, const uint8_t symbol);
static  void  PrintString(const EPrintLine line, const char* string);
static  bool  CheckString(const char* string);
static  void  MoveBuffer(const EMoveDirection dir);
static  void  PaintVerticalPattern(const uint16_t column, const uint32_t pattern);

void  ARM_SPI_SignalEvent(uint32_t event);
static  void  Error_Handler(void);

/**************************************/

void LCD_Initialize(void) 
{
	osMessageQueueAttr_t inputMessageAttr = {
		.name = "LCD INPUT"
	};
  e_lcdThreadId       = osThreadNew(Run, NULL, NULL);
  e_lcdInputMessageId = osMessageQueueNew(LCD_MAX_MSGS, sizeof(lcdMessage_t), &inputMessageAttr);

  if ((e_lcdThreadId == NULL) || (e_lcdInputMessageId == NULL)) 
  {
   // printf"[lcd::%s] ERROR! osThreadNew [%d] osMessageQueueNew [%d]\n", __func__, 
   //                                                       (e_lcdThreadId == NULL), 
   //                                                (e_lcdInputMessageId == NULL));
    
    Error_Handler();
  }
}

static void Run(void *argument) 
{
  InitGpios();
  InitTim7();
  StartLcd();
  EraseDisplay();
	
	osStatus_t status;
	
  while(1)
  {
    lcdMessage_t lcdMsg;
		printf("[lcd::Run] Waiting for message...\n");
    status = osMessageQueueGet(e_lcdInputMessageId, &lcdMsg, NULL, osWaitForever);
   if (osOK == status) 
   {
     ProcessMessage(lcdMsg);
   }
   else
   {
    // printf"[lcd::%s] osMessageQueueGet ERROR!\n", __func__);
     
     Error_Handler();
   }
  }
}

static void ProcessMessage(const lcdMessage_t lcdMsg)
{
	switch (lcdMsg.mode)
	{
		case PRINT_NORMAL:
			HandlePrintableMsg(lcdMsg);
		  break;
		
		case PRINT_COLUMN:
			HandleColumnPrintMsg(lcdMsg);
		  break;
		
		case MOVE:
			HandleMoveLines(lcdMsg);
		  break;
	}
}

static void HandlePrintableMsg(const lcdMessage_t lcdMsg)
{
	EPrintLine line = lcdMsg.printMsg.printLine;
	const char* msg = lcdMsg.printMsg.msg;
	
	PrintString(line, msg);
}

static void HandleColumnPrintMsg(const lcdMessage_t lcdMsg)
{
	const uint16_t column  = lcdMsg.columnPrint.column;
	const uint32_t pattern = lcdMsg.columnPrint.pattern;
	
	if (column >= LCD_COLUMNS)  // unsigned -> no negative values 
  {
   // printf"[lcd::%s] Error! Column [%d] > last calumn [%d]\n", __func__, column, (LCD_COLUMNS - 1));
		
		return;
  }

  PaintVerticalPattern(column, pattern);
}

static void HandleMoveLines(const lcdMessage_t lcdMsg)
{
	EMoveDirection direction = lcdMsg.moveLines.direction;
	
	MoveBuffer(direction);
}

static void InitGpios(void) 
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  
  /* Reset - A6 */
  GPIO_InitStruct.Pin   = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  
  /* nCS - D14 */
  GPIO_InitStruct.Pin   = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

  /* A0 - F13 */
  GPIO_InitStruct.Pin   = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);	
}

static void InitTim7(void) 
{
  __HAL_RCC_TIM7_CLK_ENABLE();

  g_Tim7Handle.Instance       = TIM7;
  g_Tim7Handle.Init.Prescaler = TIM7_PRESCALER;		
  g_Tim7Handle.Init.Period    = TIM7_PERIOD;

  HAL_TIM_Base_Init(&g_Tim7Handle);
}

void StartLcd(void) {
  ResetLcd();

 LCD_wr_cmd(DISPLAY_OFF);                 /* Display off */
 LCD_wr_cmd(BIAS_SET_1_9);                /* Fija el valor de la relación de la tensión de polarización del LCD a 1/9 */
 LCD_wr_cmd(ADC_SELECT);                  /* El direccionamiento de la RAM de datos del display es la normal */
 LCD_wr_cmd(COMMON_OUTPUT_NORMAL);        /* El scan en las salidas COM es el normal */
 LCD_wr_cmd(RESISTOR_RATIO+2);            /* Fija la relación de resistencias interna a 2 [0, 7] */
 LCD_wr_cmd(BOOSTER_VR_VF_POWER_SUPPLY);  /* Power on */
 LCD_wr_cmd(START_LINE_ADDRESS);          /* Display empieza en la línea 0 */
 LCD_wr_cmd(DISPLAY_ON);                  /* Display on */
 LCD_wr_cmd(CONTRAST_MODE_SET);           /* Contraste */
 LCD_wr_cmd(CONTRAST_VALUE+20);           /* Valor contraste [0, 63] */
 LCD_wr_cmd(DISPLAY_ALL_POINTS_OFF);      /* Display all points normal */
 LCD_wr_cmd(DISPLAY_NORMAL);              /* LCD Display normal */
	printf("[lcd::%s] OK\n", __func__);
	
}

static void ResetLcd(void) 
{
	
  /** Inicialización y configuración del driver SPI **/
  /* Initialize the SPI driver */
  SPIdrv->Initialize(ARM_SPI_SignalEvent);
  /* Power up the SPI peripheral */
  SPIdrv->PowerControl(ARM_POWER_FULL);
  /* Configure the SPI to Master, 8-bit mode @20000 kBits/sec */
  SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), SPICLK);

  /* Reset */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  Delay(1);	// nRST 1us 
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  Delay(1000);	// Wait 1ms after reset
}

static void Delay(uint32_t n_microsegundos) 
{
  uint32_t n_flags_cnt = 0;
  
  /* Start Timer */
  HAL_TIM_Base_Start(&g_Tim7Handle);

  /* Timer overflows every 1us */
  do 
  {
    if (__HAL_TIM_GET_FLAG(&g_Tim7Handle, TIM_FLAG_UPDATE)) 
    {
      /* Count number of overflows */
      __HAL_TIM_CLEAR_FLAG(&g_Tim7Handle, TIM_FLAG_UPDATE);
      n_flags_cnt++;
    }
  } while (n_flags_cnt < n_microsegundos);

  __HAL_TIM_SET_COUNTER(&g_Tim7Handle, 0);  // Reset Timer counter
	
  /** Stop Timer **/
  HAL_TIM_Base_Stop(&g_Tim7Handle);
}

static void EraseLine1(void)
{
	const int lenBuffer = sizeof(g_Buffer)/sizeof(g_Buffer[0]);
	const int lenLine1 = lenBuffer/2;
  memset(g_Buffer, 0, lenLine1);
  g_PositionL1 = 0;
}

static void EraseLine2(void)
{
	const int lenBuffer = sizeof(g_Buffer)/sizeof(g_Buffer[0]);
	const int lenLine2 = lenBuffer/2;
  memset(g_Buffer + lenLine2, 0, lenLine2);
  g_PositionL2 = 0;
}

static void EraseDisplay(void) 
{
  const int lenBuffer = sizeof(g_Buffer)/sizeof(g_Buffer[0]);
  memset(g_Buffer, 0, lenBuffer);
	g_PositionL1 = 0;
	g_PositionL2 = 0;
  PrintBuffer();
}

static void PrintBuffer(void) 
{
  int i;
  LCD_wr_cmd(LSB_COLUMN_ADDRESS+0);        /* Dirección: _ _ _ _ 0 0 0 0 */
  LCD_wr_cmd(MSB_COLUMN_ADDRESS+0);        /* Dirección: 0 0 0 0 _ _ _ _ */
  LCD_wr_cmd(PAGE_ADDRESS+0);              /* Página: 0 */

  for (i = 0; i < 128; i++) 
  {
    LCD_wr_data(g_Buffer[i]);
  }

  LCD_wr_cmd(LSB_COLUMN_ADDRESS+0);        /* Dirección: _ _ _ _ 0 0 0 0 */
  LCD_wr_cmd(MSB_COLUMN_ADDRESS+0);        /* Dirección: 0 0 0 0 _ _ _ _ */
  LCD_wr_cmd(PAGE_ADDRESS+1);              /* Página: 1 */
	
  for (i = 128; i < 256; i++) 
  {
    LCD_wr_data(g_Buffer[i]);
  }
	
  LCD_wr_cmd(LSB_COLUMN_ADDRESS+0);        /* Dirección: _ _ _ _ 0 0 0 0 */
  LCD_wr_cmd(MSB_COLUMN_ADDRESS+0);        /* Dirección: 0 0 0 0 _ _ _ _ */
  LCD_wr_cmd(PAGE_ADDRESS+2);              /* Página: 2 */
	
  for (i = 256; i < 384; i++) 
  {
    LCD_wr_data(g_Buffer[i]);
  }
	
  LCD_wr_cmd(LSB_COLUMN_ADDRESS+0);        /* Dirección: _ _ _ _ 0 0 0 0 */
  LCD_wr_cmd(MSB_COLUMN_ADDRESS+0);        /* Dirección: 0 0 0 0 _ _ _ _ */
  LCD_wr_cmd(PAGE_ADDRESS+3);              /* Página: 3 */

  for (i = 384; i < 512; i++) 
  {
    LCD_wr_data(g_Buffer[i]);
  }
}

static void LCD_wr_cmd(unsigned char cmd) 
{
  /* Chip Select activate: nCS = '0' */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  /* Config: A0 = '0' */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);

  /* Data send */
  SPIdrv->Send(&cmd, sizeof(cmd));
  osThreadFlagsWait(LCD_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  /* Chip Select deactivate: nCS = '1' */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_wr_data(unsigned char data) 
{
  /* Chip Select activate: nCS = '0' */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  /* Graphic info: A0 = '1' */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

  /* Data send */
  SPIdrv->Send(&data, sizeof(data));
  osThreadFlagsWait(LCD_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  /* Chip Select deactivate: nCS = '1' */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_SymbolToLocalBuffer_L1(uint8_t symbol) 
{
  uint8_t i, value1, value2;
  uint16_t offset = 0;

  offset = 25*(symbol - ' ');
  if (g_PositionL1 + Arial12x12[offset] < 128) 
  {
    for (i = 0; i < /*12*/Arial12x12[offset]; i++)  // Cambiar para ajustar espacio entre caracteres
    {  
      value1 = Arial12x12[offset + i*2 + 1];        // Bytes impares
      value2 = Arial12x12[offset + i*2 + 2];        // Bytes pares

      g_Buffer[i + g_PositionL1] = value1;              // Página 0
      g_Buffer[i + 128 + g_PositionL1] = value2;        // Página 1
    }
    g_PositionL1 = g_PositionL1 + Arial12x12[offset];   // Primer byte de Arial tiene la anchura de datos del carácter
  }
}

static void LCD_SymbolToLocalBuffer_L2(uint8_t symbol) 
{
  uint8_t i, value1, value2;
  uint16_t offset = 0;

  offset = 25*(symbol - ' ');

  if (g_PositionL2 + Arial12x12[offset] < 128) 
  {
    for (i = 0; i < /*12*/Arial12x12[offset]; i++)  // Cambiar para ajustar espacio entre caracteres 
    {  
      value1 = Arial12x12[offset + i*2 + 1];        // Bytes impares
      value2 = Arial12x12[offset + i*2 + 2];        // Bytes pares

      g_Buffer[i + 256 + g_PositionL2] = value1;        // Página 0
      g_Buffer[i + 384 + g_PositionL2] = value2;        // Página 1
    }
    g_PositionL2 = g_PositionL2 + Arial12x12[offset];   // Primer byte de Arial tiene la anchura de datos del carácter
  }
}

static void SymbolToLocalBuffer(EPrintLine line, uint8_t symbol) 
{
	switch(line)
	{
		case PRINT_LINE_1:
			EraseLine1();
      LCD_SymbolToLocalBuffer_L1(symbol);
      break;

    case PRINT_LINE_2:
			EraseLine2();
      LCD_SymbolToLocalBuffer_L2(symbol);
      break;
		
    default:
			printf("[lcd::%s] Error! Can't print on line [%d]\n", __func__, line);
      
    return;			
	}

  PrintBuffer();
}

static void PrintString(EPrintLine line, const char* string)
{
	if (!CheckString(string)) 
  {
   // printf"[lcd::%s] Error! String is larger than maximum [%d]\n", __func__, LCD_STR_MAX_LEN);
    
    return;
	}
	
	size_t strLen = strlen(string);
	switch(line)
	{
		case PRINT_LINE_1:
			EraseLine1();
      for (uint32_t i = 0; i < strLen; i++) 
      {
        LCD_SymbolToLocalBuffer_L1(string[i]);
      }
      break;

    case PRINT_LINE_2:
			EraseLine2();
      for (uint32_t i = 0; i < strLen; i++)
      {
        LCD_SymbolToLocalBuffer_L2(string[i]);
      }
      break;
		
    default:
			printf("[lcd::%s] Error! Can't print on line [%d]\n", __func__, line);
      return;			
	}

  PrintBuffer();
}

static bool CheckString(const char* string)
{
	size_t len = 0;

  while (len < LCD_STR_MAX_LEN && string[len] != '\0') 
  {
    len++;
  }

  return (string[len] == '\0');
}

static void MoveBuffer(EMoveDirection dir) 
{
  int  col;
  int  page;
  bool correct = true;

  switch(dir) 
  {
    case UP:
      col = 0;
      do 
      {
        // Compruebo que la primera fila está vacía antes de desplazar
		correct = (g_Buffer[col] & 0x01) == 0x00;
		col++;
	  } 
      while (correct && (col < 128));
	
      if (!correct) { break; }

	  for (page = 0; page < 4; page++) 
      {
        for (col = 0; col < 128; col++) 
        {
          /* Si no es la última página, le añado el LSb de la página anterior 
             en el MSb de la página actual*/
          if (3 != page) 
          {
            g_Buffer[col + page*128] = ((g_Buffer[col + (page + 1)*128]) << 7) | (g_Buffer[col + page*128] >> 1);
          } 
          else 
          {
            g_Buffer[col + page*128] = g_Buffer[col + page*128] >> 1;
          }
        }
      }
      break;
    
    case DOWN:
      // Compruebo que la ultima fila está vacia antes de desplazar
      col = 384;
      do 
      {
		correct = (g_Buffer[col] & 0x80) == 0x00; 
        col++;
      } 
      while (correct && (col < 512));
			
      if (!correct) { break; }

      for (page = 3; page >= 0; page--) 
      {
        for (col = 0; col < 128; col++) 
        {
          /* Si no es la primera página, le añado el MSb de la página anterior 
             en el LSb de la página actual*/
          if (0 != page) 
          {
            g_Buffer[col + page*128] = (g_Buffer[col + page*128] << 1) | (g_Buffer[col + (page - 1)*128] >> 7);
          } 
          else 
          {
            g_Buffer[col] = g_Buffer[col] << 1;
          }
        }
      }
      break;

      case LEFT:
        // Compruebo que la primera columna está vacia antes de desplazar
        page = 0;
        do 
        {
          correct = g_Buffer[page*128] == 0x00;
          page++;
        } 
        while (correct && (page < 4));

        if (!correct) { break; }

        for (page = 0; page < 4; page++) 
        {
          for (col = 0; col < 128; col++) 
          {
            /* Si no es la ultima columna, copio el contenido de la columna siguiente
               en la columna actual*/
            if (col < 127) 
            {
              g_Buffer[col + page*128] = g_Buffer[col+1 + page*128];
            } 
            else 
            {
              g_Buffer[col + page*128] = 0;
            }
          }
        }
        break;

      case RIGHT:
        // Compruebo que la ultima columna está vacia antes de desplazar
        page = 0;
        do 
        {
          correct = g_Buffer[127 + page*128] == 0x00;
          page++;
        } 
        while (correct && (page < 4));

        if (!correct) { break; }

        for (page = 0; page < 4; page++) 
        {
          for (col = 127; col >= 0; col--) 
          {
            /* Si no es la primera columna, copio el contenido de la columna anterior
               en la columna actual*/
            if (0 < col) 
            {
              g_Buffer[col + page*128] = g_Buffer[col-1 + page*128];
            } 
            else 
            {
              g_Buffer[col + page*128] = 0;
            }
          }
        }
        break;

    default:
      break;
  }
	
  PrintBuffer();
}

static void PaintVerticalPattern(const uint16_t column, const uint32_t pattern) 
{
  uint8_t page1 =  pattern        & 0xFF;
  uint8_t page2 = (pattern >> 8)  & 0xFF;
  uint8_t page3 = (pattern >> 16) & 0xFF;
  uint8_t page4 = (pattern >> 24) & 0xFF;

  g_Buffer[column] = page1;
  g_Buffer[column + 128] = page2;
  g_Buffer[column + 256] = page3;
  g_Buffer[column + 384] = page4;

  PrintBuffer();
}

void ARM_SPI_SignalEvent(uint32_t event) 
{
  if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE) 
  {
    // Data Transfer completed
    osThreadFlagsSet(e_lcdThreadId, LCD_TRANSFER_COMPLETE);
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
