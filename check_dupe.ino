
boolean check_dupe(char * p){ // return true if it is NOT in ram



//    Serial.print("SHA1:");
//    Serial.println(sha1(p));
    //Serial.println(sha1("abc"));

    // usage as ptr
    //      0123456789012345678901234567890123456789
    // SHA1:a94a8fe5ccb19ba61c4c0873d391e987982fbbd3

    if (files_fetched < max_ram_files){



#ifdef serial1        
      char * p8 = p + strlen(p) - 9;

      for (int i=0;i<10;i++){ // prints last 10 chars? 

        Serial.print((uint8_t) *p8);
        Serial.print(" ");

        p8++;
      }

      lf();
      Serial.println(p);
      
#endif


      uint8_t temp[20];
      
      sha1(p, &temp[0]);

      int diff;

       for (int i=0;i<files_fetched;i++){

        diff = 0;
        
        for (int j=0;j<20;j++){ // hash has 20 chars
                    
          if (temp[j] != hash[i][j]){
            diff = 1;
            break;
          }
        }
        if (diff == 0){ // dupe found
          break;
        }
       }
        
       if (diff == 0){
#ifdef serial1        
        lf();
        Serial.println(" dupe file found ");
        
#endif        
        return false;
       }

       for (int i=0;i<20;i++){              // save this hash
        hash[files_fetched][i] = temp[i];
       }

      //sha1(p, &hash[files_fetched][0]);
#ifdef serial1  
      Serial.print("SHA1:");
      for(uint16_t i = 0; i < 20; i++) {
          Serial.printf("%02x", hash[files_fetched][i]);
      }
#endif

    } else {
#ifdef serial1
      lf();
      Serial.println(" max files already loaded ");
      
#endif
      return false;
    }



return true;


}

