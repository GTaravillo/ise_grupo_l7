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
#include PATH_JUEGO

// Main stack size must be multiple of 8 Bytes
#define SERVER_STK_SZ (1024U)

typedef enum {
  PAGINA_TOP_BAR         = '0',
  PAGINA_NUEVA_PARTIDA   = 'a',
  PAGINA_RETOMAR_PARTIDA = 'b',
  PAGINA_HISTORICO       = 'c'
} EPaginasWeb;

typedef enum {
  TOPBAR_CONSUMO = 'c',
  TOPBAR_FECHA   = 'f',
  TOPBAR_HORA    = 'h'
} EVariablesTopBar;

typedef enum {
  NUEVA_PARTIDA_NOMBRE_BLANCAS = 0,
  NUEVA_PARTIDA_NOMBRE_NEGRAS,
  NUEVA_PARTIDA_TIEMPO_PARTIDA,
  NUEVA_PARTIDA_BOTON_INICIAR,
  NUEVA_PARTIDA_BOTON_PAUSAR,
  NUEVA_PARTIDA_BOTON_SUSPENDER,
  NUEVA_PARTIDA_BOTON_RENDIRSE,
  NUEVA_PARTIDA_VAR_DESCONOCIDA
} EVariablesNuevaPartida;

// typedef enum {

// } EVariablesHistorico;

/* Public */
osThreadId_t        e_serverThreadId;
osMessageQueueId_t  e_serverInputMessageId;
osMessageQueueId_t  e_serverOutputMessageId;

/* Private */
static uint64_t g_serverStk[SERVER_STK_SZ / 8];
static osSemaphoreId_t  e_consumoSemaphoreId;

// Top Bar
static char consumoActual[3] = { '0', '0', '0' };
// Nueva Partida
// Example: At the top of your servidor.c
static char nombreBlancas[TAM_NOMBRE_JUGADOR + 1];
static char nombreNegras[TAM_NOMBRE_JUGADOR + 1];
static uint8_t minutosPartida;
static uint8_t segundosPartida;
static bool btnIniciarPulsado;
static bool btnPausarPulsado;
static bool btnSuspenderPulsado;
static bool btnRendirsePulsado;

static  const osThreadAttr_t g_serverAttr = 
{
  .name       = "Server Thread",
  .stack_mem  = &g_serverStk[0],
  .stack_size = sizeof(g_serverStk)
};

static void Run(void *argument);
static const char* GetConsumoActual(void);
static uint32_t RellenarVariablesTopBar(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesNuevaPartida(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesRetomarPartida(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesHistorico(const char *env, char *buf, uint32_t buflen);
static  void  Error_Handler(void);


void Server_Initialize(void) 
{
  e_serverThreadId = osThreadNew(Run, NULL, &g_serverAttr);
//  e_serverInputMessageId  = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);
//  e_serverOutputMessageId = osMessageQueueNew(SERVER_MAX_MSGS, sizeof(serverMsg_t), NULL);
  e_consumoSemaphoreId = osSemaphoreNew(1, 1, NULL);

  if ((e_serverThreadId == NULL) /*|| (e_serverInputMessageId == NULL) || (e_serverOutputMessageId == NULL)*/) 
  {
    printf("[server::%s] osThreadNew ERROR!\n", __func__);
    
    Error_Handler();
  }
}

void SetConsumoActual(const char* consumo)
{
  osSemaphoreAcquire(e_consumoSemaphoreId, 0);
	strncmp(consumoActual, consumo, sizeof(consumoActual));
  osSemaphoreRelease(e_consumoSemaphoreId);
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

static const char* GetConsumoActual(void)
{
  const char* consumo;
  osSemaphoreAcquire(e_consumoSemaphoreId, 0);
  consumo = consumoActual;
  osSemaphoreRelease(e_consumoSemaphoreId);

  return consumo;
}

void netCGI_ProcessData(uint8_t code, const char *data, uint32_t len) {
  char var[40];

  printf("[CGI] Received POST data: %s\n", data);

  do 
  {
    data = netCGI_GetEnvVar(data, var, sizeof(var));

    const bool esNombreBlancas = strncmp(var, "player1Name=", 12) == 0;
    const bool esNombreNegras  = strncmp(var, "player2Name=", 12) == 0;
    const bool esTiempoPartida = strncmp(var, "matchTime=", 9) == 0;
    const bool esBtnIniciar    = strncmp(var, "btnIniciar=", 11) == 0;
    const bool esbtnPausar     = strncmp(var, "btnPausar=", 10) == 0;
    const bool esbtnSuspender  = strncmp(var, "btnSuspender=", 13) == 0;
    const bool esbtnRendirse   = strncmp(var, "btnRendirse=", 12) == 0;

    const EVariablesNuevaPartida variable = esNombreBlancas ? NUEVA_PARTIDA_NOMBRE_BLANCAS  :
                                            esNombreNegras  ? NUEVA_PARTIDA_NOMBRE_NEGRAS   :
                                            esTiempoPartida ? NUEVA_PARTIDA_TIEMPO_PARTIDA  :
                                            esBtnIniciar    ? NUEVA_PARTIDA_BOTON_INICIAR   :
                                            esbtnPausar     ? NUEVA_PARTIDA_BOTON_PAUSAR    :
                                            esbtnSuspender  ? NUEVA_PARTIDA_BOTON_SUSPENDER :
                                            esbtnRendirse   ? NUEVA_PARTIDA_BOTON_RENDIRSE  :
                                                              NUEVA_PARTIDA_VAR_DESCONOCIDA;
    
    switch (variable)
    {
      case NUEVA_PARTIDA_NOMBRE_BLANCAS: {
        const char* varId = "player1Name=";
        strncpy(nombreBlancas, var + strlen(varId), sizeof(nombreBlancas) - 1);
			}
      break;

      case NUEVA_PARTIDA_NOMBRE_NEGRAS: {
        const char* varId = "player2Name=";
        strncpy(nombreNegras, var + strlen(varId), sizeof(nombreNegras) - 1);
      }
      break;
        
      case NUEVA_PARTIDA_TIEMPO_PARTIDA: {
        const char* varId = "matchTime=";
        char tiempoPartida[3];
        strncpy(tiempoPartida, var + strlen(varId), sizeof(tiempoPartida) - 1);
        minutosPartida  = atoi(tiempoPartida);
        segundosPartida = 0;
      }
      break;

      case NUEVA_PARTIDA_BOTON_INICIAR: {
        btnIniciarPulsado = (strcmp(var + 11, "1") == 0);
      }
      break;

      case NUEVA_PARTIDA_BOTON_PAUSAR: {
        btnPausarPulsado = (strcmp(var + 10, "1") == 0);
      }
      break;

      case NUEVA_PARTIDA_BOTON_SUSPENDER: {
        btnSuspenderPulsado = (strcmp(var + 13, "1") == 0);
      }
      break;

      case NUEVA_PARTIDA_BOTON_RENDIRSE: {
        btnRendirsePulsado = (strcmp(var + 12, "1") == 0);
      }
      break;
      
      default:
      break;
    }
                              
    // data = netCGI_GetEnvVar(data, var, sizeof(var));
    // if (strncmp(var, "player1Name=", 12) == 0) {
    //   strncpy(nombreBlancas, var + 12, sizeof(nombreBlancas) - 1);
    // } else if (strncmp(var, "player2Name=", 12) == 0) {
    //   strncpy(nombreNegras, var + 12, sizeof(nombreNegras) - 1);
    // } else if (strncmp(var, "matchTime=", 9) == 0) {
    //   strncpy(tiempo, var + 9, sizeof(matchTimeStr) - 1);
    // } else if (strncmp(var, "incrementTime=", 14) == 0) {
    //   strncpy(incrementTimeStr, var + 14, sizeof(incrementTimeStr) - 1);
    // } else if (strncmp(var, "incrementEnabled=", 16) == 0) {
    //   strncpy(incrementEnabledStr, var + 16, sizeof(incrementEnabledStr) - 1);
    //   bool enabled = (strcmp(incrementEnabledStr, "true") == 0);
    //   incrementEnabled = enabled;
    // } else if (strncmp(var, "ayuda=", 6) == 0) {
    //   strncpy(ayudaStr, var + 6, sizeof(ayudaStr) - 1);
      
    // }
  } while (data);

  printf("[servidor::%s] Recibido:\n", __func__);
  printf("[servidor::%s] player1Name: %s\n", __func__, nombreBlancas);
  printf("[servidor::%s] player2Name: %s\n", __func__, nombreNegras);
  printf("[servidor::%s] Minutos partida: %d\n", __func__, minutosPartida);
  printf("[servidor::%s] Estado botones: INICIAR[%d] PAUSAR[%d] SUSPENDER[%d] RENDIRSE[%d]\n", 
         __func__, btnIniciarPulsado, btnPausarPulsado, btnSuspenderPulsado, btnRendirsePulsado);
  if (btnIniciarPulsado)
  {
    osThreadFlagsSet(e_juegoThreadId, FLAG_START);
  }
}

uint32_t netCGI_Script(const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  uint32_t len = 0U;
  printf("[CGI] netCGI_Script called with env: %s\n", env);

  // const char* fmt = &env[1];

  const EPaginasWeb pagina = env[0];
  const char* xml = &env[2];

  switch (pagina) {
    case PAGINA_TOP_BAR:
      len = RellenarVariablesTopBar(xml, buf, buflen);
    break;

    case PAGINA_NUEVA_PARTIDA:
      len = RellenarVariablesNuevaPartida(xml, buf, buflen);
    break;

    case PAGINA_RETOMAR_PARTIDA:
      len = RellenarVariablesRetomarPartida(xml, buf, buflen);
    break;

    case PAGINA_HISTORICO:
      len = RellenarVariablesHistorico(xml, buf, buflen);
    break;


    // case 'a': { // currentDate.cgx
    //   const char* dateStr = GetRtcDate();
    //   len = (uint32_t)sprintf(buf, fmt, dateStr);
    //   printf("[CGI] Returning date: %s\n", dateStr);
    // break; }

    // case 'b': { // currentTime.cgx
    //   const char* timeStr = GetRtcTime();
    //   len = (uint32_t)sprintf(buf, fmt, timeStr);
    //   printf("[CGI] Returning time: %s\n", timeStr);
    // break; }

    // case 'c': { // currentConsumo.cgx
    //   len = (uint32_t)sprintf(buf, fmt, "500mA");
    //   printf("[CGI] Returning consumo: 500mA\n");
    // break; }

    default:
      printf("[CGI] Unknown script code: %c\n", env[0]);
      buf[0] = '\0';
    break; 
  }

  printf("[CGI] Response buffer: %s\n", buf);
  return len;
}

static uint32_t RellenarVariablesTopBar(const char *env, char *buf, uint32_t buflen)
{
  const EVariablesTopBar variable = env[0];
  const char* xml = &env[2];
  uint32_t len = 0;

  switch (variable)
  {
    case TOPBAR_CONSUMO: {
      char valorConsumo[6];
      sprintf(valorConsumo, "%d", atoi(GetConsumoActual()));
      const char* consumo = strcat(valorConsumo, "mA");
      len = snprintf(buf, buflen, xml, consumo); }
    break;
    
    case TOPBAR_FECHA:
      len = snprintf(buf, buflen, xml, GetRtcDate());
    break;
    
    case TOPBAR_HORA:
      len = snprintf(buf, buflen, xml, GetRtcTime());
    break;

    default:
      buf[0] = '\0';
    break;
  }
	
	return len;
}

static uint32_t RellenarVariablesNuevaPartida(const char *env, char *buf, uint32_t buflen)
{
  uint32_t len = 0;

  return len;
}

static uint32_t RellenarVariablesRetomarPartida(const char *env, char *buf, uint32_t buflen)
{
  uint32_t len = 0;
  
  return len;
}

static uint32_t RellenarVariablesHistorico(const char *env, char *buf, uint32_t buflen)
{
  uint32_t len = 0;
  
  return len;
}

static void Error_Handler(void) 
{
  while (1)
  {
  }
}
