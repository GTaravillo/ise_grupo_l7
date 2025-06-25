#include "Memoria.h"
#include "TestMemoria.h"

#include <string.h>
#include <stdbool.h>

#include "../config/Paths.h"
#include PATH_COMMON
#include PATH_TABLERO
#include PATH_JUEGO

// Poner a 1 para realizar test
#define TEST_RETOMAR_PARTIDA_1 1
#define TEST_RETOMAR_PARTIDA_2 0
#define TEST_GUARDAR_PARTIDA_FINALIZADA_1 0
#define TEST_GUARDAR_PARTIDA_FINALIZADA_2 0

osThreadId_t e_testMemoriaThreadId;

static const uint8_t fecha1[TAM_FECHA] = "01/01/01";
static const uint8_t fecha2[TAM_FECHA] = "16/06/25";

static const uint8_t hora1[TAM_HORA] = "12:00:00";
static const uint8_t hora2[TAM_HORA] = "12:34:56";

static const uint8_t nombreBlancas1[TAM_NOMBRE_JUGADOR] = "Test Blancas";
static const uint8_t nombreBlancas2[TAM_NOMBRE_JUGADOR] = "Obi-Wan";

static const uint8_t nombreNegras1[TAM_NOMBRE_JUGADOR] = "Test Negras";
static const uint8_t nombreNegras2[TAM_NOMBRE_JUGADOR] = "x_Shurmano_x";

static const uint8_t turno_victoria1 = 1;
static const uint8_t turno_victoria2 = 0;

static const uint8_t tiempoBlancas1[TAM_TIEMPO_JUGADOR] = "10:00";
static const uint8_t tiempoBlancas2[TAM_TIEMPO_JUGADOR] = "03:49";

static const uint8_t tiempoNegras1[TAM_TIEMPO_JUGADOR] = "12:34";
static const uint8_t tiempoNegras2[TAM_TIEMPO_JUGADOR] = "43:21";

static const uint8_t tablero1[TAM_DATOS] = {(BLANCO << 4) | (TORRE1 - 1), 		
																						(BLANCO << 4) | (CABALLO1 - 1),
																						(BLANCO << 4) | (ALFIL1 - 1),
																						(BLANCO << 4) | (DAMA - 1),
																						(BLANCO << 4) | (REY - 1),
																						(BLANCO << 4) | (ALFIL2 - 1),
																						(BLANCO << 4) | (CABALLO2 - 1),
																						(BLANCO << 4) | (TORRE2 - 1),
																						(BLANCO << 4) | (PEON8 - 1),
																						(BLANCO << 4) | (PEON7 - 1),
																						(BLANCO << 4) | (PEON6 - 1),
																						(BLANCO << 4) | (PEON5 - 1),
																						(BLANCO << 4) | (PEON4 - 1),
																						(BLANCO << 4) | (PEON3 - 1),
																						(BLANCO << 4) | (PEON2 - 1),
																						(BLANCO << 4) | (PEON1 - 1),
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						NONE - 1,
																						(NEGRO << 4) | (PEON1 - 1),
																						(NEGRO << 4) | (PEON2 - 1),
																						(NEGRO << 4) | (PEON3 - 1),
																						(NEGRO << 4) | (PEON4 - 1),
																						(NEGRO << 4) | (PEON5 - 1),
																						(NEGRO << 4) | (PEON6 - 1),
																						(NEGRO << 4) | (PEON7 - 1),
																						(NEGRO << 4) | (PEON8 - 1),
																						(NEGRO << 4) | (TORRE2 - 1),
																						(NEGRO << 4) | (CABALLO2 - 1),
																						(NEGRO << 4) | (ALFIL2 - 1),
																						(NEGRO << 4) | (REY - 1),
																						(NEGRO << 4) | (DAMA - 1),
																						(NEGRO << 4) | (ALFIL1 - 1),
																						(NEGRO << 4) | (CABALLO1 - 1),
																						(NEGRO << 4) | (TORRE1 - 1) };

static const uint8_t tablero2[TAM_DATOS] = {NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						BLANCO | TORRE1, 		
																						BLANCO | CABALLO1,
																						BLANCO | ALFIL1,
																						BLANCO | DAMA,
																						BLANCO | REY,
																						BLANCO | ALFIL2,
																						BLANCO | CABALLO2,
																						BLANCO | TORRE2,
																						BLANCO | PEON8,
																						BLANCO | PEON7,
																						BLANCO | PEON6,
																						BLANCO | PEON5,
																						BLANCO | PEON4,
																						BLANCO | PEON3,
																						BLANCO | PEON2,
																						BLANCO | PEON1,
																						NEGRO | PEON1,
																						NEGRO | PEON2,
																						NEGRO | PEON3,
																						NEGRO | PEON4,
																						NEGRO | PEON5,
																						NEGRO | PEON6,
																						NEGRO | PEON7,
																						NEGRO | PEON8,
																						NEGRO | TORRE2,
																						NEGRO | CABALLO2,
																						NEGRO | ALFIL2,
																						NEGRO | REY,
																						NEGRO | DAMA,
																						NEGRO | ALFIL1,
																						NEGRO | CABALLO1,
																						NEGRO | TORRE1,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE};

static void Run(void *argument);
static void TestGuardarPartidaSinFinalizar(int numTest);
static void TestGuardarPartidaFinalizada(int numTest);
static void TestRetomarUltimaPartida();
static void TestLimpiarMemoria();
static bool ConfirmarDatosRecibidos(int numTest, MemoriaMsg_t mensajeRx);
static bool ConfirmarDatosJuego(MemoriaMsg_t mensajeRx);

void TestMemoriaInitialize(void)
{
  e_testMemoriaThreadId = osThreadNew(Run, NULL, NULL);

  if (e_testMemoriaThreadId == NULL)
  {
    printf("[TestMemoria::%s] ERROR! osThreadNew [%d]\n", __func__, (e_testMemoriaThreadId == NULL));
  }
}

static void Run(void *argument)
{
  osStatus_t status;
  printf("[TestMemoria::%s] Esperando a que modulo memoria termine de inicializarse\n", __func__);
	uint32_t flag = osThreadFlagsWait(FLAG_INIT_COMPLETE, osFlagsWaitAll, osWaitForever);
  printf("[TestMemoria::%s] Modulo memoria inicializado\n", __func__);
  
  MemoriaMsg_t mensajeRx = {0};
  #if TEST_RETOMAR_PARTIDA_1 == 1
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
    TestGuardarPartidaSinFinalizar(1);
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
		TestRetomarUltimaPartida();
    status = osMessageQueueGet(e_memoriaTxMessageId, &mensajeRx, NULL, osWaitForever);
    if (ConfirmarDatosRecibidos(1, mensajeRx))
    {
      // Test con juego
      ConfirmarDatosJuego(mensajeRx);
    }
  #endif
  osDelay(1000);
  memset(&mensajeRx, 0, sizeof(mensajeRx));
  #if TEST_RETOMAR_PARTIDA_2 == 1
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
    TestGuardarPartidaSinFinalizar(2);
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
		TestRetomarUltimaPartida();
    status = osMessageQueueGet(e_memoriaTxMessageId, &mensajeRx, NULL, osWaitForever);
    if (ConfirmarDatosRecibidos(2, mensajeRx))
    {
      // Test con juego
      ConfirmarDatosJuego(mensajeRx);
    }
  #endif

	#if TEST_GUARDAR_PARTIDA_FINALIZADA_1 == 1

	#endif

	#if TEST_GUARDAR_PARTIDA_FINALIZADA_2 == 1
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
    TestGuardarPartidaFinalizada(2);
		flag = osThreadFlagsWait(FLAG_READY, osFlagsWaitAll, osWaitForever);
		//TestObtenerInfoPartida();
    status = osMessageQueueGet(e_memoriaTxMessageId, &mensajeRx, NULL, osWaitForever);
    ConfirmarDatosRecibidos(2, mensajeRx);
	#endif
}

static void TestGuardarPartidaSinFinalizar(int numTest)
{
  osStatus_t status;
	printf("[TestMemoria::%s] Envio datos:\n", __func__);

  MemoriaMsg_t ultimaPartida = {0};

  switch (numTest)
  {
    case 1:
      ultimaPartida.tipoPeticion = GUARDAR_PARTIDA_SIN_FINALIZAR;
      memcpy(ultimaPartida.fechaPartida, fecha1, TAM_FECHA);
      memcpy(ultimaPartida.horaPartida, hora1, TAM_HORA);
      memcpy(ultimaPartida.nombreBlancas, nombreBlancas1, TAM_NOMBRE_JUGADOR);
      memcpy(ultimaPartida.nombreNegras, nombreNegras1, TAM_NOMBRE_JUGADOR);
      ultimaPartida.turno_victoria = turno_victoria1;
      memcpy(ultimaPartida.tiempoBlancas, tiempoBlancas1, TAM_TIEMPO_JUGADOR);
      memcpy(ultimaPartida.tiempoNegras, tiempoNegras1, TAM_TIEMPO_JUGADOR);
      memcpy(ultimaPartida.dato, tablero1, TAM_DATOS);
    break;

    case 2:
      ultimaPartida.tipoPeticion = GUARDAR_PARTIDA_SIN_FINALIZAR;
      memcpy(ultimaPartida.fechaPartida, fecha2, TAM_FECHA);
      memcpy(ultimaPartida.horaPartida, hora2, TAM_HORA);
      memcpy(ultimaPartida.nombreBlancas, nombreBlancas2, TAM_NOMBRE_JUGADOR);
      memcpy(ultimaPartida.nombreNegras, nombreNegras2, TAM_NOMBRE_JUGADOR);
      ultimaPartida.turno_victoria = turno_victoria2;
      memcpy(ultimaPartida.tiempoBlancas, tiempoBlancas2, TAM_TIEMPO_JUGADOR);
      memcpy(ultimaPartida.tiempoNegras, tiempoNegras2, TAM_TIEMPO_JUGADOR);
      memcpy(ultimaPartida.dato, tablero2, TAM_DATOS);
    break;
    
    default:
    break;
  }
	
  printf("[TestMemoria::%s] Tipo peticion [%d]\n", __func__, ultimaPartida.tipoPeticion);
  printf("[TestMemoria::%s] Fecha partida [%s]\n", __func__, ultimaPartida.fechaPartida);
  printf("[TestMemoria::%s] Hora partida [%s]\n", __func__, ultimaPartida.horaPartida);
  printf("[TestMemoria::%s] Nombre blancas [%s]\n", __func__, ultimaPartida.nombreBlancas);
  printf("[TestMemoria::%s] Nombre negras [%s]\n", __func__, ultimaPartida.nombreNegras);
  printf("[TestMemoria::%s] Turno [%d]\n", __func__, ultimaPartida.turno_victoria);
  printf("[TestMemoria::%s] Tiempo blancas [%s]\n", __func__, ultimaPartida.tiempoBlancas);
  printf("[TestMemoria::%s] Tiempo negras [%s]\n", __func__, ultimaPartida.tiempoNegras);
  printf("[TestMemoria::%s] Tablero [%s]\n", __func__, ultimaPartida.dato);

  status = osMessageQueuePut(e_memoriaRxMessageId, &ultimaPartida, 1, 0);
  printf("[TestMemoria::%s] Mensaje enviado\n", __func__);
}

static void TestGuardarPartidaFinalizada(int numTest)
{
	osStatus_t status;
	printf("[TestMemoria::%s] Envio datos:\n", __func__);

  MemoriaMsg_t partidaFinalizada = {0};

  switch (numTest)
  {
    case 1:
      partidaFinalizada.tipoPeticion = GUARDAR_PARTIDA_FINALIZADA;
      memcpy(partidaFinalizada.fechaPartida, fecha1, TAM_FECHA);
      memcpy(partidaFinalizada.horaPartida, hora1, TAM_HORA);
      memcpy(partidaFinalizada.nombreBlancas, nombreBlancas1, TAM_NOMBRE_JUGADOR);
      memcpy(partidaFinalizada.nombreNegras, nombreNegras1, TAM_NOMBRE_JUGADOR);
      partidaFinalizada.turno_victoria = turno_victoria1;
      memcpy(partidaFinalizada.tiempoBlancas, tiempoBlancas1, TAM_TIEMPO_JUGADOR);
      memcpy(partidaFinalizada.tiempoNegras, tiempoNegras1, TAM_TIEMPO_JUGADOR);
      memcpy(partidaFinalizada.dato, tablero1, TAM_DATOS);
    break;

    case 2:
      partidaFinalizada.tipoPeticion = GUARDAR_PARTIDA_FINALIZADA;
      memcpy(partidaFinalizada.fechaPartida, fecha2, TAM_FECHA);
      memcpy(partidaFinalizada.horaPartida, hora2, TAM_HORA);
      memcpy(partidaFinalizada.nombreBlancas, nombreBlancas2, TAM_NOMBRE_JUGADOR);
      memcpy(partidaFinalizada.nombreNegras, nombreNegras2, TAM_NOMBRE_JUGADOR);
      partidaFinalizada.turno_victoria = turno_victoria2;
      memcpy(partidaFinalizada.tiempoBlancas, tiempoBlancas2, TAM_TIEMPO_JUGADOR);
      memcpy(partidaFinalizada.tiempoNegras, tiempoNegras2, TAM_TIEMPO_JUGADOR);
      memcpy(partidaFinalizada.dato, tablero2, TAM_DATOS);
    break;
    
    default:
    break;
  }
	
  printf("[TestMemoria::%s] Tipo peticion [%d]\n", __func__, partidaFinalizada.tipoPeticion);
  printf("[TestMemoria::%s] Fecha partida [%s]\n", __func__, partidaFinalizada.fechaPartida);
  printf("[TestMemoria::%s] Hora partida [%s]\n", __func__, partidaFinalizada.horaPartida);
  printf("[TestMemoria::%s] Nombre blancas [%s]\n", __func__, partidaFinalizada.nombreBlancas);
  printf("[TestMemoria::%s] Nombre negras [%s]\n", __func__, partidaFinalizada.nombreNegras);
  printf("[TestMemoria::%s] Turno [%d]\n", __func__, partidaFinalizada.turno_victoria);
  printf("[TestMemoria::%s] Tiempo blancas [%s]\n", __func__, partidaFinalizada.tiempoBlancas);
  printf("[TestMemoria::%s] Tiempo negras [%s]\n", __func__, partidaFinalizada.tiempoNegras);
  printf("[TestMemoria::%s] Tablero [%s]\n", __func__, partidaFinalizada.dato);

  status = osMessageQueuePut(e_memoriaRxMessageId, &partidaFinalizada, 1, 0);
  printf("[TestMemoria::%s] Mensaje enviado\n", __func__);
}

static void TestRetomarUltimaPartida()
{
  osStatus_t status;
	printf("[TestMemoria::%s] Peticion retomar ultima partida\n", __func__);

  MemoriaMsg_t ultimaPartida = {0};

  ultimaPartida.tipoPeticion = RETOMAR_ULTIMA_PARTIDA;

  status = osMessageQueuePut(e_memoriaRxMessageId, &ultimaPartida, 1, 0);
  printf("[TestMemoria::%s] Mensaje enviado\n", __func__);
}

static void TestLimpiarMemoria()
{
  
}

static bool ConfirmarDatosRecibidos(int numTest, MemoriaMsg_t mensajeRx)
{
  printf("[TestMemoria::%s] Confirmo datos recibidos:\n", __func__);
  bool turnoCorrecto;
  bool tiempoBlancasCorrecto;
  bool tiempoNegrasCorrecto;
  bool datoCorrecto;

	printf("[TestMemoria::%s] Turno: [0x%02X] = [0x%02X]\n", __func__, mensajeRx.turno_victoria, turno_victoria1);

	printf("[TestMemoria::%s] Tiempo blancas:\n", __func__);
	for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
	{
		printf("[TestMemoria::%s] [0x%02X] = [0x%02X]\n", __func__, mensajeRx.tiempoBlancas[i], tiempoBlancas1[i]);
		osDelay(10);
	}

	printf("[TestMemoria::%s] Tiempo negras:\n", __func__);
	for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
	{
		printf("[TestMemoria::%s] [0x%02X] = [0x%02X]\n", __func__, mensajeRx.tiempoNegras[i], tiempoNegras1[i]);
		osDelay(10);
	}

	printf("[TestMemoria::%s] Tablero:\n", __func__);
	for (int i = 0; i < TAM_DATOS; i++)
	{
		printf("[TestMemoria::%s] [0x%02X] = [0x%02X]\n", __func__, mensajeRx.dato[i], tablero1[i]);
		osDelay(10);
	}

  switch (numTest)
  {
    case 1:
      turnoCorrecto         = mensajeRx.turno_victoria == turno_victoria1;
      tiempoBlancasCorrecto = memcmp(mensajeRx.tiempoBlancas, tiempoBlancas1, TAM_TIEMPO_JUGADOR) == 0;
      tiempoNegrasCorrecto  = memcmp(mensajeRx.tiempoNegras, tiempoNegras1, TAM_TIEMPO_JUGADOR) == 0;
      datoCorrecto          = memcmp(mensajeRx.dato, tablero1, TAM_DATOS) == 0;
    break;

    case 2:
      turnoCorrecto         = mensajeRx.turno_victoria == turno_victoria2;
      tiempoBlancasCorrecto = memcmp(mensajeRx.tiempoBlancas, tiempoBlancas2, TAM_TIEMPO_JUGADOR) == 0;
      tiempoNegrasCorrecto  = memcmp(mensajeRx.tiempoNegras, tiempoNegras2, TAM_TIEMPO_JUGADOR) == 0;
      datoCorrecto          = memcmp(mensajeRx.dato, tablero2, TAM_DATOS) == 0;
    break;

    default:
    break;
  }

  printf("[TestMemoria::%s] turnoCorrecto[%d] tiempoBlancasCorrecto[%d] tiempoNegrasCorrecto[%d] datoCorrecto[%d]\n", 
          __func__, turnoCorrecto, tiempoBlancasCorrecto, tiempoNegrasCorrecto, datoCorrecto);
  return turnoCorrecto && tiempoBlancasCorrecto && tiempoNegrasCorrecto && datoCorrecto;
}

static bool ConfirmarDatosJuego(MemoriaMsg_t mensajeRx)
{
  printf("[TestMemoria::%s] Seteo datos en el juego:\n", __func__);

  printf("[TestMemoria::%s] Seteo Tiempo Blancas: [%s]\n", __func__, mensajeRx.tiempoBlancas);
  int minutos, segundos;

  sscanf((const char*)mensajeRx.tiempoBlancas, "%2d:%2d", &minutos, &segundos);
  printf("[TestMemoria::%s] Minutos Blancas[%d] Segundos Blancas[%d]\n", __func__, minutos, segundos);
  setTiempoBlancas(minutos, segundos);

  sscanf((const char*)mensajeRx.tiempoNegras, "%2d:%2d", &minutos, &segundos);
  printf("[TestMemoria::%s] Minutos Negras[%d] Segundos Negras[%d]\n", __func__, minutos, segundos);
  setTiempoNegras(minutos, segundos);

  printf("[TestMemoria::%s] Turno actual[%d]\n", __func__, mensajeRx.turno_victoria);
  SetTurno(mensajeRx.turno_victoria);

  printf("[TestMemoria::%s] Seteo mapa\n", __func__);
  setMap(mensajeRx.dato, TAM_DATOS);

  osDelay(1000);

  printf("[TestMemoria::%s] Obtengo datos del juego:\n", __func__);

  char tiempoBlancas[TAM_TIEMPO_JUGADOR + 1];
  snprintf(tiempoBlancas, sizeof(tiempoBlancas), "%02d:%02d", GetMinutosBlancas(), GetSegundosBlancas());
  printf("[TestMemoria::%s] Tiempo Blancas [%s]\n", __func__, tiempoBlancas);

  char tiempoNegras[TAM_TIEMPO_JUGADOR + 1];
  snprintf(tiempoNegras, sizeof(tiempoNegras), "%02d:%02d", GetMinutosNegras(), GetSegundosNegras());
  printf("[TestMemoria::%s] Tiempo Negras [%s]\n", __func__, tiempoNegras);

  bool turnoBlancas = GetTurno();
  printf("[TestMemoria::%s] Turno actual[%d]\n", __func__, turnoBlancas);

  uint8_t mapa[64];
  printf("[TestMemoria::%s] Obtengo mapa\n", __func__);
  getMap(mapa, TAM_DATOS);

  const bool turnoCorrecto         = mensajeRx.turno_victoria == turnoBlancas;
  const bool tiempoBlancasCorrecto = memcmp(mensajeRx.tiempoBlancas, tiempoBlancas, TAM_TIEMPO_JUGADOR) == 0;
  const bool tiempoNegrasCorrecto  = memcmp(mensajeRx.tiempoNegras, tiempoNegras, TAM_TIEMPO_JUGADOR) == 0;
  const bool datoCorrecto          = memcmp(mensajeRx.dato, mapa, TAM_DATOS) == 0;

  printf("[TestMemoria::%s] turnoCorrecto[%d] tiempoBlancasCorrecto[%d] tiempoNegrasCorrecto[%d] datoCorrecto[%d]\n", 
          __func__, turnoCorrecto, tiempoBlancasCorrecto, tiempoNegrasCorrecto, datoCorrecto);
  return turnoCorrecto && tiempoBlancasCorrecto && tiempoNegrasCorrecto && datoCorrecto;
}