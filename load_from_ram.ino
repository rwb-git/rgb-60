


void load_local(void){
#ifdef serial1
  Serial.print(" local ");
  Serial.print(current_local_file);
#endif
  if (files_fetched > current_local_file){

    get_it();
    
  } else {

    if (files_fetched > 0){
      current_local_file = 0;
      get_it();  
    }
  }
}


void get_it(){
#ifdef serial1
    Serial.print(" ptr ");
    Serial.print((uint32_t) ram_ptrs[current_local_file]);
#endif    

    parse_ram_file(ram_ptrs[current_local_file]);

    next_local_file();
}

void next_local_file(){
  
    current_local_file++;
  
    if (current_local_file >= files_fetched){
  
      current_local_file = 0;
    } 
}


void previous_local_file(){

    // it will call next_local_file so we have to decrement twice

    for (int i=0;i<2;i++){
      if (current_local_file > 0){
  
        current_local_file --;
      } else {
         
         current_local_file = files_fetched - 1;
      }
    }
}

