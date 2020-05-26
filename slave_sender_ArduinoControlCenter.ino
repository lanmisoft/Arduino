// Wire Slave Receiver & Sender
// by Lanmisoft <https://lanmisoft.com>

// Demonstrates use of the Wire library

// Created 21.04.2020

// This example code is in the public domain.
//#define DEBUG_MODE_ON
#include <Wire.h>

//===CHANGE HERE VALUES COUNT FOR SENDING!!!!=========================
uint8_t VALUES_TO_SEND_COUNT = 8;

//======CHANGE HEX address to match in Arduino Control Center!!!!
uint8_t HEX_ADRRESS = 4; 

const uint8_t CRC  = 2;
const uint8_t FLOAT_BYTE_SIZE = 4;
uint8_t **values_array;
bool pinStatus, pinStatus2;
static volatile uint8_t arrayIndex;

void setup() {
  Serial.begin(19200);
  pinMode(13, OUTPUT); //just a example
  pinStatus = digitalRead(13);
  pinMode(12, OUTPUT);
  pinStatus = digitalRead(12);//just a example
  InitializeArray();
  Wire.begin(HEX_ADRRESS); // join i2c bus with address HEX_ADRRESS 4
  Wire.onRequest(requestEvent); // register request event
  Wire.onReceive(receiveEvent); // register receive event
}

void loop() {
  //Here change value as you need it, data from sensors, pin statuses, etc...
  //any number value(int, bool, word)  will be converted to float (4 byte) array value
  float value;
  uint8_t index;
  value = 37.19;
  index = 0;//first value has index 0
  writeValuesAsFloatByteArray(value, index);
  digitalWrite(13, pinStatus);
  value = digitalRead(13);
  index = 1;
  writeValuesAsFloatByteArray(value, index); //VALUES_TO_SEND_COUNT = 2
  value = 487.1;
  index = 3;
  writeValuesAsFloatByteArray(value, index);
  value = 21.15;
  index = 4;
  writeValuesAsFloatByteArray(value, index);
  digitalWrite(12, pinStatus2);
  value = digitalRead(12);
  index = 4;
  writeValuesAsFloatByteArray(value, index);
  value = digitalRead(13);
  index = 5;
  writeValuesAsFloatByteArray(value, index);

  value = 13554.11;
  index = 6;
  writeValuesAsFloatByteArray(value, index);

  value = 216.16;
  index = 7;
  //VALUES_TO_SEND_COUNT = 8
  //if you want to send more, change value in header
  writeValuesAsFloatByteArray(value, index);

  if (pinStatus)pinStatus = false; else pinStatus = true;
  if (pinStatus2)pinStatus2 = false; else pinStatus2 = true;
  delay(1000);
}


//===========================functions required for sending values===========================
void receiveEvent(int received) {
  arrayIndex = Wire.read();   // receive byte as an uint8_t
 }

void requestEvent() {
  uint8_t totalarray[FLOAT_BYTE_SIZE + CRC];
  uint8_t valuepos;
  if (arrayIndex < VALUES_TO_SEND_COUNT) {
    uint8_t *valuearray  = (uint8_t*)&values_array[arrayIndex][0];
    uint16_t crc_out = calc_crc((uint8_t *)valuearray, FLOAT_BYTE_SIZE);
   // for each value is peformed CRC check so you can be sure
   //that same value will be received in Master arduino controlled with Arduino Control Center
    totalarray[0] = highByte(crc_out);
    totalarray[1] = lowByte(crc_out);

    for (valuepos = 2; valuepos < FLOAT_BYTE_SIZE + CRC; valuepos++)
      totalarray[valuepos] = valuearray[valuepos - 2];

#ifdef DEBUG_MODE_ON
    float  floatFromBytes = *(float *)&valuearray[0];
    Serial.print("floatFromBytes "); Serial.println(floatFromBytes);
    Serial.print("arrayIndex "); Serial.println(arrayIndex);
    Serial.print("crc_out "); Serial.println(crc_out);
    for (int i = 0; i < FLOAT_BYTE_SIZE + CRC; i++) {
      Serial.print(totalarray[i]); Serial.print(" ");
    }
    Serial.println(); Serial.println();
#endif
    Wire.write(totalarray, sizeof totalarray);

  } else {
    uint8_t errorarray[FLOAT_BYTE_SIZE + CRC];
    //-127 indicates in Arduino Control Center as error
    errorarray[0] = 201; errorarray[1] = 192; errorarray[2] = 0; errorarray[3] = 0; errorarray[4] = 254; errorarray[5] = 194;
    Wire.write(errorarray, sizeof errorarray);

  }
}

void InitializeArray()
{
  values_array = new uint8_t*[VALUES_TO_SEND_COUNT];//creating VALUES_TO_SEND_COUNT rows
  for (uint8_t i = 0; i < VALUES_TO_SEND_COUNT; ++i)//each row 4 byte size for float value as byte array
    values_array[i] = new uint8_t[FLOAT_BYTE_SIZE];

  for (uint8_t  i = 0; i < VALUES_TO_SEND_COUNT; ++i)//all bytes to 0 initialization
    for (uint8_t  j = 0; j < FLOAT_BYTE_SIZE; ++j)
      values_array[i][j] = 0;
}

void writeValuesAsFloatByteArray (float value, uint8_t valuesToSendIndex)
//function for writing values to values array by index(first value is 0, second 1,...
{
  if (valuesToSendIndex < VALUES_TO_SEND_COUNT) {//valuesToSendIndex cannot be equal or greather than VALUES_TO_SEND_COUNT defined in header!!
    uint8_t *p = (uint8_t*)&value;
    uint8_t i;
    for (i = 0; i < FLOAT_BYTE_SIZE; i++)
      values_array[valuesToSendIndex][i] = p[i];
  } else {
    Serial.print("VALUES_TO_SEND_COUNT = "); Serial.println(VALUES_TO_SEND_COUNT);
    Serial.print("valuesToSendIndex must be less tnan VALUES_TO_SEND_COUNT = "); Serial.println(valuesToSendIndex);
  }

}
//crc calculation
uint16_t calc_crc(uint8_t *msg, int n)
{
  uint16_t x = 0;

  while (n--)
  {
    x = crc_xmodem_update(x, (uint16_t) * msg++);
  }

  return (x);
}


uint16_t crc_xmodem_update (uint16_t crc, uint8_t data)
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021; //(polynomial = 0x1021)
    else
      crc <<= 1;
  }
  return crc;
}



