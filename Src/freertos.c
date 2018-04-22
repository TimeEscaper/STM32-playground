/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include <string.h>
#include <stdlib.h>
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "obs_protocol.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId i2cTaskHandle;
osMessageQId i2cInterruptQueueHandle;

/* USER CODE BEGIN Variables */
uint8_t i2cRxBuffer[COMMON_PARCEL_LENGTH] = { 0 };
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartI2CTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void processParcel(const uint8_t *parcel);
void logHalErrorUart(int8_t errorCode);
void logProtocolErrorUart(int8_t errorCode);
void sendPositionUart(const obs_position_t *position);
void sendPointsUart(const obs_points_t *points);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of i2cTask */
  osThreadDef(i2cTask, StartI2CTask, osPriorityHigh, 0, 128);
  i2cTaskHandle = osThreadCreate(osThread(i2cTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of i2cInterruptQueue */
  osMessageQDef(i2cInterruptQueue, 1, uint32_t);
  i2cInterruptQueueHandle = osMessageCreate(osMessageQ(i2cInterruptQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* StartI2CTask function */
void StartI2CTask(void const * argument)
{
  /* USER CODE BEGIN StartI2CTask */
  uint16_t bufferSize = sizeof(i2cRxBuffer);
  uint32_t i2cErrorCode = 0;
  /* Infinite loop */
  for(;;)
  {
	  HAL_StatusTypeDef status = HAL_I2C_Slave_Receive_DMA(&hi2c1, i2cRxBuffer, bufferSize);
	  if (status == HAL_OK)
	  {
		  BaseType_t passed = xQueueReceive(i2cInterruptQueueHandle, (void*)&i2cErrorCode, portMAX_DELAY);
		  if (passed != pdPASS)
		  {
			  continue;
		  }

		  if (i2cErrorCode == HAL_I2C_ERROR_NONE || i2cErrorCode == HAL_I2C_ERROR_AF)
		  {
			  processParcel(i2cRxBuffer);
		  }
		  else
		  {
			  logHalErrorUart(i2cErrorCode);
		  }
	  }
  }
  /* USER CODE END StartI2CTask */
}

/* USER CODE BEGIN Application */
void processParcel(const uint8_t *parcel)
{
	obs_message_type_t messageType;
	uint8_t err = resolveMessageType(parcel, &messageType);
	if (err != NO_ERROR)
	{
		logProtocolErrorUart(err);
		return;
	}

	if (messageType == POSITION)
	{
		obs_position_t position;
		err = decodePositionMessage(parcel, &position);
		if (err != NO_ERROR)
		{
			logProtocolErrorUart(err);
			return;
		}
		sendPositionUart(&position);
	}
	else
	{
		obs_points_t points;
		err = decodePointsMessage(parcel, &points);
		if (err != NO_ERROR)
		{
			logProtocolErrorUart(err);
			return;
		}
		sendPointsUart(&points);
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* hi2c)
{
	uint32_t errorCode = HAL_I2C_GetError(hi2c);
	BaseType_t taskWoken = pdFALSE;
	xQueueSendFromISR(i2cInterruptQueueHandle, (void*)&errorCode, &taskWoken);
	if (taskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(taskWoken);
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c)
{
	uint32_t errorCode = HAL_I2C_GetError(hi2c);
	BaseType_t taskWoken = pdFALSE;
	xQueueSendFromISR(i2cInterruptQueueHandle, (void*)&errorCode, &taskWoken);
	if (taskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(taskWoken);
	}
}

void logHalErrorUart(int8_t errorCode)
{
	char messageStr[30];
	sprintf(messageStr, "HAL error code: %d\r\n", errorCode);
	HAL_UART_Transmit(&huart4, (uint8_t*)messageStr, strlen(messageStr), 1000);
}

void logProtocolErrorUart(int8_t errorCode)
{
	char messageStr[30];
	sprintf(messageStr, "Protocol error code: %d\r\n", errorCode);
	HAL_UART_Transmit(&huart4, (uint8_t*)messageStr, strlen(messageStr), 1000);
}

void sendPositionUart(const obs_position_t *position)
{
	char messageStr[30];
	sprintf(messageStr, "X: %f; Y: %f; Voltage: %f;\r\n", position->x, position->y, position->voltage);
	HAL_UART_Transmit(&huart4, (uint8_t*)messageStr, strlen(messageStr), 1000);
}

void sendPointsUart(const obs_points_t *points)
{
	char messageStr[30];
	sprintf(messageStr, "1: %f; 7: %f; 15: %f;\r\n", points->points[0], points->points[6], points->points[15]);
	HAL_UART_Transmit(&huart4, (uint8_t*)messageStr, strlen(messageStr), 1000);
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
