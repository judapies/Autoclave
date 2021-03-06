//Falta crear menu de ajuste de variables y de norma CFR21

#include <18F4685.h>
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#include <LCD420.c>

#define   START         !input(PIN_B2)
#define   DOWN          !input(PIN_B3)

#define   DOOR          input(PIN_A4)
#define   RESR          input(PIN_A5)
#define   TERM          !input(PIN_A3)

#define   LOW           !input(PIN_A2)

#define   SSR_on           output_bit(PIN_E2,1)
#define   SSR_off          output_bit(PIN_E2,0)

#define   Desfogue_on      output_bit(PIN_C0,1)
#define   Desfogue_off     output_bit(PIN_C0,0)

#define   AguaIn_on        output_bit(PIN_C1,1)
#define   AguaIn_off       output_bit(PIN_C1,0)

#define   Solenoide_on     output_bit(PIN_C2,1)
#define   Solenoide_off    output_bit(PIN_C2,0)

#define   Aux_on           output_bit(PIN_C3,1)
#define   Aux_off          output_bit(PIN_C3,0)

#define   LedDoor_on       output_bit(PIN_B7,1)
#define   LedDoor_off      output_bit(PIN_B7,0)

#define   LedWater_on      output_bit(PIN_B6,1)
#define   LedWater_off     output_bit(PIN_B6,0)

#define   LedFail_on       output_bit(PIN_E0,1)
#define   LedFail_off      output_bit(PIN_E0,0)

#define   LedStart_on      output_bit(PIN_E1,1)
#define   LedStart_off     output_bit(PIN_E1,0)

#define   Buzzer_on        output_bit(PIN_C4,1)
#define   Buzzer_off       output_bit(PIN_C4,0)

#define OFF 0
#define ON  1

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaLlenado=OFF,AlarmaTermostato=OFF,AlarmaPuerta=OFF,AlarmaEmergencia=OFF,AlarmaPresion=OFF;
short AlarmaCiclo=OFF;

//Banderas//--------------------
short IniciaLlenado=OFF,CalderinLleno=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short Ciclo=OFF;
short PARAMETERS=OFF,PROGRAM=OFF,CYCLE=OFF,UP=OFF,eDesfogue=OFF,eLlenado=OFF,eSolenoide=OFF,eSSR=OFF,eBuzzer=OFF,Codigo=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoLlenado=0,TiempoSensadoNivel=0,Tp=0,Tc=0;

//Control//------------------------
float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0;
int8 Programa=1,sEst=0,mEst=0,sEstP=0,mEstP=0,sSec=0,mSec=0,sSecP=0,mSecP=0,Test=1;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=20,TTrampa=85;
float V0=0.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=2.0;
float V1=0.0,Presion=0.0;
float Temperatura=0.0,K=0.006429;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0,Config=0,code=0,Parametro=1;

//Varios//--------------------------------
short desfoguelento=0,Lectura=0;
short flagTC=0,flagStart=OFF;
int8 Control=0,tiemporecta=0,Controlando=0,tStart=0;
int8 Menu=0,i=0,CiclosL=0,CiclosH=0;
int8 Setpoint=0,tiempo_esterilizacion=0,tiempo_secado=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Nivel=1,tinicio=0,PresionCamara=0,Ciclos=0,NivelSeleccionado=0;
int8 r=0,t=0,TiempoControl=0;
int8 y=0;
int16 t_exhaust=0;
int8 DSet[5]={134,121,134,121,121};
int8 DTime[5]={3,15,12,30,30};
int8 DTimeS[5]={0,0,30,30,0};
int8 DExh[5]={1,1,1,1,4};
int8 Set[5]={134,121,134,121,121};
int8 Time[5]={3,15,12,30,30};
int8 TimeS[5]={0,0,30,30,0};
int8 Exh[5]={1,1,1,1,4};
char const Cycles[5][12]={"1-Fast 134","2-Fast 121","3-W.Dry 134","4-W.Dry 121","5-Slow  121"};
char const Testeo[9][12]={"EV Exhaust ","EV Llenado ","Solenoide  ","Resistencia","Buzzer     ","SW Puerta  ","SW Reserva ","Termostato ","NivelTanque"};
char const estado[2][4]={"OFF","ON "};

#include "Funciones.h"

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(DesfogueSuave){
      if(Nivel<6){
         if(t_exhaust<=NivelSeleccionado) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
            Desfogue_on;
         else
            Desfogue_off;
      
         if(t_exhaust>=60000) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
            t_exhaust=0;
          // tiempoctrl incrementa una unidad cada Xms, por eso se dividen los tiempos por 0.005.
      }else{
         Desfogue_on;
      }
   }  
}

#int_TIMER1
void temp1s(void){
   
   set_timer1(40536); // Se repite cada 5ms
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   leerTeclado();
   
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
      if(IniciaCiclo && !Esterilizando){
         TiempoCiclo++;
      }
      
      if(flagStart)
         tStart++;
      
      if(IniciaLlenado && !CalderinLleno){
         TiempoLlenado++;
      }
      
      if(SensadoNivel){
         TiempoSensadoNivel++;
      }
      
      tiempos=0;Lectura=1;tinicio++;
   
      if(Esterilizando && !Desfogando){               
         if(sEst>0){
            sEst--;
         }else{
            if(mEst>0){
               mEst--;
               sEst=59;
            }
         }
      }       
      
      if(Secando){               
         if(sSec>0){
            sSec--;
         }else{
            if(mSec>0){
               mSec--;
               sSec=59;
            }
         }    
      }
   }
}


void main(){
   LedDoor_off;
   LedWater_off;
   Buzzer_on;
   delay_ms(100);
   Buzzer_off;
   lcd_init();
   port_b_pullups(true);
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN1);
   setup_adc(ADC_CLOCK_DIV_32 );
   enable_interrupts(global);
   LeeEEPROM();
   delay_ms(1);
   LimitaValores();
   
//Mensaje Bienvenida//-----------------------------   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"   Autoclave    ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"  Tuttnauer JP  ");
   delay_ms(2500);
   printf(lcd_putc,"\f");
   
   for(i=0;i<80;i++){
      Temperatura=LeerPT100(80);
   }
   
   if(CYCLE){
      printf(lcd_putc,"\f");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"Loading....");
      resetParams();
   }
   
   if(START && UP){
      Menu=100;
   }
   
   while(true)
   {
      if(RESR){
         LedWater_on;
      }else{
         LedWater_off;
      }
      
      if(DOOR){
         LedDoor_on;
      }else{
         LedDoor_off;
      }
      output_bit(PIN_E1,IniciaCiclo);
      delay_ms(1);
      
      // Monitoreo Nivel de Agua//--------------------------------------
            if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
               TiempoSensadoNivel=0;
               Controlando=0;   
               SensadoNivel=OFF;
            } else{
               if(TiempoSensadoNivel>3){
                  SensadoNivel=OFF;
                  CalderinLleno=ON;
                  IniciaLlenado=OFF;                  
               }else{
                  SensadoNivel=ON;
               }
            }

// Tiempo de Muestreo//--------------------------------------
      if(tiempos3>=20){
         Temperatura=LeerPt100(80);
         PresionCamara=Leer_Sensor_Presion_Camara(5);
         tiempos3=0;
      }
      
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==0){ //Menu Principal.
      if(CYCLE){
         Programa++; 
         delay_ms(100);
      }
     
      if(Programa>5)
         Programa=1;
         
      if(PARAMETERS){         
         lcd_gotoxy(1,2);
         printf(lcd_putc,"%03u?C E=%02um S=%02u",Set[Programa-1],Time[Programa-1],TimeS[Programa-1]);
         delay_ms(3000);
         printf(lcd_putc,"\f");
      }
      Solenoide_on;
      lcd_gotoxy(1,2);
      printf(lcd_putc,"%3.1f ",Temperatura);
      lcd_gotoxy(13,2);
      printf(lcd_putc,"%03LuK",PresionCamara);
      lcd_gotoxy(1,1);
      printf(lcd_putc,"%s",Cycles[Programa-1]);
      lcd_gotoxy(12,1);
      printf(lcd_putc,"ST.BY");
      
      if(START){
         Setpoint=Set[Programa-1];
         mEstP=Time[Programa-1];
         mSecP=TimeS[Programa-1];
         sEstP=0;
         sSecP=0;
         mEst=mEstP;
         sEst=sEstP;
         mSec=mSecP;
         sSec=sSecP;
         Menu=20;
         lcd_gotoxy(1,2);
         printf(lcd_putc,"%05Lu ",Ciclos);
         delay_ms(2000);
         if(Exh[Programa-1]!=1)desfoguelento=1;
         NivelSeleccionado=9990*Nivel;
         printf(lcd_putc,"\f");
      }
      
      if(UP && DOWN){
         Menu=1;
         delay_ms(100);
         printf(lcd_putc,"\f");
      }
      
      if(PROGRAM){
         Menu=2;
         delay_ms(100);
         printf(lcd_putc,"\f");
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==1){ //Test de componentes.
      if(UP){
         Test++; 
         printf(lcd_putc,"\f");
         delay_ms(100);
      }
      
      if(DOWN){
         Test--; 
         printf(lcd_putc,"\f");
         delay_ms(100);
      }
      
      if(Test>9)
         Test=1;
         
      if(Test<1)
         Test=9; 
         
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Test Componentes");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"%s",Testeo[Test-1]);  
      
      if(CYCLE){
         Menu=0;
         delay_ms(500);
         printf(lcd_putc,"\f");
         Desfogue_off;
         AguaIn_off;
         Aux_off;
         Solenoide_off;
         Buzzer_off;
         SSR_off;
      }
      
      if(Test==1){
         if(START){
            delay_ms(500);
            eDesfogue=!eDesfogue;            
            output_bit(PIN_C0,eDesfogue);
         }
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[eDesfogue]);
      }else if(Test==2){
         if(START){
            delay_ms(500);
            eLlenado=!eLlenado;
            output_bit(PIN_C1,eLlenado);
            output_bit(PIN_C3,eLlenado);
         }
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[eLlenado]);
      }else if(Test==3){
         if(START){
            delay_ms(500);
            eSolenoide=!eSolenoide;
            output_bit(PIN_C2,eSolenoide);
         }
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[eSolenoide]);
      }else if(Test==4){
         if(START){
            delay_ms(500);
            eSSR=!eSSR;            
            output_bit(PIN_E2,eSSR);
         }
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[eSSR]);
      }else if(Test==5){
         if(START){
            delay_ms(500);
            eBuzzer=!eBuzzer;
            output_bit(PIN_C4,eBuzzer);
         }
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[eBuzzer]);
      }else if(Test==6){
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[!DOOR]);
      }else if(Test==7){
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[!RESR]);
      }else if(Test==8){
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[!TERM]);
      }else if(Test==9){
         lcd_gotoxy(13,2);
         printf(lcd_putc,"%s",estado[LOW]);
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==2){ //Menu Parametros.
      lcd_gotoxy(1,1);
      printf(lcd_putc,"%s",Cycles[Programa-1]);
      if(!Codigo){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"CODE:%03u",code);
         
         if(UP){
            code++;
            delay_ms(100);
         }
         
         if(DOWN){
            if(code>0)
               code--;
            delay_ms(100);
         }
         
         if(PROGRAM){
            if(code==1) {
               Codigo=ON;
               printf(lcd_putc,"\f");
            }else if(code==11){
               Codigo=ON;
               printf(lcd_putc,"\f");
            }else if(code==33){
               Codigo=ON;
               printf(lcd_putc,"\f");
            }else{
               code=0;
               printf(lcd_putc,"\f");
            }
         }
      }else{
         if(code==1){ 
            if(Parametro>3)
               Parametro=1;
            
            if(Parametro==1){
               if(Set[Programa-1]<60)Set[Programa-1]=60;
               if(Set[Programa-1]>137)Set[Programa-1]=137;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"SteTemp   %03u ",Set[Programa-1]);
               if(UP){
                  Set[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  Set[Programa-1]--;
                  delay_ms(100);
               }
               
               if(PROGRAM){
                  write_eeprom(100+((Programa-1)*4),Set[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }else if(Parametro==2){
               if(Time[Programa-1]<3)Time[Programa-1]=3;
               if(Time[Programa-1]>99)Time[Programa-1]=99;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"SteTime   %03u ",Time[Programa-1]);
               if(UP){
                  Time[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  Time[Programa-1]--;            
                  delay_ms(100);
               }
               
               if(PROGRAM){
                  write_eeprom(101+((Programa-1)*4),Time[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }else if(Parametro==3){
               if(TimeS[Programa-1]>99)TimeS[Programa-1]=99;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"Dry Time   %03u ",TimeS[Programa-1]);
               if(UP){
                  TimeS[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  if(TimeS[Programa-1]>0)
                     TimeS[Programa-1]--; 
                  delay_ms(100);   
               }
               if(PROGRAM){
                  write_eeprom(102+((Programa-1)*4),TimeS[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }
         }else if(code==11 || code==33){ 
            if(Parametro>4)
               Parametro=1;
            
            if(Parametro==1){
               if(Set[Programa-1]<60)Set[Programa-1]=60;
               if(Set[Programa-1]>137)Set[Programa-1]=137;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"SteTemp   %03u ",Set[Programa-1]);
               if(UP){
                  Set[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  Set[Programa-1]--;
                  delay_ms(100);
               }
               
               if(PROGRAM){
                  write_eeprom(100+((Programa-1)*4),Set[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }else if(Parametro==2){
               if(Time[Programa-1]<3)Time[Programa-1]=3;
               if(Time[Programa-1]>99)Time[Programa-1]=99;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"SteTime   %03u ",Time[Programa-1]);
               if(UP){
                  Time[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  Time[Programa-1]--;            
                  delay_ms(100);
               }
               
               if(PROGRAM){
                  write_eeprom(101+((Programa-1)*4),Time[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }else if(Parametro==3){
               if(TimeS[Programa-1]>99)TimeS[Programa-1]=99;

               lcd_gotoxy(1,2);
               printf(lcd_putc,"Dry Time   %03u ",TimeS[Programa-1]);
               if(UP){
                  TimeS[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  if(TimeS[Programa-1]>0)
                     TimeS[Programa-1]--; 
                  delay_ms(100);   
               }
               if(PROGRAM){
                  write_eeprom(102+((Programa-1)*4),TimeS[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }else if(Parametro==4){
               if(Exh[Programa-1]>4)Exh[Programa-1]=1;
               if(Exh[Programa-1]<1)Exh[Programa-1]=4;
               
               lcd_gotoxy(1,2);
               printf(lcd_putc,"Ex Mode    %03u ",Exh[Programa-1]);
               if(UP){
                  Exh[Programa-1]++;
                  delay_ms(100);
               }
            
               if(DOWN){
                  Exh[Programa-1]--;            
                  delay_ms(100);
               }
               if(PROGRAM){
                  write_eeprom(103+((Programa-1)*4),Exh[Programa-1]);
                  delay_ms(20);
                  Buzzer_on;
                  delay_ms(100);
                  Buzzer_off;
                  Parametro++;
                  printf(lcd_putc,"\f");
               }
            }
         }
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   if(Menu==20){ // Este es el menu de cuando el proceso se inicio.
  
   if(TiempoCiclo>5400){
      AlarmaTiempoProlongado=ON;
   }else{
      AlarmaTiempoProlongado=OFF;
   }
   
   if(PresionCamara>260){
      AlarmaPresion=ON;
   }
   
   if(Temperatura>Setpoint+3.0){
      AlarmaSobreTemperatura=ON;
   }
   
   if(TiempoLlenado>600){
      AlarmaLlenado=ON;
   }else{
      AlarmaLlenado=OFF;
   }
   
   if(!TERM && PresionCamara<20){
      delay_ms(500);
      if(!TERM && PresionCamara<20){
         delay_ms(500);
         if(!TERM && PresionCamara<20){
            AlarmaTermostato=ON;
         }
      }
   }
   
   if(DOOR){
      if(!Secando){
         AlarmaPuerta=ON;
      }
   }else{
      AlarmaPuerta=OFF;
   }
   
   if(START){
      flagStart=ON;
   }else{
      flagStart=OFF;
      tStart=0;
   }
   
   if(tStart>3){
      AlarmaEmergencia=ON;
   }
   
   if(DOWN && !IniciaCiclo && PresionCamara<=2){
      Solenoide_on;
      Buzzer_on;
      delay_ms(10000);
      Solenoide_off;
      Buzzer_off;
   }else{
      Solenoide_off;
   }
   
   if(Temperatura>100 && PresionCamara<10 && !Desfogando){
      AlarmaCiclo=ON;
   }
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaLlenado || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion ||
   AlarmaCiclo){
      if(AlarmaEmergencia){
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Desfogue_on;
         AguaIn_off;
         Aux_off;
         SSR_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         DesfogueSuave=OFF;
         Controlando=0;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"STOP ");
         LedFail_on;
      }else if(AlarmaPuerta){
         SSR_off;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         AguaIn_off;
         Aux_off;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"DOOR ");
      }else if(AlarmaLlenado){
         Desfogue_off;
         AguaIn_off;
         Aux_off;
         SSR_off;            
         Controlando=0;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"FILL ");
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         LedFail_on;
      }else if(AlarmaTermostato){
         tiempo_esterilizacion=0;tiempo_secado=0;         
         Desfogue_on;
         AguaIn_off;
         Aux_off;
         SSR_off;
         Esterilizando=OFF;
         Desfogando=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"TERM ");
         LedFail_on;
      }else if(AlarmaSobreTemperatura){
         tiempo_esterilizacion=0;tiempo_secado=0;                              
         Desfogue_on;
         SSR_off;
         Controlando=0;
         AguaIn_off;
         Aux_off;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"SOBRE");
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         LedFail_on;
      }else if(AlarmaTiempoProlongado){
         Desfogue_on;
         SSR_off;
         Controlando=0;
         AguaIn_off;
         Aux_off;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"TIME ");
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         LedFail_on;
      }else if(AlarmaPresion){
         Desfogue_on;
         SSR_off;
         Controlando=0;
         AguaIn_off;
         Aux_off;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"PRES ");
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         LedFail_on;
      }else if(AlarmaCiclo){
         Desfogue_on;
         SSR_off;
         Controlando=0;
         AguaIn_off;
         Aux_off;
         lcd_gotoxy(12,1);
         printf(lcd_putc,"LPRES");
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         LedFail_on;
      }
   }else{  
         if(!CalderinLleno){
            lcd_gotoxy(12,1);
            printf(lcd_putc,"WATER");
            Control=0;
            IniciaLlenado=ON;
            Controlando=0;
            AguaIn_on;
            Aux_on;
            Desfogue_on;
            IniciaCiclo=ON;
         }else{
            AguaIn_off;
            Aux_off;
            if(Temperatura>=((float)Setpoint-0.3)){
               Esterilizando=ON;
            }
            
            if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
               Esterilizando=OFF;
            }
            
            if(!Esterilizando){               
               Control=1;
               Controla();
               Desfogue_off;
               lcd_gotoxy(12,1);
               printf(lcd_putc,"HEAT ");
               IniciaCiclo=ON;
            }else{
               TiempoCiclo=0;
               if(mEst==0 && sEst==0){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<25){
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        if(PresionCamara<2){
                           if(!Ciclo){
                              Ciclo=ON;
                              Ciclos++;
                              CiclosL=make8(Ciclos,0);
                              CiclosH=make8(Ciclos,1);
                              write_eeprom(37,CiclosL);
                              delay_ms(20);
                              write_eeprom(38,CiclosH);
                              delay_ms(20);
                              for(i=0;i<4;i++){
                                 Buzzer_on;
                                 delay_ms(1000);
                                 Buzzer_off;
                                 delay_ms(1000);
                              }  
                           }
                           Desfogue_on;
                        }
                        lcd_gotoxy(12,1);
                        printf(lcd_putc,"END  ");
                        lcd_gotoxy(7,2);
                        printf(lcd_putc,"     ");
                        IniciaCiclo=OFF;
                     }else{
                        Desfogando=ON;
                        DesfogueSuave=ON;
                        Control=0;
                        SSR_off;
                        lcd_gotoxy(12,1);
                        printf(lcd_putc,"EXH  ");
                        lcd_gotoxy(7,2);
                        printf(lcd_putc,"     ");
                     }
                  }else{
                     Desfogue_on;
                     SSR_off;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<2){
                        if(mSec==0 && sSec==0){
                           Solenoide_off;
                           Secando=ON;
                           IniciaCiclo=OFF;
                           if(!Ciclo){
                              Ciclo=ON;
                              Ciclos++;
                              CiclosL=make8(Ciclos,0);
                              CiclosH=make8(Ciclos,1);
                              write_eeprom(37,CiclosL);
                              delay_ms(20);
                              write_eeprom(38,CiclosH);
                              delay_ms(20);
                              for(i=0;i<4;i++){
                                 Buzzer_on;
                                 delay_ms(1000);
                                 Buzzer_off;
                                 delay_ms(1000);
                              }
                           }
                           delay_ms(2000);
                           lcd_gotoxy(12,1);
                           printf(lcd_putc,"END  ");
                           lcd_gotoxy(7,2);
                           printf(lcd_putc,"     ");
                        }else{
                           Secando=ON;  
                           lcd_gotoxy(12,1);
                           printf(lcd_putc,"DRY  ");
                           lcd_gotoxy(7,2);
                           printf(lcd_putc,"%02u:%02u",mSec,sSec);
                           Desfogue_on;
                           Solenoide_on;
                        }
                     }else{
                        Desfogando=ON;
                        lcd_gotoxy(12,1);
                        printf(lcd_putc,"EXH  ");
                        Control=0;
                        SSR_off;
                        Controlando=0;
                        lcd_gotoxy(7,2);
                        printf(lcd_putc,"     ");
                        Desfogue_on;
                     }
                  }
                  
               }else{
                  lcd_gotoxy(12,1);
                  printf(lcd_putc,"STER ");
                  Control=1;
                  Controla();
                  lcd_gotoxy(7,2);
                  printf(lcd_putc,"%02u:%02u",mEst,sEst);
               }
            }
            
         }
   }

   if(Lectura==1){// Utilizado para visualizar la lectura de la temperatura        
      Lectura=0;
      lcd_gotoxy(1,2);
      printf(lcd_putc,"%3.1f ",Temperatura);
      lcd_gotoxy(13,2);
      printf(lcd_putc,"%03LuK",PresionCamara);
      lcd_gotoxy(1,1);
      printf(lcd_putc,"%s",Cycles[Programa-1]);
   }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==100){ // Configuraciones
      if(PARAMETERS){
         Config++; 
         delay_ms(100);
         printf(lcd_putc,"\f");
      }
      
      if(PROGRAM){
         delay_ms(500);
         if(PROGRAM){
            write_eeprom(20,(int8)Ganancia);
            delay_ms(10);
            write_eeprom(21,Ajuste);
            delay_ms(10);
            write_eeprom(22,Ajusten);
            delay_ms(10);
            write_eeprom(26,Integral);            
            delay_ms(10);
            write_eeprom(27,Derivativo);
            delay_ms(10);
            write_eeprom(39,Ttrampa);
            delay_ms(10); 
            Buzzer_on;
            delay_ms(1000);
            reset_cpu();
         }
      }
      
      if(Config>6)
         Config=1;
         
      if(Config<1)
         Config=6; 
         
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Configuraciones ");      
      
      if(Config==1){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"P");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02.0f",Ganancia);
         if(UP){
            delay_ms(100);
            Ganancia+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            Ganancia-=1.0;
         }
         if(Ganancia>30)Ganancia=30;
         if(Ganancia<1)Ganancia=1;
      }else if(Config==2){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"D");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02u",Derivativo);
         if(UP){
            delay_ms(100);
            Derivativo+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            Derivativo-=1.0;
         }
         if(Derivativo>80)Derivativo=80;
         if(Derivativo<1)Derivativo=1;
      }else if(Config==3){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"I");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02u",Integral);
         if(UP){
            delay_ms(100);
            Integral+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            Integral-=1.0;
         }
         if(Integral>3)Integral=3;
         if(Integral<1)Integral=1;
      }else if(Config==4){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"TTrampa");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02u",TTrampa);
         if(UP){
            delay_ms(100);
            TTrampa+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            TTrampa-=1.0;
         }
         if(TTrampa>100)TTrampa=100;
         if(TTrampa<80)TTrampa=80;
      }else if(Config==5){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"Aj+");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02u",Ajuste);
         if(UP){
            delay_ms(100);
            Ajuste+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            Ajuste-=1.0;
         }
         if(Ajuste>10)Ajuste=10;
      }else if(Config==6){
         lcd_gotoxy(1,2);
         printf(lcd_putc,"Aj-");
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%02u",Ajusten);
         if(UP){
            delay_ms(100);
            Ajusten+=1.0;
         }
         if(DOWN){
            delay_ms(100);
            Ajusten-=1.0;
         }
         if(Ajusten>10)Ajusten=10;
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      
   }
}

