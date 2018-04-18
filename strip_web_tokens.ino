/*
 * 
 *  4-4-2018
 *  
 *    web file has extra tokens on both ends that are not in files sent from app, so strip those and save the common part to make dupe checking easier
 */


 char * strip_web_tokens(char * paych){

  /*
   *  file_ID is first extra token
   * 
   */


  int mode = 0;
   
  if (mode == 0){
    
      file_ID_str = strsep(&paych," ");
      
      file_ID = get_uint16(file_ID_str);
       
  }
  

/*
 *  all the common stuff will be copied to a new string and the original will be freed
 * 
 */

//  char * newp = paych;

/*
 * 
 *  why not just return the pointer at this point, and write a NULL at the end?
 *  
 *    that requires a strsep that does not alter the data, or copies to the new ptr before.
 *    
 *    strsep finds the next " ", returns the old paych value, puts a NULL where the " " was, and changes paych to point to the byte after that new NULL
 *    
 *    memccpy(*dest, *src, int " ", maxbytes) returns ptr to next byte in dest
 *    
 *    so, strdup old string into new string which will allocate more than we need.
 *    
 *    copy what we want into new string, use it, and remember to free the original ptr
 *    
 *    or do this:
 *    
 *      search from beginning to find " " and save ptr to next val; save file_ID first.
 *      
 *      search from end for N " " and put NULL at end of what we want. save the extra tokens first
 */


  unsigned long len = strlen(paych);

  // get trailing tokens that are in web file but not in app files

  boolean not_done = true;

  char * testptr;

  while (not_done){

    testptr = (char *) memrchr(paych,' ',len);  // memrchr returns a pointer to last ' ' in paych in the first len bytes
#ifdef serial1
    lf();
    Serial.print(" strip paych len ");
    Serial.println(len);
    
  
    Serial.print(" testptr ");
    Serial.println((unsigned long)testptr);
    
    
#endif    
    if ((testptr - paych) == len){

      len --;
      
    } else {

      not_done = false;
      
    }
    
  }

  int tokencnt = 0;

  char * tok;

  tok = (char *) malloc(500);

  unsigned long diff;

  char * old = (char *) memrchr(paych,' ',len); 

  for (int i=0;i<27; i++){

    testptr = (char *) memrchr(paych,' ',len); 
  
    if (testptr == old){

      len --;
      
    } else {


      diff = old - testptr;

      len -= diff;

      if (diff > 1){

        memcpy(tok,testptr + 1,diff-1);  // add 1 to skip the space

        tok[diff-1] = '\0';

  
        switch (tokencnt){
                        
          case 5:


            files_on_web = get_uint16(tok); //strsep(&paych," ")); 
#ifdef serial1          
            Serial.print(" total files ");            
            Serial.print(files_on_web);
#endif
     
            
            // this is where we need to truncate paych for SHA1() to compare with files from app. SHA1 processes a string so we need a '\0' here
            //
            // also, the app file has a space at the end so bump testptr past the final space
            testptr++;
            *testptr = '\0';
            
            break;
                  
        }

        tokencnt++;
      }

      
    }

    old = testptr;
    
    
  }

  free(tok);
  
  return paych;
  
 }



