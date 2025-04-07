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
  
LCD - SPI1  
    MOSI:       PB5  ->  mbed_05  
    SCK:        PA5  ->  mbed_07 -> OJO! Si se pone PB3 no funciona printf tras inicialización SPI  
    Reset:      PA6  ->  mbed_06  
    A0:         PF13 ->  mbed_08  
    nCS:        PD14 ->  mbed_11  
  
Led Strip - SPI2  
    MOSI:       PB15 ->  DATA  
    SCK:        PB13 ->  CLK  
  
ETH - RMII  
    TXD0:       PG13  
    TXD1:       PG14  
    TX_EN:      PG11  
    RXD0:       PC4  
    RXD1:       PC5  
    REF_CLK:    PA1  
    CRS_DV:     PA7  
    MDC:        PC1  
    MDIO:       PA2  
  
Posicion (pruebas) - I2C1  
    SDA:        PB8  
    SCL:        PB9  
  
Problemas a solucionar  
    Mejorar gestión eventos RTC (se salta algunos segundos)  