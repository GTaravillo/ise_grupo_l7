#include "stm32f4xx_hal.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void WakeUpOtherBoard(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {
        // Simulate wake-up trigger (e.g., button press or timed pulse)
        WakeUpOtherBoard();
    }
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PD15 as push-pull output
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void WakeUpOtherBoard(void)
{
    // Send a rising edge pulse to PA0 of Board A
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_Delay(10);  // Keep it low
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);  // Rising edge
}