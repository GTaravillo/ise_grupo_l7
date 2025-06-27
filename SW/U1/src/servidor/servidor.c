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
#include PATH_MEMORIA

// Main stack size must be multiple of 8 Bytes
#define SERVER_STK_SZ (1024U)

typedef enum {
  PAGINA_TOP_BAR         = '0',
  PAGINA_NUEVA_PARTIDA   = 'a',
  PAGINA_RETOMAR_PARTIDA = 'b',
  PAGINA_PARTIDA_ACTUAL  = 'c',
  PAGINA_HISTORICO       = 'd'
} EPaginasWeb;

typedef enum {
  TOPBAR_CONSUMO = 'c',
  TOPBAR_FECHA   = 'f',
  TOPBAR_HORA    = 'h'
} EVariablesTopBar;

typedef enum {
  RETOMAR_TIEMPO_BLANCAS = 'b',
  RETOMAR_TIEMPO_NEGRAS  = 'n',
  RETOMAR_TURNO          = 't'
} EVariablesPartidaActual;

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

typedef enum {
  RETOMAR_PARTIDA_TIEMPO_BLANCAS = '1',
  RETOMAR_PARTIDA_TIEMPO_NEGRAS  = '2',
  RETOMAR_PARTIDA_NOMBRE_BLANCAS = 'b',
  RETOMAR_PARTIDA_FECHA          = 'f',
  RETOMAR_PARTIDA_HORA           = 'h',
  RETOMAR_PARTIDA_NOMBRE_NEGRAS  = 'n',
  RETOMAR_PARTIDA_BOTON_REANUDAR = 'r',
  RETOMAR_PARTIDA_TURNO          = 't',
	RETOMAR_PARTIDA_VAR_DESCONOCIDA
} EVariablesRetomarPartida;

// typedef enum {

// } EVariablesHistorico;

/* Public */
osThreadId_t        e_serverThreadId;
osMessageQueueId_t  e_serverInputMessageId;
osMessageQueueId_t  e_serverOutputMessageId;

/* Private */
static uint64_t g_serverStk[SERVER_STK_SZ / 8];
static osSemaphoreId_t  e_consumoSemaphoreId;

// Datos memoria
static bool existePartidaRetomar;
static uint16_t numPartidasFinalizadas;
// Top Bar
static char consumoActual[4] = { '0', '0', '0' };
// Nueva Partida
static char nombreBlancas[TAM_NOMBRE_JUGADOR + 1];
static char nombreNegras[TAM_NOMBRE_JUGADOR + 1];
static bool turnoBlancas;
static uint8_t minutosBlancas;
static uint8_t segundosBlancas;
static uint8_t minutosNegras;
static uint8_t segundosNegras;
static bool btnIniciarPulsado;
static bool btnPausarPulsado;
static bool btnSuspenderPulsado;
static bool btnRendirsePulsado;

static bool btnReanudarPulsado;

static  const osThreadAttr_t g_serverAttr = 
{
  .name       = "Server Thread",
  .stack_mem  = &g_serverStk[0],
  .stack_size = sizeof(g_serverStk)
};

static void Run(void *argument);
//static const char* GetConsumoActual(void);
static uint32_t RellenarVariablesTopBar(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesNuevaPartida(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesRetomarPartida(const char *env, char *buf, uint32_t buflen);
static uint32_t RellenarVariablesHistorico(const char *env, char *buf, uint32_t buflen);
static void ProcesarIniciarPartida(void);
static void ProcesarSuspenderPartida(void);
static void ProcesarRetomarPartida(void);
static void ProcesarFinalizarPartida(void);
static void Error_Handler(void);


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

//void SetConsumoActual(uint8_t consumo)
//{
//  osSemaphoreAcquire(e_consumoSemaphoreId, 0);
//  snprintf(consumoActual, sizeof(consumoActual), "%u", consumo);
//  osSemaphoreRelease(e_consumoSemaphoreId);
//}


//static const char* GetConsumoActual(void)
//{
//  const char* consumo;
//  osSemaphoreAcquire(e_consumoSemaphoreId, 0);
//  consumo = consumoActual;
//  osSemaphoreRelease(e_consumoSemaphoreId);

//  return consumo;
//}


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
  
  // Espera a que la memoria este inicializada y coge datos
  osThreadFlagsWait(FLAG_INIT_COMPLETE, osFlagsWaitAll | osFlagsNoClear , osWaitForever);
  // existePartidaRetomar   = HayPartidaARetomar();
  // numPartidasFinalizadas = ObtenerNumeroPartidasFinalizadas();
}

void netCGI_ProcessData(uint8_t code, const char *data, uint32_t len) {
  char var[40];

  printf("[CGI] Received POST data: %s\n", data);
  btnIniciarPulsado   = 0;
  btnPausarPulsado    = 0;
  btnSuspenderPulsado = 0;
  btnRendirsePulsado  = 0;
  btnReanudarPulsado  = 0;
  do 
  {
    data = netCGI_GetEnvVar(data, var, sizeof(var));

    const bool esNombreBlancas = strncmp(var, "player1Name=", 12) == 0;
    const bool esNombreNegras  = strncmp(var, "player2Name=", 12) == 0;
    const bool esTiempoPartida = strncmp(var, "matchTime=", 10) == 0;
    const bool esBtnIniciar    = strncmp(var, "btnIniciar=", 11) == 0;
    const bool esBtnPausar     = strncmp(var, "btnPausar=", 10) == 0;
    const bool esBtnSuspender  = strncmp(var, "btnSuspender=", 13) == 0;
    const bool esBtnRendirse   = strncmp(var, "btnRendirse=", 12) == 0;

    const EVariablesNuevaPartida varNueva = esNombreBlancas ? NUEVA_PARTIDA_NOMBRE_BLANCAS   :
                                            esNombreNegras  ? NUEVA_PARTIDA_NOMBRE_NEGRAS    :
                                            esTiempoPartida ? NUEVA_PARTIDA_TIEMPO_PARTIDA   :
                                            esBtnIniciar    ? NUEVA_PARTIDA_BOTON_INICIAR    :
                                            esBtnPausar     ? NUEVA_PARTIDA_BOTON_PAUSAR     :
                                            esBtnSuspender  ? NUEVA_PARTIDA_BOTON_SUSPENDER  :
                                            esBtnRendirse   ? NUEVA_PARTIDA_BOTON_RENDIRSE   :
																															NUEVA_PARTIDA_VAR_DESCONOCIDA;
		
		const bool esBtnRetomar = strncmp(var, "btnReanudar=", 11) == 0;
		
		const EVariablesRetomarPartida varRetomar = esBtnRetomar ? RETOMAR_PARTIDA_BOTON_REANUDAR :
                                                               RETOMAR_PARTIDA_VAR_DESCONOCIDA;
    
		if (varNueva != NUEVA_PARTIDA_VAR_DESCONOCIDA)
		{
			switch (varNueva)
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
					minutosBlancas  = atoi(tiempoPartida);
					segundosBlancas = 0;
					minutosNegras  = atoi(tiempoPartida);
					segundosNegras = 0;
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
		}
		
		else if (varRetomar != RETOMAR_PARTIDA_VAR_DESCONOCIDA)
		{
			switch (varRetomar)
			{
				case RETOMAR_PARTIDA_BOTON_REANUDAR:
					btnReanudarPulsado = (atoi(var + 12) == 1);
				break;
				
				default:
				break;
			}	
		}

  } while (data);

  printf("[servidor::%s] Recibido:\n", __func__);
  printf("[servidor::%s] player1Name: %s\n", __func__, nombreBlancas);
  printf("[servidor::%s] player2Name: %s\n", __func__, nombreNegras);
  printf("[servidor::%s] Minutos blancas: %d\n", __func__, minutosBlancas);
	printf("[servidor::%s] Minutos negras: %d\n", __func__, minutosNegras);
  printf("[servidor::%s] Estado botones: INICIAR[%d] PAUSAR[%d] SUSPENDER[%d] RENDIRSE[%d] REANUDAR[%d]\n", 
         __func__, btnIniciarPulsado, btnPausarPulsado, btnSuspenderPulsado, btnRendirsePulsado, btnReanudarPulsado);
  if (btnIniciarPulsado)
  {
    ProcesarIniciarPartida();
  }
  else if (btnPausarPulsado)
  {
    osThreadFlagsSet(e_juegoThreadId, FLAG_PAUSE);
  }
  else if (btnSuspenderPulsado)
  {
    ProcesarSuspenderPartida();
  }
  else if (btnRendirsePulsado)
  {
    ProcesarFinalizarPartida();
  }
  else if (btnReanudarPulsado)
  {
    ProcesarRetomarPartida();
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

    case PAGINA_PARTIDA_ACTUAL:
      //len = RellenarVariablesPartidaActual(xml, buf, buflen);
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
      // const char* consumo = strcat(valorConsumo, "mA");
      // len = snprintf(buf, buflen, xml, consumo); 
      }
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

  // if (!HayPartidaARetomar())
  // {
  //   printf("[servidor::%s] No hay partida a retomar\n", __func__);
  //   return len;
  // }

  MemoriaMsg_t msgTx = {
    .tipoPeticion = RETOMAR_ULTIMA_PARTIDA,
  };

  osMessageQueuePut(e_memoriaRxMessageId, &msgTx, 1, 0);

  MemoriaMsg_t msgRx = {0};
  osMessageQueueGet(e_memoriaTxMessageId, &msgRx, NULL, osWaitForever);

  if (msgRx.tipoPeticion == ERROR_SIN_DATOS)
  {
    printf("[servidor::%s] ERROR!\n", __func__);
    return len;
  }

  char fechaPartida[TAM_FECHA + 1];
  char horaPartida[TAM_HORA + 1];
  char nombreJugador1[TAM_NOMBRE_JUGADOR + 1];
  char nombreJugador2[TAM_NOMBRE_JUGADOR + 1];
  char tiempoBlancas[TAM_TIEMPO_JUGADOR + 1];
  char tiempoNegras[TAM_TIEMPO_JUGADOR + 1];
  uint8_t turno;

  strncpy(fechaPartida, (const char*)msgRx.fechaPartida, TAM_FECHA);
  fechaPartida[TAM_FECHA] = '\0';

  strncpy(horaPartida, (const char*)msgRx.horaPartida, TAM_HORA);
  horaPartida[TAM_HORA] = '\0';

  strncpy(nombreJugador1, (const char*)msgRx.nombreBlancas, TAM_NOMBRE_JUGADOR);
  nombreJugador1[TAM_NOMBRE_JUGADOR] = '\0';

  strncpy(nombreJugador2, (const char*)msgRx.nombreNegras, TAM_NOMBRE_JUGADOR);
  nombreJugador2[TAM_NOMBRE_JUGADOR] = '\0';

  strncpy(tiempoBlancas, (const char*)msgRx.tiempoBlancas, TAM_TIEMPO_JUGADOR);
  tiempoBlancas[TAM_TIEMPO_JUGADOR] = '\0';

  strncpy(tiempoNegras, (const char*)msgRx.tiempoNegras, TAM_TIEMPO_JUGADOR);
  tiempoNegras[TAM_TIEMPO_JUGADOR] = '\0';

  turno = msgRx.turno_victoria;

  const EVariablesRetomarPartida variable = env[0];
  const char* xml = &env[2];

  switch (variable)
  {
    case RETOMAR_PARTIDA_FECHA: {
      //minutosBlancas  = GetMinutosBlancas();
      //segundosBlancas = GetSegundosBlancas();
      //char tiempoBlancas[6];
      //snprintf(tiempoBlancas, sizeof(tiempoBlancas), "%02d:%02d", minutosBlancas, segundosBlancas);
      len = snprintf(buf, buflen, xml, fechaPartida); }
    break;
    
    case RETOMAR_PARTIDA_HORA: {
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      len = snprintf(buf, buflen, xml, horaPartida); }
    break;
    
    case RETOMAR_PARTIDA_NOMBRE_BLANCAS: {
      // const bool turnoActualBlancas = GetTurnoActual();
      // const char* nombreTurno;
      // turnoBlancas = turnoActualBlancas;
      // if (turnoBlancas)
      // {
      //   nombreTurno = "Blancas";
      // }
      // else 
      // {
      //   nombreTurno = "Negras";
      // }
      len = snprintf(buf, buflen, xml, nombreJugador1); }
    break;

    case RETOMAR_PARTIDA_NOMBRE_NEGRAS: {
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      len = snprintf(buf, buflen, xml, nombreJugador2); }
    break;

    case RETOMAR_PARTIDA_TURNO: {
      const char* turnoStr = (turno == 1) ? "Blancas" : "Negras";
      len = snprintf(buf, buflen, xml, turnoStr);
    } break;

    case RETOMAR_PARTIDA_TIEMPO_BLANCAS: {
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      len = snprintf(buf, buflen, xml, tiempoBlancas); }
    break;

    case RETOMAR_PARTIDA_TIEMPO_NEGRAS: {
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      len = snprintf(buf, buflen, xml, tiempoNegras); }
    break;

    case RETOMAR_PARTIDA_BOTON_REANUDAR: //{
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      //len = snprintf(buf, buflen, xml, tiempoNegras); }
    break;

    default:
      buf[0] = '\0';
    break;
  }
	
	return len;
}

static uint32_t RellenarVariablesPartidaActual(const char *env, char *buf, uint32_t buflen)
{
  const EVariablesPartidaActual variable = env[0];
  const char* xml = &env[2];
  uint32_t len = 0;

  switch (variable)
  {
    case RETOMAR_TIEMPO_BLANCAS: //{
      //minutosBlancas  = GetMinutosBlancas();
      //segundosBlancas = GetSegundosBlancas();
      //char tiempoBlancas[6];
      //snprintf(tiempoBlancas, sizeof(tiempoBlancas), "%02d:%02d", minutosBlancas, segundosBlancas);
      //len = snprintf(buf, buflen, xml, tiempoBlancas); }
    break;
    
    case RETOMAR_TIEMPO_NEGRAS: //{
      //minutosNegras  = GetMinutosNegras();
      //segundosNegras = GetSegundosNegras();
      //char tiempoNegras[6];
      //snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", minutosNegras, segundosNegras);
      //len = snprintf(buf, buflen, xml, tiempoNegras); }
    break;
    
    case RETOMAR_TURNO: //{
      // const bool turnoActualBlancas = GetTurnoActual();
      // const char* nombreTurno;
      // turnoBlancas = turnoActualBlancas;
      // if (turnoBlancas)
      // {
      //   nombreTurno = "Blancas";
      // }
      // else 
      // {
      //   nombreTurno = "Negras";
      // }
      // len = snprintf(buf, buflen, xml, nombreTurno); }
    break;

    default:
      buf[0] = '\0';
    break;
  }
	
	return len;
}

static uint32_t RellenarVariablesHistorico(const char *env, char *buf, uint32_t buflen)
{
  uint32_t len = 0;
  
  return len;
}

static void ProcesarIniciarPartida(void)
{
  setTiempoBlancas(minutosBlancas, 0);
  setTiempoNegras(minutosNegras, 0);
  osThreadFlagsSet(e_juegoThreadId, FLAG_START);
}

static void ProcesarSuspenderPartida(void)
{
  MemoriaMsg_t msgTx = {0};

  msgTx.tipoPeticion = GUARDAR_PARTIDA_SIN_FINALIZAR;

  strncpy((char*)msgTx.fechaPartida, GetRtcDate(), TAM_FECHA);
  strncpy((char*)msgTx.horaPartida, GetRtcTime(), TAM_HORA);

  strncpy((char*)msgTx.nombreBlancas, nombreBlancas, TAM_NOMBRE_JUGADOR);
  strncpy((char*)msgTx.nombreNegras, nombreNegras, TAM_NOMBRE_JUGADOR);

  msgTx.turno_victoria = GetTurno();

  char tiempo[TAM_TIEMPO_JUGADOR + 1];
  snprintf(tiempo, sizeof(tiempo), "%02d:%02d", GetMinutosBlancas(), GetSegundosBlancas());
  strncpy((char*)msgTx.tiempoBlancas, tiempo, TAM_TIEMPO_JUGADOR);

  memset(&tiempo, 0, TAM_TIEMPO_JUGADOR);

  snprintf(tiempo, sizeof(tiempo), "%02d:%02d", GetMinutosNegras(), GetSegundosNegras());
  strncpy((char*)msgTx.tiempoNegras, tiempo, TAM_TIEMPO_JUGADOR);

  getMap(msgTx.dato, TAM_DATOS);

  osMessageQueuePut(e_memoriaRxMessageId, &msgTx, 1, 0);
  osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
}

static void ProcesarRetomarPartida(void)
{
  MemoriaMsg_t msgTx = {
    .tipoPeticion = RETOMAR_ULTIMA_PARTIDA,
  };

  osMessageQueuePut(e_memoriaRxMessageId, &msgTx, 1, 0);

  MemoriaMsg_t msgRx = {0};
  osMessageQueueGet(e_memoriaTxMessageId, &msgRx, NULL, osWaitForever);

  if (msgRx.tipoPeticion == ERROR_SIN_DATOS)
  {
    printf("[servidor::%s] ERROR!\n", __func__);
    return;
  }

  int minutos = 0, segundos = 0;
  sscanf((const char*)msgRx.tiempoBlancas, "%2d:%2d", &minutos, &segundos);
  setTiempoBlancas(minutos, segundos);

  sscanf((const char*)msgRx.tiempoNegras, "%2d:%2d", &minutos, &segundos);
  setTiempoNegras(minutos, segundos);

  SetTurno(msgRx.turno_victoria);
  
  setMap(msgRx.dato, TAM_DATOS);

  osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
}

static void ProcesarFinalizarPartida(void)
{
  MemoriaMsg_t msgTx = {0};

  msgTx.tipoPeticion = GUARDAR_PARTIDA_FINALIZADA;

  strncpy((char*)msgTx.fechaPartida, GetRtcDate(), TAM_FECHA);
  strncpy((char*)msgTx.horaPartida, GetRtcTime(), TAM_HORA);

  strncpy((char*)msgTx.nombreBlancas, nombreBlancas, TAM_NOMBRE_JUGADOR);
  strncpy((char*)msgTx.nombreNegras, nombreNegras, TAM_NOMBRE_JUGADOR);

  msgTx.turno_victoria = GetTurno() ? 0 : 1;  // Gana el contrario

  char tiempo[TAM_TIEMPO_JUGADOR + 1];
  snprintf(tiempo, sizeof(tiempo), "%02d:%02d", GetMinutosBlancas(), GetSegundosBlancas());
  strncpy((char*)msgTx.tiempoBlancas, tiempo, TAM_TIEMPO_JUGADOR);

  memset(&tiempo, 0, TAM_TIEMPO_JUGADOR);

  snprintf(tiempo, sizeof(tiempo), "%02d:%02d", GetMinutosNegras(), GetSegundosNegras());
  strncpy((char*)msgTx.tiempoNegras, tiempo, TAM_TIEMPO_JUGADOR);
  
  osMessageQueuePut(e_memoriaRxMessageId, &msgTx, 1, 0);

  osThreadFlagsSet(e_juegoThreadId, FLAG_FINALIZAR);
}

static void Error_Handler(void) 
{
  while (1)
  {
  }
}
