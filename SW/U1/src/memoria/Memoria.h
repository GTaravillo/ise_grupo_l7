#ifndef __MEMORIA_H
#define __MEMORIA_H

/* ARM */
#include "cmsis_os2.h"
/* Interfaces */
#include "../config/Paths.h"
#include PATH_COMMON

#define TAM_COLA_MSGS_RX 2
#define TAM_COLA_MSGS_TX 2

#define FLAG_INIT_COMPLETE 0x01
#define FLAG_READY 0x02


extern osThreadId_t        e_memoriaThreadId;
extern osMessageQueueId_t  e_memoriaRxMessageId;
extern osMessageQueueId_t  e_memoriaTxMessageId;

typedef enum {
  ERROR_SIN_DATOS               = 0,
  GUARDAR_PARTIDA_SIN_FINALIZAR = 1,
  GUARDAR_PARTIDA_FINALIZADA    = 2,
  RETOMAR_ULTIMA_PARTIDA        = 3,
  OBTENER_INFO_PARTIDAS         = 4,
  LIMPIAR_MEMORIA               = 0xFF   // Cuidado! Elimina todos los datos en memoria
} ETipoPeticion;

typedef struct {
  ETipoPeticion tipoPeticion;
  uint8_t fechaPartida[TAM_FECHA + 1];           // Fecha finalizacion/suspension. Solo numeros (ddmmaa)
  uint8_t horaPartida[TAM_HORA + 1];             // Hora finalizacion/suspension. Solo numeros (hhmmss)
  uint8_t nombreBlancas[TAM_NOMBRE_JUGADOR + 1];
  uint8_t nombreNegras[TAM_NOMBRE_JUGADOR + 1];
  uint8_t turno_victoria;                        // 1 = blancas. Turno si partida sin finalizar. Victoria si finalizada.
  uint8_t tiempoBlancas[TAM_TIEMPO_JUGADOR + 1]; // Tiempo restante blancas. Solo numeros (mmss)
  uint8_t tiempoNegras[TAM_TIEMPO_JUGADOR + 1];  // Tiempo restante negras. Solo numeros (mmss)
  uint8_t dato[TAM_DATOS];                       // 0-63
} MemoriaMsg_t;

// typedef struct {
//   ETipoPeticion tipoPeticion;
//   PartidaInMsg_t msg;   
// } MemoriaInMsg_t;

// typedef struct {
//   uint8_t dato[64];
// } MemoriaOutMsg_t;

// typedef struct {
//   ETipoPeticion tipoPeticion;
//   uint8_t turno;              // 1 = turno actual blancas
//   uint8_t tiempoBlancas[4];   // Tiempo restante blancas. Solo numeros (mmss)
//   uint8_t tiempoNegras[4];    // Tiempo restante negras. Solo numeros (mmss)
//   uint8_t dato[64];           // 0-63
// } InfoOutMsg_t;

// typedef struct {
//   ETipoPeticion tipoPeticion;
//   uint8_t fechaPartida[6];    // Fecha finalizacion/suspension. Solo numeros (ddmmaa)
//   uint8_t horaPartida[6];     // Hora finalizacion/suspension. Solo numeros (hhmmss)
//   uint8_t nombreBlancas[12];
//   uint8_t nombreNegras[12];
//   uint8_t turno;              // 1 = turno actual blancas
//   uint8_t tiempoBlancas[4];   // Tiempo restante blancas. Solo numeros (mmss)
//   uint8_t tiempoNegras[4];    // Tiempo restante negras. Solo numeros (mmss)
//   uint8_t dato[64];           // 0-63
// } InfoInMsg_t;


void MemoriaInitialize(void);
// Solo para servidor
uint16_t ObtenerNumeroPartidasFinalizadas(void);
MemoriaMsg_t ObtenerInfoPartidaFinalizada(uint16_t numPartida);

#endif  // __MEMORIA_H