#ifndef OBS_PROTOCOL_H_
#define OBS_PROTOCOL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * Number of OBS graphic points,
 * transmitted in one message.
 *
 * Value defined in MATE mission.
 */
#define OBS_POINTS_NUMBER 16

/**
 * Size of the parcel that can hold
 * both types of messages: position
 * and points.
 */
#define COMMON_PARCEL_LENGTH 66

/** Size for parcel with position message */
#define POSITION_PARCEL_SIZE 14

/** Size for parcel with points message */
#define PARCEL_SIZE_POINTS 66

/** NPE checks for better reliability */
#define NULL_POINTER_ERROR 1

/** If message type cannot be resolved (broken parcel) */
#define UNDEFINED_TYPE_ERROR 2

/** If checksum in parcel is wrong */
#define CHECKSUM_ERROR 3

/** Types of the OBS messages */
typedef enum
{
	POSITION,
	POINTS
} obs_message_type_t;

/** OBS position message */
typedef struct
{
	float x;
	float y;
	float voltage;
} obs_position_t;

/** OBS graphic points message */
typedef struct
{
	float points[OBS_POINTS_NUMBER];
} obs_points_t;


/** Determine message type of received parcel */
uint8_t resolveMessageType(uint8_t *parcel, obs_message_type_t *type);

/** Decode OBS position message from parcel */
uint8_t decodePositionMessage(uint8_t *parcel, obs_position_t *message);

/** Decode OBS points message from parcel */
uint8_t decodePointsMessage(uint8_t *parcel, obs_points_t *message);

/** Checksum */
bool isChecksumm8bCorrect(uint8_t *msg, uint16_t length);


#endif /* OBS_PROTOCOL_H_ */
