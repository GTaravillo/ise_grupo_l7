// Mifare RC522 RFID Card reader 13.56 MHz
// STM32F103 RFID RC522 SPI1 / UART / USB / Keil HAL / 2017 vk.com/zz555

// PA0  - (OUT)	LED2
// PA1	- (IN)	BTN1
// PA4  - (OUT)	SPI1_NSS (Soft)
// PA5  - (OUT)	SPI1_SCK
// PA6  - (IN)	SPI1_MISO (Master In)
// PA7  - (OUT)	SPI1_MOSI (Master Out)
// PA9	- (OUT)	TX UART1 (RX-RS232)
// PA10	- (IN)	RX UART1 (TX-RS232)
// PA11 - (OUT) USB_DM
// PA12 - (OUT) USB_DP
// PA13 - (IN)	SWDIO
// PA14 - (IN)	SWDCLK
// PC13 - (OUT)	LED1

// MFRC522		STM32F103		DESCRIPTION
// CS (SDA)		PA4					SPI1_NSS	Chip select for SPI
// SCK				PA5					SPI1_SCK	Serial Clock for SPI
// MOSI				PA7 				SPI1_MOSI	Master In Slave Out for SPI
// MISO				PA6					SPI1_MISO	Master Out Slave In for SPI
// IRQ				-						Irq
// GND				GND					Ground
// RST				3.3V				Reset pin (3.3V)
// VCC				3.3V				3.3V power

#include "stm32f4xx_hal.h"
#include "rc522.h"
#include <stdio.h>
#include "../config/Paths.h"
#include "../com_placas/ComunicacionPlacas.h"  //PATH_COM_PLACAS

SPI_HandleTypeDef hspi3;
// osMessageQueueId_t cola_nfc;
osThreadId_t tid_Thread_NFC;
osStatus_t status_cola;
ComPlacasMsg_t msg;

//osThreadId_t tid_ThSimNfc;


// RC522
uint8_t MFRC522_Check(uint8_t* id);
uint8_t MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID);
void MFRC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t MFRC522_ReadRegister(uint8_t addr);
void MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t* TagType);
uint8_t MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
uint8_t MFRC522_Anticoll(uint8_t* serNum, uint8_t CL);
void MFRC522_CalulateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
uint8_t MFRC522_SelectTag(uint8_t* serNum, uint8_t CL);
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t* recvData);
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t* writeData);
void MFRC522_Init(void);
void MFRC522_Reset(void);
void MFRC522_AntennaOn(void);
void MFRC522_AntennaOff(void);
void MFRC522_Halt(void);

//static void MX_SPI1_Init(void);
//static void MX_GPIO_Init(void);
void RC_RUN(void *argument);
int Init_Thread_NFC (void);



int Init_Thread_NFC (void) {
 
  // cola_nfc = osMessageQueueNew(10, sizeof(ComPlacasMsg_t), NULL);
  tid_Thread_NFC = osThreadNew(RC_RUN, NULL, NULL);
  if (tid_Thread_NFC == NULL) {
    return(-1);
  }
 
  return(0);
}

uint8_t SPI1SendByte(uint8_t data) {
	unsigned char writeCommand[1];
	unsigned char readValue[1];

	writeCommand[0] = data;
	HAL_SPI_TransmitReceive(&hspi3, (uint8_t*)&writeCommand, (uint8_t*)&readValue, 1, 10);
	return readValue[0];

	//while (SPI1->SR & SPI_SR_BSY);								// STM32F030 - ждем конец передачи
	//while (SPI1->SR & SPI_I2S_FLAG_BSY);					// STM32F103 - ждем конец передачи

	//while (!(SPI1->SR & SPI_SR_TXE));     				// убедиться, что предыдущая передача завершена (STM32F103)
	//SPI1->DR = data;															// вывод в SPI1
	//while (!(SPI1->SR & SPI_SR_RXNE));     				// ждем окончания обмена (STM32F103)
	//for (uint8_t i=0; i<50; i++) {};
	//data = SPI1->DR;															// читаем принятые данные
	//return data;
}

void SPI1_WriteReg(uint8_t address, uint8_t value) {
	cs_reset();
	SPI1SendByte(address);
	SPI1SendByte(value);
	cs_set();
}

uint8_t SPI1_ReadReg(uint8_t address) {
	uint8_t	val;

	cs_reset();
	SPI1SendByte(address);
	val = SPI1SendByte(0x00);
	cs_set();
	return val;
}

void MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	addr = (addr << 1) & 0x7E;															// Address format: 0XXXXXX0
  SPI1_WriteReg(addr, val);
}

uint8_t MFRC522_ReadRegister(uint8_t addr) {
	uint8_t val;

	addr = ((addr << 1) & 0x7E) | 0x80;
	val = SPI1_ReadReg(addr);
	return val;
}

uint8_t MFRC522_Check(uint8_t* id) {
	uint8_t status;
	status = MFRC522_Request(PICC_REQIDL, id);							// Find cards, return card type
	if (status == MI_OK) status = MFRC522_Anticoll(id, 1);			// Card detected. Anti-collision, return card serial number 4 bytes
	MFRC522_Halt();																					// Command card into hibernation
	return status;
}

uint8_t MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID) {
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) return MI_ERR;
	}
	return MI_OK;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) | mask);
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask){
	MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) & (~mask));
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t* TagType) {
	uint8_t status;
	uint16_t backBits;																			// The received data bits

	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		// TxLastBists = BitFramingReg[2..0]
	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10)) status = MI_ERR;
	return status;
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	uint8_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
		break;
	}

	MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

	// Writing data to the FIFO
	for (i = 0; i < sendLen; i++) MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);

	// Execute the command
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		// StartSend=1,transmission of data starts

	// Waiting to receive data to complete
	i = 2000;	// i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
	do {
		// CommIrqReg[7..0]
		// Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);																// StartSend=0

	if (i != 0)  {
		if (!(MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {
			status = MI_OK;
			if (n & irqEn & 0x01) status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE) {
				n = MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits) *backLen = (n-1)*8+lastBits; else *backLen = n*8;
				if (n == 0) n = 1;
				if (n > MFRC522_MAX_LEN) n = MFRC522_MAX_LEN;
				for (i = 0; i < n; i++) backData[i] = MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);		// Reading the received data in FIFO
			}
		} else status = MI_ERR;
	}
	return status;
}

uint8_t MFRC522_Anticoll(uint8_t* serNum, uint8_t CL) {
	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);												// TxLastBists = BitFramingReg[2..0]
	serNum[0] = CL == 1 ? PICC_ANTICOLL : 0x95;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
	if (status == MI_OK) {
		// Check card serial number
		for (i = 0; i < 4; i++) serNumCheck ^= serNum[i];
		if (serNumCheck != serNum[i]) status = MI_ERR;
	}
	return status;
}

void MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);													// CRCIrq = 0
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);													// Clear the FIFO pointer
	// Write_MFRC522(CommandReg, PCD_IDLE);

	// Writing data to the FIFO
	for (i = 0; i < len; i++) MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	// Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));																						// CRCIrq = 1

	// Read CRC calculation result
	pOutData[0] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

uint8_t MFRC522_SelectTag(uint8_t* serNum, uint8_t CL) {
	uint8_t i;
	uint8_t status;
	uint8_t size=0;
	uint16_t recvBits;
	uint8_t buffer[9];

	buffer[0] = CL == 1 ? PICC_SElECTTAG : 0x95;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) buffer[i+2] = *(serNum+i);
	MFRC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	if (CL == 1){
		if ((status == MI_OK) && (recvBits == 0x18)){
			size = buffer[0];
		}else{
			size = 0;
		}
	}else if(CL == 2){
		if ((status == MI_OK) && (recvBits == 0x18)){
			size = 1;
		}else{
			size = 0;
		}
	}
	return size;
}

uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) {
	uint8_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12];

	// Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) buff[i+2] = *(Sectorkey+i);
	for (i=0; i<4; i++) buff[i+8] = *(serNum+i);
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	if ((status != MI_OK) || (!(MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08))) status = MI_ERR;
	return status;
}

uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	uint8_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	MFRC522_CalculateCRC(recvData,2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90)) status = MI_ERR;
	return status;
}

uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t* writeData) {
	uint8_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) status = MI_ERR;
	if (status == MI_OK) {
		// Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++) buff[i] = *(writeData+i);
		MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) status = MI_ERR;
	}
	return status;
}

void MFRC522_Init(void) {
	MFRC522_Reset();
	MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
	MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);
	MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);				// 48dB gain
	MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);
	MFRC522_AntennaOn();																		// Open the antenna
}

void MFRC522_Reset(void) {
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

void MFRC522_AntennaOn(void) {
	uint8_t temp;

	temp = MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_AntennaOff(void) {
	MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

static void MX_SPI3_Init(void)
{

  /* SPI1 parameter configuration*/
	
	__SPI3_CLK_ENABLE();
	HAL_SPI_MspInit(&hspi3);
	
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
	
  if (HAL_SPI_Init(&hspi3) == HAL_OK)
  {
    printf("SPI inicialized succesfully.\n");
  }else if ((HAL_SPI_Init(&hspi3) == HAL_ERROR))
	{
		printf("Error ocurred during SPI inicializing.\n");
	}else if ((HAL_SPI_Init(&hspi3) == HAL_TIMEOUT))
	{
		printf("Timeout during SPI initialization.\n");
	}

}

static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  //__HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pins : PA0 PA4 PA13 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_10;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /*Configure GPIO pin Output Level */
  // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

}

void RC_RUN(void *argument){
	uint8_t state = 99;
	uint8_t CT[3];
	uint8_t pData[16];
	uint8_t i=0;
	uint8_t readUid[5];
	uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
	uint8_t KEY_B[6]= {0xff,0xff,0xff,0xff,0xff,0xff};	
	uint8_t size=0;
	
	MX_GPIO_Init();
  MX_SPI3_Init();
  MFRC522_Init();
	msg.mensaje[1] = 0;
	msg.remitente = MENSAJE_NFC;
  //uint8_t loop = 0;
  uint32_t flag;
  /* USER CODE BEGIN 2 */
  osDelay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //osThreadFlagsWait(FLAG_EMPIEZA_FINALIZA, osFlagsWaitAny, osWaitForever);
  while (1)
  {
//		
//		msg.mensaje[0] = /*(char)pData[0]*/ 0x22;
//    status_cola=osMessageQueuePut(e_comPlacasTxMessageId, &msg, 0U, 0U);
    
    /* USER CODE END WHILE */
   
	  state = MFRC522_Request(PICC_REQALL, CT);
    //printf("[RC522::%s] state[%d]\n", __func__, state);
	if (state == MI_OK)
	{
		
		state = MFRC522_Anticoll(readUid, 1);//防冲撞
		if (state == MI_OK)
		{
			printf("Card read: %d %d %d\n",CT[0],CT[1],CT[2]);
			printf("UID: %d %d %d %d %d ",readUid[0],readUid[1],readUid[2],readUid[3],readUid[4]);
			size = MFRC522_SelectTag(readUid, 1);
		}
		
		state = MFRC522_Anticoll(readUid, 2);//防冲撞2
		if (state == MI_OK)
		{
			printf("%d %d %d %d %d\n",readUid[0],readUid[1],readUid[2],readUid[3],readUid[4]);
			size = MFRC522_SelectTag(readUid, 2);
		}
//		if (size != 0)
//		{
//			state = MFRC522_Auth(0x60, 0x00, KEY_A, readUid);
//			
//			if(state == MI_OK)//验证A成功
//			{
//					printf("Key A correct.\n");
//					osDelay(100);
//			}
//			else
//			{
//					printf("Key A incorrect.\n");
//					osDelay(100);
//			}
//								
//			// 验证B密钥 块地址 密码 SN  
//			state = MFRC522_Auth(0x61, 0x00, KEY_B, readUid);
//			if(state == MI_OK)//验证B成功
//			{
//					printf("Key B correct.\n");
//			}
//			else
//			{
//					printf("Key B incorrect.\n");					
//			}
//			osDelay(100);
//		}
			
		//state = MFRC522_Read(0x00, pData);
		if(size != 0)
		{
			state = MFRC522_Read(0x04, pData);
			if (state == MI_OK)
			{
				printf("Read: ");
				for(i = 0; i < 16; i++)
				{
          if (i < 9)
					printf("%02x ", pData[i]);
          if (i == 9)
            printf("\n pieza =");
          if (i >= 9)
            printf("%c ", pData[i]);
				}
        printf("%c%c%c%c",pData[9],pData[10],pData[11],pData[12]);
				printf("\n");
        msg.destinatario = MENSAJE_JUEGO;
				msg.mensaje[0] = (char)(pData[9]-48);
        
        printf("[RC522::%s] Mensaje a enviar:\n", __func__);
        printf("[RC522::%s] remitente[%d] mensaje[0] = [0x%02X] mensaje[1] = [0x%02X]\n", __func__, msg.remitente, msg.mensaje[0], msg.mensaje[1]);
        status_cola=osMessageQueuePut(e_comPlacasTxMessageId, &msg, 1, 0);
				MFRC522_Halt();
        osThreadFlagsWait(FLAG_PIEZA_LEIDA, osFlagsWaitAll, 5000);
       
			}
			else
			{
				printf("Fail to read content.\n");
			}
		}
		
    
	}else if(state == MI_NOTAGERR){
		printf("No card read\n");
	}else{
		//printf("[RC522::%s]Error! State[%d]\n", __func__, state);
	}
  
  flag = osThreadFlagsWait(FLAG_FINALIZA , osFlagsWaitAny, 10U);
  if(flag == FLAG_FINALIZA){
    HAL_SPI_MspDeInit(&hspi3);
    break;
  }
	osDelay(500);
  
  
  
  osThreadYield(); 
	}
  
  osThreadYield();
}

/* Para probación
int ThSimNfc(void){
    tid_ThSimNfc = osThreadNew(nfc_sim, NULL, NULL);
    if (tid_ThSimNfc == NULL) {
        return(-1);
    }
    return(0);
}

void nfc_sim(void* argument){
  int flag[2];
  uint8_t i;
  //osDelay(5000);
  
  for(i=0; i<=3; i++){
    flag[1] =osThreadFlagsSet(tid_Thread_NFC, FLAG_PIEZA_LEIDA);
    printf("Read finished: [%d]\n", flag[1]);
    osDelay(5000);
  }
  flag[0]=osThreadFlagsSet(tid_Thread_NFC, FLAG_FINALIZA);
  printf("End reading: [%d]\n", flag[0]);
  osThreadYield(); 
}
*/
