#include <Arduino.h>

#include "GPIO_DEAKIN.h"
#include "TIMER_DEAKIN.h"

//R,G,B,Y in PORTA & R,G,B,Y in PORTB
char pinArr[8] = { 7, 6, 5, 4, 15, 18, 19, 21};
char emptyPattern = 0;

char redOnly = 0b10001000;
char greenOnly = 0b01000100;
char blueOnly = 0b00100010;
char yellowOnly = 0b00010001;

char evenLED = 0b10101010;
char oddLED = 0b01010101;

void setup()
{
  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Configuring GPIO...");
  
  GPIO_DEAKIN::Config_GPIO_Pins(pinArr, OUTPUT);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, emptyPattern);

  Serial.println("Finished Setup");
}

void loop()
{
  int interval = 2000;
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, redOnly);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, greenOnly);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, blueOnly);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, yellowOnly);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, evenLED);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, oddLED);
  delay(interval);
  GPIO_DEAKIN::GPIO_Display_Pattern(pinArr, emptyPattern);
  delay(interval);

  GPIO_DEAKIN::GPIO_Sequential_Lights(pinArr, 1000);



}