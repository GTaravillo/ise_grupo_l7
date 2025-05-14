#ifndef __MEMORIA_H
#define __MEMORIA_H

#include "cmsis_os2.h"

extern osThreadId_t        e_memoriaThreadId;
extern osMessageQueueId_t  e_memoriaMessageId;  // Cola recepción datos

void MemoriaInitialize(void);

#endif  // __MEMORIA_H