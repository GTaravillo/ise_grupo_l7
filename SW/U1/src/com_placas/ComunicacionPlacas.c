#include "ComunicacionPlacas.h"

/*USART Driver*/
extern 	ARM_DRIVER_USART Driver_USART7;
				ARM_DRIVER_USART *USARTdrv = &Driver_USART7;

				osThreadId_t 				id_th_com;
				osMessageQueueId_t	id_msg_send;
				osMessageQueueId_t	id_msg_receive;
static	osStatus_t					status_com;
static  osThreadId_t 				id_th_receive;
				osThreadId_t 				id_th_send;

static	char soh;
static 	char cmd;
static  char len;
static	char len_send;
static 	char payload[RECEIVE_MSG_SIZE - 4];
static	char eot;

static	char msg_receive[RECEIVE_MSG_SIZE];
static	char msg_send[SEND_MSG_SIZE];

				int 	Init_Th_com(void);
static	void	Init_USART(void);
//static 	void 	Th_com(void *argument);
static 	void 	Th_receive(void *argument);
static 	void 	Th_send(void *argument);
static 	void 	myUSART_callback(uint32_t event);

static uint8_t horaCorrecta(char hora[]);
static uint8_t tempCorrecta(char tempRef[], uint8_t len);
static uint8_t primerAnalisis(void);
static uint8_t segundoAnalisis(void);
static uint8_t analisisRespuesta(void);

static 	void 		discard(void);
static 	uint8_t receiveAnalysis(uint8_t byte);
static	uint8_t	msgAnalysis(char ascii);
static 	uint8_t eotAnalysis(void);


int Init_Th_com(void)	{
	Init_USART();
//  id_th_com 			= osThreadNew(Th_com, NULL, NULL);
	id_th_receive 	= osThreadNew(Th_receive, NULL, NULL);
	id_th_send 			= osThreadNew(Th_send, NULL, NULL);
	id_msg_send 		= osMessageQueueNew(SEND_MSG_COUNT, SEND_MSG_SIZE, NULL);
	id_msg_receive	= osMessageQueueNew(RECEIVE_MSG_COUNT, RECEIVE_MSG_SIZE, NULL);
	
//  if (id_th_com == NULL || id_msg_com == NULL) {
	if ((id_th_send == NULL) || (id_th_receive == NULL) || 
		  (id_msg_send == NULL) || (id_msg_receive == NULL)) {
    return(-1);
  }

  return(0);
}

static void Init_USART(void)	{
	ARM_USART_CAPABILITIES drv_capabilities;
	
	/*Initialize the USART driver*/
	USARTdrv->Initialize(myUSART_callback);
	
	/*Power up the USART peripheral*/
	USARTdrv->PowerControl(ARM_POWER_FULL);
	
	/*Configure the USART to 115200 bauds*/
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | 
										ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | 
										ARM_USART_FLOW_CONTROL_NONE, 115200);
	
	/*Enable Receiver and Transmitter lines*/
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);		// Enable TX output
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);		// Enable RX output
}

static 	void 	myUSART_callback(uint32_t event) {
	if (event & ARM_USART_EVENT_SEND_COMPLETE) {
		osThreadFlagsSet(id_th_send, SEND_COMPLETE);
	}
	else if (event & ARM_USART_EVENT_RECEIVE_COMPLETE){
		osThreadFlagsSet(id_th_receive, RECEIVE_COMPLETE);
	}
}

//static void discard(void) {
//	int i;
//	
//	soh = 0;
//	cmd = 0;
//	len = 0;
//	for (i = 0; i < COM_MSG_SIZE; i++) {
//		msg[i] = 0;
//	}
//	eot = 0;
//	
//	for (i = 0; i < COM_MSG_SIZE; i++) {
//		msg_com[i] = 0;
//	}
//}

//static uint8_t receiveAnalysis(uint8_t byte) {
//	int i;
//	switch (byte) {
//		case SOH:
//			if ((0 == soh) && (0 == cmd) && (0 == len)) {
//				soh = byte;
//				return MSG_OK;
//			}
//			else {
//				discard();
//				return MSG_ERR;
//			}
//			
//		
//		case CMD_SETHORA:
//		case CMD_HORAOK:
//		case CMD_SETTEMPREF:
//		case CMD_TEMPREFOK:
//		case CMD_GETALL:
//		case CMD_GETALLOK:
//		case CMD_ERASEALL:
//		case CMD_ERASEOK:
//			if ((0 != soh) && (0 == cmd) && (0 == len)) {
//				cmd = byte;
//				return MSG_OK;
//			}
//			else {
//				discard();
//				return MSG_ERR;
//			}
//		
//		case EOT:
//			if ((0 != soh) && (0 != cmd) && (0 != len) && 
//					eotAnalysis()) {
//				eot = byte;
//				/* Ensamblaje del mensaje a enviar */
//				msg_com[0] = soh;
//				msg_com[1] = cmd;
//				msg_com[2] = len;
//				if (len > 0x04) {
//					for (i = 0; i < (len - 4); i++) {
//						msg_com[i + 3] = msg[i];
//					}
//					msg_com[len - 1] = eot;
//				}
//				else {
//					msg_com[3] = eot;
//				}
//				
//				return ((CMD_HORAOK != cmd) || (CMD_TEMPREFOK != cmd) || (CMD_GETALLOK != cmd) || (CMD_ERASEOK != cmd)) ? MSG_FIN_RECEIVE : MSG_FIN_SEND;		// end
//			}
//			else {
//				discard();
//				return MSG_ERR;
//			}
//			
//		default:		// msg y len
//			if ((0 != soh) && (0 != cmd) && (0 == len) && (0x03 < byte)) {	// len
//				len = byte;
//				return MSG_OK;
//			}
//			else if ((0 != soh) && (0 != cmd) && (0 != len) && 
//							 (msgAnalysis(byte))) {			// msg
//				return MSG_OK;
//			}
//			else {
//				discard();
//				return MSG_ERR;
//			}
//	}
//}

//static uint8_t msgAnalysis(char ascii) {
//	uint8_t bytePuesto = 0;
//	int i;
//	
//	switch (cmd) {
//		case CMD_SETHORA:
//		case CMD_HORAOK:
//			if ((0x2F < ascii) && (ascii < 0x3B) && (0x0C == len)) { 	// 0x30 = '0' , 0x39 = '9', 0x3A = ':'
//				for (i = 0; i < (len - 4); i++) {
//					if ((0 == msg[i]) && (!bytePuesto)) {
//						bytePuesto = 1;
//						msg[i] = ascii;
//					}
//				}
//				return MSG_OK;
//			}
//			else {
//				return MSG_ERR;
//			}
//			
//		case CMD_SETTEMPREF:
//		case CMD_TEMPREFOK:
//			if ((((0x2F < ascii) && (ascii < 0x3B)) || (0x2E == ascii)) && 		// 0x2E = '.', 0x30 = '0' , 0x39 = '9'
//					 ((0x07 == len) || (0x08 == len))) { 		// min len = 7 (e.g. 5.0), max len = 8 (e.g. 30.0)
//				for (i = 0; i < (len - 4); i++) {
//					if ((0 == msg[i]) && (!bytePuesto)) {
//						bytePuesto = 1;
//						msg[i] = ascii;
//					}
//				}
//				return MSG_OK;
//			}
//			else {
//				return MSG_ERR;
//			}
//			
//		case CMD_GETALL:
//		case CMD_ERASEALL:
//		case CMD_ERASEOK:
//			if ((0x04 == len) && (0 == msg[0])) {
//				return MSG_OK;
//			}
//			else {
//				return MSG_ERR;
//			}
//		
//		case CMD_GETALLOK:
//			for (i = 0; i < (len - 4); i++) {
//				if ((0 == msg[i]) && (!bytePuesto)) {
//					bytePuesto = 1;
//					msg[i] = ascii;
//				}
//			}
//			return MSG_OK;
//			
//		default:
//			return MSG_ERR;
//	}
//}

//static void eraseMsgCom() {
//	int i;
//	int len = msg_com[2];
//	
//	for (i = 0; i < len; i++) {
//		msg[i] = 0;
//	}
//}

//static uint8_t eotAnalysis(void) {
//	int i;
//	int cnt = 0;
//	if ((SOH == soh) && (0 != cmd) && (0 != len)) {
//		for (i = 0; i < 40; i++) {
//			if (0 != msg[i]) {
//				cnt++;
//			}
//		}
//	}
//	if ((len - 4) == cnt) {
//		return MSG_OK;
//	}
//	else {
//		return MSG_ERR;
//	}
//}

static uint8_t horaCorrecta(char hora[]) {
	if ((('0' <= hora[0]) && (hora[0] <= '2')) && 	// Decenas hora
			(('0' <= hora[1]) && (hora[1] <= '9')) &&		// Unidades hora
			 (':' == hora[2]) &&												// ':'
			(('0' <= hora[3]) && (hora[3] <= '5')) &&		// Decenas minuto
			(('0' <= hora[4]) && (hora[4] <= '9')) &&		// Unidades minuto
			 (':' == hora[5]) &&												// ':'
			(('0' <= hora[6]) && (hora[6] <= '5')) && 	// Decenas segundo
			(('0' <= hora[7]) && (hora[7] <= '9'))) {		// Unidades segundo)
		if ((2 == hora[0]) && (('0' <= hora[1]) && (hora[1] <= '3'))) {	// Horas: [20, 23]
			return (1);
		}
		else if (2 != hora[0]) {		// Horas [00, 19]
			return (1);
		}
	}
	return (0);
}

static uint8_t tempCorrecta(char tempRef[], uint8_t len) {
	if (3 == len) {	// Tr [5.0, 9.5]
		if ((('5' <= tempRef[0]) && (tempRef[0] <= '9')) &&		// Unidades Tr
				 ('.' == tempRef[1]) &&														// '.'
				(('0' == tempRef[2]) || ('5' == tempRef[2]))) {	  // Decimales Tr
			return (1);		
		}	
	}
	else if (4 == len) {
		if ((('0' <= tempRef[0]) && (tempRef[0] <= '3')) &&		// Decenas Tr
				(('0' <= tempRef[1]) && (tempRef[1] <= '9')) &&		// Unidades Tr
				 ('.' == tempRef[2]) &&														// '.'
				(('0' == tempRef[3]) || ('5' == tempRef[3]))) {		// Decimales Tr
			if (('0' == tempRef[0]) && 	// Si las decenas de Tr = 0
				 (('5' <= tempRef[1]) && (tempRef[1] <= '9'))) {	// Las unidades deben estar entre [5, 9]
				return (1);
			}
			else if (('3' == tempRef[0]) && 	// Si las decenas de Tr = 3
							 ('0' == tempRef[1]) && 	// Unidades de Tr = 0
							 ('0' == tempRef[3])) {		// Decimales de Tr = 0
			return (1);
			}
		}
	}
	
	return (0);
}

static uint8_t primerAnalisis(void) {
	/* Primera recepción: 3 bytes en variables globales */
	
	/* Correcto si todas las condiciones se cumplen */
	/* SOH */
	if ((SOH == soh) && ((CMD_SETHORA == cmd) || (CMD_SETTEMPREF == cmd) ||
			(CMD_GETBUFFCIRC == cmd) || (CMD_ERASEALL == cmd)) &&
		  ((4 <= len) && (len <= RECEIVE_MSG_SIZE))) {
		return MSG_OK;
	}

	/* Reseteo las variables globales implicadas */
	soh = 0;
	cmd = 0;
	len = 0;

	return MSG_ERR;
}

static uint8_t segundoAnalisis(void) {
	int i;

	
	/* Segunda recepción */
	/* Compruebo que el último byte es EOT */
	if (EOT == eot) {
		/* Compruebo las condiciones de cada comando */
		/* Recordar que en el primer analisis compruebo que sea uno de los 
			 posibles comandos aceptados */
		
		switch (cmd) {
			case CMD_SETHORA:
				/* Condiciones */
				/* Longitud payload = 8 y hora en formato correcto */
				if ((LEN_HORA == len) && (horaCorrecta(payload))) {
					/* Todo correcto. Formo mensaje a enviar a módulo principal */
					msg_receive[0] = soh;
					msg_receive[1] = cmd;
					msg_receive[2] = len;
					for (i = 0; i < (len - 4); i++) {
						msg_receive[i + 3] = payload[i];
					}
					msg_receive[len - 1] = eot;
					
					/* Pongo mensaje en la cola */
					status_com = osMessageQueuePut(id_msg_receive, msg_receive, NULL, 0U);
					
					return MSG_OK;
				}
				else {
					soh = 0;
					cmd = 0;
					len = 0;
					for (i = 0; i < RECEIVE_MSG_SIZE; i++) {
						payload[i] = 0;
					}
					eot = 0;
					
					return MSG_ERR;
				}
			
			case CMD_SETTEMPREF:
				/* Condiciones */
				/* Longitud payload = 3/4 */
				if ((3 == (len - 4)) || (4 == (len - 4))) {
					/* Compruebo formato temperatura */
					if(tempCorrecta(payload, (len - 4))) {
						/* Todo correcto. Formo mensaje a enviar a módulo principal */
						msg_receive[0] = soh;
						msg_receive[1] = cmd;
						msg_receive[2] = len;
						for (i = 0; i < (len - 4); i++) {
							msg_receive[i + 3] = payload[i];
						}
						msg_receive[len - 1] = eot;
						
						/* Pongo mensaje en la cola */
						status_com = osMessageQueuePut(id_msg_receive, msg_receive, NULL, 0U);
						
						return MSG_OK;
					}
				}
				else {
					soh = 0;
					cmd = 0;
					len = 0;
					for (i = 0; i < RECEIVE_MSG_SIZE; i++) {
						payload[i] = 0;
					}
					eot = 0;
					
					return MSG_ERR;
				}
			
			case CMD_GETBUFFCIRC:
				/* Todo correcto. Formo mensaje a enviar a módulo principal */
				msg_receive[0] = soh;
				msg_receive[1] = cmd;
				msg_receive[2] = len;
				msg_receive[3] = eot;
			
				/* Pongo mensaje en la cola */
				status_com = osMessageQueuePut(id_msg_receive, msg_receive, NULL, 0U);
			
				return MSG_OK;
			
			case CMD_ERASEALL:
				/* Todo correcto. Formo mensaje a enviar a módulo principal */
				msg_receive[0] = soh;
				msg_receive[1] = cmd;
				msg_receive[2] = len;
				msg_receive[3] = eot;
			
				/* Pongo mensaje en la cola */
				status_com = osMessageQueuePut(id_msg_receive, msg_receive, NULL, 0U);
			
				return MSG_OK;
		}
	}
	
	/* EOT != eot */
	soh = 0;
	cmd = 0;
	len = 0;
	for (i = 0; i < RECEIVE_MSG_SIZE; i++) {
		payload[i] = 0;
	}
	eot = 0;
	
	return MSG_ERR;

}

static uint8_t analisisRespuesta(void) {
	char temp_soh = msg_send[0];
	char temp_cmd = msg_send[1];
	char temp_len = msg_send[2];
	char temp_eot = msg_send[temp_len - 1];
	/* Compruebo si el mensaje recibido desde el módulo principal es correcto */
	/* A tener en cuenta: para esta comprobación, doy por hecho que el módulo 
		 principal ha comprobado que los elementos del payload son correctos */
	
	if ((SOH == temp_soh) && (EOT == temp_eot)) {
		switch (temp_cmd) {
		case CMD_HORAOK:
			/* Condiciones */
			/* Longitud payload = 8 */
			if (LEN_HORA == temp_len) {
				/* Todo correcto */
				len_send = temp_len;
				return MSG_OK;
			}
				
			return MSG_ERR;

		
		case CMD_TEMPREFOK:
			/* Condiciones */
			/* Longitud payload = 8 */
			if (LEN_TEMPREF == temp_len) {
				/* Todo correcto */
				len_send = temp_len;
				return MSG_OK;
			}
				
			return MSG_ERR;
		
		case CMD_GETBUFFOK:
			/* Condiciones */
			/* Longitud payload = 40 */
			if (LEN_ELEMBUFF == temp_len) {
				/* Todo correcto */
				len_send = temp_len;
				return MSG_OK;
			}
				
			return MSG_ERR;
		
		case CMD_ERASEALL:
			/* Condiciones */
			/* Longitud payload = 40 */
			if (LEN_BORRAR == temp_len) {
				/* Todo correcto */
				len_send = temp_len;
				return MSG_OK;
			}
				
			return MSG_ERR;
		
		default:
			return MSG_ERR;
		}
	}
	return MSG_ERR;
}

static 	void 	Th_send(void *argument) {

	uint32_t flag;

	while(1) {

		status_com = osMessageQueueGet(id_msg_send, msg_send, NULL, osWaitForever);
		if (MSG_OK == analisisRespuesta()) {	// Analizo la respuesta enviada desde el módulo principal
			USARTdrv->Send(msg_send, len_send);	// Mensaje recibido desde módulo principal. Lo envío a TeraTerm
			flag = osThreadFlagsWait(SEND_COMPLETE, osFlagsWaitAll, osWaitForever);	//
		}
	}
}

static 	void 	Th_receive(void *argument) {

	uint32_t flag;
	char temp_msg[RECEIVE_MSG_SIZE] = {0};
	int i;
	
	while(1) {

		USARTdrv->Receive(temp_msg, 3); // Hasta el byte que indica la longitud total de la trama
		flag = osThreadFlagsWait(RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);	// Espero 5 seg a que se reciban los 3 bytes
		
		if (RECEIVE_COMPLETE == flag) {
			/* Guardo los 3 primeros bytes en sus correspondientes variables globales para su posterior analisis */
			soh = temp_msg[0];
			cmd = temp_msg[1];
			len = temp_msg[2];
			
			/* Analizo los 3 primeros bytes */
			if (MSG_OK == primerAnalisis()) {
				USARTdrv->Receive(temp_msg, (len - 3)); // Hasta lo que queda de mensaje según parámetro LEN
				flag = osThreadFlagsWait(RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);	// Espero 5 seg a que se reciban los 3 bytes
				
				if (RECEIVE_COMPLETE == flag) {
					/* Vuelco el contenido de payload del mensaje capturado en el array payload */
					for (i = 0; i < (len - 4); i++) {
						payload[i] = temp_msg[i];
					}
					
					eot = temp_msg[len - 4];	// Último byte
					
					/* Analizo payload y eot */
					segundoAnalisis();
				}
				/* Si no recibo todo el mensaje, reseteo las variables globales anteriores */
				else {
					soh = 0;
					cmd = 0;
					len = 0;
				}
			}
			
		}
	}
}