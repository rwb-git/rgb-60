// some constants are hard-coded  in espShow() (in file rgb.ino), which are from my tests with 80 MHz nodeMCU/esp8266. you can easily
// change that code to use the adafruit calculated values, or enter your own.

// some settings that you can change with the app are saved in a small SPIFFS file
//
// Tools - Erase Falsh - Only Sketch     -> so it won't erase files in SPIFFS
//
// Tools - Flash Size - 1M(512K SPIFFS)   -> this worked in my wemos D1 mini lite and my nodeMCU. any value > 0 for SPIFFS should be ok

// enter your router SSID and password
const char * ssid = "NETGEAR84";
const char * password = "rb76543aaa789&*()cvbn";

#define DATA_PIN D5              // this is the pin that you connect to the LED data line. you can change it here to use another GPIO pin
                            // you might need a level shifter since the esp is 3.3 volts and the LED is 5 volts, but sometimes the lower
                            // voltage is ok.

//#define scope_calc D1     // just some scope testing. you don't need these two.
//#define scope_rgb D2



//#define serial1 1         // un-comment this to enable lots of printing to the serial monitor. sidenote: when I flash my esp and have the USB cable
                            // connected, my esp will eventually hang. I think it's stuck in the mode where it is waiting for the bootloader to write
                            // the flash. sometimes the watchdog timers reset and sometimes they don't. but when the USB cable is disconnected, my
                            // device runs for many hours without resetting or hanging. So, if your esp gets stuck and stops cycling the leds, try
                            // disconnecting the USB cable, which means the esp will need external power.


#define actual_max_ram_files  50  // this is used to create arrays once. I'm not sure what the upper limit actually is.

// the parameters in the following section can be changed with android app RGB60, or you can edit them here and the app is not needed

uint16_t max_ram_files = 33;// this is the maximum number of design files that will be loaded into ram. the code also checks available ram so if
                            // this value is too large it won't hurt. the number of files on the web server is checked as well, and if all of
                            // those have been fetched it will stop trying to download files. this value cannot exceed actual_max_ram_files
                            
uint16_t cycle_delay = 8;   // seconds between file change when cycling local files in ram or fetching from web

#define rgbleds 60          // the best value here is 60, but you can use more or less if you like. designs fetched from the web will look
                            // different if the led count does not match the original. please use 60. or don't :)

int fetch_from_web = 1;     // if this is 1 and auto_play == 1, the esp will try to load files from the web. change it to 0 to disable this feature.

uint8_t led_blink_mode = 0; // 0 about 1 hz.   1 = blink when loading a new file from ram or the web or the app.    2 led is ofoff

uint8_t auto_play = 1;      // 1 = on = play the next file after cycle_delay seconds.  if fetch_from_web == 1, it will try to download files. 
                            // 0 = off = play the current design forever.


// --------------- in general you should not need to edit anything below this line or in the other files -------------------------------------------


//  ---------- some notes you can ignore, and I should delete -----------------

// multiple files (tabs)
//
//    if they don't have extensions they are just added to main file. if they are .h .cpp .c you have to #include. main file is C++ so maybe try to program in C++


// changing led and soft to 32bit fixed most of the soft flicker. 32 bits? maybe try 16 uint, with the fix so that negative deltas work?

// dr dg db need 32 bits, or uint? does int16 work when it can be 255x256 delta?

// does bad url crash? send 234 instead of 442 and crash? 
//
//    maybe it assumes paych is good design. need some sanity checks, and don't go past paych end
//
// should everything be int16_t? uint16_t? does 8 bit make sense here? get the web design and << 8. do int16 math, then >> 8 for leds?
//
//   strip has to be re-done if number of leds is chnged via app
//
// if web led cnt is different need to adjust
//
// load 50 designs or whatever into ram and then recycle them instead of web fetch. store the parsed version?


// ESP.restart();  // restarts the CPU
//
// gpio pins can be referenced by the board labels: D1 D2... digitalWrite(D2). also git doc says you can use gpio number: GPIO2 = 2 as in digitalWrite(2)

//
// anything that takes > 50 msec should call elay(n), or yield() which is equivalent to delay(0)

// Serial.setDebugOutput(true) enables diagnostic output from wifi libraries

// use Progmem to save designs in flash? current upload says 1044464 flash of which 26% 274280 is used, leaving about 754k. typical design is about 400 bytes so
// that would hold 1400+ designs. so fetch from web once, then periodically send max file ID to get number of new files then fetch them all. I'd need to 
// approve new files before adding to this queue to keep someone from flooding everyone's flash. DERP flash can only be written by bootloader (i think avr has
// a mode where it can be done at runtime but esp8266 might not have that capability)
//
//    so just store in ram, and that way you fetch all the designs from web only once per boot. if I need to control bandwidth I could limit online designs to
//    50 or whatever, maybe rotating daily. since nobody is going to use this don't worry too much about bandwidth.

//
/*   exception codes
 *    
 *      0 can be bad pointer or other
 *      6 divide by zero
 *      9 bad pointer or other
 *      28  "
 *      29  "
 *      
 *   reset codes printed in ROM code on boot (and they don't change after soft wdt or software reset); i don't know how to see these
 *   
 *      1 power on
 *      2 external
 *      3 hardware wdt
 *      
 *   reset codes from get rst info
 *   
 *      0 power
 *      1 hardware wdt
 *      2 fatal exception
 *      3 soft wdt
 *      4 soft reset
 *      5 deep sleep
 *      6 hardware reset
 */



#include "FS.h"   // spiffs file system

#include <Hash.h>

#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>

#if defined(ESP8266) 
// ESP8266 show() is external to enforce ICACHE_RAM_ATTR execution
extern "C" void ICACHE_RAM_ATTR espShow(
  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
#elif defined(ESP32)
extern "C" void espShow(
  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
#endif // ESP8266

#define one_sec_led BUILTIN_LED  //D4    // builtin led on nodemcu



/*
 * 
 * 
 * 
 * Some background information in case you want to look at the code; you don't need to know any of this to use the sketch and the android app.
 * 
 * 
 *    each led has a structure in sram:
 *    
 *      16 bit green. low byte first, then high byte, same order as C++. low byte is like a fractional part, which enables smooth transitions
 *      high byte is the one that is sent to the led.
 *      
 *      16 bit red. yeah, the leds expect data in grb order.
 *      16 bit blue
 *      
 *      color index: this is the color scheme for this led that you set in the app. every led can use the same color scheme, or every led can have its
 *      own color scheme if sufficient sram is there, or you can have any other combination.
 *      
 *      state: this is the initial state for this led that you set in the profile screen of the app when you drag the line up or down.
 *      
 *      frame: this is the point within a state that is also set by the profile. each color scheme has one or more states with one or more blend frames
 *      and one or more hold frames, which you set in the "edit color" screen of the app. 
 *      
 *   color schemes are sent to the arduino:
 *      
 *     states = number of states for this color scheme. in the app, under "edit color", these are the rectangles over at the far right
 *        
 *     each state is then sent in 11 bytes
 *        
 *     the first three bytes are green red and blue hold values. these values are sent to the led for all the hold frames
 *     
 *     the next six bytes are the delta values that the app calculates for a smooth transition during the blend frames. they are in the same
 *     order as the first six bytes in the led structs.
 *        
 *     total frames for this color scheme; this is the total for the hold and blend frames from the edit color screen. since this is one byte
 *     that total is limited to 255
 *        
 *     hold frames is the final byte in a color scheme struct
 * 
 * 
 */


int wifistatus;

int files_on_web = 5; // need > 0 for first fetch, then it gets the actual count.

HTTPClient http;

int auto_save = 1; // 1 = save files in ram. 0 = disable when editing design

uint32_t heap;

uint16_t current_local_file = 0;

uint16_t files_fetched = 0;

uint16_t web_files_fetched = 0;

uint16_t app_msg;




uint8_t ram_file_type[actual_max_ram_files]; // 0 web file, 1 app file

char * ram_ptrs[actual_max_ram_files]; // save N files in ram. after that, stop fetching them and strdup these strings

uint8_t pixels[rgbleds * 3];

WiFiServer server(80);

uint32_t uptime_minutes;

int print_344 = 0;

int filecnt = 0;

uint16_t ledcnt = 0;

uint8_t hash[actual_max_ram_files][20];

char * file_ID_str;

int debug3 = 0;

uint16_t itest,web_led_cnt,web_color_cnt,file_ID;

struct color_state{

  int16_t dr,dg,db;

  uint8_t frames,hold,r,g,b;
  
};

struct color_scheme{

  uint16_t total_frames;

  uint16_t states;

  color_state * color_states;

  
};

color_scheme * color_schemes;

uint8_t color_scheme_cnt;

struct rgb{

  uint8_t r,g,b;
  
};


struct rgb_int16{

  int16_t r,g,b;
  
};

struct led_struct{

  uint8_t frame,state,color;

  uint16_t r,g,b;
  

};


int16_t debug5 = 0;

uint16_t debug = 0;


boolean load_one = false;

uint16_t   soft_start, // number of frames used to transition from one design to the next when the app sends a new one. this value is set to 130 (I think) in the app
          
          color_cnt;  // the number of color schemes in this design. quite often this is one, but it can theoretically be as large as rgbleds
          
          

uint16_t  max_design; // keep track of the largest design we have uploaded to make it easier to find unused sram

rgb soft_goal[rgbleds];

rgb_int16 soft_delta[rgbleds];

led_struct led_block[rgbleds];




uint32_t cycles, rgbcycles,millis2,millisrgb;




//----------------------------------------------------------------------------------------------------------------------





void setup() {



  uint32_t ccnt = ESP.getCycleCount();

  cycles = ccnt;

  millis2 = millis();
  millisrgb = millis2;

  
  pinMode(one_sec_led, OUTPUT);

#ifdef scope_rgb
  pinMode(scope_rgb, OUTPUT);
  pinMode(scope_calc, OUTPUT);
#endif
  
  pinMode(DATA_PIN, OUTPUT);




#ifdef serial1

  Serial.begin(115200);

  Serial.println();


  Serial.printf("Connecting to %s ", ssid);
#endif



    heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. it varies with design file size but should be the same for 
                               // a particular file
#ifdef serial1                                        
    Serial.print(" heap ");
    Serial.println(heap);

#endif

  check_spiffs_format();
  
  try_to_read_settings_file();
  
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
#ifdef serial1    
    Serial.print(".");
#endif    
  }
  

#ifdef serial1  
  Serial.println(" connected");
#endif

  server.begin();










  
  
/**
 *    from ESP.h which i cannot find locally but is on web
 * 
 * AVR macros for WDT managment
 
      typedef enum {
          WDTO_0MS    = 0,   //!< WDTO_0MS
          WDTO_15MS   = 15,  //!< WDTO_15MS
          WDTO_30MS   = 30,  //!< WDTO_30MS
          WDTO_60MS   = 60,  //!< WDTO_60MS
          WDTO_120MS  = 120, //!< WDTO_120MS
          WDTO_250MS  = 250, //!< WDTO_250MS
          WDTO_500MS  = 500, //!< WDTO_500MS
          WDTO_1S     = 1000,//!< WDTO_1S
          WDTO_2S     = 2000,//!< WDTO_2S
          WDTO_4S     = 4000,//!< WDTO_4S
          WDTO_8S     = 8000 //!< WDTO_8S
      } WDTO_t;
      
      
      #define wdt_enable(time)    ESP.wdtEnable(time)
      #define wdt_disable()       ESP.wdtDisable()
      #define wdt_reset()         ESP.wdtFeed()


      
        void wdtDisable();
        void wdtFeed();

        void deepSleep(uint64_t time_us, RFMode mode = RF_DEFAULT);
        uint64_t deepSleepMax();

        bool rtcUserMemoryRead(uint32_t offset, uint32_t *data, size_t size);
        bool rtcUserMemoryWrite(uint32_t offset, uint32_t *data, size_t size);

        void reset();
        void restart();

        uint16_t getVcc();
        uint32_t getFreeHeap();

        uint32_t getChipId();

        const char * getSdkVersion();
        String getCoreVersion();
        String getFullVersion();

        uint8_t getBootVersion();
        uint8_t getBootMode();

        uint8_t getCpuFreqMHz();

        uint32_t getFlashChipId();
        //gets the actual chip size based on the flash id
        uint32_t getFlashChipRealSize();
        //gets the size of the flash as set by the compiler
        uint32_t getFlashChipSize();
        uint32_t getFlashChipSpeed();
        FlashMode_t getFlashChipMode();
        uint32_t getFlashChipSizeByChipId();

        uint32_t magicFlashChipSize(uint8_t byte);
        uint32_t magicFlashChipSpeed(uint8_t byte);
        FlashMode_t magicFlashChipMode(uint8_t byte);

        bool checkFlashConfig(bool needsEquals = false);

        bool flashEraseSector(uint32_t sector);
        bool flashWrite(uint32_t offset, uint32_t *data, size_t size);
        bool flashRead(uint32_t offset, uint32_t *data, size_t size);

        uint32_t getSketchSize();
        String getSketchMD5();
        uint32_t getFreeSketchSpace();
        bool updateSketch(Stream& in, uint32_t size, bool restartOnFail = false, bool restartOnSuccess = true);

        String getResetReason();
        String getResetInfo();
        struct rst_info * getResetInfoPtr();

        bool eraseConfig();

        inline uint32_t getCycleCount();
};

uint32_t EspClass::getCycleCount()
{
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount":"=a" (ccount));
    return ccount;
}

extern EspClass ESP;

*/

  const char * sdk = ESP.getSdkVersion();
  String core = ESP.getCoreVersion();
  String full = ESP.getFullVersion();

  uint8_t bootver = ESP.getBootVersion();
  uint8_t bootmode = ESP.getBootMode();

  uint8_t cpu = ESP.getCpuFreqMHz();

  uint32_t sketchspace = ESP.getFreeSketchSpace();
  
  String resetreason = ESP.getResetReason();
  String resetinfo = ESP.getResetInfo();

  file_ID_str = "342";

  file_ID = 342;



  uint32_t ccnt2;


  color_cnt = 1; // ordinarily color_cnt comes from app. set it to 1 for our default design in initialize_leds()

  initialize_leds();
  
#ifdef serial1
  Serial.print("\n");
  Serial.print(WiFi.localIP());
#endif

  WiFiClient client;
   
  const int httpPort = 80;

  const char * host = "www.fork20.xyz";

  const char * php = "/rgb20/load_file_esp2.php";

  const char * file_ID = "?file_ID="; 

  const char * ID = "323"; 


  String url = php;
  url += file_ID;
  url += ID;

  const char * httpc = "http://";




  if (fetch_from_web == 1){
    fetch_design();
  }



   
}


void lf(void){

#ifdef serial1
  Serial.print("\n");
  yield();  // needed this for file 326 orangered (url has 324).   363 to get 365 also wdt

#endif
}











int debug2=0;  // test malloc/free which leaked on arduino IDE avr code



void print1(char * c, int i){

#ifdef serial1
      lf();
      Serial.print(c);
      Serial.println(i);
     
#endif
}




//-----------------------------------------------------------------------------


  uint32_t ccnt;

  uint32_t mil;

  int new_file = 0;





void loop() {

  ccnt = ESP.getCycleCount();  // 2^32 / 80e6 = 53.7 seconds to rollover

  mil = millis();    // 2^32 / 1000 / 60 / 60 / 24 = 49.7 days to rollover

  uptime_minutes = mil / 60000;

  if (load_one){ // app pressed next or previous

    load_one = false;

    if (files_fetched > 1){ // don't reload 1 file
      
      load_local();
      
      new_file = 1;
      
      millis2 = mil;
      
      cycles = ccnt;
    }
  
  } else if (auto_play == 1){ 
    
    if ((mil - millis2) > (cycle_delay * 1000)){

      if(led_blink_mode == 1){
        digitalWrite(one_sec_led,LOW); // on
      }

      if ((fetch_from_web==1) && (files_fetched < max_ram_files) && (heap > 3000) && (web_files_fetched < files_on_web)){
            
        fetch_design();

        new_file = 1;
  
      } else if (files_fetched > 1){ // don't reload 1 file
        
        load_local();

        new_file = 1;
      }
      
      print1(" fetched ",files_fetched);
      print1(" fetched from web ",web_files_fetched);
      print1(" current local file ",current_local_file);
      print1(" max ram files ",max_ram_files);

      millis2 = mil;
      
      cycles = ccnt;
      
      if(led_blink_mode == 1){
        digitalWrite(one_sec_led,HIGH); // off
      }
    }
  }

  if (new_file > 0){
    
     calc_soft_start();

     new_file = 0;
  }


  server_loop();


  // avr sketch says 312 hz -> 3205 usec x 80 = 256410

  //if ((ccnt - rgbcycles) > 156800){  // about 510 hz?
  if ((ccnt - rgbcycles) > 256410){  // about 312 hz

    if (led_blink_mode == 0){
  
      ledcnt++;
  
      if (ledcnt == 160){
  
        digitalWrite(one_sec_led,LOW);
        
      } else if (ledcnt == 320){
  
        ledcnt = 0;
  
        digitalWrite(one_sec_led,HIGH);
      }
    }

    rgbcycles = ccnt;
    
#ifdef scope_rgb    
    digitalWrite(scope_calc,HIGH); 
#endif
    
    update_rgb();

    // 175.6 usec for my calc + ada strip.setPixel; varies a lot depending on design
    // strip.setPixel is pretty consistent, about 57 usec which is trivial considering that each led takes 30 usec to send the data
#ifdef scope_rgb    
    digitalWrite(scope_calc,LOW);
    
    //digitalWrite(scope_rgb,HIGH); 
#endif
    // if count > (max - 123840) we need to enable interrupts and wait for cycles to rollover, which happens every 54 seconds at 80 MHz

    if (ccnt > 4294842455){ // if cycles rolls over while interrupts are disabled it might reset, so wait

      while(ccnt > 4294842455){ 

        yield();

        ccnt = ESP.getCycleCount();  // 2^32 / 80e6 = 53.7 seconds to rollover
      }

      rgbcycles = ccnt;
    }
    //} else {


      noInterrupts();
      espShow(DATA_PIN,pixels,rgbleds * 3,true);
      interrupts();
   // }
    
    //digitalWrite(scope_rgb,LOW);
  }

}
