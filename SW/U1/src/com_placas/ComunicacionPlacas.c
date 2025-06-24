#include "ComunicacionPlacas.h"

#include <stdio.h>
#include <string.h>

#include "../Config/Paths.h"
#include PATH_JUEGO
#include PATH_SERVER

/*USART Driver*/
extern 	ARM_DRIVER_USART Driver_USART7;
        ARM_DRIVER_USART *USARTdrv = &Driver_USART7;

        osThreadId_t        e_comPlacasRxThreadId;
        osThreadId_t        e_comPlacasTxThreadId;
        osMessageQueueId_t  e_comPlacasRxMessageId;
        osMessageQueueId_t  e_comPlacasTxMessageId;
static	osStatus_t          status;

void ComunicacionPlacasInitialize(void);

static void InitUart(void);
static void RunRx(void *argument);
static void RunTx(void *argument);

static void ProcesarMensajeRecibido(ComPlacasMsg_t mensajeRx);

static void ProcesarMensajeLcd(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeLedStrip(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeServidor(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeRtc(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajePosicion(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeMemoria(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeJuego(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeDistancia(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeNfc(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeAdc(ComPlacasMsg_t mensajeRx);
static void ProcesarMensajeMicrofono(ComPlacasMsg_t mensajeRx);

static void UartCallback(uint32_t event);

void ComunicacionPlacasInitialize(void)	{
  InitUart();
  e_comPlacasRxThreadId  = osThreadNew(RunRx, NULL, NULL);
  osDelay(100);
  e_comPlacasTxThreadId  = osThreadNew(RunTx, NULL, NULL);
  e_comPlacasTxMessageId = osMessageQueueNew(NUMERO_MENSAJES_COM_PLACAS_MAX, sizeof(ComPlacasMsg_t), NULL);
  e_comPlacasRxMessageId = osMessageQueueNew(NUMERO_MENSAJES_COM_PLACAS_MAX, sizeof(ComPlacasMsg_t), NULL);
	
  if ((e_comPlacasRxThreadId == NULL)  || (e_comPlacasTxThreadId == NULL) || 
      (e_comPlacasTxMessageId == NULL) || (e_comPlacasRxMessageId == NULL)) 
  {
    printf("[com::%s] ERROR!\n", __func__);
  }

}

static void InitUart(void)	
{
  printf("[com::%s]\n", __func__);
  ARM_USART_CAPABILITIES drv_capabilities;
	
  /*Initialize the USART driver*/
  USARTdrv->Initialize(UartCallback);
	
  /*Power up the USART peripheral*/
  USARTdrv->PowerControl(ARM_POWER_FULL);
	
  /*Configure the USART to 115200 bauds*/
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | 
                    ARM_USART_PARITY_NONE       | ARM_USART_STOP_BITS_1 | 
                    ARM_USART_FLOW_CONTROL_NONE, 115200);
	
  /*Enable Receiver and Transmitter lines*/
  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);		// Enable TX output
  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);		// Enable RX output
}

static void RunTx(void *argument) 
{
  ComPlacasMsg_t mensajeTx;
  uint32_t flag;
  int bytesMensaje = sizeof(mensajeTx);
  printf("[com::%s] Bytes mensaje [%d]\n", __func__, bytesMensaje);
  
  while(1) 
  {
    //printf("[com::%s] Esperando mensaje...\n", __func__);
    status = osMessageQueueGet(e_comPlacasTxMessageId, &mensajeTx, NULL, 1000);

    if (status != osOK)
    {
     // printf("[com::%s] Error! e_comPlacasTxMessageId status [%d]\n", __func__, status);
      continue;
    }

    printf("[com::%s] Mensaje a enviar: remitente[%d] destinatario[%d]\n", __func__, mensajeTx.remitente, mensajeTx.destinatario);
	  for (int i = 0; i < TAM_MENSAJE_MAX; i++)
	  {
	    printf("[com::%s] Mensaje a enviar: mensaje[%d] = [0x%02X]\n", __func__, i, mensajeTx.mensaje[i]);
	  }
    USARTdrv->Send(&mensajeTx, sizeof(ComPlacasMsg_t));
	  flag = osThreadFlagsWait(ERROR_FRAMING | SEND_COMPLETE, osFlagsWaitAll, 1000);
    
    if (flag == ERROR_FRAMING)
    {
      USARTdrv->Control(ARM_USART_ABORT_SEND, 0);
      USARTdrv->Control(ARM_USART_ABORT_RECEIVE, 0);
      continue;
    }
    else if (flag & osFlagsError)
    {
      continue; 
    }
  }
}

static void RunRx(void *argument) 
{
  printf("[com::%s]\n", __func__);

  uint32_t flag;
  ComPlacasMsg_t mensajeRx = {};
  int bytesMensaje = sizeof(ComPlacasMsg_t);
  printf("[com::%s] Bytes mensaje [%d]\n", __func__, bytesMensaje);

  while(1) 
  {
		memset(&mensajeRx, 0, sizeof mensajeRx);
    //printf("[com::%s] Esperando mensaje...\n", __func__);
    USARTdrv->Receive(&mensajeRx, bytesMensaje); // Hasta el byte que indica la longitud total de la trama
    flag = osThreadFlagsWait(ERROR_FRAMING | RECEIVE_COMPLETE, osFlagsWaitAny, 1000);
    //printf("flag[%d]", flag);
    if (flag == ERROR_FRAMING)
    {
      USARTdrv->Control(ARM_USART_ABORT_SEND, 0);
      USARTdrv->Control(ARM_USART_ABORT_RECEIVE, 0);
      continue;
    }
    else if (flag & osFlagsError)
    {
      continue; 
    }

    printf("[com::%s] Mensaje recibido: remitente[%d] destinatario[%d]\n", __func__, mensajeRx.remitente, mensajeRx.destinatario);
	  for (int i = 0; i < TAM_MENSAJE_MAX; i++)
    {
      printf("[com::%s] Mensaje recibido: mensaje[%d] = [%d]\n", __func__, i, mensajeRx.mensaje[i]);
    }

    ProcesarMensajeRecibido(mensajeRx);
  }
}

static void ProcesarMensajeRecibido(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.remitente);
  switch (mensajeRx.remitente)
  {
    case MENSAJE_LCD:
      ProcesarMensajeLcd(mensajeRx);
    break;

    case MENSAJE_LED_STRIP:
      ProcesarMensajeLedStrip(mensajeRx);
    break;

    case MENSAJE_SERVIDOR:
      ProcesarMensajeServidor(mensajeRx);
    break;

    case MENSAJE_RTC:
      ProcesarMensajeRtc(mensajeRx);
    break;

    case MENSAJE_POSICION:
      ProcesarMensajePosicion(mensajeRx);
    break;

    case MENSAJE_MEMORIA:
      ProcesarMensajeMemoria(mensajeRx);
    break;

    case MENSAJE_JUEGO:
      ProcesarMensajeJuego(mensajeRx);
    break;

    case MENSAJE_DISTANCIA:
      ProcesarMensajeDistancia(mensajeRx);
    break;

    case MENSAJE_NFC:
      ProcesarMensajeNfc(mensajeRx);
    break;

    case MENSAJE_ADC:
      ProcesarMensajeAdc(mensajeRx);
    break;

    default:
     // printf("[com::%s] Remitente desconocido [%d]\n", __func__, mensajeRx.remitente);
    break;
  }
}

static void ProcesarMensajeLcd(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Destinatario [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;


    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeLedStrip(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
			
		break;

    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeServidor(ComPlacasMsg_t mensajeRx)
{

  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
	
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
			
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;


    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeRtc(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;

    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajePosicion(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;

    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeMemoria(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;


    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeJuego(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;


    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeDistancia(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      osThreadFlagsSet(e_juegoThreadId, FLAG_SENSOR_DISTANCIA);
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;

    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeNfc(ComPlacasMsg_t mensajeRx)
{
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR:
      
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
    {
      JuegoMsg_t msg = {
        .remitente = mensajeRx.remitente,
        .pieza     = mensajeRx.mensaje[0]
      };
      printf("[com::%s] Enviar mensaje:\n", __func__);
      printf("[com::%s] remitente[%d] pieza[%d]\n", __func__, msg.remitente, msg.pieza);
      osMessageQueuePut(e_juegoRxMessageId, &msg, 1, 0);
    }
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;

    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}

static void ProcesarMensajeAdc(ComPlacasMsg_t mensajeRx)
{
	
  printf("[com::%s] Remitente [%d]\n", __func__, mensajeRx.destinatario);
  switch (mensajeRx.destinatario)
  {
    case MENSAJE_LCD:
      
    break;

    case MENSAJE_LED_STRIP:
      
    break;

    case MENSAJE_SERVIDOR: {				
			  SetConsumoActual(mensajeRx.mensaje[1]); // 3 bytes
		
				if(mensajeRx.mensaje[0] > 50)
				{
					osThreadFlagsSet(e_juegoThreadId,FLAG_PAUSE );
				}
    }
    break;

    case MENSAJE_RTC:
      
    break;

    case MENSAJE_POSICION:
      
    break;

    case MENSAJE_MEMORIA:
      
    break;

    case MENSAJE_JUEGO:
      
    break;

    case MENSAJE_DISTANCIA:
      
    break;

    case MENSAJE_NFC:
      
    break;

    case MENSAJE_ADC:
      
    break;


    default:
      printf("[com::%s] Destinatario desconocido [%d]\n", __func__, mensajeRx.destinatario);
    break;
  }
}


static void UartCallback(uint32_t event) 
{
	printf("[com::%s] event[%#x]\n", __func__, event);
  if (event & ARM_USART_EVENT_SEND_COMPLETE) 
  {
    osThreadFlagsSet(e_comPlacasTxThreadId, SEND_COMPLETE);
  }
  else if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
  {
    osThreadFlagsSet(e_comPlacasRxThreadId, RECEIVE_COMPLETE);
  }
else if (event & ARM_USART_EVENT_RX_FRAMING_ERROR)
  {
    osThreadFlagsSet(e_comPlacasRxThreadId, ERROR_FRAMING);
  }
}
