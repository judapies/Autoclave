// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave con Calderin
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
//#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HSPLL,WDT4096,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR,NOBROWNOUT,PUT,NOCPD,NOSTVREN,NODEBUG,NOWRT,NOWRTD,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTRB,NOEBTR,NOCPB,NOLPT1OSC,NOXINST,BORV43
//#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOBROWNOUT,NOVREGEN,NOMCLR,NOPUT,NOCPD,NOSTVREN,NODEBUG,NOWRT,NOWRTD,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTRB,NOEBTR,NOCPB,NOLPT1OSC,NOXINST
#use delay(clock=48000000)
//#use delay(clock=20000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N,errors)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xC0)
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
int8 l=0,Termostato=0,Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0,Dato_Exitoso=0,Menu=0,MenuAntt=240,z=0,Dato2=0,Inicio=0,Setpoint=0,codigoAlarma=0;
float Tpulso=0.0,error=0.0,d3=0.0;
float Temperatura=0.0,TempUSB=0.0,Temporal=0.0;
int8 Year=0,Mes=0,Dia=0,Hora=0,Minuto=0,Segundo=0,dow=0,Ciclo=0,SetTime=0,Abre=10;// dow=Day Of Week
int8 mEst=0,sEst=0,sSec=0,mSec=0,Menu2=0,tCodigo=0,Menu2_ant=0;
short Activo=0,Lectura=0,Grafica=0,Entro=0,cuentaAbre=0,errorCodigo=OFF;
int16 PresionCamara=0,PresionPreCamara=0,tiempos=0,tiempos2=0,tabre=0,posicion=0;

signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido

const char Texto[12][20]={"CALENTAMIENTO","NO ENVUELTOS 134","NO ENVUELTOS 121","ENVUELTOS 134","ENVUELTOS 121","ENVUELTO DOBLE 1",
"ENVUELTO DOBLE 2","PRION","BOWIE & DICK","TEST DE FUGAS","TEST DE COMPONENTES","HORA Y FECHA"};
char usuario[20];
byte fstate;                     //Guardara el estado del bus I2C
byte buffer[0x25], txbuf[0x25],rxbuf[0x25];     //Buffer de memoria

#include "Funciones.h"

#INT_SSP
void ssp_interupt (){
   
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupción

   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {         
       //Manda al maestro la información contenida en la posición de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
   }else{                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Información recibida corresponde a la posicion
         posicion = incoming;          //Se guarda posición
      }
      else if (fState == 2) {          //Información recibida corresponde al dato
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
      if(errorCodigo)
         tCodigo++;
      else
         tCodigo=0;
         
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
   usb_init();
   usb_task();
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   printf("page Bienvenida");
   SendDataDisplay();
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(3000);
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
   restart_wdt();
// Tiempo de Grafica//--------------------------------------
   if(tiempos2>=2000){
      Grafica=1;
      tiempos2=0;
   }
// -----------------//--------------------------------------
  
   if(Menu2==20){
      if(Menu2!=Menu){
         printf("page Funcionamiento");
         SendDataDisplay();
         Menu2_ant=Menu;
      }
   }
   
   if(Menu2<20 || Menu2==100){
      if(Menu2_ant!=Menu){
         printf("page Bienvenida");
         SendDataDisplay();
      }
   }
   
   /*
      if(Parada==0 && Entro==0){
         if(Activo==0){
            printf("page Emergencia");
            SendDataDisplay();
            Activo=1;
         }
      }else{
         if(Activo==1 && Parada==1){
            printf("page Bienvenida");
            SendDataDisplay();
            Activo=0;Entro=0;
         }
      }
   */
   if(Menu==240){
      if(RX_Buffer[4]==0x0a){//Oprimio Subir Puerta
         Abre=5;
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;                       
      }else if(RX_Buffer[4]==0x0b){//Solto Subir Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00;           
         Abre=10;
      }else if(RX_Buffer[4]==0x0c){//Oprimio Bajar Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00; 
         Abre=15;
      }else if(RX_Buffer[4]==0x0d){//Solto Bajar Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00;                       
         Abre=10;
      }
      Menu2_ant=Menu;
   }
   
   
   if(Menu==20){// Menu de Funcionamiento
      Entro=1;
      
      if(RX_Buffer[4]==0x0a){//Oprimio Subir Puerta
         Abre=5;
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;                       
      }
      
      if(RX_Buffer[4]==0x0b){//Solto Subir Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00;           
         Abre=10;
      }
      
      if(RX_Buffer[4]==0x0c){//Oprimio Bajar Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00; 
         Abre=15;
      }
      
      if(RX_Buffer[4]==0x0d){//Solto Bajar Puerta
         RX_Buffer[4]=0x00;            
         RX_Buffer2[4]=0x00;                       
         Abre=10;
      }
      //if(Entro==1){ 
       if(codigoAlarma==1 || codigoAlarma==200){
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==2){
         printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==9){
         printf("Funcionamiento.t4f.txt=\" Error Empaque\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==3){
         printf("Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==4){                  
         printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==5){
         printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==6){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         printf("Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==7){ // Alarma de Error de que bomba no ha arrancado.
         printf("Funcionamiento.t4f.txt=\" Error Bomba\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==8){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         printf("Funcionamiento.t4f.txt=\" Error Vacio\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==10){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreCalentando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==11){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreVacio\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==12){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Calentando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==13){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==14){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Finalizado\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==15){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Secando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==16){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Esterilizando\"");
         SendDataDisplay();
         tCodigo=0;
         errorCodigo=OFF;
      }else if(codigoAlarma==19){ 
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
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
            printf("add 12,0,%1.0f",Temperatura*0.66);
            SendDataDisplay();
            printf("add 12,1,%1.0f",PresionCamara*0.66);
            SendDataDisplay();
            printf("add 12,2,%1.0f",PresionPreCamara*0.66);
            SendDataDisplay();
            Grafica=0;
         }
         if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
         {                      
            if(Ciclo<=10){
               printf("t2f.txt=\"%u\"",Ciclo);
               SendDataDisplay();
               printf("t3f.txt=\"%s\"",Texto[Ciclo-1]);
               SendDataDisplay();
            }else{
               printf("t2f.txt=\"%u\"",Ciclo);
               SendDataDisplay();
               printf("t3f.txt=\"Personalizado\"");
               SendDataDisplay();
            }
            Lectura=0;
            //printf("tuser.txt=\"%s\"",usuario);
            //SendDataDisplay();
            printf("t6f.txt=\"%3.1f°C\"",Temperatura);
            SendDataDisplay();
            printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
            SendDataDisplay();
            printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
            SendDataDisplay();
            printf("Funcionamiento.t9f.txt=\"%03u°C\"",Setpoint);
            SendDataDisplay();      
            printf("Funcionamiento.t10f.txt=\"%02u:%02u\"",mEst,sEst);
            SendDataDisplay();      
            printf("Funcionamiento.t11f.txt=\"%02u:%02u\"",mSec,sSec);
            SendDataDisplay();      
      
            printf("fecha.txt=\" Fecha:%02u/%02u/20%02u   Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
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
    
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tamaño 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   }
}
