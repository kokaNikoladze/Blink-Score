#include "Adafruit_TLC5947.h"
#include "Metronome.h"
#define NUM_TLC5947 3

#define data    4
#define Clock   5
#define latch   7 // latch
#define oe      6 // tlc output enable
#define max_de  2 // max3085 driver output enable

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5947, Clock, data, latch);
metronome::State myMetro{};

void setup() {

  Serial.begin(115200); //153600
  pinMode(oe, OUTPUT);
  digitalWrite(oe, LOW);
  tlc.begin();
  delay(500);
  tlc.write();
  digitalWrite(oe, HIGH);

  //disable rs485 driver output
  pinMode(max_de, OUTPUT);
  digitalWrite(max_de, LOW);
}

uint8_t barNum[ 32 ] = {55, 54, 53, 52, 57, 56, 51,
                        50, 59, 58, 49, 48, 61, 60,
                        47, 46, 63, 62, 45, 44, 65,
                        64, 43, 42, 67, 66, 41, 40
                       };

uint8_t desk;
uint8_t sevSeg;
uint8_t sevSegPage;
uint8_t sevSegNumber;
uint8_t bar = 0;

uint16_t strip_low;
uint16_t strip_high;

int strip;

float blobCentre = -1;
uint8_t metro = 0;
uint8_t BPM = 60;


int packetBuffer[8] = {};
uint8_t packet_i = 0;
int oldBar = -1;

void ReceivePacket()
{
  while (0 < Serial.available()) {
    if (8 <= packet_i) {
      packet_i = 0;
      return;
    }

    packetBuffer[packet_i] = Serial.read();

    if (255 == packetBuffer[packet_i]) {
      if (packet_i < 7) {
        packet_i = 0;
        return;
      }

      packet_i = 0;

      desk = packetBuffer[0];
      if (desk != 3) {            //desk num
        continue;                 //continue;
      }

      sevSeg = packetBuffer[1];
      sevSegPage = sevSeg >> 7;
      sevSegNumber = sevSeg & 127;
      bar = packetBuffer[2];
      strip_low = packetBuffer[3];
      strip_high = (packetBuffer[4] & 127) << 7;
      strip = strip_high | strip_low;
      blobCentre = ((float)strip * 0.01) - 1;
      metro = packetBuffer[5];
      BPM = packetBuffer[6];

      return;
    }
    ++packet_i;
  }
}

void loop()
{

  metro = 0;
  ReceivePacket();
  if (metro != 0) {
    metronome::hit(&myMetro, metro - 1, BPM, (unsigned long)millis());
  }

  disp(sevSegNumber);

  setBar(bar);
  ledStrip(blobCentre);
  metroUpdate();

  tlc.write();
  delay(1);
}

void setBar(uint8_t newBar)
{
  const unsigned long duration = 500;
  const unsigned long blink_half_period = 50;
  const int blink_brightness = 500;
  const int solid_brightness = 100;

  static unsigned long last_change = 0;

  if (newBar == oldBar) {
    // if changed recently, then modulate brightness
    unsigned long since_last_change = millis() - last_change;

    if (duration < since_last_change) {
      tlc.setPWM(barNum[newBar - 1], solid_brightness);
      return;
    }

    int on = (since_last_change / blink_half_period) & 1;
    int brightness = on ? blink_brightness : 0;

    tlc.setPWM(barNum[newBar - 1], brightness);

    return;
  }

  // save: last bar change was now
  last_change = millis();

  if (oldBar != 0) {
    tlc.setPWM(barNum[oldBar - 1], 0);
  }
  if (newBar != 0) {
    tlc.setPWM(barNum[newBar - 1], solid_brightness);
  }
  oldBar = newBar;
}

void metroUpdate() {

  int PWM [4];
  int pins [] = {71, 70, 69, 68};
  metronome::fade(&myMetro, millis(), PWM, 4);
  for (int i = 0; i < 4; i++) {
    tlc.setPWM(pins[i], linearPWM(PWM[i]) * 5);
  }
}

void disp(int num) {

  // Segment Definitions
  uint8_t Disp[ 3 ][ 7 ] = {{37, 36, 31, 32, 33, 38, 39},  // Digit 1
    {24, 23, 28, 29, 30, 34, 35},  // Digit 2
    {20, 19, 25, 26, 27, 21, 22},  // Digit 3
  };

  // A  B  C  D  E  F  G
  uint8_t number[ 11 ][ 7 ] = {{ 1, 1, 1, 1, 1, 1, 0 }, // pattern for a 0
    { 0, 1, 1, 0, 0, 0, 0 },  // pattern for a 1
    { 1, 1, 0, 1, 1, 0, 1 },  // pattern for a 2
    { 1, 1, 1, 1, 0, 0, 1 },  // pattern for a 3
    { 0, 1, 1, 0, 0, 1, 1 },  // pattern for a 4
    { 1, 0, 1, 1, 0, 1, 1 },  // pattern for a 5
    { 1, 0, 1, 1, 1, 1, 1 },  // pattern for a 6
    { 1, 1, 1, 0, 0, 0, 0 },  // pattern for a 7
    { 1, 1, 1, 1, 1, 1, 1 },  // pattern for a 8
    { 1, 1, 1, 0, 0, 1, 1 },  // pattern for a 9

    { 1, 1, 0, 0, 1, 1, 1 }
  }; // P
  if (sevSegPage) {

    num = constrain(num, 0, 99);
    byte fir = 10;
    byte sec = num / 10;
    byte thi = num % 10;

    for ( uint8_t i = 0; i < 7 ; i++ ) {
      tlc.setPWM(Disp[0][i], (number[fir][i]) ? 4096 : 0 );
      tlc.setPWM(Disp[1][i], (number[sec][i]) ? 4096 : 0 );
      tlc.setPWM(Disp[2][i], (number[thi][i]) ? 4096 : 0 );
    }

  } else {

    byte fir = num / 100;
    byte sec = num % 100 / 10;
    byte thi = num % 10;

    for ( uint8_t i = 0; i < 7 ; i++ ) {
      tlc.setPWM(Disp[0][i], (number[fir][i]) ? 4096 : 0 );
      tlc.setPWM(Disp[1][i], (number[sec][i]) ? 4096 : 0 );
      tlc.setPWM(Disp[2][i], (number[thi][i]) ? 4096 : 0 );
    }
  }
}


void ledStrip(int blobCentre) {

  int offset = 100;
  for (int i = 0; i < 19; i++) {
    int brightness = constrain(strip - (offset * i), 0, 100);
    tlc.setPWM(i, brightness);
  }
}

/*
  void ledStrip(float blobCentre) {

  int maxBlobBrightness = 100;
  float width = 1.0f + blobCentre; //value from 1 to 19;

  for (int i = 0; i < 19; i++) {
    float X = (i - blobCentre);
    float brightness = constrain((width - (X * X)) / width, 0.0f, 1.0f);
    if (i < blobCentre) {
      brightness = 1.0f;
    }
    tlc.setPWM(i, linearPWM(brightness * 100));  //edited
  }
  }
*/

int linearPWM(int percentage) {
  double a = 9.7758463166360387E-01;
  double b = 5.5498961535023345E-02;
  return floor((a * exp(b * percentage) + .5)) - 1;
}


