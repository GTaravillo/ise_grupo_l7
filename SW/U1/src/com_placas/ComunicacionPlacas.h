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
#define	SEND_RECEIVE      (RECEIVE_COMPLETE | SEND_COMPLETE)

#define NUMERO_MENSAJES_MAX 10    // Maximum number of messages in queue
#define TAM_MENSAJE_MAX  40    // Maximum message size in bytes

typedef enum
{
  MENSAJE_NFC          = 1,
  MENSAJE_DISTANCIA    = 2,
  MENSAJE_ALIMENTACION = 3,
  MENSAJE_MICROFONO    = 4
} ETipoMensaje;

typedef struct
{
  ETipoMensaje tipoMensaje;
  const char mensaje[TAM_MENSAJE_MAX - 1];
} mensaje_t;

extern osThreadId_t        e_comPlacasRxThreadId;
extern osThreadId_t        e_comPlacasTxThreadId;
extern osMessageQueueId_t  e_comPlacasRxMessageId;
extern osMessageQueueId_t  e_comPlacasTxMessageId;

void ComunicacionPlacasInitialize(void);

#endif  // __COMUNICACION_PLACAS_H
