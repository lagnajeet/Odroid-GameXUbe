int32 Wheel(int8 WheelPos) { 
   WheelPos = 255 - WheelPos; 
   if(WheelPos < 85){ 
      return Strip_Color(255 - (WheelPos * 3), 0, WheelPos * 3); 
   } 
   else if(WheelPos < 170){ 
      WheelPos -= 85; 
      return Strip_Color(0, (WheelPos * 3), 255 - WheelPos * 3); 
   } 
   else{ 
      WheelPos -= 170; 
      return Strip_Color(WheelPos * 3, 255 - (WheelPos * 3), 0); 
   } 
} 

void Rainbow(int8 wait) { 
   unsigned int16 i,j; 
   for(j=0;j<256;j++) { 
      for(i=0;i<STRIP_SIZE;i++) { 
         Strip_setPixelColor(i, Wheel((i+j) & 255)); 
      } 
   Strip_Show(); 
   delay_ms(wait); 
   } 
} 

void RainbowCycle(int8 wait) { 
   int16 i,j; 
   for(j=0;j<256*5; j++) { // 5 cycles of all colors on wheel 
      for(i=0;i<STRIP_SIZE;i++) { 
      Strip_setPixelColor(i, Wheel(((i * 256 / STRIP_SIZE) + j) & 255)); 
    } 
   Strip_Show(); 
   delay_ms(wait); 
   } 
} 

void ColorWipe(int32 c, int8 wait) { 
   int16 i; 
   for(i=0;i<STRIP_SIZE;i++) { 
      Strip_setPixelColor(i,c); 
      Strip_Show(); 
      delay_ms(wait); 
  } 
} 

void TheaterChase(int32 c, int8 wait){ 
   int16 j,q,i; 
   for(j=0;j<10;j++){ 
      for(q=0;q<3;q++){ 
         for(i=0;i<STRIP_SIZE;i=i+3){ 
            Strip_setPixelColor(i+q, c); 
         } 
         Strip_show(); 
         delay_ms(wait); 
         for(i=0;i<STRIP_SIZE;i=i+3){ 
            Strip_setPixelColor(i+q, 0); 
         } 
      } 
   } 
} 

void TheaterChaseRainbow(int8 wait){ 
   int16 j,q,i; 
   for(j=0;j<256;j++){ 
      for(q=0;q<3;q++){ 
         for(i=0;i<STRIP_SIZE;i=i+3){ 
            Strip_setPixelColor(i+q, Wheel((i+j) % 255)); 
         } 
         Strip_show(); 
         delay_ms(wait); 
         for(i=0;i<STRIP_SIZE;i=i+3){ 
            Strip_setPixelColor(i+q,0); 
         } 
      } 
   } 
}
