#define WS2812_ZERO 0b10000000 
#define WS2812_ONE  0b11111000 

#define STRIP_SIZE 18 
volatile unsigned int8 Strip_RGBData[STRIP_SIZE][3]={0}; 

//takes a 24bit color and stores it in the array split it in 3 8bit variables 
void Strip_setPixelColor(int8 pixel, int32 color){ 
   Strip_RGBData[pixel][0] = (int8)(color >> 16); 
   Strip_RGBData[pixel][1] = (int8)(color >> 8); 
   Strip_RGBData[pixel][2] = (int8)(color); 
} 
//The same above, but does take separated values, "OVERLOADED FUNCTION" 
void Strip_setPixelColor(int8 pixel, int8 r, int8 g, int8 b){ 
   Strip_RGBData[pixel][0] = r; 
   Strip_RGBData[pixel][1] = g; 
   Strip_RGBData[pixel][2] = b; 
} 
//Makes a 24bit color from 3 8bit color variables 
int32 Strip_Color(int8 r, int8 g, int8 b){ 
   return ((int32)r << 16) | ((int32)g <<  8) | b; 
} 
//Output the Strip_RGBData array to the chips 
void Strip_Show(){ 
   unsigned int8 x,y,z; 
   for(x=0;x<STRIP_SIZE;x++){ 
      for(y=0;y<3;y++){ 
         for(z=0;z<8;z++){ 
            if(bit_test(Strip_RGBData[x][y],7-z)){ 
               spi_write(ws2812_one); 
            } 
            else{ 
               spi_write(ws2812_zero); 
            } 
         } 
      } 
   } 
   delay_us(40); 
} 

void Strip_ClearAll(){ 
   memset(Strip_RGBData,0,STRIP_SIZE); 
   Strip_Show(); 
} 

void Strip_SetAll(int8 value){ 
   memset(Strip_RGBData,value,STRIP_SIZE); 
   Strip_Show(); 
} 
void Strip_Init(){ 
   setup_spi(SPI_MASTER | SPI_L_TO_H | SPI_CLK_DIV_16); 
   Strip_Show(); 
} 
