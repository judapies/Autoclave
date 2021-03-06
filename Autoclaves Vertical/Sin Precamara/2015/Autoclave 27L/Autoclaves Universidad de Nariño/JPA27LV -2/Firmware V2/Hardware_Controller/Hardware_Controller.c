// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// JP Inglobal. 2015

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR// PLL1 para 4 MHz
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL3,CPUDIV4,VREGEN,NOMCLR// PLL1 para 12 MHz
#use delay(clock=48000000)
#use RS232(BAUD=9600, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Definici?n de Entradas
#define   TERM          !input(PIN_D6)
#define   DOOR          !input(PIN_D5)
#define   STOP          !input(PIN_D4)
#define   LEVELT        input(PIN_D1)
#define   LEVELR        input(PIN_D0)

// Definici?n de Salidas
#define   Resistencia_on     output_bit(PIN_B0,1)
#define   Resistencia_off    output_bit(PIN_B0,0)
#define   EV_Desfogue_on     output_bit(PIN_B1,1)
#define   EV_Desfogue_off    output_bit(PIN_B1,0)
#define   EV_Trampa_on       output_bit(PIN_B3,1)
#define   EV_Trampa_off      output_bit(PIN_B3,0)
#define   EV_Agua_on         output_bit(PIN_B2,1)
#define   EV_Agua_off        output_bit(PIN_B2,0)
#define   Motor_Puerta_on    output_bit(PIN_B5,1)
#define   Motor_Puerta_off   output_bit(PIN_B5,0)
#define   EV_in_Agua_on      output_bit(PIN_B4,1)
#define   EV_in_Agua_off     output_bit(PIN_B4,0)
#define   Bomba_Vacio_on     output_bit(PIN_B6,1)
#define   Bomba_Vacio_off    output_bit(PIN_B6,0)
#define   Bomba_Agua_on      output_bit(PIN_B7,1)
#define   Bomba_Agua_off     output_bit(PIN_B7,0)

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
int8 rxbuf[Lenbuf];

float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0;
short emergency=0,desfoguelento=0,flag_agua=0,TiempoFuncionamiento=0,flag_agua2=0,CamaraOUT=0,CamaraIN=0,flag_reservorio=0;
int8 Menu=1,bite=0,TamanoCadena=0,i=0,Posicion=0,Decimal=0,r=0,t=0,TiempoControl=0,q=0,z=0;
int8 Resistencia=0,EVDesfogue=0,EVAgua=0,EVTrampa=0,MotorPuerta=0,EVEntradaAgua=0,BombaVacio=0,BombaAgua=0,Control=0,Alarma=0,AlarmaAgua=0,AlarmaTemp=0;
int8 Puerta=0,Parada=0,NivelReservorio=0,NivelTanque=0,Presostato=0,ADC_Pt100=0,y=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Minutos=15,Nivel=0,t_exhaust=0,tinicio=0,tagua=0,treservorio=0;
short disp_Calentando=0,disp_Llenando=0,disp_Secando=0,disp_Esterilizando=0,disp_Despresurizando=0,disp_Stop=0,disp_Door=0,disp_Fin=0,CadenaRecibida=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido
// Variables para Pt100
short Lectura=0,Expulsa=0, est=0,Visua=0,flagtimer=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,R2=0,fin_ciclo=0,tiempo_desfogue=0,tiempo_esterilizacion=0,tiempo_secado=0,Exhaust=0,p=0,MediaMovil=2;
float V0=0.0,R1=6180.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=0.0,d3=0.0,d4=0.0,d5=0.0;
float V1=0.0,Presion=0.0,d6=0.0,d7=0.0,d8=0.0;
float Setpoint=0.0,Temperatura=0.0,TempUSB=0.0,PresionCamara=0.0,K=0.006429;

char dato[14];
char datoTx[11];
char datoRx[14];
char ADCTemperatura[4];
char SetTemperatura[3]={0,0,0};

#int_rda
void rda_isr(void)//funci?n de interrupci?n por recepci?n de datos USART 
{ 
   dato[bite]=getch();
   
   if((bite<13 && strcmp(dato[bite],'\r')==0))
   {
      bite=0;
   }
   else
   {
      if(bite==13 && strcmp(dato[bite],'\r')==0)
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
         if(bite>13)
         {
            bite=0;
         }
         else
         {
            CadenaRecibida=0;
            bite++;
         }
      }
   }
}

void ActualizaRecta(){
   X[y]=Temperature;
   y++;
   if(y>=2)
   {
      y=0;
      Aumento=X[1]-X[0];
   }
}

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}
float Leer_Sensor(int media)
{
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/41.55; // Se elimina la ganancia
   Pt=(R1+(float)R2)/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-0.5;
   //Temperature=(Pt-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
            
   if(l>(media-1))
   {l=0;ActualizaRecta();}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   Temperature=(Prom-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
   //Prom+=((float)Ajuste-(float)Ajusten);
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

float Leer_Sensor_Presion(int media){
   promediopresion=0.0;
   V1 = sensores(1);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V1 = V1*(4.97/1023.0);
   Presion=(V1-0.2)/(K);
   
   if(Presion<0.0)
   {
      Presion=0.0;
   }
   
   if(r>media-1)
   {r=0;}
   PromPresion[r]=Presion;r++;
         
   for(t=0;t<=(media-1);t++)
   {
      promediopresion+=PromPresion[t];
   } 
   promediopresion=promediopresion/media;   
   
   return promediopresion;
}

void EnviarCodigo(void)
{
   datoTx[0]=Puerta+48;
   datoTx[1]=Parada+48;
   datoTx[2]=NivelReservorio+48;
   //datoTx[3]=NivelTanque+48;
   datoTx[3]=flag_agua+48;
   datoTx[4]=Presostato+48;
   //datoTx[4]=EVAgua+48;
   datoTx[5]=((int8)Temperatura)+48;
   datoTx[6]=Alarma+48;
   datoTx[7]=AlarmaAgua+48;
   datoTx[8]=EVAgua+48;
   //itoa(Leer_Sensor(5),10,ADCTemperatura);
   /*for(i=0;i<4;i++)
   {
      if(ADCTemperatura[i]==null)
      {
         datoTx[i+5]='n';
      }
      else
      {
         datoTx[i+5]=ADCTemperatura[i];
      }
   }*/
   datoTx[9]=((int8)d3)+48;
   datoTx[10]='\r';
   for(q=0;q<11;q++)
   {
      putc(datoTx[q]);
      delay_ms(10);
   }   
}

void Determina(void){
   Resistencia=datoRx[0]-48;
   EVDesfogue=datoRx[1]-48;
   EVAgua=datoRx[2]-48;
   EVTrampa=datoRx[3]-48;
   MotorPuerta=datoRx[4]-48;
   EVEntradaAgua=datoRx[5]-48;
   BombaVacio=datoRx[6]-48;
   BombaAgua=datoRx[7]-48;
   Control=datoRx[8]-48;
   Nivel=datoRx[12]-48; 
   Setpoint=datoRx[9]-48;
   /*   for(i=0;i<3;i++)
      {
         if(strcmp(datoRx[i+9],'n')==0)
         {
            break;
         }
         else
         {
            SetTemperatura[i]=datoRx[i+9];
         }
      }     
      Setpoint=atof(SetTemperatura);*/
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(EVDesfogue==2 && Presostato>=2)
   {
      Resistencia_off;
      if(t_exhaust<=(9990*Nivel)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
      {
         //Resistencia_on;
         EV_Desfogue_on;
      }
      else         
      {
         //Resistencia_off;
         EV_Desfogue_off;
      }   
      
      if(t_exhaust>=60000) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
      {
         t_exhaust=0;
      } // tiempoctrl incrementa una unidad cada Xms, por eso se dividen los tiempos por 0.005.
   }  
}

#int_TIMER1
void temp1s(void){
   
   //set_timer1(45612);  // 5 ms 5536 , Posiblemente cargar con 22144 debido a F=12MHz
   set_timer1(5536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if(flag_reservorio==1)
   {
      treservorio++;
   }
      
   if(tiempos3==100)
   {tiempos3=0;Expulsa=1;}
   
   if(tiempos==200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {
      tiempos=0;Lectura=1;TiempoControl++;
      if(EVAgua==1)
      {
         tinicio++;
         if(flag_agua2==0)
         {
            tagua++;
         }
      }
   }
}

void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   error=desvio+Setpoint-Temperatura;      // C?lculo del error ******
   
   if(error>0.9 && error<15 && Aumento<0.3)
   {
      if(TiempoControl>=20)
      {
         TiempoControl=0;
         Ganancia+=2;
      }  
   }
   
   if(error<-0.3)
   {
      Ganancia=Ganancia2;
   }
   
   if(Ganancia<1)
   {
      Ganancia=1;
   }
   
   if(error<0.0)     // Anti Wind_DOWN    
   {
      error=0.0;
   }
   //tmp=(Setpoint-Prom)*Ganancia;  // Control Proporcional.
   tmp=error*Ganancia;  // Control Proporcional.
         
   if(tmp>Setpoint)   // Anti Wind-UP      
   {
      tmp=Setpoint;
   }
               
   if(tmp<0.0)     // Anti Wind_DOWN    
   {
      tmp=0.0;
   }
   
   // Tpulso(t)= Tciclo*(y(t)-ymin)/(ymax - ymin); calculo de ciclo util para control de resistencia.ymax=140.ymin=0;
   // Tpulso es float, hay que pasar ese ciclo a una salida de un puerto usando el TMR1.
   Tpulso=(tmp/Setpoint)*Tciclo;  
                                      
   if(tiempoctrl<=(int16)(Tpulso/0.005)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
   {
      Resistencia_on;
   }
   else         
   {
      Resistencia_off;
   }
         
   if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
   {
      tiempoctrl=0;
   } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
//--------------------------------------------------------------------------------------------------------------------//   
}

void main()
{
   enable_interrupts(INT_RDA);      //Habilita interrupci?n por recepci?n RS23   
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN3);
   setup_adc(ADC_CLOCK_DIV_32 );
   sensores(0);
   output_d(0);
   Ganancia=read_eeprom(10);desvio=read_eeprom(11);Tciclo=read_eeprom(12);
   Ajuste=read_eeprom(13);Ajusten=read_eeprom(14);Ganancia2=read_eeprom(15);
   MediaMovil=read_eeprom(20);
   EV_Agua_off;EV_Trampa_off;EV_Desfogue_off;Resistencia_off;
   enable_interrupts(global);
   usb_init();
   usb_task();
   Puerta=0;Parada=0;NivelReservorio=0;NivelTanque=0;Presostato=0;ADC_Pt100=0;
   EnviarCodigo();
   tiempo_esterilizacion=0;
   for(z=0;z<40;z++)
   {
      Temperatura=Leer_Sensor(MediaMovil);
   }
  
while(true)
{
   if(STOP || Visua==1)
   {
      EV_Desfogue_on;emergency=1;
      disable_interrupts(INT_TIMER0);
   }        
   else
   {
      if(emergency==1)
      {
         delay_ms(1500);reset_cpu();
      }
   }
   
   if(!LEVELR && !STOP)
   {
      flag_reservorio=0;   
      treservorio=0;
      EV_in_Agua_on;
   }
   else
   {
      if(treservorio>=400)
      {
         EV_in_Agua_off;
         flag_reservorio=0;   
      }
      else
      {
         flag_reservorio=1;   
      }
   }
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated())
{
   if(usb_kbhit(1))
   {
      usb_get_packet(1, rxbuf, Lenbuf);
      if(rxbuf[0]==1)
      {
         Ganancia=rxbuf[1];desvio=rxbuf[2];Tciclo=rxbuf[3];MediaMovil=rxbuf[5];Ganancia2=rxbuf[6];Ajuste=rxbuf[7];
         Ajusten=rxbuf[10];R2=rxbuf[11];
      } // Falta escribir en la eeprom
      if(rxbuf[12]==1)   
      {
         write_eeprom(10,(int8)Ganancia);write_eeprom(11,(int8)desvio);write_eeprom(12,(int8)Tciclo);write_eeprom(20,MediaMovil);
         write_eeprom(13,(int8)Ajuste);write_eeprom(14,(int8)Ajusten);write_eeprom(15,(int8)Ganancia2);
      }  
      
      if(rxbuf[13]==1)
      {
         Visua=1;
      }
      else
      {
         Visua=0;
      }
      
      if(rxbuf[8]==1)// Camara OUT
      {
         CamaraOUT=1;
      }
      else
      {
         CamaraOUT=0;
      }
      
      if(rxbuf[9]==1)// Camara IN
      {
         CamaraIN=1;
      }
      else
      {
         CamaraIN=0;
      }
    }
    
    TempUSB=Temperatura;
    recbuf[0]=(int8)(TempUSB);
    recbuf[1]=120;
    d3=TempUSB-(int8)TempUSB;
    d3=d3*10.0;
    recbuf[2]=(int8)d3;
    recbuf[3]=(int8)Pt;
    recbuf[4]=(int8)Setpoint;
    recbuf[5]=(int8)PresionCamara;
    d4=Pt-(int8)Pt;
    d4=d4*10.0;
    recbuf[6]=(int8)d4;
    recbuf[7]=(int8)NivelTanque;
    recbuf[8]=(int8)NivelReservorio;
    recbuf[9]=(int8)Tpulso;
    d5=Tpulso-(int8)Tpulso;
    d5=d5*10.0;
    recbuf[10]=(int8)d5;
    recbuf[11]=(int8)error;
    d6=error-(int8)error;
    d6=d6*10.0;
    recbuf[12]=(int8)d6;
    
    recbuf[13]=(int8)Aumento;
    d7=Aumento-(int8)Aumento;
    d7=d7*10.0;
    recbuf[14]=(int8)d7;
    
    recbuf[15]=(int8)Ganancia;
    d8=Ganancia-(int8)Ganancia;
    d8=d8*10.0;
    recbuf[16]=(int8)d8;
    recbuf[17]=(int8)Puerta;
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tama?o 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   
   if(Expulsa==1)
   {   
      Temperatura=Leer_Sensor(MediaMovil);
      PresionCamara=Leer_Sensor_Presion(5);
      Expulsa=0;
   }    

   if(tinicio>5400 && tiempo_esterilizacion==0)//tinicio expresado en segundos      
   {
         if(PresionCamara>2.0)
         {
            EV_Desfogue_on;
         }
         Alarma=1;
         EV_in_Agua_off;
         EV_Agua_off;
         Resistencia_off;
   }
   else
   {      
      if(EVDesfogue<1 && tiempo_esterilizacion==1 && (Temperatura<110.0 || Temperatura>145.0))
      {
         Alarma=2;
         EV_in_Agua_off;
         EV_Agua_off;
         Resistencia_off;
      }
      
      if(tagua>720) // Si se el ingreso de agua al tanque dura mas de 12 minutos se activa la alarma por falta de agua.
      {
         AlarmaAgua=1; 
         EV_in_Agua_off;
         EV_Agua_off;
         Resistencia_off;
      }
      
      if(Control==1 && !DOOR && !STOP && AlarmaAgua==0 && Alarma==0) // Si esta en condiciones normales para ejecutar el control.
      {
         if(!TERM)
         {
            Controla();
         }
         else
         {
            Resistencia_off;
         }
         
         if(CamaraOUT==1)
         {
            EV_Desfogue_on;
         }
         else
         {
            EV_Desfogue_off; 
         }
      }
      else
      {
         Resistencia_off;
      }
            
// Manejo de llenado de Autoclave con Agua       
      if(EVAgua!=0)
      {
         if(NivelTanque==1)
         {
            if(flag_agua==0)
            {
               delay_ms(1000);
            
               if(input(PIN_D1))
               {   
                  flag_agua=1;
                  EV_Agua_off;
                  if(flag_agua2==0)
                  {
                     flag_agua2=1;p=3;
                  }
               }
            }
            else
            {
               EV_Agua_off;
            }
         }
         else
         {
            if(flag_agua==0)
            {
               EV_Agua_on;tiempo_esterilizacion=0;
            }
         }
      }
// Fin de Manejo de llenado de Autoclave con Agua
      
//      
      if(Temperatura>=(Setpoint-1.0)&&p==3)// Si activa alarma de esterilizaci?n. Inicia tiempo de esterilizaci?n.
      {
         tiempo_esterilizacion=1;    
         p=1;
         //Ganancia=Ganancia2;
      }        
      
      if(EVDesfogue==1 )
      {
         EV_Desfogue_on;
         Resistencia_off;
      } 
      
      if(EVDesfogue==3 && !STOP )
      {
         if(Presostato<2)
         {
            EV_Desfogue_off;
         }
         else
         {
            EV_Desfogue_on;
         }
         Resistencia_off;
         if(Control==3)
         {
            reset_cpu();
         }
      }
   }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
 
      Puerta=input(PIN_D5);
      Parada=input(PIN_D4);
      NivelReservorio=input(PIN_D0);
      NivelTanque=input(PIN_D1);
      //Temperatura=Leer_Sensor(10);
      d3=Temperatura-(int8)Temperatura;
      d3=d3*10.0;
      Presostato=(int8)PresionCamara;
      if(CadenaRecibida==1)
      {
         Determina();
         EnviarCodigo();
         CadenaRecibida=0;
      } 
   }
}
