

void initialize_leds(){

  // put something there to show that they are working while we wait for app to send a design
  //
  // it seems that the data order is green red blue...

  uint16_t frame,state;

  uint8_t *Z;

  frame = 0;
  state = 0;
  
  int j = 0;

  uint8_t k;

  for (uint8_t i=0;i< rgbleds;i++){

//    pixels[j++] = 0; // green
//    pixels[j++] = 0; // red
//    pixels[j++] = 0; // blue

    led_block[i].color = 0;
    led_block[i].frame = frame & 0x00FF;
    led_block[i].state = state & 0x00FF;

    frame += 11;   

    if (frame >=254){
      frame = 0;
      state++;
    }

  }




  color_scheme_cnt = 1;

 


  color_schemes = (color_scheme *) malloc(sizeof(struct color_scheme) * color_scheme_cnt);

  color_schemes[0].states = 3;

  color_schemes[0].color_states = (color_state *) malloc(sizeof(struct color_state) * color_schemes[0].states);

  
  // state 0

  color_schemes[0].color_states[0].g = 255;
  color_schemes[0].color_states[0].r = 0;
  color_schemes[0].color_states[0].b = 0;

  color_schemes[0].color_states[0].dg = -259;  // this is (next green - this green) * 256 / blend frames.      blend frames = frames - hold, so it's (0 - 255) * 256 /(254 - 2) = -259
  color_schemes[0].color_states[0].dr = 259;    
  color_schemes[0].color_states[0].db = 0;

  color_schemes[0].color_states[0].frames = 254;
  color_schemes[0].color_states[0].hold = 2;


  
  // state 1


  color_schemes[0].color_states[1].g = 0;
  color_schemes[0].color_states[1].r = 255;
  color_schemes[0].color_states[1].b = 0;

  color_schemes[0].color_states[1].dg = 0;
  color_schemes[0].color_states[1].dr = -259;
  color_schemes[0].color_states[1].db = 259;

  color_schemes[0].color_states[1].frames = 254;
  color_schemes[0].color_states[1].hold = 2;

  
  // state 2

  color_schemes[0].color_states[2].g = 0;
  color_schemes[0].color_states[2].r = 0;
  color_schemes[0].color_states[2].b = 255;

  color_schemes[0].color_states[2].dg = 259;
  color_schemes[0].color_states[2].dr = 0;
  color_schemes[0].color_states[2].db = -259;

  color_schemes[0].color_states[2].frames = 254;
  color_schemes[0].color_states[2].hold = 2;



}




