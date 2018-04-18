
void fetch_design(void){


  filecnt++;

  String httpurl = "http://www.fork20.xyz/rgb20/load_file_esp2.php?file_ID=";

  if (file_ID < 10){

    char ff1[1];
    sprintf(ff1,"%d",file_ID);
    httpurl += ff1;  
  }else if (file_ID < 100){
    
    char ff2[2];
    sprintf(ff2,"%d",file_ID);
    httpurl += ff2;
  }else if (file_ID < 1000){
    
    char ff3[3];
    sprintf(ff3,"%d",file_ID);
    httpurl += ff3;
  }else if (file_ID < 10000){
    
    char ff4[4];
    sprintf(ff4,"%d",file_ID);
    httpurl += ff4;
  }
  
  
  
  httpurl += "&dir=1";

  yield();

  http.begin(httpurl);
  yield();
  ESP.wdtFeed();
  
  int httpcode = http.GET();
  yield();
  ESP.wdtFeed();
  
  if (httpcode > 0){
    yield();
    ESP.wdtFeed();
    
    String pay = http.getString();
    ESP.wdtFeed();
    
    yield();
    
    parse_web_file(pay);
    
    yield();
    
    ESP.wdtFeed();
  } else {


    ESP.wdtFeed();
#ifdef serial1    
    Serial.print(" nope ");
    Serial.print(httpcode);
    
    Serial.println(http.errorToString(httpcode)); //.c_str());
    
    
#endif    
  }

  ESP.wdtFeed();

  http.end();

  
}


