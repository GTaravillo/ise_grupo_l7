#include "thDistancia.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include <stdio.h>

extern osThreadId_t tid_ThDistancia;  
osThreadId_t tid_ThDistancia;
extern ARM_DRIVER_I2C Driver_I2C1;
void I2C_SignalEvent(uint32_t event);

ARM_DRIVER_I2C* I2Cdrv = &Driver_I2C1;
VL6180x_RangeData_t Range;


/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
int ThDistancia(void);
void Thread_Dis(void* argument);
/* Private functions ---------------------------------------------------------*/
void MyDev_Init(){
    I2Cdrv->Initialize(I2C_SignalEvent);
    I2Cdrv->PowerControl(ARM_POWER_FULL);
    I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
    
}

void I2C_SignalEvent(uint32_t event){
    uint8_t mask = ARM_I2C_EVENT_TRANSFER_DONE;
    if(event & mask){
        osThreadFlagsSet(tid_ThDistancia, 0x01);
    }
}
int ThDistancia(void){
    tid_ThDistancia = osThreadNew(Thread_Dis, NULL, NULL);
    if (tid_ThDistancia == NULL) {
        return(-1);
    }
    return(0);
}

void Thread_Dis(void* argument){
    uint8_t dev = 0x29;
    MyDev_Init();
    //printf("I2C Inicializado\n");
    osDelay(1000);
    VL6180x_InitData(dev);
    //printf("VL6180x Data Inicializado\n");
    VL6180x_Prepare(dev);
    //printf("VL6180x Preparado\n");
    do {
        VL6180x_RangePollMeasurement(dev, &Range);
        //printf("VL6180x Detecta una medida\n");
        if (Range.errorStatus == 0 ){
            printf("%d mm\n", Range.range_mm);
        }else{
            printf("Error");
        }
        
    } while (1);
    

}