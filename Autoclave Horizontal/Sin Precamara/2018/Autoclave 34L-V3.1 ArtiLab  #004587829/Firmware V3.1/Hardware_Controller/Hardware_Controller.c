// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave con Calderin
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// JP Inglobal. 2015

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)
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
#define   Bomba_on           output_bit(PIN_C1,1)
#define   Bomba_off          output_bit(PIN_C1,0)

#define   EV_Desfogue_on     output_bit(PIN_B2,1)
#define   EV_Desfogue_off    output_bit(PIN_B2,0)
#define   EV_Paso_on         output_bit(PIN_B3,1)
#define   EV_Paso_off        output_bit(PIN_B3,0)
#define   EV_Agua_on         output_bit(PIN_B4,1)
#define   EV_Agua_off        output_bit(PIN_B4,0)
#define   EV_Aire_on         output_bit(PIN_B5,1)
#define   EV_Aire_off        output_bit(PIN_B5,0)
#define   EV_Bomba_on        output_bit(PIN_B6,1)
#define   EV_Bomba_off       output_bit(PIN_B6,0)
#define   EV_AireBomba_on    output_bit(PIN_B7,1)
#define   EV_AireBomba_off   output_bit(PIN_B7,0)

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
int8 Puerta=0,Parada=0,NivelReservorio=0,NivelTanque=0,Presostato=0,ADC_Pt100=0,y=0,Chksum=0,flagprevacio=10,flagvacio=10;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Nivel=0,t_exhaust=0,tinicio=0,tagua=0,treservorio=0,PresionCamara16=0,PresionPreCamara16=0,tvacio=0;

// Variables para Pt100
short Lectura=0,Expulsa=0,Visua=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,tiempo_esterilizacion=0,p=0,MediaMovil=2;
float V0=0.0,R1=6110.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=0.0,d3=0.0,d4=0.0,d5=0.0;
float V1=0.0,Presion=0.0,d6=0.0,d7=0.0,d8=0.0,V2=0.0,Presionp=0.0;
float Setpoint=0.0,Temperatura=0.0,TempUSB=0.0,PresionCamara=0.0,PresionPreCamara=0.0,K=0.006429;
float a=0.0011,b=2.3302,c=-244.0723;

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x10], txbuf[0x11],rxbuf[0x11];     //Buffer de memoria

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
   V0 = V0/41.14; // Se elimina la ganancia
   Pt=(R1)/((1024.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-0.6; //Ajuste de impedancia de cableado de PT100
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
   //Temperature=(Prom-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

float Leer_Sensor_Presion_Camara(int media){
   promediopresion=0.0;
   V1 = sensores(1);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V1 = V1*(5.0/1024.0);
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

float Leer_Sensor_Presion_PreCamara(int media){
   promediopresionp=0.0;
   V2 = sensores(2);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V2 = V2*(5.0/1024.0);
   Presionp=(V2-0.2)/(K);
   //Presionp=Presionp-6.8;
   
   if(Presionp<0.0)
   {
      Presionp=0.0;
   }
   
   if(r2>media-1)
   {r2=0;}
   PromPresionp[r2]=Presionp;r2++;
         
   for(t2=0;t2<=(media-1);t2++)
   {
      promediopresionp+=PromPresionp[t2];
   } 
   promediopresionp=promediopresionp/media;   
   
   return promediopresionp;
}


void Carga_Vector(void){
   txbuf[0]=Puerta;                       // Flag de estado de interruptor de puerta
   txbuf[1]=Parada;                       // Flag de estado de parada de Emergencia
   txbuf[2]=NivelReservorio;              // Nivel bajo del nivel de agua del calderin.
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
   //txbuf[15]=Atmosferica;
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
   Atmosferica=rxbuf[13];
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
      Bomba_off;
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
   {tiempos3=0;Expulsa=1;}
   
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {
      if(flag_vacio==1){
         tvacio++;
      }
      
      tiempos=0;Lectura=1;TiempoControl++;
      if(EVAgua>=1)
      {
         if(p!=1){
            tinicio++;
         }
         
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

void ControlaPresion(){
   if(PresionPrecamara16>=100)
   {
      Resistencia_off;
   }
   
   if(PresionPrecamara16<80)
   {
      Resistencia_on;
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
   Temperatura=Leer_Sensor(MediaMovil);
   PresionCamara=Leer_Sensor_Presion_Camara(5);
   PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
   Puerta=!DOOR;
   Parada=!STOP;
   NivelReservorio=LOW;
   NivelTanque=HIGH;
   Presostato=(int8)PresionCamara;
   Lee_Vector();
   Carga_Vector();
   enable_interrupts(INT_SSP);
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN1);
   setup_adc(ADC_CLOCK_DIV_32 );
   enable_interrupts(global);
  
   sensores(0);
   Ganancia=read_eeprom(10);desvio=read_eeprom(11);Tciclo=read_eeprom(12);
   Ajuste=read_eeprom(13);Ajusten=read_eeprom(14);Ganancia2=read_eeprom(15);
   MediaMovil=read_eeprom(20);
   EV_Agua_off;EV_Paso_off;EV_Desfogue_off;Resistencia_off;
   usb_init();
   usb_task();
   Puerta=0;Parada=0;NivelReservorio=0;NivelTanque=0;Presostato=0;ADC_Pt100=0;
   tiempo_esterilizacion=0;
   for(z=0;z<80;z++)
   {
      Temperatura=Leer_Sensor(MediaMovil);
   }
   EV_Agua_off;
   PulsoPositivo=10;

while(true)
{
   if(STOP || Visua==1)
   {
      emergency=1;
      EV_Desfogue_on;
      EV_Agua_off;
      Resistencia_off;
      Bomba_off;
      EVDesfogue=0;
      disable_interrupts(INT_TIMER0);
      EV_Bomba_off;
      EV_AireBomba_off;
      EV_Aire_off;
      EV_Paso_off;
      EV_Agua_off;
      Resistencia_off;
   } 
   
      if(!LOW && !STOP && AlarmaAgua==0 && Alarma==0)
      {
         flag_reservorio=0;   
         treservorio=0;
         EV_Agua_on;
         EV_Paso_on;
         Resistencia_off; 
         //flag_agua=0;
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
         }
         else
         {
            treservorio=0;
            flag_reservorio=0;
            if(EVAgua==0 && AlarmaAgua==0 && Alarma==0)
            {
               EV_Agua_on;
            }
         }
      }
   
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated())
{
   if(usb_kbhit(1))
   {
      usb_get_packet(1, rxubuf, Lenbuf);
      if(rxubuf[0]==1)
      {
         Ganancia=rxubuf[1];desvio=rxubuf[2];Tciclo=rxubuf[3];MediaMovil=rxubuf[5];Ganancia2=rxubuf[6];Ajuste=rxubuf[7];
         Ajusten=rxubuf[10];//R2=rxubuf[11];
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
    recbuf[18]=(int8)PresionPreCamara;
               
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tamaño 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   
   if(Expulsa==1)
   {   
      Temperatura=Leer_Sensor(MediaMovil);
      d3=Temperatura-(int8)Temperatura;
      d3=d3*10.0;
      PresionCamara=Leer_Sensor_Presion_Camara(5);
      PresionCamara16=(int16)Leer_Sensor_Presion_Camara(5);
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
      PresionPreCamara16=(int16)Leer_Sensor_Presion_PreCamara(5);
      Puerta=!DOOR;
      Parada=!STOP;
      NivelReservorio=LOW;
      NivelTanque=HIGH;
      //Temperatura=Leer_Sensor(10);
      Presostato=(int8)PresionCamara;
      Lee_Vector();
      Carga_Vector();
      Expulsa=0;
   }    

   if(tinicio>7200 && tiempo_esterilizacion==0 && !STOP)//tinicio expresado en segundos      
   {
         EV_Desfogue_on;
         Alarma=1;
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Bomba_off;
   }
   else
   {      
      if(EVDesfogue<1 && tiempo_esterilizacion==1 && (Temperatura<110.0 || Temperatura>145.0) && !STOP)
      {
         Alarma=2;
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Bomba_off;
      }
      
      if(tagua>720 && !STOP) // Si se el ingreso de agua al tanque dura mas de 12 minutos se activa la alarma por falta de agua.
      {
         AlarmaAgua=1; 
         EV_Agua_off;
         EV_Paso_off;
         Resistencia_off;
         Bomba_off;
      }
      
      if(BombaVacio==5 && !STOP)
      {
         ControlaPresion();
         
            if(PresionCamara16>(Atmosferica-41) && PulsoPositivo==10)
            {
               flag_vacio=1;
               Bomba_on;
               EV_Bomba_on;
               EV_AireBomba_off;
               EV_Aire_off;
               EV_Desfogue_off;
               EV_Paso_off;
               EV_Agua_off;
               flag_pulso=0;
            }
            else
            {
               PulsoPositivo=5;
            }
            
            if(PulsoPositivo==5)
            {
               if(flag_pulso==0)
               {
                  PulsosVacio++;
                  flag_pulso=1;
               }
               Bomba_on;
               EV_Bomba_off;
               EV_AireBomba_on;
               EV_Aire_off;
               EV_Desfogue_off;
               EV_Paso_on;
               
               EV_Agua_off;
               if(PresionCamara16>=Atmosferica-1)
               {
                  //if(PulsosVacio<5)
                  //{
                     EV_Paso_off;
                  //}
                  PulsoPositivo=10;
               }  
            }
         
         if(tvacio>=1200){            
            flag_vacio=0;
            flagprevacio=5;
            Bomba_off;
         }
         
         if(PulsosVacio>=4)
         {
            flagprevacio=5;
            Bomba_off;
         }
      }
      
      if(Control==1 && !DOOR && !STOP && AlarmaAgua==0 && Alarma==0) // Si esta en condiciones normales para ejecutar el control.
      {
         Bomba_off;
         EV_Bomba_off;
         EV_AireBomba_off;
         EV_Aire_off;
         EV_Desfogue_off;
         EV_Paso_on;
         EV_Agua_off;
         if(!TERM && LOW)
         {
            Controla();
         }
         else
         {
            Resistencia_off;
         }
         
      }/*
      else
      {
         Resistencia_off;
      }*/
       
      if(Temperatura>=(Setpoint-1.0)&&p==3)// Si activa alarma de esterilización. Inicia tiempo de esterilización.
      {
         tiempo_esterilizacion=1;    
         p=1;
         tinicio=0;
         //Ganancia=Ganancia2;
      }        
      
      if(EVDesfogue==1 && !STOP) // Desfogue Rapido
      {
         EV_Bomba_off;
         EV_AireBomba_off;
         EV_Aire_off;
         EV_Desfogue_on;
         EV_Paso_off;
         EV_Agua_off;
         Resistencia_off;
         Bomba_off;
      } 
      
      if(EVDesfogue==3 && !STOP ) // Ciclo de PostVacio
      {
         if(PresionCamara16>=(Atmosferica-38))
         {
            Bomba_on;
            EV_Bomba_on;
            EV_AireBomba_off;
            EV_Aire_off;
            EV_Desfogue_off;
            EV_Paso_off;
            Resistencia_off;
         }
         
         if(PresionCamara16<(Atmosferica-41))
         {
            Bomba_on;
            EV_Bomba_off;
            EV_AireBomba_on;
            EV_Aire_off;
            EV_Desfogue_off;
            EV_Paso_off;            
            Resistencia_off;
         }
      }
      
      if(EVDesfogue==4 && !STOP ) // Fin de Ciclo
      {
         Bomba_off;
         EV_Bomba_off;
         EV_AireBomba_off;
         EV_Aire_on;
         EV_Desfogue_off;
         EV_Paso_off;
         Resistencia_off;
      }
   }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
 
   }
}
