#ifndef __COMUNICACION_PLACAS_H
#define __COMUNICACION_PLACAS_H

#include "cmsis_os2.h"  
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

#define USART_EVENT (ARM_USART_EVENT_SEND_COMPLETE     | \
                     ARM_USART_EVENT_RECEIVE_COMPLETE  | \
                     ARM_USART_EVENT_TRANSFER_COMPLETE | \
                     ARM_USART_EVENT_TX_COMPLETE)

#define SEND_COMPLETE     0X01
#define RECEIVE_COMPLETE  0x02
#define ERROR_FRAMING     0x80
#define	SEND_RECEIVE      (RECEIVE_COMPLETE | SEND_COMPLETE)
#define TAM_MENSAJE_MAX   2

#define ERROR_FRAMING     0x80

#define NUMERO_MENSAJES_COM_PLACAS_MAX 10    // Maximum number of messages in queue

typedef enum
{
  MENSAJE_LCD          = 1,
  MENSAJE_LED_STRIP    = 2,
  MENSAJE_SERVIDOR     = 3,
  MENSAJE_RTC          = 4,
  MENSAJE_POSICION     = 5,
  MENSAJE_MEMORIA      = 6,
  MENSAJE_JUEGO        = 7,
  MENSAJE_DISTANCIA    = 8,
  MENSAJE_NFC          = 9,
  MENSAJE_ADC					 = 10
} EModulos;

typedef struct
{
  EModulos remitente;
  EModulos destinatario;
  uint8_t mensaje[TAM_MENSAJE_MAX];
} ComPlacasMsg_t;

extern osThreadId_t        e_comPlacasRxThreadId;
extern osThreadId_t        e_comPlacasTxThreadId;
extern osMessageQueueId_t  e_comPlacasRxMessageId;
extern osMessageQueueId_t  e_comPlacasTxMessageId;

void ComunicacionPlacasInitialize(void);

#endif  // __COMUNICACION_PLACAS_H
