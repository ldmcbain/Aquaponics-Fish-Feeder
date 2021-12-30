#include "./myeeprom.h"


StoredSettings romSettings;



/**
 * Retrieves stored settings from EEPROM so that WiFi setup does not have to be repeated on every power cycle
 * 
 * @return struct
 * @author costmo
 * @since  20180929
 */
StoredSettings getStoredSettings()
{
  uint addr = 0;
  EEPROM.begin( 512 );
  EEPROM.get( addr, romSettings );
  EEPROM.end();
  /*
  Serial.println("EEPROM c_str RESTORE values:");
  Serial.printf("R1:%s G1:%s B1:%s W1:%s ",romSettings.RD1,romSettings.GR1,romSettings.BL1,romSettings.WH1);
  Serial.printf("R2:%s G2:%s B2:%s W2:%s ",romSettings.RD2,romSettings.GR2,romSettings.BL2,romSettings.WH2);
  Serial.printf("R3:%s G3:%s B3:%s W3:%s ",romSettings.RD3,romSettings.GR3,romSettings.BL3,romSettings.WH3);
  Serial.println("");
  */
  return romSettings;
}


/**
 * Stores the input settings to EEPROM
 * 
 * @return void
 * @author costmo
 * @since  20180929
 */
void saveSettings(String t1, String t2, String t3, String t4, bool t1e, bool t2e, bool t3e, bool t4e)
{
  uint addr = 0;

  strcpy( romSettings.T1, t1.c_str() );
  strcpy( romSettings.T2, t2.c_str() );
  strcpy( romSettings.T3, t3.c_str() );
  strcpy( romSettings.T4, t4.c_str() );
  romSettings.T1_enable=t1e;
  romSettings.T2_enable=t2e;
  romSettings.T3_enable=t3e;
  romSettings.T4_enable=t4e;
  
/*
  Serial.println("EEPROM c_str values:");
  Serial.printf("R1:%s G1:%s B1:%s W1:%s ",romSettings.RD1,romSettings.GR1,romSettings.BL1,romSettings.WH1);
  Serial.printf("R2:%s G2:%s B2:%s W2:%s ",romSettings.RD2,romSettings.GR2,romSettings.BL2,romSettings.WH2);
  Serial.printf("R3:%s G3:%s B3:%s W3:%s ",romSettings.RD3,romSettings.GR3,romSettings.BL3,romSettings.WH3);
  Serial.println("");
*/
  EEPROM.begin( 512 );
  EEPROM.put( addr, romSettings );
  EEPROM.end();
}
