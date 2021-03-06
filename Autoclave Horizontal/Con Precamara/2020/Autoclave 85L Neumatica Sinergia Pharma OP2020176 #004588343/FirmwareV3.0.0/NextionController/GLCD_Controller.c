//Falta crear menu de ajuste de variables y de norma CFR21
//Falta revisar niveles de agua con variable calderin lleno de bomba de agua

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

#define   STOP          !input(PIN_B0)
#define   DOORS         !input(PIN_B1)
#define   LOWB          !input(PIN_B2)
#define   EMPAQUES      !input(PIN_B3)

#define   UP            !input(PIN_B4)
#define   DOWN          !input(PIN_B5)
#define   RIGHT         !input(PIN_B6)
#define   LEFT          !input(PIN_B7)

//#define   EMPAQUEL      !input(PIN_B4)
//#define   DOORL         !input(PIN_B5)
//#define   BLOQUEOS      !input(PIN_B6)
//#define   BLOQUEOL      !input(PIN_B7)

// Variables creada para no eliminar entradas creadas del programa anterior
short EMPAQUEL,DOORL,BLOQUEOS,BLOQUEOL;
#define OFF 0
#define ON  1
#define RX_BUFFER_SIZE  10
char Rx_Buffer[RX_BUFFER_SIZE+1];
char Rx_Buffer2[RX_BUFFER_SIZE+1];
char RX_Wr_Index=0;

//Comunicacion//------------------------
byte txbuf[0X25];
byte SlaveB0Tx[0X26];
byte SlaveC0Tx[0X25];
byte SlaveC0Rx[0X25];
byte dato=0;            //Contendr? la informaci?n a enviar o recibir
byte direccion=0;       //Contendr? la direcci?n del esclavo en el bus I2C
byte posicion=0;        //Contendr? la posici?n en el buffer de memoria del esclavo donde se almacenar? o de donde se recuperar? el dato
//byte ByteConfig1=0,ByteConfig2=0,ByteConfig3=0;

/* Tabla de Modulos, Primera Fila A?o Regular y el siguiente a?o Bisiesto
E   F   M   A   M   J   J   A   S   O   N   D
0   3   3   6   1   4   6   2   5   0   3   5
0   3   4   0   2   5   0   3   6   1   4   6
*/
int8 Regular[12]={0,3,3,6,1,4,6,2,5,0,3,5};
int8 Bisiesto[12]={0,3,4,0,2,5,0,3,6,1,4,6};

//Salidas//------------------------
short suministroAire=OFF,aguaEnfriador=OFF,bombaVacio=OFF,empaqueLimpio=OFF,empaqueSucio=OFF,suministroVapor=OFF,Paso=OFF,outVacio=OFF,Desfogue=OFF,Aire=OFF,cilindroSucio=OFF,cilindroLimpio=OFF,Aux=OFF;
short seguroValvula=OFF,seguroValvula2=OFF,EstadoAux1=OFF,EstadoAux2=OFF;

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaTermostato=OFF,AlarmaPuertaS=OFF,AlarmaPuertaL=OFF,AlarmaEmergencia=OFF,AlarmaVacio=OFF,AlarmaEmpaqueL=OFF,AlarmaEmpaqueS=OFF;
short AlarmaBomba=OFF,guardaCiclo=OFF,Cilindro=OFF,AlarmaLlenado=OFF,cerroSucio=OFF,cerroLimpio=OFF,pulsoSubirS=OFF,pulsoSubirL=OFF,vacioEmpaqueL=OFF,vacioEmpaqueS=OFF,abrirPuertaL=OFF,abrirPuertaS=OFF,pulsoBajarS=OFF;
int16 TiempoVacio=600,tVacioEmpaqueS=0,tVacioEmpaqueL=0;

//Banderas//--------------------
short EstadoAguaEnfriador=OFF,EstadoSuministroV=OFF,EstadoCilindrosL=OFF,EstadoCilindrosS=OFF,EstadoEmpaqueL=OFF,EstadoEmpaqueS=OFF,oprimioIniciar=OFF,oprimioCiclo=OFF,oprimioCicloP=OFF,oprimioConfig=OFF;
short IniciaLlenado=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF,CalderinLleno=OFF,Start=OFF;
short EstadoDesfogue=OFF,EstadoSuministro=OFF,EstadoAire=OFF,PruebaEstados=OFF,disp_Guarda=OFF,Ciclo=OFF,EstadoAgua2=OFF;
short Op=OFF,Op2=OFF,Minutos=OFF,Segundos=OFF,GuardaEEPROM=OFF,EstadoPaso=OFF,EstadoVacio=OFF,EstadoBuzzer=OFF,flagAlarma=OFF;
short flagBuzzer=OFF,flagSilencio=OFF,Finalizo=OFF,AlarmaPresion=OFF,EstadoACierre=OFF,EstadoBomba=OFF,Libre=OFF,ingreso=OFF,Fugas=OFF;
short PreCalentamiento=OFF,IniciaVacio=OFF,IniciaVacio2=OFF,Vacio=OFF,flag_vac=OFF,flag_vacio=OFF,flag_pulso=OFF,EstadoAireBomba=OFF;
short SensadoNivelL=OFF,NivelBajo=OFF,onBomba=OFF,flagBombaAgua=OFF,flagBombaAguaOff=OFF,flagImprimir=OFF,entro=OFF;
short EstadoEmpaque=OFF,EstadoSSR1=OFF,EstadoSSR2=OFF,ReservorioLleno=OFF,SensadoNivelR=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoSensadoNivel=0,Tp=0,Tc=0,tguarda=0,tNivelBajo=0,tIniciar=0,tCiclof=0,tCicloP=0,tConfig=0,TiempoLlenado=OFF;
int8 sEst=0,mEst=0,sSec=0,mSec=0,sEstp=0,mEstp=0,sSecp=0,mSecp=0;// Tiempo transcurrido

//Control//------------------------
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Temporal=0.0,Aumento=0.0,tmp2=0.0;
float K=0.0012858;
float R10=6800,AN=0,BN=0;
float X[2]={0.0,0.0};
float promediopresion=0.0,promediopresionp=0.0;
int8 h=0,l=0,MediaMovil=20,PantallaPrincipal=0,TipoClave=0,Modulo=0,TiempoSensadoNivelR=0,subirPuertaLimpio=0,cerroSucioInt=0,cerroLimpioInt=0;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,Temperatura2=0.0;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0,CicloLibre=0;
float p1=0.0000011617,p2=-0.0010575,p3=0.44114,p4=65.043,Tactual=0.0;
//float p1=-0.00036062,p2=0.30947,p3=72.816,Tactual=0.0;

//Varios//--------------------------------
short desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagImprime=0;
int8 Control=0,tiemporecta=0,Controlando=0,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0,Codigo=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,i=0,CiclosL=0,CiclosH=0,CicloSeleccionado=0,Tapertura=0;
int16 CantidadPulsos=4,tEmpaqueS=0,tEmpaqueL=0,tEnfriador=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,tbomba=0,tvacio=0,tiempoImpresion=0,PresionFinal=0;
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0,NivelDes=0;
int8 y=0,ConteoBuzzer=0,posicionDato=0,codigoAlarma=0,codigoAnt=0,codigoUsuario2=0;
int16 tbuzzer=0,tsilencio=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0,cicloVisualizado=0,cicloVisto=0,NivelVacio=25,Atmosferica=75,PulsoPositivo=10,PulsosVacio=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,dowTx=0,SetTime=0,Imprime=0,Pulsos=0,j=0,codigoUsuario=0,codigoUsuarioant=0;
signed int8 G1=0,G2=0,sg1=0,sg2=0;
const char Texto[12][20]={"CALENTAMIENTO","NO ENVUELTOS 134","NO ENVUELTOS 121","ENVUELTOS 134","ENVUELTOS 121","ENVUELTO DOBLE 1",
"ENVUELTO DOBLE 2","PRION","BOWIE & DICK","TEST DE FUGAS","TEST DE COMPONENTES","HORA Y FECHA"};

//Textil & Instrumental 134?C, 5m,15m
//Prion 134?C, 20m,15m
//Caucho 121?C, 25m,15m
//Contenedores 134?C, 5m,25m
//Rapid 134?C, 5m,8m

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
   int8 Minutoest;
   int8 Segundoest;
   int8 Minutosec;
   int8 Segundosec;
   int8 PulsosVacio;
   int8 NivelVacio;
   int8 NivelDesfogue;
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
      if(Rx_Buffer2[0]==0x65 && Rx_Buffer2[1]==0xff && Rx_Buffer2[2]==0xff && Rx_Buffer2[8]==0x00 && Rx_Buffer2[9]==0xff ){
         Dato_Exitoso=5;
      }else{
         Dato_Exitoso=10;
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
   
   if(flagAlarma)
      tbuzzer++;
      
   if(flagSilencio)   
      tsilencio++;

   if(tiempos>=200){  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
      if(flagTC)
         TiempoControl++;
      
      tiemporecta++;
      
      if(IniciaCiclo && tiempoImpresion<CalculoImpresion){
         tiempoImpresion++;
      }else if(tiempoImpresion>=CalculoImpresion){
         flagImprime=1;
         tiempoImpresion=0;
      }
      
      if(oprimioIniciar)
         tIniciar++;
      else
         tIniciar=0;
         
      if(oprimioCiclo)
         tCiclof++;
      else
         tCiclof=0;
      
      if(oprimioCicloP)
         tCicloP++;
      else
         tCicloP=0;
         
      if(oprimioConfig)
         tConfig++;
      else
         tConfig=0;
      
      if(vacioEmpaqueL)
         tVacioEmpaqueL++;
      else
         tVacioEmpaqueL=0;
      
      if(vacioEmpaqueS)
         tVacioEmpaqueS++;
      else
         tVacioEmpaqueS=0;
         
      if(empaqueLimpio && !EMPAQUEL)
         tEmpaqueL++;
      else
         tEmpaqueL=0;
      
      if(empaqueSucio && !EMPAQUES)
         tEmpaqueS++;
      else
         tEmpaqueS=0;
      
      if(IniciaLlenado && !CalderinLleno)
         TiempoLlenado++;
      
      if(SensadoNivel)
         TiempoSensadoNivel++;
         
      if(SensadoNivelR)
         TiempoSensadoNivelR++;   
      
      if(guardaCiclo)
         tguarda++;
      
      if(IniciaCiclo && !Esterilizando && DOORS && DOORL)
         TiempoCiclo++;
      
      if(SensadoNivel)
         TiempoSensadoNivel++;
         
      if(SensadoNivelL)
         tNivelBajo++;
      
      if(IniciaVacio)
         tvacio++;         
      
      if(IniciaVacio2)
         tbomba++;
         
      if(EstadoAgua2)   
         tEnfriador++;
      
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

void main()
{
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc(ADC_CLOCK_DIV_64);
   setup_adc_ports(AN0_TO_AN5);
   port_b_pullups(true);
   enable_interrupts(int_rda);
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   LeeEEPROM();
   rtc_init();
   delay_ms(1);
   LimitaValores();
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(3500); 
   printf("page Usuarios");
   SendDataDisplay();
   ApagaSalidas();   
   TiempoVacio=360; 
   Sensor[0].Gain=106.07;
   Sensor[1].Gain=106.07;
   Imprime=10;
   CalculoImpresion=timpresion*60;
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(0,MediaMovil);
      Temperatura2=LeerPT100(1,MediaMovil);
      PresionCamara=Leer_Sensor_Presion_Camara(20);
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(20);
   }
   seguroValvula=OFF;
   seguroValvula2=OFF;
   if(!DOORS){
      abrePuertaS();
   }else{
      cierraPuertaS();
   }
         
   //if(ingreso){
   if(!DOORL){
      //abrePuertaL();
      monitoreaPuertaLimpio();
   }else{
      cierraPuertaL();
   }
   
   while(true){
      
      LeeDisplay();
      Envio_Esclavos();
      delay_ms(10);
      Lectura_Esclavos();
      rtc_get_date(Dia,Mes,Year,dow);
      rtc_get_time(Hora,Minuto,Segundo);
      restart_wdt();
      if(!Esterilizando || Desfogando){
         CalculoImpresion=180;
      }else{
         CalculoImpresion=60;
      }
      
      graficaCurva(2000);
      muestreo(20);
      monitoreaAgua();
      monitoreaStop();
      
      /*
      if(Menu!=20 && !PruebaEstados){
         if(!DOORS){
            abrePuertaS();
         }else{
            cierraPuertaS();
         }
         
         //if(ingreso){
            if(!DOORL){
               //abrePuertaL();
               monitoreaPuertaLimpio();
            }else{
               cierraPuertaL();
            }
         //}
      }
      
      if(ingreso && !PruebaEstados)
         monitoreaPuertaLimpio();
         
      if(PresionCamara>Atmosferica+15){
         cierraPuertaS();
         cierraPuertaL();
      }
      */


//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(1500);
      printf("page Usuarios");
      SendDataDisplay();
      ApagaSalidas();
   }else if(Menu==0){ //Menu de Contrase?a de acceso.
      entro=OFF;
      ApagaSalidas();
      //printf("bkcmd=0");
      //SendDataDisplay();
      if(RX_Buffer[4]==0x11){//11
         if(TipoClave!=0){
            printf("page Menu");
            SendDataDisplay();
            PantallaPrincipal=0;
         }
      }
      
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
               for(i=0;i<40;i++){//Lee contrase?as de usuarios
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
               printf("page Menu");
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
                  printf("page Menu");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;
                  RX_Buffer2[3]=0x00;
                  codigoUsuario=codigoUsuarioant;
               } 
            }
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page Menu");
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
    }else if(Menu==1){ //Menu Principal.
      tCiclof=0;
      oprimioCiclo=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      codigoUsuarioant=codigoUsuario;
      
      if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Calentamiento
         delay_ms(5);
         if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Calentamiento
            printf("page Funcionamiento");
            SendDataDisplay();
            ConfiguraCiclo(0,15,0,1,0,1,121,6,25,0);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//02, No Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,3,0,1,0,2,134,6,25,0);            
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x03  && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//03, No Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x03 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();     
            ConfiguraCiclo(0,15,0,1,0,3,121,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//04, Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,4,0,15,0,4,134,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//05, Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,15,0,5,121,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=7;
            NivelVacio=32;
            Fugas=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//06, Envuelto Doble 1
         delay_ms(5);
         if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,7,0,20,0,6,134,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//07, Envuelto Doble 2
         delay_ms(5);
         if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,7,0,30,0,7,134,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//08, Prion
         delay_ms(5);
         if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,20,0,15,0,8,134,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//09, Bowie & Dick
         delay_ms(5);
         if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(30,3,0,1,0,9,134,6,25,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//0a, Test de Fugas
         delay_ms(5);
         if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,0,0,10,30,1,17,1);
            MenuAnt=Menu;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Ciclos Fijos
         printf("page Menu");
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
   }else if(Menu==2){ //Menu de Tiempo Esterilizacion     
         printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
    }else if(Menu==3){ //Menu de Tiempo de Secado
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
    }else if(Menu==4){ //Menu de Temperatura
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Setpoint<110)
            Setpoint=138;
         if(Setpoint>138)
            Setpoint=110;
         
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
    }else if(Menu==5){//Menu de Nivel de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Nivel<25)
            Nivel=100;
         if(Nivel>100)
            Nivel=25;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
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
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Nivel++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Nivel--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tnivel.txt=\"%Lu\"",Nivel);
      SendDataDisplay();
   }else if(Menu==6){ // Menu de Prueba de Componentes
         PruebaEstados=ON;
         
         printf("t0.txt=\"Test %02u\"",Codigo);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x01){//Oprimio EV Suministro
            EstadoSuministro=!EstadoSuministro;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSuministro){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            suministroAire=ON;
         }else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            suministroAire=OFF;
         }
         
         if(RX_Buffer[4]==0x02){//Oprimio EV Desfogue
            EstadoDesfogue=!EstadoDesfogue;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoDesfogue){
            printf("b1.bco=2016");
            SendDataDisplay(); 
            Desfogue=ON;
         }else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            Desfogue=OFF;
         }
         
         if(RX_Buffer[4]==0x03){//Oprimio EV Paso
            EstadoPaso=!EstadoPaso;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoPaso){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            Paso=ON;
         }else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            Paso=OFF;
         }
        
         if(RX_Buffer[4]==0x04){//Oprimio Vacio
            EstadoVacio=!EstadoVacio;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoVacio){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            outVacio=ON;
         }else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            outVacio=OFF;
         }
         
         if(RX_Buffer[4]==0x05){//Oprimio Aire
            EstadoAire=!EstadoAire;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAire){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            Aire=ON;
         }else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            Aire=OFF;
         }
         
         if(RX_Buffer[4]==0x06){//Oprimio Bomba Vacio
            EstadoBomba=!EstadoBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBomba){
            printf("b5.bco=2016");
            SendDataDisplay(); 
            if(ReservorioLleno)
               bombaVacio=ON;
            else
               bombaVacio=OFF;
         }else{
            printf("b5.bco=63488");
            SendDataDisplay(); 
            bombaVacio=OFF;
         }
         
         if(RX_Buffer[4]==0x07){//Oprimio EV Agua
            EstadoAguaEnfriador=!EstadoAguaEnfriador;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAguaEnfriador){
            printf("b7.bco=2016");
            SendDataDisplay(); 
            aguaEnfriador=ON;
         }else{
            printf("b7.bco=63488");
            SendDataDisplay(); 
            aguaEnfriador=OFF;
         }
         
         if(RX_Buffer[4]==0x08){//Oprimio EV Suministro Vapor
            EstadoSuministroV=!EstadoSuministroV;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSuministroV){
            printf("b6.bco=2016");
            SendDataDisplay(); 
            suministroVapor=ON;
         }else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
            suministroVapor=OFF;
         }
         
         if(RX_Buffer[4]==0x09){//Oprimio Abrir Cierre
            EstadoCilindrosL=!EstadoCilindrosL;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoCilindrosL){
            printf("b12.bco=2016");
            SendDataDisplay(); 
            cilindroLimpio=OFF;
         }else{
            printf("b12.bco=63488");
            SendDataDisplay(); 
            cilindroLimpio=ON;
         }
         
         if(RX_Buffer[4]==0x0a){//Oprimio Empaque
            EstadoEmpaqueL=!EstadoEmpaqueL;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoEmpaqueL && DOORL){
            printf("b16.bco=2016");
            SendDataDisplay(); 
            empaqueLimpio=ON;            
         }else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
            empaqueLimpio=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Oprimio Abrir Cierre
            EstadoCilindrosS=!EstadoCilindrosS;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoCilindrosS){
            printf("b17.bco=2016");
            SendDataDisplay(); 
            cilindroSucio=OFF;
         }else{
            printf("b17.bco=63488");
            SendDataDisplay(); 
            cilindroSucio=ON;
         }
         
         if(RX_Buffer[4]==0x0c){//Oprimio Empaque
            EstadoEmpaqueS=!EstadoEmpaqueS;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoEmpaqueS && DOORS){
            printf("b25.bco=2016");
            SendDataDisplay(); 
            empaqueSucio=ON;            
         }else{
            printf("b25.bco=63488");
            SendDataDisplay(); 
            empaqueSucio=OFF;
         }
         
         if(RX_Buffer[4]==0x0d){//Oprimio Buzzer
            EstadoBuzzer=!EstadoBuzzer;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBuzzer){
            printf("b21.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b21.bco=63488");
            SendDataDisplay(); 
         }
         
         if(RX_Buffer[4]==0x0e){//Oprimio Aux1
            EstadoAux1=!EstadoAux1;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAux1){
            seguroValvula=ON;
            printf("b22.bco=2016");
            SendDataDisplay(); 
         }else{
            seguroValvula=OFF;
            printf("b22.bco=63488");
            SendDataDisplay(); 
         }
         
         if(RX_Buffer[4]==0x0f){//Oprimio Aux2
            EstadoAux2=!EstadoAux2;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAux2){
            seguroValvula2=ON;
            printf("b28.bco=2016");
            SendDataDisplay(); 
         }else{
            seguroValvula2=OFF;
            printf("b28.bco=63488");
            SendDataDisplay(); 
         }
         
         if(DOORS){
            printf("b8.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b8.bco=63488");
            SendDataDisplay(); 
         }
         
         if(DOORL){
            printf("b26.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b26.bco=63488");
            SendDataDisplay(); 
         }
         
         if(EMPAQUEL){
            printf("b9.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b9.bco=63488");
            SendDataDisplay(); 
         }
         
         if(EMPAQUES){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }
         
         if(STOP){
            printf("b10.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b10.bco=63488");
            SendDataDisplay(); 
         }
         
         if(LOWB){
            printf("b23.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b23.bco=63488");
            SendDataDisplay(); 
         }
         
         if(BLOQUEOL){
            printf("b24.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b24.bco=63488");
            SendDataDisplay(); 
         }
         
         if(BLOQUEOS){
            printf("b27.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b27.bco=63488");
            SendDataDisplay(); 
         }         
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"A4:%3.0f %03Lu\"",sensores(4),PresionPreCamara);
         SendDataDisplay();
         printf("b18.txt=\"A5:%3.0f %03Lu\"",sensores(5),PresionCamara);
         SendDataDisplay();
         printf("b19.txt=\"T2:%3.1f\"",Temperatura2);
         SendDataDisplay();
         printf("b20.txt=\"T1:%3.1f\"",Temperatura);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x21)//Oprimio salir
         {
            printf("page Menu");
            SendDataDisplay();
            PantallaPrincipal=0;
            PruebaEstados=OFF;
            ApagaSalidas();
            // Falta apagar Todo
         }
     }else if(Menu==7){ //Menu de Configuraci?n de Fecha y Hora
      
         if(RX_Buffer[4]==0x0a)//Selecciono Hora
            Opcion=4;
         
         if(RX_Buffer[4]==0x0b)//Selecciono Minuto
            Opcion=5;
         
         if(RX_Buffer[4]==0x0c)//Selecciono Dia
            Opcion=1;
         
         if(RX_Buffer[4]==0x0d)//Selecciono Hora
            Opcion=2;
         
         if(RX_Buffer[4]==0x0e)//Selecciono Year
            Opcion=3;
         
         if(Opcion>5)
            Opcion=1;
         if(Opcion<1)
            Opcion=5;
            
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            printf("page Menu");
            SendDataDisplay();  
            PantallaPrincipal=0;
            Op=OFF;
            Op2=OFF;
            
            if(esBisiesto(YearTx))
               Modulo=Bisiesto[MesTx];
            else
               Modulo=Regular[MesTx];
         
            dowTx=((YearTx-1)%7+((YearTx-1)/4-3*((YearTx-1)/100+1)/4)%7+Modulo+DiaTx%7)%7;
            SetTime=5;
            Envio_Esclavos();
            delay_ms(100);   
            SetTime=10;
            rtc_set_datetime(DiaTx,MesTx,YearTx,dowTx,HoraTx,MinutoTx);
         }
         
         if(RX_Buffer[4]==0x2a){//Tecla Arriba Oprimida
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
         
         if(RX_Buffer[4]==0x2b){//Tecla Abajo Oprimida 
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
               //BorraBuffer();
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
    }else if(Menu==8){ //Menu Ciclos Personalizados
      tCicloP=0;
      oprimioCicloP=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelVacio);
         SendDataDisplay();
         //printf("desfogue.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelDesfogue);
         printf("desfogue.txt=\"N/A\"");
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
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[CicloLibre-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[CicloLibre-1].NivelVacio);
         SendDataDisplay();
         //printf("desfogue.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].NivelDesfogue);
         printf("desfogue.txt=\"N/A\"");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page Menu");
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
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Pulsos");
            SendDataDisplay();     
            Pulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
         }
      }
      
      if(RX_Buffer[4]==0x60 && CicloLibre>0){//60, NivelVacio
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Nivel");
            SendDataDisplay();     
            Nivel=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         }
      }
      
      if(RX_Buffer[4]==0x80 && CicloLibre>0){//80, NivelDesfogue
         //printf("page NivelDes");
         //SendDataDisplay();     
         //NivelDes=CiclosPersonalizados[CicloLibre-1].NivelDesfogue;
      }
      
      if(RX_Buffer[4]==0x70 && CicloLibre>0){//70, Iniciar Ciclo
         printf("page Funcionamiento");
         SendDataDisplay();       
         Libre=ON;
         ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest,CiclosPersonalizados[CicloLibre-1].Minutoest,
         CiclosPersonalizados[CicloLibre-1].Segundosec,CiclosPersonalizados[CicloLibre-1].MinutoSec,0,CicloLibre+10,
         CiclosPersonalizados[CicloLibre-1].Temperatura,CiclosPersonalizados[CicloLibre-1].PulsosVacio,CiclosPersonalizados[CicloLibre-1].NivelVacio
         ,0);
         NivelVacio=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         CantidadPulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
         Fugas=OFF;
      }
   }else if(Menu==9){//Menu de Pulsos de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Pulsos<4)
            Pulsos=15;
         if(Pulsos>15)
            Pulsos=4;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].PulsosVacio=Pulsos;
               write_eeprom(105+((CicloLibre-1)*8),Pulsos);
               delay_ms(20);            
               //NivelSeleccionado=9990*Nivel;
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Pulsos++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Pulsos--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tnivel.txt=\"%u\"",Pulsos);
      SendDataDisplay();
   }else if(Menu==10){//Menu de Usuarios
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
   }else if(Menu==11){//Menu de Configuraciones
      tConfig=0;
      oprimioConfig=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
      
      if(RX_Buffer[4]==0x03){//03, Modificaci?n de clave Usuarios
         printf("page Usuarios");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x04){//03, Modificaci?n de clave Usuarios
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
         printf("page Menu");
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
   }else if(Menu==12){//Menu de Visualizacion de Ciclos Realizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      //SendDataDisplay();
      GuardaEEPROM=OFF;
      
      i2c_start();            // Comienzo de la comunicaci?n
      i2c_write(0xB0);   // Direcci?n del esclavo en el bus I2C
      i2c_write(9);    // Posici?n de donde se leer? el dato en el esclavo
      i2c_write(0);    // Posici?n de donde se leer? el dato en el esclavo
      i2c_start();            // Reinicio
      i2c_write(direccion+1); // Direcci?n del esclavo en modo lectura
      dato=i2c_read(0);       // Lectura del dato
      i2c_stop();
      
      printf("estado.txt=\"Oprima Exportar para imprimir\"");
      SendDataDisplay();     
      
      if(RX_Buffer[6]==0x01){//01, Ciclos
         printf("page Ajustes");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[6]==0x02){//02, Exportar
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         printf("estado.txt=\"!Imprimiendo!\"");
         SendDataDisplay();
         exportarData();
      }
      
      if(RX_Buffer[4]>0x00 || RX_Buffer[5]>0x00 || UP || DOWN || RIGHT || LEFT){
         if(UP){
            if(cicloVisualizado<28){
               cicloVisualizado++;
            }
            delay_ms(20);
         }else if(DOWN){
            if(cicloVisualizado>0){
               cicloVisualizado--;
            }
            delay_ms(20);
         }else if(RIGHT){
            if(cicloVisto<200){
               cicloVisto++;
            }
            delay_ms(20);            
         }else if(LEFT){
            if(cicloVisto>0){
               cicloVisto--;
            }
            delay_ms(20);
         }else{
            cicloVisualizado=RX_Buffer[4]-1;
            cicloVisto=RX_Buffer[5]-1;
            RX_Buffer[4]=0x00;
            RX_Buffer[5]=0x00;
            RX_Buffer2[4]=0x00;
            RX_Buffer2[5]=0x00;
         }
         printf("h0.val=%u",cicloVisto+1);
         SendDataDisplay();   
         printf("h1.val=%u",cicloVisualizado+1);
         SendDataDisplay();   
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
            if(ACiclos[i]>20)
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
         printf("T%u.txt=\"%3.1f?C\"",i+1,TCiclos[i]);
         SendDataDisplay();
         printf("Pc%u.txt=\"%03LukPa\"",i+1,PcCiclos[i]);
         SendDataDisplay();
         printf("Ppc%u.txt=\"%03LukPa\"",i+1,PpcCiclos[i]);
         SendDataDisplay();         
         codificaAlarma();
      }  
   }else if(Menu==13){//Menu de Nivel de Desfogue
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(NivelDes<1)
            NivelDes=6;
         if(NivelDes>6)
            NivelDes=1;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].NivelDesfogue=NivelDes;
               write_eeprom(107+((CicloLibre-1)*8),NivelDes);
               delay_ms(20);            
               GuardaEEPROM=ON;
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
   }else if(Menu==14){//Menu de Activacion de Ciclos
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
   }else if(Menu==15){//Menu de Clave Correcta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page Menu");
      //printf("page Funcionamiento");
      SendDataDisplay();
   }else if(Menu==16){//Menu de Clave InCorrecta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page Clave");
      SendDataDisplay();
   }else if(Menu==20){ // Este es el menu de cuando el proceso se inicio.FUNCIONAMIENTO
   
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
   
   leePulsadores();
   /*
   if(PresionCamara>Atmosferica+15){
      cierraPuertaS();
      cierraPuertaL();
   }
   */
   if(!Start){
   
      /*
      if(pulsoSubirS)
         cierraPuertaS();
      
      if(pulsoBajarS)
         abrePuertaS();
         
      if(RX_Buffer[4]==0x0a){//Oprimio Subir Puerta
         //cierraPuertaS();
         pulsoSubirS=ON;
         pulsoBajarS=OFF;
         RX_Buffer[4]=0x00;
         //RX_Buffer2[4]=0x00;                       
      }
      
      if(RX_Buffer[4]==0x0b){//Solto Subir Puerta
         RX_Buffer[4]=0x00;            
         //RX_Buffer2[4]=0x00;           
         pulsoSubirS=OFF;
         pulsoBajarS=OFF;
         if(!DOORS && !EMPAQUES)
            abrePuertaS();
         else
            cierraPuertaS();
      }
      
      if(RX_Buffer[4]==0x0c){//Oprimio Bajar Puerta
         pulsoSubirS=OFF;
         RX_Buffer[4]=0x00;
         //RX_Buffer2[4]=0x00; 
         if(Temperatura<Tapertura && cerroLimpio)
            pulsoBajarS=ON;
      }   
      
      if(RX_Buffer[4]==0x0d){//Solto Bajar Puerta
         RX_Buffer[4]=0x00;            
         //RX_Buffer2[4]=0x00;                       
      }
      */
   }
         
   if(PresionPreCamara>(350-Atmosferica) || PresionCamara>350){
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
   
   if(TiempoLlenado>600){
      AlarmaLlenado=ON;
   }else{
      AlarmaLlenado=OFF;
   }
   
   //if(!DOORS && !EMPAQUES){      
   if(!DOORS){      
      Paso=OFF;
      //if(!Secando && !Finalizo && !Desfogando){
      if(!Finalizo){
         AlarmaPuertaS=ON;
      }
      cerroSucio=OFF;
   }else{
      AlarmaPuertaS=OFF;
      cerroSucio=ON;
   }
   /*
   if(!DOORL && !EMPAQUEL){      
      Paso=OFF;
      //if(!Secando && !Finalizo && !Desfogando){
      if(!Finalizo){
         AlarmaPuertaL=ON;
      }
      cerroLimpio=OFF;
   }else{
      AlarmaPuertaL=OFF;
      cerroLimpio=ON;
   }
   */
   if(!STOP){
      AlarmaEmergencia=ON;
   }
   /*
   if(tEmpaqueS>10)
      AlarmaEmpaqueS=ON;
      
   if(tEmpaqueL>10)
      AlarmaEmpaqueL=ON;   
   */
   if(AlarmaPuertaS || AlarmaPuertaL || AlarmaTermostato || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion || AlarmaBomba || AlarmaEmpaqueL || AlarmaEmpaqueS
         || AlarmaVacio || AlarmaLlenado || !Start){
      Esterilizando=OFF;
      Secando=OFF;
      ActivaAlarma(1);
      printf("b2.pic=86");
      SendDataDisplay();
      if(AlarmaEmergencia){
         codigoAlarma=1;
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         Paso=OFF;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         if(!EMPAQUES && PresionCamara<Atmosferica+4)
            empaqueSucio=OFF;
         
         if(!EMPAQUEL && PresionCamara<Atmosferica+4)
            empaqueLimpio=OFF;
            
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         Esterilizando=OFF;
         Desfogando=OFF;
         DesfogueSuave=OFF;
         //IniciaCiclo=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaPuertaS || AlarmaPuertaL){
         codigoAlarma=2;
         printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Paso=OFF;
         suministroVapor=OFF;
         if(AlarmaEmpaqueL)
            empaqueLimpio=OFF;
         if(AlarmaEmpaqueS)
            empaqueSucio=OFF;
         //Suministro_off;
         BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         IniciaLlenado=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaLlenado){
         codigoAlarma=9;
         printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         Aire=OFF;
         Desfogue=OFF;
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Esterilizando=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=21;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaTermostato){
         codigoAlarma=3;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         Desfogue=ON;                  
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaSobreTemperatura){                  
         codigoAlarma=4;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();  
         suministroVapor=OFF;
         Desfogue=ON;
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaTiempoProlongado){
         codigoAlarma=5;
         printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Desfogue=ON;
         Paso=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaPresion){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         codigoAlarma=6;
         printf("Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         if(PresionPreCamara>(Atmosferica+4) && DOORS)
            Paso=ON;
         else
            Paso=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaBomba){ // Alarma de Error de que bomba no ha arrancado.
         codigoAlarma=7;
         printf("Funcionamiento.t4f.txt=\" Error Bomba\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaVacio){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         codigoAlarma=8;
         printf("Funcionamiento.t4f.txt=\" Error Vacio\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
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
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaEmpaqueS || AlarmaEmpaqueL){ // Alarma de Error de que el empaque no se presuriza
         codigoAlarma=9;
         printf("Funcionamiento.t4f.txt=\" Error Empaque\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         if(AlarmaEmpaqueL && PresionCamara<Atmosferica+4)
            empaqueLimpio=OFF;
         if(AlarmaEmpaqueS && PresionCamara<Atmosferica+4)
            empaqueSucio=OFF;
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         
         if(!flagImprimir){
            Imprime=29;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(!Start){ // Inicio o pausa de Ciclo
         printf("Funcionamiento.t4f.txt=\" Ciclo no Iniciado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Desfogue=OFF;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         EstadoBuzzer=OFF;
         codigoAlarma=19;
      }
   }else if(Start){
      printf("b2.pic=85");
      SendDataDisplay();
      if(flagImprime){
         Imprime=5;
         Envio_Esclavos();
         delay_ms(100);
         Imprime=10;
         flagImprime=0;
      }
      
      if(!Fugas){//Si selecciono un ciclo diferente al test de fugas
         EstadoBuzzer=OFF;
         IniciaCiclo=ON;
      
      if(!Precalentamiento){ 
         SuministroVapor=ON;
         aguaEnfriador=OFF;
         Paso=OFF;
         Desfogue=OFF;
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreCalentando\"");
         SendDataDisplay();
         ControlaPresion(); 
         Controlando=0;
         IniciaVacio=OFF;
         IniciaVacio2=OFF;
         codigoAlarma=10;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
      }else{
         if(!Vacio){ //Si no ha realizado los pulsos de vacio
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            if(!flag_vac){//Ciclo inicial para purga de Bomba
               flag_vac=ON;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;
               //Paso=OFF;
               aguaEnfriador=ON;
               delay_ms(70);
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               IniciaVacio2=ON;
               PulsoPositivo=10; //Prueba para ver rendimiento de bomba de vacio************
            }
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            ControlaPresion();
            IniciaVacio=ON;
               
            if(PresionCamara>NivelVacio && PulsoPositivo==10){//Pulso Negativo
               flag_vacio=1;
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               outVacio=ON;
               Desfogue=OFF;
               Aire=OFF;                     
               Paso=OFF;
               if(PulsosVacio>=2)
                  aguaEnfriador=ON;
               else
                  aguaEnfriador=OFF;
               if(flag_pulso==0){
                  PulsosVacio++;
                  flag_pulso=1;
               }
               printf("Funcionamiento.t5f.txt=\" Pulso de Vacio %u\"",PulsosVacio);
               SendDataDisplay();
            }else{
               PulsoPositivo=5;
            }
            
            if(PulsosVacio>=CantidadPulsos && PulsoPositivo==5){//Si realiza los pulsos de vacio, continua con el proceso de calentamiento
               Vacio=ON;
               outVacio=OFF;
               BombaVacio=OFF;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;                                 
               IniciaVacio=OFF;
               Paso=ON;
               aguaEnfriador=OFF;
               PulsoPositivo=10;
            }
              
            if(PulsoPositivo==5){//Pulso Positivo
               tvacio=0;
               flag_pulso=0;
               printf("Funcionamiento.t5f.txt=\" Pulso Positivo %u\"",PulsosVacio);
               SendDataDisplay();
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;                     
               Paso=ON;     
               AguaEnfriador=OFF;
               if(PresionCamara>=Atmosferica+50){
                  Paso=OFF;
                  PulsoPositivo=10;
               }  
            }
               
            if(tbomba>=120 && PulsosVacio==0){//Si no se ha generado ningun pulso de vacio despues de 120 segundos se genera alarma
               if(PresionCamara>Atmosferica){
                  AlarmaBomba=ON;
               }else{
                  tbomba=0;
                  IniciaVacio2=OFF;
               }
             }
               
            if(tvacio>=TiempoVacio){//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
               AlarmaVacio=ON;
            }
         }else{
            IniciaVacio=OFF;        
            if(Temperatura>=((float)Setpoint-0.3)){
               Esterilizando=ON;
            }
            
            if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
               Esterilizando=OFF;
            }
            
            if(!Esterilizando){
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
               Controlando=1;
               Controla();
               Paso=ON;
               BombaVacio=OFF;
               outVacio=OFF;
               Aire=OFF;
               Desfogue=OFF;
               aguaEnfriador=OFF;
            }else{
               TiempoCiclo=0;
               if(mEst==0 && sEst==0){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<(Atmosferica+2)){
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        //if(!abrirPuertaL){
                           BombaVacio=OFF;
                        //}
                        outVacio=OFF;
                        Aire=OFF;
                        Desfogue=ON;
                        Paso=OFF;
                        //Suministro_off;
                        aguaEnfriador=OFF;
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
                           if(!flagImprimir){//Imprime Ciclo Finalizado
                              Imprime=28;
                              Envio_Esclavos();
                              delay_ms(100);
                              Imprime=10;
                              flagImprimir=1;
                           }
                        }
                        incrementaCiclos();
                     }else if(PresionCamara>(Atmosferica+10)){
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
                        Controlando=0;
                        Desfogue=ON;
                        Paso=ON;
                        BombaVacio=OFF;
                        aguaEnfriador=OFF;
                        outVacio=OFF;
                        Aire=OFF; 
                        suministroVapor=OFF;
                     }
                  }else{                     
                     Paso=OFF;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<(Atmosferica+2)){  
                        if(sSec==0 && mSec==0){                          
                           codigoAlarma=15;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }
                           if(!DOORS){
                              Desfogue=OFF;
                              Paso=OFF;
                           }else{
                              Desfogue=ON;                              
                              Paso=OFF;
                           }    

                           BombaVacio=OFF;
                           aguaEnfriador=OFF;
                           outVacio=OFF;
                           if(PresionCamara>=Atmosferica-2){
                              Aire=ON;
                           }else{
                              Aire=OFF;
                           }
                           Desfogue=ON;
                           Paso=OFF;
                           suministroVapor=OFF;
                           Secando=OFF;
                           //if(Temperatura<=Tapertura){
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
                              if(!flagImprimir){//Imprime Ciclo Finalizado
                                 Imprime=28;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                                 flagImprimir=1;
                              }
                           //}
                           incrementaCiclos();
                           Secando=OFF;
                        }else{
                           if(Secando){
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Secando\"");
                              SendDataDisplay();
                           }
                           ControlaPresion();
                           Paso=OFF;
                           
                           if(PresionCamara>NivelVacio+5){//Si la presi?n de la camara es mayor al nivel de vacio
                              if(ReservorioLleno)
                                 BombaVacio=ON;
                              else
                                 BombaVacio=ON;
                              outVacio=ON;
                              //aguaEnfriador=ON;
                              Desfogue=OFF;
                              Aire=OFF;
                              Paso=OFF;
                           }
      
                           if(PresionCamara<=NivelVacio){//Si alcanzo el nivel de vacio requerido
                              codigoAlarma=15;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                              }
                              Secando=ON;
                              EstadoAgua2=ON;
                              
                              if(ReservorioLleno)
                                 BombaVacio=ON;
                              else
                                 BombaVacio=OFF;
                              //aguaEnfriador=ON;
                              outVacio=ON;
                              Desfogue=OFF;
                              Aire=ON;                     
                              Paso=OFF;
                           }
                           
                           if(tEnfriador>=100){
                              aguaEnfriador=OFF;
                              estadoAgua2=OFF;
                           }else{
                              aguaEnfriador=ON;
                           }
                        }
                     }else if(PresionCamara>(Atmosferica+5)){
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
                        Paso=OFF;
                        Desfogue=OFF;
                        if(ReservorioLleno)
                           BombaVacio=ON;
                        else
                           BombaVacio=OFF;   
                        aguaEnfriador=ON;
                        outVacio=ON;
                        Aire=OFF;
                        suministroVapor=OFF;
                     }
                  }
               }else{
                  codigoAlarma=16;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Esterilizando\"");
                  SendDataDisplay();
                  //Paso_on;
                  Control=1;
                  Controla();
               }
            }
         }
      } 
      }else{
      EstadoBuzzer=OFF;
      IniciaCiclo=ON;
         if(!flag_vac){
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            flag_vac=ON;
            if(ReservorioLleno)
               BombaVacio=ON;
            else
               BombaVacio=OFF;
            Desfogue=OFF;
            Aire=OFF;
            aguaEnfriador=OFF;
            Paso=OFF;
            suministroVapor=OFF;
            delay_ms(70);
            IniciaVacio2=ON;
            outVacio=ON;
         }
         
         if(tbomba>=120){
            if(PresionCamara>Atmosferica){
               AlarmaBomba=ON;
            }else{
               tbomba=0;
               IniciaVacio2=OFF;
            }
         }
         
         if(tvacio>=TiempoVacio){//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
               AlarmaVacio=ON;
         }
         
         if(PresionCamara<17){
            BombaVacio=OFF;
            outVacio=OFF;
            Desfogue=OFF;
            Aire=OFF;
            aguaEnfriador=OFF;
            Paso=OFF;
            suministroVapor=OFF;
            Esterilizando=ON;
            IniciaVacio=OFF;
            tbomba=0;
         } 
         
         if(!Finalizo){
            if(!Esterilizando){
               if(codigoAlarma==8){
                  ActivaAlarma(2);
               }else{
                  if(ReservorioLleno)
                     BombaVacio=ON;
                  else
                     BombaVacio=OFF;
                  
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Vacio\"");
                  SendDataDisplay();
               }
            }else{
               if(PresionCamara>25){
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
                  BombaVacio=OFF;
                  outVacio=OFF;
                  Desfogue=OFF;
                  Aire=ON;
                  Paso=OFF;
                  suministroVapor=OFF;
                  aguaEnfriador=OFF;
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
            if(PresionFinal<=25){
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Finalizado\"");
               SendDataDisplay();
               codigoAlarma=14;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               if(!flagImprimir){//Imprime Ciclo Finalizado
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
               if(!flagImprimir){//Imprime Ciclo Finalizado
                  Imprime=26;
                  Envio_Esclavos();
                  delay_ms(100);
                  Imprime=10;
                  flagImprimir=1;
               }
            }
            IniciaVacio=OFF;
            tbomba=0;
            BombaVacio=OFF;
            outVacio=OFF;
            Desfogue=OFF;
            Aire=ON;
            Paso=OFF;
            suministroVapor=OFF;
            aguaEnfriador=OFF;
            ActivaAlarma(2);
            if(!Finalizo){
               Finalizo=ON;
               PresionFinal=PresionCamara;
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
      Lectura=0;
      //printf("tuser.txt=\"%s\"",usuario);
      //SendDataDisplay();
      printf("t6f.txt=\"%3.1f?C\"",Temperatura);
      SendDataDisplay();
      printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
      SendDataDisplay();
      printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
      SendDataDisplay();
      printf("Funcionamiento.t9f.txt=\"%03u?C\"",Setpoint);
      SendDataDisplay();      
      printf("Funcionamiento.t10f.txt=\"%02u:%02u\"",mEst,sEst);
      SendDataDisplay();      
      printf("Funcionamiento.t11f.txt=\"%02u:%02u\"",mSec,sSec);
      SendDataDisplay();      
      
      printf("fecha.txt=\"%s Fecha:%02u/%02u/20%02u  Hora:%02u:%02u %2.1f?C\"",usuario,Dia,Mes,Year,Hora,Minuto,Tactual);
      SendDataDisplay();
   }

}else if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
     
      if(Opcion2>12)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=12;
      
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
         printf("Config.t2c.txt=\"RPT100B\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.1f\"",Sensor[0].RPT100);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida            
               if(Sensor[0].RPT100<2.0)
                  Sensor[0].RPT100+=0.1;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(Sensor[0].RPT100>0.0)
                  Sensor[0].RPT100-=0.1;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
      }else if(Opcion2==9){
         printf("Config.t2c.txt=\"RPT100B 2\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.1f\"",Sensor[1].RPT100);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
               if(Sensor[1].RPT100<2.0)
                  Sensor[1].RPT100+=0.1;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Abajo Oprimida
               if(Sensor[1].RPT100>0.0)
                  Sensor[1].RPT100-=0.1;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
         }
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
               GuardaEEPROM=ON;
            }
            printf("page Menu");
            SendDataDisplay();
            //BorraBuffer();
         }
      }
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay(); 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

   }
}
