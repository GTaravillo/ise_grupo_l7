#include "server.h"
/* ARM */
#include "stm32f4xx_hal.h"
#include "rl_net.h"
/* std */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_MAIN
#include PATH_RTC
#include PATH_COMMON
#include PATH_COM_PLACAS

// Main stack size must be multiple of 8 Bytes
#define SERVER_STK_SZ (1024U)

/* Public */
osThreadId_t        e_serverThreadId;
osMessageQueueId_t  e_serverInputMessageId;
osMessageQueueId_t  e_serverOutputMessageId;

/* Private */
static  uint64_t g_serverStk[SERVER_STK_SZ / 8];

// Nueva Partida
static uint8_t matchTime[5 + 1];     // mm:ss [5, 90]
static uint8_t incrementTime[2 + 1]; // ss    [2, 30]
static bool incrementEnabled; 

static  const osThreadAttr_t g_serverAttr = 
{
  .name       = "Server Thread",
  .stack_mem  = &g_serverStk[0],
  .stack_size = sizeof(g_serverStk)
};

static  void  Run(void *argument);
static  void  Error_Handler(void);


void Server_Initialize(void) 
{
  e_serverThreadId = osThreadNew(Run, NULL, &g_serverAttr);
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
  char var[40];

  char player1Name[TAM_NOMBRE_JUGADOR + 1] = {0};
  char player2Name[TAM_NOMBRE_JUGADOR + 1] = {0};
  char matchTimeStr[2 + 1] = {0};        // [5, 90]
  char incrementTimeStr[2 + 1] = {0};    // [2, 30]
  char incrementEnabledStr[5 + 1] = {0}; // true/false
  char ayudaStr[5 + 1] = {0};            // true/false

  printf("[CGI] Received POST data: %s\n", data);

  do 
  {
    data = netCGI_GetEnvVar(data, var, sizeof(var));
    if (strncmp(var, "player1Name=", 12) == 0) {
      strncpy(player1Name, var + 12, sizeof(player1Name) - 1);
    } else if (strncmp(var, "player2Name=", 12) == 0) {
      strncpy(player2Name, var + 12, sizeof(player2Name) - 1);
    } else if (strncmp(var, "matchTime=", 9) == 0) {
      strncpy(matchTimeStr, var + 9, sizeof(matchTimeStr) - 1);
    } else if (strncmp(var, "incrementTime=", 14) == 0) {
      strncpy(incrementTimeStr, var + 14, sizeof(incrementTimeStr) - 1);
    } else if (strncmp(var, "incrementEnabled=", 16) == 0) {
      strncpy(incrementEnabledStr, var + 16, sizeof(incrementEnabledStr) - 1);
      bool enabled = (strcmp(incrementEnabledStr, "true") == 0);
      incrementEnabled = enabled;
    } else if (strncmp(var, "ayuda=", 6) == 0) {
      strncpy(ayudaStr, var + 6, sizeof(ayudaStr) - 1);
      
    }
  } while (data);

  uint8_t matchTime     = atoi(matchTimeStr);
  uint8_t incrementTime = atoi(incrementTimeStr);
  bool ayuda            = (strcmp(ayudaStr, "true") == 0);

  printf("[CGI] player1Name: %s\n", player1Name);
  printf("[CGI] player2Name: %s\n", player2Name);
  printf("[CGI] matchTime: %d\n", matchTime);
  printf("[CGI] incrementTime: %d\n", incrementTime);
  printf("[CGI] incrementEnabled: %d\n", incrementEnabled);
}

uint32_t netCGI_Script(const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  uint32_t len = 0U;
  printf("[CGI] netCGI_Script called with env: %s\n", env);

  const char* fmt = &env[1]; // format string starts after the code

  switch (env[0]) {
    case 'a': { // currentDate.cgx
      const char* dateStr = GetRtcDate();
      len = (uint32_t)sprintf(buf, fmt, dateStr);
      printf("[CGI] Returning date: %s\n", dateStr);
    break; }

    case 'b': { // currentTime.cgx
      const char* timeStr = GetRtcTime();
      len = (uint32_t)sprintf(buf, fmt, timeStr);
      printf("[CGI] Returning time: %s\n", timeStr);
    break; }

    case 'c': { // currentConsumo.cgx
      len = (uint32_t)sprintf(buf, fmt, "500mA");
      printf("[CGI] Returning consumo: 500mA\n");
    break; }

    default:
      printf("[CGI] Unknown script code: %c\n", env[0]);
      buf[0] = '\0';
      len = 0;
    break; 
  }

  printf("[CGI] Response buffer: %s\n", buf);
  return len;
}

static void Error_Handler(void) 
{
  while (1)
  {
  }
}
