#include <SPI.h>
#include "MAX6675.h"


#define THERMO_DAT 26
#define THERMO_CLK 27
#define THERMO_CS 28

#define RELAYPIN 8

MAX6675 thermoCouple(THERMO_CS, THERMO_DAT, THERMO_CLK);
float temperature = 0;

int checktemp = 100;

void setup() {
  // put your setup code here, to run once:
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);

  delay(1000);

    SPI.begin();
  thermoCouple.begin();
  thermoCouple.setSPIspeed(4000000);
  thermoCouple.setOffset(0);

  Serial.begin(115200);

  delay(1000);

  Serial.println("This program will determine calibration settings ");
  Serial.print("for your oven to use with the EZ Make Oven.\n\n");

  Serial.print("Starting...");

  bool finish = false;
  int maxloop = 300;
  int counter = 0;

  while (!finish) {
    delay(1000);
    counter += 1;
    thermoCouple.read();
    digitalWrite(RELAYPIN, HIGH);
    temperature = thermoCouple.getCelsius();
    Serial.println(temperature);
    if (temperature >= checktemp) {
      finish = true;
      digitalWrite(RELAYPIN, LOW);
    }
    if (counter >= maxloop) {
      Serial.println("Bad Sensor!");
      while (1) { ; }
    }
  }

  Serial.println("Checking lag time...");
  thermoCouple.read();
  finish = false;
  int start_time = millis();
  float start_temp = thermoCouple.getCelsius();
  float last_temp = start_temp;

  while (!finish) {
    delay(1000);
    thermoCouple.read();
    temperature = thermoCouple.getCelsius();
    Serial.println(temperature);
    if (temperature <= last_temp) {
      finish = true;
    }

    last_temp = temperature;
  }

  float lag_temp = last_temp - checktemp;
  int lag_time = (millis() - start_time);
  Serial.println("** Calibration Results **");
  Serial.print("calibrate_temp: ");
  Serial.println(lag_temp);
  Serial.print("Calibrate Seconds: ");
  Serial.println(lag_time);
}

void loop() {
  // put your main code here, to run repeatedly:
}
