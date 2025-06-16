#include "ComunicacionPlacas.h"

#include "stdio.h"
#include "string.h"

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
static void UartCallback(uint32_t event);

void ComunicacionPlacasInitialize(void)	{
  InitUart();
  e_comPlacasRxThreadId  = osThreadNew(RunRx, NULL, NULL);
  osDelay(100);
  e_comPlacasTxThreadId  = osThreadNew(RunTx, NULL, NULL);
  e_comPlacasTxMessageId = osMessageQueueNew(NUMERO_MENSAJES_MAX, TAM_MENSAJE_MAX, NULL);
  e_comPlacasRxMessageId = osMessageQueueNew(NUMERO_MENSAJES_MAX, TAM_MENSAJE_MAX, NULL);
	
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
  mensaje_t mensajeTx;
  uint32_t flag;
  int bytesMensaje = sizeof(mensajeTx);
  printf("[com::%s] Bytes mensaje [%d]\n", __func__, bytesMensaje);
  
  while(1) 
  {
		status = osMessageQueueGet(e_comPlacasTxMessageId, &mensajeTx, NULL, osWaitForever);
    printf("[com::%s] Esperando mensaje...\n", __func__);
    status = osMessageQueueGet(e_comPlacasTxMessageId, &mensajeTx, NULL, osWaitForever);
    printf("[com::%s] Mensaje a enviar: tipo [%d]\n", __func__, mensajeTx.remitente);
	  for (int i = 0; i < (TAM_MENSAJE_MAX - 1); i++)
	  {
	    printf("[com::%s] Mensaje a enviar: mensaje[%d] = [%d]\n", __func__, i, mensajeTx.mensaje[i]);
	  }
    USARTdrv->Send(&mensajeTx, sizeof(mensajeTx));
	  //flag = osThreadFlagsWait(SEND_COMPLETE, osFlagsWaitAll, osWaitForever);
		
		osDelay(5000);
  }
}

static void RunRx(void *argument) 
{
  printf("[com::%s]\n", __func__);

  uint32_t flag;
  mensaje_t mensajeRx = {};
  int bytesMensaje = sizeof(mensajeRx);
  printf("[com::%s] Bytes mensaje [%d]\n", __func__, bytesMensaje);

  while(1) 
  {
		memset(&mensajeRx, 0, sizeof mensajeRx);
    printf("[com::%s] Esperando mensaje...\n", __func__);
    USARTdrv->Receive(&mensajeRx, bytesMensaje); // Hasta el byte que indica la longitud total de la trama
    flag = osThreadFlagsWait(RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);	// Espero 5 seg a que se reciban los 3 bytes
	
    printf("[com::%s] Mensaje recibido: tipo [%d]\n", __func__, mensajeRx.remitente);
	  for (int i = 0; i < (TAM_MENSAJE_MAX - 1); i++)
    {
      printf("[com::%s] Mensaje recibido: mensaje[%d] = [%d]\n", __func__, i, mensajeRx.mensaje[i]);
    }
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
}
