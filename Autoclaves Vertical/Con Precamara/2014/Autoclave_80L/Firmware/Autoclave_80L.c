// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci�n para Autoclave 80Lts.
// Tiene Contrase�a de incio para permitir funcionamiento de Cabina.
// Tiene Men�:Inicio Proceso, Ciclo y Cambio de Contrase�a.
// Falta Agregar Control de Temperatura para PT100.
// Ing. Juan David Pi�eros.
// Ing. Faiver Humberto Trujillo.
// JP Inglobal. 2014

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV4,VREGEN,NOMCLR// PLL1 para 4 MHz
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL3,CPUDIV4,VREGEN,NOMCLR// PLL1 para 12 MHz
#use delay(clock=48000000)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definici�n de teclado - NC
#define   UP            input(PIN_A1)
#define   DOWN          input(PIN_A2)
#define   RIGHT         input(PIN_A3)
#define   LEFT          input(PIN_A5)
#define   AL1           input(PIN_E0)
//#define   AL2           input(PIN_A6)

// Definici�n de otros puertos
#define   Camara_OUT_on     output_bit(PIN_C0,1)
#define   Camara_OUT_off    output_bit(PIN_C0,0)
#define   Resistencia_on    output_bit(PIN_C1,1)
#define   Resistencia_off   output_bit(PIN_C1,0)
#define   Camara_IN_on      output_bit(PIN_C2,1)
#define   Camara_IN_off     output_bit(PIN_C2,0)

#define USB_HID_DEVICE FALSE // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE 32 // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE 32 // size to allocate for the rx endpoint 1 buffer
#include <pic18_usb.h> // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
#include "header.h" // Configuraci�n del USB y los descriptores para este dispositivo
#include <usb.c> // handles usb setup tokens and get descriptor reports
const int8 Lenbuf = 32;
int8 recbuf[Lenbuf];
int8 rxbuf[Lenbuf];

char  JP[] = "JP Inglobal", Autoclave[] = "Autoclave Vertical",tiempo_est[]="T est=",tiempo_sec[]="T sec=";
char  Lts[] = "80 Litros",Contra[] = "Cambio Contrasena",Men[] = "Menu",Fle[] = ">",Tempe[] = "Iniciar Proceso",Param[] = "Ciclo";
char  Proc[] = "Proceso",Textil[]= "Textil-Instrumental", Calentando[]="Calentando",Level[]="Nivel";
char  Caucho[]= "Caucho",Instrumental[]= "Instrumental Suave", Esterilizando[]="Esterilizando",Secando[]="Secando";
char  Liquidos[]= "Liquidos", Vacio[]=":",Creado[]= "Libre", T40[]= "T:134�C",Test40[]= "t_est:5m", Tsec40[]= "t_sec:15m",C[]="C";
char  TC[]= "T:121�C",TestC[]= "t_est:25m",TsecC[]= "t_sec:15m", TI[]= "T:121�C",TestI[]= "t_est:20m",Desfogue_Lento[]="Desfogue Lento";
char  TsecI[]= "t_sec:N/A",TL[]= "T:121�C",TestL[]= "t_est:20m",TsecL[]= "t_sec:N/A",ok[]= "OK", T[]="Temperatura";
char  T_esterilizacion[]="T Esterilizacion",T_secado[]="T Secado",Desfogue[]="Desfogue Rapido",Si[]="Si",Noo[]="No",Finalizado[]="Finalizado";
int8 Menu=0, n_opcionH=32,n_opcionL=16,unidad=11,Flecha=16,Flecha1=3,Flecha2=16, nMenuL=16, nMenuH=32, Flx=36, Fly=8,Liquid=0;
short estadot40=0,estadoc=0,estadoi=0,estadol=0,estadop=0,Flanco=0,Flanco1=0,Flanco2=0,Flanco3=0;
//signed  int8 clave[4]={0,0,0,0};
//int8 contrasena[4]={0,0,0,0};   // 0=Verdadero, 1=Falso
int16 tiempos=0,tiempos3=0,m=0,tiempoctrl=0;
short tiempo_esterilizacion=0,tiempo_secado=0,fin_ciclo=0,i=0,n=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido
// Variables para Pt100
short Lectura=0,modifica=0,Expulsa=0, est=0;
int Setpoint=121,h=0,l=0,Nivel,Minutos=10,Open=100,Ajuste=0,CUR=100,Ajusten=0,R2=0;
float V0,R1=1000.7,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=10.0,desvio=0.0,d3=0.0;

void glcd_imagen(int8 x)
{
   char i,j;
   signed char k; 
   
   if(x==1){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(tijeras[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==2){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(guante[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==3){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(Delicado[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==4){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(Liquido[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==5){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(Proceso[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
}

void mensajes(int8 x,y){// Funcion para imprimir mensajes de Menu Principal.
  if(x==1)
   {glcd_text57(7, y, Tempe, 1, ON);}
   if(x==2)
   {glcd_text57(7, y, Param, 1, ON);}
   if(x==3)
   {glcd_text57(7, y, Contra, 1, ON);  }
   if(x==4)
   {glcd_text57(7, y, Textil, 1, ON);  }
   if(x==5)
   {glcd_text57(7, y, Caucho, 1, ON);  }
   if(x==6)
   {glcd_text57(7, y, Instrumental, 1, ON);  }
   if(x==7)
   {glcd_text57(7, y, Liquidos, 1, ON);  }
   if(x==8)
   {glcd_text57(7, y, Creado, 1, ON);  }
}
 
#int_TIMER1
void temp1s(void){
   set_timer1(36530);  // 5 ms 5536 , Posiblemente cargar con 22144 debido a F=12MHz
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if((Testerilizacion[0]==Testerilizacionp[0])&&(Testerilizacion[1]==Testerilizacionp[1])&&(Testerilizacion[2]==Testerilizacionp[2])&&(Testerilizacion[3]==Testerilizacionp[3]))
   {tiempo_secado=1;}
   
      if((Tsecado[0]==Tsecadop[0])&&(Tsecado[1]==Tsecadop[1])&&(Tsecado[2]==Tsecadop[2])&&(Tsecado[3]==Tsecadop[3]))// Si cumplio
      {
         if(Liquid==0)
         {tiempo_secado=0;fin_ciclo=1;}//tiempo de secado sin seleccionar ningun programa de desfogue lento, activa fin de ciclo.
      }
   
   if(tiempos3==12000)
   {tiempos3=0;Expulsa=1;}
   
   if(tiempos==200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {tiempos=0;Lectura=1;
   //----------------------------- Tiempo Total-----------------------------//
   
   if(tiempo_esterilizacion==1)
      {
                     
      Testerilizacion[0]++;
      if(Testerilizacion[0]>9)
         {Testerilizacion[0]=0;
          Testerilizacion[1]++;}
       if(Testerilizacion[1]>5)
         {Testerilizacion[1]=0;
          Testerilizacion[2]++;}
         if(Testerilizacion[2]>9)
            {Testerilizacion[2]=0;
             Testerilizacion[3]++;}
            if(Testerilizacion[3]>5)
               {Testerilizacion[3]=0;}
      }       
      
    if(tiempo_secado==1)
      {
                     
      Tsecado[0]++;
      if(Tsecado[0]>9)
         {Tsecado[0]=0;
          Tsecado[1]++;}
       if(Tsecado[1]>5)
         {Tsecado[1]=0;
          Tsecado[2]++;}
         if(Tsecado[2]>9)
            {Tsecado[2]=0;
             Tsecado[3]++;}
            if(Tsecado[3]>5)
               {Tsecado[3]=0;}
      }  
   }
   }
   
void Limpia(int x1,int x2,int y){
glcd_rect(x1, y, x2, y+7, YES, OFF);            // Clear the old voltage
}

void displayT(float adc,int x,int y,int w) {
   char voltage[9];    
      sprintf(voltage, "%1.1f",adc); // Converts adc to text
      glcd_rect(x, y, x+42, y+15, YES, OFF);glcd_text57(x, y, voltage, 2, ON);
      
}
 
void displayTiempo(int adc,int x,int y,int w) {
   char voltage[9];    
      if(w<4)
      {sprintf(voltage, "%i",adc);} // Converts adc to text
      else
      {sprintf(voltage, "%u",adc);} // Converts adc to text
      if(w==0)
      {glcd_text57(x, y, voltage, 1, ON);}            // Clear the old voltage
      if(w==1)
      {glcd_rect(x, y, x+5, y+7, YES, OFF);glcd_text57(x, y, voltage, 1, ON);}            // Clear the old voltage
      if(w==2)
      {glcd_rect(x, y, x+5, y+7, YES, ON);glcd_text57(x, y, voltage, 1, OFF);}            // Clear the old voltage
      if(w==3)
      {glcd_rect(x, y, x+20, y+28, YES, OFF);glcd_text57(x, y, voltage, 2, ON);}            // Clear the old voltage
      if(w==4)
      {glcd_rect(x, y, x+16, y+7, YES, ON);glcd_text57(x, y, voltage, 1, OFF);}            // Clear the old voltage
      if(w==5)
      {glcd_rect(x, y, x+12, y+16, YES, ON);glcd_text57(x, y, voltage, 2, OFF);}            // Clear the old voltage
      
}

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(10);y=read_adc();return (y);
}


void main()
{
   // TODO: USER CODE!!
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0);
   setup_adc(ADC_CLOCK_DIV_32 );
   sensores(0);
   Testerilizacionp[0]=read_eeprom(0);Testerilizacionp[1]=read_eeprom(1);Testerilizacionp[2]=read_eeprom(2);Testerilizacionp[3]=read_eeprom(3);
   Tsecadop[0]=read_eeprom(4);Tsecadop[1]=read_eeprom(5);Tsecadop[2]=read_eeprom(6);Tsecadop[3]=read_eeprom(7);Setpoint=read_eeprom(8);
   Nivel=read_eeprom(9);Minutos=read_eeprom(20);
   float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   //float promedio[10]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   //float promedio2[10]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   usb_init();
   usb_task();
   //usb_wait_for_enumeration();
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   glcd_text57(0, 0, JP, 2, ON);            // Write the new voltage
   glcd_text57(10, 20, Autoclave, 1, ON);            // Write the new voltage
   glcd_text57(30, 40, Lts, 1, ON);            // Write the new voltage
   delay_ms(2000);
   if(UP&&RIGHT)
   {glcd_fillScreen(OFF);      //Limpia la pantalla
      delay_ms(2000);
      for(;;)
      {
         if(UP)
         {Minutos++;delay_ms(40);write_eeprom(20,Minutos);}
         if(DOWN)
         {Minutos--;delay_ms(40);write_eeprom(20,Minutos);}
         if(RIGHT)
         {reset_cpu();}
         displayT((float)Minutos,20,20,3);
      }
   }
   glcd_fillScreen(OFF);      //Limpia la pantalla
   while(true)
   {
    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
    if(Menu==0){// Menu Principal, Muestra= Inicio Proceso, Proceso y cambio de contrase�a
      glcd_rect(0, 14, 127, 0, YES, ON);            // Hace un rectangulo al rededor de la palabra Menu.
      glcd_text57(40, 0, Men, 2, OFF);            // Escribe Menu en Tama�o 2. 
        
   if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {Flecha2=Flecha2-8;Flecha=Flecha-8;Flecha1=Flecha+1;Flanco = 1;delay_ms(30);glcd_text57(0, Flecha+8, Fle, 1, OFF);}}
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {Flecha2=Flecha2+8;Flecha=Flecha+8;Flecha1=Flecha-1;Flanco2 = 1;delay_ms(30);glcd_text57(0, Flecha-8, Fle, 1, OFF);}}
         else
            {Flanco2 = 0;}
        
        if(Flecha2>n_opcionH-8)
        {Flecha2=n_opcionL;Flecha=nMenuL;nMenuL=Flecha2;}
        
        if(Flecha2<n_opcionL)
        {Flecha2=n_opcionH-8;Flecha=nmenuH-8;Flecha=Flecha2;}               
        
        
        mensajes(2,n_opcionL);
        mensajes(3,n_opcionL+8);
        //mensajes(3,n_opcionL+16);

        glcd_text57(0, Flecha, Fle, 1, ON);            // Write the new voltage        
      

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=Flecha2;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF); 
            }}
         else
            {Flanco1 = 0;}   
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==16){ //Menu de Seleccion de Proceso.
      glcd_text57(15, 0, Proc, 2, ON);            // Escribe Proceso en Tama�o 2. 
        
      if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {Flecha2=Flecha2-8;Flecha=Flecha-8;Flecha1=Flecha+1;Flanco = 1;delay_ms(30);glcd_text57(0, Flecha+8, Fle, 1, OFF);}}
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {Flecha2=Flecha2+8;Flecha=Flecha+8;Flecha1=Flecha-1;Flanco2 = 1;delay_ms(30);glcd_text57(0, Flecha-8, Fle, 1, OFF);}}
         else
            {Flanco2 = 0;}
        
        if(Flecha2>n_opcionH+16)
        {Flecha2=n_opcionL;Flecha=nMenuL;nMenuL=Flecha2;}
        
        if(Flecha2<n_opcionL)
        {Flecha2=n_opcionH+16;Flecha=nMenuH+16;Flecha=Flecha2;}               
        
        
        mensajes(7,n_opcionL);
        mensajes(6,n_opcionL+8);
        mensajes(4,n_opcionL+16);
        mensajes(5,n_opcionL+24);
        mensajes(8,n_opcionL+32);

        glcd_text57(0, Flecha, Fle, 1, ON);            // Write the new voltage        
      

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=Flecha2+1;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF); }}
         else
            {Flanco1 = 0;}   
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       


//-Sub-Menus de selecci�n de proceso.
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==17){ // Menu de Liquido      
      glcd_imagen(4); //ErlenMeyer con Liquido
      glcd_rect(63, 40, 100, 53, YES, ON);            // Hace un rectangulo al rededor de la palabra Menu.
      glcd_text57(76, 44, ok, 1, OFF);            // Escribe Menu en Tama�o 2.   
        
      mensajes(7,55);
      //glcd_text57(60, 0, DuracionL, 1, ON);            // Write the new voltage        
      glcd_text57(63, 10, TL, 1, ON);            // Write the new voltage        
      glcd_text57(63, 20, TestL, 1, ON);            // Write the new voltage        
      glcd_text57(63, 30, TsecL, 1, ON);            // Write the new voltage        
      
      if(LEFT)// Si oprime Izquier|da
       {  if(Flanco3 == 0) 
            {Flanco3 = 1;delay_ms(30);Menu=16;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);}
        }
         else
            {Flanco3 = 0;}                

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=90;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;
            estadot40=0;estadoc=0;estadoi=0;estadol=1;estadop=0;Liquid=1;Setpoint=121;}}
         else
            {Flanco1 = 0;}   
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==25){ // Menu de Instrumental Suave     
      glcd_imagen(3); //Erlen Meyer
      glcd_rect(63, 40, 100, 53, YES, ON);            // Hace un rectangulo al rededor de la palabra Menu.
      glcd_text57(76, 44, ok, 1, OFF);            // Escribe Menu en Tama�o 2.   
        
      mensajes(6,55);
      //glcd_text57(60, 0, DuracionI, 1, ON);            // Write the new voltage        
      glcd_text57(63, 10, TI, 1, ON);            // Write the new voltage        
      glcd_text57(63, 20, TestI, 1, ON);            // Write the new voltage        
      glcd_text57(63, 30, TsecI, 1, ON);            // Write the new voltage        

      if(LEFT)// Si oprime Izquier|da
       {  if(Flanco3 == 0) 
            {Flanco3 = 1;delay_ms(30);Menu=16;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);}
        }
         else
            {Flanco3 = 0;}                

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=90;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;
            estadot40=0;estadoc=0;estadoi=1;estadol=0;estadop=0;Liquid=1;Setpoint=121;}}
         else
            {Flanco1 = 0;}   
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==33){ // Menu de textil e instrumental     
      glcd_imagen(1); //Tijeras y Camisa
      glcd_rect(63, 40, 100, 53, YES, ON);            // Hace un rectangulo al rededor de la palabra Menu.
      glcd_text57(76, 44, ok, 1, OFF);            // Escribe Menu en Tama�o 2.   
        
      mensajes(4,55);
      //glcd_text57(60, 0, Duracion40, 1, ON);            // Write the new voltage        
      glcd_text57(63, 10, T40, 1, ON);            // Write the new voltage        
      glcd_text57(63, 20, Test40, 1, ON);            // Write the new voltage        
      glcd_text57(63, 30, Tsec40, 1, ON);            // Write the new voltage        
      
      if(LEFT)// Si oprime Izquier|da
       {  if(Flanco3 == 0) 
            {Flanco3 = 1;delay_ms(30);Menu=16;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);}
        }
         else
            {Flanco3 = 0;}                

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=100;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;
            estadot40=1;estadoc=0;estadoi=0;estadol=0;estadop=0;Liquid=0;glcd_text57(0, 55, Calentando, 1, ON); Setpoint=134;
            glcd_rect(25, 18, 40, 53, NO, ON);enable_interrupts(global);}}
         else
            {Flanco1 = 0;}   
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==41){ // Menu de Caucho     
      glcd_imagen(2); //Guante
      glcd_rect(63, 40, 100, 53, YES, ON);            // Hace un rectangulo al rededor de la palabra Menu.
      glcd_text57(76, 44, ok, 1, OFF);            // Escribe Menu en Tama�o 2.   
        
      mensajes(5,55);
      //glcd_text57(60, 0, DuracionC, 1, ON);            // Write the new voltage        
      glcd_text57(63, 10, TC, 1, ON);            // Write the new voltage        
      glcd_text57(63, 20, TestC, 1, ON);            // Write the new voltage        
      glcd_text57(63, 30, TsecC, 1, ON);            // Write the new voltage        
      
      if(LEFT)// Si oprime Izquier|da
       {  if(Flanco3 == 0) 
            {Flanco3 = 1;delay_ms(30);Menu=16;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);}
        }
         else
            {Flanco3 = 0;}                

      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;Menu=100;Flecha2=n_opcionL; Flecha=n_opcionL;delay_ms(30);glcd_fillScreen(OFF);n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;
            estadot40=0;estadoc=1;estadoi=0;estadol=0;estadop=0;Liquid=0;glcd_text57(0, 55, Calentando, 1, ON); Setpoint=121;
            glcd_rect(25, 18, 40, 53, NO, ON);enable_interrupts(global);}}
         else
            {Flanco1 = 0;}   
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==49){ // Menu de Proceso Personalizado
      glcd_text57(10, 0, T_esterilizacion, 1, ON);            // Escribe Menu en Tama�o 2.         
      
      if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {Flanco = 1;delay_ms(30);
            if((Flx==36)&&(Fly==8))
               {Testerilizacionp[3]++;}
            if((Flx==42)&&(Fly==8))
               {Testerilizacionp[2]++;}
            if((Flx==54)&&(Fly==8))
               {Testerilizacionp[1]++;}
            if((Flx==60)&&(Fly==8))
               {Testerilizacionp[0]++;}
            
            if((Flx==36)&&(Fly==40))
               {Tsecadop[3]++;}
            if((Flx==42)&&(Fly==40))
               {Tsecadop[2]++;}
            if((Flx==54)&&(Fly==40))
               {Tsecadop[1]++;}
            if((Flx==60)&&(Fly==40))
               {Tsecadop[0]++;}  
            if(Fly==24)   
            {unidad=1;}
            
            if(Fly==56)
            {Setpoint++;}
            }
       }
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {Flanco2 = 1;delay_ms(30);
            if((Flx==36)&&(Fly==8))
               {Testerilizacionp[3]--;}
            if((Flx==42)&&(Fly==8))
               {Testerilizacionp[2]--;}
            if((Flx==54)&&(Fly==8))
               {Testerilizacionp[1]--;}
            if((Flx==60)&&(Fly==8))
               {Testerilizacionp[0]--;}
            
            if((Flx==36)&&(Fly==40))
               {Tsecadop[3]--;}
            if((Flx==42)&&(Fly==40))
               {Tsecadop[2]--;}
            if((Flx==54)&&(Fly==40))
               {Tsecadop[1]--;}
            if((Flx==60)&&(Fly==40))
               {Tsecadop[0]--;}  
            if(Fly==24)   
               {unidad=0;}
            if(Fly==56)
            {Setpoint--;}               
            }
      }
         else
            {Flanco2 = 0;}
            
            if((Flx==36)&&(Fly==8))
               {displayTiempo(Testerilizacionp[3],36,8,2);displayTiempo(Testerilizacionp[2],42,8,1);
               displayTiempo(Testerilizacionp[1],54,8,1);displayTiempo(Testerilizacionp[0],60,8,1);}
            if((Flx==42)&&(Fly==8))
               {displayTiempo(Testerilizacionp[3],36,8,1);displayTiempo(Testerilizacionp[2],42,8,2);
               displayTiempo(Testerilizacionp[1],54,8,1);displayTiempo(Testerilizacionp[0],60,8,1);}
            if((Flx==54)&&(Fly==8))
               {displayTiempo(Testerilizacionp[3],36,8,1);displayTiempo(Testerilizacionp[2],42,8,1);
               displayTiempo(Testerilizacionp[1],54,8,2);displayTiempo(Testerilizacionp[0],60,8,1);}
            if((Flx==60)&&(Fly==8))
               {displayTiempo(Testerilizacionp[3],36,8,1);displayTiempo(Testerilizacionp[2],42,8,1);
               displayTiempo(Testerilizacionp[1],54,8,1);displayTiempo(Testerilizacionp[0],60,8,2);}  
            if((Fly!=8))
               {displayTiempo(Testerilizacionp[3],36,8,1);displayTiempo(Testerilizacionp[2],42,8,1);
               displayTiempo(Testerilizacionp[1],54,8,1);displayTiempo(Testerilizacionp[0],60,8,1);}  
            
            
      glcd_text57(15, 17, Desfogue, 1, ON);            // Escribe Menu en Tama�o 2.   
            
            if(Fly==24)
               {if(unidad==1)
                  {glcd_rect(48, 25, 48+11, 25+7, YES, ON);glcd_text57(48, 25, Si, 1, OFF);Liquid=0;}
                else
                  {glcd_rect(48, 25, 48+11, 25+7, YES, ON);glcd_text57(48, 25, Noo, 1, OFF);Liquid=1;}
               }
            else
               {if(unidad==1)
                  {glcd_rect(48, 25, 48+11, 25+7, YES, OFF);glcd_text57(48, 25, Si, 1, ON);}
                else
                  {glcd_rect(48, 25, 48+11, 25+7, YES, OFF);glcd_text57(48, 25, Noo, 1, ON);}
               }
      
      if((Flx>60)&&(Fly<40))            
         {Flx=36;Fly=Fly+16;}
      if((Flx<36)&&(Fly<=40))            
         {Flx=36;Fly=8;}   
      
      glcd_text57(48, 8, Vacio, 1, ON);
      glcd_text57(48, 42, Vacio, 1, ON);
      
      
      glcd_text57(30, 34, T_secado, 1, ON);            // Escribe Menu en Tama�o 2.   
      displayTiempo(Tsecadop[3],36,42,1);displayTiempo(Tsecadop[2],42,42,1);
      displayTiempo(Tsecadop[1],54,42,1);displayTiempo(Tsecadop[0],60,42,1);

            if(Fly==40) // Si posici�n en y esta en donde aparece lo de tiempo de secado
               {if(unidad==1) // Si se acepto el desfogue rapido.
               
                  {  if(Flx==36) // Si esta en la decena de minutos.
                        {displayTiempo(Tsecadop[3],36,42,2);displayTiempo(Tsecadop[2],42,42,1);
                        displayTiempo(Tsecadop[1],54,42,1);displayTiempo(Tsecadop[0],60,42,1);}
                        
                     if(Flx==42) // Si esta en la unidad de minutos.
                        {displayTiempo(Tsecadop[3],36,42,1);displayTiempo(Tsecadop[2],42,42,2);
                        displayTiempo(Tsecadop[1],54,42,1);displayTiempo(Tsecadop[0],60,42,1);}
                        
                     if(Flx==54) // Si esta en la decena de segundos.
                        {displayTiempo(Tsecadop[3],36,42,1);displayTiempo(Tsecadop[2],42,42,1);
                        displayTiempo(Tsecadop[1],54,42,2);displayTiempo(Tsecadop[0],60,42,1);}
                        
                     if(Flx==60) // Si esta en la decena de segundos.
                        {displayTiempo(Tsecadop[3],36,42,1);displayTiempo(Tsecadop[2],42,42,1);
                        displayTiempo(Tsecadop[1],54,42,1);displayTiempo(Tsecadop[0],60,42,2);}
                        
                     if(Flx>60) // Si paso de unidades de segundos, quiere decir que acepto lo programado y vuelve a Men� principal.  
                        {Fly=Fly+16;estadot40=0;estadoc=0;estadoi=0;estadol=0;estadop=1;} 
                        
               }
                else
                  {Fly=56;estadot40=0;estadoc=0;estadoi=0;estadol=0;estadop=1;}
               }
               
               
               if(Fly==56)
               {glcd_text57(0, 55, T, 1, ON); // Muestra la palabra Temperatura de esterilizaci�n
               glcd_text57(66, 55, Vacio, 1, ON); // 
               displayTiempo(Setpoint,72,55,4);
               glcd_circle(93,55,2,NO,ON);
               }
               if(Fly!=56)
               {  glcd_text57(0, 55, T, 1, ON); // Muestra la palabra Temperatura de esterilizaci�n
               glcd_text57(66, 55, Vacio, 1, ON); // 
               displayTiempo(Setpoint,72,55,1);
               glcd_circle(93,55,2,NO,ON);
               }
               
       if(LEFT)// Si oprime Izquier|da
       {  if(Flanco3 == 0) 
            {Flanco3 = 1;
               if(Fly==24){Fly=Fly-16;}
               else{Flx=Flx-6;}
               if((Fly==40)&&(Flx<36)){Fly=Fly-16;}
               if((Fly==8)&&(Flx<36)){Menu=0;glcd_fillScreen(OFF);Flecha2=n_opcionL; Flecha=n_opcionL;n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;}              
               if(Flx==48){Flx=42;};if(Flx==55){Flx=30;};delay_ms(30);delay_ms(30);
               if(Fly==56){Fly=8;Flx=36;};delay_ms(30);
             }
        }
         else
            {Flanco3 = 0;}                
               
        if(Testerilizacionp[0]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Testerilizacionp[0]=9;}
        if(Testerilizacionp[1]<0) // Si esta unidad es menor a 0 pasa a 5.
          {Testerilizacionp[1]=5;}
        if(Testerilizacionp[2]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Testerilizacionp[2]=9;}
        if(Testerilizacionp[3]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Testerilizacionp[3]=9;}  
          
        if(Testerilizacionp[0]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Testerilizacionp[0]=0;}
        if(Testerilizacionp[1]>5) // Si esta unidad es mayor a 5 pasa a 0.
          {Testerilizacionp[1]=0;}
        if(Testerilizacionp[2]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Testerilizacionp[2]=0;}
        if(Testerilizacionp[3]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Testerilizacionp[3]=0;}    
          
        if(Tsecadop[0]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Tsecadop[0]=9;}
        if(Tsecadop[1]<0) // Si esta unidad es menor a 0 pasa a 5.
          {Tsecadop[1]=5;}
        if(Tsecadop[2]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Tsecadop[2]=9;}
        if(Tsecadop[3]<0) // Si esta unidad es menor a 0 pasa a 9.
          {Tsecadop[3]=9;}  
          
        if(Tsecadop[0]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Tsecadop[0]=0;}
        if(Tsecadop[1]>5) // Si esta unidad es mayor a 5 pasa a 0.
          {Tsecadop[1]=0;}
        if(Tsecadop[2]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Tsecadop[2]=0;}
        if(Tsecadop[3]>9) // Si esta unidad es mayor a 9 pasa a 0.
          {Tsecadop[3]=0;}      
         
         if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;
               if(Fly==24){Fly=Fly+16;}
               else{Flx=Flx+6;}
               if(Flx==48){Flx=54;};if(Flx==30){Flx=55;};delay_ms(30);
               if(Fly==56)
               { write_eeprom(0,Testerilizacionp[0]);write_eeprom(1,Testerilizacionp[1]);write_eeprom(2,Testerilizacionp[2]);
                 write_eeprom(3,Testerilizacionp[3]);
                 write_eeprom(4,Tsecadop[0]);write_eeprom(5,Tsecadop[1]);write_eeprom(6,Tsecadop[2]);write_eeprom(7,Tsecadop[3]);
                 write_eeprom(8,Setpoint);
                 if(Liquid==1)
                 {Menu=90;glcd_fillScreen(OFF);}
                 else
                 {Menu=100;glcd_rect(25, 18, 40, 53, NO, ON);Flecha2=n_opcionL; Flecha=n_opcionL;
                 n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;glcd_fillScreen(OFF);glcd_text57(0, 55, Calentando, 1, ON);}
               }
             }
       }
         else
            {Flanco1 = 0;}
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
      if(Menu==90){ // Este es el menu de variaci�n de desfogue.
      glcd_text57(20,2,Desfogue_Lento,1,ON);
      glcd_text57(30,15,Level,2,ON);
      displayTiempo(Nivel,50,41,5);
      glcd_rect(0,0,127,29,NO,ON);
      
      if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {Flanco = 1;delay_ms(30);Nivel++;
            }
       }
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {Flanco2 = 1;delay_ms(30);Nivel--;              
            }
      }
         else
            {Flanco2 = 0;}
      if(Nivel>6)
      {Nivel=1;}
      if(Nivel<1)
      {Nivel=6;}
      
      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;
               Menu=100;glcd_rect(25, 18, 40, 53, NO, ON);enable_interrupts(global);glcd_fillScreen(OFF);Flecha2=n_opcionL; Flecha=n_opcionL;
               n_opcionH=32;n_opcionL=16;nMenuL=16;nMenuH=32;glcd_text57(0, 55, Calentando, 1, ON);write_eeprom(9,Nivel);
               glcd_rect(25, 18, 40, 53, NO, ON);Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=1;
             }
       }
         else
            {Flanco1 = 0;}
         
      }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
      while(Menu==100){ // Este es el menu de cuando el proceso se inicio.

         if(usb_enumerated())
            {
               if(usb_kbhit(1))
                  {usb_get_packet(1, rxbuf, Lenbuf);
                     if(rxbuf[0]==1)
                        {Ganancia=rxbuf[1];desvio=rxbuf[2];Tciclo=rxbuf[3];Setpoint=rxbuf[4];Minutos=rxbuf[5];Open=rxbuf[6];Ajuste=rxbuf[7];
                        Ajusten=rxbuf[10];R2=rxbuf[11];} // Falta escribir en la eeprom
                  }
               
               recbuf[0]=(int8)Prom;
               recbuf[1]=120;
               d3=Prom-(int8)Prom;
               d3=d3*10.0;
               recbuf[2]=(int8)d3;
               recbuf[3]=(int8)Pt;
               recbuf[4]=(int8)V0;
                         
             if(rxbuf[8]==1)
               {Camara_OUT_on;}
             if(rxbuf[8]==0)
               {Camara_OUT_off;}  
               
             if(rxbuf[9]==1)
               {Camara_IN_on;}
             if((rxbuf[9]==0)&&(Prom<(float)Open))
               {Camara_IN_off;}    
               
                
                usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tama�o 32bytes del EP1 al PC
             }

      if(n==0)
      {glcd_imagen(5);glcd_text57(0, 0, tiempo_est, 1, ON);glcd_text57(0, 8, tiempo_sec, 1, ON);// Muestra grafica de proceso y palabras.
       glcd_circle(95,40,2,NO,ON);
       glcd_text57(102,40, C, 2, ON);
       if(estadot40==1)// Si selecciono programa de Textil e instrumental. Ajusta tiempos de esterilizaci�n y secado.
         {Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=0;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=1;Setpoint=134;}
      
      if(estadoc==1)// Si selecciono programa de Caucho. Ajusta tiempos de esterilizaci�n y secado.
         {Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=2;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=1;minutos=0;Setpoint=121;}
      
      if(estadoi==1)// Si selecciono programa de Instrumental Suave. Ajusta tiempos de esterilizaci�n y secado.
         {Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=2;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=0;minutos=10;Setpoint=121;}
      
      if(estadol==1)// Si selecciono programa de Liquidos. Ajusta tiempos de esterilizaci�n y secado.
         {Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=2;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=0;minutos=10;Setpoint=121;}
       
      // Muestra Valor de esterilizacion programado.
      displayTiempo(Testerilizacionp[3],36,0,0);displayTiempo(Testerilizacionp[2],42,0,0);glcd_text57(48, 0, Vacio, 1, ON);
      displayTiempo(Testerilizacionp[1],54,0,0);displayTiempo(Testerilizacionp[0],60,0,0);
      // Muestra tiempo de secado programado.
      displayTiempo(Tsecadop[3],36,8,0);displayTiempo(Tsecadop[2],42,8,0);glcd_text57(48, 8, Vacio, 1, ON);
      displayTiempo(Tsecadop[1],54,8,0);displayTiempo(Tsecadop[0],60,8,0);
         
      V0 = sensores(0);
      //rPt=(V0*R1)/(1023.0-V0);Temperature=(Pt-100.0972)/0.3850;
      Pt=(R1+(float)R2)/((1023.0/V0)-1);
      Temperature=(Pt-100.1395)/0.3838;  // Ecuaci�n determinada por linealizaci�n en Matlab. Revisar este polinomio.  
      for(h=0;h<=79;h++)
            {
               promedio[h]=Temperature;
            } 
       n=1;     
      }

//-------------------------------Control de Calentamiento------------------------------------------------------------//         
         error=desvio+(float)Setpoint-Prom;      // C�lculo del error ******

         tmp=error*Ganancia;  // Control Proporcional.
         
         if(tmp>Setpoint)   // Anti Wind-UP      
            {tmp=Setpoint;}
            
         if(tmp<0.0)     // Anti Wind_DOWN    
            {tmp=0.0;}   
      
         Tpulso=(tmp/Setpoint)*Tciclo;  // Tpulso(t)= Tciclo*(y(t)-ymin)/(ymax - ymin); calculo de ciclo util para control de resistencia.ymax=140.ymin=0;
                                   // Tpulso es float, hay que pasar ese ciclo a una salida de un puerto usando el TMR1.
         
         if(tiempoctrl<=(int16)(Tpulso/0.005)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
         {Resistencia_on;}
         else
         {Resistencia_off;}
         
         if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
         {tiempoctrl=0;} // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
//--------------------------------------------------------------------------------------------------------------------//         
            
         if(Lectura==1)
         {         
            V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
            Pt=(R1+(float)R2)/((1023.0/V0)-1);
//            Temperature=(Pt-100.0972)/0.3850;  // Ecuaci�n determinada por linealizaci�n en Matlab. Revisar este polinomio.  
            Temperature=(Pt-100.1395)/0.3838;  // Ecuaci�n determinada por linealizaci�n en Matlab. Revisar este polinomio.  
            
            if(l>=80)
            {l=0;}
            promedio[l]=Temperature;l++;
         
            for(h=0;h<=79;h++)
            {
               Prom+=promedio[h];
            } 
               Prom=Prom/80;   
               Prom+=((float)Ajuste-(float)Ajusten);
             
            Lectura=0;
            
         displayT(Prom,79,20,3);
         displayTiempo(Setpoint,8,20,4);
         displayTiempo((int8)Tpulso,8,40,4);
         
         // Muestra Valor de esterilizacion actual.
         displayTiempo(Testerilizacion[3],80,0,1);displayTiempo(Testerilizacion[2],86,0,1);glcd_text57(92, 0, Vacio, 1, ON);
         displayTiempo(Testerilizacion[1],98,0,1);displayTiempo(Testerilizacion[0],104,0,1);
         // Muestra tiempo de secado actual.
         displayTiempo(Tsecado[3],80,8,1);displayTiempo(Tsecado[2],86,8,1);glcd_text57(92, 8, Vacio, 1, ON);
         displayTiempo(Tsecado[1],98,8,1);displayTiempo(Tsecado[0],104,8,1);
         }
         
      
         if(Prom>=((float)Setpoint-1.0))// Si activa alarma de esterilizaci�n. Inicia tiempo de esterilizaci�n.
            {
            if(tiempo_secado==0)
               {glcd_text57(0, 56, Esterilizando, 1, ON);tiempo_esterilizacion=1;
               if(est==0)
                  {Limpia(0,90,56);glcd_rect(25, 18, 40, 53, NO, OFF);glcd_rect(40, 18, 75, 26, NO, ON);est=1;}
               }
             }
          
          if((Prom>=(float)Open)&&(Prom<((float)Setpoint-1.0)))
             {Camara_IN_on;
               if(Expulsa==1)
                 {Camara_OUT_on;delay_ms(250);Camara_OUT_off;Expulsa=0;}
             }
         
      
      if((tiempo_secado==1)&&(Liquid==0)) //Si no selecciono un programa con desfogue lento, Hacer desfogue total.
      {tiempo_esterilizacion=0;
         if(i==0)
            {Limpia(0,90,56);glcd_rect(40, 18, 75, 26, NO, OFF);glcd_rect(65, 18, 78, 53, NO, ON);i=1;} 
         Camara_OUT_on;glcd_text57(0, 56, Secando, 1, ON);
      }
     
      if(tiempo_secado==0) //Si esta calentando, debe tener la electrovalvula cerrada.
      {Camara_OUT_off;}
            
      // Si cumplio el tiempo de esterilizaci�n, verifique si existe tiempo de secado.
      if((Testerilizacion[0]==Testerilizacionp[0])&&(Testerilizacion[1]==Testerilizacionp[1])&&(Testerilizacion[2]==Testerilizacionp[2])&&(Testerilizacion[3]==Testerilizacionp[3]))
      {tiempo_esterilizacion=0;tiempo_secado=1;
      
      // Si hay desfogue lento, muestra desfogue lento y hace rampa. Al finalizar la rampa hace fin de ciclo.
      if(Liquid==1)   
         {tiempo_secado=0;
            if(i==0)
              {Limpia(0,90,56);glcd_rect(40, 18, 75, 26, NO, OFF);glcd_rect(65, 18, 78, 53, NO, ON);i=1;
              glcd_text57(0, 56, Desfogue_Lento, 1, ON);}
           
         for(m=0;m<Minutos*15;m++)// Utilizado para realizar la rampa.
         {Resistencia_off;Camara_IN_off;Camara_OUT_on;delay_ms(100*Nivel);Camara_OUT_off;delay_ms(2000-(100*Nivel));}     
         fin_ciclo=1;
         }
      while(fin_ciclo==1) //Si termino proceso.
      {Camara_OUT_off;Camara_IN_off;glcd_fillScreen(OFF); glcd_text57(20, 0, Proc, 2, ON);glcd_text57(10, 40, Finalizado, 2, ON);delay_ms(30000);}   
      }
     
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      
   }
}
