/** Tested on Arduino Nano v3.0 */

#include <Wire.h>

/** Parcel type: OBS position or data points */
#define PARCEL_POSITION 0
#define PARCEL_POINTS 1

/** Parcel size in bytes */
#define PARCEL_SIZE_POSITION 14
#define PARCEL_SIZE_POINTS 66

/**
 * Parcel format:
 * 
 * 1) For OBS position:
 * [1 byte type] [4 byte X] [4 byte Y] [4 byte Voltage] [1 byte checksum]
 * 
 * 2) For data apoints:
 * [1 byte type] 16x[4 byte amplitude] [1 byte checksum]
 * 
 * Multibyte values are represented in format: [HI to LO]
 */

/** Sample device address */
#define SLAVE_ADDRESS 15

typedef union {
  float floatValue;
  uint8_t bytes[4];
} float_union_t;

int counter;
int counterMax;

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  delay(100);
}

void makePositionParcel(uint8_t *parcel) {
  parcel[0] = PARCEL_POSITION;
  int index = 1;
  for (int i = 0; i < 3; i++) {
    float value = random(0, 100) / 100.0;
    appendFloat(parcel, value, index);
    index += 4;
  }
  addChecksumm8b(parcel, PARCEL_SIZE_POSITION);
}

void makePointsParcel(uint8_t *parcel) {
  parcel[0] = PARCEL_POINTS;
  int index = 1;
  for (int i = 0; i < 15; i++) {
    float value = random(0, 100) / 100.0;
    appendFloat(parcel, value, index);
    index += 4;
  }
  addChecksumm8b(parcel, PARCEL_SIZE_POINTS);
}

void appendFloat(uint8_t *arr, float value, int index) {
    float_union_t unionValue;
    unionValue.floatValue = value;
    arr[index] = unionValue.bytes[0];
    arr[index+1] = unionValue.bytes[1];
    arr[index+2] = unionValue.bytes[2];
    arr[index+3] = unionValue.bytes[3];
}

void addChecksumm8b(uint8_t *msg, uint16_t length) {
    uint8_t crc = 0;
    int i = 0;

    for(i=0; i < length - 1; i++){
            crc ^= msg[i];
        }

    msg[length-1] = crc;
}

