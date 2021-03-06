// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave con Calderin
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
//#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
//#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOBROWNOUT,NOVREGEN,NOMCLR,NOPUT,NOCPD,NOSTVREN,NODEBUG,NOWRT,NOWRTD,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTRB,NOEBTR,NOCPB,NOLPT1OSC,NOXINST
#use delay(clock=48000000)
//#use delay(clock=20000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)


#define RX_BUFFER_SIZE  10
char Rx_Buffer[RX_BUFFER_SIZE+1];
char Rx_Buffer2[RX_BUFFER_SIZE+1];
char RX_Wr_Index=0;
char RX_Rd_Index=0;
char RX_Counter=0;

#define TX_BUFFER_SIZE  24
char Tx_Buffer[TX_BUFFER_SIZE+1];
char TX_Wr_Index=0;
char TX_Counter=0;
//int1 RX_Buffer_Overflow=0;

// Variables para Pt100
int8 l=0,Termostato=0,Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0,Dato_Exitoso=0,Menu=0,MenuAntt=240,z=0,Dato2=0,Inicio=0,Setpoint=0,Alarma=0;
float Tpulso=0.0,error=0.0,d3=0.0;
float Temperatura=0.0,TempUSB=0.0,Temporal=0.0;
int8 Year=0,Mes=0,Dia=0,Hora=0,Minuto=0,Segundo=0,dow=0,Ciclo=0,SetTime=0,Abre=10;// dow=Day Of Week
short Activo=0,Lectura=0,Grafica=0,Entro=0,cuentaAbre=0;
int16 PresionCamara=0,PresionPreCamara=0,tiempos=0,tiempos2=0,tabre=0;

signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x25], txbuf[0x25],rxbuf[0x25];     //Buffer de memoria

#include "Funciones.h"

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

#int_rda 
void serial_rx_isr(){

   Dato2=fgetc();
   if(Dato2==0x65){//Inicio Comunicacion
      Inicio=1;
      RX_Wr_Index =0;
   }
   //Inicio=1;
   if(Inicio==1){
      Rx_Buffer2[RX_Wr_Index] = Dato2;
      RX_Wr_Index++;
   }
   if(RX_Wr_Index >= RX_BUFFER_SIZE){
      RX_Wr_Index =0;
      Inicio=0;
   }

   if(RX_Wr_Index==0){
      if(Rx_Buffer2[0]==0x65 && Rx_Buffer2[1]==0xff && Rx_Buffer2[2]==0xff && Rx_Buffer2[8]==0x00 && Rx_Buffer2[9]==0xff ){
         Dato_Exitoso=5;
      }else{
         Dato_Exitoso=10;
      }
   }
   //Dato_Exitoso=5; 
}

#int_TIMER1
void temp1s(void){
   set_timer1(5536); // Se repite cada 5ms
   tiempos++;
   tiempos2++;
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {
      tiempos=0;Lectura=1;
      if(cuentaAbre)
         tabre++;
   }
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
   
   enable_interrupts(int_rda);
   enable_interrupts(INT_SSP);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   //usb_init();
   //usb_task();
   //setup_wdt(WDT_ON);
   enable_interrupts(global);
   printf("page Bienvenida");
   SendDataDisplay();
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(6000);
   printf("page Bienvenida");
   SendDataDisplay();
   //printf("page Emergencia");
   //SendDataDisplay();
   Lee_Vector();
   Carga_Vector();
   
while(true)
{
   LeeDisplay();
   Lee_Vector();
   Carga_Vector();
   delay_ms(20);
   //restart_wdt();
// Tiempo de Grafica//--------------------------------------
   if(tiempos2>=8000){
      Grafica=1;
      tiempos2=0;
   }
// -----------------//--------------------------------------
   //if(Entro==0){
   //printf("t1.txt=\"%u %u %u %u %2.1f %u\"",Parada,Puerta,Termostato,Ciclo,Temperatura,Entro);
   //SendDataDisplay();
   //}
   if(tabre<3 && cuentaAbre){
      Abre=5;       
   }else if(tabre>=3){
      Abre=10;
      cuentaAbre=0;
      tabre=0;
   }
   
   if(Menu==240){
      if(RX_Buffer[4]==0x22){
         cuentaAbre=1;
      }
      if(Ciclo>0){
         printf("page Funcionamiento");
         SendDataDisplay();
      }
      
      if(Parada==0 && Entro==0){
         if(Activo==0){
            printf("page Emergencia");
            SendDataDisplay();
            Activo=1;
         }
      }else{
         if(Activo==1){
            printf("page Bienvenida");
            SendDataDisplay();
            Activo=0;
         }
      }
   }
   
   
   if(Menu==20){// Menu de Funcionamiento
      Entro=1;
      if(RX_Buffer[4]==0x22){
         cuentaAbre=1;
      }
      //if(Entro==1){ 
         if(Alarma==1){
            printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==2){
            printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Pausado\"");
            SendDataDisplay();
         }else if(Alarma==3){
            printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==4){
            printf("Funcionamiento.t4f.txt=\" Termostato\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==5){
            printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==6){
            printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==7){
            printf("Funcionamiento.t4f.txt=\" SobrePresi?n\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
         }else if(Alarma==20){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Llenando\"");
            SendDataDisplay();
         }else if(Alarma==21){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Calentando\"");
            SendDataDisplay();
         }else if(Alarma==22){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Calentando\"");
            SendDataDisplay();
         }else if(Alarma==23){
            printf("Funcionamiento.t4f.txt=\" Puede Abrir\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Finalizado\"");
            SendDataDisplay();
         }else if(Alarma==24){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
            SendDataDisplay();
         }else if(Alarma==25){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Secando\"");
            SendDataDisplay();
         }else if(Alarma==26){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Esterilizando\"");
            SendDataDisplay();
         }
      
         if(Grafica==1){
            printf("add 14,0,%1.0f",Temperatura*0.75);
            SendDataDisplay();
            Grafica=0;
         }
         if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
         {                      
            if(Ciclo==1){
               printf("t2f.txt=\"1\"");
               SendDataDisplay();
               printf("t3f.txt=\"PLASTICO\"");
               SendDataDisplay();
            }else if(Ciclo==2){
               printf("t2f.txt=\"2\"");
               SendDataDisplay();
               printf("t3f.txt=\"INSTRUMENTAL\"");
               SendDataDisplay();
            }else if(Ciclo==3){
               printf("t2f.txt=\"3\"");
               SendDataDisplay();
               printf("t3f.txt=\"LIQUIDOS A\"");
               SendDataDisplay();
            }else if(Ciclo==4){
               printf("t2f.txt=\"4\"");
               SendDataDisplay();
               printf("t3f.txt=\"TEXTIL\"");
               SendDataDisplay();
            }else if(Ciclo==5){
               printf("t2f.txt=\"5\"");
               SendDataDisplay();
               printf("t3f.txt=\"LIBRE\"");
               SendDataDisplay();
            }
            Lectura=0;
            printf("t6f.txt=\"%3.1f\"",Temperatura);
            SendDataDisplay();
            printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
            SendDataDisplay();
            printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
            SendDataDisplay();
            printf("Funcionamiento.t9f.txt=\"%03u\"",Setpoint);
            SendDataDisplay();      
            printf("Funcionamiento.t10f.txt=\"%02u\"",((Testerilizacion[3]*10)+Testerilizacion[2]));
            SendDataDisplay();      
            printf("Funcionamiento.t13f.txt=\"%02u\"",((Testerilizacion[1]*10)+Testerilizacion[0]));
            SendDataDisplay();      
            printf("Funcionamiento.t11f.txt=\"%02u\"",((Tsecado[3]*10)+Tsecado[2]));
            SendDataDisplay();      
            printf("Funcionamiento.t16f.txt=\"%02u\"",((Tsecado[1]*10)+Tsecado[0]));
            SendDataDisplay();      
            printf("code_c");
            SendDataDisplay();      
            printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
            SendDataDisplay();
         }
      //}
   }else{
      Entro=0;
   }
   }
}
