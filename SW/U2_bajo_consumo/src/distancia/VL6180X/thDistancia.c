#include "../../config/Paths.h"
#include "thDistancia.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include <stdio.h>
#include "../../com_placas/ComunicacionPlacas.h"  //PATH_COM_PLACAS

//extern osThreadId_t tid_ThDistancia;  
osThreadId_t tid_ThDistancia;
//osThreadId_t tid_ThSimDis;
extern ARM_DRIVER_I2C Driver_I2C2;
void I2C_SignalEvent(uint32_t event);
void I2C_SignalEvent_dis(uint32_t event);

ARM_DRIVER_I2C* I2Cdrv = &Driver_I2C2;
VL6180x_RangeData_t Range;
ComPlacasMsg_t msg_dis;
uint8_t read_h = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
int ThDistancia(void);
void Thread_Dis(void* argument);
/* Private functions ---------------------------------------------------------*/
void MyDev_Init(){
	I2Cdrv->Initialize(I2C_SignalEvent_dis);
    I2Cdrv->PowerControl(ARM_POWER_FULL);
    I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
    
}

void I2C_SignalEvent_dis(uint32_t event){
    uint8_t mask = ARM_I2C_EVENT_TRANSFER_DONE;
    if(event & mask){
        osThreadFlagsSet(tid_ThDistancia, 0x01);
    }
}
int ThDistancia(void){
    tid_ThDistancia = osThreadNew(Thread_Dis, NULL, NULL);
    if (tid_ThDistancia == NULL) {
      printf("[distancia::%s] ERROR HILO!", __func__);
        return(-1);
    }
    return(0);
}

void Thread_Dis(void* argument){
    uint8_t dev = 0x29;
		osStatus_t status_cola;
	  msg_dis.remitente    = MENSAJE_DISTANCIA;
	  msg_dis.destinatario = MENSAJE_JUEGO;
		
    int flag;
    uint8_t key = 0;
    MyDev_Init();
    //printf("I2C Inicializado\n");
    osDelay(1000);
    VL6180x_InitData(dev);
    //printf("VL6180x Data Inicializado\n");
    VL6180x_Prepare(dev);
    //printf("VL6180x Preparado\n");
    do {
        if(key == 0){
          flag = FLAG_EMPIEZA_DIS;//osThreadFlagsWait(FLAG_EMPIEZA_DIS, osFlagsWaitAny, osWaitForever);
          osThreadFlagsClear(FLAG_EMPIEZA_DIS);
          key = (flag & FLAG_EMPIEZA_DIS) == FLAG_EMPIEZA_DIS ? 1 : 0;
					printf("[distancia::%s] key[%d]\n", __func__, key);
          flag = 0;
          
        } else {
          
          VL6180x_RangePollMeasurement(dev, &Range);
          //printf("VL6180x Detecta una medida\n");
          if (Range.errorStatus == 0 ){
              
              if (read_h == 0){
                printf("%d mm\n", Range.range_mm);
                msg_dis.mensaje[0] = Range.range_mm;
                status_cola=osMessageQueuePut(e_comPlacasTxMessageId, &msg_dis, 0U, 0U);
              }
              read_h = Range.range_mm;
          }else{
              read_h = 0;
              printf("Error\n");
          }
          printf("ESPERO FLAG JUEGO\n");
          flag = osThreadFlagsWait(FLAG_PARA_DIS, osFlagsWaitAny, 1000U);
					VL6180x_ClearErrorInterrupt(dev);
					VL6180x_Prepare(dev);
					printf("FLAG [%d] JUEGO RECIBIDA\n", flag);
          osThreadFlagsClear(FLAG_PARA_DIS);
          key = flag == FLAG_PARA_DIS ? 0 : 1;
          flag = 0;
         
          
        }
        
    } while (1);
    osThreadYield(); 

}

/* Para probaci�n
int ThSimDis(void){
    tid_ThSimDis = osThreadNew(dis_sim, NULL, NULL);
    if (tid_ThSimDis == NULL) {
        return(-1);
    }
    return(0);
}

void dis_sim(void* argument){
  int flag[2];
  //osDelay(5000);
  do{
    flag[0]=osThreadFlagsSet(tid_ThDistancia, FLAG_EMPIEZA_DIS);
    printf("dintance ditection on [%d]\n", flag[0]);
    osDelay(20000);
    flag[1] =osThreadFlagsSet(tid_ThDistancia, FLAG_PARA_DIS);
    printf("dintance ditection off [%d]\n", flag[1]);
    osDelay(3000);
  }while(1);
  osThreadYield(); 
}
*/
