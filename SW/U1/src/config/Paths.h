#ifndef __PATHS_H
#define __PATHS_H

#define WORKSPACE 1

/* Headers */
#if WORKSPACE == 1		// Fabian
	#define PATH_MAIN       "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/main.h"
	#define PATH_SERVER     "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/servidor/servidor.h"
	#define PATH_ADC        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/adc/adc.h"
	#define PATH_BUTTONS    "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/buttons/buttons.h"
	#define PATH_NET_CONFIG "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/config/NetConfig.h"
	#define PATH_IRQ        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/irq/stm32f4xx_it.h"
	#define PATH_LCD        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/lcd/lcd.h"
	#define PATH_LED        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/led/led.h"
	#define PATH_LED_STRIP  "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/led/LedStripManager.h"
	#define PATH_RTC        "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/rtc/rtc.h"
	#define PATH_UART       "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/uart/uart.h"
	#define PATH_POSITION   "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/posicion/PositionManager.h"
	#define PATH_MEMORIA    "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/memoria/Memoria.h"
	#define PATH_TEST_MEMORIA    "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/memoria/TestMemoria.h"
	#define PATH_COM_PLACAS "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/com_placas/ComunicacionPlacas.h"
	#define PATH_PRINCIPAL  "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/juego/principal.h"
	#define PATH_TABLERO		"C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/juego/tablero.h"
	#define PATH_COMMON		  "C:/Users/Fabian/Desktop/workspace/UPM/Laboratorio/ISE/Bloque_II/ise_grupo_l7/SW/U1/src/common/CommonDefines.h"

#elif WORKSPACE == 2		// Yuanze

	#define PATH_MAIN        "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/main.h"
	#define PATH_SERVER      "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/servidor/server.h"
	#define PATH_ADC         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/adc/adc.h"
	#define PATH_BUTTONS     "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/buttons/buttons.h"
	#define PATH_NET_CONFIG  "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/config/NetConfig.h"
	#define PATH_IRQ         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/irq/stm32f4xx_it.h"
	#define PATH_LCD         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/lcd/lcd.h"
	#define PATH_LED         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/led/led.h"
	#define PATH_LED_STRIP   "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/led/LedStripManager.h"
	#define PATH_RTC         "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/rtc/rtc.h"
	#define PATH_UART        "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/uart/uart.h"
	#define PATH_POSITION    "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/posicion/PositionManager.h"
	#define PATH_MEMORIA     "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/memoria/Memoria.h"
	#define PATH_COM_PLACAS  "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/com_placas/ComunicacionPlacas.h"
	#define PATH_PRINCIPAL   "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/juego/principal.h"
	#define PATH_TABLERO		 "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/juego/tablero.h"
	#define PATH_COMMON			 "D:/Estudio/ISE/Bloq2/ise_grupo_l7/SW/U1/src/common/CommonDefines.h"

#elif WORKSPACE == 3		// Raul
	#define PATH_MAIN        "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/main.h"
	#define PATH_SERVER      "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/servidor/server.h"
	#define PATH_ADC         "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/adc/adc.h"
	#define PATH_BUTTONS     "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/buttons/buttons.h"
	#define PATH_NET_CONFIG  "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/config/NetConfig.h"
	#define PATH_IRQ         "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/irq/stm32f4xx_it.h"
	#define PATH_LCD         "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/lcd/lcd.h"
	#define PATH_LED         "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/led/led.h"
	#define PATH_LED_STRIP   "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/led/LedStripManager.h"
	#define PATH_RTC         "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/rtc/rtc.h"
	#define PATH_UART        "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/uart/uart.h"
	#define PATH_POSITION    "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/posicion/PositionManager.h"
	#define PATH_MEMORIA     "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/memoria/Memoria.h"
	#define PATH_COM_PLACAS  "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/com_placas/ComunicacionPlacas.h"
	#define PATH_PRINCIPAL   "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/juego/principal.h"
	#define PATH_TABLERO		 "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/juego/tablero.h"
	#define PATH_COMMON		 	 "C:/Users/Olivia/Desktop/UNIVERSIDAD/10o_SEMESTRE/ISE/MASTERCHESS/ise_grupo_l7/SW/U1/src/common/CommonDefines.h"

#endif

#endif /* __PATHS_H */
