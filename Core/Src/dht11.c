/*
 * dht11.c
 *
 *  Created on: Oct 5, 2020
 *      Author: Training
 */


#include "main.h"
#include <stdint.h>


/**
  * @brief    setting GPIO pin as output
  * @param 1  GPIO port used
  * @param 2  pin used in particular GPIO port
  * @retval None
  */


void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


/**
  * @brief    setting GPIO pin as input
  * @param 1  GPIO port used
  * @param 2  pin used in particular GPIO port
  * @retval None
  */
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


/**
  * @brief    starting dht11 sensor by setting paricular pin as output intially and then as input
  * @param 1  No parameter
  * @retval None
  */
void DHT11_Start (void)
{
Set_Pin_Output (dht11_GPIO_Port, dht11_Pin);
HAL_GPIO_WritePin (dht11_GPIO_Port, dht11_Pin, DISABLE);
delay_microsecond (18000);
HAL_GPIO_WritePin(dht11_GPIO_Port, dht11_Pin, ENABLE);
delay_microsecond(20);
Set_Pin_Input(dht11_GPIO_Port, dht11_Pin);
}


/**
  * @brief    Checking response or output from the dht11 sensor
  * @param 1  No parameter
  * @retval	  uint8_t response either 1 or -1
  */
uint8_t DHT11_Check_Response (void)
{
uint8_t Response = 0;
delay_microsecond (40);
if (!(HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)))
{
delay_microsecond (80);
if ((HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)))
Response = 1;
else
Response = -1;
}
while ((HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)));
return Response;
}


/**
  * @brief    Now starting to read from the dht11 sensor
  * @param 1  No parameter
  * @retval	  uint8_t data of 1 byte from the sensor
  */
uint8_t DHT11_Read (void)
{
uint8_t i,j;
for (j=0;j<8;j++)
{
while (!(HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)));
delay_microsecond (40);
if (!(HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)))
{
i&= ~(1<<(7-j));
}
else
{
i|= (1<<(7-j));
}
while ((HAL_GPIO_ReadPin (dht11_GPIO_Port, dht11_Pin)));
}
return i;
}
