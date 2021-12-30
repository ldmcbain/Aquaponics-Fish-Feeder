/*
 * ENDEAVOR Web based Aquaponics Controller
 * This project was created by Endeavor Labs (a mission of Endeavor Church) to create
 * an Arduino based controller to service an aquaponics setup meant for a class setting.
 * 
 * The system is to be designed to use a ESP32 Dev board to control:
 *  - 3D printed fish feeder driven by servo motor
 *      Based on https://www.thingiverse.com/thing:4627735, modified to hook onto tank
 *  - Water temperature sensor
 *  - Air Temperature/Humidity sensor
 *  - Water level sensor
 *  - Pump cutoff based upon low water level reading
 *  - Real Time Clock (RTC) / Data loger (SD card) used for obtaining date/time for feeding etc
 *  - Web based interface for viewing data, manual feed control, data log entry and viewing. (stored on SD card)
 * 
 * This software is designed to create a permanent Access Point with a captive portal
 * to a webpage which will control the Feeder and view/monitor sensors and logs as noted above
 * 
 * Will utilize Plotly JS script library (plotly.js stored in SPIFFS) to view data graphs.
 * Stored data logs and sensor logs will be stored on SD card
 * 
 * I have found partial plotly files which will likely work and are much smaller.  using minimized basic package located here:
 * https://github.com/plotly/plotly.js/blob/master/dist/README.md
 * also possible to create own package customized to only what I need:
 * https://github.com/plotly/plotly.js/blob/master/CUSTOM_BUNDLE.md
 * Note: I may have issues w/ plotly as it is LARGE and unsure how to reference a JS file on an SD card.
 * Alternatives may be required.
 * https://forum.arduino.cc/t/arduino-mega-json-rest-interfaces-for-html-guis/126471/9
 * 
 * 
 * Note for possible future upgrade.. ability to upload data directly to web based server.
 * current use is offline-only.
 *      https://github.com/plotly/arduino-api
 *      https://www.instructables.com/Plotly-Arduino-Data-Visualization/
 * 
 * This version is based upon initial work with the Endeavor Sign V2 and is using that project
 * as a base.  It may possibly also utilize the Endeavor Labs ESP32 board.
 * 
 * Board - ESP32S Dev Module (thin version of ESP module [22.86mm pin width] from HiLetgo ESP-WROOM-32S)
 *          I believe this is a NodeMCU ESP32S board design https://www.amazon.com/gp/product/B0718T232Z/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
 * Upload Speed - 115200
 * CPU Freq - 240MHz
 * Flash Freq 40MHz
 * Mode - QIO
 * Flash size - 4MB (32Mb)
 * Partition - Default 4MB wigh SPIFFS (1.2MB APP/1.5MB SPIFFS)
 */

#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "./server_html.h"
#include "./myeeprom.h"
#include <SPIFFS.h>
#include <Wire.h>
#include "RTClib.h"
#include "DST_RTC.h"

#define FILESYSTEM SPIFFS

//motor pins
#define MOTOR_A 12
#define MOTOR_B 13
#define MOTOR_C 14
#define MOTOR_D 15
#define NUMBER_OF_STEPS_PER_REV 512


// Replace with your network credentials
const char* ssid     = "AQUA_P_";
const char* password = NULL;  //making this an open access?

// Set web server port number to 80
WebServer server(80);
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

RTC_DS1307 rtc; // clock object
DST_RTC dst_rtc; // DST object
const char rulesDST[]="US";

uint8_t Alarm1, Alarm2, Alarm3, Alarm4;

void handleRoot()
{
  buildWebsite();
  server.send(200, "text/html", webSite);
}

void handleFeed()
{
  //Manually engage servo to feed fish
  Serial.printf("%02i/%02i/%i_%02i:%02i:%02i - Feeding Fish..\n",month(),day(),year(),hour(),minute(),second());
  moveDeg(36);
  MotorOff();
}

void handleSetTime()
{
  //set the time based upon something from the website..
  //Serial.printf("attempting to set time to: %s/%s/%s %s:%s:%s",server.arg("MM"),server.arg("D"),server.arg("Y"),server.arg("H"),server.arg("m"),server.arg("s"));
  //Serial.println("");
  settime(atoi(server.arg("MM").c_str()),
          atoi(server.arg("D").c_str()),
          atoi(server.arg("Y").c_str()),
          atoi(server.arg("H").c_str()),
          atoi(server.arg("m").c_str()),
          atoi(server.arg("s").c_str()));
}

void settime(int Month, int Day, int Year, int Hour, int Min, int Sec)
{
  //set the RTC time clock
  rtc.adjust(DateTime(Year, Month, Day, Hour, Min, Sec));
  Serial.printf("Time set to: %02i/%02i/%i %02i:%02i:%02i",Month,Day,Year,Hour,Min,Sec);
  Serial.println("");
  //init time for time.h
  setTime(Hour, Min, Sec, Day, Month, Year);
  
  // DST? If we're in it, let's subtract an hour from the RTC time to keep our DST calculation correct. This gives us
  // Standard Time which our DST check will add an hour back to if we're in DST.
  DateTime standardTime = now();
  if (dst_rtc.checkDST(standardTime) == true) { // check whether we're in DST right now. If we are, subtract an hour.
    standardTime = standardTime.unixtime() - 3600;
    rtc.adjust(standardTime);
    Serial.println("Time adjusted for Daylight Savings Time");
  }
}

//function will parse out a time in string format "HH:MMam" to integers
void ParseTime(char* Instr, int* Hour, int* Min, bool* AM)
{
  //char param[10];
  char out[3] = "";
  int colon;
  colon=0;
  //strcopy(param,Instr);
  for(int i=0;i<strlen(Instr);i++)
  {
    if(Instr[i]==':')
    {
      out[i]=0;
      *Hour=atoi(out);
      colon=i+1;
      out[0]=0;
    } else if((Instr[i]!='a') && (Instr[i]!='p'))
    {
      out[i-colon]=Instr[i];
    } else
    {
      out[i]=0;
      *Min=atoi(out);
      if(Instr[i]=='a') *AM=true; else *AM=false;
    }
  }
}

//clear existing alarms and set new ones.
void handleSetAlarms()
{
  int Hour, Min;
  bool AM;
  char instr[10];

  if(Alarm.isAllocated(Alarm1))
  {
    Alarm.disable(Alarm1);
    Alarm.free(Alarm1);
    Serial.println("Alarm1 Freed");
  }
  if(Alarm.isAllocated(Alarm2))
  {
    Alarm.disable(Alarm2);
    Alarm.free(Alarm2);
    Serial.println("Alarm1 Freed");
  }
  if(Alarm.isAllocated(Alarm3))
  {
    Alarm.disable(Alarm3);
    Alarm.free(Alarm3);
    Serial.println("Alarm1 Freed");
  }
  if(Alarm.isAllocated(Alarm4))
  {
    Alarm.disable(Alarm4);
    Alarm.free(Alarm4);
    Serial.println("Alarm1 Freed");
  }
  
  if(server.hasArg("E1"))
  {
    strcpy(instr,server.arg("E1").c_str());
    T1_time = server.arg("E1");
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event1 set for: %02i:%02i:00\n",Hour,Min);
    //sprintf(T1_time,"%02i:%02i\n",Hour,Min);
    T1_e = true;

    Alarm1 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  } else T1_e = false;
  if(server.hasArg("E2"))
  {
    strcpy(instr,server.arg("E2").c_str());
    T2_time = server.arg("E2");
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event2 set for: %02i:%02i:00\n",Hour,Min);
    T2_e = true;
    
    Alarm2 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  } else T2_e = false;
  if(server.hasArg("E3"))
  {
    strcpy(instr,server.arg("E3").c_str());
    T3_time = server.arg("E3");
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event3 set for: %02i:%02i:00\n",Hour,Min);
    //sprintf(T3_time,"%02i:%02i\n",Hour,Min);
    T3_e = true;
  
    Alarm3 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  } else T3_e = false;
  if(server.hasArg("E4"))
  {
    strcpy(instr,server.arg("E4").c_str());
    T4_time = server.arg("E4");
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event4 set for: %02i:%02i:00\n",Hour,Min);
    //sprintf(T4_time,"%02i:%02i\n",Hour,Min);
    T4_e = true;
  
    Alarm4 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  } else T4_e = false;
  handleSAVE();
}


time_t syncProvider()
{
  DateTime standardTime = rtc.now();
  
  DateTime theTime = dst_rtc.calculateTime(standardTime); // takes into account DST
  return theTime.unixtime();
}


void handleSAVE()
{
  Serial.println("Saving Settings");
  
  Serial.printf("T1-%s %s\n",T1_time,T1_e ? "Enabled" : "Disabled");
  Serial.printf("T2-%s %s\n",T2_time,T2_e ? "Enabled" : "Disabled");
  Serial.printf("T3-%s %s\n",T3_time,T3_e ? "Enabled" : "Disabled");
  Serial.printf("T4-%s %s\n",T4_time,T4_e ? "Enabled" : "Disabled");
  
  saveSettings(T1_time,T2_time,T3_time,T4_time,T1_e,T2_e,T3_e,T4_e);
}

void handleRESTORE()
{
  int Hour, Min;
  bool AM;
  char instr[10];
  
  getStoredSettings();
  T1_time=String(romSettings.T1);
  T2_time=String(romSettings.T2);
  T3_time=String(romSettings.T3);
  T4_time=String(romSettings.T4);
  T1_e = romSettings.T1_enable;
  T2_e = romSettings.T2_enable;
  T3_e = romSettings.T3_enable;
  T4_e = romSettings.T4_enable;

  if(T1_e)
  {
    strcpy(instr,T1_time.c_str());
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event1 set for: %02i:%02i:00\n",Hour,Min);  
    Alarm1 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  }
  if(T2_e)
  {
    strcpy(instr,T2_time.c_str());
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event2 set for: %02i:%02i:00\n",Hour,Min);  
    Alarm2 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  }
  if(T3_e)
  {
    strcpy(instr,T3_time.c_str());
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event3 set for: %02i:%02i:00\n",Hour,Min);  
    Alarm3 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  }
  if(T4_e)
  {
    strcpy(instr,T4_time.c_str());
    ParseTime(instr,&Hour,&Min, &AM);
    if(AM && (Hour==12)) Hour = 0;
    if(!AM && (Hour!=12)) Hour += 12;
    Serial.printf("Event4 set for: %02i:%02i:00\n",Hour,Min);  
    Alarm4 = Alarm.alarmRepeat(Hour,Min,0,handleFeed);
  }
}

//function will parse out a time in string format "HH:MMam" to integers
void parseStored(char* Instr, int* Hour, int* Min, bool* AM)
{
  //char param[10];
  char out[3] = "";
  int colon;
  colon=0;
  //strcopy(param,Instr);
  for(int i=0;i<strlen(Instr);i++)
  {
    if(Instr[i]==':')
    {
      out[i]=0;
      *Hour=atoi(out);
      colon=i+1;
      out[0]=0;
    } else if((Instr[i]!='a') && (Instr[i]!='p'))
    {
      out[i-colon]=Instr[i];
    } else
    {
      out[i]=0;
      *Min=atoi(out);
      if(Instr[i]=='a') *AM=true; else *AM=false;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  //Initialize Clock
  Wire.begin();
  rtc.begin();
  
  /*
    This line sets the RTC with an explicit date & time (standard time - not DST), for example to set
    March 28, 2020 at 23:58:5 you would call:
    // rtc.adjust(DateTime(2020, 3, 28, 23, 58, 5));
    If used load the sketch a second time with this line commented out or the RTC will reset to
    this time on power up or reset.
  */
  
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
    // DST? If we're in it, let's subtract an hour from the RTC time to keep our DST calculation correct. This gives us
    // Standard Time which our DST check will add an hour back to if we're in DST.
    /*
    DateTime standardTime = rtc.now();
    if (dst_rtc.checkDST(standardTime) == true) { // check whether we're in DST right now. If we are, subtract an hour.
      standardTime = standardTime.unixtime() - 3600;
      rtc.adjust(standardTime);
    }
    */
    
  }
  //setSyncProvider(syncProvider);
  if (timeStatus()!=timeSet)
  {
    //give a default time if power lost so feeder will continue.
    settime(1,1,2021,1,0,0);
  }

  //may need to comment this out for the 1st run to init the save points first
  handleRESTORE();

  //Create and initialize alarms
  /*Alarm1 = Alarm.alarmRepeat(0,0,0,nullFunction);
  Alarm2 = Alarm.alarmRepeat(1,0,0,nullFunction);
  Alarm3 = Alarm.alarmRepeat(2,0,0,nullFunction);
  Alarm4 = Alarm.alarmRepeat(3,0,0,nullFunction);
  Alarm.disable(Alarm1);
  Alarm.disable(Alarm2);
  Alarm.disable(Alarm3);
  Alarm.disable(Alarm4);*/
  //todo - load from eprom stored values.

  // Initialize SPIFFS
  if(!FILESYSTEM.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Starting AP ");
  Serial.println(ssid);

  //set SSID based upon the mac address of the device (in case we have multiple)
  //strcat(ssid,WiFi.softAPmacAddress().c_str());
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP(ssid, password) ? "soft-AP setup": "Failed to connect");
  delay(100);
  Serial.println(WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0))? "Configuring Soft AP" : "Error in Configuration");


  //setup captive portal so all traffic goes to main page
  dnsServer.start(DNS_PORT, "*", apIP);
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP:");
  Serial.println(WiFi.softAPIP());
  startWebServer();

  
  pinMode(MOTOR_A,OUTPUT);
  pinMode(MOTOR_B,OUTPUT);
  pinMode(MOTOR_C,OUTPUT);
  pinMode(MOTOR_D,OUTPUT);
}

void loop(){
  //DateTime standardTime = rtc.now();

  //Serial.println("Standard Time");
  //printTheTime(standardTime);
  
  //DateTime theTime = dst_rtc.calculateTime(standardTime); // takes into account DST

  //Serial.println("time adjusted for Daylight Saving Time");
  //printTheTime(theTime);
  
  dnsServer.processNextRequest();
  server.handleClient();
  Alarm.delay(1);
}


// print time to serial
void printTheTime(DateTime theTimeP) {
  Serial.print(theTimeP.year(), DEC);
  Serial.print('/');
  Serial.print(theTimeP.month(), DEC);
  Serial.print('/');
  Serial.print(theTimeP.day(), DEC);
  Serial.print(' ');
  Serial.print(theTimeP.hour(), DEC);
  Serial.print(':');
  Serial.print(theTimeP.minute(), DEC);
  Serial.print(':');
  Serial.print(theTimeP.second(), DEC);
  Serial.println();
}

void startWebServer()
{  
  //setup to send files stored in SPIFFS when requested
  server.on("/plotly-basic.js", HTTP_GET, [](){
    if (!handleFileRead("/plotly-basic.js")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/GridStyle.css", HTTP_GET, [](){
    if (!handleFileRead("/GridStyle.css")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/MainStyle.css", HTTP_GET, [](){
    if (!handleFileRead("/MainStyle.css")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/jquery-3.6.0.slim.min.js", HTTP_GET, [](){
    if (!handleFileRead("/jquery-3.6.0.slim.min.js")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/jquery.timepicker.js", HTTP_GET, [](){
    if (!handleFileRead("/jquery.timepicker.js")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/jquery.timepicker.css", HTTP_GET, [](){
    if (!handleFileRead("/jquery.timepicker.css")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on( "/", handleRoot);
  server.on("/settime",handleSetTime);
  server.on("/feed",handleFeed);
  server.on("/savealarms",handleSetAlarms);
  server.onNotFound(handleRoot);
  server.begin();
}

bool handleFileRead(String path) {
  String contentType = getContentType(path);
  if (exists(path)) {
    File file = FILESYSTEM.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  File file = FILESYSTEM.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}


void Motorwrite(int a,int b,int c,int d){
digitalWrite(MOTOR_A,a);
digitalWrite(MOTOR_B,b);
digitalWrite(MOTOR_C,c);
digitalWrite(MOTOR_D,d);
}

void MotorOff()
{
Motorwrite(0,0,0,0);
}

void onestepfwd(int delayval){
Motorwrite(1,0,0,0);
Alarm.delay(delayval);
Motorwrite(1,1,0,0);
Alarm.delay(delayval);
Motorwrite(0,1,0,0);
Alarm.delay(delayval);
Motorwrite(0,1,1,0);
Alarm.delay(delayval);
Motorwrite(0,0,1,0);
Alarm.delay(delayval);
Motorwrite(0,0,1,1);
Alarm.delay(delayval);
Motorwrite(0,0,0,1);
Alarm.delay(delayval);
Motorwrite(1,0,0,1);
Alarm.delay(delayval);
}

void onestepbck(int delayval){
Motorwrite(0,0,0,1);
Alarm.delay(delayval);
Motorwrite(0,0,1,1);
Alarm.delay(delayval);
Motorwrite(0,0,1,0);
Alarm.delay(delayval);
Motorwrite(0,1,1,0);
Alarm.delay(delayval);
Motorwrite(0,1,0,0);
Alarm.delay(delayval);
Motorwrite(1,1,0,0);
Alarm.delay(delayval);
Motorwrite(1,0,0,0);
Alarm.delay(delayval);
Motorwrite(1,0,0,1);
Alarm.delay(delayval);
}

void moveDeg(float degree)
{
  int i;
  if(degree>0){for(i=0;i<((degree/360)*NUMBER_OF_STEPS_PER_REV);i++){onestepfwd(10);}}
  else{for(i=0;i>((degree/360)*NUMBER_OF_STEPS_PER_REV);i--){onestepbck(10);}}
}
