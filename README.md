# rgb-60
arduino IDE code for ESP8266, wemos D1 mini lite

(Android app RGB60 has not been published in the playstore as of this minute, but it will soon be available, free and without ads.)

This code is for use with Android app RGB60, but does not require that app. You can add your wifi SSID and password to the top of avrdude_cmdline and connect 60 RGB LEDs WS8212b type to the appropriate data pin, and your ESP8266 should immediately fetch a small design file from my web server and display it on your leds.

The "normal" way to use this code is to allow your device to fetch several files which will be stored in RAM and played one after another based on the cycle time you enter in the code before flashing, or you set with the app.

If you choose to use the app RGB60, you can easily make your own color schemes and either send them to your device via your wifi, or you can upload them to my server and your device will then load them whenever it powers on or resets.
