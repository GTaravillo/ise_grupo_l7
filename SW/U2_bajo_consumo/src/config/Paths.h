#ifndef __PATHS_H
#define __PATHS_H

#define WORKSPACE 1

/* Headers */
#if WORKSPACE == 1		// Fabian
	#define PATH_MAIN        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/main.h"
	#define PATH_ADC         "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/adc/adc.h"
	#define PATH_COM_PLACAS  "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/com_placas/ComunicacionPlacas.h"
	#define PATH_NET_CONFIG  "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/config/NetConfig.h"
	#define PATH_DISTANCIA   "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/distancia/VL6180X/thDistancia.h"
	#define PATH_IRQ         "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/irq/stm32f4xx_it.h"
  #define PATH_NFC         "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/nfc/RC522.h"
	// #define PATH_UART        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U2_bajo_consumo/src/uart/uart.h"

#elif WORKSPACE == 2		// Yuanze
	#define PATH_MAIN        "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/main.h"
	#define PATH_ADC         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/adc/adc.h"
	#define PATH_COM_PLACAS  "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/com_placas/ComunicacionPlacas.h"
	#define PATH_NET_CONFIG  "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/config/NetConfig.h"
	#define PATH_DISTANCIA   "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/distancia/thDistancia.h"
	#define PATH_IRQ         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/irq/stm32f4xx_it.h"
  #define PATH_NFC         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/nfc/RC522.h"
	// #define PATH_UART        "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U2_bajo_consumo/src/uart/uart.h"

#elif WORKSPACE == 3		// Gonzalo
	#define PATH_ADC         "E:/repositorio_ISE/SW\U2_bajo_consumo/src/adc/adc.h"
	#define PATH_COM_PLACAS  "E:/repositorio_ISE/SW\U2_bajo_consumo/src/com_placas/ComunicacionPlacas.h"
	#define PATH_NET_CONFIG  "E:/repositorio_ISE/SW\U2_bajo_consumo/src/config/NetConfig.h"
	#define PATH_DISTANCIA   "E:/repositorio_ISE/SW\U2_bajo_consumo/src/distancia/thDistancia.h"
	#define PATH_IRQ         "E:/repositorio_ISE/SW\U2_bajo_consumo/src/irq/stm32f4xx_it.h"
  #define PATH_NFC         "E:/repositorio_ISE/SW\U2_bajo_consumo/src/nfc/RC522.h"
	// #define PATH_UART        "E:/repositorio_ISE/SW\U2_bajo_consumo/src/uart/uart.h"
#endif

#endif /* __PATHS_H */
