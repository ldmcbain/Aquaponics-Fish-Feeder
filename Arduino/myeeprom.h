#ifndef _MYEEPROMH_
#define _MYEEPROMH_

#include <EEPROM.h>
#include <Arduino.h>

// Values saved to EEPROM for persistent storage of settings
struct StoredSettings
  {
    char T1[10];
    char T2[10];
    char T3[10];
    char T4[10];
    bool T1_enable;
    bool T2_enable;
    bool T3_enable;
    bool T4_enable;
  };

extern StoredSettings romSettings;

StoredSettings getStoredSettings();
//void wipeSettings();
void saveSettings(String t1, String t2, String t3, String t4, bool t1e, bool t2e, bool t3e, bool t4e);

#endif
