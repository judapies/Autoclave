// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave con Calderin
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV2,VREGEN,NOMCLR
#use delay(clock=20000000)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)


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

float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0,promediopresionp=0.0;
short emergency=0,flag_agua=0,flag_agua2=0,CamaraOUT=0,CamaraIN=0,flag_reservorio=0,flag_presostato=0,flag_vacio=0;
int8 r=0,t=0,r2=0,t2=0,TiempoControl=0,z=0,Frio=0,llena=0,PulsoPositivo=10,PulsosVacio=0,flag_pulso=0,Atmosferica=71;
int8 Resistencia=0,EVDesfogue=0,EVAgua=0,EVTrampa=0,MotorPuerta=0,EVEntradaAgua=0,BombaVacio=0,BombaAgua=0,Control=0,Alarma=0,AlarmaAgua=0;
int8 Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0,Presostato=0,ADC_Pt100=0,y=0,Chksum=0,flagprevacio=10,flagvacio=10;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Nivel=0,t_exhaust=0,tinicio=0,tagua=0,treservorio=0,PresionCamara16=0,PresionPreCamara16=0,tvacio=0;

// Variables para Pt100
short Lectura=0,Expulsa=0,Visua=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,tiempo_esterilizacion=0,p=0,MediaMovil=2,Termostato=0;
float V0=0.0,R1=6110.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=0.0,d3=0.0,d4=0.0,d5=0.0;
float V1=0.0,Presion=0.0,d6=0.0,d7=0.0,d8=0.0,V2=0.0,Presionp=0.0;
float Setpoint=0.0,Temperatura=0.0,TempUSB=0.0,PresionCamara=0.0,PresionPreCamara=0.0,K=0.006429;
float a=0.0011,b=2.3302,c=-244.0723,Temporal=0.0;

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x20], txbuf[0x20],rxbuf[0x20];     //Buffer de memoria

#INT_SSP
void ssp_interupt (){
   
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupción

   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {         
       //Manda al maestro la información contenida en la posición de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
      //i2c_write (txbuf[0]);      
      //i2c_write ((int8)V2);
      //Pt+=1.0;
   }
   /* Sino está solicitando lectura es que está enviando algo */
   else {                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Información recibida corresponde a la posicion
         posicion = incoming;          //Se guarda posición
      }
      else if (fState == 2) {          //Información recibida corresponde al dato
         rxbuf[posicion] = incoming;
      }
  }
}


void Carga_Vector(void){
   /*txbuf[0]=10;                       // Flag de estado de interruptor de puerta
   txbuf[1]=20;                       // Flag de estado de parada de Emergencia
   txbuf[2]=30;              // Nivel bajo del nivel de agua del calderin.
   txbuf[3]=flag_agua;                    // Bandera que indica que el calderin esta con agua.
   txbuf[4]=Presostato;                   // Parte baja de Presion de Camara
   txbuf[5]=(int8)Temperatura;            // Parte entera de Temperatura de Autoclave
   txbuf[6]=Alarma;                       // Alarma para errores de Funcionamiento de la autoclave
   txbuf[7]=AlarmaAgua;                   //  Alarma para errores de Agua
   txbuf[8]=Chksum;                       // Checksum para verificación de comunicación
   txbuf[9]=(int8)d3;                     // Parte Decimal de Temperatura
   txbuf[10]=make8(PresionCamara16,0);    // Parte baja de Presion de Camara
   txbuf[11]=make8(PresionCamara16,1);    // Parte alta de Presion de Camara
   txbuf[12]=make8(PresionPreCamara16,0); // Parte Baja de Presion de Precamara
   txbuf[13]=make8(PresionPreCamara16,1); // Parte Alta de Presion de Precamara
   txbuf[14]=flagprevacio;
   //txbuf[15]=Atmosferica;*/
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
   enable_interrupts(global);
   usb_init();
   usb_task();
   

while(true)
{
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
    Lee_Vector();
   
    TempUSB=Temperatura;
    recbuf[0]=(int8)(TempUSB);
    recbuf[1]=120;
    d3=TempUSB-(int8)TempUSB;
    d3=d3*10.0;
    recbuf[2]=(int8)d3;
    recbuf[3]=(int8)PresionCamara;
    recbuf[4]=(int8)8<<(int16)PresionCamara;
    recbuf[5]=(int8)PresionPreCamara;    
    recbuf[6]=(int8)8<<(int16)PresionPreCamara;
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
