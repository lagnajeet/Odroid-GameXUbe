#define __USB_PIC_PERIF__ 1

#include <18LF25K50.h>
#device ADC=8
#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOPROTECT                //Code not protected from reading
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOPUT                    //No Power Up Timer
#FUSES NOCPD                    //No EE protection
#FUSES STVREN                   //Stack full/underflow will cause reset
#FUSES NODEBUG                  //No Debug mode for ICD
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOWRT                    //Program memory not write protected
#FUSES NOWRTD                   //Data EEPROM not write protected
#FUSES MCLR                     //Master Clear pin enabled
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)
#FUSES PLL3X, PLLEN, HSH, NOCPUDIV, PRIMARY, NOIESO, NOFCMEN
#use delay(internal=48000000,USB_FULL) 
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8)

#DEFINE USB_HID_DEVICE  TRUE //Tells the CCS PIC USB firmware to include HID handling code.
#define USB_EP1_TX_ENABLE  USB_ENABLE_INTERRUPT   //turn on EP1 for IN bulk/interrupt transfers
#define USB_EP1_TX_SIZE    64  //allocate 64 bytes in the hardware for transmission
#define USB_EP1_RX_ENABLE  USB_ENABLE_INTERRUPT   //turn on EP1 for OUT bulk/interrupt transfers
#define USB_EP1_RX_SIZE    64  //allocate 64 bytes in the hardware for reception

#define LOADER_SIZE                 (0x7FF)
#define LOADER_START                (0)
#define LOADER_END                  (LOADER_SIZE)
#define APPLICATION_START           (LOADER_SIZE+1)
#define APPLICATION_END             (getenv("PROGRAM_MEMORY")-1)
#define APPLICATION_ISR             (APPLICATION_START+8)

#ifndef _bootloader
 //in the application, this moves the reset and isr vector out of the bootload
 //space.  it then reserves the loader space from being used by the application.
 #build(reset=APPLICATION_START, interrupt=APPLICATION_ISR)
 #org 0, LOADER_END {}
#endif 

#include <pic18_usb.h>
#include "usb_desc_hid.h"    //USB Configuration and Device descriptors for this UBS device
#include <usb.c> 

#use FAST_IO(ALL) 
#include <WS2812.h> 
#include <WS2812_Effects.c> 

int1 systemup=0;
int1 LEDOFF=0;
unsigned int16 counter1=0;
int pwrButtonPWM=0;
unsigned int16 caseFanPWM=12000;
signed int PWMDirection=1;
// Pin B2 is high if system is on 

#INT_RTCC
void  RTCC_isr(void) 
{
   int i=0;
   if(systemup==0)     //the system is off. do a fade in , fade out of the blue power button
    {       
         if(caseFanPWM>0)
            caseFanPWM--;
         set_pwm1_duty((int16)caseFanPWM);
         set_pwm2_duty((int16) pwrButtonPWM);
         if(pwrButtonPWM==128)
            PWMDirection=-1;
         if(pwrButtonPWM==3)
            PWMDirection=1;
         pwrButtonPWM=pwrButtonPWM+PWMDirection;
         counter1=0;
    }
    else
    {
      LEDOFF=0;
      caseFanPWM=10200;
      set_pwm1_duty((int16)caseFanPWM);
      pwrButtonPWM=0;
      set_pwm2_duty((int16) 0);
    }
}


void main() 
{
  unsigned int8 in_data[65];
  unsigned int8 out_data[65];
  unsigned int idx=0;
  int1 buttonPressed=0;
  char kbChar;
  int8 R,G,B;
  setup_adc_ports(sAN0 | sAN3 | sAN4);
  setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_0);
  setup_timer_0(RTCC_INTERNAL|RTCC_DIV_4);      //52.4 ms overflow
  enable_interrupts(INT_RTCC);
  enable_interrupts(GLOBAL);
  setup_timer_2(T2_DIV_BY_16, 106, 1); //142 us overflow, 142 us interrupt
  unsigned int32 commandcounter_before = 65535;
  unsigned int32 commandcounter = 0;
  setup_ccp1(CCP_PWM|CCP_SHUTDOWN_AC_L|CCP_SHUTDOWN_BD_L);
  setup_ccp2(CCP_PWM);
  set_pwm2_duty((int16) 0);
  int i = 0;
  unsigned int value = 0;
  output_high(PIN_B3);
  output_high(PIN_B7);
  output_float(PIN_C0);
  output_low(PIN_C0);
  output_high(PIN_C0);
  delay_ms(1500);
  output_low(PIN_C0);
  //output_float(PIN_C0);
  set_tris_b(0b00000000);
  set_tris_c(0b00000000);
  for (i = 0; i < 65; i++) {
    in_data[i] = 0;
    out_data[i] = 0;
  }
  Strip_Init();
  for (i = 0; i < STRIP_SIZE; i++)
    Strip_setPixelColor(i, 16, 32, 16);
  Strip_Show();
  while (TRUE) 
  {
    output_high(PIN_B7);   
    usb_task();
    if (usb_enumerated()) {
      usb_get_packet(1, in_data, 64);
      commandcounter = in_data[32];
      commandcounter = (commandcounter << 8) | in_data[31];
      commandcounter = (commandcounter << 8) | in_data[30];
      commandcounter = (commandcounter << 8) | in_data[29];
      if (commandcounter_before != commandcounter) {
        for (i = 0; i < 64; i++)
          out_data[i] = in_data[i];
        if (in_data[0] == 1) {
          disable_interrupts(GLOBAL);
          write_eeprom(0, 0x9B);
          reset_cpu();
        } else if (in_data[0] == 2) {
          delay_ms(1);
          reset_cpu();
        } else if (in_data[0] == 3) {
          write_eeprom(0, 0xFF);
        }
        commandcounter_before = commandcounter;
      }
    }
    //putc('*');
    if (!input(PIN_A4)) 
    {
      if(buttonPressed==0)
      {
         putc(128);
         buttonPressed=1;
        output_low(PIN_C0);
         output_high(PIN_C0);
         delay_ms(5000);
         output_low(PIN_C0);
         systemup=0;
      }
    }
    else
    {
      buttonPressed=0;
    }
    

    if(kbhit())
    {

      kbChar=getc();
      
      if(kbChar==5)
      {
         systemup=1;
         idx=1;
      }
      else if(kbChar==4)
      {
         systemup=0;
         idx=0;
         for (i = 0; i < STRIP_SIZE; i++)
                   Strip_setPixelColor(i, 0, 0, 0);
         Strip_Show();         
      }
      else
      {
         if(idx==1)
            R=kbChar;
         else if(idx==2)
            G=kbChar;        
         else if(idx==3)
         {
            b=kbChar;       
            idx=0;
            for (i = 0; i < STRIP_SIZE; i++)
                   Strip_setPixelColor(i, G, R, B);
            Strip_Show();             
         }
         idx++;
      }

    } 
    
    if (input(PIN_B2))  //This pin senses if the system is on or not
    {
      systemup=1;
      LEDOFF=0;
    }
    else
    {
      systemup=0;
      if(LEDOFF==0)
         {
            LEDOFF=1;
            for (i = 0; i < STRIP_SIZE; i++)
                   Strip_setPixelColor(i, 0, 0, 0);
            Strip_Show();  
         }
    }
      //putc('/');
    //{
      //output_low(PIN_C0);
      //output_high(PIN_C0);
      //delay_ms(250);
      //output_low(PIN_C0);
      //output_float(PIN_C0);
      //delay_ms(10000);
      //printf("PIN_A4 is Low");
      //set_pwm2_duty((int16) 200);

    //}
    //else
      //set_pwm2_duty((int16) 0);
    //if (input(PIN_C7)) 
    //  set_pwm2_duty((int16) 0);
    //else
    //  set_pwm2_duty((int16) 200);
    //else
    //   output_low (PIN_C0);

    //TODO: User Code
    //      for(i=0;i<100;i++)
    //      {
    //         set_pwm2_duty((int16)i);
    //         delay_ms (10);
    //      }
    //      for(i=100;i>0;i--)
    //      {
    //         set_pwm2_duty((int16)i);
    //         delay_ms (10);
    //      }
    //      set_adc_channel(4);
    //      delay_us(10);
    //      value = read_adc();
    //if(input(PIN_A1))
    //    set_pwm2_duty((int16)100);
    //else
    //    set_pwm2_duty((int16)0);

    //ColorWipe(strip_Color(255, 0, 0), 50); 
    // ColorWipe(strip_Color(0, 255, 0), 50); 
    // ColorWipe(strip_Color(0, 0, 255), 50); 

    // TheaterChase(Strip_Color(127,127,127), 50); 
    // TheaterChase(Strip_Color(127,0,0), 50); 
    //TheaterChase(Strip_Color(0,127,0), 50); 
    // TheaterChase(Strip_Color(0,0,127), 50); 

    //Rainbow(20); 

    //RainbowCycle(20); 

    //TheaterChaseRainbow(50); 
  }

}
