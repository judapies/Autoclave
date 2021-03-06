#include <18F2550.h>
#device adc=10
//#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HSPLL,WDT8192,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR,NOBROWNOUT,NOCPD,PUT,BORV43,NOSTVREN,NODEBUG,NOWRT,NOWRTD,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTRB,NOEBTR,NOCPB,NOLPT1OSC,NOXINST
#use delay(clock=48000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N,errors,stream=display)
#use rs232(baud=19200, xmit=PIN_B2, rcv=PIN_B3, bits=8, parity=N,errors,stream=printer)
//#use rs232(baud=19200, xmit=PIN_C0, rcv=PIN_C1, bits=8, parity=N,errors,stream=printer)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xB0)
#define ON  1
#define OFF 0

#define USB_HID_DEVICE FALSE // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE 32 // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE 32 // size to allocate for the rx endpoint 1 buffer
#include <pic18_usb.h> // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
#include "header.h" // Configuración del USB y los descriptores para este dispositivo
#include <usb.c> // handles usb setup tokens and get descriptor reports
const int8 Lenbuf = 32; 
int8 recbuf[Lenbuf];
int8 rxubuf[Lenbuf];

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
int8 l=0,Termostato=0,Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0,Dato_Exitoso=0,Menu=0,MenuAntt=240,z=0,Dato2=0,Inicio=0,Setpoint=0,codigoAlarma=0,codigoAnt=0,Alarma=0,respuesta=0,PuertaL=0;
float Tpulso=0.0,error=0.0,d3=0.0;
float Temperatura=0.0,TempUSB=0.0,Temporal=0.0;
int8 Year=0,Mes=0,Dia=0,Hora=0,Minuto=0,Segundo=0,dow=0,Ciclo=0,SetTime=0,Abre=10;// dow=Day Of Week
int8 mEst=0,sEst=0,sSec=0,mSec=0,Menu2=0,tCodigo=0,Menu2_ant=0,posicionl=0,posicionh=0,i=0;
short Activo=0,Lectura=0,Grafica=0,Entro=0,cuentaAbre=0,errorCodigo=OFF;
int16 PresionCamara=0,PresionPreCamara=0,tiempos=0,tiempos2=0,tabre=0,CiclosTotales=0,posicion=0;
int8 YearRx=0,MesRx=0,DiaRx=0,HoraRx=0,MinutoRx=0,SegundoRx=0,Imprime=0,CiclosL=0,CiclosH=0,AireInt=0;
short flagImprime=0,flagImprime2=0,Finalizo=0;

signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido

char const texto[9][20]={"LIQUIDOS A","CAUCHO","INSTRUMENTAL","LIQUIDOS B","LIBRE 1","LIBRE 2","LIBRE 3",""};
const char user[10][20]={"Admin","Admin","Admin","Admin","Admin","Admin","Admin","Admin","Admin","Admin"};
char usuario[20];
byte fstate;                     //Guardara el estado del bus I2C
byte buffer[0x25], txbuf[0x25],rxbuf[0x25];     //Buffer de memoria


int8 rxbuf1[100];
int8 rxbuf2[100];
int8 rxbuf3[100];
int8 rxbuf4[100];
#include "Funciones.h"

#INT_SSP
void ssp_interupt (){ 
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupción
  
  if(fstate == 0x80) {        
       //Manda al maestro la información contenida en la posición de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
   }else{/* Sino está solicitando lectura es que está enviando algo *///Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Información recibida corresponde a la posicion
         posicionl = incoming;          //Se guarda posición
      }else if (fState == 2) {          //Información recibida corresponde al dato
         posicionh = incoming;
         posicion=make16(posicionh,posicionl);
      }else if (fState == 3) {          //Información recibida corresponde al dato         
         if(posicion<40){
            rxbuf[posicion] = incoming;
         }else if(posicion>=40 && posicion<=136){
            rxbuf1[posicion-40] = incoming;
         }else if(posicion>136 && posicion<=235){
            rxbuf2[posicion-137] = incoming;
         }else if(posicion>235 && posicion<=334){
            rxbuf3[posicion-236] = incoming;
         }else if(posicion>334 && posicion<=425){
            rxbuf4[posicion-335] = incoming;            
         }
      }
  }
}

float inttofloat(int8 decimal){
   float tmp;
   tmp=decimal;
   return tmp/10;
}

#int_rda 
void serial_rx_isr(){

   Dato2=fgetc(display);
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
      if(errorCodigo)
         tCodigo++;
      else
         tCodigo=0;
         
      //if(cuentaAbre)
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
   usb_init();
   usb_task();
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   //Inicio de Impresora
   putc(27,printer);
   putc(64,printer);
   delay_us(10);
   putc(10,printer);
   fprintf(display,"page Bienvenida");
   SendDataDisplay();
   fprintf(display,"page Bienvenida");
   SendDataDisplay();
   fprintf(display,"bkcmd=0");
   SendDataDisplay();
   delay_ms(2000);
   fprintf(display,"page Bienvenida");
   SendDataDisplay();
   Lee_Vector();
   Carga_Vector();
   
while(true)
{
   LeeDisplay();
   Lee_Vector();
   Carga_Vector();
   delay_ms(20);
   restart_wdt();
// Tiempo de Grafica//--------------------------------------
   if(tiempos2>=6000){
      Grafica=1;
      tiempos2=0;
   }
// -----------------//--------------------------------------


//----------------------------------------------- Impresora-----------------------------------------------------------------------------------------------------------
   if(codigoAlarma==10){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"------Llenando Camara ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==12){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"--------- Calentando ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==13){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"------- Despresurizando ------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==15){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"----------- Secando ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==16){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"------- Esterilizando -------- \n");
         delay_us(10);
      }
   }
   
   if(Imprime==15 && !flagImprime2){
      if(Ciclo>0 && Ciclo<11){
         fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");
         delay_us(10);
         fprintf(printer," OPERADOR:______________________\n");
         delay_us(10);
         fprintf(printer,"  Ciclo de Esterilizacion #%02u \n",Ciclo);
         delay_us(10);
         fprintf(printer,"%s\n",Texto[Ciclo-1]);
         delay_us(10);
         fprintf(printer,"Ciclo #%05Lu\n",CiclosTotales);
         delay_us(10);
         fprintf(printer,"FECHA: %02u-%02u-%02u \n",Dia,Mes,Year);
         delay_us(10);
         //fprintf(printer,"   HORA   TEMP    PRESION \n");
         fprintf(printer," HORA  TEMP     PC     PP   \n");
         delay_us(10);
         //fprintf(printer,"  %02u:%02u  %03.1f°C  %03LukPa \n",HoraRx,MinutoRx,Temperatura,PresionCamara);
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa \n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara);
         delay_us(10);
      }
      delay_ms(1000);
      flagImprime2=1;
      flagImprime=1;
      Finalizo=0;
   }
   if(Imprime==5 && !flagImprime && !Finalizo){
      fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa \n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara);
      flagImprime=1;
   }else if(Imprime==20 && !Finalizo){
      fprintf(printer," Parada de Emergencia Activada \n");
      delay_us(10);
      fprintf(printer,"       Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==21 && !Finalizo){
      fprintf(printer,"   Tiempo de Llenado prolongado\n");
      delay_us(10);
      fprintf(printer,"    EA01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==22 && !Finalizo){
      fprintf(printer,"      Termostato Activado\n");
      delay_us(10);
      fprintf(printer,"    EA02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==24 && !Finalizo){
      fprintf(printer,"Tiempo de Calentamiento Excesivo\n");
      delay_us(10);
      fprintf(printer,"    ET01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==23 && !Finalizo){
      fprintf(printer," Sobrepaso en la temperatura\n");
      delay_us(10);
      fprintf(printer,"    ET02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==25 && !Finalizo){
      fprintf(printer," Sobrepaso en la presion\n");
      delay_us(10);
      fprintf(printer,"    EP01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==26 && !Finalizo){
      fprintf(printer,"       Error Ciclo       \n");
      delay_us(10);
      fprintf(printer,"    EV01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==27 && !Finalizo){
      fprintf(printer," Tiempo de Vacio Excesivo\n");
      delay_us(10);
      fprintf(printer,"    EV02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==28 && !Finalizo){
      fprintf(printer,"      Ciclo Finalizado \n");
      delay_us(10);
      fprintf(printer,"       Correctamente \n");
      delay_us(10);
      fprintf(printer,"  %02u:%02u  %03.1f°C  %03LukPa  \n",Hora,Minuto,Temperatura,PresionCamara); 
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==29 && !Finalizo){
      fprintf(printer,"    Error Empaque Puerta\n");
      delay_us(10);
      fprintf(printer,"    EE01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==10){
      flagImprime=0;
      flagImprime2=0;
   }
   
   
   if(rxbuf1[0]==0xff && rxbuf1[1]==0xff){//indica que debe iniciar la escritura en la Impresora
      delay_ms(1000);
      fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");
      delay_us(10);
      fprintf(printer," OPERADOR  %s\n",user[rxbuf1[2]]);
      delay_us(10);
      fprintf(printer,"  Ciclo de Esterilizacion #%02u \n",rxbuf1[3]);
      delay_us(10);
      fprintf(printer,"%s\n",Texto[rxbuf1[3]-1]);
      delay_us(10);
      fprintf(printer,"Ciclo #%05Lu\n",CiclosTotales);
      delay_us(10);
      fprintf(printer,"FECHA: %02u-%02u-%02u \n",rxbuf1[4],rxbuf1[5],rxbuf1[6]);
      delay_us(10);
      fprintf(printer," HORA  TEMP     PC     PP   Al\n");
      delay_us(10);
      
      /*
      fprintf(printer," OPERADOR:%02u\n",rxbuf1[2]);
      delay_us(10);
      fprintf(printer," CICLO:%02u\n",rxbuf1[3]);
      delay_us(10);            
      fprintf(printer," FECHA:%02u/%02u/20%02u\n",rxbuf1[4],rxbuf1[5],rxbuf1[6]);
      delay_us(10);  
      fprintf(printer,"Hora   PC   PPC  Temp  Alar\n",);
      delay_us(10); 
      */
      for(i=0;i<10;i++){
         Hora=rxbuf1[13+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }
         Minuto=rxbuf1[14+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf1[9+(i*9)],rxbuf1[10+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf1[11+(i*9)],rxbuf1[12+(i*9)]);     
         if(PresionPreCamara>400){
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf1[7+(i*9)]+inttofloat(rxbuf1[8+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }   
         Alarma=rxbuf1[15+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      } 
      for(i=0;i<11;i++){
         Hora=rxbuf2[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf2[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf2[2+(i*9)],rxbuf2[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf2[4+(i*9)],rxbuf2[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf2[(i*9)]+inttofloat(rxbuf2[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf2[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      } 
      for(i=0;i<11;i++){
         Hora=rxbuf3[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf3[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf3[2+(i*9)],rxbuf3[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf3[4+(i*9)],rxbuf3[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf3[(i*9)]+inttofloat(rxbuf3[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf3[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         delay_ms(10);
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_ms(10);
      }
      for(i=0;i<2;i++){
         Hora=rxbuf4[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf4[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf4[2+(i*9)],rxbuf4[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf4[4+(i*9)],rxbuf4[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf4[(i*9)]+inttofloat(rxbuf4[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf4[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      }         
      fprintf(printer,"Impresion Finalizada\n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      respuesta=5;
      rxbuf1[0]=0;
      rxbuf1[1]=0;
    }else{
      respuesta=10;
    }
//-----------------------------------------------Fin Impresora-----------------------------------------------------------------------------------------------------------

   if(Menu2==20){//Menu recibido en I2C
      if(Menu2!=Menu){
         fprintf(display,"page Funcionamiento");
         SendDataDisplay();
         Menu2_ant=Menu;
      }
   }
   
   if(Menu2<20 || Menu2==100){
      if(Menu2_ant!=Menu){
         fprintf(display,"page Bienvenida");
         SendDataDisplay();
      }
   }else if(Menu2==255){
      if(Menu2!=Menu){
         fprintf(display,"page Emergencia");
         SendDataDisplay();
         Menu=255;
         BorraBuffer();
      }
   }

   if(Menu==240){
      Menu2_ant=Menu;
   }
   
   
   if(Menu==20){// Menu de Funcionamiento
      Entro=1;
      //if(Entro==1){ 
       if(codigoAlarma==1 || codigoAlarma==200){
         fprintf(display,"Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==2){
         fprintf(display,"Funcionamiento.t4f.txt=\" Puerta Lado Sucio Abierta\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==9){
         fprintf(display,"Funcionamiento.t4f.txt=\" Puerta Lado Limpio Abierta\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==3){
         fprintf(display,"Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==4){                  
         fprintf(display,"Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==5){
         fprintf(display,"Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==6){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         fprintf(display,"Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==7){ // Alarma de Error de que bomba no ha arrancado.
         fprintf(display,"Funcionamiento.t4f.txt=\" Error Ciclo\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==8){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         fprintf(display,"Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==10){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Llenando Camara\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==11){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" PreVacio\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==12){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Calentando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==13){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Despresurizando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==14){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Abra Lado Limpio\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Finalizado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==15){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Secando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==16){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Esterilizando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==19){ 
         fprintf(display,"Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         fprintf(display,"Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else{
         delay_ms(1000);
         Lee_Vector();
         if((codigoAlarma>20 && codigoAlarma<200) || codigoAlarma==0){
            errorCodigo=ON;
            //if(tCodigo>5)
               //reset_cpu();
         }
      }
      
         if(Grafica==1){
            fprintf(display,"add 14,0,%1.0f",Temperatura);
            SendDataDisplay();
            Grafica=0;
         }
         if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
         {                      
            if(Ciclo<=10){
               fprintf(display,"t2f.txt=\"%u\"",Ciclo);
               SendDataDisplay();
               fprintf(display,"t3f.txt=\"%s\"",Texto[Ciclo-1]);
               SendDataDisplay();
            }
            Lectura=0;
            //printf("tuser.txt=\"%s\"",usuario);
            //SendDataDisplay();
            fprintf(display,"t6f.txt=\"%3.1f°C\"",Temperatura);
            SendDataDisplay();
            fprintf(display,"Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
            SendDataDisplay();
            fprintf(display,"Funcionamiento.t12f.txt=\"000kPa\"");
            SendDataDisplay();
            fprintf(display,"Funcionamiento.t9f.txt=\"%03u°C\"",Setpoint);
            SendDataDisplay();      
            fprintf(display,"Funcionamiento.t10f.txt=\"%02u\"",mEst);
            SendDataDisplay();      
            fprintf(display,"Funcionamiento.t13f.txt=\"%02u\"",sEst);
            SendDataDisplay();      
            fprintf(display,"Funcionamiento.t11f.txt=\"%02u\"",mSec);
            SendDataDisplay();      
            fprintf(display,"Funcionamiento.t16f.txt=\"%02u\"",sSec);
            SendDataDisplay();      
      
            fprintf(display,"fecha.txt=\" Fecha:%02u/%02u/20%02u   Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
            SendDataDisplay();
         }
      //}
   }else{
      Entro=0;
   }
   
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated())
{
   
   if(usb_kbhit(1))
   {
      usb_get_packet(1, rxubuf, Lenbuf);
      
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
    
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tamańo 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
   
   }
}
