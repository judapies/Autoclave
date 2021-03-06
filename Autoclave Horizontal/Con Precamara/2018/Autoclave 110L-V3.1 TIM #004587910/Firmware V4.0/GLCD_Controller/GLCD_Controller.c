//Falta crear menu de ajuste de variables y de norma CFR21

#include <18F4685.h>
#device adc=10
#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HS,WDT8192,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
//#include "HDM64GS12.c"
//#include "GRAPHICS.c"
///#include "imagen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <DS1302.c>

#define   UP            !input(PIN_B6)
#define   DOWN          !input(PIN_B5)
#define   RIGHT         !input(PIN_B4)
#define   LEFT          !input(PIN_B3)

#define   DOOR          !input(PIN_B2)
#define   STOP          !input(PIN_B1)
#define   TERM          !input(PIN_B0)
#define   PRES          !input(PIN_A4)

#define   HIG           input(PIN_A5)
#define   LOW           input(PIN_E0)

#define   SSR_on           output_bit(PIN_C1,0)
#define   SSR_off          output_bit(PIN_C1,1)

#define   Buzzer_on        output_bit(PIN_C5,1)
#define   Buzzer_off       output_bit(PIN_C5,0)

#define   Solenoide_on     output_bit(PIN_D0,1)
#define   Solenoide_off    output_bit(PIN_D0,0)

#define   Desfogue_on      output_bit(PIN_D2,0)
#define   Desfogue_off     output_bit(PIN_D2,1)

#define   AguaIn_on        output_bit(PIN_D4,0)
#define   AguaIn_off       output_bit(PIN_D4,1)

#define   Paso_on          output_bit(PIN_D3,0)
#define   Paso_off         output_bit(PIN_D3,1)

#define OFF 0
#define ON  1
#define RX_BUFFER_SIZE  10
char Rx_Buffer[RX_BUFFER_SIZE+1];
char Rx_Buffer2[RX_BUFFER_SIZE+1];
char RX_Wr_Index=0;
char RX_Rd_Index=0;
char RX_Counter=0;
int1 RX_Buffer_Overflow=0;

#define TX_BUFFER_SIZE  24
char Tx_Buffer[TX_BUFFER_SIZE+1];
char TX_Wr_Index=0;
char TX_Rd_Index=0;
char TX_Counter=0;
//int1 RX_Buffer_Overflow=0;

//Comunicacion//------------------------
byte txbuf[0X20];
byte SlaveA0Tx[0X20];
byte dato=0;            //Contendrá la información a enviar o recibir
byte direccion=0;       //Contendrá la dirección del esclavo en el bus I2C
byte posicion=0;        //Contendrá la posición en el buffer de memoria del esclavo donde se almacenará o de donde se recuperará el dato
byte ByteConfig1=0,ByteConfig2=0,ByteConfig3=0;

/* Tabla de Modulos, Primera Fila Ańo Regular y el siguiente ańo Bisiesto
E   F   M   A   M   J   J   A   S   O   N   D
0   3   3   6   1   4   6   2   5   0   3   5
0   3   4   0   2   5   0   3   6   1   4   6
*/
int8 Regular[12]={0,3,3,6,1,4,6,2,5,0,3,5};
int8 Bisiesto[12]={0,3,4,0,2,5,0,3,6,1,4,6};

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaLlenado=OFF,AlarmaTermostato=OFF,AlarmaPuerta=OFF,AlarmaEmergencia=OFF;

//Banderas//--------------------
short IniciaLlenado=OFF,CalderinLleno=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short EstadoDesfogue=OFF,EstadoAgua=OFF,EstadoSSR=OFF,PruebaEstados=OFF,disp_Guarda=OFF,Ciclo=OFF,flagCalentando=OFF,flagEsterilizando=OFF;
short flagDesfogando=OFF,Op=OFF,Op2=OFF,Minutos=OFF,Segundos=OFF,GuardaEEPROM=OFF,EstadoPaso=OFF,EstadoBloqueo=OFF,EstadoBuzzer=OFF,flagAlarma=OFF;
short flagBuzzer=OFF,flagSilencio=OFF,Finalizo=OFF,AlarmaPresion=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoLlenado=0,TiempoSensadoNivel=0,Tp=0,Tc=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido

//Control//------------------------
float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Gain=141.6,RPT100=0.5,Temporal=0.0;
float R1=100,R2=3000,R3=100,R4=3000,R5=100,R6=3000,R7=3000,R8=3100,R9=3100,R10=3100,AN=0,BN=0;
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0,promediopresionp=0.0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=20,TTrampa=0,n=0,PantallaPrincipal=0,TipoClave=0,Modulo=0,Digito=0;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,K=0.0012858,Pc=0.0,Pp=0.0;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0;

//Varios//--------------------------------
char kpa[]="kPa";
short Flanco=0,Flanco1=0,Flanco2=0,visual=1,desfoguelento=0,Lectura=0,Grafica=0;
short disp_Stop=0,flagTC=0,flagPersonalizado=0,flagImprime=0,flagImprimir=0;
int8 Control=0,tiemporecta=0,Controlando=0,ActivaImpresion=1,timpresion=2,CalculoImpresion=0,Inicio=1,Dato_Exitoso=0,Dato2=0,z=0;
int8 Menu=0, MenuAnt=0,MenuAntt=240,unidad=1,i=0,CiclosL=0,CiclosH=0,CicloSeleccionado=0;
int8 Setpoint=0,Opcion=1,Opcion2=1,tiempo_esterilizacion=0,tiempo_secado=0,tiempoImpresion=0;
int16 tiempos=0,tiempos2=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,NivelSeleccionado=0;
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int16 passw=0;
int8 r=0,t=0,r22=0,t2=0,TiempoControl=0;
int8 y=0,ConteoBuzzer=0;
int16 t_exhaust=0,tbuzzer=0,tsilencio=0;
int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,SegundoTx=0,dowTx=0,SetTime=0,Imprime=0;
signed int8 G1=0,G2=0,sg1=0,sg2=0;

typedef struct{
   char Letra[30];
   char Estado[4];
}MENUU;

MENUU Menus;

#include "Funciones.h"

#int_rda HIGH
void serial_rx_isr(){

   Dato2=fgetc();
   if(Dato2==0x65){//Inicio Comunicacion
      Inicio=1;
      RX_Wr_Index =0;
   }
   //Inicio=1;
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
   //Dato_Exitoso=5;
   if(Menu==20)
     Buzzer_on;
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(DesfogueSuave)
   {
      if(Nivel<6)
      {
         if(t_exhaust<=NivelSeleccionado) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
         {
            Desfogue_on;
         }else{            
            Desfogue_off;
         }   
      
         if(t_exhaust>=60000) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
         {
            t_exhaust=0;
         } // tiempoctrl incrementa una unidad cada Xms, por eso se dividen los tiempos por 0.005.
      }
      else
      {
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
   
   if(flagAlarma)
      tbuzzer++;
      
   if(flagSilencio)   
      tsilencio++;
   
   if(Controlando==1){
      if(tiempoctrl<Tp) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
      {
         SSR_on;
      }
      else         
      {
         SSR_off;
      }
            
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
      {
         tiempoctrl=0;
      } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
   }
   
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {
      if(flagTC)
         TiempoControl++;
      
      tiemporecta++;
      
      if(IniciaCiclo && tiempoImpresion<CalculoImpresion){
         tiempoImpresion++;
      }else if(tiempoImpresion>=CalculoImpresion){
         flagImprime=1;
         tiempoImpresion=0;
      }
      
      if(IniciaCiclo && !Esterilizando && !Desfogando && !Finalizo){
         TiempoCiclo++;
      }
      
      if(IniciaLlenado && !CalderinLleno){
         TiempoLlenado++;
      }
      
      if(SensadoNivel){
         TiempoSensadoNivel++;
      }
      
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
   
   for (posicion=0;posicion<0x10;posicion++){
      SlaveA0Tx[posicion] = 0x00;
   }
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN3);
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
   printf("page Clave");
   SendDataDisplay();
   ApagaSalidas();
   Desfogue_on;
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(MediaMovil);
   }
   
   if(UP && DOWN && RIGHT && LEFT)//Si oprime boton de Toma.
   {
      write_eeprom(37,0);
      delay_ms(20);
      write_eeprom(38,0);
      delay_ms(20);
      reset_cpu();
   }
   
   while(true)
   {
      LeeDisplay();
      Envio_Esclavos();
      delay_ms(1);
      Lectura_Esclavos();
      restart_wdt();

      // Monitoreo Nivel de Agua//--------------------------------------
            if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
               TiempoSensadoNivel=0;
               Controlando=0;   
               CalderinLleno=OFF;
            } else{
               if(HIG){
                  SensadoNivel=ON;
                  if(TiempoSensadoNivel>2)
                  {
                     SensadoNivel=OFF;
                     CalderinLleno=ON;
                     IniciaLlenado=OFF;                  
                  }
               }else{
                  SensadoNivel=OFF;
                  TiempoSensadoNivel=0;
               }
            }
// Tiempo de Grafica//--------------------------------------
      if(tiempos2>=4000){
         Grafica=1;
         tiempos2=0;
      }

// Tiempo de Muestreo//--------------------------------------
      if(tiempos3>=20){
         Temperatura=LeerPt100(MediaMovil);
         PresionCamara=Leer_Sensor_Presion_Camara(5);
         PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
         rtc_get_date(Dia,Mes,Year,dow);
         rtc_get_time(Hora,Minuto,Segundo);
         tiempos3=0;
      }
      
// Monitoreo Parada Emergencia//--------------------------------------      
      if(!STOP && !IniciaCiclo && !PruebaEstados){
         if(!disp_Stop)
         {
            disp_Stop=1;Menu=255;
         }
         Controlando=0;
         Desfogue_on;
         AguaIn_off;
         SSR_off;
         Paso_off;
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
      printf("page Clave");
      SendDataDisplay();
      ApagaSalidas();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==0){ //Menu de Contraseńa de acceso.
      ApagaSalidas();
      //printf("bkcmd=0");
      //SendDataDisplay();
      if(Digito==1){
         printf("t0.txt=\"%u\"",Contrasena[0]);
         SendDataDisplay();
         printf("va0.val=1");
         SendDataDisplay();
      }
      
      if(Digito==2){
         printf("t0.txt=\"%u%u\"",Contrasena[0],Contrasena[1]);
         SendDataDisplay();
         printf("va0.val=2");
         SendDataDisplay();
      }
      
      if(Digito==3){
         printf("t0.txt=\"%u%u%u\"",Contrasena[0],Contrasena[1],Contrasena[2]);
         SendDataDisplay();
         printf("va0.val=3");
         SendDataDisplay();
      }
      
      if(Digito==4){
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
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseńa de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            
            if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3])){
               //BorraBuffer();
               printf("page MenuPrincipal");
               SendDataDisplay();
            }else{
               //BorraBuffer();
               printf("page Clave");
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
         printf("titulo.txt=\"Ingrese Contraseńa\"");
         SendDataDisplay();
      }
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseńa de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            
            if((RX_Buffer[4]==Password[0]+0x30)&&(RX_Buffer[5]==Password[1]+0x30)&&(RX_Buffer[6]==Password[2]+0x30)&&(RX_Buffer[7]==Password[3]+0x30)){
               //BorraBuffer();
               printf("page MenuPrincipal");
               SendDataDisplay();
            }else{
               //BorraBuffer();
               printf("page Clave");
               SendDataDisplay();
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      if(RX_Buffer[3]==0x0f || UP){//0f, recibe caracteres ingresados desde el Display
         delay_ms(5);
         if(RX_Buffer[3]==0x0f || UP){
         if(PantallaPrincipal==0){//Si esta en menu principal
            if(RX_Buffer[4]==0x37){//Ajustes y pasa a solicitud de password de tecnico
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
            if(Opcion==8){// Cambio de Clave
               printf("page Clave");
               SendDataDisplay(); 
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
               TipoClave=2;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
               Opcion=0;
            }else if(Opcion==7){//Hora y Fecha
               printf("page Fecha");
               SendDataDisplay();     
               PantallaPrincipal=0;
               MenuAnt=Menu;
               Op=OFF;
               Op2=OFF;
               DiaTx=Dia;
               MesTx=Mes;
               YearTx=Year;
               HoraTx=Hora;
               MinutoTx=Minuto;
            }else if(Opcion==6){//Test
               printf("page Test");
               SendDataDisplay();     
               PantallaPrincipal=0;
               MenuAnt=Menu;
               Op=OFF;
               Op2=OFF;
            }else if(Opcion==5){//Ciclo Personalizado
               printf("page MenuPrincipal");
               SendDataDisplay();     
               PantallaPrincipal=1;
               MenuAnt=Menu;
               Op=OFF;
               Op2=OFF;
            }else if(Opcion==4){//LiquidosB Nivel de Desfogue 
               printf("page Nivel");
               SendDataDisplay();       
               ConfiguraCiclo(0,0,0,3,0,0,0,0,1,4,121);
               Op=OFF;
               Op2=OFF;
               MenuAnt=2;
            }else if(Opcion==3){//LiquidosA Nivel de Desfogue 
               printf("page Nivel");
               SendDataDisplay();       
               ConfiguraCiclo(0,0,5,1,0,0,0,0,1,3,121);
               Op=OFF;
               Op2=OFF;
               MenuAnt=2;
            }else if(Opcion==2){//Instrumental
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,0,3,0,0,0,5,0,0,2,134);
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
               if(Temperatura>60)TTrampa=TTrampa+5;
            }else if(Opcion==1){//Plastico
               printf("page Funcionamiento");
               SendDataDisplay();       
               ConfiguraCiclo(0,0,5,1,0,0,3,0,0,1,121);
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
               if(Temperatura>60)TTrampa=TTrampa+5;
            }
         }else if(PantallaPrincipal==1){//Si esta en menu de ciclo libre
            if(RX_Buffer[4]==0x37){//Ajustes y pasa a solicitud de password de tecnico
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
            if(Opcion==15){// Iniciar Ciclo
               printf("page Funcionamiento");
               SendDataDisplay();  
               MenuAnt=Menu;
               IniciaCiclo=ON;
               NivelSeleccionado=9990*Nivel;
               CicloSeleccionado=5;
               if(ActivaImpresion==1){
                  Imprime=5;
                  Envio_Esclavos();
                  delay_ms(100);
                  Imprime=10;
               }
               Tsecado[0]=Tsecadop[0];
               Tsecado[1]=Tsecadop[1];
               Tsecado[2]=Tsecadop[2];
               Tsecado[3]=Tsecadop[3];
               Testerilizacion[0]=Testerilizacionp[0];
               Testerilizacion[1]=Testerilizacionp[1];
               Testerilizacion[2]=Testerilizacionp[2];
               Testerilizacion[3]=Testerilizacionp[3];
               if(Temperatura>60)TTrampa=TTrampa+5;
            }else if(Opcion==14){//Nivel de Desfogue
               printf("page Nivel");
               SendDataDisplay();    
               Op=OFF;
               Op2=OFF;
               MenuAnt=1;
            }else if(Opcion==13){//Temperatura 
               printf("page Temperatura");
               SendDataDisplay();  
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
            }else if(Opcion==12){//Tiempo de Secado
               printf("page TiempoSec");
               SendDataDisplay();   
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
            }else if(Opcion==11){//Instrumental
               printf("page TiempoEst");
               SendDataDisplay(); 
               Op=OFF;
               Op2=OFF;
               MenuAnt=Menu;
            }
         }
         }
      }else if(PantallaPrincipal==0){
         if(Opcion>8)
            Opcion=1;
            
         if(Opcion<1)
            Opcion=8;
         
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
         
         printf("ttitulo.txt=\"SELECCIONE SU CICLO\"");
         SendDataDisplay();
            
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Derecha Oprimida
            Opcion++;
            RX_Buffer[4]=0x00;
            Op=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c && Op){//Tecla Derecha Oprimida
            Opcion++;
            RX_Buffer[4]=0x00;
            Op=OFF;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0e && !Op2){//Tecla Izquierda Oprimida
            Opcion--;
            RX_Buffer[4]=0x00;
            Op2=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0f && Op2){//Tecla Izquierda Oprimida
            Opcion--;
            RX_Buffer[4]=0x00;
            Op2=OFF;
            //BorraBuffer();
         }
         
         if(Opcion==8){
            printf("bc.pic=83");
            SendDataDisplay();
            printf("bd.pic=36");
            SendDataDisplay();
            printf("bi.pic=40");
            SendDataDisplay();
            printf("bc.pic2=83");
            SendDataDisplay();
            printf("bd.pic2=36");
            SendDataDisplay();
            printf("bi.pic2=40");
            SendDataDisplay();
            printf("textop.txt=\"CAMBIO DE CLAVE\"");
            SendDataDisplay();
         }
         
         if(Opcion==7){
            printf("bc.pic=84");
            SendDataDisplay();
            printf("bd.pic=38");
            SendDataDisplay();
            printf("bi.pic=52");
            SendDataDisplay();
            printf("bc.pic2=84");
            SendDataDisplay();
            printf("bd.pic2=38");
            SendDataDisplay();
            printf("bi.pic2=52");
            SendDataDisplay();
            printf("textop.txt=\"HORA Y FECHA\"");
            SendDataDisplay();
         }
         
         if(Opcion==6){
            printf("bc.pic=93");
            SendDataDisplay();
            printf("bd.pic=40");
            SendDataDisplay();
            printf("bi.pic=44");
            SendDataDisplay();
            printf("bc.pic2=93");
            SendDataDisplay();
            printf("bd.pic2=40");
            SendDataDisplay();
            printf("bi.pic2=44");
            SendDataDisplay();
            printf("textop.txt=\"TEST COMPONENTES\"");
            SendDataDisplay();
         }
         
         if(Opcion==5){
            printf("bc.pic=87");
            SendDataDisplay();
            printf("bd.pic=52");
            SendDataDisplay();
            printf("bi.pic=46");
            SendDataDisplay();
            printf("bc.pic2=87");
            SendDataDisplay();
            printf("bd.pic2=52");
            SendDataDisplay();
            printf("bi.pic2=46");
            SendDataDisplay();
            printf("textop.txt=\"LIBRE\"");
            SendDataDisplay();
         }
         
         if(Opcion==4){
            printf("bc.pic=89");
            SendDataDisplay();
            printf("bd.pic=44");
            SendDataDisplay();
            printf("bi.pic=45");
            SendDataDisplay();
            printf("bc.pic2=89");
            SendDataDisplay();
            printf("bd.pic2=44");
            SendDataDisplay();
            printf("bi.pic2=45");
            SendDataDisplay();
            printf("textop.txt=\"LIQUIDOS B\"");
            SendDataDisplay();
         }
    
         if(Opcion==3){
            printf("bc.pic=88");
            SendDataDisplay();
            printf("bd.pic=46");
            SendDataDisplay();
            printf("bi.pic=37");
            SendDataDisplay();
            printf("bc.pic2=88");
            SendDataDisplay();
            printf("bd.pic2=46");
            SendDataDisplay();
            printf("bi.pic2=37");
            SendDataDisplay();
            printf("textop.txt=\"LIQUIDOS A\"");
            SendDataDisplay();
         }
         
         if(Opcion==2){
            printf("bc.pic=86");
            SendDataDisplay();
            printf("bd.pic=45");
            SendDataDisplay();
            printf("bi.pic=36");
            SendDataDisplay();
            printf("bc.pic2=86");
            SendDataDisplay();
            printf("bd.pic2=45");
            SendDataDisplay();
            printf("bi.pic2=36");
            SendDataDisplay();
            printf("textop.txt=\"INSTRUMENTAL\"");
            SendDataDisplay();
         }
  
         if(Opcion==1){
            printf("bc.pic=91");
            SendDataDisplay();
            printf("bd.pic=37");
            SendDataDisplay();
            printf("bi.pic=38");
            SendDataDisplay();
            printf("bc.pic2=91");
            SendDataDisplay();
            printf("bd.pic2=37");
            SendDataDisplay();
            printf("bi.pic2=38");
            SendDataDisplay();
            printf("textop.txt=\"PLASTICO\"");
            SendDataDisplay();
         }
      }else if(PantallaPrincipal==1){
         printf("ttitulo.txt=\"CICLO LIBRE\"");
         SendDataDisplay();
         if(Opcion>15)
            Opcion=11;
            
         if(Opcion<11)
            Opcion=15;
            
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
            
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Derecha Oprimida
            Opcion++;
            RX_Buffer[4]=0x00;
            Op=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c && Op){//Tecla Derecha Oprimida
            Opcion++;
            RX_Buffer[4]=0x00;
            Op=OFF;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0e && !Op2){//Tecla Izquierda Oprimida
            Opcion--;
            RX_Buffer[4]=0x00;
            Op2=ON;
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0f && Op2){//Tecla Izquierda Oprimida
            Opcion--;
            RX_Buffer[4]=0x00;
            Op2=OFF;
            //BorraBuffer();
         }
  
         if(Opcion==15){
            printf("bc.pic=85");
            SendDataDisplay();
            printf("bd.pic=57");
            SendDataDisplay();
            printf("bi.pic=47");
            SendDataDisplay();
            printf("bc.pic2=85");
            SendDataDisplay();
            printf("bd.pic2=57");
            SendDataDisplay();
            printf("bi.pic2=47");
            SendDataDisplay();
            printf("textop.txt=\"INICIAR CICLO\"");
            SendDataDisplay();
         }
         
         if(Opcion==14){
            printf("bc.pic=90");
            SendDataDisplay();
            printf("bd.pic=41");
            SendDataDisplay();
            printf("bi.pic=56");
            SendDataDisplay();
            printf("bc.pic2=90");
            SendDataDisplay();
            printf("bd.pic2=41");
            SendDataDisplay();
            printf("bi.pic2=56");
            SendDataDisplay();
            printf("textop.txt=\"NIVEL DESFOGUE\"");
            SendDataDisplay();
         }
         
         if(Opcion==13){
            printf("bc.pic=92");
            SendDataDisplay();
            printf("bd.pic=47");
            SendDataDisplay();
            printf("bi.pic=58");
            SendDataDisplay();
            printf("bc.pic2=92");
            SendDataDisplay();
            printf("bd.pic2=47");
            SendDataDisplay();
            printf("bi.pic2=58");
            SendDataDisplay();
            printf("textop.txt=\"TEMPERATURA\"");
            SendDataDisplay();
         }
         
         if(Opcion==12){
            printf("bc.pic=95");
            SendDataDisplay();
            printf("bd.pic=56");
            SendDataDisplay();
            printf("bi.pic=57");
            SendDataDisplay();
            printf("bc.pic2=95");
            SendDataDisplay();
            printf("bd.pic2=56");
            SendDataDisplay();
            printf("bi.pic2=57");
            SendDataDisplay();
            printf("textop.txt=\"T. SECADO\"");
            SendDataDisplay();
         }

         if(Opcion==11){
            printf("bc.pic=94");
            SendDataDisplay();
            printf("bd.pic=58");
            SendDataDisplay();
            printf("bi.pic=41");
            SendDataDisplay();
            printf("bc.pic2=94");
            SendDataDisplay();
            printf("bd.pic2=58");
            SendDataDisplay();
            printf("bi.pic2=41");
            SendDataDisplay();
            printf("textop.txt=\"T. ESTERILIZACIÓN\"");
            SendDataDisplay();
         }
         
         if(RX_Buffer[4]==0x11 || DOWN){//11 Boton de regresar
            delay_ms(200);
            if(RX_Buffer[4]==0x11 || DOWN){
               PantallaPrincipal=0;
               printf("page MenuPrincipal");
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
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
            printf("page MenuPrincipal");
            SendDataDisplay();  
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               write_eeprom(0,Testerilizacionp[0]);delay_ms(20);write_eeprom(1,Testerilizacionp[1]);delay_ms(20);
               write_eeprom(2,Testerilizacionp[2]);delay_ms(20);write_eeprom(3,Testerilizacionp[3]);delay_ms(20);
               Testerilizacion[0]=Testerilizacionp[0];
               Testerilizacion[1]=Testerilizacionp[1];
               Testerilizacion[2]=Testerilizacionp[2];
               Testerilizacion[3]=Testerilizacionp[3];
               GuardaEEPROM=ON;
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
                  Testerilizacionp[0]++;
                  if(Testerilizacionp[0]>9){
                     Testerilizacionp[1]++;
                     Testerilizacionp[0]=0;
                  }
               }
               if(Minutos){
                  Testerilizacionp[2]++;
                  if(Testerilizacionp[2]>9){
                     Testerilizacionp[3]++;
                     Testerilizacionp[2]=0;
                  }
               }    
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               if(Segundos){
                  Testerilizacionp[0]--;
                  if(Testerilizacionp[0]<0){
                     Testerilizacionp[1]--;
                     Testerilizacionp[0]=9;
                  }
               }
               if(Minutos){
                  Testerilizacionp[2]--;
                  if(Testerilizacionp[2]<0){
                     Testerilizacionp[3]--;
                     Testerilizacionp[2]=9;
                  }
               }    
            }
         }
         
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Arriba Oprimida
            if(Segundos){
               Testerilizacionp[0]++;
               if(Testerilizacionp[0]>9){
                  Testerilizacionp[1]++;
                  Testerilizacionp[0]=0;
               }
            }
            if(Minutos){
               Testerilizacionp[2]++;
               if(Testerilizacionp[2]>9){
                  Testerilizacionp[3]++;
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
                  Testerilizacionp[0]=0;
               }
            }
            if(Minutos){
               Testerilizacionp[2]++;
               if(Testerilizacionp[2]>9){
                  Testerilizacionp[3]++;
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
                  Testerilizacionp[0]=9;
               }
            }
            if(Minutos){
               Testerilizacionp[2]--;
               if(Testerilizacionp[2]<0){
                  Testerilizacionp[3]--;
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
                  Testerilizacionp[0]=9;
               }
            }
            if(Minutos){
               Testerilizacionp[2]--;
               if(Testerilizacionp[2]<0){
                  Testerilizacionp[3]--;
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
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
            printf("page MenuPrincipal");
            SendDataDisplay();  
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               write_eeprom(4,Tsecadop[0]);delay_ms(10);write_eeprom(5,Tsecadop[1]);delay_ms(10);
               write_eeprom(6,Tsecadop[2]);delay_ms(10);write_eeprom(7,Tsecadop[3]);delay_ms(10);
               Tsecado[0]=Tsecadop[0];
               Tsecado[1]=Tsecadop[1];
               Tsecado[2]=Tsecadop[2];
               Tsecado[3]=Tsecadop[3];
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
                  Tsecadop[0]++;
                  if(Tsecadop[0]>9){
                     Tsecadop[1]++;
                     Tsecadop[0]=0;
                  }
               }
               if(Minutos){
                  Tsecado[2]++;
                  if(Tsecadop[2]>9){
                     Tsecadop[3]++;
                     Tsecadop[2]=0;
                  }
               }  
            }
         }
         
         if(DOWN){
            delay_ms(200);
            if(DOWN){
               if(Segundos){
                  Tsecadop[0]--;
                  if(Tsecadop[0]<0){
                     Tsecadop[1]--;
                     Tsecadop[0]=9;
                  }
               }
               if(Minutos){
                  Tsecadop[2]--;
                  if(Tsecadop[2]<0){
                     Tsecadop[3]--;
                     Tsecadop[2]=9;
                  }
               }  
            }
         }
         
         if(RX_Buffer[4]==0x0d && !Op){//Tecla Arriba Oprimida
            if(Segundos){
               Tsecadop[0]++;
               if(Tsecadop[0]>9){
                  Tsecadop[1]++;
                  Tsecadop[0]=0;
               }
            }
            if(Minutos){
               Tsecado[2]++;
               if(Tsecadop[2]>9){
                  Tsecadop[3]++;
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
                  Tsecadop[0]=0;
               }
            }
            if(Minutos){
               Tsecadop[2]++;
               if(Tsecadop[2]>9){
                  Tsecadop[3]++;
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
                  Tsecadop[0]=9;
               }
            }
            if(Minutos){
               Tsecadop[2]--;
               if(Tsecadop[2]<0){
                  Tsecadop[3]--;
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
                  Tsecadop[0]=9;
               }
            }
            if(Minutos){
               Tsecadop[2]--;
               if(Tsecadop[2]<0){
                  Tsecadop[3]--;
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
         if(Setpoint<105)
            Setpoint=134;
         if(Setpoint>134)
            Setpoint=105;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page MenuPrincipal");
            SendDataDisplay();  
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               write_eeprom(8,Setpoint);
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay();
         if(Nivel<1)
            Nivel=6;
         if(Nivel>6)
            Nivel=1;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            if(Nivel<6)
               desfoguelento=1;
            else            
               desfoguelento=0;
            if(!GuardaEEPROM){    
               write_eeprom(9,Nivel);
               delay_ms(20);            
               NivelSeleccionado=9990*Nivel;
               GuardaEEPROM=ON;
            }
            if(MenuAnt==1){
               //Buzzer_on;
               printf("page MenuPrincipal");
               SendDataDisplay(); 
               PantallaPrincipal=1;
            }else if(MenuAnt==2){
               printf("page Funcionamiento");
               SendDataDisplay(); 
               if(Temperatura>60)TTrampa=TTrampa+5;
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
         
         if(RX_Buffer[4]==0x01)//Oprimio EV Desfogue
         {
            EstadoDesfogue=!EstadoDesfogue;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoDesfogue){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            Desfogue_on;
         }
         else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            Desfogue_off;
         }
         
         if(RX_Buffer[4]==0x02)//Oprimio EV Agua
         {
            EstadoAgua=!EstadoAgua;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAgua){
            printf("b1.bco=2016");
            SendDataDisplay(); 
            AguaIn_on;
         }
         else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            AguaIn_off;
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
        
         if(RX_Buffer[4]==0x05)//Oprimio Bloqueo
         {
            EstadoBloqueo=!EstadoBloqueo;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBloqueo){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            Solenoide_on;
         }
         else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            Solenoide_off;
         }
         
         if(RX_Buffer[4]==0x07)//Oprimio SSR1
         {
            EstadoSSR=!EstadoSSR;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSSR){
            printf("b6.bco=2016");
            SendDataDisplay(); 
            SSR_on;
         }
         else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
            SSR_off;
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
         
         if(HIG){
            printf("b16.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
         }
         
         if(LOW){
            printf("b17.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b17.bco=63488");
            SendDataDisplay(); 
         }
         
         if(UP){
            delay_ms(100);
            if(UP){
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
            }
         }
         
         if(DOWN){
            delay_ms(100);
            if(DOWN){
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
            }
         }
         
         printf("b13.txt=\"%3.0f\"",sensores(0));
         SendDataDisplay();
         printf("b14.txt=\"%3.0f\"",sensores(2));
         SendDataDisplay();
         printf("b15.txt=\"%3.0f\"",sensores(3));
         //printf("b15.txt=\"%3.0f\"",Presionp);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x21)//Oprimio salir
         {
            printf("page MenuPrincipal");
            SendDataDisplay();
            PantallaPrincipal=0;
            PruebaEstados=OFF;Desfogue_off;AguaIn_off;SSR_off;
            ApagaSalidas();
            // Falta apagar Todo
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
         
         if(Opcion==1){
            printf("tdia.bco=27501");
            SendDataDisplay();  
            printf("tdia.pco=65535");
            SendDataDisplay();  
            printf("tmes.bco=65535");
            SendDataDisplay();  
            printf("tmes.pco=0");
            SendDataDisplay();  
            printf("tyear.bco=65535");
            SendDataDisplay();  
            printf("tyear.pco=0");
            SendDataDisplay();  
            printf("thora.bco=65535");
            SendDataDisplay();  
            printf("thora.pco=0");
            SendDataDisplay();  
            printf("tminutos.bco=65535");
            SendDataDisplay();  
            printf("tminutos.pco=0");
            SendDataDisplay();  
         }else if(Opcion==2){
            printf("tdia.bco=65535");
            SendDataDisplay();  
            printf("tdia.pco=0");
            SendDataDisplay();  
            printf("tmes.bco=27501");
            SendDataDisplay();  
            printf("tmes.pco=65535");
            SendDataDisplay();  
            printf("tyear.bco=65535");
            SendDataDisplay();  
            printf("tyear.pco=0");
            SendDataDisplay();  
            printf("thora.bco=65535");
            SendDataDisplay();  
            printf("thora.pco=0");
            SendDataDisplay();  
            printf("tminutos.bco=65535");
            SendDataDisplay();  
            printf("tminutos.pco=0");
            SendDataDisplay();
         }else if(Opcion==3){
            printf("tdia.bco=65535");
            SendDataDisplay();  
            printf("tdia.pco=0");
            SendDataDisplay();  
            printf("tmes.bco=65535");
            SendDataDisplay();  
            printf("tmes.pco=0");
            SendDataDisplay();  
            printf("tyear.bco=27501");
            SendDataDisplay();  
            printf("tyear.pco=65535");
            SendDataDisplay();  
            printf("thora.bco=65535");
            SendDataDisplay();  
            printf("thora.pco=0");
            SendDataDisplay();  
            printf("tminutos.bco=65535");
            SendDataDisplay();  
            printf("tminutos.pco=0");
            SendDataDisplay();
         }else if(Opcion==4){
            printf("tdia.bco=65535");
            SendDataDisplay();  
            printf("tdia.pco=0");
            SendDataDisplay();  
            printf("tmes.bco=65535");
            SendDataDisplay(); 
            printf("tmes.pco=0");
            SendDataDisplay();  
            printf("tyear.bco=65535");
            SendDataDisplay();  
            printf("tyear.pco=0");
            SendDataDisplay();  
            printf("thora.bco=27501");
            SendDataDisplay();  
            printf("thora.pco=65535");
            SendDataDisplay();  
            printf("tminutos.bco=65535");
            SendDataDisplay();  
            printf("tminutos.pco=0");
            SendDataDisplay();
         }else if(Opcion==5){
            printf("tdia.bco=65535");
            SendDataDisplay();  
            printf("tdia.pco=0");
            SendDataDisplay();  
            printf("tmes.bco=65535");
            SendDataDisplay(); 
            printf("tmes.pco=0");
            SendDataDisplay();  
            printf("tyear.bco=65535");
            SendDataDisplay();  
            printf("tyear.pco=0");
            SendDataDisplay();  
            printf("thora.bco=65535");
            SendDataDisplay();  
            printf("thora.pco=0");
            SendDataDisplay();  
            printf("tminutos.bco=27501");
            SendDataDisplay();  
            printf("tminutos.pco=65535");
            SendDataDisplay();
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
               //BorraBuffer();
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
   if(Menu==20){ // Este es el menu de cuando el proceso se inicio.
   //Lectura_Esclavos();
   
   //Envio_Esclavos();
   if(CicloSeleccionado==1){
      printf("t2f.txt=\"1\"");
      SendDataDisplay();
      printf("t3f.txt=\"PLASTICO\"");
      SendDataDisplay();
   }else if(CicloSeleccionado==2){
      printf("t2f.txt=\"2\"");
      SendDataDisplay();
      printf("t3f.txt=\"INSTRUMENTAL\"");
      SendDataDisplay();
   }else if(CicloSeleccionado==3){
      printf("t2f.txt=\"3\"");
      SendDataDisplay();
      printf("t3f.txt=\"LIQUIDOS A\"");
      SendDataDisplay();
   }else if(CicloSeleccionado==4){
      printf("t2f.txt=\"4\"");
      SendDataDisplay();
      printf("t3f.txt=\"LIQUIDOS B\"");
      SendDataDisplay();
   }else if(CicloSeleccionado==5){
      printf("t2f.txt=\"5\"");
      SendDataDisplay();
      printf("t3f.txt=\"LIBRE\"");
      SendDataDisplay();
   }
   
   if(RIGHT){
      //glcd_init(ON);
      delay_ms(1000);
      flagCalentando=OFF;
      flagDesfogando=OFF;
      flagEsterilizando=OFF;
   }
   
   if(PresionPreCamara>302 || PresionCamara>255){
      AlarmaPresion=ON;
   }
   
   if(TiempoCiclo>1800){
      if(PresionPrecamara<10){
         AlarmaTiempoProlongado=ON;
      }
   }else{
      AlarmaTiempoProlongado=OFF;
   }
   
   if(Temperatura>Setpoint+3.0){
      AlarmaSobreTemperatura=ON;
   }
   
   if(TiempoLlenado>600){
      AlarmaLlenado=ON;
   }else{
      AlarmaLlenado=OFF;
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
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaLlenado || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion){
      ActivaAlarma(2);
      if(AlarmaEmergencia)
      {
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue_on;
         AguaIn_off;
         SSR_off;
         Paso_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         DesfogueSuave=OFF;
         //IniciaCiclo=OFF;
         Controlando=0;
         if(!flagImprimir && ActivaImpresion==1){
            Imprime=20;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
      }else{
         if(AlarmaPuerta)
         {
            printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Pausado\"");
            SendDataDisplay();
            SSR_off;
            Paso_off;
            IniciaLlenado=OFF;
            IniciaCiclo=OFF;
            Controlando=0;
            Solenoide_off;
         }else{
            if(AlarmaLlenado)
            {
               printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Cancelado\"");
               SendDataDisplay();
               Desfogue_off;
               AguaIn_off;
               SSR_off;
               Paso_off;
               Controlando=0;
               if(!flagImprimir && ActivaImpresion==1){
                  Imprime=21;
                  Envio_Esclavos();
                  delay_ms(100);
                  Imprime=10;
                  flagImprimir=1;
               }
            }else{
               if(AlarmaTermostato) // Alarma de Termostato
               {
                  tiempo_esterilizacion=0;tiempo_secado=0;         
                  printf("Funcionamiento.t4f.txt=\" Termostato\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Cancelado\"");
                  SendDataDisplay();
                  Desfogue_on;
                  AguaIn_off;
                  SSR_off;
                  Paso_off;
                  Esterilizando=OFF;
                  Desfogando=OFF;
                  IniciaCiclo=OFF;
                  Controlando=0;
                  if(!flagImprimir && ActivaImpresion==1){
                     Imprime=22;
                     Envio_Esclavos();
                     delay_ms(100);
                     Imprime=10;
                     flagImprimir=1;
                  }
               }else{
                  if(AlarmaSobreTemperatura) // Alarma de Sobre Temperatura
                  {
                     tiempo_esterilizacion=0;tiempo_secado=0;         
                     printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
                     SendDataDisplay();
                     printf("Funcionamiento.t5f.txt=\" Cancelado\"");
                     SendDataDisplay();  
                     Desfogue_on;
                     SSR_off;
                     Paso_off;
                     Controlando=0;
                     AguaIn_off;
                     if(!flagImprimir && ActivaImpresion==1){
                        Imprime=23;
                        Envio_Esclavos();
                        delay_ms(100);
                        Imprime=10;
                        flagImprimir=1;
                     }
                  }else{
                     if(AlarmaTiempoProlongado) // Alarma de Error de Tiempo: Error por exceso de tiempo de calentamiento.
                     {
                        printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Cancelado\"");
                        SendDataDisplay();
                        Desfogue_on;
                        AguaIn_off;
                        SSR_off;
                        Paso_off;
                        Controlando=0;
                        if(!flagImprimir && ActivaImpresion==1){
                           Imprime=24;
                           Envio_Esclavos();
                           delay_ms(100);
                           Imprime=10;
                           flagImprimir=1;
                        }
                     }else{
                        if(AlarmaPresion) // Alarma de Error de SobrePresion: Error por exceso de Presion.
                        {
                           printf("Funcionamiento.t4f.txt=\" SobrePresión\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Cancelado\"");
                           SendDataDisplay();
                           Desfogue_on;
                           AguaIn_off;
                           SSR_off;
                           if(PresionPreCamara>20 && DOOR)
                              Paso_on;
                           else
                              Paso_off;
                           Controlando=0;
                           if(!flagImprimir && ActivaImpresion==1){
                              Imprime=24;
                              Envio_Esclavos();
                              delay_ms(100);
                              Imprime=10;
                              flagImprimir=1;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else
   {  
      Buzzer_off;
      IniciaCiclo=ON;
      if(flagImprime && ActivaImpresion==1){
         Imprime=5;
         Envio_Esclavos();
         delay_ms(100);
         Imprime=10;
         flagImprime=0;
      }
         if(!CalderinLleno){
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            printf("Funcionamiento.t5f.txt=\" Llenando\"");
            SendDataDisplay();
            Control=0;
            IniciaLlenado=ON;
            Controlando=0;
            Desfogue_on;
            AguaIn_on;
            Paso_off;
         }
         else{
            AguaIn_off;
            if(Temperatura>=((float)Setpoint-0.3)){
               Esterilizando=ON;
            }
            
            if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
               Esterilizando=OFF;
            }
            
            if(!Esterilizando){
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Calentando\"");
               SendDataDisplay();
               Control=1;
               Controla();
               if(PresionPreCamara>100){
                  Paso_on;
               }
               if(Temperatura<=TTrampa){
                  if(PresionCamara>5){
                     Desfogue_on;
                  }else if(PresionCamara<2){
                     Desfogue_off;
                  }
               }else{
                  Desfogue_off;
               }
            }else{
               if((Testerilizacion[0]==0)&&(Testerilizacion[1]==0)&&
               (Testerilizacion[2]==0)&&(Testerilizacion[3]==0)){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<25){
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        Paso_off;
                        if(!DOOR){
                           Desfogue_off;
                           Solenoide_off;
                           Paso_off;
                        }else{
                           Desfogue_on;
                           Solenoide_off;
                           Paso_off;
                        }
                        if(PresionCamara<2){
                           if((RX_Buffer[3]==0x0f || UP) && DOOR){//0f, recibe caracteres ingresados desde el Display
                              delay_ms(50);
                              if((RX_Buffer[3]==0x0f || UP) && DOOR){
                                 Solenoide_on;
                                 delay_ms(2000);
                                 Solenoide_off;
                              }
                           }
                           ActivaAlarma(2);
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                           SendDataDisplay();
                           Finalizo=ON;
                           if(!Ciclo){
                              Ciclo=ON;
                              Ciclos++;
                              CiclosL=make8(Ciclos,0);
                              CiclosH=make8(Ciclos,1);
                              write_eeprom(37,CiclosL);
                              delay_ms(20);
                              write_eeprom(38,CiclosH);
                              delay_ms(20);
                              if(ActivaImpresion==1){
                                 Imprime=25;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                              }
                           }
                        }
                     }else{
                        Desfogando=ON;
                        DesfogueSuave=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Control=0;
                        SSR_off;
                        Paso_off;
                     }
                  }else{
                     //SSR_off;
                     Paso_off;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<2){
                        if((Tsecado[0]==0)&&(Tsecado[1]==0)&&(Tsecado[2]==0)&&(Tsecado[3]==0)){
                           if((RX_Buffer[3]==0x0f || UP) && DOOR){//0f, recibe caracteres ingresados desde el Display
                              delay_ms(50);
                              if((RX_Buffer[3]==0x0f || UP) && DOOR){
                                 Solenoide_on;
                                 delay_ms(2000);
                                 Solenoide_off;
                              }
                           }
                           if(!DOOR){
                              Desfogue_off;
                              Solenoide_off;
                              Paso_off;
                           }else{
                              Desfogue_on;
                              Solenoide_off;
                              Paso_off;
                           }
                           SSR_off;
                           ActivaAlarma(2);
                           Secando=OFF;
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                           SendDataDisplay();
                           Finalizo=ON;
                           if(!Ciclo){
                              Ciclo=ON;
                              Ciclos++;
                              CiclosL=make8(Ciclos,0);
                              CiclosH=make8(Ciclos,1);
                              write_eeprom(37,CiclosL);
                              delay_ms(20);
                              write_eeprom(38,CiclosH);
                              delay_ms(20);
                              if(ActivaImpresion==1){
                                 Imprime=25;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                              }
                           }
                           Secando=OFF;
                        }else{
                           Secando=ON;
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Secando\"");
                           SendDataDisplay();
                           Paso_off;
                           CalientaChaqueta();
                        }
                     }else{
                        Desfogando=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Control=0;
                        SSR_off;
                        Controlando=0;
                        Paso_off;
                        Desfogue_on;
                     }
                  }
                  
               }else{
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

   if(Grafica==1){
      printf("add 14,0,%1.0f",Temperatura*1.5);
      SendDataDisplay();
      Grafica=0;
   }
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
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
      if(Menu==100){ //Menu de Configuración de Parametros de Control
     
      if(Opcion2>10)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=10;
      
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
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Ganancia-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
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
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Derivativo--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
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
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Integral--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
         
         if(Integral>40)
            Integral=1;
         if(Integral<1)
            Integral=40;
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
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Tciclo-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
         
         if(Tciclo>20)
            Tciclo=5;
         if(Tciclo<5)
            Tciclo=20;
      }else if(Opcion2==5){
         printf("Config.t2c.txt=\"Ajuste +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Ajuste<40)
                  Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Ajuste>0)
                  Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==6){
         printf("Config.t2c.txt=\"Ajuste -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Ajusten<40)
                  Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Ajusten>0)
                  Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==7){
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
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(MediaMovil>10)
                  MediaMovil--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==8){
         printf("Config.t2c.txt=\"RPT100B\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.1f\"",RPT100);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(RPT100<2.0)
                  RPT100+=0.1;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(RPT100>0.0)
                  RPT100-=0.1;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
               //BorraBuffer();
            }
         }
      }else if(Opcion2==9){
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
      }else if(Opcion2==10){
         printf("Config.t2c.txt=\"Ciclos\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%03Lu\"",Ciclos);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x0c || LEFT){//Tecla Izquierda Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0c || LEFT){
            Opcion2++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
      }
         
      if(RX_Buffer[4]==0x0d || RIGHT){//Tecla Derecha Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0d || RIGHT){
            Opcion2--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
      }
      
      if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){//Tecla Regresar
         delay_ms(20);
         if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){
            if(!GuardaEEPROM){
               write_eeprom(20,(int8)Ganancia);
               delay_ms(10);
               write_eeprom(26,(int8)Integral);
               delay_ms(10);
               write_eeprom(27,(int8)Derivativo);
               delay_ms(10);
               write_eeprom(25,(int8)Tciclo);
               delay_ms(10);
               write_eeprom(21,Ajuste);
               delay_ms(10);
               write_eeprom(22,Ajusten);
               delay_ms(10);
               write_eeprom(24,(int8)MediaMovil);
               delay_ms(10);
               write_eeprom(32,(int8)RPT100);
               delay_ms(10);
               Temporal=RPT100-((int8)RPT100);
               Temporal=Temporal*10;
               write_eeprom(33,(int8)Temporal);
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
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay(); 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

   }
}
