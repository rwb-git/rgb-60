
void calc_soft_start(){

  millis2 = millis(); // reset auto play on new file from any source

  soft_start = 300; // remember this is decremented to zero for each file and has to be reset here
  //
  // 600 appeared to work correctly

  for (int i=0;i<rgbleds;i++){

    //int16_t delta = (b2 - b1) * 256 / frames;  <- this works; see math_8266.ino

  
    // all these are 16 bit values with the msb being the "whole" part and the lsb is the "fractional" part

    soft_delta[i].r = (soft_goal[i].r * 256 - led_block[i].r) / soft_start; 

    soft_delta[i].g = (soft_goal[i].g * 256 - led_block[i].g) / soft_start; 

    soft_delta[i].b = (soft_goal[i].b * 256 - led_block[i].b) / soft_start; 


  }

}







void soft(){

  /*
   *      do I need to clear the low bytes on the last step through here? this phase ends at the initial index point sent by the app and can be in a hold phase
   *      or a blend phase. In the hold phase everything will be fine. In the middle of a blend phase that is going up to 255 is it possible to overflow and fall
   *      back to 0 (black), or while going down to zero can we underflow and go to 255 = full on which will be an annoying blink. I've never seen either happen, so
   *      maybe the math simply prevents it.
   *      
   *      Also there was a flicker issue with soft start that I vaguely recall I solved by making soft start not quite go to the exact goal but fall a bit short,
   *      and that might be why it never (?) flickers at the end of soft start. In other words this was an issue and I think I fixed it in the app rather than here.
   * 
   */

  soft_start--;

  for (uint8_t i=0;i<rgbleds;i++){

      led_block[i].r += soft_delta[i].r;
      led_block[i].g += soft_delta[i].g;
      led_block[i].b += soft_delta[i].b;    

  
  }

  if (soft_start == 0){
  
    for (uint8_t i=0;i<rgbleds;i++){
  
        led_block[i].r = soft_goal[i].r << 8; //&= 0xFF00;
        led_block[i].g = soft_goal[i].g << 8; // &= 0xFF00;
        led_block[i].b = soft_goal[i].b << 8; // &= 0xFF00;
    }
    
  }
}


