# ISE2025_grupoLAB_CastroPonceFabian

LCD - SPI1
    MOSI:       PB5  ->  mbed_05
    SCK:        PA5  ->  mbed_07 -> OJO! Si se pone PB3 no funciona printf tras inicialización SPI
    Reset:      PA6  ->  mbed_06
    A0:         PF13 ->  mbed_08
    nCS:        PD14 ->  mbed_11

ETH - RMII
    TXD0:       PG13
    TXD1:       PB13
    TX_EN:      PG11
    RXD0:       PC4
    RXD1:       PC5
    REF_CLK:    PA1
    CRS_DV:     PA7
    MDC:        PC1
    MDIO:       PA2

POT - ADC
    POT_1:      PA3  ->  mbed_19
    POT_2:      PC0  ->  mbed_20

Problemas
    Alarma RTC no se setea correctamente