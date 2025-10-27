#include <Arduino.h>
#include <SPI.h>
#include "MAX6675.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "InterpolationLib.h"

#define THERMO_DAT 12
#define THERMO_CLK 13
#define THERMO_CS 10

#define TFT_DC 22
#define TFT_CS 21
#define TFT_MOSI 3
#define TFT_SCK 2
#define TFT_RST 5
#define TFT_MISO 0
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define TEXT_HEIGHT 24
#define DIVIDER_LINE_THICKNESS 3

#define RELAYPIN 8
#define STARTBTN 4
#define BUZZERPIN 6

// Datasheet settings
const float timerange[] = {0.0, 280.0};
const float temprange[] = {25.0, 235.0};
double times[] = {0, 30, 120, 150, 210, 240, 280};  // interpolation requires this to be a non-constant double
double temps[] = {25, 100, 150, 183, 235, 183, 25}; // interpolation requires this to be a non-constant double
const float maxTemperature[] = {210.0, 235.0};

// Adafruit's EZ Make Oven Settings
// const float timerange[] = {0.0, 340.0};
// const float temprange[] = {30.0, 240.0};
// double times[] = {0, 20, 30, 40, 110, 120, 130, 150, 200, 210, 220, 240, 340};   // interpolation requires this to be a non-constant double
// double temps[] = {30, 90, 100, 110, 140, 150, 150, 183, 230, 235, 240, 183, 50}; // interpolation requires this to be a non-constant double
// const float maxTemperature[] = {210.0, 240.0};

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);

MAX6675 thermoCouple(THERMO_CS, &SPI);
float temperature = 0;

#define READ_DELAY 250    // time to wait in between thermocouple reads
#define SWITCH_DELAY 1000 // time to wait in between relay switching

// calculate some constants based on the defined constants above
const int zeroY = SCREEN_HEIGHT - TEXT_HEIGHT - 2 - DIVIDER_LINE_THICKNESS;
const int arrayLen = sizeof(temps) / sizeof(temps[0]);
const float PpSX = SCREEN_WIDTH / (timerange[1] - timerange[0]);
const float PpDY = zeroY / (temprange[1] - temprange[0]);
const float secondsPerPixel = 1 / PpSX;

unsigned long int lastSwitch = 0; // last time the SSR was switched
unsigned long int lastRead;       // last time the MAX6675 was read
unsigned long int startTime;      // time of reflow start
float targetTemperature = temps[0];
bool switchState = false; // state of relay (true =  on)
int currentStage = 1;
bool stageDone = false;

const char *currentTemperatureChar[5];
const char *targetTemperatureChar[5];

void finishedReflow();
float getTargetTemp();

void setup()
{
  // set pins. Relay first, we don't want it turning on until we're ready.
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);
  pinMode(STARTBTN, INPUT_PULLUP);
  pinMode(BUZZERPIN, OUTPUT);

  Serial.begin(115200);

  // first order of business: initialize the display! (Also allows for some natural delay to start Serial.begin)
  SPI.begin();
  thermoCouple.begin();
  tft.begin();
  tft.setRotation(1);
  tft.cp437(true);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(ILI9341_BLACK);

  // start the thermocouple stuff
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
  Serial.println(temperature);
  lastRead = millis();

  // check for obvious thermocouple issues
  // if (thermoCouple.getRawData() == 0)
  // {
  //   tft.print("Temperature  = 0 degrees C\nMAX6675 disconnected.\n(Or you're doing winter reflow?)\n\nHit start to bypass");
  //   while (digitalRead(STARTBTN))
  //   {
  //   }
  //   tft.fillScreen(ILI9341_BLACK);
  //   delay(500);
  // }
  // else if (temperature < -274)
  // {
  //   tft.print("Temperature below -274 C.\nEither you're breaking physics or\nthe thermocouple is disconnected.\nEither way, please fix that.");
  //   return;
  // }

  // draw the key point lines
  int mappedTime;
  int mappedTemp;
  for (int i = 1; i < arrayLen - 1; i++)
  {
    mappedTime = (times[i] - timerange[0]) * PpSX;
    mappedTemp = zeroY - (temps[i] - temprange[0]) * PpDY;

    // draw the horizontal dotted lines
    if (times[i] > maxTemperature[0])
    {
      for (int j = SCREEN_WIDTH; j * 3 > mappedTime; j--)
      {
        tft.drawPixel(j * 3, mappedTemp, ILI9341_PURPLE);
      }
    }
    else if (times[i] < maxTemperature[0])
    {
      for (int j = 0; j * 3 < mappedTime; j++)
      {
        tft.drawPixel(j * 3, mappedTemp, ILI9341_PURPLE);
      }
    }

    // draw the vertical lines
    for (int k = (SCREEN_HEIGHT - TEXT_HEIGHT); k * 3 > mappedTemp; k--)
    {
      tft.drawPixel(mappedTime, k * 3, ILI9341_PURPLE);
    }
  }
  tft.fillRect(0, SCREEN_HEIGHT - TEXT_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, ILI9341_BLACK);

  // draw the profile to the screen
  int interpolatedPoint;
  for (int i = 0; i < SCREEN_WIDTH; i++)
  {
    interpolatedPoint = Interpolation::ConstrainedSpline(times, temps, arrayLen, secondsPerPixel * i);
    tft.drawPixel(i, zeroY - (interpolatedPoint - temprange[0]) * PpDY, ILI9341_CYAN);
  }

  // draw the divider line and add text
  for (int i = 0; i < DIVIDER_LINE_THICKNESS; i++)
  {
    tft.drawFastHLine(0, SCREEN_HEIGHT - TEXT_HEIGHT - i - 2, SCREEN_WIDTH, ILI9341_WHITE);
  }
  tft.setTextSize(TEXT_HEIGHT / 8);

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
        tft.print("Ready.   ");
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

  // we're good to go! Find out whether to switch the SSR to start
  tft.fillRect(0, SCREEN_HEIGHT - TEXT_HEIGHT, SCREEN_WIDTH, TEXT_HEIGHT, ILI9341_BLACK);
  tft.setCursor(0, SCREEN_HEIGHT - TEXT_HEIGHT);
  temperature = thermoCouple.getCelsius();

  // if you're somehow below room temp, bring it up to room temp before starting the clock
  if (temperature < temps[0])
  {
    tft.print("Below Room Temp...");
    digitalWrite(RELAYPIN, HIGH);
    switchState = true;
    lastSwitch = millis();
    temperature = thermoCouple.getCelsius();
    delay(READ_DELAY);
  }

  targetTemperature = temps[1];
  if (temperature < targetTemperature)
  {
    digitalWrite(RELAYPIN, HIGH);
    switchState = true;
    lastSwitch = millis();
  }

  lastRead = millis();
  startTime = lastRead;
}

void loop()
{
  if (millis() > lastRead + READ_DELAY)
  {
    temperature = thermoCouple.getCelsius();
    targetTemperature = getTargetTemp();
    if (temperature > targetTemperature)
    {
      stageDone = true;
    }
    if (stageDone && (millis() - startTime) / 1000 > times[currentStage])
    {
      stageDone = false;
      currentStage++;
      if (currentStage == arrayLen - 1)
      {
        finishedReflow();
      }
    }
    lastRead = millis();

    // show the temperatures on the screen
    tft.setCursor(5 * .6 * TEXT_HEIGHT, SCREEN_HEIGHT - TEXT_HEIGHT);
    tft.print(temperature);
    tft.print("/");
    tft.print((int)targetTemperature);
    tft.write(0xF8);
    tft.print("C");

    // dtostrf(temperature, 5, 1, &currentTemperatureChar);

    // also print it to serial for debug purposes
    Serial.print((millis() - startTime) / 1000);
    Serial.print("\tTemperature: ");
    Serial.print(temperature);
    Serial.print("\tTarget: ");
    Serial.print(targetTemperature);
    Serial.print("\tStage: ");
    Serial.println(currentStage);
  }

  // if enough time has passed since the last time we switched AND a switch is needed, make it
  if (targetTemperature > temperature && !switchState && millis() > lastSwitch + SWITCH_DELAY)
  {
    digitalWrite(RELAYPIN, HIGH);
    switchState = true;
    lastSwitch = millis();
  }
  else if (targetTemperature < temperature && switchState && millis() > lastSwitch + SWITCH_DELAY)
  {
    digitalWrite(RELAYPIN, LOW);
    switchState = false;
    lastSwitch = millis();
  }

  // this is mildly inefficient but the RP2040 has more than enough oomph
  tft.setCursor(0, SCREEN_HEIGHT - TEXT_HEIGHT);
  tft.print((millis() - startTime) / 1000);
  tft.drawPixel((millis() - startTime) / 1000 * PpSX, zeroY - (temperature - temprange[0]) * PpDY, ILI9341_YELLOW);
}

void finishedReflow()
{
  digitalWrite(RELAYPIN, LOW);
  tft.setCursor(0, SCREEN_HEIGHT - TEXT_HEIGHT);
  tft.print("Done in: ");
  tft.print((millis() - startTime) / 1000);
  while (1)
  {
  }
}

float getTargetTemp()
{
  float tempTarget;
  tempTarget = temps[currentStage];

  return tempTarget;
}