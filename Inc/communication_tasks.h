/*
 * Tasks.h
 *
 *  Created on: 13 мар. 2018 г.
 *      Author: sibirsky
 */

#ifndef COMMUNICATION_TASKS_H_
#define COMMUNICATION_TASKS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "protocol.h"

#define CHECKSUM_ERROR 100

#define I2C_TASK_DOWNTIME 500/portTICK_RATE_MS
#define UART_QUEUE_TIMEOUT 100/portTICK_RATE_MS

/** Wrap for multitask error handling */
typedef struct
{
	uint32_t error;
	data_t data;
} message_t;

/** Queue for synchronizing I2C interrupts and I2C error handling */
QueueHandle_t i2cInterruptQueue;

/** Queue for transmitting data to first UART task */
QueueHandle_t uart1TxQueue;

/**
 * I2C communication task.
 * Parameters: I2C_HandleTypeDef
 */
void i2cTask(void *pvParameters);

/**
 * First UART task: computer communication
 * Parameters: UART_HandleTypeDef
 */
void uartTask1(void *pvParameters);

/**
 * Second UART task
 * Parameters: UART_HandleTypeDef
 * Not implemented yet!
 */
void uartTask2(void *pvParameters);

/** Tasks and queues initialization */
BaseType_t initTasks(I2C_HandleTypeDef* hi2c, UART_HandleTypeDef* huart1);

/** Blink led in case of I2C task critical error */
void i2cErrorAlert();

/** Blink led in case of UART task critical error */
void uart1ErrorAlert();


#endif /* COMMUNICATION_TASKS_H_ */
