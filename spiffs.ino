/*
void read_dir(){

  Serial.println(" ------------------- read dir  ------------------------");

  if(SPIFFS.begin()){
    Serial.println(" spiffs begin ok ");
  } else {
    Serial.println(" spiffs begin fail ");

    Serial.print("\n");
    return;
  }
  
  Dir dir = SPIFFS.openDir("/");

  while (dir.next()){

    Serial.print("\n");
    Serial.print("\n");

    Serial.print(dir.fileName());
    Serial.print(" size ");
    File f = dir.openFile("r");

    if (!f){
      Serial.println(" fail to open ");
    } else {
      Serial.println(f.size());
  
      read_file(f);
      }
  }
}



void read_file(File f){

      if (f){
        
        Serial.println("file open for read ok");

        String s = f.readStringUntil('\n');
        Serial.println(s);
        
        f.close();
      }  
   
}
*/



void check_spiffs_format(){
     
  
  if(SPIFFS.begin()){
#ifdef serial1
    Serial.println(" spiffs begin ok ");
#endif    
  } else {
    #ifdef serial1
    Serial.println(" spiffs begin fail ");
#endif
    

    return;
  }

  if (!SPIFFS.exists("/formatComplete.txt")){
#ifdef serial1
    Serial.println("formatting spiffs...");
#endif    
    
    SPIFFS.format();
#ifdef serial1
    Serial.println("formatting complete");
#endif    

    File f = SPIFFS.open("/formatComplete.txt","w");

    FILE * fp = (FILE *) &f;

    if (!f){
#ifdef serial1
      Serial.print("file open fail");
#endif      
    } else {
      
#ifdef serial1
      Serial.print("file open ok");
#endif
      
      f.print("some text\n");
      
      f.close();

      f = SPIFFS.open("/formatComplete.txt","r");

      fp = (FILE *) &f;

      if (f){
#ifdef serial1
        Serial.println("file open for read ok");
#endif        
        
        char buff[300];
        fgets(buff,300,(FILE*)fp);
#ifdef serial1
        Serial.print(buff);
#endif
        f.close();
      } else {
#ifdef serial1        
        Serial.print("file open for read fail");
#endif        
      }
    }
    lf();
  } else {
#ifdef serial1    
    Serial.print("spiffs already formatted");
#endif    
  }

}


void save_settings(){

  // auto_play = keep playing one design or go to next; does this make sense? if it's off and esp resets, what gets played, unless i
  // save designs in spiffs, and then i need a setting = starting file

  // auto save to ram = does this make sense? i only turn this off to edit designs

  // these all make sense
  // cycle time per file
  // max ram files
  // led blink mode
  // auto web fetch
  // auto play spiffs
  
  // all in one file or separate files? if flash is written in blocks, it might be better to write the whole file 
  // also, several can be combined into one byte
  
  if(SPIFFS.begin()){
#ifdef serial1    
    Serial.println(" spiffs begin ok ");
#endif    
  } else {
#ifdef serial1    
    Serial.println(" spiffs begin fail ");
#endif
    

    return;
  }

    File f = SPIFFS.open("/settings_dir/settings","w");

    FILE * fp = (FILE *) &f;

    if (!f){
#ifdef serial1      
      Serial.print("file open fail");
#endif      
    } else {
#ifdef serial1      
      Serial.print("file open ok");
#endif      
 
      char buf[300];
      sprintf(buf,"%d %d %d %d \n",max_ram_files, cycle_delay, fetch_from_web, led_blink_mode); // needs final " " or app says not an integer
      
      f.print(buf);
      
      f.close();
    }

    read_settings();
}


void read_settings(){


      File f = SPIFFS.open("/settings_dir/settings","r");  // r w a r+ w+ a+ 

      FILE * fp = (FILE *) &f;

      if (f){
        
        String s = f.readStringUntil('\n');

#ifdef serial1        
        Serial.println("file open for read ok");

        Serial.println(s);
#endif
        parse_settings(s);
        f.close();
      }  
  
  
}

void parse_settings(String s){
#ifdef serial1
  Serial.print(" parse ");
  Serial.println(s);
#endif
  int cnt = 0;
  
  char * paych = strdup(&s[0]); // strdup copies pay, and needs to be freed when done
   
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

    while (strlen(token) > 0){
#ifdef serial1
      Serial.println(token);

      Serial.print(" cnt ");
      Serial.print(cnt);
#endif
      uint16_t u16;
      uint8_t u8;
      
      switch(cnt){
        case 0:
        
          u16 = get_uint16(token);
#ifdef serial1
          Serial.print(" uint16 = ");
          Serial.println(u16);
          max_ram_files = u16;
#endif
          break;

        case 1:
        
          u16 = get_uint16(token);
#ifdef serial1
          Serial.print(" uint16 = ");
          Serial.println(u16);
#endif
          cycle_delay = u16;
          
          break;

        case 2:
        
          u8 = get_uint8(token);
#ifdef serial1
          Serial.print(" uint8 = ");
          Serial.println(u8);
#endif
          fetch_from_web = u8;
          
          break;

        case 3:
        
          u16 = get_uint16(token);
#ifdef serial1
          Serial.print(" uint16 = ");
          Serial.println(u16);
#endif
          led_blink_mode = (int) u16;
          
          break;
      }
      cnt++;
      
      token = strsep(&paych," ");

      if ((token) == NULL){ //"\0"){
#ifdef serial1        
        Serial.println(" null found ");
#endif
        break;
      }
      
    }
  }
#ifdef serial1
  Serial.println("freeing paych");
#endif  
  free(save_paych);
}


void try_to_read_settings_file(){
  
  if(SPIFFS.begin()){
#ifdef serial1    
    Serial.println(" spiffs begin ok ");
#endif    
  } else {
#ifdef serial1    
    Serial.println(" spiffs begin fail ");
#endif
   

    return;
  }

  read_settings();
  
}

