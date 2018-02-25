/** Tested on Arduino Nano v3.0 */

#include <Wire.h>

uint8_t mode;

void setup() {
  mode = 10;
  pinMode(LED_BUILTIN, OUTPUT);
  randomSeed(analogRead(0));
  Serial.begin(9600);
  Wire.begin(8);
  Wire.onReceive(receive);
  Wire.onRequest(request);
}

void loop() {
  delay(100);
}

void receive(int howMany) {
  uint8_t parcel[2];
  
  while (Wire.available()) {
    parcel[0] = Wire.read();
    parcel[1] = Wire.read();
  }

  if (isChecksumm8bCorrect(parcel, 2)) {
    mode = parcel[0];
  } else {
    mode = 10;
    errorBlink();
  }
}

void request() {
    uint8_t sendParcel[3];
    switch (mode) {
      case 0: 
        sendParcel[0] = random(0, 10);
        sendParcel[1] = random(0, 10);
        break;
      case 1:
        sendParcel[0] = random(10, 20);
        sendParcel[1] = random(10, 20);
        break;
      case 2:
        sendParcel[0] = random(20, 30);
        sendParcel[1] = random(20, 30);
        break;
      default:
        sendParcel[0] = random(30, 100);
        sendParcel[1] = random(30, 100);
        break;
    }
    addChecksumm8b(sendParcel, 3);
    Wire.write(sendParcel, sizeof(sendParcel));
}

void errorBlink() {
  digitalWrite(LED_BUILTIN, HIGH);  
  delay(500);                       
  digitalWrite(LED_BUILTIN, LOW);   
  delay(500);                       
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


