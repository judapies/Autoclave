//Falta crear menu de ajuste de variables y de norma CFR21

#include <18F4685.h>
#device adc=10
#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HS,WDT8192,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR,NOBROWNOUT
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <DS1302.c>
#include <24512.c>

#define   UP            !input(PIN_B0)
#define   DOWN          !input(PIN_B1)
#define   RIGHT         !input(PIN_B2)
#define   LEFT          !input(PIN_B3)

#define   TERM          !input(PIN_B4)
#define   STOP          !input(PIN_B5)
#define   DOOR          !input(PIN_B6)
#define   LRES          !input(PIN_B7)

//#define   LOW           //input(PIN_A2)
//#define   HIG           //input(PIN_A3)

#define   SSR_on           output_bit(PIN_E1,1)
#define   SSR_off          output_bit(PIN_E1,0)

#define   SSR2_on          output_bit(PIN_E2,1)
#define   SSR2_off         output_bit(PIN_E2,0)

#define   Buzzer_on        output_bit(PIN_D7,1)
#define   Buzzer_off       output_bit(PIN_D7,0)

#define   Paso_on          output_bit(PIN_C0,0)//EV1
#define   Paso_off         output_bit(PIN_C0,1)

#define   Desfogue_on      output_bit(PIN_C1,0)//EV2
#define   Desfogue_off     output_bit(PIN_C1,1)

#define   Vacio_on         output_bit(PIN_C2,0)//EV3
#define   Vacio_off        output_bit(PIN_C2,1)

#define   Aire_on          output_bit(PIN_D0,0)//EV4
#define   Aire_off         output_bit(PIN_D0,1)

#define   AireB_on         output_bit(PIN_D1,0)//EV5
#define   AireB_off        output_bit(PIN_D1,1)

#define   BombaVacio_on    output_bit(PIN_C5,0)//EV6
#define   BombaVacio_off   output_bit(PIN_C5,1)

#define   Agua_on          output_bit(PIN_D3,0)//EV7
#define   Agua_off         output_bit(PIN_D3,1)

#define   EV8_on           output_bit(PIN_D2,0)//EV8
#define   EV8_off          output_bit(PIN_D2,1)


#define OFF 0
#define ON  1
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

//Comunicacion//------------------------
byte SlaveA0Tx[0X20];
byte dato=0;            //Contendrá la información a enviar o recibir
byte direccion=0;       //Contendrá la dirección del esclavo en el bus I2C
byte posicion=0;        //Contendrá la posición en el buffer de memoria del esclavo donde se almacenará o de donde se recuperará el dato

/* Tabla de Modulos, Primera Fila Año Regular y el siguiente año Bisiesto
E   F   M   A   M   J   J   A   S   O   N   D
0   3   3   6   1   4   6   2   5   0   3   5
0   3   4   0   2   5   0   3   6   1   4   6
*/
int8 Regular[12]={0,3,3,6,1,4,6,2,5,0,3,5};
int8 Bisiesto[12]={0,3,4,0,2,5,0,3,6,1,4,6};

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaLlenado=OFF,AlarmaTermostato=OFF,AlarmaPuerta=OFF,AlarmaEmergencia=OFF,AlarmaBomba=OFF;
short AlarmaVacio=OFF;

//Banderas//--------------------
short IniciaLlenado=OFF,CalderinLleno=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short EstadoDesfogue=OFF,EstadoAgua=OFF,EstadoSSR=OFF,EstadoSSR2=OFF,PruebaEstados=OFF,Ciclo=OFF;
short Op=OFF,Op2=OFF,Minutos=OFF,Segundos=OFF,GuardaEEPROM=OFF,EstadoPaso=OFF,EstadoBuzzer=OFF,flagAlarma=OFF;
short flagBuzzer=OFF,flagSilencio=OFF,Finalizo=OFF,AlarmaPresion=OFF,Libre=OFF,purga=OFF,Fugas=OFF,disp_Guarda=OFF,guardaCiclo=OFF;
short PreCalentamiento=OFF,IniciaVacio=OFF,IniciaVacio2=OFF,Vacio=OFF,flag_vac=OFF,flag_vacio=OFF,flag_pulso=OFF,EstadoAireBomba=OFF;
short EstadoVacio=OFF,EstadoAire=OFF,EstadoBomba=OFF,EstadoAgua2=OFF,EstadoACierre=OFF,entro=OFF,LOW=OFF,HIG=OFF;
short SensadoNivelL=OFF,NivelBajo=OFF,onBomba=OFF,flagBombaAgua=OFF,flagBombaAguaOff=OFF,borra=OFF,purgaAgua=OFF,purgo=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoLlenado=0,TiempoSensadoNivel=0,Tp=0,Tc=0,tBombaAgua=0,tBombaAguaOff=0,tTermostato=0,tStop=0,tPurgaAgua=0;
int8 sEst=0,mEst=0,sSec=0,mSec=0,sEstp=0,mEstp=0,sSecp=0,mSecp=0,conteoPresion=0;// Tiempo transcurrido

//Control//------------------------
float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Gain=141.6,RPT100=0.5,Temporal=0.0;
float R10=6800;
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0,promediopresionp=0.0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=20,PantallaPrincipal=0,TipoClave=0,Modulo=0,Digito=0,Tapertura=0,pulsosConfigurados=3;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0,tmp2=0.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,Temperatura2=0.0,K=0.0012858;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0,CicloLibre=0,TTrampa=85;

//Varios//--------------------------------
int8 CantidadPulsos=4,NivelVacio=25,Pulsos=0,posicionDato=0,codigoAlarma=0;
int16 TiempoVacio=0,tguarda=0,tNivelBajo=0,Atmosferica=0;
short desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagImprime=0,flagImprimir=0;
int8 Control=0,tiemporecta=0,Controlando=0,ActivaImpresion=1,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0,z=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,i=0,CiclosL=0,CiclosH=0,CicloSeleccionado=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0,tiempoImpresion=0,cicloVisto=0,cicloVisualizado=0,j=0,vacioTest=17,vacioCiclo=25;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,NivelDesfogue=1;
int16 tbomba=0,tvacio=0;
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0;
int8 y=0,ConteoBuzzer=0,codigoAnt=0,PulsoPositivo=10,PulsosVacio=0;
int16 tbuzzer=0,tsilencio=0,t_exhaust=0,NivelSeleccionado=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,dowTx=0,SetTime=0,Imprime=0;
signed int8 G1=0,G2=0,sg1=0,sg2=0;
signed int16 presionManometrica=0;
//int8 iconoGrande[10]={25,16,22,24,23,28,20,46};
//int8 iconoPeque[10]={34,36,18,33,32,38,19,17};
int8 iconoGrande[11]={22,62,16,60,58,64,23,28,20,46,79};
int8 iconoPeque[11]={18,61,36,59,57,63,32,38,19,17,78};
//char const texto[11][20]={"NO ENVUELTO","ENVUELTO","LIQUIDOS A","LIQUIDOS B","BOWIE & DICK","TEST VACIO","LIBRE","TEST COMP.","HORA Y FECHA","CLAVE","DATOS"};
char const texto[11][20]={"NO ENVUELTO","ENVUELTO","DEL. NO ENVUELTO","DEL. ENVUELTO","BOWIE & DICK","TEST VACIO","LIBRE","TEST COMP.","HORA Y FECHA","CLAVE","DATOS"};
float TCiclos[7];
int16 PcCiclos[7];
int16 PpcCiclos[7];
int8 horaCiclos[7];
int8 minutoCiclos[7];
int8 ACiclos[7];
int8 diaCiclos=0,mesCiclos=0,yearCiclos=0,usuarioCiclos=0,cicloCiclos=0;

typedef struct{
   int8 Temperatura;
   int8 MinutoestH;
   int8 MinutoestL;
   int8 Segundoest;
   int8 Minutosec;
   int8 Segundosec;
   int8 PulsosVacio;
   int8 NivelVacio;
   int8 NivelDesfogue;
   int8 Minutoest;
}Personalizado;

typedef struct{
   float BN;
   float AN;
   float V0;
   float Pt;
   float RPT100;
   int8 l;
   int8 h;
   float Prom;
   float Temperature;
   int8 Ajuste;
   int8 Ajusten;
   float ajp,ajn;
   float promedio[80];
   float Gain;
}SensorPT100;

SensorPT100 Sensor[2];

Personalizado CiclosPersonalizados[4];

#include "Funciones.h"

#int_rda HIGH
void serial_rx_isr(){
   Dato2=fgetc();
   if(Dato2==0x65){//Inicio Comunicacion
      Inicio=1;
      RX_Wr_Index =0;
   }
   if(Inicio==1){
      Rx_Buffer2[RX_Wr_Index] = Dato2;
      RX_Wr_Index++;
   }
   if(RX_Wr_Index >= RX_BUFFER_SIZE){
      RX_Wr_Index =0;
      Inicio=0;
   }
   if(RX_Wr_Index==0){
      if(Rx_Buffer2[0]==0x65 && Rx_Buffer2[1]==0xff && Rx_Buffer2[2]==0xff && Rx_Buffer2[8]==0x00 && Rx_Buffer2[9]==0xff )
         Dato_Exitoso=5;
      else
         Dato_Exitoso=10;
   }
   //if(Menu==20)
     //Buzzer_on;
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(DesfogueSuave){
      if(NivelDesfogue<6){
         if(t_exhaust<=NivelSeleccionado){ // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
            Desfogue_on;
         }else{            
            Desfogue_off;
         }   
      
         if(t_exhaust>=60000){ // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
            t_exhaust=0;
         } // tiempoctrl incrementa una unidad cada Xms, por eso se dividen los tiempos por 0.005.
      }else{
         Desfogue_on;
      }
   }  
}

#int_TIMER1
void temp1s(void){
   set_timer1(40536); // Se repite cada 5ms
   tiempos++;
   tiempos2++;
   tiempos3++;
   tiempoctrl++;
   
   if(STOP){
      tStop++;
   }else{
      tStop=0;
   }
   
   if(!TERM){
      tTermostato++;
   }else{
      tTermostato=0;
   }
      
   if(flagAlarma)
      tbuzzer++;
      
   if(flagSilencio)   
      tsilencio++;
   
   if(Controlando==1){
      if(tiempoctrl<Tp) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
         SSR_on;
      else         
         SSR_off;
            
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
         tiempoctrl=0;// tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
   }
   
   if(tiempos>=200){  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
      if(flagTC)
         TiempoControl++;
      
      if(purgaAgua)
         tPurgaAgua++;
      
      if(flagAlarma)
         flagBuzzer=!flagBuzzer;
         
      tiemporecta++;
      
      if(flagBombaAgua)
         tBombaAgua++;
      
      if(flagBombaAguaOff)
         tBombaAguaOff++;
      
      if(IniciaCiclo && tiempoImpresion<CalculoImpresion){
         tiempoImpresion++;
      }else if(tiempoImpresion>=CalculoImpresion){
         flagImprime=1;
         tiempoImpresion=0;
      }
      
      if(IniciaCiclo && !Esterilizando)
         TiempoCiclo++;
      
      if(IniciaLlenado && !CalderinLleno)
         TiempoLlenado++;
      
      if(SensadoNivel)
         TiempoSensadoNivel++;
      
      if(SensadoNivelL)
         tNivelBajo++;
         
      if(IniciaVacio)
         tvacio++;         
      
      if(IniciaVacio2)
         tbomba++;   
         
      if(guardaCiclo)
         tguarda++;    
         
      tiempos=0;Lectura=1;tinicio++;
   
      if(Esterilizando && !Desfogando){               
         if(sEst>0){
            sEst--;
         }else if(mEst>0){
            mEst--;
            sEst=59;
         }
      }
      if(Secando){
         if(sSec>0){
            sSec--;
         }else if(mSec>0){
            mSec--;
            sSec=59;
         }
      }
   }
}

void main(){
   
   for (posicion=0;posicion<0x10;posicion++){
      SlaveA0Tx[posicion] = 0x00;
   }
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN5);
   setup_adc(ADC_CLOCK_DIV_64);
   port_b_pullups(true);
   enable_interrupts(int_rda);
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   LeeEEPROM();
   rtc_init();
   delay_ms(1);
   LimitaValores();
   CalculoImpresion=timpresion*60;
   ActivaImpresion=1;
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(1500);
   printf("page Clave");
   SendDataDisplay();
   ApagaSalidas();
   TiempoVacio=360; 
   Sensor[0].Gain=104.42;
   Sensor[1].Gain=104.42;
   Imprime=10;
   NivelSeleccionado=4995*NivelDesfogue;
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(0,MediaMovil);
      Temperatura2=LeerPT100(1,MediaMovil);
      PresionCamara=Leer_Sensor_Presion_Camara(5);
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
   }
   
   if(!DOOR){
      if(abs((float)PresionCamara-(float)Atmosferica)>=3 && PresionCamara<102){//Funcion de Auto Ajuste de Presion Atmosferica
         Atmosferica=PresionCamara;
         write_eeprom(31,Atmosferica);
         delay_ms(10);
         Buzzer_on;
         delay_ms(1000);
         Buzzer_off;
      }
   }
   
   while(STOP)//Si oprime boton de Toma.
   {
      if(tStop>=600){
         write_eeprom(37,0);
         delay_ms(20);
         write_eeprom(38,0);
         delay_ms(20);
         reset_cpu();
         Buzzer_on;
         delay_ms(1000);
         Buzzer_off;
      }
   }
   
   while(true){
      LeeDisplay();
      Envio_Esclavos();
      delay_ms(1);
      //Lectura_Esclavos();
      restart_wdt();
      presionManometrica=PresionCamara-Atmosferica;
      if(!Esterilizando || Desfogando){
         CalculoImpresion=180;
      }else{
         CalculoImpresion=60;
      }
      graficaCurva(6000);
      muestreo(20);
      if(!PruebaEstados){
         monitoreaAgua();
         monitoreaStop();
      }
           
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(1500);
      printf("page Clave");
      SendDataDisplay();
      ApagaSalidas();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==0){ //Menu de Contraseña de acceso.
      //ApagaSalidas();
      entro=OFF;
      SSR_off;
      SSR2_off;
      if(Digito==1){
         printf("t0.txt=\"%u\"",Contrasena[0]);
         SendDataDisplay();
         printf("va0.val=1");
         SendDataDisplay();
      }else if(Digito==2){
         printf("t0.txt=\"%u%u\"",Contrasena[0],Contrasena[1]);
         SendDataDisplay();
         printf("va0.val=2");
         SendDataDisplay();
      }else if(Digito==3){
         printf("t0.txt=\"%u%u%u\"",Contrasena[0],Contrasena[1],Contrasena[2]);
         SendDataDisplay();
         printf("va0.val=3");
         SendDataDisplay();
      }else if(Digito==4){
         printf("t0.txt=\"%u%u%u%u\"",Contrasena[0],Contrasena[1],Contrasena[2],Contrasena[3]);
         SendDataDisplay();
         printf("va0.val=4");
         SendDataDisplay();
      }
      
      if(UP){
         delay_ms(200);
         if(UP){
         if(Digito==0)
            Digito=1;
            if(Contrasena[Digito-1]<9)
               Contrasena[Digito-1]++;
         }
      }
      
      if(DOWN){
         delay_ms(200);
         if(Digito==0)
            Digito=1;
         if(DOWN){
            if(Contrasena[Digito-1]>0)
               Contrasena[Digito-1]--;
         }
      }
      
      if(RIGHT){
         delay_ms(200);
         if(RIGHT){
            Digito++;
         }
      }
      
      if(Digito>4){
      Digito=0;
      printf("t0.txt=\"\"");
      SendDataDisplay();
         if(TipoClave==0){
            if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3])){
               printf("page ClaveCorrecta");
               SendDataDisplay();
            }else{
               printf("page ClaveBad");
               SendDataDisplay();
            } 
         }else if(TipoClave==1){
            if(Contrasena[0]==4&&Contrasena[1]==4&&Contrasena[2]==4&&Contrasena[3]==4){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
            }else{
               printf("page MenuPrincipal");
               SendDataDisplay();
            }
         }else if(TipoClave==2){
            if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3])){
               printf("page Clave");
               SendDataDisplay();
               printf("titulo.txt=\"Clave Nueva\"");
               SendDataDisplay();
               TipoClave=3;
            }else{
               printf("page MenuPrincipal");
               SendDataDisplay();
            } 
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page MenuPrincipal");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(10,Contrasena[0]);delay_ms(20);write_eeprom(11,Contrasena[1]);delay_ms(20);
               write_eeprom(12,Contrasena[2]);delay_ms(20);write_eeprom(13,Contrasena[3]);delay_ms(20);
               GuardaEEPROM=ON;
            }
         }
      }
      
      if(RX_Buffer[4]==0x11){//11
         if(TipoClave!=0){
            printf("page MenuPrincipal");
            SendDataDisplay();
            PantallaPrincipal=0;
         }
      }
      
      if(TipoClave==3){
         printf("titulo.txt=\"Clave Nueva\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==2){
         printf("titulo.txt=\"Clave Actual\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==1){
         printf("titulo.txt=\"Clave Tecnico\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==0){
         printf("titulo.txt=\"Ingrese Contraseña\"");
         SendDataDisplay();
      }
      
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
            if((RX_Buffer[4]==Password[0]+0x30)&&(RX_Buffer[5]==Password[1]+0x30)&&(RX_Buffer[6]==Password[2]+0x30)&&(RX_Buffer[7]==Password[3]+0x30)){
                  printf("page ClaveCorrecta");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               }else{
                  printf("page ClaveBad");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               } 
            }
         }else if(TipoClave==1){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
            }else{
               printf("page MenuPrincipal");
               SendDataDisplay();
            }
         }else if(TipoClave==2){
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
            if((RX_Buffer[4]==Password[0]+0x30)&&(RX_Buffer[5]==Password[1]+0x30)&&(RX_Buffer[6]==Password[2]+0x30)&&(RX_Buffer[7]==Password[3]+0x30)){
               printf("page Clave");
               SendDataDisplay();
               printf("titulo.txt=\"Clave Nueva\"");
               SendDataDisplay();
               TipoClave=3;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
            }else{
               printf("page MenuPrincipal");
               SendDataDisplay();
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
            } 
            }
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page MenuPrincipal");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(10,RX_Buffer[4]-0x30);delay_ms(20);write_eeprom(11,RX_Buffer[5]-0x30);delay_ms(20);
               write_eeprom(12,RX_Buffer[6]-0x30);delay_ms(20);write_eeprom(13,RX_Buffer[7]-0x30);delay_ms(20);
               GuardaEEPROM=ON;
            }
            RX_Buffer[3]=0x00;
            RX_Buffer2[3]=0x00;
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==1){ //Menu Principal.
      revisePresion();
      GuardaEEPROM=OFF;
      PruebaEstados=OFF;
      if(RX_Buffer[3]==0x0f || UP){//0f, recibe caracteres ingresados desde el Display
         delay_ms(5);
         if(RX_Buffer[3]==0x0f || UP){
            if(RX_Buffer[4]==0x37){//Ajustes y pasa a solicitud de password de tecnico
               printf("page Clave");
               SendDataDisplay(); 
               MenuAnt=Menu;
               TipoClave=1;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
               Opcion=0;
            } 
            if(Opcion==11){// Datos
               printf("page Ciclos");
               SendDataDisplay(); 
               MenuAnt=Menu;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
               Opcion=0;
            }else if(Opcion==10){// Cambio de Clave
               printf("page Clave");
               SendDataDisplay(); 
               MenuAnt=Menu;
               TipoClave=2;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
               Opcion=0;
            }else if(Opcion==9){//Hora y Fecha
               printf("page Fecha");
               SendDataDisplay();     
               MenuAnt=Menu;               
               DiaTx=Dia;
               MesTx=Mes;
               YearTx=Year;
               HoraTx=Hora;
               MinutoTx=Minuto;
            }else if(Opcion==8){//Test de Componentes
               printf("page Test");
               SendDataDisplay();     
               MenuAnt=Menu;               
            }else if(Opcion==7){//Ciclo Personalizado
               printf("page CicloPersona");
               SendDataDisplay(); 
               MenuAnt=Menu;
            }else if(Opcion==5){//Test de Bowie & Dick
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(30,3,0,2,0,5,134,pulsosConfigurados,vacioCiclo,0);
               MenuAnt=Menu;     
            }else if(Opcion==6){//Test de Vacio
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,15,0,0,0,6,30,1,vacioTest,1);
               MenuAnt=Menu;     
            }else if(Opcion==4){//Delicado Envuelto
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,20,0,20,0,4,121,pulsosConfigurados,vacioCiclo,0);
               MenuAnt=Menu;     
            }else if(Opcion==3){//Delicado no envuelto
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,20,0,1,0,3,121,pulsosConfigurados,vacioCiclo,0);
               MenuAnt=Menu;     
            }else if(Opcion==2){//Instrumental Envuelto
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,4,0,20,0,2,134,pulsosConfigurados,vacioCiclo,0);
               MenuAnt=Menu;               
            }else if(Opcion==1){//Instrumental no Envuelto
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,4,0,1,0,1,134,pulsosConfigurados,vacioCiclo,0);               
               MenuAnt=Menu;
            }
         }
      }else{
         if(Opcion>sizeof(iconoGrande))
            Opcion=1;
            
         if(Opcion<1)
            Opcion=sizeof(iconoGrande);
         
         if(RIGHT){
            delay_ms(200);
            if(RIGHT){
               Opcion++;
            }
         }
         
         if(LEFT){
            delay_ms(200);
            if(LEFT){
               Opcion--;
            }
         }
         
         if(Opcion==1){
            printf("bc.pic=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic=%u",iconoPeque[Opcion]);
            SendDataDisplay();
            printf("bi.pic=%u",iconoPeque[sizeof(iconoGrande)-1]);
            SendDataDisplay();
            printf("bc.pic2=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic2=%u",iconoPeque[Opcion]);
            SendDataDisplay();
            printf("bi.pic2=%u",iconoPeque[sizeof(iconoGrande)-1]);
            SendDataDisplay();
            printf("textop.txt=\"%s\"",texto[Opcion-1]);
            SendDataDisplay();
         }else if(Opcion==sizeof(iconoGrande)){
            printf("bc.pic=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic=%u",iconoPeque[0]);
            SendDataDisplay();
            printf("bi.pic=%u",iconoPeque[Opcion-2]);
            SendDataDisplay();
            printf("bc.pic2=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic2=%u",iconoPeque[0]);
            SendDataDisplay();
            printf("bi.pic2=%u",iconoPeque[Opcion-2]);
            SendDataDisplay();
            printf("textop.txt=\"%s\"",texto[Opcion-1]);
            SendDataDisplay();
         }else{
            printf("bc.pic=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic=%u",iconoPeque[Opcion]);
            SendDataDisplay();
            printf("bi.pic=%u",iconoPeque[Opcion-2]);
            SendDataDisplay();
            printf("bc.pic2=%u",iconoGrande[Opcion-1]);
            SendDataDisplay();
            printf("bd.pic2=%u",iconoPeque[Opcion]);
            SendDataDisplay();
            printf("bi.pic2=%u",iconoPeque[Opcion-2]);
            SendDataDisplay();
            printf("textop.txt=\"%s\"",texto[Opcion-1]);
            SendDataDisplay();
         }
         
         printf("ttitulo.txt=\"SELECCIONE SU CICLO\"");
         SendDataDisplay();
            
         if(RX_Buffer[4]==0x0d || RX_Buffer[4]==0x0c){//Tecla Derecha Oprimida
            Opcion++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x0e || RX_Buffer[4]==0x0f){//Tecla Izquierda Oprimida
            Opcion--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
      }
      
      if(RIGHT && LEFT){
         delay_ms(20);
         if(RIGHT && LEFT){
            printf("page Clave");
            SendDataDisplay(); 
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            TipoClave=1;
            RX_Buffer[3]=0x00;
            RX_Buffer2[3]=0x00;
            Opcion=0;
         }
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==2){ //Menu de Tiempo Esterilizacion     
      revisePresion();
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }           
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            delay_ms(200);
            if(RX_Buffer[4]==0x11 || LEFT){
               Segundos=OFF;
               Minutos=OFF;
               printf("page CicloPersona");
               SendDataDisplay();  
               if(!GuardaEEPROM){
                  CiclosPersonalizados[CicloLibre-1].Minutoest=mEstp;
                  CiclosPersonalizados[CicloLibre-1].Segundoest=sEstp;
                  write_eeprom(101+((CicloLibre-1)*8),mEstp);
                  delay_ms(10);
                  write_eeprom(102+((CicloLibre-1)*8),sEstp);
                  delay_ms(10);
                  mEst=mEstp;
                  sEst=sEstp;
                  GuardaEEPROM=ON;
               }
            }
         }
         
         if(RIGHT){
            delay_ms(200);
            if(RIGHT){
               if(!Segundos){
                  Segundos=ON;
                  Minutos=OFF;
                  printf("tsecest.pco=65535");
                  SendDataDisplay();  
                  printf("tminest.pco=0");
                  SendDataDisplay();  
               }else if(!Minutos){
                  Segundos=OFF;
                  Minutos=ON;
                  printf("tsecest.pco=0");
                  SendDataDisplay();  
                  printf("tminest.pco=65535");
                  SendDataDisplay();  
               }
            }
         }
         
         if(UP){
            delay_ms(200);
            if(UP){
               if(Segundos){
                  sEstp++;
               }
               if(Minutos){
                  mEstp++;
               }    
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               if(Segundos && sEstp>0){
                  sEstp--;
               }
               if(Minutos){
                  mEstp--;
               }    
            }
         }
         
         if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla arriba Oprimida
            if(Segundos){
               sEstp++;
            }
            if(Minutos){
               mEstp++;
            }  
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla abajo oprimida
            if(Segundos && sEstp>0){
               sEstp--;
            }
            if(Minutos){
               mEstp--;
            }
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;
         }
         
         if(mEstp<1)mEstp=99;
         if(mEstp>99)mEstp=1;
         if(sEstp>59)sEstp=0;
            
         printf("tminest.txt=\"%02u\"",mEstp);
         SendDataDisplay();
         printf("tsecest.txt=\"%02u\"",sEstp);
         SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==3){ //Menu de Tiempo de Secado
      revisePresion();        
            
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Minutosec=mSecp;
               CiclosPersonalizados[CicloLibre-1].Segundosec=sSecp;
               write_eeprom(103+((CicloLibre-1)*8),mSecp);
               delay_ms(10);
               write_eeprom(104+((CicloLibre-1)*8),sSecp);
               delay_ms(10);
               mSec=mSecp;
               sSec=sSecp;
               GuardaEEPROM=ON;
            }
         }
         
         if(RIGHT){
            delay_ms(200);
            if(RIGHT){
               if(!Segundos){
                  Segundos=ON;
                  Minutos=OFF;
                  printf("tminsec.pco=0");
                  SendDataDisplay();                   
                  printf("tsecsec.pco=65535");
                  SendDataDisplay();  
               }else if(!Minutos){
                  Segundos=OFF;
                  Minutos=ON;
                  printf("tminsec.pco=65535");
                  SendDataDisplay();  
                  printf("tsecsec.pco=0");
                  SendDataDisplay();  
               }
            }
         }
         
         if(UP){
            delay_ms(200);
            if(UP){
               if(Segundos){
                  sSecp++;
               }
               if(Minutos){
                  mSecp++;
               }  
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               if(Segundos && sSecp>0){
                  sSecp--;
               }
               if(Minutos){
                  mSecp--;
               }  
            }
         }
         
         if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla Derecha Oprimida
            if(Segundos){
               sSecp++;
            }
            if(Minutos){
               mSecp++;
            }  
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00; 
         }
         
         if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla Izquierda Oprimida
            if(Segundos && sSecp>0){
               sSecp--;
            }
            if(Minutos){
               mSecp--;
            } 
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
         if(mSecp<1)mSecp=99;
         if(mSecp>99)mSecp=1;
         if(sSecp>59)mSecp=0;
            
      printf("tminsec.txt=\"%02u\"",mSecp);
      SendDataDisplay();
      printf("tsecsec.txt=\"%02u\"",sSecp);
      SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==4){ //Menu de Temperatura
      revisePresion();
         if(Setpoint<105)
            Setpoint=134;
         if(Setpoint>134)
            Setpoint=105;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Temperatura=Setpoint;
               write_eeprom(100+((CicloLibre-1)*8),Setpoint);
               delay_ms(10);
               GuardaEEPROM=ON;
            }
         }
         
         if(UP){
            delay_ms(200);
            if(UP){
               Setpoint++;
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               Setpoint--;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Setpoint++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Setpoint--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
      printf("tsettem.txt=\"%03u\"",Setpoint);
      SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==5){//Menu de Nivel
      revisePresion();
         if(Nivel<15)
            Nivel=Atmosferica;
         if(Nivel>Atmosferica)
            Nivel=15;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            printf("page CicloPersona");
            SendDataDisplay(); 
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].NivelVacio=Nivel;
               write_eeprom(106+((CicloLibre-1)*8),Nivel);
               delay_ms(20);            
               GuardaEEPROM=ON;
            }
         }
         
         if(UP){
            delay_ms(200);
            if(UP){
               Nivel++;
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               Nivel--;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Nivel++;            
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Nivel--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
         }
         
      printf("tnivel.txt=\"%02Lu\"",Nivel);
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==6){ // Menu de Prueba de Componentes
         PruebaEstados=ON;
         
         if(RX_Buffer[4]==0x01)//Oprimio EV Paso
         {
            EstadoPaso=!EstadoPaso;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoPaso){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            Paso_on;
         }else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            Paso_off;
         }
         
         if(RX_Buffer[4]==0x02)//Oprimio EV Desfogue
         {
            EstadoDesfogue=!EstadoDesfogue;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoDesfogue){
            printf("b1.bco=2016");
            SendDataDisplay(); 
            Desfogue_on;
         }else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            Desfogue_off;
         }
         
         if(RX_Buffer[4]==0x03)//Oprimio EV Vacio
         {
            EstadoVacio=!EstadoVacio;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoVacio){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            Vacio_on;
         }else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            Vacio_off;
         }
        
         if(RX_Buffer[4]==0x04)//Oprimio EV Aire
         {
            EstadoAire=!EstadoAire;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAire){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            Aire_on;
         }else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            Aire_off;
         }
         
         if(RX_Buffer[4]==0x05)//Oprimio EV Aire Bomba
         {
            EstadoAireBomba=!EstadoAireBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAireBomba){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            AireB_on;
         }else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            AireB_off;
         }
         
         if(RX_Buffer[4]==0x06)//Oprimio Bomba Vacio
         {
            EstadoBomba=!EstadoBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBomba){
            printf("b5.bco=2016");
            SendDataDisplay(); 
            if(!onBomba){
               purgaBomba();
               onBomba=ON;
            }
            BombaVacio_on;
         }else{
            printf("b5.bco=63488");
            SendDataDisplay(); 
            BombaVacio_off;
            onBomba=OFF;
         }
         
         if(RX_Buffer[4]==0x07)//Oprimio EV Agua
         {
            EstadoAgua=!EstadoAgua;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAgua){
            printf("b6.bco=2016");
            SendDataDisplay(); 
            Agua_on;
         }else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
            Agua_off;
         }
         
         if(RX_Buffer[4]==0x08)//Oprimio EV Agua2
         {
            EstadoAgua2=!EstadoAgua2;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAgua2){
            printf("b7.bco=2016");
            SendDataDisplay(); 
            //EV8_on;
            enciendeBombaAgua();
         }else{
            printf("b7.bco=63488");
            SendDataDisplay(); 
            //EV8_off;
            apagaBombaAgua();
         }
         
         if(RX_Buffer[4]==0x21)//Oprimio SSR1
         {
            EstadoSSR=!EstadoSSR;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSSR){
            printf("b21.bco=2016");
            SendDataDisplay(); 
            SSR_on;
         }else{
            printf("b21.bco=63488");
            SendDataDisplay(); 
            SSR_off;
         }
         
         if(RX_Buffer[4]==0x22)//Oprimio SSR2
         {
            EstadoSSR2=!EstadoSSR2;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSSR2){
            printf("b22.bco=2016");
            SendDataDisplay(); 
            SSR2_on;
         }else{
            printf("b22.bco=63488");
            SendDataDisplay(); 
            SSR2_off;
         }
         
         if(RX_Buffer[4]==0x17)//Oprimio Abrir Cierre
         {
            EstadoACierre=!EstadoACierre;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoACierre){
            printf("b16.bco=2016");
            SendDataDisplay(); 
            //sistemaBloqueo(1);
         }
         else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
            //sistemaBloqueo(0);
         }
         
         if(DOOR){
            printf("b8.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b8.bco=63488");
            SendDataDisplay(); 
         }
         
         if(TERM){
            printf("b9.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b9.bco=63488");
            SendDataDisplay(); 
         }
         
         if(STOP){
            printf("b10.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b10.bco=63488");
            SendDataDisplay(); 
         }
         /*
         if(PRES){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }*/
         
         if(RX_Buffer[4]==0x13)//Oprimio Buzzer
         {
            EstadoBuzzer=!EstadoBuzzer;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBuzzer){
            printf("b12.bco=2016");
            SendDataDisplay(); 
            Buzzer_on;
         }
         else{
            printf("b12.bco=63488");
            SendDataDisplay(); 
            Buzzer_off;
         }
         
         if(LOW){
            printf("b17.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b17.bco=63488");
            SendDataDisplay(); 
         }
         
         if(HIG){
            printf("b23.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b23.bco=63488");
            SendDataDisplay(); 
         }
         
         /*if(HRES){
            printf("b24.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b24.bco=63488");
            SendDataDisplay(); 
         }*/
         
         if(LRES){
            printf("b25.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b25.bco=63488");
            SendDataDisplay(); 
         }
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"A5:%3.0f %03Lu\"",sensores(5),PresionCamara);
         SendDataDisplay();
         printf("b18.txt=\"A4:%3.0f %03Lu\"",sensores(4),PresionPreCamara);
         SendDataDisplay();
         printf("b19.txt=\"%3.1f %3.1f\"",Temperatura2,Sensor[1].Pt);
         SendDataDisplay();
         printf("b20.txt=\"%3.1f %3.1f\"",Temperatura,Sensor[0].Pt);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x50){//Oprimio salir
            printf("page MenuPrincipal");
            SendDataDisplay();
            PantallaPrincipal=0;
            PruebaEstados=OFF;
            ApagaSalidas();
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==7){ //Menu de Configuración de Fecha y Hora
         Buzzer_off;         
         SSR2_off;
         if(RX_Buffer[4]==0x0a){//Selecciono Hora
            Opcion=4;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Minuto
            Opcion=5;
         }
         
         if(RX_Buffer[4]==0x0c){//Selecciono Dia
            Opcion=1;
         }
         
         if(RX_Buffer[4]==0x0d){//Selecciono Hora
            Opcion=2;
         }
         
         if(RX_Buffer[4]==0x0e){//Selecciono Year
            Opcion=3;
         }
         
         if(RIGHT){
            delay_ms(200);
            if(RIGHT){
               Opcion++;
            }
         }
         if(Opcion>5)
            Opcion=1;
         if(Opcion<1)
            Opcion=5;
            
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            printf("page MenuPrincipal");
            SendDataDisplay();  
            PantallaPrincipal=0;
            Op=OFF;
            Op2=OFF;
            
            if(esBisiesto(YearTx))
               Modulo=Bisiesto[MesTx];
            else
               Modulo=Regular[MesTx];
         
            dowTx=((YearTx-1)%7+((YearTx-1)/4-3*((YearTx-1)/100+1)/4)%7+Modulo+DiaTx%7)%7;
            rtc_set_datetime(DiaTx,MesTx,YearTx,dowTx,HoraTx,MinutoTx);
         }
         
         if((RX_Buffer[4]==0x2a)|| UP){//Tecla Arriba Oprimida
            delay_ms(200);
            if((RX_Buffer[4]==0x2a)|| UP){   
               if(Opcion==1){
                  if(MesTx==2){
                     if(esBisiesto(YearTx)){
                        if(DiaTx<29)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }else{
                        if(DiaTx<28)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }
                  }else{
                     if(MesTx<=7){
                        if(MesTx % 2 ==0){
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }else{
                           if(DiaTx<31)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }    
                     }else{
                        if(MesTx % 2 ==0){
                           if(DiaTx<31)
                              DiaTx++;  
                           else
                              DiaTx=1;
                        }else{
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;
                        }    
                     }
                  }
               }else if(Opcion==2){
                  if(MesTx<12)
                     MesTx++;
                  else
                     MesTx=1;
               }else if(Opcion==3){
                  if(YearTx<99)
                     YearTx++;
                  else 
                     YearTx=0;
               }else if(Opcion==4){
                  if(HoraTx<24)
                     HoraTx++;
                  else
                     HoraTx=0;
               }else if(Opcion==5){
                  if(MinutoTx<59)
                     MinutoTx++;
                  else
                     MinutoTx=0;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
            }
         }
         
         if(RX_Buffer[4]==0x2b || DOWN){//Tecla Abajo Oprimida
            delay_ms(200);
            if(RX_Buffer[4]==0x2b || DOWN){   
               if(Opcion==1){
                  if(DiaTx>0)
                     DiaTx--;
               }else if(Opcion==2){
                  if(MesTx>1)
                     MesTx--;
                  else
                     MesTx=12;
               }else if(Opcion==3){
                  if(YearTx>0)
                     YearTx--;
                  else
                     YearTx=99;
               }else if(Opcion==4){
                  if(HoraTx>0)
                     HoraTx--;
                  else
                     HoraTx=23;
               }else if(Opcion==5){
                  if(MinutoTx>0)
                     MinutoTx--;
                  else
                     MinutoTx=59;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
            }
         }
      
      printf("thora.txt=\"%02u\"",HoraTx);
      SendDataDisplay();
      printf("tminutos.txt=\"%02u\"",MinutoTx);
      SendDataDisplay();
      printf("tdia.txt=\"%02u\"",DiaTx);
      SendDataDisplay();
      printf("tmes.txt=\"%02u\"",MesTx);
      SendDataDisplay();
      printf("tyear.txt=\"%02u\"",YearTx);
      SendDataDisplay();
      LimitaDia();  
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==8){ //Menu Ciclos Personalizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      //SendDataDisplay();
      revisePresion();
      GuardaEEPROM=OFF;
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[4]<0x04){//Personalizados
         printf("ciclo.txt=\"CICLO P%02u\"",RX_Buffer[4]+0x06);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutoest,CiclosPersonalizados[RX_Buffer[4]-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutosec,CiclosPersonalizados[RX_Buffer[4]-1].Segundosec);
         SendDataDisplay();
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelVacio);
         SendDataDisplay();
         CicloLibre=RX_Buffer[4];
      }else if(CicloLibre==0x00){
         printf("ciclo.txt=\"CICLO P--\"");
         SendDataDisplay();          
         printf("temp.txt=\"--\"");
         SendDataDisplay();
         printf("test.txt=\"--:--\"");
         SendDataDisplay();
         printf("tsec.txt=\"--:--\"");
         SendDataDisplay();
         printf("pulsos.txt=\"---\"");
         SendDataDisplay();
         printf("vacio.txt=\"---\"");
         SendDataDisplay();
      }else if(CicloLibre>0 && CicloLibre<4){
         printf("ciclo.txt=\"CICLO P%02u\"",CicloLibre+0x06);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutoest,CiclosPersonalizados[CicloLibre-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutosec,CiclosPersonalizados[CicloLibre-1].Segundosec);
         SendDataDisplay();
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[CicloLibre-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[CicloLibre-1].NivelVacio);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page MenuPrincipal");
         SendDataDisplay();              
      }
      
      if(RX_Buffer[4]==0x20 && CicloLibre>0){//20, Temperatura
         printf("page Temperatura");
         SendDataDisplay();     
         Setpoint=CiclosPersonalizados[CicloLibre-1].Temperatura;
      }
      
      if(RX_Buffer[4]==0x30 && CicloLibre>0){//30, Test
         printf("page TiempoEst");
         SendDataDisplay();     
         mEstp=CiclosPersonalizados[CicloLibre-1].Minutoest;
         sEstp=CiclosPersonalizados[CicloLibre-1].Segundoest;
      }
      
      if(RX_Buffer[4]==0x40 && CicloLibre>0){//30, Tsec
         printf("page TiempoSec");
         SendDataDisplay();     
         mSecp=CiclosPersonalizados[CicloLibre-1].Minutosec;
         sSecp=CiclosPersonalizados[CicloLibre-1].Segundosec;
      }
      
      if(RX_Buffer[4]==0x50 && CicloLibre>0){//50, PulsosVacio
         printf("page Pulsos");
         SendDataDisplay();     
         Pulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
      }
      
      if(RX_Buffer[4]==0x80 && CicloLibre>0){//80, Nivel Vacio
         printf("page Nivel");
         SendDataDisplay();     
         Nivel=CiclosPersonalizados[CicloLibre-1].NivelVacio;         
      }
      
      if(RX_Buffer[4]==0x70 && CicloLibre>0){//70, Iniciar Ciclo         
         printf("page Funcionamiento");
         SendDataDisplay();       
         Libre=ON;
            ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest,CiclosPersonalizados[CicloLibre-1].Minutoest,
            CiclosPersonalizados[CicloLibre-1].Segundosec,CiclosPersonalizados[CicloLibre-1].MinutoSec,0,CicloLibre+6,
            CiclosPersonalizados[CicloLibre-1].Temperatura,CiclosPersonalizados[CicloLibre-1].PulsosVacio,CiclosPersonalizados[CicloLibre-1].NivelVacio
            ,0);
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==9){//Menu de Pulsos de Vacio
      revisePresion();
         if(Pulsos<2)
            Pulsos=8;
         if(Pulsos>8)
            Pulsos=2;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].PulsosVacio=Pulsos;
               write_eeprom(105+((CicloLibre-1)*8),Pulsos);
               delay_ms(20);                           
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Pulsos++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Pulsos--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
         
      printf("tpulsos.txt=\"%u\"",Pulsos);
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==12){//Menu de Visualizacion de Ciclos Realizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      //SendDataDisplay();
      SSR2_off;
      GuardaEEPROM=OFF;
      Buzzer_off;
      /* 
      i2c_start();            // Comienzo de la comunicación
      i2c_write(0xA0);   // Dirección del esclavo en el bus I2C
      i2c_write(9);    // Posición de donde se leerá el dato en el esclavo
      i2c_write(0);    // Posición de donde se leerá el dato en el esclavo
      i2c_start();            // Reinicio
      i2c_write(direccion+1); // Dirección del esclavo en modo lectura
      dato=i2c_read(0);       // Lectura del dato
      i2c_stop();
      */      
      printf("estado.txt=\"----\"");
      SendDataDisplay();   
      
      if(RX_Buffer[6]==0x01){//01, Ciclos
         printf("page MenuPrincipal");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[6]==0x02){//02, Izquierda
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisto>0)
            cicloVisto--;
         printf("h0.val=%u",cicloVisto);
         SendDataDisplay();     
         actualizaTabla();
      }
      
      if(RX_Buffer[6]==0x03){//03, Arriba
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisualizado<28)
            cicloVisualizado++;
         printf("h1.val=%u",cicloVisualizado);
         SendDataDisplay();    
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x04){//04, Abajo
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisualizado>0)
            cicloVisualizado--;
         printf("h1.val=%u",cicloVisualizado);
         SendDataDisplay();  
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x05){//05, Derecha
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisto<200)
            cicloVisto++;
         printf("h0.val=%u",cicloVisto);
         SendDataDisplay();  
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x06){//06, Imprime
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         printf("t0.txt=\"!IMPRIMIENDO!\"");
         SendDataDisplay(); 
         exportarData();
      }
      
      if(RX_Buffer[4]>0x00 || RX_Buffer[5]>0x00){
         cicloVisualizado=RX_Buffer[4]-1;
         cicloVisto=RX_Buffer[5]-1;
         RX_Buffer[4]=0x00;
         RX_Buffer[5]=0x00;
         RX_Buffer2[4]=0x00;
         RX_Buffer2[5]=0x00;
         actualizaTabla();
      }
      
      //Imprime Valores Estaticos
      printf("fecha.txt=\"%02u/%02u/%02u\"",diaCiclos,mesCiclos,yearCiclos);
      SendDataDisplay();
      
      printf("ciclo.txt=\"%02u\"",cicloCiclos);
      SendDataDisplay();
      
      printf("t0.txt=\"Ciclo %02u\"",cicloVisto);
      SendDataDisplay();
      
      printf("usuario.txt=\"---\"");
      SendDataDisplay();
      
      //Imprime Valores Dinamicos
      for(i=0;i<7;i++){
         printf("hora%u.txt=\"%02u.  %02u:%02u\"",i+1,cicloVisualizado+i+1,horaCiclos[i],minutoCiclos[i]);
         SendDataDisplay();
         printf("T%u.txt=\"%3.1f°C\"",i+1,TCiclos[i]);
         SendDataDisplay();
         printf("Pc%u.txt=\"%03LukPa\"",i+1,PcCiclos[i]);
         SendDataDisplay();
         printf("Ppc%u.txt=\"%03LukPa\"",i+1,PpcCiclos[i]);
         SendDataDisplay();         
         codificaAlarma();
      }  
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==15){//Menu de Clave Correcta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page MenuPrincipal");
      SendDataDisplay();      
      SSR2_off;
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==16){//Menu de Clave InCorrecta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page Clave");
      SendDataDisplay();
      SSR2_off;
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   if(Menu==20){ // Este es el menu de cuando el proceso se inicio.
   GuardaDatoEstatico();
   guardaCiclo=ON;
   if(!Libre){
      printf("t2f.txt=\"%u\"",CicloSeleccionado);
      SendDataDisplay();
      printf("t3f.txt=\"%s\"",Texto[CicloSeleccionado-1]);
      SendDataDisplay();
   }else{
      printf("t2f.txt=\"%u\"",CicloLibre+6);
      SendDataDisplay();
      printf("t3f.txt=\"Personalizado\"");
      SendDataDisplay();
   }
      
   if(PresionPreCamara>350 || PresionCamara>350){
      conteoPresion++;
      if(PresionPreCamara>350 || PresionCamara>350){
         if(conteoPresion>=50)
            AlarmaPresion=ON;  
      }
   }else{
      conteoPresion=0;
   }
   
   if(TiempoCiclo>1800 && !Fugas){//ANALIZAR BIEN ESTA ALARMA
      if(TiempoCiclo>5400){
         AlarmaTiempoProlongado=ON;
      }
      if(PresionPrecamara<10){
         AlarmaTiempoProlongado=ON;
      }
   }else{
      AlarmaTiempoProlongado=OFF;
   }
   
   if(Temperatura>Setpoint+4.0 && !Fugas){
      AlarmaSobreTemperatura=ON;
   }
   
   if(tTermostato>=2000){
      AlarmaTermostato=ON;
   }
   
   if(TiempoLlenado>1800){
      AlarmaLlenado=ON;
   }else{
      AlarmaLlenado=OFF;
   }
   
   if(!DOOR){
      Paso_off;
      if(!Secando && !Finalizo && !Desfogando){
         AlarmaPuerta=ON;
      }
   }else{
      AlarmaPuerta=OFF;
   }

   if(tStop>=300){
      AlarmaEmergencia=ON;         
   }
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion || AlarmaBomba 
         || AlarmaVacio || AlarmaLlenado){
      ActivaAlarma(1);
      if(AlarmaEmergencia){
         codigoAlarma=1;
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         if(PresionCamara<Atmosferica)
            Aire_on;
         else
            Aire_off;
         AireB_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         DesfogueSuave=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=20;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaPuerta){
         codigoAlarma=2;
         printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_off;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         apagaBombaAgua();
         //EV8_off;
         Vacio_off;
         SSR_off;
         SSR2_off;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
         /*if(!flagImprimir && ActivaImpresion==1){
            Imprime=20;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }*/
      }else if(AlarmaLlenado){
         codigoAlarma=9;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
            printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
            Aire_off;
            AireB_off;
            Desfogue_off;
            Paso_off;
            BombaVacio_off;
            Agua_off;
            //EV8_off;
            apagaBombaAgua();
            Vacio_off;
            SSR_off;
            SSR2_off;
            IniciaLlenado=OFF;
            IniciaCiclo=OFF;
            Esterilizando=OFF;
            Controlando=0;
            if(!flagImprimir){
               Imprime=21;
               Envio_Esclavos();
               delay_ms(100);
               Imprime=10;
               flagImprimir=1;
            }
      }else if(AlarmaTermostato){
         codigoAlarma=3;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_on;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=22;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaSobreTemperatura){                  
         codigoAlarma=4;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();  
         Aire_off;
         AireB_off;
         Desfogue_on;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=23;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaTiempoProlongado){
         codigoAlarma=5;
         printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_on;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=24;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaPresion){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         codigoAlarma=6;
         printf("Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_on;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         if(PresionCamara>(Atmosferica+4) && DOOR)
            Paso_on;
         else
            Paso_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=25;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaBomba){ // Alarma de Error de que bomba no ha arrancado.
         codigoAlarma=7;
         printf("Funcionamiento.t4f.txt=\" Error Bomba\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_off;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=26;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else if(AlarmaVacio){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         codigoAlarma=8;
         printf("Funcionamiento.t4f.txt=\" Error Vacio\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Aire_off;
         AireB_off;
         Desfogue_off;
         Paso_off;
         BombaVacio_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Vacio_off;
         SSR_off;
         SSR2_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=27;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }
   }else{
      if(!Fugas){//Si selecciono un ciclo diferente al test de fugas
         Buzzer_off;
         IniciaCiclo=ON;
      
      if(flagImprime){
         Imprime=5;
         Envio_Esclavos();
         delay_ms(100);
         Imprime=10;
         flagImprime=0;
      }
       
      if(!CalderinLleno){
         if(LRES){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Llenado Generador\"");
            SendDataDisplay();
            Agua_on;
            //EV8_on;
            enciendeBombaAgua();
         }else{
            printf("Funcionamiento.t4f.txt=\" Reservorio Vacio\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Llenado Generador\"");
            SendDataDisplay();
            Agua_off;
            //EV8_off;
            apagaBombaAgua();
         }
         Control=0;
         IniciaLlenado=ON;
         Controlando=0;
         Desfogue_off;
         Paso_off;
         Aire_off;
         AireB_off; 
         SSR_off;
         SSR2_on;
      }else if(!Precalentamiento){
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreCalentando\"");
         SendDataDisplay();
         ControlaPresion(); 
         Control=0;
         IniciaVacio=OFF;
         IniciaVacio2=OFF;
         codigoAlarma=10;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
         Desfogue_off;
         Agua_off;
         //EV8_off;
         apagaBombaAgua();
         Paso_off;
         SSR2_on;
         Desfogue_off;
         Aire_off;
         AireB_off;              
      }else{
         if(!Vacio){ //Si no ha realizado los pulsos de vacio 
            if(desfoguelento){
               CantidadPulsos=3;
               NivelVacio=30;
            }
               
            codigoAlarma=11;
            ControlaPresion();
            IniciaVacio=ON;
            
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            if(!flag_vac){//Ciclo inicial para purga de Bomba
               flag_vac=ON;
               purgaBomba();
               IniciaVacio2=ON;
               PulsoPositivo=10; //Prueba para ver rendimiento de bomba de vacio************
            }
               
            if(PresionCamara>NivelVacio && PulsoPositivo==10){//Pulso Negativo
               flag_vacio=1;
               if(flag_pulso==0){
                  PulsosVacio++;
                  flag_pulso=1;
                  Desfogue_on;
                  delay_ms(12000);
                  Desfogue_off;
                  tvacio=0;
               }
               printf("Funcionamiento.t5f.txt=\" Pulso de Vacio %u\"",PulsosVacio);
               SendDataDisplay();
               salidasPulsoNegativo();
               //monitoreaNivelAlto(); 
            }else{
               PulsoPositivo=5;
               tvacio=0;
            }
            
            if(PulsosVacio>=CantidadPulsos && PulsoPositivo==5){//Si realiza los pulsos de vacio, continua con el proceso de calentamiento
               Vacio=ON;
               Vacio_off;
               Desfogue_off;
               Aire_off;
               AireB_off;
               Paso_on;
               Agua_off;
               //EV8_off;
               apagaBombaAgua();
               BombaVacio_off; 
               SSR2_off;
               IniciaVacio=OFF;
               PulsoPositivo=10;
               flag_vac=0;
            }
              
            if(PulsoPositivo==5){//Pulso Positivo
               tvacio=0;
               flag_pulso=0;
               printf("Funcionamiento.t5f.txt=\" Pulso Positivo %u\"",PulsosVacio);
               SendDataDisplay();
               salidasPulsoPositivo();
               if(PresionCamara>=Atmosferica+25){
                  Paso_off;
                  PulsoPositivo=10;
               }  
            }
               
            if(tbomba>=60 && PulsosVacio==1 && PulsoPositivo==10){//Si no se ha generado ningun pulso de vacio despues de 60 segundos se genera alarma
               if(PresionCamara>(Atmosferica-2) && flag_pulso==1){
                  AlarmaBomba=ON;
               }else{
                  tbomba=0;
                  IniciaVacio2=OFF;
               }
             }else if(PulsosVacio>=1 && flag_pulso==0){
               IniciaVacio2=OFF;
               tbomba=0;
             }
               
            if(tvacio>840)//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
               AlarmaVacio=ON;            
         }else{
            IniciaVacio=OFF;          
            IniciaVacio2=OFF;   
            if(!Esterilizando){
               if(Temperatura>=((float)Setpoint-0.3)){
                  Esterilizando=ON;
               }
               codigoAlarma=12;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               flag_vac=OFF;
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Calentando\"");
               SendDataDisplay();
               Control=1;
               Controla();
               Paso_on;
               BombaVacio_off;
               Vacio_off;
               Aire_off;
               AireB_off;
               //Desfogue_off;
               purgaAguaTanque();
               Agua_off;
               //EV8_off;
               apagaBombaAgua();
               
               /*if(Temperatura<=TTrampa && desfoguelento){
                  Desfogue_on;
               }else{
                  Desfogue_off;
               }*/
            }else{
               TiempoCiclo=0;
               if(mEst==0 && sEst==0){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<(Atmosferica+20)){
                        Secando=OFF; 
                        DesfogueSuave=OFF;
                        BombaVacio_off;
                        Vacio_off;
                        Aire_off;
                        Desfogue_on;
                        Paso_off;                        
                        SSR2_off;
                        //SSR_off;
                        if(Temperatura<=Tapertura && PresionCamara<(Atmosferica+2)){
                           ActivaAlarma(2);
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                           SendDataDisplay();
                           Finalizo=ON;
                           codigoAlarma=14;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                              } 
                              if(!flagImprimir){
                                 Imprime=28;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                                 flagImprimir=1;
                              }
                              incrementaCiclos();
                        }
                        if(!HIG){
                           SSR_off;
                           monitoreaNivelAlto();
                        }else{
                           ControlaPresion();
                           apagaBombaAgua();
                           Agua_off;
                        }
                     }else if(PresionCamara>(Atmosferica+20)){
                        codigoAlarma=13;
                        if(codigoAnt!=codigoAlarma){
                           codigoAnt=codigoAlarma;
                           GuardaDatoCiclo();                  
                        }
                        Desfogando=ON;
                        DesfogueSuave=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Control=0;
                        //Desfogue_on;
                        if(!HIG){
                           SSR_off;
                           Paso_off;
                           monitoreaNivelAlto();
                        }else{
                           Paso_off;
                           ControlaPresion();
                           apagaBombaAgua();
                           Agua_off;
                        }
                        BombaVacio_off;
                        Vacio_off;
                        Aire_off; 
                        SSR2_off;
                        //SSR_off;
                     }
                  }else{                     
                     //Paso_off;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<=(Atmosferica+10)){  
                        if(sSec==0 && mSec==0){                          
                           Vacio_off;
                           Aire_on;
                           Desfogue_off;
                           Paso_off;
                           SSR2_off;
                           //SSR_off;
                           Secando=OFF;
                           if(PresionCamara>Atmosferica-10 && PresionCamara<Atmosferica+6){
                              ActivaAlarma(2);
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                              SendDataDisplay();
                              Finalizo=ON;
                              codigoAlarma=14;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                              }
                              if(!flagImprimir){
                                 Imprime=28;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                                 flagImprimir=1;
                              }
                              incrementaCiclos();
                              BombaVacio_off;                           
                              AireB_off;
                           }else{
                              BombaVacio_on;                           
                              AireB_on;
                           }
                           Secando=OFF;
                           if(!HIG){
                              SSR_off;
                              monitoreaNivelAlto();
                           }else{
                              ControlaPresion();
                              apagaBombaAgua();
                              Agua_off;
                           }
                        }else{
                           if(Secando){
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Secando\"");
                              SendDataDisplay();
                           }
                           if(Temperatura<110){
                              SSR2_on;
                           }else{
                              SSR2_off;
                           }
                           Paso_off;
                           if(!HIG){
                              SSR_off;
                              monitoreaNivelAlto();
                           }else{
                              ControlaPresion();
                              apagaBombaAgua();
                              Agua_off;
                           }
                        
                           if(!flag_vac){//Ciclo inicial para purga de Bomba
                              flag_vac=ON;
                              purgaBomba();
                           }
                           
                           if(PresionCamara<=Atmosferica){//Si alcanzo el nivel de vacio requerido
                              Secando=ON;
                           }
                           
                           if(PresionCamara>NivelVacio+5){//Si la presión de la camara es mayor al nivel de vacio
                              salidasPulsoNegativo(); 
                              //Agua_off;  
                              //EV8_off;
                              //apagaBombaAgua();
                           }
      
                           if(PresionCamara<=NivelVacio){//Si alcanzo el nivel de vacio requerido
                              Aire_on;
                              AireB_on;
                              codigoAlarma=15;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                                 delay_ms(100);
                              }
                              Secando=ON;
                              Desfogue_off;
                              Paso_off;
                              BombaVacio_on; 
                              Vacio_off;
                           }
                        }
                     }else if(PresionCamara>(Atmosferica+15)){
                        codigoAlarma=13;
                        if(codigoAnt!=codigoAlarma){
                           codigoAnt=codigoAlarma;
                           GuardaDatoCiclo();                  
                        }
                        Desfogando=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Control=0;
                        Controlando=0;
                        Desfogue_on;
                        if(!HIG){
                           SSR_off;
                           Paso_off;
                           monitoreaNivelAlto();
                        }else{
                           Paso_off;
                           ControlaPresion();
                           apagaBombaAgua();
                           Agua_off;
                        }
                        BombaVacio_off;
                        Vacio_off;
                        Aire_off; 
                        SSR2_on;
                        //SSR_off;   
                     }
                  }
               }else{
                  if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
                     Esterilizando=OFF;
                  }
                  codigoAlarma=16;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Esterilizando\"");
                  SendDataDisplay();
                  Paso_on;
                  Desfogue_off;
                  BombaVacio_off;
                  Agua_off;
                  Vacio_off;
                  Aire_off; 
                  SSR2_off;
                  //EV8_off;
                  apagaBombaAgua();
                  Control=1;
                  Controla();
               }
            }
         }
      } 
      }else{
         if(flagImprime){
            Imprime=5;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprime=0;
         }
         SSR_off;
         SSR2_off;
         Buzzer_off;
         IniciaCiclo=ON;
         if(!flag_vac){
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            flag_vac=ON;
            Vacio_off;
            Desfogue_off;
            Aire_off;
            AireB_on;
            Paso_off;
            Agua_off;
            for(i=0;i<10;i++){
               BombaVacio_on;
               delay_ms(100);
               BombaVacio_off;
               delay_ms(100);
            }
            BombaVacio_on;
            delay_ms(7000);
            IniciaVacio2=ON;
            IniciaVacio=ON;
            Vacio_on;
         }
         
         if(tbomba>=60){
            if(PresionCamara>(Atmosferica-2)){
               AlarmaBomba=ON;
            }else{
               tbomba=0;
               IniciaVacio2=OFF;
            }
         }
         
         if(tvacio>840){//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
            AlarmaVacio=ON;
         }
         
         if(PresionCamara<=vacioTest){
            Vacio_off;
            Desfogue_off;
            Aire_off;
            AireB_off;
            Paso_off;
            Agua_off;
            BombaVacio_off;
            Esterilizando=ON;
            IniciaVacio=OFF;
            tbomba=0;
         } 
         
         if(!Finalizo){
            if(!Esterilizando){
               if(codigoAlarma==8){
                  ActivaAlarma(2);
               }else{
                  Vacio_on;
                  Desfogue_off;
                  Aire_off;
                  AireB_off;
                  Paso_off;
                  Agua_off;
                  BombaVacio_on;
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Vacio\"");
                  SendDataDisplay();
               }
            }else{
               if(PresionCamara>50){
                  printf("Funcionamiento.t4f.txt=\" Error Ciclo\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                  SendDataDisplay();
                  codigoAlarma=8;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  if(!flagImprimir){//Imprime Ciclo Finalizado
                     Imprime=26;
                     Envio_Esclavos();
                     delay_ms(100);
                     Imprime=10;
                     flagImprimir=1;
                  }
                  IniciaVacio=OFF;
                  tbomba=0;
                  Vacio_off;
                  Desfogue_off;
                  Aire_on;
                  AireB_off;
                  Paso_off;
                  Agua_off;
                  BombaVacio_off;
                  ActivaAlarma(2);
                  incrementaCiclos();
                  Esterilizando=OFF;
               }else{
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Manteniendo Vacio\"");
                  SendDataDisplay();
               }
            }
         }
         
         if(mEst==0 && sEst==0){                
            IniciaVacio=OFF;
            tbomba=0;
            BombaVacio_off;
            Vacio_off;
            Desfogue_off;
            Aire_on;
            Paso_off;
            Agua_off;
            ActivaAlarma(2);
            if(!Finalizo){
               Finalizo=ON;
               if(PresionCamara<=50){
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                  SendDataDisplay();
                  codigoAlarma=14;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  if(!flagImprimir){
                     Imprime=28;
                     Envio_Esclavos();
                     delay_ms(100);
                     Imprime=10;
                     flagImprimir=1;
                  }
               }else{
                  printf("Funcionamiento.t4f.txt=\" Error Ciclo\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                  SendDataDisplay();
                  codigoAlarma=8;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  if(!flagImprimir){
                     Imprime=27;
                     Envio_Esclavos();
                     delay_ms(100);
                     Imprime=10;
                     flagImprimir=1;
                  }
               }
            }
            incrementaCiclos();
            Esterilizando=OFF;
         }
      }
   }

   if(tguarda>=180){
      if(!Ciclo)
         GuardaDatoCiclo();
      tguarda=0;
   }

   if(Grafica==1){
      printf("add 14,0,%1.0f",Temperatura*0.5);
      SendDataDisplay();
      printf("add 14,1,%1.0f",PresionCamara*0.5);
      SendDataDisplay();
      printf("add 14,2,%1.0f",PresionPreCamara*0.5);
      SendDataDisplay();
      Grafica=0;
   }
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      printf("tuser.txt=\"%s\"",1);
      SendDataDisplay();
      printf("t6f.txt=\"%3.1f°C\"",Temperatura);
      SendDataDisplay();
      printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
      SendDataDisplay();
      printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
      SendDataDisplay();
      if(!Fugas){
         printf("Funcionamiento.t9f.txt=\"%03u\"",Setpoint);
         SendDataDisplay();      
      }else{
         printf("Funcionamiento.t9f.txt=\"---\"");
         SendDataDisplay();      
      }
      printf("Funcionamiento.t10f.txt=\"%02u\"",mEst);
      SendDataDisplay();      
      printf("Funcionamiento.t13f.txt=\"%02u\"",sEst);
      SendDataDisplay();      
      printf("Funcionamiento.t11f.txt=\"%02u\"",mSec);
      SendDataDisplay();      
      printf("Funcionamiento.t16f.txt=\"%02u\"",sSec);
      SendDataDisplay();      
      
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      
      if(LEFT && Finalizo){
         delay_ms(1000);
         if(LEFT && Finalizo){
            printf("page MenuPrincipal");
            SendDataDisplay();
            borraVariables();
         }
      }
   }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuración de Parametros de Control
      Buzzer_off;
      SSR2_off;
      if(Opcion2>16)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=16;
      
      if(Opcion2==1){
         printf("Config.t2c.txt=\"Proporcional\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Ganancia);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Ganancia+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Ganancia-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         
         if(Ganancia>40)
            Ganancia=1;
         if(Ganancia<1)
            Ganancia=40;
      }else if(Opcion2==2){
         printf("Config.t2c.txt=\"Derivativo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Derivativo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Derivativo++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;                           
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Derivativo--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;                           
            }
         }
         
         if(Derivativo>40)
            Derivativo=1;
         if(Derivativo<1)
            Derivativo=40;
      }else if(Opcion2==3){
         printf("Config.t2c.txt=\"Integral\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Integral);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Integral++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Integral--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;                           
            }
         }
         
         if(Integral>4)
            Integral=1;
         if(Integral<1)
            Integral=4;
      }else if(Opcion2==4){
         printf("Config.t2c.txt=\"Tciclo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Tciclo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Tciclo+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Tciclo-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         
         if(Tciclo>20)
            Tciclo=5;
         if(Tciclo<5)
            Tciclo=20;
      }else if(Opcion2==5){
         printf("Config.t2c.txt=\"Ajuste +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[0].Ajuste<40)
                  Sensor[0].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[0].Ajuste>0)
                  Sensor[0].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==6){
         printf("Config.t2c.txt=\"Ajuste -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[0].Ajusten<40)
                  Sensor[0].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }         
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[0].Ajusten>0)
                  Sensor[0].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==7){
         printf("Config.t2c.txt=\"Ajuste2 +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[1].Ajuste<40)
                  Sensor[1].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[1].Ajuste>0)
                  Sensor[1].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }           
         }
      }else if(Opcion2==8){
         printf("Config.t2c.txt=\"Ajuste2 -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[1].Ajusten<40)
                  Sensor[1].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[1].Ajusten>0)
                  Sensor[1].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==9){
         printf("Config.t2c.txt=\"Media\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",MediaMovil);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(MediaMovil<80)
                  MediaMovil++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(MediaMovil>10)
                  MediaMovil--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }            
         }
      }else if(Opcion2==10){
         printf("Config.t2c.txt=\"TTrampa\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",TTrampa);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(TTrampa<100)
                  TTrampa++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(TTrampa>50)
                  TTrampa--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==11){
         printf("Config.t2c.txt=\"Vacio Ciclo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",vacioCiclo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
                  ++vacioCiclo;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
                  --vacioCiclo;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         if(vacioCiclo>30)
            vacioCiclo=17;
         if(vacioCiclo<17)
            vacioCiclo=30;
      }else if(Opcion2==12){
         printf("Config.t2c.txt=\"Ciclos\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%05Lu\"",Ciclos);
         SendDataDisplay();
      }else if(Opcion2==13){
         printf("Config.t2c.txt=\"Atmosferica\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03Lu\"",Atmosferica);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Atmosferica<100)
                  ++Atmosferica;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Atmosferica>50)
                  --Atmosferica;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==14){
         printf("Config.t2c.txt=\"T. Apertura\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03u\"",Tapertura);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Tapertura<100)
                  ++Tapertura;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Tapertura>50)
                  --Tapertura;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==15){
         printf("Config.t2c.txt=\"Pulsos Config\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",pulsosConfigurados);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
                  ++pulsosConfigurados;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
                  --pulsosConfigurados;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         if(pulsosConfigurados>8)
            pulsosConfigurados=2;
         if(pulsosConfigurados<2)
            pulsosConfigurados=8;
      }else if(Opcion2==16){
         printf("Config.t2c.txt=\"Vacio Test\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",vacioTest);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
                  ++vacioTest;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
                  --vacioTest;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         if(vacioTest>30)
            vacioTest=17;
         if(vacioTest<17)
            vacioTest=30;
      }
      
      if(RX_Buffer[4]==0x0c || LEFT){//Tecla Izquierda Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0c || LEFT){
            Opcion2++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
      }
         
      if(RX_Buffer[4]==0x0d || RIGHT){//Tecla Derecha Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0d || RIGHT){
            Opcion2--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
         }
      }
      
      if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){//Tecla Regresar
         delay_ms(20);
         if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){
            if(!GuardaEEPROM){
               write_eeprom(18,(int8)Integral);
               delay_ms(10);
               write_eeprom(19,(int8)Derivativo);
               delay_ms(10);
               write_eeprom(20,(int8)Ganancia);
               delay_ms(10);
               write_eeprom(21,Sensor[0].Ajuste);
               delay_ms(10);
               write_eeprom(22,Sensor[0].Ajusten);
               delay_ms(10);
               write_eeprom(23,Sensor[1].Ajuste);
               delay_ms(10);
               write_eeprom(24,Sensor[1].Ajusten);
               delay_ms(10);
               write_eeprom(25,(int8)MediaMovil);
               delay_ms(10);
               write_eeprom(26,(int8)Tciclo);
               delay_ms(10);
               write_eeprom(28,(int8)Sensor[0].RPT100);
               delay_ms(10);
               Temporal=Sensor[0].RPT100-((int8)Sensor[0].RPT100);
               Temporal=Temporal*10;
               write_eeprom(27,(int8)Temporal);
               delay_ms(10);
               write_eeprom(30,(int8)Sensor[1].RPT100);
               delay_ms(10);
               Temporal=Sensor[1].RPT100-((int8)Sensor[1].RPT100);
               Temporal=Temporal*10;
               write_eeprom(29,(int8)Temporal);
               delay_ms(10);
               write_eeprom(31,Atmosferica);
               delay_ms(10);
               write_eeprom(32,Tapertura);
               delay_ms(10);
               write_eeprom(39,TTrampa);
               delay_ms(10);
               write_eeprom(46,NivelDesfogue);
               delay_ms(10);
               write_eeprom(47,pulsosConfigurados);
               delay_ms(10);
               write_eeprom(48,vacioTest);
               delay_ms(10);
               write_eeprom(49,vacioCiclo);
               delay_ms(10);
               GuardaEEPROM=ON;
               NivelSeleccionado=4995*NivelDesfogue;
               Buzzer_on;
               delay_ms(1000);
               Buzzer_off;
            }
            printf("page MenuPrincipal");
            SendDataDisplay();
         }
      }
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay(); 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
   }
}
