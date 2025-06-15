# ISE2025_SW  
  
  
Environment:  
    Xtal: 8MHz  
    ARM Compiler: default compiler version 6  
    Core Clock:
  
    HSE_VALUE         =  8MHz  
	PLLM              =  8    [0,63]      ->  8MHz/8    =  1MHz  
	PLLN              =  336  [50,432]    ->  1MHz*336  =  336MHz  
	PLLP              =  RCC_PLLP_DIV2    ->  336MHz/2  =  168MHz  = SystemCoreClock  
	AHBCLKDivider     =  RCC_SYSCLK_DIV1  ->  168MHz  
      APB1CLKDivider  =  RCC_HCLK_DIV4    ->  168MHz/4  =  42MHz   = APB1 peripheral clocks (max 45MHz)  
                                              42MHz*2   =  84MHz   = APB1 Timer clocks  
      APB2CLKDivider  =  RCC_HCLK_DIV2    ->  168MHz/2  =  84MHz   = APB2 peripheral clocks (max 90MHz)  
                                              84MHz*2   =  168MHz  = APB2 Timer clocks  
  
Printf:         PB3  
  
## <ins>U1</ins>  
  
### LCD  
    **SPI1**  
    MOSI:       PB5  ->  mbed_05  
    SCK:        PA5  ->  mbed_07  
    Reset:      PA6  ->  mbed_06  
    A0:         PF13 ->  mbed_08  
    nCS:        PD14 ->  mbed_11  
  
### LEDs RGB  
    **SPI2**  
    MOSI:       PB15 ->  DATA  
    SCK:        PD3  ->  CLK  
  
### Servidor Web / RTC
    **ETH**  
    RMII  
        TXD0:       PG13  (NO SE PUEDE CAMBIAR)  
        TXD1:       PB13  (NO SE PUEDE CAMBIAR)  
        TX_EN:      PG11  (NO SE PUEDE CAMBIAR)  
        RXD0:       PC4   (NO SE PUEDE CAMBIAR)  
        RXD1:       PC5   (NO SE PUEDE CAMBIAR)  
        REF_CLK:    PA1   (NO SE PUEDE CAMBIAR)  
        CRS_DV:     PA7   (NO SE PUEDE CAMBIAR)  
    Management Data Interface  
        MDC:        PC1   (NO SE PUEDE CAMBIAR)  
        MDIO:       PA2   (NO SE PUEDE CAMBIAR)  
  
### Expansor GPIO - Posicion
    **I2C1**  
    SDA:        PB8  
    SCL:        PB9  
  
### Memoria  
    **I2C2**  
    SCL         PF1  
    SDA         PF0  
  
### Comunicación entre placas  
    **UART7**  
    TX:         PF7  
    RX:         PE7

  
## <ins>U2 - Bajo Consumo</ins>  
  
### Distancia  
    **I2C2**  
    SCL         PF1  
    SDA         PF0  
  
### Comunicación entre placas  
    **UART7**  
    TX:         PF7  
    RX:         PE7
  
### RFID/NFC  
    **SPI3**  
    nCS(SDA):   PA15
    SCK:        PC10
    MISO:       PC11
    MOSI:       PC12
  
### Alimentación  
    ADC:        PC0  
  
### Ruido  
    ADC:        PC3
  
  
# Problemas a solucionar  
  
## RTC  
    Mejorar gestión eventos RTC (se salta algunos segundos)  
    Corregir gestión hora SNTP: a las 01:00:00 se muestra 25:00:00 cuando lo coge del servidor  
