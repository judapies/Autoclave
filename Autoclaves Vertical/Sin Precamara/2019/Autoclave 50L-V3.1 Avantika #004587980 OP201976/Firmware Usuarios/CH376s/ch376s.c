#include <18F2550.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR// PLL1 para 4 MHz
//#device PASS_STRINGS = IN_RAM 
#use delay(clock=20000000)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Definición de otros puertos
#define   buz_on         output_bit(PIN_A5,1)
#define   buz_off        output_bit(PIN_A5,0)

#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xB0)

int8 rxbuf[100];
int8 rxbuf2[100];
int8 rxbuf3[100];
int8 rxbuf4[100];
int8 i=0;
char plan[65]="\r\nHora \tPC  \tPPC \tTemp \tAlarma\r\n";
char nombre[20]="CICLO";
char usuario[10]="Usuario:";
char ciclo[10]="Ciclo:";
char dia[10]="Dia:";
char mes[10]="Mes:";
char year[10]="Ano:";
int8 posicionl=0,posicionh=0;
int16 PresionPreCamara=0.0,PresionCamara=0.0;
int8 Alarma;
int8 Hora=0,Minuto=0;
int16 posicion=0;
float Temperatura=0.0;
char tab[2]="\t";
char enter[4]="\r\n";
byte fstate=0;                     //Guardara el estado del bus I2C
byte respuesta=0,incoming=0;
#include "CH367s.h"

#INT_SSP
void ssp_interupt (){
   int incoming;                //Variable donde se recibe el byte que manda el maestro 
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupción
   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {        
       //Manda al maestro la información contenida en la posición de memoria que le ha solicitado
      i2c_write (respuesta);
      //i2c_write (dato[0]);
   }else{/* Sino está solicitando lectura es que está enviando algo *///Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Información recibida corresponde a la posicion
         posicionl = incoming;          //Se guarda posición
      }else if (fState == 2) {          //Información recibida corresponde al dato
         posicionh = incoming;
         posicion=make16(posicionh,posicionl);
      }else if (fState == 3) {          //Información recibida corresponde al dato         
         if(posicion<=96){
            rxbuf[posicion] = incoming;
         }else if(posicion>96 && posicion<=195){
            rxbuf2[posicion-97] = incoming;
         }else if(posicion>195 && posicion<=295){
            rxbuf3[posicion-196] = incoming;
         }else if(posicion>296 && posicion<=320){
            rxbuf4[posicion-297] = incoming;            
         }
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
enable_interrupts(global);
for(i=0;i<100;i++){
   rxbuf[i]=0;
}
buz_on;
delay_ms(500);
buz_off;
i=0;
while (true){    

   if(rxbuf[0]==0xff && rxbuf[1]==0xff){//indica que debe iniciar la escritura en USB
   delay_ms(1000);   
   reset();
      if(set_usb()!=0x00){
      respuesta=1;
      delay_ms(500);
         //if(check()==0x14){
         if(check()!=0x00){
         respuesta=2;
         delay_ms(500);
            //if(mount()==0x14){
            if(mount()!=0x00){
            respuesta=3;
            delay_ms(500);
               name(nombre);
               if(create()==0x14){
                  respuesta=4;
                  buz_on;
                  delay_ms(500);
                  buz_off;
                  escribir(usuario);
                  writeInt8(rxbuf[2]);
                  escribir(tab);
                  escribir(ciclo);
                  writeInt8(rxbuf[3]);
                  escribir(tab);
                  escribir(dia);
                  writeInt8(rxbuf[4]);
                  escribir(tab);
                  escribir(mes);
                  writeInt8(rxbuf[5]);
                  escribir(tab);
                  escribir(year);
                  writeInt8(rxbuf[6]);
                  escribir(tab);
                  escribir(plan);
                  respuesta=5;
               for(i=0;i<10;i++){
                     Hora=rxbuf[13+(i*9)];
                     if(Hora>25){
                        Hora=0;
                        break;
                     }
                     writeInt8(Hora);
                     Minuto=rxbuf[14+(i*9)];
                     if(Minuto>59){
                        Minuto=0;
                        break;
                     }
                     writeInt8(Minuto);
                     escribir(tab);
                     PresionCamara=make16(rxbuf[9+(i*9)],rxbuf[10+(i*9)]);
                     if(PresionCamara>400){
                        PresionCamara=0;
                        break;
                     }
                     writeInt16(PresionCamara);
                     escribir(tab);
                     PresionPreCamara=make16(rxbuf[11+(i*9)],rxbuf[12+(i*9)]);     
                     writeInt16(PresionPreCamara);
                     if(PresionPreCamara>400){
                        PresionPreCamara=0;
                        break;
                     }
                     escribir(tab);
                     Temperatura=rxbuf[7+(i*9)]+inttofloat(rxbuf[8+(i*9)]);
                     if(Temperatura>150){
                        Temperatura=0;
                        break;
                     }
                     writeFloat(Temperatura);                          
                     escribir(tab);
                     Alarma=rxbuf[15+(i*9)];
                     if(Alarma>20){
                        Alarma=0;
                        break;
                     }
                     writeInt8(Alarma);
                     escribir(enter);
                     buz_on;
                     delay_ms(30);
                     buz_off;
                     delay_ms(30);
               }
               for(i=0;i<11;i++){
                     Hora=rxbuf2[6+(i*9)];
                     if(Hora>25){
                        Hora=0;
                        break;
                     }
                     writeInt8(Hora);                     
                     Minuto=rxbuf2[7+(i*9)];
                     if(Minuto>59){
                        Minuto=0;
                        break;
                     }
                     writeInt8(Minuto);
                     escribir(tab);
                     PresionCamara=make16(rxbuf2[2+(i*9)],rxbuf2[3+(i*9)]);
                     if(PresionCamara>400){
                        PresionCamara=0;
                        break;
                     }
                     writeInt16(PresionCamara);
                     escribir(tab);
                     PresionPreCamara=make16(rxbuf2[4+(i*9)],rxbuf2[5+(i*9)]);    
                     if(PresionPreCamara>400){                     
                        PresionPreCamara=0;
                        break;
                     }
                     writeInt16(PresionPreCamara);
                     escribir(tab);
                     Temperatura=rxbuf2[(i*9)]+inttofloat(rxbuf2[1+(i*9)]);
                     if(Temperatura>150){
                        Temperatura=0;
                        break;
                     }
                     writeFloat(Temperatura);                          
                     escribir(tab);
                     Alarma=rxbuf2[8+(i*9)];
                     if(Alarma>20){
                        Alarma=0;
                        break;
                     }
                     writeInt8(Alarma);
                     escribir(enter);
                     buz_on;
                     delay_ms(30);
                     buz_off;
                     delay_ms(30);
               }
               for(i=0;i<11;i++){
                     Hora=rxbuf3[6+(i*9)];
                     if(Hora>25){
                        Hora=0;
                        break;
                     }
                     writeInt8(Hora);                     
                     Minuto=rxbuf3[7+(i*9)];
                     if(Minuto>59){
                        Minuto=0;
                        break;
                     }
                     writeInt8(Minuto);
                     escribir(tab);
                     PresionCamara=make16(rxbuf3[2+(i*9)],rxbuf3[3+(i*9)]);
                     if(PresionCamara>400){
                        PresionCamara=0;
                        break;
                     }
                     writeInt16(PresionCamara);
                     escribir(tab);
                     PresionPreCamara=make16(rxbuf3[4+(i*9)],rxbuf3[5+(i*9)]);  
                     if(PresionPreCamara>400){
                        PresionPreCamara=0;
                        break;
                     }
                     writeInt16(PresionPreCamara);
                     escribir(tab);
                     Temperatura=rxbuf3[(i*9)]+inttofloat(rxbuf3[1+(i*9)]);
                     if(Temperatura>150){
                        Temperatura=0;
                        break;
                     }
                     writeFloat(Temperatura);                          
                     escribir(tab);
                     Alarma=rxbuf3[8+(i*9)];
                     if(Alarma>20){
                        Alarma=0;
                        break;
                     }
                     writeInt8(Alarma);
                     escribir(enter);
                     buz_on;
                     delay_ms(30);
                     buz_off;
                     delay_ms(30);
               }
               for(i=0;i<3;i++){
                     Hora=rxbuf4[6+(i*9)];
                     if(Hora>25){
                        Hora=0;
                        break;
                     }
                     writeInt8(Hora);                     
                     Minuto=rxbuf4[7+(i*9)];
                     if(Minuto>59){
                        Minuto=0;
                        break;
                     }
                     writeInt8(Minuto);
                     escribir(tab);
                     PresionCamara=make16(rxbuf4[2+(i*9)],rxbuf4[3+(i*9)]);
                     if(PresionCamara>400){
                        PresionCamara=0;
                        break;
                     }
                     writeInt16(PresionCamara);
                     escribir(tab);
                     PresionPreCamara=make16(rxbuf4[4+(i*9)],rxbuf4[5+(i*9)]); 
                     if(PresionPreCamara>400){
                        PresionPreCamara=0;
                        break;
                     }
                     writeInt16(PresionPreCamara);
                     escribir(tab);
                     Temperatura=rxbuf4[(i*9)]+inttofloat(rxbuf4[1+(i*9)]);
                     if(Temperatura>150){
                        Temperatura=0;
                        break;
                     }
                     writeFloat(Temperatura);                          
                     escribir(tab);
                     Alarma=rxbuf4[8+(i*9)];
                     if(Alarma>20){
                        Alarma=0;
                        break;
                     }
                     writeInt8(Alarma);
                     escribir(enter);
                     buz_on;
                     delay_ms(30);
                     buz_off;
                     delay_ms(30);
               }
               respuesta=6;
          close();
               }
            }
          }
      }
          respuesta=7;
          rxbuf[0]=0;
          rxbuf[1]=0;
          buz_on;
          delay_ms(500);
          buz_off;
          reset();
          reset_cpu();
      }   
      reset();
      
   }
}
