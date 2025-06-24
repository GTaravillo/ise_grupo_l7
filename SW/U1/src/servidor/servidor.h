#ifndef SERVIDOR_H
#define SERVIDOR_H

/* ARM */
#include "cmsis_os2.h"
/* std */
#include <stdint.h>
#include <stdbool.h>

#define SERVER_MAX_MSGS  10

typedef enum {
  SERVER_RUNNING = 0x01
} EServerFlags;

extern osThreadId_t        e_serverThreadId;
extern osMessageQueueId_t  e_serverInputMessageId;
extern osMessageQueueId_t  e_serverOutputMessageId;

void Server_Initialize(void);
void SetConsumoActual(uint8_t consumo);

#endif
