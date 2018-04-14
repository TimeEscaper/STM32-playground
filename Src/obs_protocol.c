#include "obs_protocol.h"

uint8_t resolveMessageType(uint8_t *parcel, obs_message_type_t *type)
{
	if (!parcel || !type)
	{
		return NULL_POINTER_ERROR;
	}

	uint8_t typeFlag = parcel[0];
	switch (typeFlag)
	{
		case 0:
			*type = POSITION;
			return 0;

		case 1:
			*type = POINTS;
			return 0;

		default:
			return UNDEFINED_TYPE_ERROR;
	}
}

uint8_t decodePositionMessage(uint8_t *parcel, obs_position_t *message)
{
	if (!parcel || !message)
	{
		return NULL_POINTER_ERROR;
	}
	if (!isChecksumm8bCorrect(parcel, POSITION_PARCEL_SIZE))
	{
		return CHECKSUM_ERROR;
	}
	/**TODO: Add type check? */

	message->x = readFloat(&parcel[1]);
	message->y = readFloat(&parcel[5]);
	message->voltage = readFloat(&parcel[9]);

	return 0;
}

uint8_t decodePointsMessage(uint8_t *parcel, obs_points_t *message)
{
	if (!parcel || !message)
	{
		return NULL_POINTER_ERROR;
	}
	if (!isChecksumm8bCorrect(parcel, PARCEL_SIZE_POINTS))
	{
		return CHECKSUM_ERROR;
	}
	/**TODO: Add type check? */

	int i;
	int index = 1;
	for (i = 0; i < OBS_POINTS_NUMBER; i++)
	{
		message->points[i] = readFloat(&parcel[index]);
		index += 4;
	}

	return 0;
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

float readFloat(uint8_t *p)
{
	union
	{
	  float floatValue;
	  uint8_t bytes[4];
	} float_union;

	float_union.bytes[0] = *(p++);
	float_union.bytes[1] = *(p++);
	float_union.bytes[2] = *(p++);
	float_union.bytes[3] = *(p++);

	return float_union.floatValue;
}
