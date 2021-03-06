// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave con Calderin
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
#fuses HS,WDT4096,NOPROTECT,NOLVP,NODEBUG,NOVREGEN,NOMCLR
#use delay(clock=20000000)
//#use RS232(BAUD=19200, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7, stream=printer)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xC0)

#define ON  1
#define OFF 0

#define   M1p_on           output_bit(PIN_B6,1)
#define   M1p_off          output_bit(PIN_B6,0)

#define   M1n_on           output_bit(PIN_B7,1)
#define   M1n_off          output_bit(PIN_B7,0)

#define   M2p_on           output_bit(PIN_B3,1)
#define   M2p_off          output_bit(PIN_B3,0)

#define   M2n_on           output_bit(PIN_B2,1)
#define   M2n_off          output_bit(PIN_B2,0)

#define   S1p_on           output_bit(PIN_B4,1)
#define   S1p_off          output_bit(PIN_B4,0)

#define   S1n_on           output_bit(PIN_B5,1)
#define   S1n_off          output_bit(PIN_B5,0)

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x20], txbuf[0x20],rxbuf[0x20];     //Buffer de memoria
int8 conteo=0,accion=0,conteoAnt=0;
short nuevoDato=0,flagSubePuerta=0,flagBajaPuerta=0,flagSubePuertaOff=0,flagBajaPuertaOff=0;
int16 tSubePuerta=0,tSubePuertaOff=0,tBajaPuerta=0,tBajaPuertaOff=0,tiempos=0;

void subePuerta(){
   if(tSubePuerta>60){
      M1p_off;
      M1n_off;
      M2p_off;
      M2n_off;
      flagSubePuerta=OFF;
      flagSubePuertaOff=ON;
      if(tSubePuertaOff>=(1080/120)*tSubePuerta){
         tSubePuerta=0;
      }
   }else{
      M1p_on;
      M1n_off;
      M2p_on;
      M2n_off;
      flagSubePuerta=ON;
      flagSubePuertaOff=OFF;
      tSubePuertaOff=0;
   }
}

void borraSubePuerta(){
   flagSubePuertaOff=OFF;
   tSubePuerta=0;
   tSubePuertaOff=0;
   flagSubePuerta=OFF;
}

void bajaPuerta(){
   if(tBajaPuerta>60){
      M1p_off;
      M1n_off;
      M2p_off;
      M2n_off;
      flagBajaPuerta=OFF;
      flagBajaPuertaOff=ON;
      if(tBajaPuertaOff>=(1080/120)*tBajaPuerta){
         tBajaPuerta=0;
      }
   }else{
      M1p_off;
      M1n_on;
      M2p_off;
      M2n_on;
      flagBajaPuerta=ON;
      flagBajaPuertaOff=OFF;
      tBajaPuertaOff=0;
   }
}

void borraBajaPuerta(){
   flagBajaPuertaOff=OFF;
   tBajaPuerta=0;
   tBajaPuertaOff=0;
   flagBajaPuerta=OFF;
}

#int_TIMER1
void temp1s(void){
   set_timer1(40536); // Se repite cada 5ms
   tiempos++;
   
   if(tiempos>=200){  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
      
      if(flagSubePuerta)
         tSubePuerta++;
      
      if(flagSubePuertaOff)
         tSubePuertaOff++;
      
      if(flagBajaPuerta)
         tBajaPuerta++;
      
      if(flagBajaPuertaOff)
         tBajaPuertaOff++;
      tiempos=0;
   
   }
}

#INT_SSP
void ssp_interupt (){
   
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupci?n

   if(fstate == 0x80) {         
      i2c_write (txbuf[posicion]);
   }else {                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Informaci?n recibida corresponde a la posicion
         posicion = incoming;          //Se guarda posici?n
      }
      else if (fState == 2) {          //Informaci?n recibida corresponde al dato
         rxbuf[posicion] = incoming;
      }
  }
}

void Lee_Vector(void){
   accion=rxbuf[1];
   conteo=rxbuf[0];
}

void CargaVector(void){
   txbuf[0]=conteo;
   txbuf[1]=accion;
}


void main()
{
   fState = 0;
   for (posicion=0;posicion<0x10;posicion++){
      buffer[posicion] = 0x00;
      txbuf[posicion] = 0x00;
      rxbuf[posicion] = 0x00;
   }
   
   Lee_Vector();
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);   
   enable_interrupts(INT_SSP);
   setup_wdt(WDT_ON);
   enable_interrupts(global);
  
   M1p_off;
   M1n_off;
   M2p_off;
   M2n_off;
   S1p_off;
   S1n_off;
      
   while(true){
      Lee_Vector();
      CargaVector();
      restart_wdt();
      
      if(accion==10){
         subePuerta();
      }else if(accion==20){
         bajaPuerta();
      }else if(accion==30){
         M1p_off;
         M1n_off;
         M2p_off;
         M2n_off;
         S1p_off;
         S1n_off;
         borraBajaPuerta();
         borraSubePuerta();
      }else if(accion==40){
         S1p_on;
         S1n_off;
      }else if(accion==50){
         S1p_off;
         S1n_on;
      }else if(accion==60){
         S1p_off;
         S1n_off;
      }else if(accion==70){
         M1p_off;
         M1n_off;
         M2p_off;
         M2n_off;
         borraBajaPuerta();
         borraSubePuerta();
      }else{
         M1p_off;
         M1n_off;
         M2p_off;
         M2n_off;
         S1p_off;
         S1n_off;
         borraBajaPuerta();
         borraSubePuerta();
      } 

   }
}
