/*
Velocidad?

*/

#include "PositionManager.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED

#define POWER_ON_RESET  150     // Tiempo (ms) que debe mantenerse en reset al inicializar
#define SLAVE_1_ADDR    0x20    // 0x20 en 7 bits es 0x40 en 8 bits - A0, A1 Y A2 SIN SOLDAR
#define SLAVE_2_ADDR    0x21	  // 0x21 en 7 bits es 0x42 en 8 bits - A0 SOLDADO A VCC
#define SLAVE_3_ADDR    0x22	  // 0x20 en 7 bits es 0x40 en 8 bits - A1 SOLDADO A VCC
#define SLAVE_4_ADDR    0x24	  // 0x20 en 7 bits es 0x40 en 8 bits - A2 SOLDADO A VCC

	// estos buffers guardan la info de cada expansor i2c
  uint8_t buff_exp1[2] = {0x00, 0x00};				// 1er expansor -  a1, b1, c1, d1, e1, f1, g1, h1  	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																							//  							 a2, b2, c2, d2, e2, f2, g2, h2  	 --  	P17, P16, P15, P14, P13, P12, P11, P10
															
	uint8_t buff_exp2[2] = {0xFF, 0xFF};				// 2o expansor  -  a3, b3, c3, d3, e3, f3, g3, h3  	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																							//  							 a4, b4, c4, d4, e4, f4, g4, h4  	 --  	P17, P16, P15, P14, P13, P12, P11, P10
															
	uint8_t buff_exp3[2] = {0xFF, 0xFF}; 				// 3o expansor  -  a5, b5, c5, d5, e5, f5, g5, h5 	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																							//  							 a6, b6, c6, d6, e6, f6, g6, h6	   -- 	P17, P16, P15, P14, P13, P12, P11, P10
															
	uint8_t buff_exp4[2] = {0x00, 0x00}; 				// 4o expansor  -  a7, b7, c7, d7, e7, f7, g7, h7	   -- 	P00, P01, P02, P03, P04, P05, P06, P07
																							//  							 a8, b8, c8, d8, e8, f8, g8, h8	   --  	P17, P16, P15, P14, P13, P12, P11, P10
	
	uint8_t last_buff_exp1[2] = {0x00, 0x00};
	uint8_t last_buff_exp2[2] = {0xFF, 0xFF};
	uint8_t last_buff_exp3[2] = {0xFF, 0xFF};
	uint8_t last_buff_exp4[2] = {0x00, 0x00};
	
	//Mapeado de 1 expansor:
	/*
		P17 - 0xFF,0x7F / P16 - 0xFF,0xBF / P15 - 0xFF,0xDF / P14 - 0xFF,0xEF / 	TODO EL RATO, 1ER BYTE A FF
		P13 - 0xFF,0xF7 / P12 - 0xFF,0xFB / P11 - 0xFF,0xFD / P10 - 0xFF,0xFE
		
		P00 - 0xFE,0xFF / P01 - 0xFD,0xFF / P02 - 0xFB,0xFF / P03 - 0xF7,0xFF /   TODO EL RATO, 2º BYTE A FF
	  P04 - 0xEF,0xFF / P05 - 0xDF,0xFF / P06 - 0xBF,0xFF / P07 - 0x7F,0xFF
	*/

/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

osThreadId_t e_positionManagerThreadId;
osMessageQueueId_t  e_positionMessageId;
	


/* Private */
static  void  Run(void *argument);

void  I2C_SignalEvent(uint32_t event);

void Pcf8575Initialize(void);

void leerExpansor(uint8_t slave_addr, uint8_t *buffer);

static void checkExpansorChanges(uint8_t *buff_exp, uint8_t *prev_buff_exp, int exp_num);

uint8_t mapPinToCasilla(int exp_num, int pin);



/**************************************/

void PositionManagerInitialize(void)
{
  e_positionManagerThreadId = osThreadNew(Run, NULL, NULL);
	
	e_positionMessageId = osMessageQueueNew(10, sizeof(ECasilla), NULL);

  if ((e_positionManagerThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_positionManagerThreadId == NULL));
  }
}

static void Run(void *argument)
{
	int32_t status;
  printf("[posicion::Run] Initializing I2C\n");
  I2Cdrv->Initialize(I2C_SignalEvent);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, NULL);
  osDelay(1000);
	
	Pcf8575Initialize();
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin   = GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  while (1)
  {
		osThreadFlagsWait(HALL_DETECTED, osFlagsWaitAll, osWaitForever);  // Espera a que haya interrupción (cambio de estado de algún hall)

		leerExpansor(SLAVE_1_ADDR, buff_exp1);
		leerExpansor(SLAVE_2_ADDR, buff_exp2);
		leerExpansor(SLAVE_3_ADDR, buff_exp3);
		leerExpansor(SLAVE_4_ADDR, buff_exp4);
		
    // Comprobar cambios para cada expansor
    checkExpansorChanges(buff_exp1, last_buff_exp1, 1);
    checkExpansorChanges(buff_exp2, last_buff_exp2, 2);
    checkExpansorChanges(buff_exp3, last_buff_exp3, 3);
    checkExpansorChanges(buff_exp4, last_buff_exp4, 4);
  
    // Actualiza los valores previos de los buffers después de la lectura
    memcpy(last_buff_exp1, buff_exp1, 2);
    memcpy(last_buff_exp2, buff_exp2, 2);
    memcpy(last_buff_exp3, buff_exp3, 2);
    memcpy(last_buff_exp4, buff_exp4, 2);

  }
}

void I2C_SignalEvent(uint32_t event) 
{
	printf("I2C_SignalEvent [%#x]\n", event);
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) 
  {
    /* Transfer or receive is finished */
		osThreadFlagsSet(e_positionManagerThreadId, ARM_I2C_EVENT_TRANSFER_DONE);
  }
}

void Pcf8575Initialize(void) {
    uint8_t writeBuffer[2] = {0xFF, 0xFF}; // Configura todos los pines como entradas (1 = High)

    const uint8_t slaveAddresses[4] = {
        SLAVE_1_ADDR,
        SLAVE_2_ADDR,
        SLAVE_3_ADDR,
        SLAVE_4_ADDR
    };

    for (int i = 0; i < 4; i++) {
        int32_t status = I2Cdrv->MasterTransmit(slaveAddresses[i], writeBuffer, 2, false);
        if (status != ARM_DRIVER_OK) {
            printf("Error al enviar datos al expansor %d (addr 0x%02X)\n", i + 1, slaveAddresses[i]);
            continue;
        }
        osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
    }
    printf("Todos los expansores PCF8575 inicializados correctamente.\n");
}

void leerExpansor(uint8_t slave_addr, uint8_t *buffer) {
    int32_t status = I2Cdrv->MasterReceive(slave_addr, buffer, 2, false);
    if (status != ARM_DRIVER_OK) {
        printf("Error reading from slave 0x%02X\n", slave_addr);
        return;
    }
    osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
}

uint8_t mapPinToCasilla(int exp_num, int pin)
{
    // Definimos cómo se mapea cada expansor a las filas y columnas del tablero
    // Por ejemplo, el expansor 1 mapea a la fila 1, el expansor 2 a la fila 2, etc.
    // Y el pin se mapea directamente a las columnas (1 a 8)

    uint8_t fila = exp_num;  // Fila en el tablero (1 a 4 corresponden a las primeras 4 filas)
    uint8_t columna = pin + 1;  // Las columnas en el tablero son 1-8, y los pins 0-7

    // La casilla en formato numérico es columna * 10 + fila
    uint8_t casilla = (columna * 10) + fila;

    return casilla;
}

static void checkExpansorChanges(uint8_t *buff_exp, uint8_t *prev_buff_exp, int exp_num)
{
    for (int i = 0; i < 2; i++) {
        uint8_t diff = buff_exp[i] ^ prev_buff_exp[i]; // XOR para obtener los bits que han cambiado
        if (diff != 0) {
            for (int b = 0; b < 8; b++) {
                if (diff & (1 << b)) {
                    // Calculamos la casilla en el tablero de ajedrez
                    uint8_t casilla = mapPinToCasilla(exp_num, (i * 8) + b);

                    // Llenar la estructura con la casilla
                    ECasilla casilla_Accionada;
                    casilla_Accionada.casilla = casilla;

                    // Enviar la casilla a la cola
                    osMessageQueuePut(e_positionMessageId, &casilla_Accionada, 0, osWaitForever);
                }
            }
        }
    }
}

static void ProcessCasillaChanges(void *argument)
{
    ECasilla casilla_Accionada;
    while (1) {
        // Esperar por un mensaje de cambio de casilla
        osMessageQueueGet(e_positionMessageId, &casilla_Accionada, NULL, osWaitForever);

        // Aquí procesas el cambio (puedes actualizar el tablero, verificar la jugada, etc.)
        printf("Casilla cambiada: %d\n", casilla_Accionada.casilla);

        // Aquí puedes agregar la lógica para actualizar el estado del tablero o realizar alguna acción
    }
}
