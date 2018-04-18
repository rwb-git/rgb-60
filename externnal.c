
// all of the following is derived from the code in adafruit esp8266.c

/*
 *    ICACHE_RAM_ATTR is not clear to me. web says it means this code will be in instruction ram iram(0), whatever that means. I would guess
 *    that adafruit programmer did this because the code is timing critical. he/she also noted that espShow() has to be extern for this to work.
 *    
 *    web also seems to say that extern is redundant because it is default behavior when a function is not static. and it means that the function
 *    is defined in another file, which possibly made sense to adafruit since they actually have a separate esp8266.c file, while my code
 *    here is all in .ino files which are treated as one file by the compiler.
 *    
 *    in other words it works so i'm leaving it as is.
 */


#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#ifdef ESP8266
#include <eagle_soc.h>
#endif


static uint32_t _getCycleCount(void) __attribute__((always_inline));
static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}



#ifdef ESP8266
void ICACHE_RAM_ATTR espShow(
 uint8_t pin, uint8_t *pixels, uint32_t numBytes, boolean is800KHz) {
#else
void espShow(
 uint8_t pin, uint8_t *pixels, uint32_t numBytes, boolean is800KHz) {
#endif

// for 60 leds this takes 30 usec * 60 = 1.8 msecs so watchdog is ok

// comment these three out to use the next section where adafruit calculates based on F_CPU
#define CYCLES_800_T0H  27
#define CYCLES_800_T1H  59
#define CYCLES_800      86


//#define CYCLES_800_T0H  (F_CPU / 2500000) // 0.4us            32 for 80 MHZ
//#define CYCLES_800_T1H  (F_CPU / 1250000) // 0.8us            64 for 80 MHZ
//#define CYCLES_800      (F_CPU /  800000) // 1.25us per bit   100 cycles for 80 MHZ


// my tests with 60 leds, nodeMCU/esp8266
//
// looks ok 26 58 84
// looks ok 24 56 80
// looks ok 20 52 72
// looks ok 18 50 68
// looks ok 14 46 60   and ok at 13 45 58, 12 44 56, 11 43 54 = some flicker at 11 43 54; ok, some designs flicker at 14 46 60
// fails 10 40 50 = black, fails at 10 42 52
// fails 48 80 128 = lighter colors than it should be
// fails 46 78 124 = lighter colors than it should be
// fails 44 76 120 = lighter colors than it should be
// looks ok 40 72 112
//
// so, based on this, avg values would be (40 + 14) / 2 = 27, (72 + 46) / 2 = 59, (112 + 60) / 2 = 86

// short pulse 
//                14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48
//
// spec for "0"                     XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// spec for '1"                                 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// adafruit value                                                       32
// my success     .....XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// my average                                            27

// long pulse
//                43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80
// spec for "0"                                          XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// spec for "1"                              XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// adafruit value                                                                64
// my success     .........XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// my average                                                     59

  // so zero code is 0.25 to 0.55 hi then 0.7 to 1.0 hi = 20 to 44 hi then 56 to 80 lo; avg 32 hi  68 lo
  //
  //   and 1 code is 0.65 to 0.95 hi and 0.3 to 0.6 low = 52 to 76 hi then 24 to 48 lo; avg 64 hi  36 lo

  // cycles count rolls over every 54 seconds on 80 MHz which will not be good and might trigger a reset
  //
  // cycles = leds x 3 x 8 x one bit
  //          60 x 3 x 8 x 86 = 123840
  //
  //  verify: that should be about 30 usec x 60 = 1.8 msec;   123840 / 80e6 = 1.548 msec
  //
  // so, max cycle count = 2^32 - 1 = 4 294 967 295
  //
  // if count > (max - 123840) we need to enable interrupts and wait for cycles to rollover

  uint8_t *p, *end, pix, mask;
  uint32_t t, time0, time1, period, c, startTime, pinMask;
  pinMask   = _BV(pin);
  p         =  pixels;
  end       =  p + numBytes;
  pix       = *p++;
  mask      = 0x80;
  startTime = 0;

  time0  = CYCLES_800_T0H;  // try other values since this flickered some
                            //
                            // spec is 0.2 to 0.5 narow pulse which is 16 to 40; avg 28
                            //
                            // wide pulse is 0.75 to 1.05 = 60 to 84; avg 72
                            //
                            // wait. worldsemi spec says 0 code is 0.4 hi 0.85 lo and 1 code is 0.8 hi and 0.45 lo, all +/- 150 nsec
                            //
                            // so zero code is 0.25 to 0.55 hi then 0.7 to 1.0 hi = 20 to 44 hi then 56 to 80 lo
                            //
                            //   and 1 code is 0.65 to 0.95 hi and 0.3 to 0.6 low = 52 to 76 hi then 24 to 48 lo
  time1  = CYCLES_800_T1H;
  period = CYCLES_800;

  for(t = time0;; t = time0) { // endless loop that sets t = time0 each time
    
    if(pix & mask) t = time1;                             // Bit high duration
    
    while(((c = _getCycleCount()) - startTime) < period); // Wait for bit start

    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);       // Set high

    startTime = c;                                        // Save start time

    while(((c = _getCycleCount()) - startTime) < t);      // Wait high duration
    
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);       // Set high

    if(!(mask >>= 1)) {                                   // Next bit/byte. >>= means mask = mask >> 1; when it shifts out of lsb this block executes
      if(p >= end) break;
      pix  = *p++;
      mask = 0x80;
    }
  }
  while((_getCycleCount() - startTime) < period); // Wait for last bit
}
#endif // ESP8266




