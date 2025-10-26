#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include "MAX6675.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define THERMO_DAT 12
#define THERMO_CLK 13
#define THERMO_CS 10

#define TFT_DC 22
#define TFT_CS 21

#define RELAYPIN 8
#define STARTBTN 4
#define BUZZERPIN 5

// const float timerange[] = {0.0, 340.0};
// const float temprange[] = {30.0, 240.0};
// const int times[] = {0, 20, 30, 40, 110, 120, 130, 150, 200, 210, 220, 240, 340};
// const int temps[] = {30, 90, 100, 110, 140, 150, 150, 183, 230, 235, 240, 183, 50};
const float timerange[] = {0.0, 280.0};
const float temprange[] = {25.0, 235.0};
const int times[] = {0, 30, 120, 150, 210, 240, 260, 280};
const int temps[] = {25, 100, 150, 183, 235, 183, 150, 117};

int graph_width;
int graph_height;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define TEXT_HEIGHT 24
#define DIVIDER_LINE_THICKNESS 3

MAX6675 thermoCouple(THERMO_CS, &SPI);
float temperature = 0;

// For the Adafruit shield, these are the default.
#define TFT_DC 22
#define TFT_CS 21

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, 3, 2, 5, 0);

int lastSwitch = 0;

void setup()
{
  Serial.begin(115200);

  // first order of buisness: initialize the display! (Also allows for some natural delay to start Serial.begin)
  SPI.begin();
  thermoCouple.begin();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  // set pins
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);
  pinMode(STARTBTN, INPUT_PULLUP);

  // calculate some constants based on the defined constants above
  const int zeroY = SCREEN_HEIGHT - TEXT_HEIGHT - 2 - DIVIDER_LINE_THICKNESS;
  const int arrayLen = sizeof(temps) / sizeof(temps[0]);
  const float PpSX = SCREEN_WIDTH / (timerange[1] - timerange[0]);
  const float PpDY = zeroY / (temprange[1] - temprange[0]);

  Serial.println(__FILE__);
  Serial.print("MAX6675_LIB_VERSION: ");
  Serial.println(MAX6675_LIB_VERSION);
  Serial.println();

  thermoCouple.setSPIspeed(4000000);
  thermoCouple.setOffset(273);
  int status = thermoCouple.read();
  if (status != 0)
  {
    Serial.println(status);
  }
  temperature = thermoCouple.getCelsius();

  // map the profile to the screen, then draw it
  int mappedTimes[arrayLen];
  int mappedTemps[arrayLen];
  for (int i = 0; i < arrayLen; i++)
  {
    mappedTimes[i] = (times[i] - timerange[0]) * PpSX;
    mappedTemps[i] = zeroY - (temps[i] - temprange[0]) * PpDY;
  }
  for (int i = 0; i < arrayLen - 1; i++)
  {
    tft.drawLine(mappedTimes[i], mappedTemps[i], mappedTimes[i + 1], mappedTemps[i + 1], ILI9341_CYAN);
  }

  // draw the divider line and add text
  for (int i = 0; i < DIVIDER_LINE_THICKNESS; i++)
  {
    tft.drawFastHLine(0, SCREEN_HEIGHT - TEXT_HEIGHT - i - 2, SCREEN_WIDTH, ILI9341_WHITE);
  }
  tft.setCursor(0, SCREEN_HEIGHT - TEXT_HEIGHT);
  tft.setTextSize(TEXT_HEIGHT / 8);
  tft.print("Ready.");

  // who says waiting is boring? This bit of code doesn't!
  int j = 0;
  long unsigned int lastDot = millis() - 1000;
  while (digitalRead(STARTBTN))
  { // wait until the button is pressed
    if (millis() > lastDot + 1000)
    {
      tft.setCursor(0, SCREEN_HEIGHT - TEXT_HEIGHT);
      switch (j % 4)
      {
      case 0:
        tft.fillRect(.6 * TEXT_HEIGHT * 7.5, SCREEN_HEIGHT - (TEXT_HEIGHT * 0.5), 0.6 * TEXT_HEIGHT * 4, TEXT_HEIGHT * 0.5, ILI9341_BLACK);
        break;
      case 1:
        tft.print("Ready..");
        break;
      case 2:
        tft.print("Ready...");
        break;
      case 3:
        tft.print("Ready....");
        break;
      }
      j++;
      lastDot = millis();
    }
  }
}

void loop()
{
}