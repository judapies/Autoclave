#include <18F2550.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR// PLL1 para 4 MHz
#use delay(clock=20000000)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CH367s.h"

// Definici?n de otros puertos
#define   Luz_on         output_bit(PIN_C2,1)
#define   Luz_off        output_bit(PIN_C2,0)

#define   buz_on         output_bit(PIN_A5,1)
#define   buz_off        output_bit(PIN_A5,0)


//#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xB0)

byte rxbuf[350],txbuf[0x20];
int i;
char plan[65]="\r\nHora \tPresion Camara \tPresion Precamara \tTemperatura \tAlarma\r\n";
char nombre[]="PRUEBACHANDA.TXT";
char letra[20]="Hola";
char string[10],data[];
int8 posicionl,posicionh;
float PresionPreCamara=0.0,PresionCamara=0.0,Alarma;
char punto=".";
char back[]="\n";
char tab[]="\t";
short usb_connected;

byte fstate;                     //Guardara el estado del bus I2C
byte posicion,respuesta,comando,incoming,status;


#INT_SSP

void ssp_interupt (){
   int incoming;                //Variable donde se recibe el byte que manda el maestro
  
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupci?n
 

   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {        
       //Manda al maestro la informaci?n contenida en la posici?n de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
   }/* Sino est? solicitando lectura es que est? enviando algo */
   else {                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Informaci?n recibida corresponde a la posicion
         posicionl = incoming;          //Se guarda posici?n
      }else if (fState == 2) {          //Informaci?n recibida corresponde al dato
         posicionh = incoming;
         posicion=make16(posicionh,posicionl);
      }else if (fState == 3) {          //Informaci?n recibida corresponde al dato
         rxbuf[posicion] = incoming;
      }
  }
}

float inttofloat(int8 decimal){
   float tmp;
   tmp=decimal;
   return tmp/10;
}

void main (){

   fState = 0; 
   enable_interrupts(INT_SSP);
   //enable_interrupts(global);
   buz_on;
   delay_ms(500);
   buz_off;
   
   reset();
   delay_ms(100);
   set_usb();
   delay_ms(100);
   check();
   delay_ms(100);
   mount();
   name(nombre);
   create();
   escribir(letra);   
   delay_ms(100);
   close();

while (true){
   
   buz_on;
   delay_ms(500);
   buz_off; 
}   
}

/*reset();
   if(set_usb()==0x51){
      buz_on;
      delay_ms(100);
      buz_off;
      if(check()==0x14){ 
      buz_on;
      delay_ms(100);
      buz_off;
         if(mount()==0x14){
         buz_on;
      delay_ms(100);
      buz_off;
            name(nombre); 
            if(create()==0x14){ 
            buz_on;
      delay_ms(100);
      buz_off;
               if(rxbuf[0]==0xff && rxbuf[1]==0xff){//indica que debe iniciar la escritura en USB
                  for(i=0;i<35;i++){
                  //Convierte Temperatura en vector de String
                     itoa(rxbuf[7+(i*9)],10,string);
                     strcat(data,string);
                     strcat(data,punto);
                     itoa(inttofloat(rxbuf[8+(i*9)]),10,string);
                     strcat(data,string);
                     strcat(data,tab);//Tabular
                     PresionCamara=make16(rxbuf[9+(i*9)],rxbuf[10+(i*9)]);
                     itoa(inttofloat(PresionCamara),10,string);
                     strcat(data,string);
                     strcat(data,tab);//Tabular
                     PresionPreCamara=make16(rxbuf[11+(i*9)],rxbuf[12+(i*9)]);     
                     itoa(inttofloat(PresionPreCamara),10,string);
                     strcat(data,string);
                     strcat(data,tab);//Tabular
                     Alarma=rxbuf[13+(i*9)];
                     itoa(inttofloat(Alarma),10,string);
                     strcat(data,string);
                     strcat(data,back);//Tabular y realizar Enter
                     output_toggle(PIN_A5);
                  }
               }    
               rxbuf[0]=0;rxbuf[1]=0;
               escribir(plan); 
               close();
            if(close()==0x14){
            }
            else
               respuesta=1;
            }
            else
               respuesta=1;
         }else     
            respuesta=1;
      }
      else     
         respuesta=1;
   }
   else     
   respuesta=1;*/
