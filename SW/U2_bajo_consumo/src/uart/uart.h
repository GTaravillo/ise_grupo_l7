#ifndef __COM_H
#define __COM_H

#include "cmsis_os2.h"  
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

#define USART_EVENT (ARM_USART_EVENT_SEND_COMPLETE /*Send completed; however USART may still transmit data*/| \
										 ARM_USART_EVENT_RECEIVE_COMPLETE /*Receive completed*/| \
										 ARM_USART_EVENT_TRANSFER_COMPLETE /*Transfer completed*/| \
										 ARM_USART_EVENT_TX_COMPLETE /*Transmit completed (optional)*/)

#define SEND_COMPLETE 		0X01
#define RECEIVE_COMPLETE	0x02
#define	SEND_RECEIVE			(RECEIVE_COMPLETE | SEND_COMPLETE)

/*Formato tramas de comunicación: SOH CMD LEN Payload EOT*/
#define SOH							0x01	/*Todas las tramas commienzan con este carácter (Start of Head)*/
/*CMD: Si el equipo acepta el valor, éste devolverá la misma trama con el valor CMD en complemento a uno*/
#define CMD_SETHORA 		0x20 	/*Establece la hora*/
#define CMD_HORAOK 			0xDF	
#define CMD_SETTEMPREF	0x25 	/*Establece el valor de la temperatura de referencia*/
#define CMD_TEMPREFOK		0xDA
#define CMD_GETBUFFCIRC	0x55	/*Solicita todas las medidas almacenadas en el buffer circular. El equipo responde con tantas tramas del tipo "Medida" como medidas tenga almacenadas*/
#define CMD_GETBUFFOK		0xAA
#define	CMD_ERASEALL		0x60	/*Borra las medidas almacenadas en el buffer circular*/
#define CMD_ERASEOK			0x9F

#define LEN_HORA				0x0C	/*Longitud trama de respuesta a Puesta en hora (CMD 0x20)*/
#define LEN_TEMPREF			0x08	/*Longitud trama de respuesta a Establecer Tr (CMD 0x25)*/
#define LEN_ELEMBUFF		0x28	/*Longitud trama de respuesta de cada elemento del buffer circular (CMD 0x55)*/
#define LEN_BORRAR			0x04	/*Longitud trama de respuesta a Borrar medidas (CMD 0x60)*/


#define	EOT							0xFE	/*Fin de la trama*/

#define SEND_MSG_COUNT 	10			// Maximum number of messages in queue
#define SEND_MSG_SIZE		40		// Maximum message size in bytes

#define RECEIVE_MSG_COUNT 	10			// Maximum number of messages in queue
#define RECEIVE_MSG_SIZE		12			// Maximum message size in bytes

#define MSG_ERR					0
#define MSG_OK					1

extern 	osThreadId_t				id_th_com;
extern 	osThreadId_t 				id_th_send;
extern 	osMessageQueueId_t	id_msg_send;
extern	osMessageQueueId_t	id_msg_receive;

				int Init_Th_com(void);

#endif
