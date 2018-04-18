
uint32_t web_cnt = 0;


WiFiClient client; // = server.available();

void server_loop()
{
  client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
    web_cnt++;

#ifdef serial1    
    Serial.print("\n[Client connected] ");
    Serial.print(web_cnt);
#endif
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        //Serial.print(line);

        app_msg = 0; // will be set in parse_app_msg

        if (line.length() > 100){

          parse_local(line);
      
          calc_soft_start();

          send_status();

        } else {

          parse_app_msg(line); // handle the msg there. send reply to app here
          
          switch (app_msg){
  
            case 2:   // toggle auto fetch
            case 7:   // request status
            case 9:   // cycle time, seconds
            case 10:  // clear all files from ram 
            case 11:  // toggle web fetch
            case 13:  // toggle auto save
            case 14:  // app is scanning IPs
            case 15:  // set files cnt
            case 16:  // next file
            case 17:  // previous file
            case 18:  // save settings file in spiffs
  
              send_status();

              //read_dir();
  
              break;

            case 8: // led blink mode

              char buf[10];
              sprintf(buf,"%d %d \r\n",app_msg,led_blink_mode); // needs final " " or app says not an integer
            
              client.print(buf);
              break;

/*
            case 28: // led blink mode

              char buf[10];
              sprintf(buf,"%d %d \r\n",app_msg,led_blink_mode); // needs final " " or app says not an integer
            
              client.print(buf);
              break;

  */            



            case 12: { // sdk info

              char bigbuf[400];  // hope this is enough

              String full = ESP.getFullVersion();
              
              const char * sdk = ESP.getSdkVersion();
              String core = ESP.getCoreVersion();
              
              
              uint8_t bootver = ESP.getBootVersion();
              uint8_t bootmode = ESP.getBootMode();
              
              uint8_t cpu = ESP.getCpuFreqMHz();
              
              uint32_t sketchspace = ESP.getFreeSketchSpace();
              
              //String resetreason = ESP.getResetReason();
              //String resetinfo = ESP.getResetInfo();


              if (sizeof(full) < 370){
              
                sprintf(bigbuf,"%d %s %s %s %d %d %d %d \r\n",app_msg,full.c_str(),sdk,core.c_str(),cpu,sketchspace,bootver,bootmode); 
                
              } else {

                sprintf(bigbuf,"%d string too long \r\n",app_msg); 
              }
            
              client.print(bigbuf);
              break;
          } // weird compiler error without these braces around case 12
  /*
            case 15: // set files cnt

              char buf2[10];
              sprintf(buf2,"%d \r\n",123); // needs final " " or app says not an integer
            
              client.print(buf2);
              break;  
              */        
          } // switch()
    
        } // else = short messages (line length < 100), meaning not a design file

          break;
       // }
      } // if client.available
    }
    delay(1); // give the web browser time to receive the data

    // close the connection:
    client.stop();

#ifdef serial1    
    Serial.println("[Client disonnected]");
#endif
    
  }
}

void send_status(){

    char buf[300];

    String resetreason = ESP.getResetReason().c_str();

    char * cc = &resetreason[0];

    char * loc;
    while (strchr(cc,' ')){

      loc = strchr(cc,' ');

      *loc = '_';

    }

    // needs final " " or app says not an integer (if final entity is a number)
    sprintf(buf,"%d %d %d %d %d %d %d %d %d %s \r\n",app_msg,heap,files_fetched,cycle_delay,auto_play,\  
      uptime_minutes,fetch_from_web,auto_save,max_ram_files,resetreason.c_str()); 
    

    client.print(buf);
  
}

