




void parse_app_msg(String pay){

  /*
   *    every message consists of: type, num_tokens. 
   *    
   *      if num_tokens > 0 then that many should follow.
   * 
   *    1 0 = design mode: user is editing design in RGB60 app so just display it and don't save in ram
   *  
   *    2 0 = auto-web mode: fetch new web file every N seconds and save in ram, up to M files or ram full. after that it auto-plays files from ram.
   *    
   *    3 0 = app file mode: user is sending files from rgb60 local storage so save in ram
   *    
   *    4 N = seconds to wait before fetching next web file or cycling to next ram file
   *    
   *    5 M = number of files to fetch from web; if fewer files are available use the lower number
   *    
   *    6 B = soft blend frames
   *    
   *    7 0 = query esp status
   *    
   *    8 0 = change builtin led blink mode
   *    
   */

  char * paych = strdup(&pay[0]); // strdup copies pay, and needs to be freed when done
   
  char * save_paych = paych; // save so i can free, which requires original pointer value, and strsep alters paych

  char * token;

  uint16_t data;

  if (strlen(paych) > 0){
#ifdef serial1
    lf();
    Serial.print(" raw msg ");
    Serial.println(paych);
    
#endif

    token = strsep(&paych," ");

    if (strlen(token) > 0){
   
      app_msg = get_uint16(token);
#ifdef serial1
      Serial.print(" msg ");
      Serial.print(app_msg);
#endif

      int i;

      switch(app_msg){
        case 2:
          
          if (auto_play == 1){ // on
            auto_play = 0;     // off
          } else {
            auto_play = 1;
          }
          break;

        case 8:

          led_blink_mode++;
          
          if (led_blink_mode > 2){
          
            led_blink_mode = 0;
          }
          digitalWrite(one_sec_led,HIGH);  // off
          break;

        case 10: // clear all files from ram

          for (i = (files_fetched - 1);i>=0;i--){

            free(ram_ptrs[i]);

          }

          current_local_file = 0;

          files_fetched = 0;

          web_files_fetched = 0;


          break;

        case 11:

          if (fetch_from_web ==1 ){

            fetch_from_web = 0;
          } else {
            fetch_from_web = 1;
          }
    
          break;

          
        case 13:

          if (auto_save ==1 ){

            auto_save = 0;
          } else {
            auto_save = 1;
          }
    
          break;

        case 16:

          load_one = true;
          millis2 = 0;
//          next_local_file();
          break;
          
        case 17:

          load_one = true;
          millis2 = 0;
          previous_local_file();
          break;

        case 18:

          save_settings();
          //save_settings_in_spiffs();
          break;
         

      }
      
    }

  }

  
  if (strlen(paych) > 0){
#ifdef serial1
    lf();
    Serial.print(" raw msg ");
    Serial.println(paych);
    
#endif
    
    token = strsep(&paych," ");

    if (strlen(token) > 0){
   
      data = get_uint16(token);

#ifdef serial1
      Serial.print(" data ");
      Serial.print(data);
#endif
      
      switch(app_msg){
      
        case 9: // cycle time, seconds

          if (data >= 5){

            cycle_delay = data;
          }

          break;  
      
        case 15: // files cnt

          if (data >= 1){

            if (data <= actual_max_ram_files){

              max_ram_files = data;
            
            } else {
              max_ram_files = actual_max_ram_files;
            }
          }

          break;  

       
      }
      
    }

  }

  free(save_paych);
  

}



 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
  


void parse_local(String pay){  // file from android app

  char * paych = strdup(&pay[0]); // strdup copies pay, and needs to be freed when done
  char * save = paych;

  if (auto_save == 1){ // disable auto save when editing design

    if (check_dupe(paych)){
  
      ram_ptrs[files_fetched] = strdup(paych); // make another copy to save permanently
    
      ram_file_type[files_fetched] = 1;
#ifdef serial1    
      Serial.print(" ptr ");
      Serial.print((uint32_t) ram_ptrs[files_fetched]);
#endif    
      files_fetched++;
  
      print1(" fetched ",files_fetched);
  
    }
  }
#ifdef serial1
  lf();
  Serial.print(" paych len ");
  Serial.println(strlen(paych));
  
#endif
  parse(paych,1);

  free(save);
  
}




 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
  




void parse_web_file(String pay){

  char * paych = strdup(&pay[0]); // strdup copies pay, and needs to be freed when done
  
  char * save = paych;
  
  paych = strip_web_tokens(paych);

  if (check_dupe(paych)){

    ram_ptrs[files_fetched] = strdup(paych); // make another copy to save permanently
  
    ram_file_type[files_fetched] = 0;
#ifdef serial1  
    Serial.print(" ptr ");
    Serial.print((uint32_t) ram_ptrs[files_fetched]);
#endif  
    files_fetched++;

    web_files_fetched++;

  }

  parse(paych,1);  // mode is 1 since we stripped extra web stuff

  free(save);
  
}




 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
  



void parse_ram_file(char * pay){ 

  char * paych = strdup(pay); // strdup copies pay, and needs to be freed when done

  char * save = paych;

  parse(paych,1);

  free(save);
  
}






 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
 //-----------------------------------------------------------------------------
  



void parse(char * paych, uint8_t mode){ // mode = 0 web file format     mode = 1 app file format


  for (int i=0;i<color_scheme_cnt;i++){
    free(color_schemes[i].color_states);  
  }

  
  free(color_schemes);
  
  char * token;

  int cnt = 0;

  if (mode == 0){
    
      file_ID_str = strsep(&paych," ");
      
      file_ID = get_uint16(file_ID_str);
    
  }
  
  itest = get_uint16(strsep(&paych," "));

  web_led_cnt = get_uint16(strsep(&paych," "));

  paych = read_color_string(paych);
  
  get_uint16(strsep(&paych," "));  // not used
  
  
  web_color_cnt = get_uint16(strsep(&paych," "));

  color_scheme_cnt = web_color_cnt;

  color_schemes = (color_scheme *) malloc(sizeof(struct color_scheme) * web_color_cnt); //color_scheme_cnt);

  uint16_t bytes;

  for (int i=0; i<web_color_cnt;i++){
  
    bytes = get_uint16(strsep(&paych," "));  // states * 5
  
    color_schemes[i].states = bytes / 5;
    
    color_schemes[i].color_states = (color_state *) malloc(sizeof(struct color_state) * color_schemes[i].states);  

    color_schemes[i].total_frames = 0;
    
    uint8_t byte1;

    uint8_t hold;

    int cnt = 0;

    uint8_t state = 0;

    for (int j=0; j< bytes;j++){

      // bytes should be a multiple of 5: hold   blend   r g b   
      //
      // app sends hold+blend and hold to avr


      byte1 = get_uint8(strsep(&paych," "));


      switch(cnt){

        case 0:
          color_schemes[i].color_states[state].hold = byte1;
 
          hold = byte1;
          break;

        case 1:
          color_schemes[i].color_states[state].frames = byte1 + hold;


          color_schemes[i].total_frames += byte1 + hold;

          break;
                    
        case 2:
          color_schemes[i].color_states[state].r = byte1;
          break;

        case 3:
          color_schemes[i].color_states[state].g = byte1;
          break;
                    
        case 4:
          color_schemes[i].color_states[state].b = byte1;


          state++;
          cnt = -1;
          
          break;
      }

      cnt++;
      
    }

    calc_deltas(i);
  }


  // the next 5 are not used



  uint16_t byte1 = get_uint16(strsep(&paych," "));
  
  byte1 = get_uint16(strsep(&paych," "));

  byte1 = get_uint16(strsep(&paych," "));

  byte1 = get_uint16(strsep(&paych," "));

  byte1 = get_uint16(strsep(&paych," "));
  
  uint8_t num_lc3 = get_uint16(strsep(&paych," ")); 

  for (int i=0; i<num_lc3;i++){

    paych = read_lc3(paych);

    
  }


  if (mode == 0){
        
      uint16_t total_files = get_uint16(strsep(&paych," ")); 
        
      char * path = strsep(&paych," ");
      
      char * filename = strsep(&paych," ");
    
      char * owner_ID = strsep(&paych," ");
      
      char * owner = strsep(&paych," ");
    
      uint16_t anons_files = get_uint16(strsep(&paych," ")); 
    
  } // mode = 0
  
  debug5=99900;
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


void calc_deltas(int i){

  int states = color_schemes[i].states;
  
  uint16_t frames,r1,r2,g1,g2,b1,b2;

  r1 = color_schemes[i].color_states[0].r;
  g1 = color_schemes[i].color_states[0].g;
  b1 = color_schemes[i].color_states[0].b;

  for (int j=0;j<states;j++){

    yield();  // needed this for file 326 orangered (url has 324).   363 to get 365 also wdt

    if (j == (states - 1)){ // final state blends to first state
   
      r2 = color_schemes[i].color_states[0].r;
      g2 = color_schemes[i].color_states[0].g;
      b2 = color_schemes[i].color_states[0].b;
   
    } else {
      
      r2 = color_schemes[i].color_states[j+1].r;
      g2 = color_schemes[i].color_states[j+1].g;
      b2 = color_schemes[i].color_states[j+1].b;
   
    }
    
    frames = color_schemes[i].color_states[j].frames - color_schemes[i].color_states[j].hold;

    if (frames > 0){

      color_schemes[i].color_states[j].dr = ((r2 -r1) * 256) / frames;  
      color_schemes[i].color_states[j].dg = ((g2 -g1) * 256) / frames;
      color_schemes[i].color_states[j].db = ((b2 -b1) * 256) / frames;
   
    } else {
    
      color_schemes[i].color_states[j].dr = 0;
      color_schemes[i].color_states[j].dg = 0;
      color_schemes[i].color_states[j].db = 0;
  
    }
    
    r1 = r2;
    g1 = g2;
    b1 = b2;
   
  }

  
}



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------





char * read_lc3(char * paych){

  char * token;

  float ind1,ind2,ss1,ss2;
  
  for (int i=0;i<4; i++){

    token = strsep(&paych," ");                 // if paych is null this returns null. if it finds a " ", it changes it to \0 and changes paych to point to the next byte.
                                                  // the return value is a pointer to the original start of paych, meaning paych's old value
                                                  // if no " " is found paych is set to null 

    float fl = get_float(token);

    switch(i){
      case 0:
        ind1 = fl;
        break;
      case 1:
        ind2 = fl;
        break;
      case 2:
        ss1 = fl;
        break;
      case 3:
        ss2 = fl;

        handle_lc3(ind1, ind2, ss1, ss2);
        break;        
    }
  }

 

  return paych;
  
    
}



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------



void handle_lc3(float ind1, float ind2, float s1, float s2){


      int led1,led2;


      int led_cnt1 = rgbleds - 1;

      if (ind1 < 0.01) {  // limits were 0.001 and 0.999 from app. adjusted to try to reduce flicker after soft

        led1 = 0;
      } else { // this one starts in the middle of the led string

        led1 = 1 + (int) (ind1 * led_cnt1);
      }

      if (ind2 > 0.99) {

        led2 = led_cnt1;
      } else { // this one ends in the middle somewhere

        led2 = (int) (ind2 * led_cnt1);
      }

    uint8_t r2,g2,b2,r1,g1,b1,denom;

    r1=0;
    r2=0;
    g1=0;
    g2=0;
    b1=0;
    b2=0;
    
    int ind;

    if (led2 == led1){
#ifdef serial1
      Serial.print(" error in handle lc3 ");
#endif
      
      return; // huge error
    }

    float m = (s2-s1)/(led2-led1);

    color_state cs;

    float b = s2 - m *  led2;
    float s,ratio;

    for (int i=led1;i<=led2;i++){  // i is led number

      int color_index = led_block[i].color;

      int maxf = color_schemes[color_index].total_frames;
      
      s = m * (float) i + b;

      ind = (int) (s * (maxf - 1)); // ind is the frame in the entire color scheme

      if (ind < 0){
        ind = 0;
      } else if (ind > (maxf-1)){

        ind = (int) (maxf - 1);   // ind is the overall frame number in the color scheme. now we find which state it is in
      }

      boolean not_found = true;

      int frm3 = 0;

      int frm4 = 0;

      int state = 0;

      while (not_found){

        yield();  // needed this for file 326 orangered (url has 324).   363 to get 365 also wdt
        
        frm3 += color_schemes[color_index].color_states[state].frames; // frm3 is the final frame for this state as counted from zero at start of first frame

        if (frm3 >= ind){
          
          led_block[i].state = state;

          led_block[i].frame = ind - frm4; // fhm4 is the first frame of this state, as counted from 0 at start of first frame

          not_found = false;

          // if we are in a hold phase then the goal rgb is simple
          // but if we are in a blend phase we have to ratio the rgb values

          cs = color_schemes[color_index].color_states[state];

          if (led_block[i].frame < cs.hold){

            soft_goal[i].r = cs.r;
            soft_goal[i].g = cs.g;
            soft_goal[i].b = cs.b;

          } else { // blending

            // r1 = hold val    r2 = hold from next state  rgoal = (r2-r1) * this frame / blend frames

            if (state >= (color_schemes[color_index].states - 1)){
        
              r2 = color_schemes[color_index].color_states[0].r;
              g2 = color_schemes[color_index].color_states[0].g;
              b2 = color_schemes[color_index].color_states[0].b;
              
            } else {
              
              r2 = color_schemes[color_index].color_states[state + 1].r;
              g2 = color_schemes[color_index].color_states[state + 1].g;
              b2 = color_schemes[color_index].color_states[state + 1].b;              
            }           
          
            r1 = cs.r;
            g1 = cs.g;
            b1 = cs.b;
            
            denom = cs.frames - cs.hold; // total blend frames
  
            if (denom != 0){
  
              ratio = (float) (led_block[i].frame - cs.hold) / (float) denom;  // frame >= hold
              
              soft_goal[i].r = ratio_rgb(ratio,r1,r2); // goal is uint8_t
              soft_goal[i].g = ratio_rgb(ratio,g1,g2); 
              soft_goal[i].b = ratio_rgb(ratio,b1,b2); 
                
            } else { // error has occurred
#ifdef serial1
              lf();
              Serial.println("error in parsegoalblend ");
              
#endif              
              soft_goal[i].r = r2;
              soft_goal[i].g = g2;
              soft_goal[i].b = b2;
              
            }
          }
          
        }

        frm4 = frm3;

        state++;
      
      }
      
    }
  
}




//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------




  uint8_t ratio_rgb(float frame_ratio,uint8_t r1,uint8_t r2){

    // if the blend is increasing, avr soft start roundoff can make it go through 255 causing flicker. same problem for decreasing.
    //
    // THE PROBLEM is not with this actual value, but with avr error using the ideal delta when the soft start ends on the wrong frame within
    // a blend segment, and by the end of that blend it has over or underflowed. normal blending always works, it seems.
    //
    // maybe i should cap all colors between 1..254 or 2..253, but then at night "black" would have a pretty strong glow.
    //
    // REMEMBER that it's not the end of soft that blows up, it's the end of the blend phase that soft leads into, which is why we
    // correct all values, not just those close to 0 or 255

    int correction = 0;

    float val; // = (((float)(r2-r1) * frame_ratio) + r1);

    if (r2 > r1){ // this blend is increasing, so avoid overflow in avr

      val = ((float)(r2-r1) * frame_ratio) + r1;
      
      if (val > 6) {
        correction = -5;
      } else {

        correction = val;
      }

    } else if (r2 < r1){ //this blend is decreasing, so avoid going through 0 in avr roundoff

      val = r1 - (float)(r1-r2) * frame_ratio;
      
      if (val < 249) {
        correction = 6;
      } else {

        correction = 255 - val;
      }
    } else { // r1 = r2

      correction = 0;
      val = r1;
    }
correction = 0;
    return (uint8_t)(val + correction);
  }




//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------



char * read_color_string(char * paych){

  char * token;
  
  for (int i=0;i<web_led_cnt; i++){

    token = strsep(&paych," ");                 // if paych is null this returns null. if it finds a " ", it changes it to \0 and changes paych to point to the next byte.
                                                  // the return value is a pointer to the original start of paych, meaning paych's old value
                                                  // if no " " is found paych is set to null 
    led_block[i].color = get_uint8(token);    
    
  }
  
  return paych;
  
    
}




//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


float get_float(char * token){

  float ret = atof(token); 

  return ret;
  
}



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


uint8_t get_uint8(char * token){

  uint8_t ret = atoi(token); 

  return ret;
  
}



//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


uint16_t get_uint16(char * token){

  uint16_t ret = atoi(token); 

  return ret;
  
}

