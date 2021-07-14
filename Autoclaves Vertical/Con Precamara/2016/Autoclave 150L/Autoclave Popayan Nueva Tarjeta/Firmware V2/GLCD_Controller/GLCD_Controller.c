// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave 80Lts.
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// Ing. Faiver Humberto Trujillo.
// JP Inglobal. 2014

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR// PLL1 para 4 MHz
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL3,CPUDIV4,VREGEN,NOMCLR// PLL1 para 12 MHz
#use delay(clock=48000000)
#use RS232(BAUD=9600, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definición de teclado - NC
#define   UP            input(PIN_A5)
#define   DOWN          input(PIN_E0)
#define   RIGHT         input(PIN_E1)
#define   LEFT          input(PIN_E2)

//signed  int8 clave[4]={0,0,0,0};
//int8 contrasena[4]={0,0,0,0};   // 0=Verdadero, 1=Falso
char kpa[]="kPa";
float promedio[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
int8 Puerta=0,Parada=1,NivelReservorio=0,NivelTanque=0,Presostato=0,h=0,l=0,Ajuste=0,Ajusten=0,R2=0,Alarma=0,AlarmaAgua=0;
short Flanco=0,Flanco1=0,Flanco2=0,emergency=0,visual=1,desfoguelento=0,Lectura=0,Expulsa=0, est=0,Visua=0,flag_agua=0,CadenaRecibida=0;
short disp_Calentando=0,disp_Llenando=0,disp_Secando=0,disp_Esterilizando=0,disp_Despresurizando=0,disp_Stop=0,disp_Door=0,disp_Fin=0,z=0;
short disp_Alarma=0,disp_AlarmaAgua=0;
int8 Resistencia=0,EVDesfogue=0,EVAgua=0,EVTrampa=0,MotorPuerta=0,EVEntradaAgua=0,BombaVacio=0,BombaAgua=0,Control=0,Decimal=0;
int8 Menu=0, unidad=1,bite=0,TamanoCadena=0,i=0;
int8 Setpoint=0,fin_ciclo=0,Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0,Exhaust=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Minutos=15,Nivel=0,tiempo_desfogue=0;
float ADC_Pt100=0.0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
char dato[10];
char datoTx[14];
char datoRx[11];
char ADCTemperatura[4];
char SetTemperatura[3]={0,0,0};
float V0=0.0,R1=6180.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=0.0,d3=0.0;

typedef struct{
   char Letra[30];
}MENUU;

MENUU Menus;

void displayMenu(char palabra[30], int x, int y, int sombreado, int tamano)
{
   strcpy(Menus.Letra,palabra);
   if(sombreado==2)
      {glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   if(sombreado==1)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);}
   if(sombreado==0)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   glcd_update();
}

void displayTiempo(int digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==2)
   {
      sprintf(voltage, "%03u", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }
   else
   {
      sprintf(voltage, "%u", digito);
   
      if(sombreado==1)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, voltage, tamano, OFF);}
      if(sombreado==0)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, voltage, tamano, ON);}
   }
   glcd_update();
}

#int_rda
void rda_isr(void)//función de interrupción por recepción de datos USART 
{ 
   dato[bite]=getch();
   if((bite<10 && strcmp(dato[bite],'\r')==0))
   {
      bite=0;
   }
   else
   {
      if(bite==10 && strcmp(dato[bite],'\r')==0)
      {
         TamanoCadena=bite;
         bite=0;
         CadenaRecibida=1;
         for(i=0;i<TamanoCadena+1;i++)
         {
            datoRx[i]=dato[i];
         }   
      }
      else
      {
         if(bite>10)
         {
            bite=0;
         }
         else
         {
            //displayTiempo(bite,50,50,2,1);
            CadenaRecibida=0;
            bite++;
         }
      }
   }
}

void EnviarCodigo(void)
{
   datoTx[0]=Resistencia+48;
   datoTx[1]=EVDesfogue+48;
   datoTx[2]=EVAgua+48;
   datoTx[3]=EVTrampa+48;
   datoTx[4]=MotorPuerta+48;
   datoTx[5]=EVEntradaAgua+48;
   datoTx[6]=BombaVacio+48;
   datoTx[7]=BombaAgua+48;
   datoTx[8]=Control+48;
   datoTx[12]=Nivel+48;
   datoTx[9]=Setpoint+48;
   /*itoa(Setpoint,10, SetTemperatura);
   
   for(i=0;i<3;i++)
   {
      if(SetTemperatura[i]==null)
      {
         datoTx[i+9]='n';
      }
      else
      {
         datoTx[i+9]=SetTemperatura[i];
      }
   }*/
   datoTx[13]='\r';
      
      
   for(i=0;i<14;i++)
   {
      putc(datoTx[i]);
      delay_ms(12);
   }   
}

void Determina(void){
         Puerta=DatoRx[0]-48;
         Parada=DatoRx[1]-48;
         NivelReservorio=DatoRx[2]-48;
         NivelTanque=DatoRx[3]-48;
         Presostato=DatoRx[4]-48;
         ADC_Pt100=DatoRx[5]-48;
         Alarma=DatoRx[6]-48;
         AlarmaAgua=DatoRx[7]-48;
         /*for(i=0;i<4;i++)
         {
            if(strcmp(datoRx[i+5],'n')==0 )
            {
               break;
            }
            else
            {
               ADCTemperatura[i]=datoRx[i+5];
            }
         }
         ADC_Pt100=atof(ADCTemperatura);
         for(i=0;i<4;i++)
         {
            datoRx[i+5]=null;
            ADCTemperatura[i]=null;
         }*/
         Decimal=DatoRx[9]-48;
         ADC_Pt100=ADC_Pt100+((float)Decimal/10);
}

void glcd_imagen(int8 x)
{
   char i,j;
   signed char k; 
   /*
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
   }*/
   
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

#int_TIMER1
void temp1s(void){
   //set_timer1(45612);  // 5 ms 5536 , Posiblemente cargar con 22144 debido a F=12MHz
   set_timer1(5536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
      
   if(tiempos3>=40 && Exhaust==2)
   {tiempos3=0;Expulsa=1;tiempo_desfogue++;}
   
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
   glcd_rect(x1, y, x2, y+7, YES, OFF);
}

void displayT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%3.1f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*8), YES, OFF);
   /*glcd_rect(x, y, x+53, y+15, YES, ON);*/glcd_text57(x, y, voltage, w, ON);
}



// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(10);y=read_adc();return (y);
}

void MuestraTiemposProgramados(){
// Muestra Valor de esterilizacion programado.
   displayTiempo(Testerilizacionp[3],36,0,0,1);displayTiempo(Testerilizacionp[2],42,0,0,1);
   strcpy(Menus.Letra,":");displayMenu(Menus.Letra,49,0,2,1); 
   displayTiempo(Testerilizacionp[1],54,0,0,1);displayTiempo(Testerilizacionp[0],60,0,0,1);
// Muestra tiempo de secado programado.
   displayTiempo(Tsecadop[3],36,8,0,1);displayTiempo(Tsecadop[2],42,8,0,1);
   strcpy(Menus.Letra,":");displayMenu(Menus.Letra,49,8,2,1); 
   displayTiempo(Tsecadop[1],54,8,0,1);displayTiempo(Tsecadop[0],60,8,0,1);
}

void Leer_Sensor(int media)
{
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V0 = V0/39.40;
   Pt=(R1+(float)R2)/((1023.0/V0)-1.0);
   Temperature=(Pt-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
            
   if(l>media-1)
   {l=0;}
   promedio[l]=Temperature;l++;
         
   for(h=0;h<=(media-1);h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   Prom+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
}

void displayContrasena(int digito, int x, int y, int sombreado)
{
   char voltage[9];
   sprintf(voltage, "%i", digito);
   
   if(sombreado==1)
      {glcd_rect(x, y, x+18, y+24, YES, ON);glcd_text57(x+2, y+1, voltage, 2, OFF);}
   else   
      {glcd_rect(x, y, x+18, y+24, NO, OFF);glcd_text57(x+2, y+1, voltage, 2, ON);}

}

void main()
{
   enable_interrupts(INT_RDA);      //Habilita interrupción por recepción RS23   
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN3);
   setup_adc(ADC_CLOCK_DIV_32 );
   sensores(0);
   output_d(0);   
   Testerilizacionp[0]=read_eeprom(0);Testerilizacionp[1]=read_eeprom(1);Testerilizacionp[2]=read_eeprom(2);Testerilizacionp[3]=read_eeprom(3);
   Tsecadop[0]=read_eeprom(4);Tsecadop[1]=read_eeprom(5);Tsecadop[2]=read_eeprom(6);Tsecadop[3]=read_eeprom(7);Setpoint=read_eeprom(8);
   Nivel=read_eeprom(9);
   Password[0]=read_eeprom(10);Password[1]=read_eeprom(11);Password[2]=read_eeprom(12);Password[3]=read_eeprom(13);
   enable_interrupts(global);
   //Ganancia=read_eeprom(10);desvio=read_eeprom(11);Tciclo=read_eeprom(12);Ajuste=read_eeprom(13);Ajusten=read_eeprom(14);
   //Ganancia2=read_eeprom(15);Minutos=read_eeprom(20); 
   
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   strcpy(Menus.Letra,"JP Inglobal");
   displayMenu(Menus.Letra,0,0,0,2);     
   strcpy(Menus.Letra,"Autoclave Horizontal");
   displayMenu(Menus.Letra,0,20,0,1);       
   strcpy(Menus.Letra," 110 Litros  ");
   displayMenu(Menus.Letra,0,35,0,2);
   delay_ms(1000);
   delay_ms(1000);
   
   glcd_fillScreen(OFF);      //Limpia la pantalla
   displayContrasena(Contrasena[0],20,30,1);
   displayContrasena(Contrasena[1],40,30,0);
   displayContrasena(Contrasena[2],60,30,0);
   displayContrasena(Contrasena[3],80,30,0);
   EnviarCodigo();
   delay_ms(500);
   Determina();
   delay_ms(500);
   Parada=1;
   while(true)
   {
      glcd_update();
      if(CadenaRecibida==1)
      {
         Determina();
         EnviarCodigo();
         CadenaRecibida=0;
      }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==0){ //Menu de Contraseña.
      
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9)   
                  {
                     Contrasena[i-1]=0;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }
               else
               {
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                   displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }      
            }     
      }
         else
            {Flanco2 = 0;}  
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(30);unidad++;
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }
            }
      }
         else
            {Flanco1 = 0;}  
    
      if(unidad>4)
      {glcd_fillScreen(OFF);unidad=4;
         if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2) // Si Ingresa clave para reset general del sistema.
            {write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
             write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
             reset_cpu();}        
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicación del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=0;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            displayContrasena(Contrasena[0],20,30,1);
            displayContrasena(Contrasena[1],40,30,0);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       


//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==1){ //Menu Principal.      
      
      if(visual==1)
      {
         if(Opcion>6)
         {Opcion=1;}
         if(Opcion<1)
         {Opcion=6;}
         
         strcpy(Menus.Letra,"Menu Principal");
         displayMenu(Menus.Letra,30,0,1,1);
      
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Preciclo");
            displayMenu(Menus.Letra,0,10,1,1);}
         else
            {strcpy(Menus.Letra,"Preciclo");
            displayMenu(Menus.Letra,0,10,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Liquidos");
            displayMenu(Menus.Letra,0,19,1,1);}
         else
            {strcpy(Menus.Letra,"Liquidos");
            displayMenu(Menus.Letra,0,19,0,1);}   
            
          if(Opcion==3)   
            {strcpy(Menus.Letra,"Instrumental Suave");
            displayMenu(Menus.Letra,0,28,1,1);}
         else
            {strcpy(Menus.Letra,"Instrumental Suave");
            displayMenu(Menus.Letra,0,28,0,1);}
            
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Textil e Instrumental");
            displayMenu(Menus.Letra,0,37,1,1);}
         else
            {strcpy(Menus.Letra,"Textil e Instrumental");
            displayMenu(Menus.Letra,0,37,0,1);}   
            
         if(Opcion==5)   
            {strcpy(Menus.Letra,"Personalizado");
            displayMenu(Menus.Letra,0,46,1,1);}
         else
            {strcpy(Menus.Letra,"Personalizado");
            displayMenu(Menus.Letra,0,46,0,1);}  
            
         if(Opcion==6)   
            {strcpy(Menus.Letra,"Cambio Clave");
            displayMenu(Menus.Letra,0,55,1,1);}
         else
            {strcpy(Menus.Letra,"Cambio Clave");
            displayMenu(Menus.Letra,0,55,0,1);}     
            visual=0;
      }
      if(UP)//Si oprime hacia arriba
         {  if(Flanco == 0) 
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {  if(Flanco2 == 0) 
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
       if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(30);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
         /*      
         if(LEFT)//Si oprime boton de Toma.
         {delay_ms(700);Menu=2;glcd_fillscreen(OFF);}*/
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
//-Sub-Menus de selección de proceso.

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==4){ // Menu de Pre-ciclo
      
      if(visual==1)
      {
         glcd_imagen(2); //Guante
         strcpy(Menus.Letra,"Preciclo");
         displayMenu(Menus.Letra,20,0,1,2);
        
         strcpy(Menus.Letra,"Preciclo");
         displayMenu(Menus.Letra,7,55,0,1);
         
         strcpy(Menus.Letra,"Temperatura:105 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:5m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 5m");
         displayMenu(Menus.Letra,0,46,0,1);
         visual=0;
      }
      
      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=0;visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=0;minutos=0;Setpoint=105;Menu=20;
         Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;enable_interrupts(global);glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Calentando");displayMenu(Menus.Letra,0,55,2,1);
         glcd_rect(15, 18, 30, 53, NO, ON);
         glcd_imagen(5);
         strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
         strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
         glcd_circle(95,40,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
         MuestraTiemposProgramados();
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==5){ // Menu de Liquido      
      
      if(visual==1)
      {
         glcd_imagen(4); //ErlenMeyer con Liquido
         strcpy(Menus.Letra,"Liquidos");
         displayMenu(Menus.Letra,20,0,1,2);
      
         strcpy(Menus.Letra,"Liquidos");
         displayMenu(Menus.Letra,7,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:121 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:20m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= N/A");
         displayMenu(Menus.Letra,0,46,0,1);    
         visual=0;
      }   
         
      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }               

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=2;desfoguelento=1;visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=0;Setpoint=121;Menu=14;delay_ms(700);glcd_fillScreen(OFF);    
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==6){ // Menu de Instrumental Suave     
      
      if(visual==1)
      {
         glcd_imagen(3); //Erlen Meyer
         
         strcpy(Menus.Letra,"Ins. Suave");
         displayMenu(Menus.Letra,0,0,1,2);
         
         strcpy(Menus.Letra,"Instrumental Suave");
         displayMenu(Menus.Letra,7,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:121 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:15m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= N/A");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }            

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=1;desfoguelento=1;visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=0;Setpoint=121;Menu=14;delay_ms(700);glcd_fillScreen(OFF);    
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==7){ // Menu de textil e instrumental     
      
      if(visual==1)
      {
         glcd_imagen(1); //Tijeras y Camisa
         strcpy(Menus.Letra,"Tex & Ins");
         displayMenu(Menus.Letra,0,0,1,2);
      
         strcpy(Menus.Letra,"Textil e Instrumental");
         displayMenu(Menus.Letra,0,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:121 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:25m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 10m");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }                 

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=2;visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=1;Setpoint=121;Menu=20;delay_ms(700);
         Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;enable_interrupts(global);glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Calentando");
         displayMenu(Menus.Letra,0,55,2,1);
         glcd_rect(15, 18, 30, 53, NO, ON);
         glcd_imagen(5);
         strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
         strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
         glcd_circle(95,40,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
         MuestraTiemposProgramados();
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
// Testerilizacionp[3]<,Tsecadop[3]<,Setpoint

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==8){ // Menu de Proceso Personalizado
        
         if(visual==1)
         {
            if(Opcion>5)
            {Opcion=1;}
            if(Opcion<1)
            {Opcion=5;}
            
            strcpy(Menus.Letra,"Proceso Personalizado");
            displayMenu(Menus.Letra,0,0,1,1);
      
            if(Opcion==1)   
               {strcpy(Menus.Letra,"T. Esterilizacion");
               displayMenu(Menus.Letra,0,10,1,1);}
            else
               {strcpy(Menus.Letra,"T. Esterilizacion");
               displayMenu(Menus.Letra,0,10,0,1);}
            
            if(Opcion==2)   
               {strcpy(Menus.Letra,"T. Secado");
               displayMenu(Menus.Letra,0,20,1,1);}
            else
               {strcpy(Menus.Letra,"T. Secado");
               displayMenu(Menus.Letra,0,20,0,1);}   
               
             if(Opcion==3)   
               {strcpy(Menus.Letra,"Temperatura");
               displayMenu(Menus.Letra,0,30,1,1);}
            else
               {strcpy(Menus.Letra,"Temperatura");
               displayMenu(Menus.Letra,0,30,0,1);}
               
            if(Opcion==4)   
               {strcpy(Menus.Letra,"Nivel de desfogue");
               displayMenu(Menus.Letra,0,40,1,1);}
            else
               {strcpy(Menus.Letra,"Nivel de desfogue");
               displayMenu(Menus.Letra,0,40,0,1);}   
               
            if(Opcion==5)   
               {strcpy(Menus.Letra,"Continuar");
               displayMenu(Menus.Letra,0,50,1,1);}
            else
               {strcpy(Menus.Letra,"Continuar");
               displayMenu(Menus.Letra,0,50,0,1);}  
                         
            visual=0;
         }
         if(UP)//Si oprime hacia arriba
         {  if(Flanco == 0) 
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {  if(Flanco2 == 0) 
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
         if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(300);Menu=Opcion+10;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
               
         if(LEFT)//Si oprime boton de Toma.
         {delay_ms(700);Menu=1;glcd_fillscreen(OFF);visual=1;}
         
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==9){ // Menu de Cambio de Contraseña
        
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Contrasena[0],20,30,1);
         displayContrasena(Contrasena[1],40,30,0);
         displayContrasena(Contrasena[2],60,30,0);
         displayContrasena(Contrasena[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Contrasena[0],20,30,0);
            displayContrasena(Contrasena[1],40,30,1);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Contrasena[0],20,30,0);
               displayContrasena(Contrasena[1],40,30,0);
               displayContrasena(Contrasena[2],60,30,1);
               displayContrasena(Contrasena[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Contrasena[0],20,30,0);
                  displayContrasena(Contrasena[1],40,30,0);
                  displayContrasena(Contrasena[2],60,30,0);
                  displayContrasena(Contrasena[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9)   
                  {
                     Contrasena[i-1]=0;
                  }
               }
            }
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                  }
               }      
            }     
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {glcd_fillScreen(OFF);
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=91;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicación del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
         }
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==91){ // Menu de Cambio de Contraseña
        
      strcpy(Menus.Letra,"Clave Nueva");
      displayMenu(Menus.Letra,0,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Password[0],20,30,1);
         displayContrasena(Password[1],40,30,0);
         displayContrasena(Password[2],60,30,0);
         displayContrasena(Password[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Password[0],20,30,0);
            displayContrasena(Password[1],40,30,1);
            displayContrasena(Password[2],60,30,0);
            displayContrasena(Password[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Password[0],20,30,0);
               displayContrasena(Password[1],40,30,0);
               displayContrasena(Password[2],60,30,1);
               displayContrasena(Password[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Password[0],20,30,0);
                  displayContrasena(Password[1],40,30,0);
                  displayContrasena(Password[2],60,30,0);
                  displayContrasena(Password[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Password[i-1]++;
                  if(Password[i-1]>9)   
                  {
                     Password[i-1]=0;
                  }
               }
            }
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {
                     Password[i-1]--;
                     if(Password[i-1]<0)
                     {
                        Password[i-1]=9;
                     }
                  }
               }      
            }     
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {
         Opcion=1;unidad=1;
         write_eeprom(10,Password[0]);write_eeprom(11,Password[1]);write_eeprom(12,Password[2]);write_eeprom(13,Password[3]);
         delay_ms(1000);glcd_fillScreen(OFF);Menu=1;
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==11){ // Menu de Tiempo de Esterilización
     
      if(visual==1)
      {
         if(unidad<1)
         {unidad=4;}
         
         strcpy(Menus.Letra,"T. Ester.");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(unidad==4)
         {displayTiempo(Testerilizacionp[3],20,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[3],20,40,0,2);}
         
         if(unidad==3)
         {displayTiempo(Testerilizacionp[2],40,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[2],40,40,0,2);}
         
         if(unidad==2)
         {displayTiempo(Testerilizacionp[1],70,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[1],70,40,0,2);}
         
         if(unidad==1)
         {displayTiempo(Testerilizacionp[0],90,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[0],90,40,0,2);}
         
         strcpy(Menus.Letra,":");
         displayMenu(Menus.Letra,60,42,2,2);     
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Testerilizacionp[unidad-1]<9)
            {
               delay_ms(30);Testerilizacionp[unidad-1]++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Testerilizacionp[unidad-1]>0)
            {
               delay_ms(30);Testerilizacionp[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(0,Testerilizacionp[0]);write_eeprom(1,Testerilizacionp[1]);
            write_eeprom(2,Testerilizacionp[2]);write_eeprom(3,Testerilizacionp[3]);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==12){ // Menu de Tiempo de Secado
     
      if(visual==1)
      {
         if(unidad<1)
         {unidad=4;}
         
         strcpy(Menus.Letra," Secado");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(unidad==4)
         {displayTiempo(Tsecadop[3],20,40,1,2);}
         else
         {displayTiempo(Tsecadop[3],20,40,0,2);}
         
         if(unidad==3)
         {displayTiempo(Tsecadop[2],40,40,1,2);}
         else
         {displayTiempo(Tsecadop[2],40,40,0,2);}
         
         if(unidad==2)
         {displayTiempo(Tsecadop[1],70,40,1,2);}
         else
         {displayTiempo(Tsecadop[1],70,40,0,2);}
         
         if(unidad==1)
         {displayTiempo(Tsecadop[0],90,40,1,2);}
         else
         {displayTiempo(Tsecadop[0],90,40,0,2);}
         
         strcpy(Menus.Letra,":");
         displayMenu(Menus.Letra,60,42,2,2); 
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Tsecadop[unidad-1]<9)
            {
               delay_ms(30);Tsecadop[unidad-1]++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Tsecadop[unidad-1]>0)
            {
               delay_ms(30);Tsecadop[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(4,Tsecadop[0]);write_eeprom(5,Tsecadop[1]);
            write_eeprom(6,Tsecadop[2]);write_eeprom(7,Tsecadop[3]);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==13){ // Menu de Temperatura
     
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTiempo(Setpoint,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Setpoint<134)
            {
               if(Setpoint<100)
               {
                  Setpoint=100;delay_ms(100);visual=1;
               }
               else
               {
                  delay_ms(30);Setpoint++;visual=1;
               }
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Setpoint>105)
            {
               delay_ms(30);Setpoint--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(8,Setpoint);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==14){ // Menu de Desfogue
     
      if(visual==1)
      {         
         strcpy(Menus.Letra," Desfogue");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTiempo(Nivel,50,40,0,2);
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Nivel<6)
            {
               delay_ms(30);Nivel++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Nivel>1)
            {
               delay_ms(30);Nivel--;visual=1;     
            }
         }
         
         if(desfoguelento==1)
         {
            if(RIGHT)//Si oprime SET
            {  
               write_eeprom(9,Nivel);
               Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;enable_interrupts(global);glcd_fillScreen(OFF);
               strcpy(Menus.Letra,"Calentando");
               displayMenu(Menus.Letra,0,55,2,1);
               glcd_rect(15, 18, 30, 53, NO, ON);
               glcd_imagen(5);
               strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
               strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
               glcd_circle(95,40,2,NO,ON);
               strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
               MuestraTiemposProgramados();
               EnviarCodigo();
            }      
         }
         else
         {
            if(LEFT)//Si oprime boton de Toma.
            {
               if(Nivel<6)
               {
                  desfoguelento=1;
               }
               else
               {
                  desfoguelento=0;
               }
               delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
               write_eeprom(9,Nivel);
            }
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==15){ //Menu de Continuar
      
      if(visual==1)
      {
         strcpy(Menus.Letra,"Continuar");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(Opcion==2)
         {strcpy(Menus.Letra,"SI");
         displayMenu(Menus.Letra,50,25,1,2);
         strcpy(Menus.Letra,"NO");
         displayMenu(Menus.Letra,50,45,0,2);
         }
         if(Opcion==1)
         {strcpy(Menus.Letra,"SI");
         displayMenu(Menus.Letra,50,25,0,2);
         strcpy(Menus.Letra,"NO");
         displayMenu(Menus.Letra,50,45,1,2);
         }
         visual=0;
      }
      
        
      if(UP)//Si oprime hacia arriba
      {
         if(Opcion<2)
         {
            Opcion++;
         }
         visual=1;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Opcion>1)
         {
            Opcion--;
         }  
         visual=1;
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==2)
         {
            Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;enable_interrupts(global);glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Calentando");
            displayMenu(Menus.Letra,0,55,2,1);
            glcd_rect(15, 18, 30, 53, NO, ON);
            glcd_imagen(5);
            strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
            strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
            MuestraTiemposProgramados();
            if(Nivel<6)
               {
                  desfoguelento=1;
               }
               else
               {
                  desfoguelento=0;
               }
               EnviarCodigo();
         }
         else
         {
            delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
         }
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
      }
    
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   if(Menu==20){ // Este es el menu de cuando el proceso se inicio.
   if(flag_agua==1 && Puerta==1 && Parada==1 && tiempo_esterilizacion<2 && Alarma==0 && AlarmaAgua==0)
   {  
      // Debe enviar a Hardware_Controller flag que indique que puede realizar control. En Hardware_Controller si esta activado el flag**
      // de control debe tener automaticamente la electrovalvula de desfogue cerrada.**
      if(tiempo_esterilizacion==0 && disp_Calentando==0)
      {
         glcd_rect(0, 55, 127, 63, YES, OFF);
         strcpy(Menus.Letra,"Calentando");displayMenu(Menus.Letra,0,55,2,1);
         disp_Calentando=1;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0;
         EVDesfogue=0;Control=1;
      }
      if(tiempo_esterilizacion==1 && disp_Esterilizando==0)
      {
         glcd_rect(0, 55, 127, 63, YES, OFF);
         strcpy(Menus.Letra,"Esterilizando");displayMenu(Menus.Letra,0,55,2,1);
         disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=1;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0;
         EVDesfogue=0;EVTrampa=0;Control=1;
      }
      Control=1;
      EnviarCodigo();
   }
   else
   {
      Control=0;
      if(Alarma>0)
      {
         if(Alarma==1) // Alarma de Error de Tiempo: Error por exceso de tiempo de calentamiento.
         {
            if(disp_Alarma==0)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Error:ET01");displayMenu(Menus.Letra,0,55,2,1);
               disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0; 
               disp_Alarma=1;
            }
         }
         
         if(Alarma==2) // Alarma de Error de Tiempo: Error por Cambio brusco en la Temperatura
         {
            tiempo_esterilizacion=0;tiempo_secado=0;
            if(disp_Alarma==0)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Error:ET02");displayMenu(Menus.Letra,0,55,2,1);
               disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0; 
               disp_Alarma=1;
            }
         }
      }
      else
      {
         disp_Alarma=0;
         if(NivelTanque==1 && flag_agua==0 )
         {
            flag_agua=1;
            // Debe enviar a Hardware_Controller que cierre EV de agua
         } 
         else
         {
            if(tiempo_esterilizacion==0 && disp_Llenando==0 && Puerta==1)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Llenando");displayMenu(Menus.Letra,0,55,2,1);
               disp_Calentando=0;disp_Llenando=1;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0;
               // Debe enviar a Hardware_Controller que abra EV de agua y que apague Resistencia
               Control=0;
            }
            
            if(AlarmaAgua==1)
            {
               if(disp_AlarmaAgua==0)
               {
                  glcd_rect(0, 55, 127, 63, YES, OFF);
                  strcpy(Menus.Letra,"Error:EA01");displayMenu(Menus.Letra,0,55,2,1);
                  disp_Calentando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0;
                  disp_AlarmaAgua=1;
               }
            }
         }
         
         if(tiempo_esterilizacion==2 && flag_agua==1)
         {
            // Si hay desfogue lento, muestra desfogue lento y hace rampa. Al finalizar la rampa hace fin de ciclo.
            if(desfoguelento==1)   
            {
               if(z==0)
               {
                  Limpia(0,90,56);glcd_rect(25, 18, 60, 26, NO, OFF);glcd_rect(55, 18, 68, 53, NO, ON);z=1;
               }
               if(disp_Despresurizando==0)
               {
                  glcd_rect(0, 55, 127, 63, YES, OFF);
                  strcpy(Menus.Letra,"Despresurizando");displayMenu(Menus.Letra,0,55,2,1);
                  disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=1;disp_Stop=0;disp_Door=0;disp_Fin=0;
                  glcd_imagen(5);
               }
               Exhaust=2;tiempo_secado=2;
            }
            else
            {
               if(z==0)
               {
                  Limpia(0,90,56);glcd_rect(25, 18, 60, 26, NO, OFF);glcd_rect(55, 18, 68, 53, NO, ON);z=1;
                  //tiempo_secado=1;
               }
               if(disp_Secando==0)
               {
                  glcd_rect(0, 55, 127, 63, YES, OFF);
                  strcpy(Menus.Letra,"Secando");displayMenu(Menus.Letra,0,55,2,1);
                  disp_Calentando=0;disp_Llenando=0;disp_Secando=1;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=0;disp_Fin=0;
                  glcd_imagen(5);
               }
               Exhaust=1;
            }
            // Debe enviar a Hardware_Controller que apague Resistencia
         }
         
         if(Puerta==0)
         {
            if(disp_Door==0)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Puerta Abierta");displayMenu(Menus.Letra,0,55,2,1);
               disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=0;disp_Door=1;disp_Fin=0;
            }   
            // Debe enviar a Hardware_Controller que apague Resistencia
         }
      }
      EnviarCodigo();
   }

   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      //displayTiempo(NivelTanque,40,30,2,1); 
      //displayTiempo(Presostato,40,40,2,1); 
      //displayTiempo(ADCTemperatura[2],20,30,2,1); 
      //displayT(Prom,20,30,2);
      displayTiempo(Presostato,31,40,2,1);
      glcd_rect(51, 40, 62, 48, YES, OFF);glcd_text57(52, 40, kpa, 1, ON);
      displayT(ADC_Pt100,69,20,2);
      glcd_circle(95,40,2,NO,ON);
      strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
      //displayT(Pt,1,20,3);
      displayTiempo(Setpoint,5,20,2,1);
        
      if(Visua==1)// Si se activo la visualización desde software
      {
         displayTiempo((int8)desvio,5,30,4,1);
         displayTiempo((int8)Tpulso,5,40,4,1);
         displayTiempo((int8)Ganancia,30,40,4,1);
         displayTiempo((int8)Ajuste,30,30,4,1);
         displayTiempo((int8)Ajusten,30,20,4,1);
         displayTiempo((int8)Ganancia2,50,40,4,1);
      }
         
      // Muestra Valor de esterilizacion actual.
      displayTiempo(Testerilizacion[3],80,0,0,1);displayTiempo(Testerilizacion[2],86,0,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,0,2,1); 
      displayTiempo(Testerilizacion[1],98,0,0,1);displayTiempo(Testerilizacion[0],104,0,0,1);
      // Muestra tiempo de secado actual.
      displayTiempo(Tsecado[3],80,8,0,1);displayTiempo(Tsecado[2],86,8,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,8,2,1); 
      displayTiempo(Tsecado[1],98,8,0,1);displayTiempo(Tsecado[0],104,8,0,1);
   }
         
   if(ADC_Pt100>=((float)Setpoint-1.0))// Si activa alarma de esterilización. Inicia tiempo de esterilización.
   {
      if(est==0)
      {
         tiempo_esterilizacion=1;
         Limpia(0,90,56);glcd_rect(15, 18, 30, 53, NO, OFF);glcd_rect(25, 18, 60, 26, NO, ON);est=1;
         glcd_imagen(5);
      }
   }  
   
   if(Exhaust==1 && tiempo_esterilizacion==2)// Si realiza desfogue rapido
   {
      // Debe enviar a Hardware_Controller que abra EV de desfogue
      Control=0;
      if(Presostato<2)
      {
         tiempo_secado=1;
         Control=0;EVDesfogue=3;
      }
      else
      {
         EVDesfogue=1;
      }
   }
   
   if(Exhaust==2 && tiempo_esterilizacion==2 )// Si Debe realizar Rampa
   {     
      // Debe enviar a Hardware_Controller que abra Ejecute modo rampa.
      EVDesfogue=2;
      Control=0;
      if(Presostato<2)
      {
         tiempo_esterilizacion=3;
         tiempo_secado=2;
         fin_ciclo=1;
         EVDesfogue=3;
      }
   }
   
   // Si cumplio el tiempo de esterilización, verifique si existe tiempo de secado.
   if((Testerilizacion[0]==Testerilizacionp[0])&&(Testerilizacion[1]==Testerilizacionp[1])&&(Testerilizacion[2]==Testerilizacionp[2])&&(Testerilizacion[3]==Testerilizacionp[3]) && tiempo_esterilizacion==1)
   {
      tiempo_esterilizacion=2;
   } 
   
   // Si cumplio el tiempo de secado.
   if((Tsecado[0]>=Tsecadop[0])&&(Tsecado[1]>=Tsecadop[1])&&(Tsecado[2]>=Tsecadop[2])&&(Tsecado[3]>=Tsecadop[3]) && tiempo_secado==1)
   {
      tiempo_esterilizacion=3;
      tiempo_secado=2;
      fin_ciclo=1;
   }
   
   if(fin_ciclo==1)
   {
      // Debe enviar a Hardware_Controller que apague Resistencia y que cierre EV de desfogue
      Control=0;EVDesfogue=3;
      if(disp_Fin==0)
      {
         glcd_rect(0, 55, 127, 63, YES, OFF);
         strcpy(Menus.Letra,"Ciclo Finalizado");displayMenu(Menus.Letra,0,55,2,1);
         disp_Calentando=0;disp_Llenando=0;disp_Secando=1;disp_Esterilizando=0;disp_Despresurizando=1;disp_Stop=0;disp_Door=0;disp_Fin=1;
      }   
      
      if(RIGHT)
      {
         Control=3;
         EnviarCodigo();
         delay_ms(1000);
         reset_cpu();
      }
   }
   EVAgua=10;
   //EnviarCodigo();
  }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
 while(Menu==100)
 {
   
   displayTiempo(datoRx[0],0,10,2,1);
   displayTiempo(datoRx[1],0,20,2,1);
   displayTiempo(datoRx[2],0,30,2,1);
   displayTiempo(datoRx[3],0,40,2,1);
   displayTiempo(Opcion,0,50,2,1);
   
   Opcion++;
   Determina();
   displayTiempo(Puerta,50,10,2,1);
   displayTiempo(Parada,50,20,2,1);
   displayTiempo(ADC_Pt100,50,30,2,1);     
   
 }
 Determina();
      
      while(Parada==0)
      {
         delay_ms(100);
         if(disp_Stop==0)
         {
            glcd_fillScreen(OFF); 
            strcpy(Menus.Letra,"  !Parada!");displayMenu(Menus.Letra,0,10,0,2);
            strcpy(Menus.Letra," !Activada!");displayMenu(Menus.Letra,0,30,0,2);emergency=1;
            disp_Calentando=0;disp_Llenando=0;disp_Secando=0;disp_Esterilizando=0;disp_Despresurizando=0;disp_Stop=1;disp_Door=0;disp_Fin=0;
         }
         if(CadenaRecibida==1)
         {
            Determina();
            EnviarCodigo();
            CadenaRecibida=0;
         }
      }   
      if(Parada==1)
      {
         if(emergency==1)
         {glcd_fillScreen(OFF);emergency=0;visual=1;disp_Stop=0;Menu=1;reset_cpu();}
      }
   }
}
