/*
 * ultrasonic.c
 */

#include "ultrasonic.h"

extern TIM_HandleTypeDef htim2;

static void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while(__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void HCSR04_Init(void)
{
    HAL_TIM_Base_Start(&htim2);
}

float HCSR04_Read(uint8_t sensor)
{
    GPIO_TypeDef *trigPort = GPIOB;
    GPIO_TypeDef *echoPort = GPIOB;

    uint16_t trigPin;
    uint16_t echoPin;

    if(sensor == 0)
    {
        trigPin = GPIO_PIN_0;
        echoPin = GPIO_PIN_1;
    }
    else if(sensor == 1)
    {
        trigPin = GPIO_PIN_2;
        echoPin = GPIO_PIN_3;
    }
    else
    {
        trigPin = GPIO_PIN_4;
        echoPin = GPIO_PIN_5;
    }

    uint32_t timeout;
    uint32_t start;
    uint32_t end;
    uint32_t pulse;
    float distance;

    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
    delay_us(10);

    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

    timeout = HAL_GetTick();

    while(HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET)
    {
        if(HAL_GetTick() - timeout > 30)
            return 400.0f;
    }

    start = __HAL_TIM_GET_COUNTER(&htim2);

    timeout = HAL_GetTick();

    while(HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET)
    {
        if(HAL_GetTick() - timeout > 30)
            return 400.0f;
    }

    end = __HAL_TIM_GET_COUNTER(&htim2);

    if(end >= start)
        pulse = end - start;
    else
        pulse = (65535 - start) + end;

    distance = (pulse * 0.0343f) / 2.0f;

    if(distance < 2.0f)
        distance = 2.0f;

    if(distance > 400.0f)
        distance = 400.0f;

    return distance;
}

void HCSR04_ReadAll(float distance[3])
{
    distance[0] = HCSR04_Read(0);
    delay_us(50);

    distance[1] = HCSR04_Read(1);
    delay_us(50);

    distance[2] = HCSR04_Read(2);
}
