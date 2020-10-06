/*
 * dht11.h
 *
 *  Created on: Oct 5, 2020
 *      Author: Training
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void DHT11_Start (void);

uint8_t DHT11_Check_Response (void);
uint8_t DHT11_Read (void);

#endif /* INC_DHT11_H_ */



