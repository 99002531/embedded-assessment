#include "main.h"
#include "dht11.h"
#include<string.h>



ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;


 struct sensorvariables
{
volatile uint8_t humidity_first_byte,humidity_second_byte,temperature_first_byte,temperature_second_byte,response;
volatile uint16_t sum,humidity_raw,temperature_raw;
volatile int Temperature,Humidity;
}sensordht11var;



 struct potentiometer_varibles
{
volatile uint16_t pot_adc_conversion,pot_adc_value;

}potvar;


volatile uint8_t FLAG;
volatile uint8_t counter=0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);

/**
  * @brief  sending time value to timer register that should be in the micro seconds
  * @param  time that should be in micro seconds
  * @retval None
  */


void delay_microsecond(uint16_t  time)
{
__HAL_TIM_SET_COUNTER(&htim6, 0);
while ((__HAL_TIM_GET_COUNTER(&htim6))<time);
}

/**
  * @brief  setting FLAG value as one whenever interrupt invoked
  * @param  sending the GPIO pin number
  * @retval None
  */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
FLAG = 1;
}

uint16_t adcvalue=0;
int main(void)
{

  HAL_GPIO_WritePin(slave_select_GPIO_Port, slave_select_Pin, SET);  //making slave select high
  HAL_Init();


  SystemClock_Config();


  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();

  HAL_TIM_Base_Start(&htim6);  //starting timer6 for dht11 sensor

  while (1)
  {
 HAL_ADC_Start(&hadc1);    //Adc is starting
 if(HAL_ADC_PollForConversion(&hadc1, 10)==HAL_OK)   //checking Adc conversion is complete or not
{
potvar.pot_adc_value=HAL_ADC_GetValue(&hadc1);       //asssigning Adc value to a variable using Getvalue function
HAL_Delay(100);
}

 if(FLAG==1)			//checking if interrupt is invoked
 {
 HAL_GPIO_TogglePin(orange_led_GPIO_Port, orange_led_Pin);   //make orange_led_Pin toggle
 HAL_Delay(50);
 FLAG=0;													//reseting the FLAG
 counter=counter+1;											//TO store how many times interrupt is invoked
 }


 if(counter%2==1  &&  potvar.pot_adc_value>512)				//if counter value is odd and pot value is > 512
 {

 HAL_GPIO_TogglePin(blue_led_GPIO_Port, blue_led_Pin);		//toggle blue_led_Pin
 DHT11_Start();												//starting dht11 sensor
 sensordht11var.response= DHT11_Check_Response();			//waiting for acknowledgement from the sensor
 HAL_GPIO_WritePin(slave_select_GPIO_Port, slave_select_Pin, DISABLE);    //slave select make it as low

 if(sensordht11var.response==1)								//checking resonse
 {
 sensordht11var.humidity_first_byte = DHT11_Read();		//reading particular dht11 sensor connect pin for 40 micro secs to read humidity
 sensordht11var.humidity_second_byte=   DHT11_Read();	//same in hexadecimsl format
 sensordht11var.temperature_first_byte= DHT11_Read();	//reading particular dht11 sensor connect pin for 40 micro secs to read temperature
 sensordht11var.temperature_second_byte= DHT11_Read();	//same in hexadecimsl format
 sensordht11var.sum= DHT11_Read();						// adds all bytes in one variable
 sensordht11var.humidity_raw= sensordht11var.humidity_first_byte;						//stores humidity
 sensordht11var.temperature_raw= sensordht11var.temperature_first_byte;					//stores temperature
 sensordht11var.Temperature= (int) sensordht11var.temperature_raw;
 sensordht11var.Humidity= (int)sensordht11var.humidity_raw;
 uint8_t Humidity_temp= (uint8_t)sensordht11var.Temperature;
 uint8_t Temperature_temp= (uint8_t)sensordht11var.Humidity;

 HAL_SPI_Transmit(&hspi1, &Humidity_temp, 1, 10);							//SPI transmission of humidity
 HAL_Delay(100);
 }

 }
 else if (counter %2 !=0 && potvar.pot_adc_value< 512)					//if counter value is odd and pot value is less than 512
 {
 HAL_GPIO_TogglePin(green_led_GPIO_Port, green_led_Pin);				//toggle green led
 HAL_SPI_Transmit(&hspi1, 0, 1, 10);									//transmit zero indicating pot value is less than 512
 HAL_Delay(100);
 }

 else
 {
 HAL_SPI_Abort(&hspi1);													//do not SPI transmission
 }



  }
  return 0;

}




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}


static void MX_ADC1_Init(void)
{



  ADC_ChannelConfTypeDef sConfig = {0};


  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_10B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }


}


static void MX_SPI1_Init(void)
{


  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }


}


static void MX_TIM6_Init(void)
{



  TIM_MasterConfigTypeDef sMasterConfig = {0};


  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 50-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xffff-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }


}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, slave_select_Pin|dht11_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, green_led_Pin|orange_led_Pin|red_led_Pin|blue_led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : button_Pin */
  GPIO_InitStruct.Pin = button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : slave_select_Pin dht11_Pin */
  GPIO_InitStruct.Pin = slave_select_Pin|dht11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : green_led_Pin orange_led_Pin red_led_Pin blue_led_Pin */
  GPIO_InitStruct.Pin = green_led_Pin|orange_led_Pin|red_led_Pin|blue_led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
