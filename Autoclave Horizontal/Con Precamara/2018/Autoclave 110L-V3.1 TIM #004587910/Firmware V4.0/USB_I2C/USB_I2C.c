// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave con Calderin
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
#fuses HSPLL,WDT1024,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#use RS232(BAUD=19200, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7, stream=printer)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)

#define USB_HID_DEVICE FALSE // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE 32 // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE 32 // size to allocate for the rx endpoint 1 buffer
#include <pic18_usb.h> // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
#include "header.h" // Configuraci?n del USB y los descriptores para este dispositivo
#include <usb.c> // handles usb setup tokens and get descriptor reports
const int8 Lenbuf = 32; 
int8 recbuf[Lenbuf];
int8 rxubuf[Lenbuf];

/* Tabla de Modulos, Primera Fila A?o Regular y el siguiente a?o Bisiesto
E   F   M   A   M   J   J   A   S   O   N   D
0   3   3   6   1   4   6   2   5   0   3   5
0   3   4   0   2   5   0   3   6   1   4   6
*/
int8 Regular[12]={0,3,3,6,1,4,6,2,5,0,3,5};
int8 Bisiesto[12]={0,3,4,0,2,5,0,3,6,1,4,6};

// Variables para Pt100
int8 l=0,Termostato=0,Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0;
float Tpulso=0.0,error=0.0,d3=0.0;
float Temperatura=0.0,TempUSB=0.0,PresionPreCamara=0.0,Temporal=0.0;
int8 Year=0,Mes=0,Dia=0,Hora=0,Minuto=0,Segundo=0,dow=0,Ciclo=0,SetTime=0;// dow=Day Of Week
int8 YearRx=0,MesRx=0,DiaRx=0,HoraRx=0,MinutoRx=0,SegundoRx=0,dowRx=0,Modulo=0,Imprime=0;
short flagCiclo=0,flagImprime=0,Configura=0,Finalizo=0;
int16 PresionCamara=0;

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x20], txbuf[0x20],rxbuf[0x20];     //Buffer de memoria

#INT_SSP
void ssp_interupt (){
   
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupci?n

   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {         
       //Manda al maestro la informaci?n contenida en la posici?n de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
      //i2c_write (txbuf[0]);      
      //i2c_write ((int8)V2);
      //Pt+=1.0;
   }
   /* Sino est? solicitando lectura es que est? enviando algo */
   else {                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Informaci?n recibida corresponde a la posicion
         posicion = incoming;          //Se guarda posici?n
      }
      else if (fState == 2) {          //Informaci?n recibida corresponde al dato
         rxbuf[posicion] = incoming;
      }
  }
}


void Carga_Vector(void){
   txbuf[0]=Dia;
   txbuf[1]=Mes;
   txbuf[2]=Year;
   txbuf[3]=Hora;
   txbuf[4]=Minuto;
   txbuf[5]=Segundo;
   txbuf[6]=dow;
}

void Lee_Vector(void){
   Temporal=rxbuf[1];
   Temperatura=rxbuf[0];
   Temperatura=Temperatura+(Temporal/10);
   PresionCamara=make16(rxbuf[3],rxbuf[2]);
   PresionPreCamara=make16(rxbuf[5],rxbuf[4]);
   Tpulso=rxbuf[6];
   NivelAlto=rxbuf[7];
   NivelBajo=rxbuf[8];
   Termostato=rxbuf[9];
   Puerta=rxbuf[10];
   Parada=rxbuf[11];
   error=rxbuf[12];
   Ciclo=rxbuf[13];
   SetTime=rxbuf[14];
   DiaRx=rxbuf[15];
   MesRx=rxbuf[16];
   YearRx=rxbuf[17];
   HoraRx=rxbuf[18];
   MinutoRx=rxbuf[19];
   SegundoRx=rxbuf[20];
   Imprime=rxbuf[21];
}


void main()
{
   fState = 0;
   for (posicion=0;posicion<0x10;posicion++)
   {
      buffer[posicion] = 0x00;
      txbuf[posicion] = 0x00;
      rxbuf[posicion] = 0x00;
   }
   
   Lee_Vector();
   Carga_Vector();
   enable_interrupts(INT_SSP);
   usb_init();
   usb_task();
   setup_wdt(WDT_ON);
   enable_interrupts(global);
//Inicio de Impresora
   putc(27);
   putc(64);
   delay_us(10);
   putc(10);
   //32 Caracteres se pueden escribir en una linea de la impresora
 //fprintf(printer,"                                \n");
   delay_ms(1000);
   //fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");   
   //fprintf(printer,"  FECHA   HORA   TEMP    PRESION \n");
   //fprintf(printer,"01-05-18  09:10  121,2?C    12PSI  \n");
   //fprintf(printer,"%02u-%02u-%02u  %02u:%02u  %03.1f?C   %02uPSI  \n",Dia,Mes,Year,Hora,Minuto,Temperatura,PresionCamara*0.145); 

while(true)
{
   Lee_Vector();
   Carga_Vector();
   restart_wdt();
   
   if(Imprime==5 && !flagImprime && !Finalizo){
      if(!flagCiclo){
         if(Ciclo>0 && Ciclo<10){
            fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");
            fprintf(printer,"  Ciclo de Esterilizacion #%02u \n",Ciclo);
            flagCiclo=1;
            delay_us(10);
            fprintf(printer,"   FECHA: %02u-%02u-%02u \n",Dia,Mes,Year);
            fprintf(printer,"   HORA   TEMP    PRESION \n");
            fprintf(printer,"  %02u:%02u  %03.1f?C  %03LukPa  \n",Hora,Minuto,Temperatura,PresionCamara);
         }
      }else{
         fprintf(printer,"  %02u:%02u  %03.1f?C  %03LukPa  \n",Hora,Minuto,Temperatura,PresionCamara); 
      }
      flagImprime=1;
   }else if(Imprime==20 && !Finalizo){
      fprintf(printer," Parada de Emergencia Activada \n");
      fprintf(printer,"       Ciclo Cancelado \n");
      Finalizo=1;
   }else if(Imprime==21 && !Finalizo){
      fprintf(printer,"   Tiempo de Llenado prolongado\n");
      fprintf(printer,"    EA01 Ciclo Cancelado \n");
      Finalizo=1;
   }else if(Imprime==22 && !Finalizo){
      fprintf(printer,"      Termostato Activado\n");
      fprintf(printer,"    EA02 Ciclo Cancelado \n");
      Finalizo=1;
   }else if(Imprime==24 && !Finalizo){
      fprintf(printer,"Tiempo de Calentamiento Excesivo\n");
      fprintf(printer,"    ET01 Ciclo Cancelado \n");
      Finalizo=1;
   }else if(Imprime==23 && !Finalizo){
      fprintf(printer," Fluctuacion en la temperatura\n");
      fprintf(printer,"    ET02 Ciclo Cancelado \n");
      Finalizo=1;
   }else if(Imprime==25 && !Finalizo){
      fprintf(printer,"      Ciclo Finalizado \n");
      fprintf(printer,"       Correctamente \n");
      fprintf(printer,"  %02u:%02u  %03.1f?C  %03LukPa  \n",Hora,Minuto,Temperatura,PresionCamara); 
      Finalizo=1;
   }else if(Imprime==10){
      flagImprime=0;
   }
   
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated())
{
   
   if(usb_kbhit(1))
   {
      usb_get_packet(1, rxubuf, Lenbuf);
      if(rxubuf[0]==1)
      {
         txbuf[0]=10;txbuf[1]=20;txbuf[2]=30;//Trama de envio         
         
         txbuf[3]=rxubuf[1];//Ganancia Control PID
         txbuf[6]=rxubuf[2];//Desvio
         txbuf[8]=rxubuf[3];//Tciclo
         txbuf[7]=rxubuf[5];//MediaMovil
         txbuf[9]=rxubuf[6];//Integral
         txbuf[4]=rxubuf[7];//Ajuste+
         txbuf[5]=rxubuf[10];//Ajuste-
         txbuf[10]=rxubuf[11];//Derivativo
         txbuf[11]=rxubuf[12];//Parte entera Ganancia Instrumentacion
         txbuf[12]=rxubuf[13];//Parte decimal Ganancia Instrumentacion
         txbuf[13]=rxubuf[14];//Nible Bajo R51 Instrumentacion
         txbuf[14]=rxubuf[15];//Nible Alto R51 Instrumentacion
         txbuf[15]=rxubuf[16];//Parte entera RPT100 Instrumentacion
         txbuf[16]=rxubuf[17];//Parte decimal RPT100 Instrumentacion
         
      }else{
         if(rxubuf[12]==1)   
         {
            txbuf[17]=50;
         }else{
            txbuf[0]=l++;txbuf[1]=0;txbuf[2]=0;//Trama de envio
            txbuf[17]=0;
         }
      }
      
    }
   
    TempUSB=Temperatura;
    recbuf[0]=(int8)(TempUSB);
    recbuf[1]=120;
    d3=TempUSB-(int8)TempUSB;
    d3=d3*10.0;
    recbuf[2]=(int8)d3;
    recbuf[3]=rxbuf[2];
    recbuf[4]=rxbuf[3];
    recbuf[5]=rxbuf[4];    
    recbuf[6]=rxbuf[5];
    recbuf[7]=(int8)NivelBajo;
    recbuf[8]=(int8)NivelAlto;
    recbuf[9]=(int8)Tpulso;
    recbuf[10]=(int8)Termostato;
    recbuf[11]=(int8)error;     
    recbuf[13]=(int8)Puerta;
    recbuf[14]=(int8)Parada;
    
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tama?o 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   }
}
