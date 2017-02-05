char cArray[30];
int ic=0;

int readDataRF()
{
   int done = 0;
   int inByte;
  
   uint8_t buf[VW_MAX_MESSAGE_LEN];
   uint8_t buflen = VW_MAX_MESSAGE_LEN;
   
   if (vw_get_message(buf, &buflen))
   {
      for (int i = 0; i < buflen; i++)
      {
         inByte = (char)buf[i];
         
         //--------------------------------------------------
         // On recieve of variables
         //--------------------------------------------------
         if (inByte=='a')
         {
            variableA = atoi(cArray);      //String to float
         }
         
         if (inByte=='e')
         {
            variableB = atoi(cArray);      //String to float
         }
         
          if (inByte=='k')
         {
            variableC = atoi(cArray);      //String to float
         }
         
          if (inByte=='m')
         {
            variableD = atoi(cArray);      //String to float
         }
         
            if (inByte=='u')
         {
            variableE = atoi(cArray);      //String to float
         }
         
      
         //--------------------------------------------------
         
         //--------------------------------------------------
         
         //--------------------------------------------------
         // if character a to z : reset arrays
         //--------------------------------------------------
         if (inByte>96 && inByte<123)
         {
            ic=0;
            for(int i=0; i<10; i++) cArray[i] = 0;
         }
         //--------------------------------------------------

         //--------------------------------------------------
         //If digit 0 to 9 .. or decimal point (46) or negative (45)
         //--------------------------------------------------
         if ((inByte>47 && inByte<58) || inByte==46 || inByte==45 )
         {
            //add to character array
            cArray[ic] = inByte; ic++;
         }
         //--------------------------------------------------
      }
      done = 1;
   }
   //Return status
   return done;
}
