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

#define   UP            !input(PIN_B6)
#define   DOWN          !input(PIN_B5)
#define   RIGHT         !input(PIN_B4)
#define   LEFT          !input(PIN_B3)

#define   DOOR          !input(PIN_B1)
#define   STOP          !input(PIN_B2)
#define   TERM          !input(PIN_B3)
#define   PRES          !input(PIN_A4)

#define   LOW           input(PIN_B0)

#define   Buzzer_on        output_bit(PIN_C5,1)
#define   Buzzer_off       output_bit(PIN_C5,0)

#define   Suministro_on    output_bit(PIN_D0,1)
#define   Suministro_off   output_bit(PIN_D0,0)

#define   Paso_on          output_bit(PIN_D1,1)
#define   Paso_off         output_bit(PIN_D1,0)

#define   Vacio_on         output_bit(PIN_D2,1)
#define   Vacio_off        output_bit(PIN_D2,0)

#define   Desfogue_on      output_bit(PIN_D3,1)
#define   Desfogue_off     output_bit(PIN_D3,0)

#define   Aire_on          output_bit(PIN_D4,1)
#define   Aire_off         output_bit(PIN_D4,0)

#define   BombaVacio_on    output_bit(PIN_D5,1)
#define   BombaVacio_off   output_bit(PIN_D5,0)

#define   Agua_on          output_bit(PIN_D6,1)
#define   Agua_off         output_bit(PIN_D5,0)

#define OFF 0
#define ON  1
#define RX_BUFFER_SIZE  10
char Rx_Buffer[RX_BUFFER_SIZE+1];
char Rx_Buffer2[RX_BUFFER_SIZE+1];
char RX_Wr_Index=0;

//Comunicacion//------------------------
//byte txbuf[0X20];
byte SlaveA0Tx[0X20];
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

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaTermostato=OFF,AlarmaPuerta=OFF,AlarmaEmergencia=OFF,AlarmaVacio=OFF;
short AlarmaBomba=OFF,guardaCiclo=OFF;
int16 TiempoVacio=600;

//Banderas//--------------------
short IniciaLlenado=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short EstadoDesfogue=OFF,EstadoSuministro=OFF,EstadoAire=OFF,PruebaEstados=OFF,disp_Guarda=OFF,Ciclo=OFF;
short Op=OFF,Op2=OFF,Minutos=OFF,Segundos=OFF,GuardaEEPROM=OFF,EstadoPaso=OFF,EstadoVacio=OFF,EstadoBuzzer=OFF,flagAlarma=OFF;
short flagBuzzer=OFF,flagSilencio=OFF,Finalizo=OFF,AlarmaPresion=OFF,EstadoACierre=OFF,EstadoBomba=OFF,Libre=OFF,ingreso=OFF,Fugas=OFF;
short PreCalentamiento=OFF,IniciaVacio=OFF,IniciaVacio2=OFF,Vacio=OFF,flag_vac=OFF,flag_vacio=OFF,flag_pulso=OFF,EstadoAireBomba=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoSensadoNivel=0,Tp=0,Tc=0,tguarda=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido

//Control//------------------------
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Temporal=0.0;
float R7=3000,R8=3100,R9=3100,R10=3100,AN=0,BN=0;
float X[2]={0.0,0.0};
float promediopresion=0.0,promediopresionp=0.0;
int8 h=0,l=0,MediaMovil=20,PantallaPrincipal=0,TipoClave=0,Modulo=0;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,Temperatura2=0.0,K=0.0012858;
float a=0.0011,b=2.3302,c=-244.0723;

//Varios//--------------------------------
short desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagImprime=0;
int8 Control=0,tiemporecta=0,Controlando=0,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,i=0,CiclosL=0,CiclosH=0,CicloSeleccionado=0,Tapertura=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0,tiempoImpresion=0,CantidadPulsos=4;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,NivelSeleccionado=0,tbomba=0,tvacio=0;
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0,NivelDes=0;
int8 y=0,ConteoBuzzer=0,posicionDato=0,codigoAlarma=0,codigoAnt=0;
int16 t_exhaust=0,tbuzzer=0,tsilencio=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0,cicloVisualizado=0,cicloVisto=0,NivelVacio=25,Atmosferica=75,PulsoPositivo=10,PulsosVacio=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,SegundoTx=0,dowTx=0,SetTime=0,Imprime=0,CicloLibre=0,Pulsos=0,j=0,codigoUsuario=0,codigoUsuarioant=0;
signed int8 G1=0,G2=0,sg1=0,sg2=0;
const char Texto[12][20]={"CALENTAMIENTO","NO ENVUELTOS 134","NO ENVUELTOS 121","ENVUELTOS 134","ENVUELTOS 121","ENVUELTO DOBLE 1",
"ENVUELTO DOBLE 2","PRION","BOWIE & DICK","TEST DE VACIO","TEST DE COMPONENTES","HORA Y FECHA"};
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
   
   if(Menu==20)
     Buzzer_on;
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
         Suministro_on;
      else         
         Suministro_off;
            
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
         tiempoctrl=0;// tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
   }
   
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
      
      if(guardaCiclo)
         tguarda++;
      
      if(IniciaCiclo && !Esterilizando)
         TiempoCiclo++;
      
      if(SensadoNivel)
         TiempoSensadoNivel++;
      
      if(IniciaVacio)
         tvacio++;         
      
      if(IniciaVacio2)
         tbomba++;
     
      tiempos=0;Lectura=1;tinicio++;
   
      if(Esterilizando && !Desfogando){               
         if(Testerilizacion[0]>0){
            Testerilizacion[0]--;
         }else if(Testerilizacion[1]>0){
            Testerilizacion[1]--;
            Testerilizacion[0]=9;
         }else if(Testerilizacion[2]>0){
            Testerilizacion[2]--;
            Testerilizacion[1]=5;
            Testerilizacion[0]=9;
         }else if(Testerilizacion[3]>0){
            Testerilizacion[3]--;
            Testerilizacion[2]=9;
            Testerilizacion[1]=5;
            Testerilizacion[0]=9;
         }
      }
      
      if(Secando){
         if(Tsecado[0]>0){
            Tsecado[0]--;
         }else if(Tsecado[1]>0){
            Tsecado[1]--;
            Tsecado[0]=9;
         }else if(Tsecado[2]>0){
            Tsecado[2]--;
            Tsecado[1]=5;
            Tsecado[0]=9;
         }else if(Tsecado[3]>0){
            Tsecado[3]--;
            Tsecado[2]=9;
            Tsecado[1]=5;
            Tsecado[0]=9;
         }
      }
   }
}

void main()
{
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN4);
   setup_adc(ADC_CLOCK_DIV_32 );
   port_b_pullups(true);
   enable_interrupts(int_rda);
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   LeeEEPROM();
   rtc_init();
   delay_ms(1);
   LimitaValores();
   CalculoImpresion=timpresion*60;
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(1500); 
   printf("page Usuarios");
   SendDataDisplay();
   ApagaSalidas();
   AN=1023*(R8+R9);
   TiempoVacio=CantidadPulsos*180; 
   Sensor[0].Gain=152.53;
   Sensor[1].Gain=151.61;
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(1,MediaMovil);
      Temperatura2=LeerPT100(0,MediaMovil);
   }
   
   while(true)
   {
      rtc_get_date(Dia,Mes,Year,dow);
      rtc_get_time(Hora,Minuto,Segundo);
      LeeDisplay();
      Envio_Esclavos();
      delay_ms(1);
      //Lectura_Esclavos();
      restart_wdt();
      
      if(Temperatura2>40)
         Agua_on;
      else
         Agua_off;
      
// Tiempo de Grafica//--------------------------------------
      if(tiempos2>=4000){
         Grafica=1;
         tiempos2=0;
      }

// Tiempo de Muestreo//--------------------------------------
      if(tiempos3>=20){
         Temperatura=LeerPt100(1,MediaMovil);
         Temperatura2=LeerPt100(0,MediaMovil);
         PresionCamara=Leer_Sensor_Presion_Camara(5);
         PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
         tiempos3=0;
      }
      
// Monitoreo Parada Emergencia//--------------------------------------      
      if(!STOP && !IniciaCiclo && !PruebaEstados){
         if(!disp_Stop){
            disp_Stop=1;Menu=255;
         }
         Controlando=0;
         Desfogue_on;
         Paso_off;
         Suministro_off;
         if(PresionCamara<Atmosferica)
            Aire_on;
         else
            Aire_off;
         Vacio_off;         
         DesfogueSuave=OFF;
         printf("page Emergencia");
         SendDataDisplay();
         ActivaAlarma(1);
         BorraBuffer();
      }else{
         if(disp_Stop){
            reset_cpu();
         }
      }           
   //Buzzer_off;   
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(1500);
      printf("page Usuarios");
      SendDataDisplay();
      ApagaSalidas();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==0){ //Menu de Contrase?a de acceso.
      ApagaSalidas();
      //printf("bkcmd=0");
      //SendDataDisplay();
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
      }
      
      if(TipoClave==2){
         printf("titulo.txt=\"Clave Actual\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }
      
      if(TipoClave==1){
         printf("titulo.txt=\"Clave Tecnico\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }
      
      if(TipoClave==0){
         printf("titulo.txt=\"%s\"",usuario);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x11){//11, Regresar
         printf("page Usuarios");
         SendDataDisplay();       
      }
      
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contrase?a de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            
            if((RX_Buffer[4]==Clave[codigoUsuario].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario].Password[1]+0x30)&&
            (RX_Buffer[6]==Clave[codigoUsuario].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario].Password[3]+0x30)){
               printf("page ClaveCorrecta");
               SendDataDisplay();
               ingreso=ON;
            }else{
               printf("page ClaveBad");
               SendDataDisplay();
            } 
         }else if(TipoClave==1){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
            }else{
               printf("page Menu");
               SendDataDisplay();
            }
         }else if(TipoClave==2){
            if((RX_Buffer[4]==Clave[codigoUsuario].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario].Password[1]+0x30)&&
            (RX_Buffer[6]==Clave[codigoUsuario].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario].Password[3]+0x30)){
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
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page Menu");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(300+(codigoUsuario*4),RX_Buffer[4]-0x30);delay_ms(20);
               write_eeprom(301+(codigoUsuario*4),RX_Buffer[5]-0x30);delay_ms(20);
               write_eeprom(302+(codigoUsuario*4),RX_Buffer[6]-0x30);delay_ms(20);
               write_eeprom(303+(codigoUsuario*4),RX_Buffer[7]-0x30);delay_ms(20);
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
            ConfiguraCiclo(0,0,5,1,0,0,1,0,0,1,121);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//02, No Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,3,0,0,0,1,0,0,2,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x03  && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//03, No Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x03 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,5,1,0,0,1,0,0,3,121);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//04, Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,4,0,0,0,5,1,0,4,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//05, Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,5,1,0,0,5,1,0,5,121);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//06, Envuelto Doble 1
         delay_ms(5);
         if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,7,0,0,0,0,2,0,6,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//07, Envuelto Doble 2
         delay_ms(5);
         if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,7,0,0,0,0,3,0,7,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//08, Prion
         delay_ms(5);
         if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,8,1,0,0,5,1,0,8,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//09, Bowie & Dick
         delay_ms(5);
         if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,3,3,0,0,0,2,0,0,9,134);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=4;
            NivelVacio=25;
            Fugas=OFF;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//0a, Test de Fugas
         delay_ms(5);
         if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,0,5,1,0,0,3,0,0,1,21);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            CantidadPulsos=1;
            NivelVacio=17;
            Fugas=ON;
            TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(codigoUsuario!=0 && codigoUsuario!=9){       
         printf("tsw b11,0");
         SendDataDisplay();  
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
         printf("page Ajustes");
         SendDataDisplay();     
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Minutoest=Testerilizacionp[2]+(Testerilizacionp[3]*10);
               CiclosPersonalizados[CicloLibre-1].Segundoest=Testerilizacionp[0]+(Testerilizacionp[1]*10);
               write_eeprom(101+((CicloLibre-1)*8),Testerilizacionp[2]+(Testerilizacionp[3]*10));
               delay_ms(10);
               write_eeprom(102+((CicloLibre-1)*8),Testerilizacionp[0]+(Testerilizacionp[1]*10));
               delay_ms(10);
               Testerilizacion[0]=Testerilizacionp[0];
               Testerilizacion[1]=Testerilizacionp[1];
               Testerilizacion[2]=Testerilizacionp[2];
               Testerilizacion[3]=Testerilizacionp[3];
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Arriba Oprimida
            if(Segundos){
               Testerilizacionp[0]++;
               if(Testerilizacionp[0]>9){
                  Testerilizacionp[1]++;
                  if(Testerilizacionp[1]>5)
                     Testerilizacionp[1]=0;
                  Testerilizacionp[0]=0;
               }
            }
            if(Minutos){
               Testerilizacionp[2]++;
               if(Testerilizacionp[2]>9){
                  Testerilizacionp[3]++;
                  if(Testerilizacionp[3]>5)
                     Testerilizacionp[3]=0;
                  Testerilizacionp[2]=0;
               }
            }
            RX_Buffer[4]=0x00;
            Op=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c && Op){//Tecla Derecha Oprimida
            if(Segundos){
               Testerilizacionp[0]++;
               if(Testerilizacionp[0]>9){
                  Testerilizacionp[1]++;
                  if(Testerilizacionp[1]>5)
                     Testerilizacionp[1]=0;
                  Testerilizacionp[0]=0;
               }
            }
            if(Minutos){
               Testerilizacionp[2]++;
               if(Testerilizacionp[2]>9){
                  Testerilizacionp[3]++;
                  if(Testerilizacionp[3]>5)
                     Testerilizacionp[3]=0;
                  Testerilizacionp[2]=0;
               }
            }
            RX_Buffer[4]=0x00;
            Op=OFF;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0e && !Op2){//Tecla Izquierda Oprimida
            if(Segundos){
               Testerilizacionp[0]--;
               if(Testerilizacionp[0]<0){
                  Testerilizacionp[1]--;
                  if(Testerilizacionp[1]<0)
                     Testerilizacionp[1]=5;
                  Testerilizacionp[0]=9;
               }
            }
            if(Minutos){
               Testerilizacionp[2]--;
               if(Testerilizacionp[2]<0){
                  Testerilizacionp[3]--;
                  if(Testerilizacionp[3]<0)
                     Testerilizacionp[3]=5;
                  Testerilizacionp[2]=9;
               }
            }
            RX_Buffer[4]=0x00;
            Op2=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0f && Op2){//Tecla Izquierda Oprimida
            
            if(Segundos){
               Testerilizacionp[0]--;
               if(Testerilizacionp[0]<0){
                  Testerilizacionp[1]--;
                  if(Testerilizacionp[1]<0)
                     Testerilizacionp[1]=5;
                  Testerilizacionp[0]=9;
               }
            }
            if(Minutos){
               Testerilizacionp[2]--;
               if(Testerilizacionp[2]<0){
                  Testerilizacionp[3]--;
                  if(Testerilizacionp[3]<0)
                     Testerilizacionp[3]=5;
                  Testerilizacionp[2]=9;
               }
            }
            RX_Buffer[4]=0x00;
            Op2=OFF;
            //BorraBuffer();
         }
         printf("tminest.txt=\"%02u\"",((Testerilizacionp[3]*10)+Testerilizacionp[2]));
         SendDataDisplay();
         printf("tsecest.txt=\"%02u\"",((Testerilizacionp[1]*10)+Testerilizacionp[0]));
         SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==3){ //Menu de Tiempo de Secado
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Minutosec=Tsecadop[2]+(Tsecadop[3]*10);
               CiclosPersonalizados[CicloLibre-1].Segundosec=Tsecadop[0]+(Tsecadop[1]*10);
               write_eeprom(103+((CicloLibre-1)*8),Tsecadop[2]+(Tsecadop[3]*10));
               delay_ms(10);
               write_eeprom(104+((CicloLibre-1)*8),Tsecadop[0]+(Tsecadop[1]*10));
               delay_ms(10);
               Tsecado[0]=Tsecadop[0];
               Tsecado[1]=Tsecadop[1];
               Tsecado[2]=Tsecadop[2];
               Tsecado[3]=Tsecadop[3];
               GuardaEEPROM=ON;
            }
         }
          
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Arriba Oprimida
            if(Segundos){
               Tsecadop[0]++;
               if(Tsecadop[0]>9){
                  Tsecadop[1]++;
                  if(Tsecadop[1]>5)
                     Tsecadop[1]=0;
                  Tsecadop[0]=0;
               }
            }
            if(Minutos){
               Tsecado[2]++;
               if(Tsecadop[2]>9){
                  Tsecadop[3]++;
                  if(Tsecadop[3]>5)
                     Tsecadop[3]=0;
                  Tsecadop[2]=0;
               }
            }
            RX_Buffer[4]=0x00;
            Op=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c && Op){//Tecla Derecha Oprimida
            if(Segundos){
               Tsecadop[0]++;
               if(Tsecadop[0]>9){
                  Tsecadop[1]++;
                  if(Tsecadop[1]>5)
                     Tsecadop[1]=0;
                  Tsecadop[0]=0;
               }
            }
            if(Minutos){
               Tsecadop[2]++;
               if(Tsecadop[2]>9){
                  Tsecadop[3]++;
                  if(Tsecadop[3]>5)
                     Tsecadop[3]=0;
                  Tsecadop[2]=0;
               }
            }
            RX_Buffer[4]=0x00;
            Op=OFF;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0e && !Op2){//Tecla Izquierda Oprimida
            if(Segundos){
               Tsecadop[0]--;
               if(Tsecadop[0]<0){
                  Tsecadop[1]--;
                  if(Tsecadop[1]<0)
                     Tsecadop[1]=5;
                  Tsecadop[0]=9;
               }
            }
            if(Minutos){
               Tsecadop[2]--;
               if(Tsecadop[2]<0){
                  Tsecadop[3]--;
                  if(Tsecadop[3]<0)
                     Tsecadop[3]=5;
                  Tsecadop[2]=9;
               }
            }
            RX_Buffer[4]=0x00;
            Op2=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0f && Op2){//Tecla Izquierda Oprimida
            if(Segundos){
               Tsecadop[0]--;
               if(Tsecadop[0]<0){
                  Tsecadop[1]--;
                  if(Tsecadop[1]<0)
                     Tsecadop[1]=5;
                  Tsecadop[0]=9;
               }
            }
            if(Minutos){
               Tsecadop[2]--;
               if(Tsecadop[2]<0){
                  Tsecadop[3]--;
                  if(Tsecadop[3]<0)
                     Tsecadop[3]=5;
                  Tsecadop[2]=9;
               }
            }
            RX_Buffer[4]=0x00;
            Op2=OFF;
            //BorraBuffer();
         }
      printf("tminsec.txt=\"%02u\"",((Tsecadop[3]*10)+Tsecadop[2]));
      SendDataDisplay();
      printf("tsecsec.txt=\"%02u\"",((Tsecadop[1]*10)+Tsecadop[0]));
      SendDataDisplay();
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==4){ //Menu de Temperatura
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
    if(Menu==5){//Menu de Nivel de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Nivel<10)
            Nivel=100;
         if(Nivel>100)
            Nivel=10;
         
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
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==6){ // Menu de Prueba de Componentes
         PruebaEstados=ON;
         
         if(RX_Buffer[4]==0x01)//Oprimio EV Suministro
         {
            EstadoSuministro=!EstadoSuministro;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSuministro){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            Suministro_on;
         }
         else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            Suministro_off;
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
         }
         else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            Desfogue_off;
         }
         
         if(RX_Buffer[4]==0x03)//Oprimio EV Paso
         {
            EstadoPaso=!EstadoPaso;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoPaso){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            Paso_on;
         }
         else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            Paso_off;
         }
        
         if(RX_Buffer[4]==0x04)//Oprimio Vacio
         {
            EstadoVacio=!EstadoVacio;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoVacio){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            Vacio_on;
         }
         else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            Vacio_off;
         }
         
         if(RX_Buffer[4]==0x05)//Oprimio Aire
         {
            EstadoAire=!EstadoAire;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAire){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            Aire_on;
         }
         else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            Aire_off;
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
            BombaVacio_on;
         }
         else{
            printf("b5.bco=63488");
            SendDataDisplay(); 
            BombaVacio_off;
         }
         
         if(RX_Buffer[4]==0x07)//Oprimio EV AireBomba 
         {
            EstadoAireBomba=!EstadoAireBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAireBomba){
            printf("b6.bco=2016");
            SendDataDisplay(); 
            Agua_on;
         }
         else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
            Agua_off;
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
            output_bit(PIN_C1,1);
            output_bit(PIN_C2,0);
         }
         else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
            output_bit(PIN_C1,0);
            output_bit(PIN_C2,1);
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
         
         if(PRES){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }
         
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
         
         if(RX_Buffer[4]==0x14)//Oprimio Zero ADC3
         {
            G1=41-sensores(2);
            if(G1<0)
               sg1=10;
            else
               sg1=5;
                  
            write_eeprom(42,(int8)G1);
            delay_ms(10);
            write_eeprom(44,(int8)sg1);
            delay_ms(10);
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            delay_ms(1000);
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x14)//Oprimio Zero ADC3
         {
            G2=41-sensores(3);
            if(G2<0)
               sg2=10;
            else
               sg2=5;
                  
            write_eeprom(43,(int8)G2);
            delay_ms(10);
            write_eeprom(45,(int8)sg2);
            delay_ms(10);
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            delay_ms(1000);
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"AD2:%3.0fP:%2.1f\"",sensores(2),PresionCamara);
         SendDataDisplay();
         printf("b18.txt=\"AD3:%3.0fP:%2.1f\"",sensores(3),PresionPreCamara);
         SendDataDisplay();
         printf("b19.txt=\"T1:%3.1f\"",Temperatura);
         SendDataDisplay();
         printf("b20.txt=\"T1:%3.1f\"",Temperatura2);
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
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==7){ //Menu de Configuraci?n de Fecha y Hora
      
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
            rtc_set_datetime(DiaTx,MesTx,YearTx,dowTx,HoraTx,MinutoTx);
         }
         
         if(RX_Buffer[4]==0x2a){//Tecla Arriba Oprimida
               if(Opcion==1){
                  if(MesTx==2){
                     if(esBisiesto(YearTx)){
                        if(DiaTx<29)
                           DiaTx++;
                     }else{
                        if(DiaTx<28)
                           DiaTx++;
                     }
                  }else{
                     if(MesTx<=7){
                        if(MesTx % 2 ==0){
                           if(DiaTx<30)
                              DiaTx++;    
                        }else{
                           if(DiaTx<31)
                              DiaTx++;    
                        }    
                     }else{
                        if(MesTx % 2 ==0){
                           if(DiaTx<31)
                              DiaTx++;    
                        }else{
                           if(DiaTx<30)
                              DiaTx++;    
                        }    
                     }
                  }
               }else if(Opcion==2){
               if(MesTx<12)
                  MesTx++;
               }else if(Opcion==3){
                  if(YearTx<99)
                     YearTx++;
               }else if(Opcion==4){
                  if(HoraTx<24)
                     HoraTx++;
               }else if(Opcion==5){
                  if(MinutoTx<59)
                     MinutoTx++;
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
               }else if(Opcion==3){
                  if(YearTx>0)
                     YearTx--;
               }else if(Opcion==4){
                  if(HoraTx>0)
                     HoraTx--;
               }else if(Opcion==5){
                  if(MinutoTx>0)
                     MinutoTx--;
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
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==8){ //Menu Ciclos Personalizados
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      
      if(codigoUsuario!=0 && codigoUsuario!=9){       
         printf("tsw ciclo,0");
         SendDataDisplay();          
         printf("tsw temp,0");
         SendDataDisplay();          
         printf("tsw test,0");
         SendDataDisplay();
         printf("tsw tsec,0");
         SendDataDisplay();          
         printf("tsw pulsos,0");
         SendDataDisplay();
         printf("tsw vacio,0");
         SendDataDisplay();          
         printf("tsw b50,0");
         SendDataDisplay();  
      }
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[4]<0x15 && estados[RX_Buffer[4]-1]){//Personalizados
         printf("ciclo.txt=\"CICLO P%02u\"",RX_Buffer[4]);
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
         printf("ciclo.txt=\"CICLO P%02u\"",CicloLibre);
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
         printf("page Ajustes");
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
         Testerilizacionp[3]=CiclosPersonalizados[CicloLibre-1].Minutoest/10;
         Testerilizacionp[2]=CiclosPersonalizados[CicloLibre-1].Minutoest-(Testerilizacion[3]*10);
         Testerilizacionp[1]=CiclosPersonalizados[CicloLibre-1].Segundoest/10;
         Testerilizacionp[0]=CiclosPersonalizados[CicloLibre-1].Segundoest-(Testerilizacion[1]*10);
      }
      
      if(RX_Buffer[4]==0x40 && CicloLibre>0){//40, Tsec
         printf("page TiempoSec");
         SendDataDisplay();   
         Tsecadop[3]=CiclosPersonalizados[CicloLibre-1].Minutosec/10;
         Tsecadop[2]=CiclosPersonalizados[CicloLibre-1].Minutosec-(Tsecado[3]*10);
         Tsecadop[1]=CiclosPersonalizados[CicloLibre-1].Segundosec/10;
         Tsecadop[0]=CiclosPersonalizados[CicloLibre-1].Segundosec-(Tsecado[1]*10);
      }
      
      if(RX_Buffer[4]==0x50 && CicloLibre>0){//50, PulsosVacio
         printf("page Pulsos");
         SendDataDisplay();     
         Pulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
      }
      
      if(RX_Buffer[4]==0x60 && CicloLibre>0){//60, NivelVacio
         printf("page Nivel");
         SendDataDisplay();     
         Nivel=CiclosPersonalizados[CicloLibre-1].NivelVacio;
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
         if(CiclosPersonalizados[CicloLibre-1].NivelDesfogue==6){
            ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest-(Testerilizacion[1]*10),
                        CiclosPersonalizados[CicloLibre-1].Segundoest/10,
                        CiclosPersonalizados[CicloLibre-1].Minutoest-(Testerilizacion[3]*10),
                        CiclosPersonalizados[CicloLibre-1].Minutoest/10,
                        CiclosPersonalizados[CicloLibre-1].Segundosec-(Tsecado[1]*10),
                        CiclosPersonalizados[CicloLibre-1].Segundosec/10,
                        CiclosPersonalizados[CicloLibre-1].Minutosec-(Tsecado[3]*10),
                        CiclosPersonalizados[CicloLibre-1].Minutosec/10,0,CicloLibre+10,CiclosPersonalizados[CicloLibre-1].Temperatura);            
         }else{
            ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest-(Testerilizacion[1]*10),
                        CiclosPersonalizados[CicloLibre-1].Segundoest/10,
                        CiclosPersonalizados[CicloLibre-1].Minutoest-(Testerilizacion[3]*10),
                        CiclosPersonalizados[CicloLibre-1].Minutoest/10,
                        CiclosPersonalizados[CicloLibre-1].Segundosec-(Tsecado[1]*10),
                        CiclosPersonalizados[CicloLibre-1].Segundosec/10,
                        CiclosPersonalizados[CicloLibre-1].Minutosec-(Tsecado[3]*10),
                        CiclosPersonalizados[CicloLibre-1].Minutosec/10,1,CicloLibre+10,CiclosPersonalizados[CicloLibre-1].Temperatura);
            NivelSeleccionado=CiclosPersonalizados[CicloLibre-1].NivelDesfogue*9990;            
         }
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==9){//Menu de Pulsos de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Pulsos<1)
            Pulsos=15;
         if(Pulsos>15)
            Pulsos=1;
         
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
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==10){//Menu de Usuarios
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      if(ingreso)
         TipoClave=2;
      else
         TipoClave=0;
      
      if(RX_Buffer[4]==0x01){//01, Admin
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Admin";
         codigoUsuario=0;
      }else if(RX_Buffer[4]==0x02){//02, Operador 1
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 1";
         codigoUsuario=1;
      }else if(RX_Buffer[4]==0x03){//03, Operador 2
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)   
            usuario="Operador 2";
         codigoUsuario=2;
      }else if(RX_Buffer[4]==0x04){//04, Operador 3
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 3";
         codigoUsuario=3;
      }else if(RX_Buffer[4]==0x05){//05, Operador 4
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 4";
         codigoUsuario=4;
      }else if(RX_Buffer[4]==0x06){//06, Operador 5
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 5";
         codigoUsuario=5;
      }else if(RX_Buffer[4]==0x07){//07, Operador 6
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 6";
         codigoUsuario=6;
      }else if(RX_Buffer[4]==0x08){//08, Operador 7
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 7";
         codigoUsuario=7;
      }else if(RX_Buffer[4]==0x09){//09, Operador 8
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Operador 8";
         codigoUsuario=8;
      }else if(RX_Buffer[4]==0x0a){//0a, Tecnico
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso)
            usuario="Tecnico";
         codigoUsuario=9;
      }
      
      if(RX_Buffer[4]==0xaa && ingreso){//aa, Regresar
         printf("page Ajustes");
         SendDataDisplay();       
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==11){//Menu de Configuraciones
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
   if(Menu==12){//Menu de Visualizacion de Ciclos Realizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      //SendDataDisplay();
      GuardaEEPROM=OFF;
      
      i2c_start();            // Comienzo de la comunicaci?n
      i2c_write(0xB0);   // Direcci?n del esclavo en el bus I2C
      i2c_write(0);    // Posici?n de donde se leer? el dato en el esclavo
      i2c_write(0);    // Posici?n de donde se leer? el dato en el esclavo
      i2c_start();            // Reinicio
      i2c_write(direccion+1); // Direcci?n del esclavo en modo lectura
      dato=i2c_read(0);       // Lectura del dato
      i2c_stop();
      if(dato==1){
         printf("estado.txt=\"Enlazando \"");
         SendDataDisplay();     
      }else if(dato==2){
         printf("estado.txt=\"Verificando Com. \"");
         SendDataDisplay();    
      }else if(dato==3){
         printf("estado.txt=\"Montando USB\"");
         SendDataDisplay();    
      }else if(dato==4){
         printf("estado.txt=\"Creando Archivo\"");
         SendDataDisplay();    
      }else if(dato==5){
         printf("estado.txt=\"Transfiriendo Datos\"");
         SendDataDisplay();    
      }else if(dato==6){
         printf("estado.txt=\"Cerrando USB\"");
         SendDataDisplay();    
      }else{
         printf("estado.txt=\"Desconectado \"");
         SendDataDisplay();    
      }
      
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
            if(TCiclos[i]>200)
               TCiclos[i]=0;
            PcCiclos[i]=make16(read_ext_eeprom((cicloVisto*320)+7+((cicloVisualizado+i)*9)),read_ext_eeprom((cicloVisto*320)+8+((cicloVisualizado+i)*9)));
            if(PcCiclos[i]>500)
               PcCiclos[i]=0;
            PpcCiclos[i]=make16(read_ext_eeprom((cicloVisto*320)+9+((cicloVisualizado+i)*9)),read_ext_eeprom((cicloVisto*320)+10+((cicloVisualizado+i)*9)));
            if(PpcCiclos[i]>500)
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
      
      printf("t0.txt=\"Ciclo %02u\"",cicloVisto+1);
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
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==13){//Menu de Nivel de Desfogue
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==14){//Menu de Activacion de Ciclos
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
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
      delay_ms(2000);
      printf("page Menu");
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==16){//Menu de Clave InCorrecta
      delay_ms(2000);
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
      
   if(PresionPreCamara>(Atmosferica+302) || (Atmosferica+PresionCamara>255)){
      AlarmaPresion=ON;
   }
   
   if(TiempoCiclo>1800){//ANALIZAR BIEN ESTA ALARMA
      if(PresionPrecamara<10){
         AlarmaTiempoProlongado=ON;
      }
   }else{
      AlarmaTiempoProlongado=OFF;
   }
   
   if(Temperatura>Setpoint+3.0){
      AlarmaSobreTemperatura=ON;
   }
   
   if(!TERM){
      AlarmaTermostato=ON;
   }
   
   if(!DOOR){
      //Desfogue_off;
      //Solenoide_off;
      Paso_off;
      if(!Secando && !Finalizo){
         AlarmaPuerta=ON;
      }
   }else{
      AlarmaPuerta=OFF;
   }

   if(!STOP){
      AlarmaEmergencia=ON;
   }
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion || AlarmaBomba 
         || AlarmaVacio){
      ActivaAlarma(1);
      if(AlarmaEmergencia){
         codigoAlarma=1;
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         Paso_off;
         Suministro_off;
         BombaVacio_off;
         if(PresionCamara<Atmosferica)
            Aire_on;
         else
            Aire_off;
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
      }else if(AlarmaPuerta){
         codigoAlarma=2;
         printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Paso_off;
         Suministro_off;
         BombaVacio_off;
         Aire_off;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
      }else if(AlarmaTermostato){
         codigoAlarma=3;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Suministro_off;
         Desfogue_on;                  
         Paso_off;
         BombaVacio_off;
         Aire_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }else if(AlarmaSobreTemperatura){                  
         codigoAlarma=4;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();  
         Suministro_off;
         Desfogue_on;
         Paso_off;
         BombaVacio_off;
         Aire_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }else if(AlarmaTiempoProlongado){
         codigoAlarma=5;
         printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Suministro_off;
         Desfogue_on;
         Paso_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }else if(AlarmaPresion){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         codigoAlarma=6;
         printf("Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         Suministro_off;
         BombaVacio_off;
         Aire_off;
         if(PresionPreCamara>(Atmosferica+4) && DOOR)
            Paso_on;
         else
            Paso_off;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }else if(AlarmaBomba){ // Alarma de Error de que bomba no ha arrancado.
         codigoAlarma=7;
         printf("Funcionamiento.t4f.txt=\" Bomba frenada\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         Suministro_off;
         BombaVacio_off;
         Aire_off;
         
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }else if(AlarmaVacio){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         codigoAlarma=8;
         printf("Funcionamiento.t4f.txt=\" Error Vacio\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         Suministro_off;
         BombaVacio_off;
         Aire_off;
         
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
      }
   }else{
      sistemaBloqueo(1);
      if(!Fugas){//Si selecciono un ciclo diferente al test de fugas
         Buzzer_off;
         IniciaCiclo=ON;
         
      if(!Precalentamiento){      
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
      }else{
         if(!Vacio){ //Si no ha realizado los pulsos de vacio
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            if(!flag_vac){//Ciclo inicial para purga de Bomba
               flag_vac=ON;
               if(LOW)
                  BombaVacio_on;
               Vacio_off;
               Desfogue_off;
               Aire_off;
               
               Paso_off;
               delay_ms(70);
               IniciaVacio2=ON;
            }
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            ControlaPresion();
            IniciaVacio=ON;
               
            if(PresionCamara>NivelVacio && PulsoPositivo==10){//Pulso Negativo
               flag_vacio=1;
               if(LOW)
                  BombaVacio_on;
               Vacio_on;
               Desfogue_off;
               Aire_off;                     
               Paso_off;
               
               flag_pulso=0;
               printf("Funcionamiento.t5f.txt=\" Pulso de Vacio %u\"",PulsosVacio+1);
               SendDataDisplay();
            }else{
               PulsoPositivo=5;
            }
              
            if(PulsoPositivo==5){//Pulso Positivo
               if(flag_pulso==0){
                  PulsosVacio++;
                  flag_pulso=1;
               }
               printf("Funcionamiento.t5f.txt=\" Pulso Positivo %u\"",PulsosVacio);
               SendDataDisplay();
               if(LOW)
                  BombaVacio_on;
               Vacio_off;
               Desfogue_off;
               Aire_off;                     
               Paso_on;            
               
               if(PresionCamara>=Atmosferica+25){
                  Paso_off;
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
                  
            if(PulsosVacio>=CantidadPulsos){//Si realiza los pulsos de vacio, continua con el proceso de calentamiento
               Vacio=ON;
               Vacio_off;
               BombaVacio_off;
               Vacio_off;
               Desfogue_off;
               Aire_off;                                 
               
               IniciaVacio=OFF;
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
               Control=1;
               Controla();
               Paso_on;
               BombaVacio_off;
               Vacio_off;
               Aire_off;
               
               Desfogue_off;
               Paso_off;               
            }else{
               codigoAlarma=13;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               TiempoCiclo=0;
               if((Testerilizacion[0]==0)&&(Testerilizacion[1]==0)&&
               (Testerilizacion[2]==0)&&(Testerilizacion[3]==0)){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<(Atmosferica+4)){
                        codigoAlarma=15;
                        if(codigoAnt!=codigoAlarma){
                           codigoAnt=codigoAlarma;
                           GuardaDatoCiclo();                  
                        }
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        BombaVacio_off;
                        Vacio_off;
                        Aire_off;
                        
                        Desfogue_on;
                        Paso_off;
                        Suministro_off;
                        if(Temperatura<=Tapertura){
                           sistemaBloqueo(0);
                           ActivaAlarma(2);
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                           SendDataDisplay();
                           Finalizo=ON;
                        }
                        incrementaCiclos();
                     }else{
                        codigoAlarma=14;
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
                        Paso_on;
                        BombaVacio_off;
                        Vacio_off;
                        Aire_off;
                        
                        Suministro_off;
                     }
                  }else{                     
                     Paso_off;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<(Atmosferica+20)){
                        if((Tsecado[0]==0)&&(Tsecado[1]==0)&&(Tsecado[2]==0)&&(Tsecado[3]==0)){                          
                           codigoAlarma=15;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }
                           if(!DOOR){
                              Desfogue_off;
                              Paso_off;
                           }else{
                              Desfogue_on;                              
                              Paso_off;
                           }    
                           BombaVacio_off;
                           Vacio_off;
                           Aire_on;
                          
                           Desfogue_off;
                           Paso_off;
                           Suministro_off;
                           Secando=OFF;
                           if(Temperatura<=Tapertura){
                              sistemaBloqueo(0);
                              ActivaAlarma(2);
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                              SendDataDisplay();
                              Finalizo=ON;
                           }
                           incrementaCiclos();
                           Secando=OFF;
                        }else{
                           codigoAlarma=17;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }
                           Secando=ON;
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Secando\"");
                           SendDataDisplay();
                           Paso_off;
                           ControlaPresion();
                           if(PresionCamara>NivelVacio+5){//Si la presi?n de la camara es mayor al nivel de vacio
                              if(LOW)
                                 BombaVacio_on;
                              Vacio_on;
                              Desfogue_off;
                              Aire_off;
                              Paso_off;
                              
                           }
      
                           if(PresionCamara<NivelVacio){//Si alcanzo el nivel de vacio requerido
                              if(LOW)
                                 BombaVacio_on;
                              Vacio_off;
                              Desfogue_off;
                              Aire_on;                     
                              Paso_off;
                              
                           }
                        }
                     }else{
                        codigoAlarma=14;
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
                        Paso_off;
                        Desfogue_on;
                        if(LOW)
                           BombaVacio_on;
                        Vacio_off;
                        Aire_off;
                        
                        Suministro_off;
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
                  Paso_on;
                  Control=1;
                  Controla();
               }
            }
         }
      } 
      }else{
         if(!flag_vac){
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            flag_vac=ON;
            BombaVacio_on;
            Vacio_off;
            Desfogue_off;
            Aire_off;
            
            Paso_off;
            Suministro_off;
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Test de Vacio\"");
            SendDataDisplay();
            delay_ms(7000);
            IniciaVacio2=ON;
            
            Vacio_on;
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
            BombaVacio_off;
            Vacio_off;
            Desfogue_off;
            Aire_off;
            
            Paso_off;
            Suministro_off;
            Esterilizando=ON;
            IniciaVacio=OFF;
            tbomba=0;
         } 
         if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&
         (Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3])){                
            IniciaVacio=OFF;
            tbomba=0;
            BombaVacio_off;
            Vacio_off;
            Desfogue_off;
            Aire_off;
            
            Paso_off;
            Suministro_off;
            ActivaAlarma(2);
            if(PresionCamara<=20 && !Desfogando){
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Finalizado\"");
               SendDataDisplay();
               codigoAlarma=15;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
            }else{
               printf("Funcionamiento.t4f.txt=\" Error Ciclo\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Finalizado\"");
               SendDataDisplay();
               codigoAlarma=15;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
            }
            incrementaCiclos();
            Desfogando=ON;
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
      printf("add 14,0,%1.0f",Temperatura*1.5);
      SendDataDisplay();
      printf("add 14,1,%1.0f",PresionCamara*1.5);
      SendDataDisplay();
      printf("add 14,2,%1.0f",PresionPreCamara*1.5);
      SendDataDisplay();
      Grafica=0;
   }
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      printf("t6f.txt=\"%3.1f\"",Temperatura);
      SendDataDisplay();
      printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
      SendDataDisplay();
      printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
      SendDataDisplay();
      printf("Funcionamiento.t9f.txt=\"%03u\"",Setpoint);
      SendDataDisplay();      
      printf("Funcionamiento.t10f.txt=\"%02u\"",((Testerilizacion[3]*10)+Testerilizacion[2]));
      SendDataDisplay();      
      printf("Funcionamiento.t13f.txt=\"%02u\"",((Testerilizacion[1]*10)+Testerilizacion[0]));
      SendDataDisplay();      
      printf("Funcionamiento.t11f.txt=\"%02u\"",((Tsecado[3]*10)+Tsecado[2]));
      SendDataDisplay();      
      printf("Funcionamiento.t16f.txt=\"%02u\"",((Tsecado[1]*10)+Tsecado[0]));
      SendDataDisplay();      
      
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
      printf("fecha.font=2");
      SendDataDisplay();
   }

}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
     
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
