#ifndef SERVER_H
#define SERVER_H

/* ARM */
#include "cmsis_os2.h"
/* std */
#include <stdint.h>
#include <stdbool.h>


#define SERVER_MAX_MSGS  10

typedef enum {
  LCD_LINE_1 = 0,
  LCD_LINE_2 = 1
} ELcdLine;

// typedef struct {
  
// } serverMsg_t;

extern osThreadId_t        e_serverThreadId;
// extern osMessageQueueId_t  e_serverInputMessageId;
// extern osMessageQueueId_t  e_serverOutputMessageId;

void Server_Initialize(void);
void  SetWebTime(const char* time);
void  SetWebDate(const char* date);


// const char* SetTimeOutput();
// const char* SetDateOutput();

const char* GetLcdTextInput(ELcdLine line);
const char*  GetTime(void);
const char*  GetDate(void);

bool  IsTimeCorrect(const char* time);
bool  IsDateCorrect(const char* date);
// const char* GetTimeInput();
// const char* GetDateInput();
// const char* SetDateInput();
// const char* GetTimeOutput();
// const char* GetDatePutput();


uint16_t ADC_in         (uint32_t ch);
uint8_t  get_button     (void);
void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

#endif
