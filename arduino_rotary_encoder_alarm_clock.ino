

/*
https://github.com/ldijkman/arduino_rotary_encoder_alarm_clock
version 6 juli 2019



/*
  :Project:Allarm_Clock
  :Author: Tiziano Bianchettin
  :Date: 10/02/2017
  :Revision: 2
  :License: Public Domain
  original source http://create.arduino.cc/projecthub/Tittiamo/alarm-clock-f61bad
   thanks to: My electronics laboratory professor "Perito Carli"

 **********************************************************************
  5 cents would be appreciated http://paypal.me/pools/c/8amUN5rgb9
  http://sticker.tk/forum/index.php?action=view&id=299
  Changes made by luberth dijkman juli 2019
  save alarmtime to eeprom for reload on reboot
  rotary encoder button for input
  timeout on menus
  NOT FINNISHED YET
  and much more
  http://sticker.tk/forum/index.php?action=view&id=299
  5 cents would be appreciated http://paypal.me/LDijkman
  ******************************************************************

*/
//************libraries**************//
#include "EEPROM.h"
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>                     // https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7);
RTC_DS1307 RTC;



//**************Alarm***************//
#define LED 13
#define buzzer 8

//************Variables**************//
int hourupg;
int minupg;
int yearupg;
int monthupg;
int dayupg;
int menu = 0;
int setAll = 1;
long TempLong;
char montharray[][12] = {"null", "Januari ", "Februari ", "March ", "April ", "May ", "June ", "Juli ", "August ", "September ", "Oktober ", "November ", "December "};
char dayarray[][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int alarmHours = 8, alarmMinutes = 2;  // Holds the current alarm time

#define PCF8574_ADDR (0x20)  // address of i2c port expander PCF8574 IC

// Robust Rotary encoder reading
// Copyright John Main - best-microcontroller-projects.com
#define CLK 3
#define DATA 4
static uint8_t prevNextCode = 0;
static uint16_t store = 0;


void setup() {

  lcd.begin(20, 4);
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();

  // EEPROM.put(0, alarmHours);        // EEPROM save alarmHours
  // EEPROM.put(5, alarmMinutes);      // EEPROM save alarmMinutes
  EEPROM.get(0, alarmHours);        // EEPROM get alarmHours
  EEPROM.get(5, alarmMinutes);      // EEPROM get alarmMinutes


  //rotary encoder
  pinMode(2, INPUT_PULLUP);         // D2 rotary encoder button
  pinMode(CLK, INPUT);              // D3
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);             // D4
  pinMode(DATA, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);         // D9 enable dissable alarm

  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT); // Set buzzer as an output
  printAllOff();
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    // Set the date and time at compile time
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  // RTC.adjust(DateTime(__DATE__, __TIME__)); //removing "//" to adjust the time
  // The default display shows the date and time
  int menu = 0;
}




















void loop() {
  long timebuttonpressed;
  // check if you press the SET button and increase the menu index
  TempLong = millis();  //reset innactive time counter
  if (digitalRead(2) == LOW) {
    lcd.clear();
    while (digitalRead(2) == LOW) {                                  // while button is pressed
      timebuttonpressed = (millis() - TempLong);                      // record how long button is pressed
      lcd.setCursor(0, 1);
      lcd.print("Shortpress AlarmSet");
      lcd.setCursor(0, 2);
      lcd.print("Longpress TimeSet");
      lcd.setCursor(9, 3);
      lcd.print(timebuttonpressed / 1000);
      if (timebuttonpressed  > 4000) {                             // make some noise if buttonpress longer as 4 seconds
        tone(8, 100); //output D8 Hz
        delay(100);
        noTone(8);
        delay(100);
      }
    }

    if (timebuttonpressed  > 20 && timebuttonpressed  < 4000) {     // if buttonpressed smaller/shorter as 4 seconds
      DisplaySetHourAll();
      DisplaySetMinuteAll();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("ALARM is set for:"));
      lcd.setCursor(6, 2);
      lcd.print(alarmHours);
      lcd.print(F(":"));
      lcd.print(alarmMinutes);
      delay(2500);
      lcd.clear();
    }
    if (timebuttonpressed  > 4000) {                                 // if buttonpressed greater/longer as 4 seconds
      menu = 1;
      lcd.clear();
    }
    timebuttonpressed = 0;
  }




  // in which subroutine should we go?
  if (menu == 0)
  {
    DisplayDateTime(); // void DisplayDateTime
    Alarm(); // Alarm control
  }



  //1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
  TempLong = millis();  //reset innactive time counter
  DateTime now = RTC.now();
  if (menu == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Set time hour:"));
    lcd.setCursor(0, 1);
    lcd.print(hourupg);
    lcd.print(F(" "));
  }
  while (menu == 1) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }


    int rval;
    if ( rval = read_rotary() ) {
      hourupg = hourupg + rval;
      if (hourupg < 0) hourupg = 23;
      if (hourupg > 23) hourupg = 0;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 0);
      lcd.print(F("Set time hour:"));
      lcd.setCursor(0, 1);
      lcd.print(hourupg);
      lcd.print(F(" "));
    }
    //if button pressed wait for unpress and goto next menu
    if (digitalRead(2) == LOW) {
      while (digitalRead(2) == LOW);
      menu = 2;
    }
  }



  //222222222222222222222222222222222222222222222222222222222222222222222222222222
  TempLong = millis();  //reset innactive time counter
  if (menu == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Set time minutes:"));
    lcd.setCursor(0, 1);
    lcd.print(minupg);
    lcd.print(F(" "));
  }
  while (menu == 2) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }


    int rval;
    if ( rval = read_rotary() ) {
      minupg = minupg + rval;
      if (minupg < 0) minupg = 59;
      if (minupg > 59) minupg = 0;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 0);
      lcd.print(F("Set time minutes:"));
      lcd.setCursor(0, 1);
      lcd.print(minupg);
      lcd.print(F(" "));
    }
    //if button pressed wait for unpress and goto next menu
    if (digitalRead(2) == LOW) {
      while (digitalRead(2) == LOW);
      menu = 3;
    }
  }




  //33333333333333333333333333333333333333333333333333333333333333333333333333333
  TempLong = millis();  //reset innactive time counter
  if (menu == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Set Year:"));
    lcd.setCursor(0, 1);
    lcd.print(yearupg);
    lcd.print(F(" "));
  }
  while (menu == 3) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }



    int rval;
    if ( rval = read_rotary() ) {
      yearupg = yearupg + rval;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 0);
      lcd.print(F("Set Year:"));
      lcd.setCursor(0, 1);
      lcd.print(yearupg);
      lcd.print(F(" "));
    }
    //if button pressed wait for unpress and goto next menu
    if (digitalRead(2) == LOW) {
      while (digitalRead(2) == LOW);
      menu = 4;
    }
  }



  //44444444444444444444444444444444444444444444444444444444444444444444444444444
  TempLong = millis();  //reset innactive time counter
  if (menu == 4) {
    lcd.setCursor(0, 0);
    lcd.print(F("Set Month:"));
    lcd.setCursor(0, 1);
    lcd.print(monthupg); lcd.print(F(" "));
    lcd.print(montharray[monthupg]);
    lcd.print(F("          "));
  }
  while (menu == 4) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }




    int rval;
    if ( rval = read_rotary() ) {
      monthupg = monthupg + rval;
      if (monthupg < 1) monthupg = 12;
      if (monthupg > 12) monthupg = 1;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 0);
      lcd.print(F("Set Month:"));
      lcd.setCursor(0, 1);
      lcd.print(monthupg); lcd.print(F(" "));
      lcd.print(montharray[monthupg]);
      lcd.print(F("          "));
    }
    //if button pressed wait for unpress and goto next menu
    if (digitalRead(2) == LOW) {
      while (digitalRead(2) == LOW);
      menu = 5;
    }


  }



  //55555555555555555555555555555555555555555555555555555555555555555555555555555555555
  TempLong = millis();  //reset innactive time counter
  if (menu == 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Set Day:"));
    lcd.setCursor(0, 1);
    lcd.print(dayupg);
    lcd.print(F(" "));
  }
  while (menu == 5) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }


    int rval;
    if ( rval = read_rotary() ) {
      dayupg = dayupg + rval;
      if (dayupg < 0) dayupg = 31;
      if (dayupg > 31) dayupg = 0;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 0);
      lcd.print(F("Set Day:"));
      lcd.setCursor(0, 1);
      lcd.print(dayupg);
      lcd.print(F(" "));
    }
    //if button pressed wait for unpress and goto next menu
    if (digitalRead(2) == LOW) {
      while (digitalRead(2) == LOW);
      menu = 6;
    }


  }




  //6666666666666666666666666666666666666666666666666666666666666666666666666666666666666666
  TempLong = millis();  //reset innactive time counter
  while (menu == 6) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }

    lcd.clear();
    lcd.print(F("Save settings"));
    StoreAgg();
    delay(1500);
    menu = 0;
    lcd.clear();
  }
  delay(100);
}











void DisplayDateTime ()
{
  // We show the current date and time
  DateTime now = RTC.now();

  lcd.setCursor(0, 2);
  lcd.print(F("Hour : "));

  if (now.hour() <= 9)lcd.print(F("0"));
  lcd.print(now.hour());
  hourupg = now.hour();
  lcd.print(F(":"));
  if (now.minute() <= 9)lcd.print(F("0"));
  lcd.print(now.minute());
  minupg = now.minute();
  lcd.print(F(":"));
  if (now.second() <= 9)lcd.print(F("0"));
  lcd.print(now.second());


  lcd.setCursor(0, 1);

  lcd.print(now.day());
  dayupg = now.day();
  lcd.print(F(" "));

  monthupg = now.month();

  lcd.print(montharray[now.month()]); // if it appears error in the code, enter the code given below

  lcd.print(now.year());
  yearupg = now.year();


  lcd.setCursor(0, 0);
  lcd.print(dayarray[now.dayOfTheWeek()]); // if it appears error in the code [now.dayOfWeek()];
}






void StoreAgg()
{
  // Variable saving
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SAVING IN"));
  lcd.setCursor(0, 1);
  lcd.print(F("PROGRESS"));
  RTC.adjust(DateTime(yearupg, monthupg, dayupg, hourupg, minupg, 0));
  delay(200);
}




void DisplaySetHourAll()// Setting the alarm minutes
{ lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set HOUR Alarm:"));
  lcd.setCursor(6, 2);
  if (alarmHours <= 9)lcd.print(F("0"));
  lcd.print(alarmHours);
  lcd.print(F(":"));
  lcd.setCursor(9, 2);
  if (alarmMinutes <= 9)lcd.print(F("0"));
  lcd.print(alarmMinutes);
  TempLong = millis();  //reset innactive time counter
  while (1 == 1) { //digitalRead(2) == HIGH) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }


    int rval;
    if ( rval = read_rotary() ) {
      alarmHours = alarmHours + rval;
      if (alarmHours < 0) alarmHours = 23;
      if (alarmHours > 23) alarmHours = 0;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(6, 2);
      if (alarmHours <= 9)lcd.print(F("0"));
      lcd.print(alarmHours);
      //lcd.print(F("    "));
    }

    if (digitalRead(2) == LOW)break;
  }
  EEPROM.put(0, alarmHours);        // EEPROM save alarmHours
  //EEPROM.put(5, alarmMinutes);      // EEPROM save alarmMinutes
  //EEPROM.get(0, alarmHours);        // EEPROM get alarmHours
  //EEPROM.get(5, alarmMinutes);      // EEPROM get alarmMinutes
  delay(250);

}






void DisplaySetMinuteAll()// Setting the alarm minutes
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set Minutes Alarm:"));
  lcd.setCursor(6, 2);
  if (alarmHours <= 9)lcd.print(F("0"));
  lcd.print(alarmHours);
  lcd.print(F(":"));
  lcd.setCursor(9, 2);
  if (alarmMinutes <= 9)lcd.print(F("0"));
  lcd.print(alarmMinutes);
  TempLong = millis();  //reset innactive time counter
  while (1 == 1) { //digitalRead(2) == HIGH) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }
    int rval;
    if ( rval = read_rotary() ) {
      alarmMinutes = alarmMinutes + rval;
      if (alarmMinutes < 0) alarmMinutes = 59;
      if (alarmMinutes > 59) alarmMinutes = 0;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(9, 2);
      if (alarmMinutes <= 9)lcd.print(F("0"));
      lcd.print(alarmMinutes);


    }

    if (digitalRead(2) == LOW)break;

  }
  //EEPROM.put(0, alarmHours);        // EEPROM save alarmHours
  EEPROM.put(5, alarmMinutes);      // EEPROM save alarmMinutes
  //EEPROM.get(0, alarmHours);        // EEPROM get alarmHours
  //EEPROM.get(5, alarmMinutes);      // EEPROM get alarmMinutes
  delay(250);
}










void printAllOn() {
  lcd.setCursor(0, 3);
  lcd.print(F("Alarm : "));

  if (alarmHours <= 9)lcd.print(F("0"));
  lcd.print(alarmHours);
  lcd.print(F(":"));
  if (alarmMinutes <= 9)lcd.print(F("0"));
  lcd.print(alarmMinutes);
}



void printAllOff() {
  lcd.setCursor(0, 3);
  lcd.print(F("Alarm: Off  "));
}












void Alarm() {
  if (digitalRead(9) == LOW)        // I/O D9 alarm set unset
  {
    setAll = setAll + 1;
  }
  if (setAll == 0)
  {
    printAllOff();
    noTone (buzzer);
    digitalWrite(LED, LOW);
  }
  if (setAll == 1)
  {

    printAllOn();

    DateTime now = RTC.now();
    if ( now.hour() == alarmHours && now.minute() == alarmMinutes )
    {
      lcd.noBacklight();
      DateTime now = RTC.now();
      digitalWrite(LED, HIGH);
      tone(buzzer, 880); //play the note "A5" (LA5)
      delay (300);
      tone(buzzer, 698); //play the note "F6" (FA5)
      lcd.backlight();
      Wire.beginTransmission(PCF8574_ADDR);
      Wire.write(B01111111);
      Wire.endTransmission();
    }
    else {
      noTone (buzzer);
      digitalWrite(LED, LOW);
      Wire.beginTransmission(PCF8574_ADDR);
      Wire.write(B11111111);
      Wire.endTransmission();
    }

  }
  if (setAll == 2)
  {
    setAll = 0;
  }
  delay(200);
}











boolean SetButton() {
  boolean sval;
  sval = digitalRead(2);
  if (sval == 0) {                 // make a buzz when button pressed
    tone(8, 200); //output D8 Hz
    delay(100);
    noTone(8);
    delay(250);
  }
  //Serial.print(F("SetButton="));
  //Serial.println(sval);
  return sval;
}






void TimeOut() {
  lcd.clear();  //exit menu if 20 seconds innactive
  lcd.setCursor(0, 1);
  lcd.print(F("     TimeOut    "));
  lcd.setCursor(0, 2);
  lcd.print(F("  Start Screen  "));
  delay(2000);
  lcd.clear();
  menu = 6; // menu 6 is savedata
}







int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

  // Robust Rotary encoder reading
  // Copyright John Main - best-microcontroller-projects.com
  // https://www.best-microcontroller-projects.com/rotary-encoder.html

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
    store <<= 4;
    store |= prevNextCode;
    //if (store==0xd42b) return 1;
    //if (store==0xe817) return -1;
    if ((store & 0xff) == 0x2b) return -1;
    if ((store & 0xff) == 0x17) return 1;
  }
  return 0;

  // Robust Rotary encoder reading
  // Copyright John Main - best-microcontroller-projects.com
  // https://www.best-microcontroller-projects.com/rotary-encoder.html
}



/*
  :Project:Allarm_Clock
  :Author: Tiziano Bianchettin
  :Date: 10/02/2017
  :Revision: 2
  :License: Public Domain
  original source http://create.arduino.cc/projecthub/Tittiamo/alarm-clock-f61bad
   thanks to: My electronics laboratory professor "Perito Carli"

 **********************************************************************
  5 cents would be appreciated http://paypal.me/pools/c/8amUN5rgb9
  http://sticker.tk/forum/index.php?action=view&id=299
  Changes made by luberth dijkman juli 2019
  save alarmtime to eeprom for reload on reboot
  rotary encoder button for input
  timeout on menus
  NOT FINNISHED YET
  and much more
  http://sticker.tk/forum/index.php?action=view&id=299
  5 cents would be appreciated http://paypal.me/LDijkman
  ******************************************************************
https://github.com/ldijkman/arduino_rotary_encoder_alarm_clock
*/
