/* Temperature logging using two DS18B20's.
Code can be easily changed to allow use with multiple sensors.
Thanks to Terry King from YourDuino for the example temperature code which has been used extensively
For more information, visit joshajohnson.com*/

/* Importing Required Libraries*/
#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_PIN 2 // Define pin the One Wire interface will be using

OneWire oneWire(ONE_WIRE_BUS_PIN); // Setup a oneWire instance to communicate to the sensors

DallasTemperature sensors(&oneWire); // Pass oneWire reference to Dallas Temperature

DeviceAddress Probe01 = {0x28, 0xFF, 0x89, 0x28, 0x93, 0x16, 0x05, 0xA9}; // Defining which sensor is which. Visit http://arduino-info.wikispaces.com/Brick-Temperature-DS18B20 to find out the address values of each sensor.
DeviceAddress Probe02 = {0x28, 0xFF, 0x6A, 0x00, 0x93, 0x16, 0x04, 0x0F}; // Note: These values are specific to each sensor. Use the above link to determine yours.

/*These constants won't change. They are the lowest and highest readings you get from your sensor: */
const int sensorMin = 0;      // sensor minimum, discovered through experiment
const int sensorMax = 32;    // sensor maximum, discovered through experiment

RTC_DS1307 rtc; //Defining name for the RTC for easier coding

File myFile; // For SD Card

int pinCS = 53; // Pin 10 on Arduino Uno - Used for SPI interface with SD card

/*Definine pins for the LED's. Not essential but helps with debugging*/
const int ledGreen =  7;
const int ledRed  = 6;


void setup () {

/* Define LED's as outputs*/
pinMode(ledGreen, OUTPUT);
pinMode(ledRed, OUTPUT);

sensors.begin();

  /* set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster) */
  sensors.setResolution(Probe01, 10);
  sensors.setResolution(Probe02, 10);

  while (!Serial); // for Leonardo/Micro/Zero

/*Check the RTC is working*/
  Serial.begin(9600);
   pinMode(pinCS, OUTPUT);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC"); // Prints to serial if the RTC is not connected
     while (1);
     digitalWrite(ledRed, HIGH); // Pull Red LED high if RTC not working
      delay(1000);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!"); // Prints to serial if the RTC is not working
    /*This line sets the RTC with an explicit date & time, for example to set
     June 26, 2017 at 7pm you would call: */
    //   rtc.adjust(DateTime(2017, 6, 26, 19, 0, 0)); // un comment this line out to change the time
    digitalWrite(ledRed, HIGH); // Another error LED
      delay(1000);
  }
/* Check SD Card is working */
  if (SD.begin())
  {
    Serial.println("SD card is ready to use."); // Prints to serial that the SD card is working
    digitalWrite(ledRed, LOW);  // If all is good turn off the Red LED
      delay(1000);
  } else
  {
    Serial.println("SD card initialization failed"); // Prints to serial that the SD card is not working
    digitalWrite(ledRed, HIGH); // If there is an error with the SD card turn the Red LED on
      delay(1000);
    return;
  }

  /* Create/Open file */
  myFile = SD.open("Logger01.csv", FILE_WRITE); // CSV is used as it allows for easy analysis

  /* if the file opened okay, write to it */
  if (myFile) {
    Serial.println("Writing to file..."); // Print to serial that the SD card is being written to.
    /* Write header info to file. These will be the columns we will print data to later */
    myFile.print("Date");
    myFile.print(',');
    myFile.print("Time");
    myFile.print(',');
    myFile.print("Inside Temperature");
    myFile.print(",");
    myFile.print("Outside Temperature");
    myFile.print(",");
    myFile.println();

    myFile.close(); // close the file
    Serial.println("Done."); // Print that the header data was successfully written
    digitalWrite(ledRed, LOW); // Everything is okay so turn the Red LED off
      delay(1000);
  }
  /* if the file didn't open, print an error */
  else {
    Serial.println("error opening Logger01.csv"); // Print error if there is an issue writing to the sd card
    digitalWrite(ledRed, HIGH); // Turn the Red LED high as there in an error.
      delay(1000);
  }
  /* This part reads the file */
  myFile = SD.open("Logger01.csv");
  if (myFile) {
    Serial.println("Read:");
  /* Read the file */
    while (myFile.available()) {
      Serial.write(myFile.read());
   }
    myFile.close();
  }
  else {
    Serial.println("error opening Logger01.csv"); // Print to serial that the CSV could not be opened.
    digitalWrite(ledRed, HIGH); // Another error LED
      delay(1000);
  }
}

void loop () {

DateTime now = rtc.now(); // Allows for Time to be recorded

myFile = SD.open("Logger01.csv", FILE_WRITE); // Opens File on SD card

  /*if the file opened okay, write the time and temperature data to it. Each column is separated by a comma as we are using a .CSV document */
  if (myFile) {
    Serial.println("Writing to File"); // Print to serial things are being written
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(",");
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second() , DEC);
    myFile.print(",");
    sensors.requestTemperatures(); //Read temepratures
    printTemperature(Probe01); // Print temp from probe 1
    myFile.print(",");
    printTemperature(Probe02); // Print temp from probe 2
    myFile.print(",");
    myFile.println(); // New line as this batch of data is done


    myFile.close(); // close the file
    Serial.println("Data Logged."); // Print to serial that the Data has been successfully logged to the SD card
    digitalWrite(ledRed, LOW); // Turn the Red LED off if for some reason it is still on
    digitalWrite(ledGreen, HIGH); // Flash the Green LED on for one second to show that it has recorded the data
    delay(1000);
    digitalWrite(ledGreen, LOW);

  }
  /* if the file didn't open, print an error */
  else {
    Serial.println("error opening Logger01.csv"); // Prints error that SD card could not be printed to
    digitalWrite(ledRed, HIGH); // Red Led turns on if there is an error
      delay(1000);
  }

       delay(360000); // Delay for 6 Minutes - adjust as required
}

/* This part gets the temperature in Celsius and Prints it to both Serial and the SD card when called*/
void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00)
   {
   Serial.print("Error getting temperature  "); // Print that something went wring
   digitalWrite(ledRed, HIGH); // Red Led turns on if there is an error
      delay(1000);
   }
   else
   {
   Serial.print(tempC); // Prints to serial
   Serial.println();
   myFile.print(tempC); // Prints so SD card
   }
}
