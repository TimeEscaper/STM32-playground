#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "communication_tasks.h"


void i2cTask(void *pvParameters)
{
	typedef enum
	{
		QUERY,  // Sending query
		RECEIVE, // Receiving data
		PROCESS, // Processing data
		ERROR // Error handling
	} state_t;

	state_t currentState = QUERY;
	state_t afterErrorState = QUERY;
	uint32_t error = 0;

	I2C_HandleTypeDef* hi2c = (I2C_HandleTypeDef*)pvParameters;

	/** Shift is needed for using HAL I2C functions */
	uint16_t shiftedAddress = 8 << 1;

	/** Optimization to avoid re-creating the parcel */
	bool parcelReady = false;

	query_t query;
	uint8_t txBuffer[QUERY_PARCEL_LENGTH] = { 0 };
	uint8_t rxBuffer[DATA_PARCEL_LENGTH] = { 0 };
	query.mode = 0;

	for(;;)
	{
		/** if-else instead of switch for better code readability */
		if (currentState == QUERY)
		{
			if (!parcelReady)
			{
				if (query.mode > 2)
				{
					query.mode = 0;
				}
				makeQueryParcel(&query, txBuffer);
				parcelReady = true;
			}

			/** Start transmit */
			HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(hi2c, shiftedAddress, txBuffer, sizeof(txBuffer));
			if (status != HAL_OK)
			{
				if (status != HAL_BUSY)
				{
					error = HAL_I2C_GetError(hi2c);
					afterErrorState = QUERY;
					currentState = ERROR;
				}
				continue;
			}

			/** Transmit synchronization */
			uint32_t i2cErrorCode = 0;
			BaseType_t passed = xQueueReceive(i2cInterruptQueue, (void*)&i2cErrorCode, portMAX_DELAY);
			if (passed != pdPASS)
			{
				/** In theory, this case should never occur */
				continue;
			}
			if (i2cErrorCode != HAL_I2C_ERROR_NONE)
			{
				/** If slave can't acknowledge its address, we just restart
				 * transmitting or receiving */
				if (i2cErrorCode != HAL_I2C_ERROR_AF)
				{
					error = i2cErrorCode;
					afterErrorState = QUERY;
					currentState = ERROR;
				}
				continue;
			}

			currentState = RECEIVE;
		}

		else if (currentState == RECEIVE)
		{
			/** Start receive */
			HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(hi2c, shiftedAddress, rxBuffer, sizeof(rxBuffer));
			if (status != HAL_OK)
			{
				if (status != HAL_BUSY)
				{
					/** In case of error try to receive again */
					error = HAL_I2C_GetError(hi2c);
					afterErrorState = RECEIVE;
					currentState = ERROR;
				}
				continue;
			}

			/** Receive synchronization */
			uint32_t i2cErrorCode = 0;
			BaseType_t passed = xQueueReceive(i2cInterruptQueue, (void*)&i2cErrorCode, portMAX_DELAY);
			if (passed != pdPASS)
			{
				continue;
			}
			if (i2cErrorCode != HAL_I2C_ERROR_NONE)
			{
				if (i2cErrorCode != HAL_I2C_ERROR_AF)
				{
					error = i2cErrorCode;
					afterErrorState = QUERY;
					currentState = ERROR;
				}
				continue;
			}

			currentState = PROCESS;
		}

		else if (currentState == PROCESS)
		{
			/** Process received data */
			data_t data;
			if (decodeDataParcel(rxBuffer, &data) == DECODE_NO_ERROR)
			{
				message_t message;
				message.error = HAL_I2C_ERROR_NONE;
				message.data = data;

				BaseType_t passed = xQueueSend(uart1TxQueue, (void*)&message, UART_QUEUE_TIMEOUT);
				if (passed != pdPASS)
				{
					i2cErrorAlert();
				}
				else
				{
					/** Simulate delay just to see data on screen */
					vTaskDelay(I2C_TASK_DOWNTIME);
				}
				/** In this case we ignore failed queue pushing.
				 * Suppose that getting new data is more important.
				 * */
				query.mode++;
				parcelReady = false;

				currentState = QUERY;
			}
			else
			{
				error = CHECKSUM_ERROR;
				afterErrorState = QUERY;
				currentState = ERROR;
			}
		}

		else if (currentState == ERROR)
		{
			message_t message;
			message.error = error;

			BaseType_t passed = xQueueSend(uart1TxQueue, (void*)&message, UART_QUEUE_TIMEOUT);
			if (passed != pdPASS)
			{
				i2cErrorAlert();
			}

			/** Simulate delay just to see data on screen */
			vTaskDelay(I2C_TASK_DOWNTIME);

			currentState = afterErrorState;
		}
	}
}

void uartTask1(void *pvParameters)
{

	UART_HandleTypeDef* huart = (UART_HandleTypeDef*)pvParameters;

	message_t message;

	BaseType_t passed;
	HAL_StatusTypeDef status;

	for (;;)
	{
		passed = xQueueReceive(uart1TxQueue, (void*)&message, 0);
		if (passed == pdPASS)
		{
			char uartStr[20];
			switch (message.error)
			{
				case HAL_I2C_ERROR_NONE:
					sprintf(uartStr, "X: %u Y: %u\r\n", message.data.x, message.data.y);
					break;
				case CHECKSUM_ERROR:
					sprintf(uartStr, "Checksum error!\r\n");
					break;
				default:
					sprintf(uartStr, "Error code: %u\r\n", message.error);
					break;
			}

			status = HAL_UART_Transmit_DMA(huart, (uint8_t*)uartStr, strlen(uartStr));
			if (status != HAL_OK && status != HAL_BUSY)
			{
				/** TODO: Decide what to do in case of error */
				uart1ErrorAlert();
			}
		}
		else
		{
			taskYIELD();
		}
	}
}

BaseType_t initTasks(I2C_HandleTypeDef* hi2c, UART_HandleTypeDef* huart1)
{
	i2cInterruptQueue = xQueueCreate(1, sizeof(uint32_t));
	uart1TxQueue = xQueueCreate(1, sizeof(message_t));

	BaseType_t passed = xTaskCreate(i2cTask, "TASK_I2C", configMINIMAL_STACK_SIZE, (void*)hi2c, tskIDLE_PRIORITY+2, NULL);
	if (passed != pdPASS)
	{
		return pdFAIL;
	}
	return xTaskCreate(uartTask1, "TASK_UART_1", configMINIMAL_STACK_SIZE, (void*)huart1, tskIDLE_PRIORITY+1, NULL);
}

void i2cErrorAlert()
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13); // Orange
	HAL_Delay(50);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
}

void uart1ErrorAlert()
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); // Red
	HAL_Delay(50);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c)
{
	uint32_t errorCode = HAL_I2C_GetError(hi2c);
	BaseType_t taskWoken = pdFALSE;
	xQueueSendFromISR(i2cInterruptQueue, (void*)&errorCode, &taskWoken);
	if (taskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(taskWoken);
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c)
{
	uint32_t errorCode = HAL_I2C_GetError(hi2c);
	BaseType_t taskWoken = pdFALSE;
	xQueueSendFromISR(i2cInterruptQueue, (void*)&errorCode, &taskWoken);
	if (taskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(taskWoken);
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c)
{
	uint32_t errorCode = HAL_I2C_GetError(hi2c);
	BaseType_t taskWoken = pdFALSE;
	xQueueSendFromISR(i2cInterruptQueue, (void*)&errorCode, &taskWoken);
	if (taskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(taskWoken);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
	uart1ErrorAlert();
}
