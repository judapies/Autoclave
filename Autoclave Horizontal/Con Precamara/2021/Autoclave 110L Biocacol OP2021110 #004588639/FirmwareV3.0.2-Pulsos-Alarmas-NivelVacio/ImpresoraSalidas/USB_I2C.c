// Falta configurar impresion con lectura de datos en memoria EEPROM
//#include <16F877A.h>
//#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOBROWNOUT, NOPUT// 
#include <18F4685.h>
#fuses HS,WDT4096,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,PUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR,NOBROWNOUT,BORV42
#use delay(clock=20000000)
#use RS232(BAUD=19200, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7, stream=Printer)
#use i2c(slave, slow, sda=PIN_C4, scl=PIN_C3, address=0xB0)

#define   In0       input(PIN_A0)
#define   In1       input(PIN_A1)
#define   In2       input(PIN_A2)
#define   In3       input(PIN_A3)
#define   In5       input(PIN_A5)

#define   Aux1_on   output_bit(PIN_D1,0)
#define   Aux1_off  output_bit(PIN_D1,1)
#define   Aux2_on   output_bit(PIN_D0,0)
#define   Aux2_off  output_bit(PIN_D0,1)
#define   EV1_on    output_bit(PIN_C5,0)
#define   EV1_off   output_bit(PIN_C5,1)
#define   EV2_on    output_bit(PIN_D3,0)
#define   EV2_off   output_bit(PIN_D3,1)
#define   EV3_on    output_bit(PIN_D2,0)
#define   EV3_off   output_bit(PIN_D2,1)
#define   EV4_on    output_bit(PIN_B7,1)
#define   EV4_off   output_bit(PIN_B7,0)
#define   EV5_on    output_bit(PIN_B6,1)
#define   EV5_off   output_bit(PIN_B6,0)
#define   EV6_on    output_bit(PIN_B5,1)
#define   EV6_off   output_bit(PIN_B5,0)
#define   EV7_on    output_bit(PIN_B4,1)
#define   EV7_off   output_bit(PIN_B4,0)
#define   EV8_on    output_bit(PIN_B3,1)
#define   EV8_off   output_bit(PIN_B3,0)
#define   EV9_on    output_bit(PIN_B2,1)
#define   EV9_off   output_bit(PIN_B2,0)
#define   EV10_on   output_bit(PIN_B1,1)
#define   EV10_off  output_bit(PIN_B1,0)
#define   EV11_on   output_bit(PIN_B0,1)
#define   EV11_off  output_bit(PIN_B0,0)
#define   EV12_on   output_bit(PIN_D7,1)
#define   EV12_off  output_bit(PIN_D7,0)
#define   EV13_on   output_bit(PIN_D6,1)
#define   EV13_off  output_bit(PIN_D6,0)
#define   EV14_on   output_bit(PIN_D5,1)
#define   EV14_off  output_bit(PIN_D5,0)
#define   EV15_on   output_bit(PIN_D4,1)
#define   EV15_off  output_bit(PIN_D4,0)

#define   Buzzer_on    output_bit(PIN_C2,1)
#define   Buzzer_off   output_bit(PIN_C2,0)

#define ON 1
#define OFF 0

// Variables para Pt100
int8 Termostato=0,Puerta=0,Parada=0,NivelBajo=0,NivelAlto=0,Codigo=0,posicionl=0,posicionh=0,Alarma=0,respuesta=0;
float Tpulso=0.0,error=0.0,d3=0.0,Tmax=0.0,Tmin=0.0,promedioT=0.0;
float Temperatura=0.0,TempUSB=0.0,Temporal=0.0;
int8 Year=0,Mes=0,Dia=0,Hora=0,Minuto=0,Segundo=0,dow=0,Ciclo=0,SetTime=0;// dow=Day Of Week
int8 YearRx=0,MesRx=0,DiaRx=0,HoraRx=0,MinutoRx=0,SegundoRx=0,dowRx=0,Modulo=0,Imprime=0,CiclosL=0,CiclosH=0,i=0;
short flagImprime=0,flagImprime2=0,Configura=0,Finalizo=0,estadoBuzzer=0,Desfogando=OFF,Esterilizando=OFF,Secando=OFF;
int16 PresionCamara=0,PresionPreCamara=0,CiclosTotales=0,posicion=0,Pmax=0,Pmin=0,promedioP=0,mEst=0,sEst=0,sSec=0,mSec=0,Setpoint=0,codigoUsuario=0;
int8 EstadoSuministroA=0,estadoAgua=0,estadoBombaVacio=0,estadoEmpaqueL=0,estadoEmpaqueS=0,estadoCilindroS=0,estadoCilindroL=0,estadoSuministroV=0,estadoPaso=0,estadoVacio=0,estadoDesfogue=0,estadoAire=0;
int8 EstadoAux1=0,EstadoAux2=0,codigoAlarma=0,codigoAnt=1;

int8 rxbuf1[100];
int8 rxbuf2[100];
int8 rxbuf3[100];
int8 rxbuf4[100];

byte fstate;                     //Guardara el estado del bus I2C
byte buffer[0x25], txbuf[0x25],rxbuf[50];     //Buffer de memoria
const char texto[12][20]={"CALENTAMIENTO","NO ENVUELTOS 134","NO ENVUELTOS 121","ENVUELTOS 134","ENVUELTOS 121","ENVUELTO DOBLE 1",
"ENVUELTO DOBLE 2","PRION","BOWIE & DICK","TEST DE FUGAS","TEST DE COMPONENTES","HORA Y FECHA"};

const char user[10][20]={"Admin","Operador 1","Operador 2","Operador 3","Operador 4","Operador 5","Operador 6","Operador 7","Operador 8","Tecnico"};

#INT_SSP
void ssp_interupt (){
   
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupci?n
  
  if(fstate == 0x80) {        
       //Manda al maestro la informaci?n contenida en la posici?n de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
   }else{/* Sino est? solicitando lectura es que est? enviando algo *///Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Informaci?n recibida corresponde a la posicion
         posicionl = incoming;          //Se guarda posici?n
      }else if (fState == 2) {          //Informaci?n recibida corresponde al dato
         posicionh = incoming;
         posicion=make16(posicionh,posicionl);
      }else if (fState == 3) {          //Informaci?n recibida corresponde al dato         
         if(posicion<40){
            rxbuf[posicion] = incoming;
         }else if(posicion>=40 && posicion<=136){
            rxbuf1[posicion-40] = incoming;
         }else if(posicion>136 && posicion<=235){
            rxbuf2[posicion-137] = incoming;
         }else if(posicion>235 && posicion<=334){
            rxbuf3[posicion-236] = incoming;
         }else if(posicion>334 && posicion<=360){
            rxbuf4[posicion-335] = incoming;            
         }
      }
  }
}



void Carga_Vector(void){
   txbuf[0]=Dia;
   txbuf[1]=Codigo;
   txbuf[2]=Year;
   txbuf[3]=Hora;
   txbuf[4]=Minuto;
   txbuf[5]=Segundo;
   txbuf[6]=dow;
   txbuf[7]=Dia;
   txbuf[8]=44;
   txbuf[9]=respuesta;
}

void Lee_Vector(void){
   Temporal=rxbuf[1];
   Temperatura=rxbuf[0];
   Temperatura=Temperatura+(Temporal/10);
   PresionCamara=make16(rxbuf[3],rxbuf[2]);
   PresionPreCamara=make16(rxbuf[5],rxbuf[4]);
   Setpoint=rxbuf[6];
   EstadoAux1=rxbuf[7];
   EstadoAux2=rxbuf[8];
   mEst=rxbuf[9];
   sEst=rxbuf[10];
   mSec=rxbuf[11];
   sSec=rxbuf[12];
   Ciclo=rxbuf[13];
   SetTime=rxbuf[14];
   DiaRx=rxbuf[15];
   MesRx=rxbuf[16];
   YearRx=rxbuf[17];
   HoraRx=rxbuf[18];
   MinutoRx=rxbuf[19];
   SegundoRx=rxbuf[20];
   Imprime=rxbuf[21];
   CiclosL=rxbuf[22];
   CiclosH=rxbuf[23];
   estadoBuzzer=rxbuf[24];
   estadoSuministroA=rxbuf[25];
   estadoAgua=rxbuf[26];
   estadoBombaVacio=rxbuf[27];
   estadoEmpaqueL=rxbuf[28];
   estadoEmpaqueS=rxbuf[29];
   estadoCilindroS=rxbuf[30];
   estadoCilindroL=rxbuf[31];
   estadoSuministroV=rxbuf[32];
   estadoPaso=rxbuf[33];
   estadoVacio=rxbuf[34];
   estadoDesfogue=rxbuf[35];
   estadoAire=rxbuf[36];
   codigoAlarma=rxbuf[38];
   codigoUsuario=rxbuf[39];
   CiclosTotales=make16(CiclosH,CiclosL);
}

float inttofloat(int8 decimal){
   float tmp;
   tmp=decimal;
   return tmp/10;
}

void main()
{
   set_tris_d(0);
   set_tris_b(0);
   set_tris_a(255);
   EV1_off;EV2_off;EV3_off;EV4_off;EV5_off;EV6_off;EV7_off;EV8_off;EV9_off;EV10_off;EV11_off;EV12_off;EV13_off;EV14_off;EV15_off;
   Aux1_off;Aux2_off;
   enable_interrupts(INT_SSP); 
   setup_wdt(WDT_ON);
   enable_interrupts(global);
   Buzzer_on;
//Inicio de Impresora
   putc(27);
   delay_us(10);
   putc(64);
   delay_us(10);
   putc(10);
   //32 Caracteres se pueden escribir en una linea de la impresora
 //fprintf(printer,"                                \n");
   delay_ms(500);
   Buzzer_off;
   EV4_off;
   EV1_off;EV2_off;EV3_off;EV4_off;EV5_off;EV6_off;EV7_off;EV8_off;EV9_off;EV10_off;EV11_off;EV12_off;EV13_off;EV14_off;EV15_off;
   Aux1_off;Aux2_off;
   fprintf(printer," \n");
   delay_us(10);
   fprintf(printer,"  \n");
   delay_us(10);
   delay_ms(3500);
   restart_wdt();   
   
while(true)
{
   Lee_Vector();
   delay_ms(1);
   Carga_Vector();
   restart_wdt();   
   
   if(EstadoAux1==1)
      Aux1_on;
   else
      Aux1_off;
   
   if(EstadoAux2==1)
      Aux2_on;
   else
      Aux2_off;
   
   if(estadoAgua==1)
      EV1_on;
   else
      EV1_off;
   
   if(estadoSuministroA==1)
      EV2_on;
   else
      EV2_off;
      
   if(estadoBombaVacio==1)
      EV3_on;
   else
      EV3_off;   
   
   if(estadoEmpaqueL==1){
      EV4_on;
      /*
      if(!input_state(PIN_B7)){
         EV4_on;
         delay_ms(1000);
         EV4_off;
         delay_ms(1000);
         EV4_on;
      }
      */
   }else{
      EV4_off;
   }
      
   if(estadoEmpaqueS==1){
      EV5_on;
      /*
      if(!input_state(PIN_B6)){
         EV5_on;
         delay_ms(1000);
         EV5_off;
         delay_ms(1000);
         EV5_on;
      }
      */
   }else{
      EV5_off;   
   }
   
   if(estadoCilindroS==1){
      EV6_on;
      EV7_on;
   }else{
      EV6_off;   
      EV7_off;
   }
   
   if(estadoCilindroL==1){
      EV8_on;
      EV9_on;
   }else{
      EV8_off;   
      EV9_off;
   }
   
   if(estadoSuministroV==1)
      EV10_on;
   else
      EV10_off;   
   
   if(estadoPaso==1)
      EV11_on;
   else
      EV11_off;   
      
   if(estadoVacio==1)
      EV12_on;
   else
      EV12_off;      
      
   if(estadoDesfogue==1)
      EV13_on;
   else
      EV13_off;   
      
   if(estadoAire==1){
      if(PresionCamara<100)
         EV14_on;
      else
         EV14_off;
   }else{
      EV14_off;      
   }

//----------------------------------------------- Impresora-----------------------------------------------------------------------------------------------------------
   
   if(SetTime==5 && !Configura){
      fprintf(printer,"  \n");
      delay_us(10);
      fprintf(printer,"  Ajusto Fecha y hora \n");
      delay_us(10);
      fprintf(printer,"%02u-%02u-%02u  %02u:%02u  %03.1f?C  %03LukPa \n",DiaRx,MesRx,YearRx,HoraRx,MinutoRx,Temperatura,PresionCamara); 
      delay_us(10);
      Configura=1;
   }else if(SetTime!=5){
      Configura=0;
   }
   
   if(codigoAlarma==10){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"------Precalentando ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==11){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"--------- Pre-Vacio ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==12){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"--------- Calentando ---------- \n");
         delay_us(10);
      }
   }else if(codigoAlarma==13){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         promedioT=(Tmax+Tmin)/2.0;
         promedioP=(Pmax+Pmin)/2;
         fprintf(printer,"------------------------------- \n");
         fprintf(printer,"Promedio de Temperatura:%2.1f?C\n",promedioT);
         delay_us(10);
         fprintf(printer,"------------------------------- \n");
         delay_us(10);
         fprintf(printer,"Promedio Presion:%03Lu kPa   \n",promedioP);
         delay_us(10);
         fprintf(printer,"------- Despresurizando ------- \n");
         delay_us(10);
         Esterilizando=OFF;
         Secando=OFF;
         Desfogando=ON;
      }
   }else if(codigoAlarma==15){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"----------- Secando ---------- \n");
         delay_us(10);
         Esterilizando=OFF;
         Secando=ON;
         Desfogando=OFF;
      }
   }else if(codigoAlarma==16){
      if(codigoAnt!=codigoAlarma){
         codigoAnt=codigoAlarma;
         fprintf(printer,"------- Esterilizando -------- \n");
         delay_us(10);
         Tmin=Temperatura;
         Tmax=Temperatura;
         Pmin=PresionCamara;
         Pmax=PresionCamara;
         Esterilizando=ON;
         Secando=OFF;
         Desfogando=OFF;
      }
   }
   
   if(Esterilizando){
      if(Temperatura<Tmin){
         Tmin=Temperatura;
      }
      if(Temperatura>Tmax){
         Tmax=Temperatura;
      }
      if(PresionCamara<Pmin){
         Pmin=PresionCamara;
      }
      if(PresionCamara>Pmax){
         Pmax=PresionCamara;
      }
   }
   
   if(Imprime==15 && !flagImprime2){
      if(Ciclo>0 && Ciclo<31){
         Tmin=0;
         Tmax=0;
         Pmin=0;
         Pmax=0;
         promedioT=0;
         promedioP=0;
         fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");
         delay_us(10);
         fprintf(printer," OPERADOR:______________________\n");
         delay_us(10);
         fprintf(printer,"%s\n",user[codigoUsuario]);
         delay_us(10);
         fprintf(printer,"  Ciclo de Esterilizacion #%02u \n",Ciclo);
         delay_us(10);
         if(Ciclo<11)
            fprintf(printer,"%s\n",Texto[Ciclo-1]);
         else
            fprintf(printer,"Personalizado %02u\n",Ciclo);
         delay_us(10);
         fprintf(printer,"Tiempo Esterilizacion:%02Lu:%02Lu\n",mEst,sEst);
         delay_us(10);
         fprintf(printer,"Tiempo de Secado:%02Lu:%02Lu\n",mSec,sSec);
         delay_us(10);
         fprintf(printer,"Setpoint de Temperatura:%03Lu?C\n",Setpoint);
         delay_us(10);
         fprintf(printer,"Ciclo #%05Lu\n",CiclosTotales);
         delay_us(10);
         fprintf(printer,"FECHA: %02u-%02u-%02u \n",DiaRx,MesRx,YearRx);
         delay_us(10);
         //fprintf(printer,"   HORA   TEMP    PRESION \n");
         fprintf(printer," HORA  TEMP     PC     PP   \n");
         delay_us(10);
      }
      delay_ms(1000);
      flagImprime2=1;
      flagImprime=1;
      Finalizo=0;
   }
   if(Imprime==5 && !flagImprime && !Finalizo){
      fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa \n",HoraRx,MinutoRx,Temperatura,0XDF,PresionCamara,PresionPreCamara);
      flagImprime=1;
   }else if(Imprime==20 && !Finalizo){
      fprintf(printer," Parada de Emergencia Activada \n");
      delay_us(10);
      fprintf(printer,"       Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==21 && !Finalizo){
      fprintf(printer,"   Tiempo de Llenado prolongado\n");
      delay_us(10);
      fprintf(printer,"    EA01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==22 && !Finalizo){
      fprintf(printer,"      Termostato Activado\n");
      delay_us(10);
      fprintf(printer,"    EA02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==24 && !Finalizo){
      fprintf(printer,"Tiempo de Calentamiento Excesivo\n");
      delay_us(10);
      fprintf(printer,"    ET01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==23 && !Finalizo){
      fprintf(printer," Sobrepaso en la temperatura\n");
      delay_us(10);
      fprintf(printer,"    ET02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==25 && !Finalizo){
      fprintf(printer," Sobrepaso en la presion\n");
      delay_us(10);
      fprintf(printer,"    EP01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==26 && !Finalizo){
      fprintf(printer," Error en bomba de Vacio\n");
      delay_us(10);
      fprintf(printer,"    EV01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==27 && !Finalizo){
      fprintf(printer," Tiempo de Vacio Excesivo\n");
      delay_us(10);
      fprintf(printer,"    EV02 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==28 && !Finalizo){
      fprintf(printer,"      Ciclo Finalizado \n");
      delay_us(10);
      fprintf(printer,"       Correctamente \n");
      delay_us(10);
      fprintf(printer,"  %02u:%02u  %03.1f?C  %03LukPa  \n",HoraRx,MinutoRx,Temperatura,PresionCamara); 
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==29 && !Finalizo){
      fprintf(printer,"    Error Empaque Puerta\n");
      delay_us(10);
      fprintf(printer,"    EE01 Ciclo Cancelado \n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      Finalizo=1;
   }else if(Imprime==10){
      flagImprime=0;
      flagImprime2=0;
   }
   
   if(estadoBuzzer==1)
      Buzzer_on;
   else
      Buzzer_off;
   
   if(rxbuf1[0]==0xff && rxbuf1[1]==0xff){//indica que debe iniciar la escritura en la Impresora
      delay_ms(1000);
      fprintf(printer," AUTOCLAVE DIGITAL  JP INGLOBAL  \n");
      delay_us(10);
      fprintf(printer," OPERADOR  %s\n",user[rxbuf1[2]]);
      delay_us(10);
      fprintf(printer,"  Ciclo de Esterilizacion #%02u \n",rxbuf1[3]);
      delay_us(10);
      if(Ciclo<11)
         fprintf(printer,"%s\n",Texto[rxbuf1[3]-1]);
      else
         fprintf(printer,"Personalizado %02u\n",rxbuf1[3]);
      delay_us(10);
      fprintf(printer,"Ciclo #%05Lu\n",CiclosTotales);
      delay_us(10);
      fprintf(printer,"FECHA: %02u-%02u-%02u \n",rxbuf1[4],rxbuf1[5],rxbuf1[6]);
      delay_us(10);
      fprintf(printer," HORA  TEMP     PC     PP   Al\n");
      delay_us(10);
      
      /*
      fprintf(printer," OPERADOR:%02u\n",rxbuf1[2]);
      delay_us(10);
      fprintf(printer," CICLO:%02u\n",rxbuf1[3]);
      delay_us(10);            
      fprintf(printer," FECHA:%02u/%02u/20%02u\n",rxbuf1[4],rxbuf1[5],rxbuf1[6]);
      delay_us(10);  
      fprintf(printer,"Hora   PC   PPC  Temp  Alar\n",);
      delay_us(10); 
      */
      for(i=0;i<10;i++){
         Hora=rxbuf1[13+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }
         Minuto=rxbuf1[14+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf1[9+(i*9)],rxbuf1[10+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf1[11+(i*9)],rxbuf1[12+(i*9)]);     
         if(PresionPreCamara>400){
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf1[7+(i*9)]+inttofloat(rxbuf1[8+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }   
         Alarma=rxbuf1[15+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      } 
      for(i=0;i<11;i++){
         Hora=rxbuf2[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf2[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf2[2+(i*9)],rxbuf2[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf2[4+(i*9)],rxbuf2[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf2[(i*9)]+inttofloat(rxbuf2[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf2[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      } 
      for(i=0;i<11;i++){
         Hora=rxbuf3[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf3[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf3[2+(i*9)],rxbuf3[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf3[4+(i*9)],rxbuf3[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf3[(i*9)]+inttofloat(rxbuf3[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf3[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      }
      for(i=0;i<3;i++){
         Hora=rxbuf4[6+(i*9)];
         if(Hora>25){
            Hora=0;
            break;
         }              
         Minuto=rxbuf4[7+(i*9)];
         if(Minuto>59){
            Minuto=0;
            break;
         }
         PresionCamara=make16(rxbuf4[2+(i*9)],rxbuf4[3+(i*9)]);
         if(PresionCamara>400){
            PresionCamara=0;
            break;
         }
         PresionPreCamara=make16(rxbuf4[4+(i*9)],rxbuf4[5+(i*9)]);    
         if(PresionPreCamara>400){                     
            PresionPreCamara=0;
            break;
         }
         Temperatura=rxbuf4[(i*9)]+inttofloat(rxbuf4[1+(i*9)]);
         if(Temperatura>150){
            Temperatura=0;
            break;
         }
         Alarma=rxbuf4[8+(i*9)];
         if(Alarma>20){
            Alarma=0;
            break;
         }
         fprintf(printer,"%02u:%02u %03.1f%cC %03LukPa %03LukPa %02u\n",Hora,Minuto,Temperatura,0XDF,PresionCamara,PresionPreCamara,Alarma);
         delay_us(10);
      }         
      fprintf(printer,"Impresion Finalizada\n");
      delay_us(10);
      fprintf(printer," \n");
      delay_us(10);
      fprintf(printer,"  \n");
      delay_us(10);
      respuesta=5;
      rxbuf1[0]=0;
      rxbuf1[1]=0;
    }else{
      respuesta=10;
    }
   }
}
