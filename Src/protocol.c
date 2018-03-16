/*
 * protocol.c
 *
 *  Created on: 19 февр. 2018 г.
 *      Author: sibirsky
 */

#include "protocol.h"

void makeQueryParcel(query_t* query, uint8_t* parcel)
{
	parcel[0] = (*query).mode;
	addChecksumm8b(parcel, QUERY_PARCEL_LENGTH);
}

uint8_t decodeDataParcel(uint8_t* parcel, data_t* data)
{
	if (!isChecksumm8bCorrect(parcel, DATA_PARCEL_LENGTH))
	{
		return DECODE_CHECKSUM_ERROR;
	}

	(*data).x = parcel[0];
	(*data).y = parcel[1];
	return DECODE_NO_ERROR;
}

void addChecksumm8b(uint8_t *msg, uint16_t length)
{
    uint8_t crc = 0;
    int i = 0;

    for(i=0; i < length - 1; i++){
            crc ^= msg[i];
        }

    msg[length-1] = crc;
}

bool isChecksumm8bCorrect(uint8_t *msg, uint16_t length)
{
	uint8_t crcGot, crc = 0;
	int i;

	crcGot = msg[length-1] ;

		for(i=0; i < length - 1; i++){
			crc ^= msg[i];
		}

	if(crc == crcGot )
		return 1;
	else return 0;
}
