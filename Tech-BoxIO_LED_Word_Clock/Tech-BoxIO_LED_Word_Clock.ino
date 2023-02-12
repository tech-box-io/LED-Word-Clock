//Start by including the needed libraries
//Make sure that you have included these .zip libraries in your Arduino installation
#include "src/FastLED.h"
#include "src/RTClib.h"
#include <EEPROM.h>

//Initialize the I/O Pin we will use
#define LED_PIN 7

//Initialize the number of LEDs and the leds[] object
#define NUM_LEDS 64
CRGB leds[NUM_LEDS];

//Define the Real Time Clock (RTC) name
RTC_DS3231 rtc;

//Initialize the date/time object
DateTime now;

//Initialize the variables we will use
int lastMinute = 0;                 //Will be used to check if the minute is now different. Used to update the displayed time
int EEPROMaddr = 0;                 //Used for updating the time to be displayed from the RTC as an address in the EEPROM
int EEPROMval = 0;                  //Used for updating the time to be displayed from the RTC as a value written/read to/from the EEPROM
bool Select_FiveThirtyFace = true;   //true displays LEDs based on the FiveThirty Grid, false displays LEDs based on the HalfTo Grid


//FiveThirty Matrix Setup
//The matrix is an 8x8 grid of the letters below, and displayed time in a "Hour Minutes" fashion:
      //THRTWONE 
      //TWELEVEN 
      //ENEIGHTI 
      //SIXSEVEN 
      //FOURFIVE 
      //THIRTWEN 
      //MFIFORTY 
      //EFIVTEEN 
//The below integer arrays hold the location of the LEDs that needs to be turned on for each "time"
//Hours 1 through 12, and minutes in 5-minute intervals from '00' to '55' are assigned
//For example in FT3[5] = {0, 1, 2, 10, 18}, LEDs 0, 1, 2, 10, and 18 are illuminated when that time is to be displayed
int FTOFF[3]  = {33, 49, 57};
int FT1[3]    = {5, 6, 7};
int FT2[3]    = {3, 4, 5};
int FT3[5]    = {0, 1, 2, 10, 18};
int FT4[4]    = {32, 33, 34, 35};
int FT5[4]    = {36, 37, 38, 39};
int FT6[3]    = {24, 25, 26};
int FT7[5]    = {27, 28, 29, 30, 31};
int FT8[5]    = {18, 19, 20, 21, 22};
int FT9[4]    = {15, 23, 31, 39};
int FT10[3]   = {8, 16, 17};
int FT11[6]   = {10, 11, 12, 13, 14, 15};
int FT12[6]   = {8, 9, 10, 11, 13, 14};
int FTEVEN[4] = {56, 59, 61, 63};
int FTFIVE[5] = {52, 57, 58, 59, 61};
int FTTen[3]  = {44, 46, 47};
int FT15[7]   = {49, 50, 51, 60, 61, 62, 63};
int FT20[6]   = {44, 45, 46, 47, 54, 55};
int FT25[10]  = {44, 45, 46, 47, 54, 55, 57, 58, 59, 61};
int FT30[6]   = {40, 41, 42, 43, 54, 55};
int FT35[10]  = {40, 41, 42, 43, 54, 55, 57, 58, 59, 61};
int FT40[5]   = {51, 52, 53, 54, 55};
int FT45[9]   = {51, 52, 53, 54, 55, 57, 58, 59, 61};
int FT50[5]   = {49, 50, 51, 54, 55};
int FT55[9]   = {49, 50, 51, 54, 55, 57, 58, 59, 61};
//End FiveThirty Matrix Setup


//HalfTo Matrix Setup
//The matrix is an 8x8 grid of the letters below and displays time in a "Minutes Of Hour/Minutes To Hour" fashion:
      //TWENONTY
      //FIFVTEEN
      //HALFMTOF
      //FOURFIVE
      //SIXSEVEN 
      //EIGHTHRI
      //TWELEVEN
      //ONETWOEE
//The below integer arrays hold the location of the LEDs that needs to be turned on for each "time"
//Hours 1 through 12, and minutes in 5-minute intervals from '00' to '30' are assigned for a Minutes Of Hour/Minutes To Hour format
//For example in HTTen[3] = {0, 2, 3}, LEDs 0, 2, and 3 are illuminated when that time is to be displayed
int HTOFF[3]  = {23, 25, 28};
int HT1[3]    = {56, 57, 58};
int HT2[3]    = {59, 60, 61};
int HT3[5]    = {44, 45, 46, 54, 62};
int HT4[4]    = {24, 25, 26, 27};
int HT5[4]    = {28, 29, 30, 31};
int HT6[3]    = {32, 33, 34};
int HT7[5]    = {35, 36, 37, 38, 39};
int HT8[5]    = {40, 41, 42, 43, 44};
int HT9[4]    = {39, 47, 55, 63};
int HT10[3]   = {48, 52, 55};
int HT11[6]   = {50, 51, 52, 53, 54, 55};
int HT12[6]   = {48, 49, 50, 51, 53, 54};
int HTNONE[4] = {3, 4, 5, 13};
int HTFIVE[4] = {8, 9, 11, 13};
int HTTen[3]  = {0, 2, 3};
int HT15[7]   = {8, 9, 10, 12, 13, 14, 15};
int HT20[6]   = {0, 1, 2, 5, 6, 7};
int HT25[10]  = {0, 1, 2, 5, 6, 7, 8, 9, 11, 13};
int HT30[4]   = {16, 17, 18, 19};
int HTOF[2]   = {22, 23};
int HTTO[2]   = {21, 22};
//End HalfTo Matrix Setup


void setup(){
  //Add LEDs to the leds object from the FastLED library
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  //Initialize the Serial Comms
  Serial.begin(57600);

  //Check to see if the RTC can be found
  //If it isn't available, abort
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  //Read the EEPROM value at address 0, if it is 255 (denoting a new Arduino with default EEPROM)
  //OR if it is 254 (denoting the real time clock [RTC] had lost power since the last programming)
  //THEN update the time on the RTC to match the compile time of this code
  EEPROMval = EEPROM.read(0);
  if(EEPROMval == 255 or EEPROMval == 254)
  {
    rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
    EEPROM.write(0,1);//Write "1" to the EEPROM at address "0" to lockout this function until RTC powerloss
  }
  
  //Check if the RTC has lost power since it was last connected/setup
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");

    //Write the value "254" to the EEPROM at address "0" to note that the RTC experienced a powerloss.
    //The powerloss will make the Time displayed inaccurate, so the clock will need to be reprogrammed
    //to fix this inaccuracy. The "254" value will allow the program to update the time the next time it
    //is programmed
    EEPROM.write(0,254);
    
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    // The line below sets the RTC with an explicit date & time, for example to set
    // July 22, 2020 at 4pm you would call:
    // rtc.adjust(DateTime(2020, 7, 22, 16, 0, 0));
  }
  
}

void loop() {
  //Get the current time from the RTC and assign it to the 'now' object
  now = rtc.now();

  //If the current time is a different minute from the last, call the update time 
  //function corresponding to the selected clock face
  if(now.minute() != lastMinute)
  {
    if(Select_FiveThirtyFace == true)
    {
      //Function will update the displayed time for the "FiveThirty" face if needed
      TimeUpdateFT();
    }
    else
    {
      //Function will update the displayed time for the "HalfTo" face if needed
      TimeUpdateHT();
    }
  }
  //Set the last minute to the current minute value
  lastMinute = now.minute();
}


//Update the time for the "FiveThirty" face if needed
//Function will be called once per minute if this face is selected, but 
//the displayed time will only update once every five minutes
void TimeUpdateFT()
{
  //Use the current minute time to determine what minute to display on the clock face
  //Set the *minuteDisplay pointer to the value of the minute array to be displayed (FTFIVE, FT15, etc.)
  //Set the minuteSize variable to the length of the minute array to be displayed (FTFIVE is length 4, FT15 is length 7, etc.)
  int *minuteDisplay;
  int minuteSize;
  if(now.minute() < 5){minuteDisplay = FTEVEN; minuteSize = 4;}
  else if(now.minute() < 10){minuteDisplay = FTFIVE; minuteSize = 5;}
  else if(now.minute() < 15){minuteDisplay = FTTen; minuteSize = 3;}
  else if(now.minute() < 20){minuteDisplay = FT15; minuteSize = 7;}
  else if(now.minute() < 25){minuteDisplay = FT20; minuteSize = 6;}
  else if(now.minute() < 30){minuteDisplay = FT25; minuteSize = 10;}
  else if(now.minute() < 35){minuteDisplay = FT30; minuteSize = 6;}
  else if(now.minute() < 40){minuteDisplay = FT35; minuteSize = 10;}
  else if(now.minute() < 45){minuteDisplay = FT40; minuteSize = 5;}
  else if(now.minute() < 50){minuteDisplay = FT45; minuteSize = 9;}
  else if(now.minute() < 55){minuteDisplay = FT50; minuteSize = 5;}
  else {minuteDisplay = FT55; minuteSize = 9;}

  //Repeat the process from above for the hour to be displayed using the *hourDisplay pointer and hourSize variable
  //Time is displayed in a 12-hour format, so 0 = 12AM, 13 = 1PM, etc. 
  int *hourDisplay;
  int hourSize;
  if(now.hour() == 0 || now.hour() == 12){hourDisplay = FT12;hourSize = 6;}
  else if(now.hour() == 1 || now.hour() == 13){hourDisplay = FT1; hourSize = 3;}
  else if(now.hour() == 2 || now.hour() == 14){hourDisplay = FT2; hourSize = 3;}
  else if(now.hour() == 3 || now.hour() == 15){hourDisplay = FT3; hourSize = 5;}
  else if(now.hour() == 4 || now.hour() == 16){hourDisplay = FT4; hourSize = 4;}
  else if(now.hour() == 5 || now.hour() == 17){hourDisplay = FT5; hourSize = 4;}
  else if(now.hour() == 6 || now.hour() == 18){hourDisplay = FT6; hourSize = 3;}
  else if(now.hour() == 7 || now.hour() == 19){hourDisplay = FT7; hourSize = 5;}
  else if(now.hour() == 8 || now.hour() == 20){hourDisplay = FT8; hourSize = 5;}
  else if(now.hour() == 9 || now.hour() == 21){hourDisplay = FT9; hourSize = 4;}
  else if(now.hour() == 10 || now.hour() == 22){hourDisplay = FT10; hourSize = 3;}
  else if(now.hour() == 11 || now.hour() == 23){hourDisplay = FT11; hourSize = 6;}

  //Clear the currently illuminated LEDs
  FastLED.clear();

  //For each LED in the length of the hourDisplay pointer, set that LED to be illuminated on the next FastLED.show() call
  for(int i = 0; i < hourSize; i++)
  {
    int j = hourDisplay[i];
    //Set the color of the LED to be illuminated from one of the presets
    //In the color_presetx(y) function call, the x is the preset chosen (1-5), and y is the order the word appears on the face
    //The first word would have a y value of 1, the second would have a y value of 2, etc.
    leds[j] = color_preset3(1);    
  }

  //For each LED in the length of the minuteDisplay pointer, set that LED to be illuminated on the next FastLED.show() call
  for(int i = 0; i < minuteSize; i++)
  {
    int j = minuteDisplay[i];
    //Set the color of the LED to be illuminated from one of the presets
    //In the color_presetx(y) function call, the x is the preset chosen (1-5), and y is the order the word appears on the face
    //The first word would have a y value of 1, the second would have a y value of 2, etc.
    leds[j] = color_preset3(2);   
  }

  //Show the LEDs to be illuminated
  FastLED.show();
  
}


//Update the time for the "HalfTo" face if needed
//Function will be called once per minute if this face is selected, but 
//the displayed time will only update once every five minutes
void TimeUpdateHT()
{
  //Use the current minute time to determine what minute to display on the clock face
  //Set the *minuteDisplay pointer to the value of the minute array to be displayed (HTFIVE, HT15, etc.)
  //Set the minuteSize variable to the length of the minute array to be displayed (HTFIVE is length 4, HT15 is length 7, etc.)
  int *minuteDisplay;
  int minuteSize;
  if(now.minute() < 5){minuteDisplay = HTNONE; minuteSize = 4;}
  else if((now.minute() < 10) || (now.minute() > 54)){minuteDisplay = HTFIVE; minuteSize = 4;}
  else if((now.minute() < 15) || (now.minute() > 49)){minuteDisplay = HTTen; minuteSize = 3;}
  else if((now.minute() < 20) || (now.minute() > 44)){minuteDisplay = HT15; minuteSize = 7;}
  else if((now.minute() < 25) || (now.minute() > 39)){minuteDisplay = HT20; minuteSize = 6;}
  else if((now.minute() < 30) || (now.minute() > 34)){minuteDisplay = HT25; minuteSize = 10;}
  else {minuteDisplay = HT30; minuteSize = 4;}

  //Repeat the process from above for the hour to be displayed using the *hourDisplay pointer and hourSize variable
  //Also, repeat the process above fo rthe *OTDisplay pointer and otSize variable
  //Time is displayed in a 12-hour format, so 0 = 12AM, 13 = 1PM, etc. 
  int *OTDisplay;
  int otSize;
  int *hourDisplay;
  int hourSize;
  //If the minute value is less than 30, display in the "Minute of Hour" format
  if(now.minute() < 30){
    OTDisplay = HTOF; otSize = 2;
    if(now.hour() == 0 || now.hour() == 12){hourDisplay = HT12;hourSize = 6;}
    else if(now.hour() == 1 || now.hour() == 13){hourDisplay = HT1; hourSize = 3;}
    else if(now.hour() == 2 || now.hour() == 14){hourDisplay = HT2; hourSize = 3;}
    else if(now.hour() == 3 || now.hour() == 15){hourDisplay = HT3; hourSize = 5;}
    else if(now.hour() == 4 || now.hour() == 16){hourDisplay = HT4; hourSize = 4;}
    else if(now.hour() == 5 || now.hour() == 17){hourDisplay = HT5; hourSize = 4;}
    else if(now.hour() == 6 || now.hour() == 18){hourDisplay = HT6; hourSize = 3;}
    else if(now.hour() == 7 || now.hour() == 19){hourDisplay = HT7; hourSize = 5;}
    else if(now.hour() == 8 || now.hour() == 20){hourDisplay = HT8; hourSize = 5;}
    else if(now.hour() == 9 || now.hour() == 21){hourDisplay = HT9; hourSize = 4;}
    else if(now.hour() == 10 || now.hour() == 22){hourDisplay = HT10; hourSize = 3;}
    else if(now.hour() == 11 || now.hour() == 23){hourDisplay = HT11; hourSize = 6;}
  }
  //Else, if the minute is 30 or greater, display in the "Minute to Hour" format
  else 
  {
    OTDisplay = HTTO; otSize = 2;
    if(now.hour() == 0 || now.hour() == 12){hourDisplay = HT1;hourSize = 3;}
    else if(now.hour() == 1 || now.hour() == 13){hourDisplay = HT2; hourSize = 3;}
    else if(now.hour() == 2 || now.hour() == 14){hourDisplay = HT3; hourSize = 5;}
    else if(now.hour() == 3 || now.hour() == 15){hourDisplay = HT4; hourSize = 4;}
    else if(now.hour() == 4 || now.hour() == 16){hourDisplay = HT5; hourSize = 4;}
    else if(now.hour() == 5 || now.hour() == 17){hourDisplay = HT6; hourSize = 3;}
    else if(now.hour() == 6 || now.hour() == 18){hourDisplay = HT7; hourSize = 5;}
    else if(now.hour() == 7 || now.hour() == 19){hourDisplay = HT8; hourSize = 5;}
    else if(now.hour() == 8 || now.hour() == 20){hourDisplay = HT9; hourSize = 4;}
    else if(now.hour() == 9 || now.hour() == 21){hourDisplay = HT10; hourSize = 3;}
    else if(now.hour() == 10 || now.hour() == 22){hourDisplay = HT11; hourSize = 6;}
    else if(now.hour() == 11 || now.hour() == 23){hourDisplay = HT12; hourSize = 6;}
  }

  //Clear the currently illuminated LEDs
  FastLED.clear();

  //For each LED in the length of the hourDisplay pointer, set that LED to be illuminated on the next FastLED.show() call
  for(int i = 0; i < hourSize; i++)
  {
    int j = hourDisplay[i];
    //Set the color of the LED to be illuminated from one of the presets
    //In the color_presetx(y) function call, the x is the preset chosen (1-5), and y is the order the word appears on the face
    //The first word would have a y value of 1, the second would have a y value of 2, "of/to" will always have a value of 3
    leds[j] = color_preset1(2);    
  }

  //For each LED in the length of the OTDisplay pointer, set that LED to be illuminated on the next FastLED.show() call
  for(int i = 0; i < otSize; i++)
  {
    int j = OTDisplay[i];
    //Set the color of the LED to be illuminated from one of the presets
    //In the color_presetx(y) function call, the x is the preset chosen (1-5), and y is the order the word appears on the face
    //The first word would have a y value of 1, the second would have a y value of 2, "of/to" will always have a value of 3
    leds[j] = color_preset1(3);
  }

  //For each LED in the length of the minuteDisplay pointer, set that LED to be illuminated on the next FastLED.show() call
  for(int i = 0; i < minuteSize; i++)
  {
    int j = minuteDisplay[i];
    //Set the color of the LED to be illuminated from one of the presets
    //In the color_presetx(y) function call, the x is the preset chosen (1-5), and y is the order the word appears on the face
    //The first word would have a y value of 1, the second would have a y value of 2, "of/to" will always have a value of 3
    leds[j] = color_preset1(1);
  }

  //Show the LEDs to be illuminated
  FastLED.show();
}


//This preset assigns each word in the current time a different color
//For this preset, the word colors are: Red, Gold, and White
CRGB color_preset1(int sequence)
{
  if(sequence == 1)
  {
    return CRGB(255,0,0);
  }
  else if(sequence == 2)
  {
    return CRGB(255,215,0);
  }
  else
  {
    return CRGB(255,255,255);
  }
}


//This preset assigns each word in the current time a different color
//For this preset, the word colors are: Gold, Purple, and Silver
CRGB color_preset2(int sequence)
{
  if(sequence == 1)
  {
    return CRGB(255,215,0);
  }
  else if(sequence == 2)
  {
    return CRGB(80,0,120);
  }
  else
  {
    return CRGB(192,192,192);
  }
}


//This preset assigns each word in the current time a different color
//For this preset, the word colors are: Orange, Navy, and Silver
CRGB color_preset3(int sequence)
{
  if(sequence == 1)
  {
    return CRGB(255,50,0);
  }
  else if(sequence == 2)
  {
    return CRGB(0,0,128);
  }
  else
  {
    return CRGB(150,150,150);
  }
}


//This preset assigns each word in the current time a different color
//For this preset, the word colors are: Green, Red, and Silver
CRGB color_preset4(int sequence)
{
  if(sequence == 1)
  {
    return CRGB(0,100,0);
  }
  else if(sequence == 2)
  {
    return CRGB(255,0,0);
  }
  else
  {
    return CRGB(192,192,192);
  }
}


//This preset assigns the same color to each letter on the clock face
//For this preset, the color is: Orchid
CRGB color_preset5(int sequence)
{
  return CRGB(218,80,214);
}
