#include "server.h"
/* ARM */
#include "stm32f4xx_hal.h"
#include "rl_net.h"
/* std */
#include <stdio.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_MAIN
#include PATH_RTC

// Main stack size must be multiple of 8 Bytes
#define SERVER_STK_SZ (1024U)

typedef enum
{
  PAGINA_PRINCIPAL       = '0',
  PAGINA_NUEVA_PARTIDA   = '1',
  PAGINA_RETOMAR_PARTIDA = '2',
  PAGINA_LEADERBOARD     = '3',
  PAGINA_HISTORICO       = '4'
} EPaginaWeb;

typedef enum {
  // Global
  HORA                = 'a',
  FECHA               = 'b',
  CONSUMO             = 'c',
  // Nueva partida
  NOMBRE_BLANCAS      = 'd',
  NOMBRE_NEGRAS       = 'e',
  TIEMPO_PARTIDA      = 'f',
  INCREMENTO_CHECKBOX = 'g',
  TIEMPO_INCREMENTO   = 'h',
  AYUDA               = 'v',
  INICIAR             = 'w',
} EVariableIn;

typedef enum {
  // Global
  HORA                = 'a',
  FECHA               = 'b',
  CONSUMO             = 'c',
  // Nueva partida
  NOMBRE_BLANCAS      = 'd',
  NOMBRE_NEGRAS       = 'e',
  TIEMPO_PARTIDA      = 'f',
  INCREMENTO_CHECKBOX = 'g',
  TIEMPO_INCREMENTO   = 'h',
  AYUDA               = 'v',
  INICIAR             = 'w',
} EVariableOut;

/* Public */
osThreadId_t        e_serverThreadId;
osMessageQueueId_t  e_serverInputMessageId;
osMessageQueueId_t  e_serverOutputMessageId;

static uint8_t lastShowDate = 0;
/* Private */
static  uint64_t g_serverStk[SERVER_STK_SZ / 8];

static  const osThreadAttr_t g_serverAttr = 
{
  .name       = "Server Thread",
  .stack_mem  = &g_serverStk[0],
  .stack_size = sizeof(g_serverStk)
};

static  void  Run(void *argument);
static  void  LoadDefaultValues(void);
static  void  Error_Handler(void);


void Server_Initialize(void) 
{
  e_serverThreadId        = osThreadNew(Run, NULL, &g_serverAttr);
//  e_serverInputMessageId  = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);
//  e_serverOutputMessageId = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);

  if ((e_serverThreadId == NULL) /*|| (e_serverInputMessageId == NULL) || (e_serverOutputMessageId == NULL)*/) 
  {
    printf("[server::%s] osThreadNew ERROR!\n", __func__);
    
    Error_Handler();
  }
}

static void Run(void *argument) 
{
  switch (netInitialize())
  {
    case netOK:
      printf("[servidor::Run] netInitialize: Operation succeeded\n");
      break;

    case netBusy:
      printf("[servidor::Run] netInitialize: Process is busy\n");
      break;

    case netError:
      printf("[servidor::Run] netInitialize: Unspecified error\n");
      break;

    case netInvalidParameter:
      printf("[servidor::Run] netInitialize: Invalid parameter specified\n");
      break;

    case netWrongState:
      printf("[servidor::Run] netInitialize: Wrong state error\n");
      break;

    case netDriverError:
      printf("[servidor::Run] netInitialize: Driver error\n");
      break;

    case netServerError:
      printf("[servidor::Run] netInitialize: Server error\n");
      break;

    case netAuthenticationFailed:
      printf("[servidor::Run] netInitialize: User authentication failed\n");
      break;

    case netDnsResolverError:
      printf("[servidor::Run] netInitialize: DNS host resolver failed\n");
      break;

    case netFileError:
      printf("[servidor::Run] netInitialize: File not found or file r/w error\n");
      break;

    case netTimeout:
      printf("[servidor::Run] netInitialize: Operation timeout\n");
      break;

    default:
      break;
  }

  while (!netHTTPs_Running())
  {
    printf("[servidor::Run] HTTP server aun no esta corriendo\n");
    osDelay(100);
  }

  printf("[servidor::Run] HTTP server inicializado correctamente\n");
  //osThreadFlagsSet(e_principalThreadId, SEND_COMPLETE);
  
}

void netCGI_ProcessData(uint8_t code, const char *data, uint32_t len) {
  lastShowDate = 0;
  data = netCGI_GetEnvVar (data, var, sizeof (var));

  if (strcmp (var, "pg=lcd") == 0) 
    {
      GetLcdInput(data);
    }
}

uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
	/*
		Ejemplo:
		lcd.cgi -> c f 1 <td><input type=text name=lcd1 size=20 maxlength=21 value="%s"></td></tr>
		*env    ->   f 1 <td><input type=text name=lcd1 size=20 maxlength=21 value="%s"></td></tr>
	*/
	static uint32_t adv;
	
  uint32_t len = 0U;

	EVariableIn scriptId = env[0];

  switch (scriptId) {
    case NOMBRE_BLANCAS:
			// LCD Module control from 'lcd.cgi'
			len = HandleLcdScript(env, buf);
      break;
  }
  return len;
}

void GetLcdInput(const char *data) {
  char var[40];
  lcdMessage_t lcdMsg;
  
  do 
  {
    data = netCGI_GetEnvVar (data, var, sizeof(var));
		
    if (data == NULL) { break; }
    printf("[server_CGI::%s] var [%s]\n", __func__, var);
    
		
    if (var[0] == NULL) { continue; }
		printf("[server_CGI::%s] var[0] = %c\n", __func__, var[0]);
    
		
		const bool isLine1 = (strncmp(var, "lcd1=", 5) == 0);
		const bool isLine2 = (strncmp(var, "lcd2=", 5) == 0);
		
		lcdMsg.mode = PRINT_NORMAL;
		char stringToPrint[LCD_STR_MAX_LEN];
		
		if (isLine1)
		{
			char* lcdInputLine1 = var+5;
			lcdMsg.printMsg.printLine = PRINT_LINE_1;
			
			strcpy(stringToPrint, lcdInputLine1);
			strncpy(lcdMsg.printMsg.msg, stringToPrint, LCD_STR_MAX_LEN - 1);
			
			osStatus_t osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
		}
		else if (isLine2)
		{
			const char* lcdInputLine2 = var+5;
			lcdMsg.printMsg.printLine = PRINT_LINE_2;
			
			strcpy(stringToPrint, lcdInputLine2);
			strncpy(lcdMsg.printMsg.msg, stringToPrint, LCD_STR_MAX_LEN - 1);
			
			osStatus_t osStatus = osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 2, 0);
		}
  }
  while (data);
}

static void Error_Handler(void) 
{
  while (1)
  {
  }
}
