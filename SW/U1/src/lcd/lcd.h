#ifndef LCD_H
#define LCD_H
/* ARM */
#include "cmsis_os2.h"

/* mbed LCD */
#define LCD_ROWS    32
#define LCD_COLUMNS 128
/* Config */
#define LCD_MAX_MSGS      4  // Maximum number of messages in queue
#define LCD_STR_MAX_LEN  22

typedef enum {
  PRINT_NORMAL = 0x01,
  PRINT_COLUMN = 0x02,
  MOVE         = 0x04
} ELcdMode;

// Dirección a la que mover los caracteres
typedef enum {
  UP    = 0x01,
  RIGHT = 0x02,
  DOWN  = 0x04,
  LEFT  = 0x08
} EMoveDirection;

// Linea en la que escribir los caracteres
typedef enum {
  PRINT_LINE_1 = 0X01,
  PRINT_LINE_2 = 0x02
} EPrintLine;

typedef struct {
  EPrintLine printLine;
  char       msg[LCD_STR_MAX_LEN];
} printableMsg_t;

typedef struct {
  uint16_t column;
  uint32_t pattern;
} columnPrintMsg_t;

typedef struct {
  EMoveDirection direction;
} moveLines_t;

typedef struct {
  ELcdMode         mode;
  printableMsg_t   printMsg;
  columnPrintMsg_t columnPrint;
  moveLines_t      moveLines;
} lcdMessage_t;

extern osThreadId_t        e_lcdThreadId;
extern osMessageQueueId_t  e_lcdInputMessageId;

void  LCD_Initialize(void);

#endif
