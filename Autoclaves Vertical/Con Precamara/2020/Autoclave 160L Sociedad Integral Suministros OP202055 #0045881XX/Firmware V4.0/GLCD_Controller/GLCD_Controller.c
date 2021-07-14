//Falta crear menu de ajuste de variables y de norma CFR21

#include <18F4685.h>
#device adc=10
#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HS,WDT8192,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <DS1302.c>
#include <24512.c>

//#define   UP            !input(PIN_B0)
//#define   DOWN          !input(PIN_B1)
//#define   RIGHT         !input(PIN_B2)
//#define   LEFT          !input(PIN_B3)

#define   UPD            !input(PIN_B0)
#define   DND            !input(PIN_B1)
#define   SWBLO          !input(PIN_B2)
#define   DOORO          !input(PIN_B3)
#define   TERM           input(PIN_B4)
#define   STOP           !input(PIN_B5)
#define   SWBL           !input(PIN_B6)
#define   DOOR           !input(PIN_B7)

//#define   LOW           //input(PIN_A2)
//#define   HIG           //input(PIN_A3)

#define   SSR_on           output_bit(PIN_E1,1)
#define   SSR_off          output_bit(PIN_E1,0)

#define   SSR2_on          output_bit(PIN_E2,1)
#define   SSR2_off         output_bit(PIN_E2,0)

#define   Buzzer_on        output_bit(PIN_D7,1)
#define   Buzzer_off       output_bit(PIN_D7,0)

#define   Paso_on          output_bit(PIN_C0,0)
#define   Paso_off         output_bit(PIN_C0,1)

#define   Desfogue_on      output_bit(PIN_C1,0)
#define   Desfogue_off     output_bit(PIN_C1,1)

#define   Agua_on          output_bit(PIN_C2,0)
#define   Agua_off         output_bit(PIN_C2,1)

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
short AlarmaVacio=OFF,UP=OFF,DOWN=OFF,RIGHT=OFF,LEFT=OFF,flagAbreSecado=0;

//Banderas//--------------------
short IniciaLlenado=OFF,CalderinLleno=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short EstadoDesfogue=OFF,EstadoAgua=OFF,EstadoSSR=OFF,PruebaEstados=OFF,Ciclo=OFF;
short Op=OFF,Op2=OFF,Minutos=OFF,Segundos=OFF,GuardaEEPROM=OFF,EstadoPaso=OFF,EstadoBuzzer=OFF,flagAlarma=OFF;
short flagBuzzer=OFF,flagSilencio=OFF,Finalizo=OFF,AlarmaPresion=OFF,Libre=OFF,purga=OFF,Fugas=OFF,disp_Guarda=OFF,guardaCiclo=OFF;
short PreCalentamiento=OFF,IniciaVacio=OFF,IniciaVacio2=OFF,EstadoAireBomba=OFF;
short EstadoVacio=OFF,EstadoAire=OFF,EstadoACierre=OFF,entro=OFF,LOW=OFF,HIG=OFF;
short SensadoNivelL=OFF,NivelBajo=OFF,flagBombaAgua=OFF,flagBombaAguaOff=OFF,ingreso=0,cerro=OFF,abrio=OFF,flagLiquidos=OFF,pulsoSubir=OFF,pulsoBajar=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoLlenado=0,TiempoSensadoNivel=0,Tp=0,Tc=0,tBombaAgua=0,tBombaAguaOff=0,NivelSeleccionado=0;
int8 sEst=0,mEst=0,sSec=0,mSec=0,sEstp=0,mEstp=0,sSecp=0,mSecp=0,j=0,cicloVisto=0,codigoUsuario=0,cicloVisualizado=0,codigoUsuario2=0,codigoUsuarioant=0,NivelDes=0,conteo=0,TTrampa=0;

//Control//------------------------
float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Gain=141.6,RPT100=0.5;
float R10=6800;
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0,promediopresionp=0.0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=20,PantallaPrincipal=0,TipoClave=0,Modulo=0,Tapertura=0;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0,tmp2=0.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,Temperatura2=0.0,K=0.0012858;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0,CicloLibre=0;

//Varios//--------------------------------
int8 CantidadPulsos=4,NivelVacio=25,Pulsos=0,posicionDato=0,codigoAlarma=0,Atmosferica=0,conteoRx=0,comandoRx=0,tAbreSecado=0;
int16 TiempoVacio=0,tguarda=0,tNivelBajo=0;
short desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagImprime=0,flagImprimir=0;
int8 Control=0,tiemporecta=0,Controlando=0,ActivaImpresion=1,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,i=0,CiclosL=0,CiclosH=0,CicloSeleccionado=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0,tiempoImpresion=0;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0;
int16 tbomba=0,tvacio=0;
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0;
int8 y=0,ConteoBuzzer=0,codigoAnt=0,PulsosVacio=0;
int16 tbuzzer=0,tsilencio=0,t_exhaust=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,dowTx=0,SetTime=0,Imprime=0;
signed int8 G1=0,G2=0,sg1=0,sg2=0;
const char Texto[12][24]={"TEXTIL & INSTRUMENTAL","CAUCHO","LIQUIDOS A","LIQUIDOS B","NO ENVUELTOS 121","ENVUELTO 121",
"ENVUELTO 134","LIQUIDOS C","SOLIIDOS","DESINFECCION","TEST DE COMPONENTES","HORA Y FECHA"};

//Textil & Instrumental 134°C, 5m,15m
//Prion 134°C, 20m,15m
//Caucho 121°C, 25m,15m
//Contenedores 134°C, 5m,25m
//Rapid 134°C, 5m,8m

char usuario[20];
float TCiclos[7];
int16 PcCiclos[7];
int16 PpcCiclos[7];
int8 horaCiclos[7];
int8 minutoCiclos[7];
int8 ACiclos[7];
int8 diaCiclos=0,mesCiclos=0,yearCiclos=0,usuarioCiclos=0,cicloCiclos=0;
short estados[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

typedef struct{
   int8 Password[4];
}Claves;

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
   float promedio[80];
   float Gain;
}SensorPT100;

SensorPT100 Sensor[2];
Personalizado CiclosPersonalizados[20];
Claves Clave[10];

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
   if(Menu==20)
     Buzzer_on;
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(DesfogueSuave){
         if(t_exhaust<=NivelSeleccionado){ // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
            Desfogue_on;
         }else{            
            Desfogue_off;
         }   
      
         if(t_exhaust>=60000){ // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
            t_exhaust=0;
         } // tiempoctrl incrementa una unidad cada Xms, por eso se dividen los tiempos por 0.005.
   }  
}

#int_TIMER1
void temp1s(void){
   set_timer1(40536); // Se repite cada 5ms
   tiempos++;
   tiempos2++;
   tiempos3++;
   tiempoctrl++;
   
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
      
      tiemporecta++;
      
      if(guardaCiclo)
         tguarda++;
         
      if(flagAbreSecado)
         tAbreSecado++;
      
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
   delay_ms(10); 
   printf("page Usuarios");
   SendDataDisplay();
   ApagaSalidas();
   TiempoVacio=360; 
   Sensor[0].Gain=105.56;
   Sensor[1].Gain=105.56;
   Imprime=10;
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(0,MediaMovil);
      Temperatura2=LeerPT100(1,MediaMovil);
      PresionCamara=Leer_Sensor_Presion_Camara(10);
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(10);
   }
   
   if(UP && DOWN && RIGHT && LEFT)//Si oprime boton de Toma.
   {
      write_eeprom(37,0);
      delay_ms(20);
      write_eeprom(38,0);
      delay_ms(20);
      reset_cpu();
   }
   detienePuerta();
   detieneBloqueo();
   
   while(true){
      LeeDisplay();
      Envio_Esclavos();
      delay_ms(1);
      //Lectura_Esclavos();
      restart_wdt();
      if(!Esterilizando || Desfogando){
         CalculoImpresion=180;
      }else{
         CalculoImpresion=60;
      }
      graficaCurva(6000);
      muestreo(20);
      monitoreaAgua();
      monitoreaStop();
      
           
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(100);
      printf("page Usuarios");
      SendDataDisplay();
      //ApagaSalidas();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==0){ //Menu de Contraseña de acceso.
      entro=OFF;
      //ApagaSalidas();
      //printf("bkcmd=0");
      //SendDataDisplay();
      
      
      if(TipoClave==3){
         printf("titulo.txt=\"Clave Nueva\"");
         SendDataDisplay();
      }
      
      if(TipoClave==2){
         printf("titulo.txt=\"Clave Actual\"");
         SendDataDisplay();        
      }
      
      if(TipoClave==1){
         printf("titulo.txt=\"Clave Tecnico\"");
         SendDataDisplay();
      }
      
      if(TipoClave==0){
         printf("titulo.txt=\"%s\"",usuario);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x11){//11, Regresar
         if(TipoClave==1){
            printf("page Ajustes");
            SendDataDisplay();       
         }else{
            printf("page Usuarios");
            SendDataDisplay();       
         }
      }
      
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         delay_ms(800);
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32 &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){ // Si Ingresa clave para reset general del sistema.
               for(i=0;i<40;i++){//Lee contraseñas de usuarios
                  write_eeprom(300+i,0);delay_ms(20);
               }
               reset_cpu();
            }
            
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
               if((RX_Buffer[4]==Clave[codigoUsuario].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario].Password[1]+0x30)&&
               (RX_Buffer[6]==Clave[codigoUsuario].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario].Password[3]+0x30)
               &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){
                  printf("page ClaveCorrecta");
                  SendDataDisplay();
                  ingreso=ON;
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               }else{
                  printf("page ClaveBad");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               } 
            }
         }else if(TipoClave==1){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34&&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
               RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
            }else{
               printf("page MenuPrincipal");
               SendDataDisplay();
               RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
            }
         }else if(TipoClave==2){
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
               if((RX_Buffer[4]==Clave[codigoUsuario2].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario2].Password[1]+0x30)&&
               (RX_Buffer[6]==Clave[codigoUsuario2].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario2].Password[3]+0x30)
               &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){
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
                  codigoUsuario=codigoUsuarioant;
               } 
            }
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page MenuPrincipal");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(300+(codigoUsuario2*4),RX_Buffer[4]-0x30);delay_ms(20);
               write_eeprom(301+(codigoUsuario2*4),RX_Buffer[5]-0x30);delay_ms(20);
               write_eeprom(302+(codigoUsuario2*4),RX_Buffer[6]-0x30);delay_ms(20);
               write_eeprom(303+(codigoUsuario2*4),RX_Buffer[7]-0x30);delay_ms(20);
               GuardaEEPROM=ON;
               codigoUsuario=codigoUsuarioant;
            }
            RX_Buffer[3]=0x00;
            RX_Buffer2[3]=0x00;
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==1){ //Menu Principal.
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      codigoUsuarioant=codigoUsuario;
      
      if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Textil & Instrumental
         delay_ms(5);
         if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Textil & Instrumental
            printf("page Funcionamiento");
            SendDataDisplay();
            ConfiguraCiclo(0,5,0,15,0,1,134,7,25,0);
            Op=OFF;
            Op2=OFF;
         }
      }
      
      if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//02, Caucho
         delay_ms(5);
         if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,25,0,15,0,2,121,7,25,0);            
            Op=OFF;
            Op2=OFF;
         }
      }
      
      if(RX_Buffer[4]==0x03  && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//03, Liquidos A
         delay_ms(5);
         if(RX_Buffer[4]==0x03 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page NivelDes");
            SendDataDisplay();     
            ConfiguraCiclo(0,15,0,0,1,3,121,0,0,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=2;
            flagLiquidos=ON;
         }
      }
      
      if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//04, Liquidos B
         delay_ms(5);
         if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page NivelDes");
            SendDataDisplay();       
            ConfiguraCiclo(0,20,0,0,1,4,121,0,0,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=2;
            flagLiquidos=ON;
         }
      }
      
      if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//05, No Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,2,0,5,121,7,25,0);  
            Op=OFF;
            Op2=OFF;
            Fugas=OFF;
         }
      }
      
      if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//06, Envuelto 121
         delay_ms(5);
         if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,20,0,6,121,7,25,0);  
            Op=OFF;
            Op2=OFF;
         }
      }
      
      if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//07, Envuelto Doble 2
         delay_ms(5);
         if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,7,0,30,0,7,134,7,25,0);  
            Op=OFF;
            Op2=OFF;
         }
      }
      
      if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//08, Liquidos C
         delay_ms(5);
         if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page NivelDes");
            SendDataDisplay();       
            ConfiguraCiclo(0,30,0,0,1,8,121,0,0,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=2;
            flagLiquidos=ON;
         }
      }
      
      if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//09, Solidos
         delay_ms(5);
         if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,20,0,20,0,9,121,0,0,0);  
            Op=OFF;
            Op2=OFF;           
         }
      }
      
      if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//0a, Desinfeccion
         delay_ms(5);
         if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,25,0,15,0,10,105,0,0,0);  
         }
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Ciclos Fijos
         printf("page MenuPrincipal");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         if(codigoUsuario==0 || codigoUsuario==9){       
            printf("page Ajustes");
            SendDataDisplay();     
         }
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==2){ //Menu de Tiempo Esterilizacion     
         printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }           
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar            
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
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();            
            
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
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
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Setpoint<105)
            Setpoint=134;
         if(Setpoint>134)
            Setpoint=105;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page CicloPersona");
            SendDataDisplay();  
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Temperatura=Setpoint;
               write_eeprom(100+((CicloLibre-1)*8),Setpoint);
               delay_ms(10);
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Setpoint++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Setpoint--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tsettem.txt=\"%03u\"",Setpoint);
      SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==5){//Menu de Nivel
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Nivel<25)
            Nivel=Atmosferica;
         if(Nivel>Atmosferica)
            Nivel=25;
         
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
         
         if(RX_Buffer[4]==0x03)//Oprimio EV Agua
         {
            EstadoAgua=!EstadoAgua;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAgua){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            Agua_on;
         }else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            Agua_off;
         }
         
         if(RX_Buffer[4]==0x04)//Oprimio SSR1
         {
            EstadoSSR=!EstadoSSR;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSSR){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            SSR_on;
         }else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            SSR_off;
         }
         
         if(RX_Buffer[4]==0x05)//Oprimio Abrir Puerta
         {
            //EstadoVacio=!EstadoVacio;
            EstadoVacio=1;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoVacio){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            abrePuerta();
         }else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            
         }
        
         if(RX_Buffer[4]==0x06)//Oprimio Cierra Puerta
         {
            //EstadoAire=!EstadoAire;
            EstadoAire=1;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAire){
            printf("b5.bco=2016");
            SendDataDisplay(); 
            cierraPuerta();
         }else{
            printf("b5.bco=63488");
            SendDataDisplay(); 
            
         }
         
         if(RX_Buffer[4]==0x07)//Oprimio Detiene Puerta
         {
            EstadoAireBomba=!EstadoAireBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAireBomba){
            printf("b6.bco=2016");
            SendDataDisplay();
            EstadoAire=0;
            EstadoVacio=0;
            detienePuerta();
            
         }else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
         }
         
         if(RX_Buffer[4]==0x08)//Oprimio Buzzer
         {
            EstadoBuzzer=!EstadoBuzzer;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBuzzer){
            printf("b7.bco=2016");
            SendDataDisplay(); 
            Buzzer_on;
         }
         else{
            printf("b7.bco=63488");
            SendDataDisplay(); 
            Buzzer_off;
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
            bloqueaPuerta();
         }
         else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
            desbloqueaPuerta();
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
         
         if(SWBL){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }
         
         if(LOW){
            printf("b17.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b17.bco=63488");
            SendDataDisplay(); 
         }
         
         if(HIG){
            printf("b21.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b21.bco=63488");
            SendDataDisplay(); 
         }
         
         if(DOORO){
            printf("b12.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b12.bco=63488");
            SendDataDisplay(); 
         }
         
         if(SWBLO){
            printf("b19.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b19.bco=63488");
            SendDataDisplay(); 
         }
         
         Lectura_I2C(0XC0, 0, conteoRx);    //Lectura por I2C
         Lectura_I2C(0XC0, 1, comandoRx);    //Lectura por I2C
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"A5:%3.0f %03Lu\"",sensores(5),PresionCamara);
         SendDataDisplay();
         printf("b18.txt=\"A4:%3.0f %03Lu\"",sensores(4),PresionPreCamara);
         SendDataDisplay();
         printf("b19.txt=\"%u %u\"",conteoRx,comandoRx);
         SendDataDisplay();
         printf("b20.txt=\"%3.1f %3.1f\"",Temperatura,Sensor[0].Pt);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x21){//Oprimio salir
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
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[4]<0x15 && estados[RX_Buffer[4]-1]){//Personalizados
         printf("ciclo.txt=\"CICLO P%02u\"",RX_Buffer[4]+0x0a);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutoest,CiclosPersonalizados[RX_Buffer[4]-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutosec,CiclosPersonalizados[RX_Buffer[4]-1].Segundosec);
         SendDataDisplay();
         //printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].PulsosVacio);
         printf("pulsos.txt=\"--\"");
         SendDataDisplay();
         //printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelVacio);
         printf("vacio.txt=\"--\"");
         SendDataDisplay();
         printf("desfogue.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelDesfogue);
         //printf("desfogue.txt=\"N/A\"");
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
         printf("pulsos.txt=\"--\"");
         SendDataDisplay();
         printf("vacio.txt=\"--\"");
         SendDataDisplay();
         printf("desfogue.txt=\"--\"");
         SendDataDisplay();
      }else if(CicloLibre>0 && CicloLibre<21 && estados[CicloLibre-1]){
         printf("ciclo.txt=\"CICLO P%02u\"",CicloLibre+0x0a);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutoest,CiclosPersonalizados[CicloLibre-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutosec,CiclosPersonalizados[CicloLibre-1].Segundosec);
         SendDataDisplay();
         //printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[CicloLibre-1].PulsosVacio);
         printf("pulsos.txt=\"--\"");
         SendDataDisplay();
         //printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[CicloLibre-1].NivelVacio);
         printf("vacio.txt=\"--\"");
         SendDataDisplay();
         printf("desfogue.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].NivelDesfogue);
         //printf("desfogue.txt=\"N/A\"");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page MenuPrincipal");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Ajustes");
            SendDataDisplay();     
         }
      }
      
      if(RX_Buffer[4]==0x20 && CicloLibre>0){//20, Temperatura
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Temperatura");
            SendDataDisplay();     
            Setpoint=CiclosPersonalizados[CicloLibre-1].Temperatura;
         }
      }
      
      if(RX_Buffer[4]==0x30 && CicloLibre>0){//30, Test
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page TiempoEst");
            SendDataDisplay();     
            mEstp=CiclosPersonalizados[CicloLibre-1].Minutoest;
            sEstp=CiclosPersonalizados[CicloLibre-1].Segundoest;
         }
      }
      
      if(RX_Buffer[4]==0x40 && CicloLibre>0){//40, Tsec
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page TiempoSec");
            SendDataDisplay();   
            mSecp=CiclosPersonalizados[CicloLibre-1].Minutosec;
            sSecp=CiclosPersonalizados[CicloLibre-1].Segundosec;
         }
      }
      
      if(RX_Buffer[4]==0x50 && CicloLibre>0){//50, PulsosVacio
         /*
         if(codigoUsuario==0 || codigoUsuario==9){
            printf("page Pulsos");
            SendDataDisplay();     
            Pulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
         }
         */
      }
      
      if(RX_Buffer[4]==0x60 && CicloLibre>0){//60, NivelVacio
         /*
         if(codigoUsuario==0 || codigoUsuario==9){
            printf("page Nivel");
            SendDataDisplay();     
            Nivel=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         }
         */
      }
      
      if(RX_Buffer[4]==0x80 && CicloLibre>0){//80, NivelDesfogue
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page NivelDes");
            SendDataDisplay();     
            NivelDes=CiclosPersonalizados[CicloLibre-1].NivelDesfogue;
            MenuAnt=1;
         }
      }
      
      if(RX_Buffer[4]==0x70 && CicloLibre>0){//70, Iniciar Ciclo
         printf("page Funcionamiento");
         SendDataDisplay();       
         Libre=ON;
         if(CiclosPersonalizados[CicloLibre-1].NivelDesfogue>5){
            ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest,CiclosPersonalizados[CicloLibre-1].Minutoest,
            CiclosPersonalizados[CicloLibre-1].Segundosec,CiclosPersonalizados[CicloLibre-1].MinutoSec,0,CicloLibre+10,
            CiclosPersonalizados[CicloLibre-1].Temperatura,0,0
            ,0);
         }else{
            ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest,CiclosPersonalizados[CicloLibre-1].Minutoest,
            CiclosPersonalizados[CicloLibre-1].Segundosec,CiclosPersonalizados[CicloLibre-1].MinutoSec,1,CicloLibre+10,
            CiclosPersonalizados[CicloLibre-1].Temperatura,0,0
            ,0);
            NivelSeleccionado=4995*CiclosPersonalizados[CicloLibre-1].NivelDesfogue;
         }
         NivelVacio=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         CantidadPulsos=0;
         Fugas=OFF;
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==9){//Menu de Pulsos de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
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
   if(Menu==10){//Menu de Usuarios
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      if(ingreso)
         TipoClave=2;
      else
         TipoClave=0;
      
      if(RX_Buffer[4]==0x01){//01, Admin
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Admin";
            codigoUsuario=0;
         }
         codigoUsuario2=0;
      }else if(RX_Buffer[4]==0x02){//02, Operador 1
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 1";
            codigoUsuario=1;
         }
         codigoUsuario2=1;
      }else if(RX_Buffer[4]==0x03){//03, Operador 2
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 2";
            codigoUsuario=2;
         }
         codigoUsuario2=2;
      }else if(RX_Buffer[4]==0x04){//04, Operador 3
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 3";
            codigoUsuario=3;
         }
         codigoUsuario2=3;
      }else if(RX_Buffer[4]==0x05){//05, Operador 4
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 4";
            codigoUsuario=4;
         }
         codigoUsuario2=4;
      }else if(RX_Buffer[4]==0x06){//06, Operador 5
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 5";
            codigoUsuario=5;
         }
         codigoUsuario2=5;
      }else if(RX_Buffer[4]==0x07){//07, Operador 6
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 6";
            codigoUsuario=6;
         }
         codigoUsuario2=6;
      }else if(RX_Buffer[4]==0x08){//08, Operador 7
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 7";
            codigoUsuario=7;
         }
         codigoUsuario2=7;
      }else if(RX_Buffer[4]==0x09){//09, Operador 8
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 8";
            codigoUsuario=8;
         }
         codigoUsuario2=8;
      }else if(RX_Buffer[4]==0x0a){//0a, Tecnico
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Tecnico";
            codigoUsuario=9;
         }
         codigoUsuario2=9;
      }
      
      if(RX_Buffer[4]==0xaa && ingreso){//aa, Regresar
         printf("page Ajustes");
         SendDataDisplay();       
         codigoUsuario2=10;
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==11){//Menu de Configuraciones
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      TipoClave=0;
      
      if(RX_Buffer[4]==0x01){//01, Ciclos
         printf("page Ciclos");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x02){//02, Test de Componentes
         printf("page Test");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x03){//03, Modificación de clave Usuarios
         printf("page Usuarios");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x04){//03, Modificación de clave Usuarios
         printf("page Activa");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x11 && CodigoUsuario==9){//11, Menu Oculto Tecnico
         printf("page Clave");
         SendDataDisplay(); 
         TipoClave=1;
      }
      
      if(RX_Buffer[4]==0x05){//05, Hora y Fecha
         printf("page Fecha");
         SendDataDisplay();     
         DiaTx=Dia;
         MesTx=Mes;
         YearTx=Year;
         HoraTx=Hora;
         MinutoTx=Minuto;
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page MenuPrincipal");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         printf("page Ajustes");
         SendDataDisplay();     
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==12){//Menu de Visualizacion de Ciclos Realizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      //SendDataDisplay();
      GuardaEEPROM=OFF;
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
         printf("page Ajustes");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[6]==0x02){//02, Exportar
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         exportarData();
      }
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[5]>0x00){
         cicloVisualizado=RX_Buffer[4]-1;
         cicloVisto=RX_Buffer[5]-1;
         RX_Buffer[4]=0x00;
         RX_Buffer[5]=0x00;
         RX_Buffer2[4]=0x00;
         RX_Buffer2[5]=0x00;
         diaCiclos=read_ext_eeprom(2+(cicloVisto*320));
         if(diaCiclos>31)
            diaCiclos=1;
         mesCiclos=read_ext_eeprom(3+(cicloVisto*320));
         if(mesCiclos>12)
            mesCiclos=1;
         yearCiclos=read_ext_eeprom(4+(cicloVisto*320));
         if(yearCiclos>99)
            yearCiclos=19;
         cicloCiclos=read_ext_eeprom(1+(cicloVisto*320));
         if(cicloCiclos>20)
            cicloCiclos=1;
         usuarioCiclos=read_ext_eeprom((cicloVisto*320));
         if(usuarioCiclos>10)
            usuarioCiclos=1;
         for(i=0;i<7;i++){//Lee Valores para completar la Tabla
            horaCiclos[i]=read_ext_eeprom((cicloVisto*320)+11+((cicloVisualizado+i)*9));
            if(horaCiclos[i]>24)
               horaCiclos[i]=0;
            minutoCiclos[i]=read_ext_eeprom((cicloVisto*320)+12+((cicloVisualizado+i)*9));
            if(minutoCiclos[i]>59)
               minutoCiclos[i]=0;
            TCiclos[i]=read_ext_eeprom((cicloVisto*320)+5+((cicloVisualizado+i)*9))+inttofloat(read_ext_eeprom((cicloVisto*320)+6+((cicloVisualizado+i)*9)));
            if(TCiclos[i]>150)
               TCiclos[i]=0;
            PcCiclos[i]=make16(read_ext_eeprom((cicloVisto*320)+7+((cicloVisualizado+i)*9)),read_ext_eeprom((cicloVisto*320)+8+((cicloVisualizado+i)*9)));
            if(PcCiclos[i]>400)
               PcCiclos[i]=0;
            PpcCiclos[i]=make16(read_ext_eeprom((cicloVisto*320)+9+((cicloVisualizado+i)*9)),read_ext_eeprom((cicloVisto*320)+10+((cicloVisualizado+i)*9)));
            if(PpcCiclos[i]>400)
               PpcCiclos[i]=0;
            ACiclos[i]=read_ext_eeprom((cicloVisto*320)+13+((cicloVisualizado+i)*9));
            if(ACiclos[i]>30)
               ACiclos[i]=0;
         }
      }
      
      //Imprime Valores Estaticos
      printf("fecha.txt=\"%02u/%02u/%02u\"",diaCiclos,mesCiclos,yearCiclos);
      SendDataDisplay();
      
      printf("ciclo.txt=\"%02u\"",cicloCiclos);
      SendDataDisplay();
      
      printf("t0.txt=\"Ciclo %02u\"",cicloVisto);
      SendDataDisplay();
      
      if(usuarioCiclos==0){
         printf("usuario.txt=\"Admin\"");
         SendDataDisplay();
      }else if(usuarioCiclos>0 && usuarioCiclos<9){
         printf("usuario.txt=\"Operador%u\"",usuarioCiclos);
         SendDataDisplay();
      }else if(usuarioCiclos==9){
         printf("usuario.txt=\"Tecnico\"");
         SendDataDisplay();
      }
      
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
    if(Menu==13){//Menu de Nivel de Desfogue
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(NivelDes<1)
            NivelDes=6;
         if(NivelDes>6)
            NivelDes=1;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].NivelDesfogue=NivelDes;
               write_eeprom(107+((CicloLibre-1)*8),NivelDes);
               delay_ms(20);            
               GuardaEEPROM=ON;
            }
            
            if(Nivel<6)
               desfoguelento=1;
            else            
               desfoguelento=0;
            if(flagLiquidos){
               printf("page Funcionamiento");
               SendDataDisplay(); 
               NivelSeleccionado=4995*CiclosPersonalizados[CicloLibre-1].NivelDesfogue;
            }else{
               printf("page CicloPersona");
               SendDataDisplay(); 
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            NivelDes++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;                       
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            NivelDes--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
         
      printf("tnivel.txt=\"%u\"",NivelDes);
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==14){//Menu de Activacion de Ciclos
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      
      if(RX_Buffer[4]==0xaa){
         for(i=0;i<30;i++){
            write_eeprom(50+i,estados[i]);
            delay_ms(10);
         }
         printf("page Ajustes");
         SendDataDisplay(); 
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;
      }
      
      for(i=0;i<30;i++){
         if(RX_Buffer[4]==i+1){
            estados[i]=!estados[i];
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(estados[i]){
            printf("b%u.bco=2016",i);
            SendDataDisplay();             
         }else{
            printf("b%u.bco=63488",i);
            SendDataDisplay(); 
         }
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
      printf("t2f.txt=\"%u\"",CicloLibre+10);
      SendDataDisplay();
      printf("t3f.txt=\"Personalizado\"");
      SendDataDisplay();
   }
   
   //if(!Finalizo && !Secando){
      if(pulsoSubir)
         abrePuerta();
      else if(pulsoBajar)
        cierraPuerta(); 
      else 
         detienePuerta();
   //}
         
   if(UPD){//Oprimio Subir Puerta
      pulsoSubir=ON;
      pulsoBajar=OFF;
   }
   
   if(!DND && !UPD){//Solto Botones
      pulsoSubir=OFF;
      pulsoBajar=OFF;
   }
   
   if(DND){//Oprimio Bajar Puerta
      pulsoSubir=OFF;
      pulsoBajar=ON;
   }
      
   if(PresionPreCamara>280 || PresionCamara>280){
      AlarmaPresion=ON;
   }
   
   if(TiempoCiclo>1800 && !Fugas){//ANALIZAR BIEN ESTA ALARMA
      if(TiempoCiclo>3600){
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
   
   if(!TERM){
      delay_ms(1000);
      if(!TERM){
         AlarmaTermostato=ON;
      }
   }
   
   if(!DOOR){
      Paso_off;
      if(!Secando && !Finalizo && !Desfogando){
         AlarmaPuerta=ON;
      }
   }else{
      AlarmaPuerta=OFF;
      if(!Finalizo || !Secando){
         bloqueaPuerta();
      }
   }

   if(!STOP){
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
         Agua_off;
         SSR_off;
         SSR2_off;
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
         Desfogue_off;
         Paso_off;
         //Agua_off;
         SSR_off;
         SSR2_off;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
      }else if(AlarmaLlenado){
            printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Cancelado\"");
            SendDataDisplay();
            Desfogue_off;
            Paso_off;
            Agua_off;
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
         Desfogue_on;
         Paso_off;         
         Agua_off;
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
         Desfogue_on;
         Paso_off;
         Agua_off;
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
         Desfogue_on;
         Paso_off;
         Agua_off;
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
         Desfogue_on;
         Agua_off;
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
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Llenado Generador\"");
         SendDataDisplay();
         Control=0;
         IniciaLlenado=ON;
         Controlando=0;
         Desfogue_off;
         Paso_off;
         SSR_off;
         SSR2_off;
      }else if(!Precalentamiento){
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreCalentando\"");
         SendDataDisplay();
         ControlaPresion(); 
         Control=0;
         codigoAlarma=10;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
         Desfogue_off;
         Agua_off;
         Paso_off;
      }else{
            if(!Esterilizando){
               if(Temperatura>=((float)Setpoint-0.3)){
                  Esterilizando=ON;
               }
               codigoAlarma=12;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Calentando\"");
               SendDataDisplay();
               Control=1;
               Controla();
               Paso_on;
               if(Temperatura<=TTrampa){
                  Desfogue_on;
               }else{
                  Desfogue_off;
               }
               Agua_off;
            }else{
               TiempoCiclo=0;
               if(mEst==0 && sEst==0){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<=20){
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        Desfogue_on;
                        Paso_off;                        
                        Agua_off;
                        SSR2_off;
                        SSR_off;
                        if(PresionCamara<=2){
                           if(Temperatura<=Tapertura){
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
                                    delay_ms(200);
                                    Imprime=10;
                                    flagImprimir=1;
                                 }
                              //if(!SWBL && SWBLO){
                                // abrePuerta();
                              //}else{
                                 desbloqueaPuerta();
                              //}
                              incrementaCiclos();
                           }
                        }
                     }else if(PresionCamara>20){
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
                        Paso_on;
                        Agua_off;
                        SSR2_off;
                        SSR_off;
                     }
                  }else{                     
                     Paso_off;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<=2){  
                        if(sSec==0 && mSec==0){                          
                           codigoAlarma=15;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }
                           Agua_off;
                           Desfogue_on;
                           Paso_off;
                           SSR2_off;
                           SSR_off;
                           Secando=OFF;
                           if(PresionCamara<=1){
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
                                 delay_ms(200);
                                 Imprime=10;
                                 flagImprimir=1;
                              }
                              desbloqueaPuerta();
                           }
                           incrementaCiclos();
                           Secando=OFF;
                        }else{
                           /*
                           if(!SWBL && SWBLO){
                              if(tAbreSecado<=5){
                                 abrePuerta();
                                 flagAbreSecado=1;
                              }else{
                                 flagAbreSecado=0;
                                 detienePuerta();
                              }
                           }else{
                              desbloqueaPuerta();
                           }
                           */
                           desbloqueaPuerta();
                           Secando=ON;
                           if(Secando){
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Secando\"");
                              SendDataDisplay();
                           }
                           ControlaPresion();
                           SSR2_on;
                           Paso_off;
                        }
                     }else if(PresionCamara>15){
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
                        Paso_off;
                        Agua_off; 
                        SSR2_on;
                        SSR_off;   
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
                  Agua_off;
                  SSR2_off;
                  Control=1;
                  Controla();
               }
            }
         }
      }
   }

   if(tguarda>=180){
      if(!Ciclo)
         GuardaDatoCiclo();
      tguarda=0;
   }

   if(Grafica==1){
      printf("add 14,0,%1.0f",Temperatura*0.6);
      SendDataDisplay();
      printf("add 14,1,%1.0f",PresionCamara*0.6);
      SendDataDisplay();
      printf("add 14,2,%1.0f",PresionPreCamara*0.6);
      SendDataDisplay();
      Grafica=0;
   }
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      printf("tuser.txt=\"%s\"",1);
      SendDataDisplay();
      printf("t6f.txt=\"%3.1f°\"",Temperatura);
      SendDataDisplay();
      printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
      SendDataDisplay();
      printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
      SendDataDisplay();
      printf("Funcionamiento.t9f.txt=\"%03u\"",Setpoint);
      SendDataDisplay();      
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
      //printf("fecha.font=2");
      //SendDataDisplay();
   }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuración de Parametros de Control
     
      if(Opcion2>13)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=13;
      
      if(Opcion2==1){
         printf("Config.t2c.txt=\"Proporcional\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Ganancia);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               Ganancia+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               Ganancia-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
         
         if(Ganancia>40)
            Ganancia=1;
         if(Ganancia<1)
            Ganancia=40;
      }else if(Opcion2==2){
         printf("Config.t2c.txt=\"Tciclo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Tciclo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               Tciclo+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida            
               Tciclo-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
         
         if(Tciclo>20)
            Tciclo=5;
         if(Tciclo<5)
            Tciclo=20;
      }else if(Opcion2==3){
         printf("Config.t2c.txt=\"Ajuste +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida            
               if(Sensor[0].Ajuste<40)
                  Sensor[0].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();            
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida            
               if(Sensor[0].Ajuste>0)
                  Sensor[0].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
      }else if(Opcion2==4){
         printf("Config.t2c.txt=\"Ajuste -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida            
               if(Sensor[0].Ajusten<40)
                  Sensor[0].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();            
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida            
               if(Sensor[0].Ajusten>0)
                  Sensor[0].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
      }else if(Opcion2==5){
         printf("Config.t2c.txt=\"Ajuste2 +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               if(Sensor[1].Ajuste<40)
                  Sensor[1].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(Sensor[1].Ajuste>0)
                  Sensor[1].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();            
         }
      }else if(Opcion2==6){
         printf("Config.t2c.txt=\"Ajuste2 -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida            
               if(Sensor[1].Ajusten<40)
                  Sensor[1].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida            
               if(Sensor[1].Ajusten>0)
                  Sensor[1].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
      }else if(Opcion2==7){
         printf("Config.t2c.txt=\"Media\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",MediaMovil);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               if(MediaMovil<80)
                  MediaMovil++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(MediaMovil>10)
                  MediaMovil--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();            
         }
      }else if(Opcion2==8){
         printf("Config.t2c.txt=\"Derivativo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",Derivativo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a ){//Tecla Arriba Oprimida
               Derivativo++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer(); 
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               Derivativo--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
         }
         
         
         if(Derivativo>40)
            Derivativo=1;
         if(Derivativo<1)
            Derivativo=40;
      }else if(Opcion2==9){
         printf("Config.t2c.txt=\"Integral\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",Integral);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               Integral++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               Integral--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
         
         if(Integral>40)
            Integral=1;
         if(Integral<1)
            Integral=40;
      }else if(Opcion2==10){
         printf("Config.t2c.txt=\"Ciclos\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03Lu\"",Ciclos);
         SendDataDisplay();
      }else if(Opcion2==11){
         printf("Config.t2c.txt=\"Atmosferica\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03u\"",Atmosferica);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               if(Atmosferica<100)
                  ++Atmosferica;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(Atmosferica>50)
                  --Atmosferica;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
      }else if(Opcion2==12){
         printf("Config.t2c.txt=\"T. Apertura\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03u\"",Tapertura);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               if(Tapertura<100)
                  ++Tapertura;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(Tapertura>50)
                  --Tapertura;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();           
         }
      }else if(Opcion2==13){
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
      }
      
      if(RX_Buffer[4]==0x0c){//Tecla Izquierda Oprimida
            Opcion2++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();   
      }
         
      if(RX_Buffer[4]==0x0d){//Tecla Derecha Oprimida         
            Opcion2--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
      }
      
      if(RX_Buffer[4]==0x11){//Tecla Regresar
         delay_ms(20);
         if(RX_Buffer[4]==0x11){
            if(!GuardaEEPROM){
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
               write_eeprom(27,(int8)Integral);
               delay_ms(10);
               write_eeprom(28,(int8)Derivativo);
               delay_ms(10);
               write_eeprom(31,Atmosferica);
               delay_ms(10);
               write_eeprom(32,Tapertura);
               delay_ms(10);
               write_eeprom(39,TTrampa);
               delay_ms(10);
               GuardaEEPROM=ON;
            }
            printf("page MenuPrincipal");
            SendDataDisplay();
            //BorraBuffer();
         }
      }
      printf("fecha.txt=\"T:%2.1f°C Fecha:%02u/%02u/20%02u Hora:%02u:%02u:%02u\"",Temperatura,Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay(); 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
   }
}
