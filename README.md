# Aquaponics Fish Feeder
 ESP32 Aquaponics Fish Feeder

This is a mod of a design I found in thingiverse:
https://www.thingiverse.com/thing:497637

This was designed using an ESP32 devkit board along with a 28BYJ-48 stepper motor and ULN2003 driver board (amazon).

The ESP32 is setup to create a WIFI captive portal page which is used to setup timing on the fish feeding intervals.  You can set the time/date on the device but know that it will loose the time/date on power reset.  However the feeding interval is stored in EEPROM, so it will just reset the clock to midnight and keep on going if power is interrupted.  This is designed only for a few fish, thus the small feeder holes.  The design was changed from the original as i found that the food we used would go underneath the original paddle wheels.

I also conformal coated the ESP board and driver board before deploying it (MG Chemicals - 422C-55MLCA 422C Silicone Conformal Coating - available from amazon)
