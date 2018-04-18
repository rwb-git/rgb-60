

void update_rgb(){

  if (soft_start > 0){

    soft();
  } else {
    update_rgb2();
  }

  done_627();
}






void done_627(){

  // now copy rgb data for adafruit code

    uint8_t * p;

    p = pixels;
   
    for(int i=0; i< rgbleds; i++) {
      
        *p++ = led_block[i].g >> 8;
        *p++ = led_block[i].r >> 8;
        *p++ = led_block[i].b >> 8;
    }

}







void update_rgb2(){

print_344 = 1;

  uint8_t color_index, states, current_state, hold_frames, current_frame;


  for (uint8_t led=0;led<rgbleds;led++){
  
    color_index = led_block[led].color;

    color_scheme cs = color_schemes[color_index];

    states = color_schemes[color_index].states; //cs.states;

    current_state = led_block[led].state; 

    color_state cst = cs.color_states[current_state];

    hold_frames = cst.hold; 

    current_frame = led_block[led].frame;   // the frame within that color state, which can be in the "hold" phase or the "blend" phase 

    if (current_frame < hold_frames){ // we are in constant color phase, "hold"

      led_block[led].r = cst.r << 8;
      led_block[led].g = cst.g << 8;
      led_block[led].b = cst.b << 8;

    } else { // we are blending from the end of one state to the beginning of the next

      color_state next_cst;
  
      if (current_state == (states - 1)){ // final state
  
        next_cst = cs.color_states[0];
        
      } else { // not final state
  
        next_cst = cs.color_states[current_state + 1];
        
      }

      uint16_t rh,gh,bh;
  
      rh = next_cst.r << 8;
      gh = next_cst.g << 8;
      bh = next_cst.b << 8;
  
      led_block[led].r = desperado(cst.dr, led_block[led].r, rh);
      led_block[led].g = desperado(cst.dg, led_block[led].g, gh);
      led_block[led].b = desperado(cst.db, led_block[led].b, bh);

    }    

    led_block[led].frame ++;

    if (led_block[led].frame >= cst.frames){

      led_block[led].frame = 0;

      led_block[led].state++;

      if (led_block[led].state >= cs.states){

        led_block[led].state = 0;
      }
    } 
  }

  

}

uint16_t desperado(int16_t dr, uint16_t r, uint16_t rh){

  // this seems to eliminate the need for correction in ratio_rgb, but there is still occasional flicker

    uint16_t temp;

    temp = r + (uint16_t)dr;  // uint16 + int16

    if (dr > 0){

      // make sure temp is <= next hold and temp > old value

      if (temp < r) { // overflow

        temp = rh;
        
      } else if (temp > rh){ // went past next hold

        temp = rh;
      }
      
    } else { // decreasing or constant
      
      // make sure temp is >= next hold and temp <= old value

      if (temp > r) { // underflow

        temp = rh;
        
      } else if (temp < rh){ // went past next hold

        temp = rh;
      }
    }

    return temp;
}


