//Falta crear menu de ajuste de variables y de norma CFR21
//Falta revisar niveles de agua con variable calderin lleno de bomba de agua

#include <18F4685.h>
#device adc=10
#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HS,WDT4096,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,PUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR,NOBROWNOUT,BORV42
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N,errors)
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
byte SlaveB0Tx[50];
byte SlaveC0Tx[50];
byte SlaveC0Rx[50];
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
short EstadoEmpaque=OFF,EstadoSSR1=OFF,EstadoSSR2=OFF,ReservorioLleno=OFF,SensadoNivelR=OFF,borra=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoSensadoNivel=0,Tp=0,Tc=0,tguarda=0,tNivelBajo=0,tIniciar=0,tCiclof=0,tCicloP=0,tConfig=0,TiempoLlenado=OFF;
int8 sEst=0,mEst=0,sSec=0,mSec=0,sEstp=0,mEstp=0,sSecp=0,mSecp=0;// Tiempo transcurrido

//Control//------------------------
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Temporal=0.0,Aumento=0.0,tmp2=0.0;
float K=0.0012858,Histeresis=0.0;
float R10=6800,AN=0,BN=0;
float X[2]={0.0,0.0};
float promediopresion=0.0,promediopresionp=0.0;
int8 h=0,l=0,MediaMovil=20,PantallaPrincipal=0,TipoClave=0,Modulo=0,TiempoSensadoNivelR=0,subirPuertaLimpio=0,cerroSucioInt=0,cerroLimpioInt=0,histe=0,desviacion=0;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,Temperatura2=0.0;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0,CicloLibre=0,pulsosConfigurados=6,vacioTest=17,vacioCiclo=25;
float p1=0.0000011617,p2=-0.0010575,p3=0.44114,p4=65.043,Tactual=0.0;
//float p1=-0.00036062,p2=0.30947,p3=72.816,Tactual=0.0;

//Varios//--------------------------------
short desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagImprime=0;
int8 Control=0,tiemporecta=0,Controlando=0,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0,Codigo=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,CiclosL=0,CiclosH=0,CicloSeleccionado=0,Tapertura=0;
int16 CantidadPulsos=4,tEmpaqueS=0,tEmpaqueL=0,tEnfriador=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0,m=0;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,tbomba=0,tvacio=0,tiempoImpresion=0,PresionFinal=0;
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0,NivelDes=0;
int8 y=0,ConteoBuzzer=0,codigoAlarma=0,codigoAnt=0,codigoUsuario2=0;
int16 tbuzzer=0,tsilencio=0,tStop=0,cicloVisualizado=0,cicloVisto=0,j=0,i=0,posicionDato=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0,NivelVacio=25,Atmosferica=75,PulsoPositivo=10,PulsosVacio=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,dowTx=0,SetTime=0,Imprime=0,Pulsos=0,codigoUsuario=0,codigoUsuarioant=0;
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
#include "Menu.h"

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
   
   if(!STOP){
      tStop++;
   }else{
      tStop=0;
   }
   
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
   restart_wdt();
   printf("page Usuarios");
   SendDataDisplay();
   ApagaSalidas();   
   TiempoVacio=840; 
   Sensor[0].Gain=105.56;
   Sensor[1].Gain=105.56;
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
   
   if(UP && DOWN && RIGHT && LEFT)//Si oprime boton de Toma.
   {
      write_eeprom(37,0);
      delay_ms(20);
      write_eeprom(38,0);
      delay_ms(20);
      reset_cpu();
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
      
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   menuPrincipal();

   if(Menu==100){ //Menu de Configuraci?n de Parametros de Control  
      if(Opcion2>15)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=15;
      
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
         printf("Config.t2c.txt=\"Desvio\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.1f\"",desvio);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(desvio<3.0)
                  desvio+=0.1;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(desvio>0.0)
                  desvio-=0.1;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==9){
         printf("Config.t2c.txt=\"Histeresis\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.1f\"",Histeresis);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Histeresis<2.0)
                  Histeresis+=0.1;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Histeresis>0.0)
                  Histeresis-=0.1;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==10){
         printf("Config.t2c.txt=\"Ciclos\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%05Lu\"",Ciclos);
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
         printf("Config.t2c.txt=\"Vacio Test\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",vacioTest);
         SendDataDisplay();
         
            if(RX_Buffer[4]==0x0a){
                  ++vacioTest;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         
            if(RX_Buffer[4]==0x0b){
                  --vacioTest;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         
         if(vacioTest>30)
            vacioTest=17;
         if(vacioTest<17)
            vacioTest=30;
      }else if(Opcion2==14){
         printf("Config.t2c.txt=\"Pulsos Config\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",pulsosConfigurados);
         SendDataDisplay();
         
            if(RX_Buffer[4]==0x0a){
                  ++pulsosConfigurados;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
            
            if(RX_Buffer[4]==0x0b){
                  --pulsosConfigurados;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         
         if(pulsosConfigurados>8)
            pulsosConfigurados=2;
         if(pulsosConfigurados<2)
            pulsosConfigurados=8;
      }else if(Opcion2==15){
         printf("Config.t2c.txt=\"Vacio Ciclo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%02u\"",vacioCiclo);
         SendDataDisplay();
         
            if(RX_Buffer[4]==0x0a){
                  ++vacioCiclo;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
            
            if(RX_Buffer[4]==0x0b){
                  --vacioCiclo;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         
         if(vacioCiclo>40)
            vacioCiclo=17;
         if(vacioCiclo<17)
            vacioCiclo=40;
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
               write_eeprom(31,Atmosferica);
               delay_ms(10);
               write_eeprom(32,Tapertura);
               delay_ms(10);
               write_eeprom(50,pulsosConfigurados);
               delay_ms(10);
               write_eeprom(51,vacioTest);
               delay_ms(10);
               write_eeprom(52,vacioCiclo);
               delay_ms(10);
               histe=(int8)(Histeresis*10);
               write_eeprom(53,histe);
               delay_ms(10);
               desviacion=(int8)(desvio*10);
               write_eeprom(54,desviacion);
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
