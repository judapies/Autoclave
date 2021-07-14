// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave con Calderin
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// JP Inglobal. 2015

#include <18F4550.h>
#device adc=10
#fuses XTPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)
#include <MATH.h>
// Definición de Entradas
#define   PRES          !input(PIN_D7)
#define   TERM          !input(PIN_D6)
#define   DOOR          !input(PIN_D4)
#define   STOP          !input(PIN_D5)
#define   LOW            input(PIN_D0)
#define   MEDIUM         input(PIN_D1)
#define   HIGH           input(PIN_D2)

// Definición de Salidas
#define   Resistencia_on     output_bit(PIN_C0,1)
#define   Resistencia_off    output_bit(PIN_C0,0)
#define   Resistencia2_on    output_bit(PIN_C1,1)
#define   Resistencia2_off   output_bit(PIN_C1,0)

#define   EV_Desfogue_on     output_bit(PIN_B2,1)
#define   EV_Desfogue_off    output_bit(PIN_B2,0)
#define   EV_Paso_on         output_bit(PIN_B3,1)
#define   EV_Paso_off        output_bit(PIN_B3,0)
#define   EV_Agua_on         output_bit(PIN_B4,1)
#define   EV_Agua_off        output_bit(PIN_B4,0)
#define   Motor_Puerta_on    output_bit(PIN_B6,1)
#define   Motor_Puerta_off   output_bit(PIN_B6,0)
#define   Bomba_Vacio_on     output_bit(PIN_B5,1)
#define   Bomba_Vacio_off    output_bit(PIN_B5,0)
#define   Bomba_Agua_on      output_bit(PIN_B7,1)
#define   Bomba_Agua_off     output_bit(PIN_B7,0)

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
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0;
short emergency=0,desfoguelento=0,flag_agua=0,TiempoFuncionamiento=0,flag_agua2=0,CamaraOUT=0,CamaraIN=0,flag_reservorio=0,flag_presostato=0;
int8 Menu=1,bite=0,TamanoCadena=0,i=0,Decimal=0,r=0,t=0,TiempoControl=0,q=0,z=0;
int8 Resistencia=0,EVDesfogue=0,EVAgua=0,EVTrampa=0,MotorPuerta=0,EVEntradaAgua=0,BombaVacio=0,BombaAgua=0,Control=0,Alarma=0,AlarmaAgua=0,AlarmaTemp=0;
int8 Puerta=0,Parada=0,NivelReservorio=0,NivelTanque=0,Presostato=0,ADC_Pt100=0,y=0,Chksum=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Minutos=15,Nivel=0,t_exhaust=0,tinicio=0,tagua=0,treservorio=0,tdesfogue=0;
short disp_Calentando=0,disp_Llenando=0,disp_Secando=0,disp_Esterilizando=0,disp_Despresurizando=0,disp_Stop=0,disp_Door=0,disp_Fin=0,CadenaRecibida=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido
// Variables para Pt100
short Lectura=0,Expulsa=0, est=0,Visua=0,flagtimer=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,R2=0,fin_ciclo=0,tiempo_desfogue=0,tiempo_esterilizacion=0,tiempo_secado=0,Exhaust=0,p=0,MediaMovil=2;
float V0=0.0,R1=6200.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=0.0,d3=0.0,d4=0.0,d5=0.0;
float V1=0.0,Presion=0.0,d6=0.0,d7=0.0,d8=0.0;
float Setpoint=0.0,Temperatura=0.0,TempUSB=0.0,PresionCamara=0.0,K=0.006429,p1=5.3232,p2=-16.8884,p3=37.3752,p4=-6.2316;

char datoTx[11];
char datoRx[14];
char ADCTemperatura[4];
char SetTemperatura[3]={0,0,0};

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x10], txbuf[0x11],rxbuf[0x11];     //Buffer de memoria
short EnvioMaster;               //Indicación de fin del byte enviado por el master
short SolicitudMaster=0;         //Indicación de fin de la entrega del byte solicitado por el master
byte dato=0;            //Contendrá la información a enviar o recibir
byte direccion=0;       //Contendrá la dirección del esclavo en el bus I2C

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
   V0 = V0/39.95; // Se elimina la ganancia
   Pt=(R1+(float)R2)/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-0.5;
   //Temperature=(Pt-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
            
   if(l>(media-1))
   {l=0;ActualizaRecta();}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   Temperature=(Prom-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
   //Prom+=((float)Ajuste-(float)Ajusten);
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

float Leer_Sensor_Presion(int media){
   promediopresion=0.0;
   V1 = sensores(1);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V1 = V1*(5.0/1023.0);
   Presion=(V1-0.2)/(K);
   //Presion=Presion/0.9;
   //Presion=(pow(V1,3)*p1)+(pow(V1,2)*p2)+(V1*p3)+p4; // Polinomio obtenido con Matlab
   //Presion=Presion/0.145;
   
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

void Carga_Vector(void){
   txbuf[0]=Puerta;
   txbuf[1]=Parada;
   txbuf[2]=NivelReservorio;
   txbuf[3]=flag_agua;
   txbuf[4]=Presostato;
   txbuf[5]=(int8)Temperatura;
   txbuf[6]=Alarma;
   txbuf[7]=AlarmaAgua;
   txbuf[8]=Chksum;
   txbuf[9]=(int8)d3;
}

void Lee_Vector(void){
   Resistencia=rxbuf[0];
   EVDesfogue=rxbuf[1];
   EVAgua=rxbuf[2];
   EVTrampa=rxbuf[3];
   MotorPuerta=rxbuf[4];
   EVEntradaAgua=rxbuf[5];
   BombaVacio=rxbuf[6];
   BombaAgua=rxbuf[7];
   Control=rxbuf[8];
   Setpoint=rxbuf[9];
   Chksum=rxbuf[10];
   Nivel=rxbuf[12];
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(EVDesfogue==2 && Presostato>=2)
   {
      EV_Agua_off;
      EV_Paso_off;
      Resistencia_off;
      Resistencia2_off;
      if(Nivel<6)
      {
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
      else
      {
         EV_Desfogue_on;
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
   
   if(flag_reservorio==1)
   {
      treservorio++;
   }
         
   if(tiempos3>=100)
   {
      tiempos3=0;Expulsa=1;
   }
   
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {
      tiempos=0;Lectura=1;TiempoControl++;
      if(EVAgua>=1)
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
   error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
   
   if(error>0.9 && error<15 && Aumento<0.3)
   {
      if(Tpulso<9.0)
      {
         if(TiempoControl>=20)
         {
            TiempoControl=0;
            Ganancia+=2;
         }
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
      Resistencia2_on;
   }
   else         
   {
      Resistencia_off;
      Resistencia2_off;
   }
         
   if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
   {
      tiempoctrl=0;
   } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
//--------------------------------------------------------------------------------------------------------------------//   
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
   enable_interrupts(INT_SSP);
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN1);
   setup_adc(ADC_CLOCK_DIV_64);
   enable_interrupts(global);
   usb_init();
   usb_task();
   Temperatura=Leer_Sensor(MediaMovil);
   PresionCamara=Leer_Sensor_Presion(5);
   Puerta=!DOOR;
   Parada=!STOP;
   NivelReservorio=LOW;
   NivelTanque=MEDIUM;
   //Temperatura=Leer_Sensor(10);d3=Tem
   Presostato=(int8)PresionCamara;
   Lee_Vector();
   Carga_Vector();
  
   Ganancia=read_eeprom(10);desvio=read_eeprom(11);Tciclo=read_eeprom(12);
   Ajuste=read_eeprom(13);Ajusten=read_eeprom(14);Ganancia2=read_eeprom(15);
   MediaMovil=read_eeprom(20);
   EV_Agua_off;EV_Paso_off;EV_Desfogue_off;Resistencia_off;
   
   Puerta=0;Parada=0;NivelReservorio=0;NivelTanque=0;Presostato=0;ADC_Pt100=0;
   tiempo_esterilizacion=0;
   for(z=0;z<80;z++)
   {
      Temperatura=Leer_Sensor(MediaMovil);
   }
   EV_Agua_off;
  
while(true)
{
   if(STOP || Visua==1)
   {
      emergency=1;
      EV_Desfogue_on;
      EV_Agua_off;
      Resistencia_off;
      Resistencia2_off;
      EVDesfogue=0;
      disable_interrupts(INT_TIMER0);
   }        
   else
   {
      if(emergency==1)
      {
         delay_ms(1500);
      }
   }
   
 /*  if(!LOW && !STOP)
   {
      flag_reservorio=0;   
      treservorio=0;
      EV_Agua_on;
      Resistencia_off;      
   }
   else
   {      
      if(HIGH && !STOP)
      {
         if(treservorio>=600)
         {
            EV_Agua_off;
            flag_reservorio=0;
            flag_agua=1;
            flag_agua2=1;
         }
         else
         {
            flag_reservorio=1;           
         }
         EV_Agua_off;
      }
      else
      {
         treservorio=0;
         flag_reservorio=0;
         EV_Agua_on;
      }
   }*/
      
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated())
{
   if(usb_kbhit(1))
   {
      usb_get_packet(1, rxubuf, Lenbuf);
      if(rxubuf[0]==1)
      {
         Ganancia=rxubuf[1];desvio=rxubuf[2];Tciclo=rxubuf[3];MediaMovil=rxubuf[5];Ganancia2=rxubuf[6];Ajuste=rxubuf[7];
         Ajusten=rxubuf[10];R2=rxubuf[11];
      } // Falta escribir en la eeprom
      if(rxubuf[12]==1)   
      {
         write_eeprom(10,(int8)Ganancia);write_eeprom(11,(int8)desvio);write_eeprom(12,(int8)Tciclo);write_eeprom(20,MediaMovil);
         write_eeprom(13,(int8)Ajuste);write_eeprom(14,(int8)Ajusten);write_eeprom(15,(int8)Ganancia2);
      }  
      
      if(rxubuf[13]==1)
      {
         Visua=1;
      }
      else
      {
         Visua=0;
      }
      
      if(rxubuf[8]==1)// Camara OUT
      {
         CamaraOUT=1;
      }
      else
      {
         CamaraOUT=0;
      }
      
      if(rxubuf[9]==1)// Camara IN
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
    recbuf[5]=Presostato;
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
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tamaño 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   
   if(Expulsa==1)
   {   
      Temperatura=Leer_Sensor(MediaMovil);
      d3=Temperatura-(int8)Temperatura;
      d3=d3*10.0;
      PresionCamara=Leer_Sensor_Presion(20);
      if(PresionCamara<=255)
      {
         if(PresionCamara<2.0)
         {
            PresionCamara=0.0;
         }
         Presostato=(int8)PresionCamara;
      }
      else
      {
         Presostato=255;
      }
      Puerta=!DOOR;
      Parada=!STOP;
      NivelReservorio=LOW;
      NivelTanque=HIGH;
      //Temperatura=Leer_Sensor(10);
      
      Lee_Vector();
      Carga_Vector();
      Expulsa=0;
   }    

   if(tinicio>7200 && tiempo_esterilizacion==0)//tinicio expresado en segundos      
   {
         if(PresionCamara>2.0)
         {
            EV_Desfogue_on;
         }
         Alarma=1;
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Resistencia2_off;
   }
   else
   {      
      if(EVDesfogue<1 && tiempo_esterilizacion==1 && (Temperatura<100.0 || Temperatura>145.0))
      {
         Alarma=2;
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Resistencia2_off;
      }
      
      if(tagua>300) // Si se el ingreso de agua al tanque dura mas de 1 minuto se activa la alarma por falta de agua.
      {
         AlarmaAgua=1; 
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Resistencia2_off;
      }
      
      if(!LOW && flag_agua==1)
      {
         Alarma=3; 
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Resistencia2_off;
      }
      
      if(Control==1 && !DOOR && !STOP && AlarmaAgua==0 && Alarma==0) // Si esta en condiciones normales para ejecutar el control.
      {
         if(Temperatura<90.0 )
         {
            if(PresionCamara>=5)
            {
               EV_Desfogue_on;
            }
            if(PresionCamara<2)
            {
               EV_Desfogue_off;
            }
            
         }
         else
         {
            EV_Desfogue_off;
         }
         
         if(!TERM)
         {
            Controla();
         }
         else
         {
            Resistencia_off;
            Resistencia2_off;
         }
         
         /*if(CamaraOUT==1) // Si desea abrir electrovalvula desde Software de control 
         {
            EV_Desfogue_on;
         }
         else
         {
            EV_Desfogue_off; 
         }*/
                  
      }
      else
      {
         Resistencia_off;
         Resistencia2_off;
         flag_presostato=0;
      }
      
      // Manejo de llenado de Autoclave con Agua       
      if(EVAgua!=0 && flag_agua==0)
      {
            if((!LOW || !HIGH) && !STOP)
            {
               Resistencia_off;
               flag_reservorio=0;   
               treservorio=0;
               EV_Agua_on;
               tiempo_esterilizacion=0;
               flag_agua=0;flag_agua2=0;
               delay_ms(1000);
               EV_Agua_off;
               delay_ms(1200);
            }
            else
            {
               if(treservorio>=800)
               {
                  EV_Agua_off;
                  flag_reservorio=0;   
                  flag_agua2=1;p=3;
                  flag_agua=1;
               }
               else
               {
                  flag_reservorio=1;   
               }
            }
      }
       
      if(Temperatura>=(Setpoint-1.0)&&p==3)// Si activa alarma de esterilización. Inicia tiempo de esterilización.
      {
         tiempo_esterilizacion=1;    
         p=1;
         //Ganancia=Ganancia2;
      }        
      
      if(EVDesfogue==1 )
      {
         EV_Desfogue_on;
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Resistencia2_off;
      } 
      
      if(EVDesfogue==3 && !STOP )
      {
         EV_Paso_off;
         if(Presostato<2)
         {
            EV_Desfogue_off;
         }
         else
         {
            EV_Desfogue_on;
         }
         Resistencia_off;
         Resistencia2_off;
         if(Control==3)
         {
            reset_cpu();
         }
      }
   }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
 
   }
}
