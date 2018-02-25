/*
 * protocol.h
 *
 *  Created on: 19 февр. 2018 г.
 *      Author: sibirsky
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define QUERY_PARCEL_LENGTH 2
#define DATA_PARCEL_LENGTH 3

#define DECODE_NO_ERROR 0
#define DECODE_CHECKSUM_ERROR 1

/** Query for asking slave to send data */
typedef struct
{
	uint8_t mode;
} query_t;

/** Data example (coordinates) */
typedef struct
{
	uint8_t x;
	uint8_t y;
} data_t;

void makeQueryParcel(query_t* query, uint8_t* parcel);

uint8_t decodeDataParcel(uint8_t* parcel, data_t* data);

void addChecksumm8b(uint8_t *msg, uint16_t length);

bool isChecksumm8bCorrect(uint8_t *msg, uint16_t length);

#endif /* PROTOCOL_H_ */
