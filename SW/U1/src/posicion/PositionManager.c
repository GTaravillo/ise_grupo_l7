#include "PositionManager.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED_STRIP

#define POWER_ON_RESET  150     // Tiempo (ms) que debe mantenerse en reset al inicializar
#define SLAVE_1_ADDR    0x20    // 0x20 en 7 bits es 0x40 en 8 bits - A0, A1 Y A2 SIN SOLDAR
#define SLAVE_2_ADDR    0x21	  // 0x21 en 7 bits es 0x42 en 8 bits - A0 SOLDADO A VCC
#define SLAVE_3_ADDR    0x22	  // 0x20 en 7 bits es 0x44 en 8 bits - A1 SOLDADO A VCC
#define SLAVE_4_ADDR    0x24	  // 0x20 en 7 bits es 0x48 en 8 bits - A2 SOLDADO A VCC

const uint16_t CASILLAS[16] = {
    0x0100,  // CASILLA_0 desde el expansor
    0x0200,  // CASILLA_1
    0x0400,  // CASILLA_2
    0x0800,  // CASILLA_3
    0x1000,  // CASILLA_4
    0x2000,  // CASILLA_5
    0x4000,  // CASILLA_6
    0x8000,  // CASILLA_7
    0x0001,  // CASILLA_8
    0x0002,  // CASILLA_9
    0x0004,  // CASILLA_10
    0x0008,  // CASILLA_11
    0x0010,  // CASILLA_12
    0x0020,  // CASILLA_13
    0x0040,  // CASILLA_14
    0x0080   // CASILLA_15
};

const static uint8_t g_numeroHallMap[8][8] = {
//  1   2   3   4   5   6   7   8
	{ 0, 15, 16, 31, 32, 47, 48, 63 }, // A 
	{ 1, 14, 17, 30, 33, 46, 49, 62 }, // B
	{ 2, 13, 18, 29, 34, 45, 50, 61 }, // C
	{ 3, 12, 19, 28, 35, 44, 51, 60 }, // D
	{ 4, 11, 20, 27, 36, 43, 52, 59 }, // E
	{ 5, 10, 21, 26, 37, 42, 53, 58 }, // F
	{ 6,  9, 22, 25, 38, 41, 54, 57 }, // G
	{ 7,  8, 23, 24, 39, 40, 55, 56 }  // H
};

// estos buffers guardan la info de cada expansor i2c
uint8_t buff_exp1[2] = {0xFF, 0xFF};				// 1er expansor -  a1, b1, c1, d1, e1, f1, g1, h1  	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																						//  							 a2, b2, c2, d2, e2, f2, g2, h2  	 --  	P17, P16, P15, P14, P13, P12, P11, P10
														
uint8_t buff_exp2[2] = {0xFF, 0xFF};				// 2o expansor  -  a3, b3, c3, d3, e3, f3, g3, h3  	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																						//  							 a4, b4, c4, d4, e4, f4, g4, h4  	 --  	P17, P16, P15, P14, P13, P12, P11, P10
														
uint8_t buff_exp3[2] = {0xFF, 0xFF}; 				// 3o expansor  -  a5, b5, c5, d5, e5, f5, g5, h5 	 --  	P00, P01, P02, P03, P04, P05, P06, P07
																						//  							 a6, b6, c6, d6, e6, f6, g6, h6	   -- 	P17, P16, P15, P14, P13, P12, P11, P10
														
uint8_t buff_exp4[2] = {0xFF, 0xFF}; 				// 4o expansor  -  a7, b7, c7, d7, e7, f7, g7, h7	   -- 	P00, P01, P02, P03, P04, P05, P06, P07
																						//  							 a8, b8, c8, d8, e8, f8, g8, h8	   --  	P17, P16, P15, P14, P13, P12, P11, P10

uint8_t last_buff_exp1[2] = {0xFF, 0xFF};
uint8_t last_buff_exp2[2] = {0xFF, 0xFF};
uint8_t last_buff_exp3[2] = {0xFF, 0xFF};
uint8_t last_buff_exp4[2] = {0xFF, 0xFF};

//Mapeado de 1 expansor:
/*
	P17 - 0xFF,0x7F / P16 - 0xFF,0xBF / P15 - 0xFF,0xDF / P14 - 0xFF,0xEF / 	TODO EL RATO, 1ER BYTE A FF
	P13 - 0xFF,0xF7 / P12 - 0xFF,0xFB / P11 - 0xFF,0xFD / P10 - 0xFF,0xFE
	
	P00 - 0xFE,0xFF / P01 - 0xFD,0xFF / P02 - 0xFB,0xFF / P03 - 0xF7,0xFF /   TODO EL RATO, 2� BYTE A FF
	P04 - 0xEF,0xFF / P05 - 0xDF,0xFF / P06 - 0xBF,0xFF / P07 - 0x7F,0xFF
*/

/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

osThreadId_t e_positionManagerThreadId;
osMessageQueueId_t  e_positionMessageId;
static osTimerId_t tim_id_rebotes;

	
/* Private */
static  void  Run(void *argument);

void I2C_SignalEvent(uint32_t event);

void Pcf8575Initialize(void);

void INTpinsInitialize(void);

void leerExpansor(uint8_t slave_addr, uint8_t *buffer);

void detectarCambiosHall(uint8_t exp_num, uint8_t* buffer_actual, uint8_t* buffer_anterior);

static uint8_t mapPinToHallIndex(int numero_expansor, uint16_t cambios, uint8_t* buffer_actual, bool* ocupada);


/**************************************/
//Inicializaci�n del hilo
void PositionManagerInitialize(void)
{
  e_positionManagerThreadId = osThreadNew(Run, NULL, NULL);
	
	e_positionMessageId = osMessageQueueNew(10, sizeof(ECasilla), NULL);

  if ((e_positionManagerThreadId == NULL))
  {
    // printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_positionManagerThreadId == NULL));
  }
}

static void Run(void *argument)
{
	int32_t status;
  // printf("[posicion::Run] Initializing I2C\n");
  I2Cdrv->Initialize(I2C_SignalEvent);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, NULL);
  //osDelay(1000);
	
	Pcf8575Initialize();
  INTpinsInitialize();
	
  while (1)
  {
		uint32_t flags = osThreadFlagsWait(HALL_DETECTED_1 | HALL_DETECTED_2 | HALL_DETECTED_3 | HALL_DETECTED_4, osFlagsWaitAny, osWaitForever);  // Espera a que haya interrupci�n (cambio de estado de alg�n hall)

		switch(flags){
			case HALL_DETECTED_1:
				leerExpansor(SLAVE_1_ADDR, buff_exp1);
        // printf("LECTURA [0x%02X][0x%02X]\n", buff_exp1[0], buff_exp1[1]);
        detectarCambiosHall(0, buff_exp1, last_buff_exp1);
				break;
			
			case HALL_DETECTED_2:
				leerExpansor(SLAVE_2_ADDR, buff_exp2);
        detectarCambiosHall(1, buff_exp2, last_buff_exp2);
				break;
		
			case HALL_DETECTED_3:
				leerExpansor(SLAVE_3_ADDR, buff_exp3);
        detectarCambiosHall(2, buff_exp3, last_buff_exp3);
				break;
		
			case HALL_DETECTED_4:
				leerExpansor(SLAVE_4_ADDR, buff_exp4);
        detectarCambiosHall(3, buff_exp4, last_buff_exp4);
        osDelay(300);
        //osThreadFlagsClear(HALL_DETECTED_4);
				break;
			
			default:
				break;
		}	
  }
}

void I2C_SignalEvent(uint32_t event){
	//printf("I2C_SignalEvent [%#x]\n", event);
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) 
  {
    /* Transfer or receive is finished */
		osThreadFlagsSet(e_positionManagerThreadId, ARM_I2C_EVENT_TRANSFER_DONE);
  }
}

//Inicializaci�n de los 4 pines que conectados a los INT de cada expansor, 
// sus IRQHandler y callbacks se encuentran en stm32f4xx_it.c (carpeta irq)
void INTpinsInitialize(void){
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	
	GPIO_InitStruct.Pin   = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	// Configura el PC9 -> Expansor 1
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); // Configura el PF12 -> Expansor 2
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); // Configura el PF14 -> Expansor 3
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); // Configura el PF15 -> Expansor 4

	
	//Ajustes de prioridad y habilitaciones de las interrupciones de los 4 pines
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

//Inicializaci�n de los 4 expansores
void Pcf8575Initialize(void){
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
					// printf("Error al enviar datos al expansor %d (addr 0x%02X)\n", i + 1, slaveAddresses[i]);
					continue;
			}
			osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
	}
	// printf("Todos los expansores PCF8575 inicializados correctamente.\n");
	
}

//Funci�n que lee el estado de un expansor, pasada su SLAVE_X_ADDR 
// y lo guarda en su buff_expX[2] correspondiente
void leerExpansor(uint8_t slave_addr, uint8_t *buffer) {
	
	
	int32_t status = I2Cdrv->MasterReceive(slave_addr, buffer, 2, false);
	if (status != ARM_DRIVER_OK) {
			// printf("Error reading from slave 0x%02X\n", slave_addr);
			return;
	}
  
	osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  
  // printf("LECTURA I2C [0x%04X]\n", *buffer);
}


//Funci�n que se encarga de devolver el n�mero de la casilla que se va a enviar al m�dulo de juego
// siguiendo el mapeado g_numeroHallMap[][], mediante los atributos del n�mero del expansor y el resultado
// de la XOR entre la �ltima situcaci�n de los hall y la actual (cambios)
static uint8_t mapPinToHallIndex(int numero_expansor, uint16_t cambios, uint8_t* buffer_actual, bool* ocupada)
{
	//numero_expansor -> del 0 al 3 el n�mero del expansor que ha realizado la interrupci�n
	//cambios -> XOR entre buff_expX y last_buff_expX (compara la �ltima situaci�n de los hall y la actual, 
																														// y entrega los bits que han cambiado)
  int casilla = 0;	
	
	// printf("Cambios: %d \n", cambios);
 
	for(uint8_t i = 0; i < 16; i++){
		if (cambios == CASILLAS[i]){
			casilla = i;
      
      uint16_t actual = buffer_actual[0] << 8 | buffer_actual[1];
      *ocupada = (actual & cambios) == 0;
      // printf("CASILLA[%d] ACTUAL[0x%02X] CAMBIOS[0x%02X] PIEZA %s\n", i, actual, cambios, (actual & cambios) != 0 ? "LEVANTADA" : "OCUPADA");
      break;
		}
	}	
	casilla = (numero_expansor * 16) + casilla;
  
	return casilla;
}

//Funci�n que pasados el exp_num, y los punteros al buffer_actual y al buffer_anterior, detecta en qu� casilla
// se ha levantado o colocado pieza y se la env�a al m�dulo de juego.c
void detectarCambiosHall(uint8_t numero_expansor, uint8_t* buffer_actual, uint8_t* buffer_anterior)
{
	//numero_expansor -> del 1 al 4 el n�mero del expansor que ha realizado la interrupci�n
	//buffer_actual -> buffer con la situaci�n actual de las casillas
	//buffer_anterior -> buffer con la situcaci�n pasada de las casillas

  ECasilla casilla;
	uint16_t cambios = ((buffer_actual[0] << 8)|buffer_actual[1]) ^ ((buffer_anterior[0] << 8)|buffer_anterior[1]); // XOR: bits que cambiaron
  bool ocupada;
  
	if(cambios != 0){
	casilla.casilla = mapPinToHallIndex(numero_expansor, cambios, buffer_actual, &ocupada);
  casilla.ocupada = ocupada;
    
//  if(buffer_actual[0] && cambios == 0){
//    casilla.okupada = 1;
//  } else{
//    casilla.okupada = 0;
//  }
//  
//  if(buffer_actual[1] && cambios == 0){
//    casilla.okupada = 1;
//  } else{
//    casilla.okupada = 0;
//  }
  
  // printf("Buffer actual: %d y %d\n", buffer_actual[0], buffer_actual[1]);

//  LedStripMsg_t mensajeLed;
//  mensajeLed.posicion = casilla;
//  mensajeLed.tipoJugada = ESPECIAL;
	
    // printf("Movimiento en la casilla %d, casilla ocupada: %d\n", casilla.casilla, casilla.ocupada);
	
	//osStatus_t status = osMessageQueuePut(e_ledStripMessageId, &mensajeLed, 0, 0);
	osStatus_t status = osMessageQueuePut(e_positionMessageId, &casilla, 0, 0);

//	
//	if (status != osOK) {
//			printf("Error enviando casilla %d a la cola\n", casilla_Accionada.casilla);
//	}

	// Actualiza estado anterior
	buffer_anterior[0] = buffer_actual[0];
	buffer_anterior[1] = buffer_actual[1];
}
}
