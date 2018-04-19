# rgb-60
arduino IDE code for ESP8266, wemos D1 mini lite

(Android app RGB60 has been published and should be in the playstore, free and without ads.)

This code is for use with Android app RGB60, but does not require that app. You can add your wifi SSID and password to the top of avrdude_cmdline and connect 60 RGB LEDs WS8212b type to the appropriate data pin, and your ESP8266 should immediately fetch a small design file from my web server and display it on your leds.

The "normal" way to use this code is to allow your device to fetch several files which will be stored in RAM and played one after another based on the cycle time you enter in the code before flashing, or you set with the app.

If you choose to use the app RGB60, you can easily make your own color schemes and either send them to your device via your wifi, or you can upload them to my server and your device will then load them whenever it powers on or resets.

Known issues: even though these units have both a hardware watchdog and a software watchdog, both of mine occasionally hang and don't reset until I press the button. Then they typically run for hours and still occasionally have a hardware reset. The most recent reset cause can be found by the app button "GET STATUS". 
