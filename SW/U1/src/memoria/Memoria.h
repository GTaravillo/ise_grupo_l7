#ifndef __MEMORIA_H
#define __MEMORIA_H

#include "cmsis_os2.h"

#define TAM_COLA_MSGS_RX 70
#define TAM_COLA_MSGS_TX 70

extern osThreadId_t        e_memoriaThreadId;
extern osMessageQueueId_t  e_memoriaRxMessageId;
extern osMessageQueueId_t  e_memoriaTxMessageId;

typedef enum {
  ESCRIBIR_DATO = 0,
  RETOMAR_ULTIMA_PARTIDA = 1
} ETipoPeticion;

typedef struct {
  ETipoPeticion tipoPeticion;
  uint8_t posicion; // 0-63
  uint8_t dato;     
} MemoriaInMsg_t;

typedef struct {
  uint8_t dato[64];
} MemoriaOutMsg_t;

void MemoriaInitialize(void);

#endif  // __MEMORIA_H