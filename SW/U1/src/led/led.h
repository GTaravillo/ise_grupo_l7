#ifndef __LED_H
#define __LED_H
/* ARM */
#include "cmsis_os2.h"
/* std */
#include <stdint.h>
#include <stdbool.h>

#define LED_MAX_MSGS    4  // Maximum number of messages in queue
#define LED_MAX_STATES 10  // Maximum number of different states the leds can take

typedef enum {
	ALL_OFF = 0X00U,
  LD1     = 0x01U,
  LD2     = 0x02U,
  LD3     = 0x04U,
	ALL_ON  = 0X07U
} ELedId;

typedef enum {
  LED_ON       = 1,
  LED_OFF      = 2,
  LED_TOGGLE   = 3,
  LED_SEQUENCE = 4
} ELedMode;

typedef struct {
  ELedId leds;
} ledsOn_t;

typedef struct {
  ELedId leds;
} ledsOff_t;

typedef struct {
  ELedId leds;
} ledsToggle_t;

typedef struct {
	ELedId   ledsStates[LED_MAX_STATES];  // Ej: {LD1, LD2, LD1 | LD2} Enciende primero solo LD1, luego solo LD2 y por ultimo LD1 y LD2
	uint8_t  numStates;                   // En este ejemplo seria 3
	uint16_t timeBetweenStates_ms;        // Tiempo [ms] entre los estados. Ej: 500 -> tras medio segundo se apagaria LD1 y se encenderia LD2
	uint16_t timeToReload_ms;             // Tiempo [ms] entre el ultimo estado y el primero
} ledSequence_t;

typedef struct {
  ELedMode      mode;
  ledsOn_t      ledsOn;
  ledsOff_t     ledsOff;
  ledsToggle_t  ledsToggle;
	ledSequence_t ledSequence;
} ledMessage_t;

extern  osThreadId_t        e_ledThreadId;
extern  osMessageQueueId_t  e_ledInputMessageId;
extern  bool                e_playingSequence;

void LED_Initialize(void);

#endif /* __LED_H */
