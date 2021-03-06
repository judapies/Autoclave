//Falta crear menu de ajuste de variables y de norma CFR21

#include <18F4685.h>
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOMCLR
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"
#include "imagen2.h"

#define   UP            !input(PIN_A4)
#define   DOWN          !input(PIN_A5)
#define   RIGHT         !input(PIN_E0)
#define   LEFT          !input(PIN_E1)

#define   DOOR          !input(PIN_E2)
#define   STOP          !input(PIN_B6)
#define   TERM          !input(PIN_B7)

#define   HIGH          input(PIN_C0)
#define   LOW           input(PIN_C1)

#define   Display_on       output_bit(PIN_C7,1)
#define   Display_off      output_bit(PIN_C7,0)

#define   SSR_on           output_bit(PIN_C2,0)
#define   SSR_off          output_bit(PIN_C2,1)

#define   Desfogue_on      output_bit(PIN_C5,0)
#define   Desfogue_off     output_bit(PIN_C5,1)

#define   Paso_on          output_bit(PIN_C6,0)
#define   Paso_off         output_bit(PIN_C6,1)

//Comunicacion//------------------------
byte txbuf[0X20];
byte SlaveA0Tx[0X20];
byte dato=0;            //Contendr? la informaci?n a enviar o recibir
byte direccion=0;       //Contendr? la direcci?n del esclavo en el bus I2C
byte posicion=0;        //Contendr? la posici?n en el buffer de memoria del esclavo donde se almacenar? o de donde se recuperar? el dato
byte ByteConfig1=0,ByteConfig2=0,ByteConfig3=0;

//Alarmas//---------------------
short AlarmaTiempoProlongado=OFF,AlarmaSobreTemperatura=OFF,AlarmaLlenado=OFF,AlarmaTermostato=OFF,AlarmaPuerta=OFF,AlarmaEmergencia=OFF,AlarmaVacio=OFF;
short AlarmaBomba=OFF;
int16 TiempoVacio=600;

//Banderas//--------------------
short IniciaLlenado=OFF,CalderinLleno=OFF,IniciaCiclo=OFF,Esterilizando=OFF,Secando=OFF,SensadoNivel=OFF,Desfogando=OFF,DesfogueSuave=OFF;
short EstadoDesfogue=OFF,EstadoAgua=OFF,EstadoSSR=OFF,EstadoSSR2=OFF,EstadoSSR3=OFF,EstadoAire=OFF,PruebaEstados=OFF,disp_Guarda=OFF,IniciaVacio2=OFF;
short EstadoVacio=OFF,EstadoPaso=OFF,EstadoBuzzer=OFF,Vacio=OFF,flag_vacio=OFF,IniciaVacio=OFF,flag_vac=OFF,EstadoAireBomba=OFF,Ciclo=OFF;
short EnciendeBuzzer=OFF,BorraCuadro=OFF,PosVacio=OFF,PosCalentando=OFF,PosEsterilizando=OFF,PosDesfogando=OFF,PosSecando=OFF,Precalentamiento=OFF;

//Hardware//--------------------
short EVVacio=OFF,EVAire=OFF,EVAgua=OFF,SSR2=OFF,SSR3=OFF,Buzzer=OFF,EVAireBomba=OFF;

//Tiempos//---------------------
int16 TiempoCiclo=0,TiempoLlenado=0,TiempoSensadoNivel=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecado[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Tsecadop[4]={0,0,0,0};   // Tiempo transcurrido

//Control//------------------------
float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresion[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float PromPresionp[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float Gain=40.2,RPT100=0.5,Temporal=0.0;
float X[2]={0.0,0.0};
float promediopresion=0.0,Aumento=0.0,promediopresionp=0.0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=20;
float V0=0.0,R1=6110.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0;
float V1=0.0,Presion=0.0,V2=0.0,Presionp=0.0;
float Temperatura=0.0,K=0.006429;
float a=0.0011,b=2.3302,c=-244.0723;
int8 Integral=0,Derivativo=0;

//Varios//--------------------------------
char kpa[]="kPa";
short Flanco=0,Flanco1=0,Flanco2=0,visual=1,desfoguelento=0,Lectura=0;
short disp_Stop=0;
int8 Control=0,PulsosBuzzer=0,CantidadPulsos=4,NivelVacio=25,CiclosL=0,CiclosH=0;
int8 Menu=0, unidad=1,i=0,y=0;
int8 Setpoint=0,Atmosferica=71,Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,Nivel=0,tinicio=0,PresionCamara=0,PresionPreCamara=0,Ciclos=0,NivelSeleccionado=0;
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
int8 r=0,t=0,r2=0,t2=0,TiempoControl=0,PulsoPositivo=10,PulsosVacio=0,flag_pulso=0;
int16 t_exhaust=0,tvacio=0,tbomba=0;
   
#include "Funciones.h"   

//Funciones//----------------------------
void MuestraTiemposProgramados(){
// Muestra Valor de esterilizacion programado.
   displayTiempo(Testerilizacionp[3],36,0,0,1);displayTiempo(Testerilizacionp[2],42,0,0,1);
   strcpy(Menus.Letra,":");displayMenu(Menus.Letra,49,0,2,1); 
   displayTiempo(Testerilizacionp[1],54,0,0,1);displayTiempo(Testerilizacionp[0],60,0,0,1);
// Muestra tiempo de secado programado.
   displayTiempo(Tsecadop[3],36,8,0,1);displayTiempo(Tsecadop[2],42,8,0,1);
   strcpy(Menus.Letra,":");displayMenu(Menus.Letra,49,8,2,1); 
   displayTiempo(Tsecadop[1],54,8,0,1);displayTiempo(Tsecadop[0],60,8,0,1);
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
}#separate 
float LeerPT100(int media)
{
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/Gain; // Se elimina la ganancia
   Pt=(R1)/((1024.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-RPT100; //Ajuste de impedancia de cableado de PT100
   //Temperature=(Pt-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
            
   if(l>(media-1))
   {l=0;ActualizaRecta();}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   //Temperature=(Prom-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

float Leer_Sensor_Presion_Camara(int media){
   promediopresion=0.0;
   V1 = sensores(3);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
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

void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   error=desvio+Setpoint-Temperatura;      // C?lculo del error ******
   
   if(error>0.9 && error<15 && Aumento<0.3)
   {
      if(Tpulso<9.0)
      {
         if(TiempoControl>=Derivativo)
         {
            TiempoControl=0;
            Ganancia+=Integral;
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
      SSR_on;
   }
   else         
   {
      SSR_off;
   }
         
   if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
   {
      tiempoctrl=0;
   } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
//--------------------------------------------------------------------------------------------------------------------//   
}

void Envio_I2C(direccion, posicion, dato){

   i2c_start();            // Comienzo comunicaci?n
   i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
   i2c_write(posicion);    // Posici?n donde se guardara el dato transmitido
   i2c_write(dato);        // Dato a transmitir
   i2c_stop();             // Fin comunicaci?n
 }

void Lectura_I2C (byte direccion, byte posicion, byte &dato) {

   i2c_start();            // Comienzo de la comunicaci?n
   i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
   i2c_write(posicion);    // Posici?n de donde se leer? el dato en el esclavo
   i2c_start();            // Reinicio
   i2c_write(direccion+1); // Direcci?n del esclavo en modo lectura
   dato=i2c_read(0);       // Lectura del dato
   i2c_stop();             // Fin comunicaci?n
}

void Carga_Vector(void){
   SlaveA0Tx[0]=(int8)Temperatura;
   SlaveA0Tx[1]=(Temperatura-((int8)Temperatura))*10;
   SlaveA0Tx[2]=make8((int16)PresionCamara,0);
   SlaveA0Tx[3]=make8((int16)PresionCamara,1);
   SlaveA0Tx[4]=make8((int16)PresionPreCamara,0);
   SlaveA0Tx[5]=make8((int16)PresionPreCamara,1);
   SlaveA0Tx[6]=(int8)Tpulso*10;
   SlaveA0Tx[7]=HIGH;
   SlaveA0Tx[8]=LOW;
   SlaveA0Tx[9]=TERM;
   SlaveA0Tx[10]=DOOR;
   SlaveA0Tx[11]=STOP;
   SlaveA0Tx[12]=(int8)error;
   SlaveA0Tx[13]=(int8)EVAgua;
   SlaveA0Tx[14]=(int8)EVAire;
   SlaveA0Tx[15]=(int8)EVVacio;
   SlaveA0Tx[16]=(int8)SSR2;
   SlaveA0Tx[17]=(int8)SSR3;
   SlaveA0Tx[18]=(int8)Buzzer;
   SlaveA0Tx[19]=(int8)EVAireBomba;
}

void Lee_Vector(void){
   ByteConfig1=txbuf[0];
   ByteConfig2=txbuf[1];
   ByteConfig3=txbuf[2];
   
   if(ByteConfig1==10 && ByteConfig2==20 && ByteConfig3==30){
      Ganancia=txbuf[3];
      Ajuste=txbuf[4];
      Ajusten=txbuf[5];
      Desvio=txbuf[6];
      MediaMovil=txbuf[7];
      Tciclo=txbuf[8];
      Integral=txbuf[9];
      Derivativo=txbuf[10];
      Gain=txbuf[11]+(txbuf[12]/10);
      R1=make16(txbuf[14],txbuf[13]);
      RPT100=txbuf[15]+(txbuf[16]/10);
   }
   
   if(txbuf[17]==50){
      if(!disp_Guarda){
         glcd_fillScreen(OFF);disp_Guarda=ON;
         strcpy(Menus.Letra,"Configuracion");displayMenu(Menus.Letra,0,10,0,2);
         strcpy(Menus.Letra,"   Guardada  ");displayMenu(Menus.Letra,0,30,0,2);
      }
      write_eeprom(20,Ganancia);
      delay_ms(10);
      write_eeprom(21,(int8)Ajuste);
      delay_ms(10);
      write_eeprom(22,(int8)Ajusten);
      delay_ms(10);
      //Desvio=read_eeprom(23);
      write_eeprom(24,(int8)MediaMovil);
      delay_ms(10);
      write_eeprom(25,(int8)Tciclo);
      delay_ms(10);
      write_eeprom(26,(int8)Integral);
      delay_ms(10);
      write_eeprom(27,(int8)Derivativo);
      delay_ms(10);
      write_eeprom(28,(int8)txbuf[11]);
      delay_ms(10);
      write_eeprom(29,(int8)txbuf[12]);
      delay_ms(10);
      write_eeprom(30,(int8)txbuf[13]);
      delay_ms(10);
      write_eeprom(31,(int8)txbuf[14]);
      delay_ms(10);
      write_eeprom(32,(int8)txbuf[15]);
      delay_ms(10);
      write_eeprom(33,(int8)txbuf[16]);
      delay_ms(10);
   }else{
      disp_Guarda=OFF;
   }
}

void Envio_Esclavos(void){
   Carga_Vector();
   for(i=0;i<20;i++)
   {
      direccion=0xA0;
      Envio_I2C(direccion,i,SlaveA0Tx[i]);
   } 
}
void Lectura_Esclavos(void){
   for(i=0;i<18;i++)
   {
      direccion=0xA0;                        //Direcci?n en el bus I2c
      posicion=i;                         //Posici?n de memoria a leer
      Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
      txbuf[i]=(int8)dato;
   }
   Lee_Vector();
}

#int_TIMER0
void temp0s(void){
   set_timer0(0);
   t_exhaust++;
   if(DesfogueSuave)
   {
      if(Nivel<6)
      {
         if(t_exhaust<=(NivelSeleccionado)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
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
   tiempos3++;
   tiempoctrl++;
   
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {      
      TiempoControl++;
      if(IniciaCiclo && !Esterilizando){
         TiempoCiclo++;
      }
      
      if(IniciaLlenado && !CalderinLleno){
         TiempoLlenado++;
      }
      
      if(SensadoNivel){
         TiempoSensadoNivel++;
      }
      
      if(IniciaVacio){
         tvacio++;         
      }
      
      if(IniciaVacio2){         
         tbomba++;
      }
      
      if(EnciendeBuzzer){
         if(PulsosBuzzer>10){
            Buzzer=OFF;
         }else{
             Buzzer=!Buzzer;
             PulsosBuzzer++;
         }
      }
      
      tiempos=0;Lectura=1;tinicio++;
   
      if(Esterilizando && !Desfogando)
      {               
         Testerilizacion[0]++;
         if(Testerilizacion[0]>9)
            {Testerilizacion[0]=0;
            Testerilizacion[1]++;}
         if(Testerilizacion[1]>5)
            {Testerilizacion[1]=0;
             Testerilizacion[2]++;}
            if(Testerilizacion[2]>9)
               {Testerilizacion[2]=0;
                Testerilizacion[3]++;}
               if(Testerilizacion[3]>5)
                  {Testerilizacion[3]=0;}
      }       
      
      if(Secando)
      {               
         Tsecado[0]++;
         if(Tsecado[0]>9)
            {Tsecado[0]=0;
             Tsecado[1]++;}
          if(Tsecado[1]>5)
            {Tsecado[1]=0;
             Tsecado[2]++;}
            if(Tsecado[2]>9)
               {Tsecado[2]=0;
                Tsecado[3]++;}
               if(Tsecado[3]>5)
                  {Tsecado[3]=0;}
         }  
   }
}

void ControlaPresion(){
   if(PresionPrecamara>=150)
   {
      SSR_off;
      Precalentamiento=ON;
   }
   
   if(PresionPrecamara<130)
   {
      SSR_on;
   }

}

void CorrePrograma(){
while(true)
   {
      Envio_Esclavos();
      delay_ms(1);
      Lectura_Esclavos();

// Tiempo de Muestreo//--------------------------------------
      if(tiempos3>=20){
         Temperatura=LeerPt100(MediaMovil);
         PresionCamara=Leer_Sensor_Presion_Camara(5);
         PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
         tiempos3=0;
      }

// Monitoreo Nivel de Agua//--------------------------------------
   if(!AlarmaLlenado && !PruebaEstados && STOP){
      if(!LOW && !Esterilizando){//Si no sensa agua en el nivel bajo
         EVAgua=ON;
         IniciaLlenado=ON;
      }else{
         if(!HIGH && !CalderinLleno){//Si no sensa agua en el nivel alto y no se ha llenado el calderin
            EVAgua=ON;
            IniciaLlenado=ON;
            TiempoSensadoNivel=0;
         } else{
            if(TiempoSensadoNivel>2)
            {
               SensadoNivel=OFF;
               CalderinLleno=ON;
               IniciaLlenado=OFF;
               EVAgua=OFF;
            }
            else
            {
               SensadoNivel=ON;
            }
         }
      }
   }
      
// Monitoreo Parada Emergencia//--------------------------------------      
      if(!STOP && !IniciaCiclo && !PruebaEstados){
         if(!disp_Stop)
         {
            glcd_fillScreen(OFF);disp_Stop=1;Menu=255;
         }
         Desfogue_on;
         EVAgua=OFF;
         SSR_off;
         strcpy(Menus.Letra,"  !Parada!");displayMenu(Menus.Letra,0,10,0,2);
         strcpy(Menus.Letra," !Activada!");displayMenu(Menus.Letra,0,30,0,2);
      }else{
         if(disp_Stop){
            reset_cpu();
         }
      }
      
      glcd_update();
      
      if(UP || DOWN || RIGHT || LEFT)
      {
         tinicio=0;
      }

      while(tinicio>=14400)
      {
         glcd_fillscreen(OFF);
         Display_off;         
         glcd_update();
      }
      
      
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==0){ //Menu de Contrase?a.
      
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9)   
                  {
                     Contrasena[i-1]=0;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }
               else
               {
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                   displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }      
            }     
      }
         else
            {Flanco2 = 0;}  
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(30);unidad++;
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }
            }
      }
         else
            {Flanco1 = 0;}  
    
      if(unidad>4)
      {glcd_fillScreen(OFF);unidad=4;
         if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2) // Si Ingresa clave para reset general del sistema.
            {write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contrase?a de Fabrica y reinicia Programa.
             write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
             reset_cpu();}        
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicaci?n del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=0;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            displayContrasena(Contrasena[0],20,30,1);
            displayContrasena(Contrasena[1],40,30,0);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==1){ //Menu Principal.      
      
      if(visual==1)
      {
         if(Opcion>10)
         {Opcion=1;}
         if(Opcion<1)
         {Opcion=10;}
         
         strcpy(Menus.Letra,"Menu Principal");
         displayMenu(Menus.Letra,30,0,1,1);
      
         if(Opcion<=6)
         {
            if(Opcion==1)
               {strcpy(Menus.Letra,"Ins. Del. no Envuelto");
               displayMenu(Menus.Letra,0,10,1,1);}
            else
               {strcpy(Menus.Letra,"Ins. Del. no Envuelto");
               displayMenu(Menus.Letra,0,10,0,1);}
            
            if(Opcion==2)   
               {strcpy(Menus.Letra,"Ins. Del. Envuelto");
               displayMenu(Menus.Letra,0,19,1,1);}
            else
               {strcpy(Menus.Letra,"Ins. Del. Envuelto");
               displayMenu(Menus.Letra,0,19,0,1);}   
               
             if(Opcion==3)   
               {strcpy(Menus.Letra,"Ins. no Envuelto");
               displayMenu(Menus.Letra,0,28,1,1);}
            else
               {strcpy(Menus.Letra,"Ins. no Envuelto");
               displayMenu(Menus.Letra,0,28,0,1);}
               
            if(Opcion==4)   
               {strcpy(Menus.Letra,"Ins. Envuelto");
               displayMenu(Menus.Letra,0,37,1,1);}
            else
               {strcpy(Menus.Letra,"Ins. Envuelto");
               displayMenu(Menus.Letra,0,37,0,1);}   
               
            if(Opcion==5)   
               {strcpy(Menus.Letra,"Textil e Instrumental");
               displayMenu(Menus.Letra,0,46,1,1);}
            else
               {strcpy(Menus.Letra,"Textil e Instrumental");
               displayMenu(Menus.Letra,0,46,0,1);}  
               
            if(Opcion==6)   
               {strcpy(Menus.Letra,"Test Bowie-Dick");
               displayMenu(Menus.Letra,0,55,1,1);}
            else
               {strcpy(Menus.Letra,"Test Bowie-Dick");
               displayMenu(Menus.Letra,0,55,0,1);}     
               visual=0;
         }
         else
         {
            if(Opcion==7)
               {strcpy(Menus.Letra,"Test de Fugas");
               displayMenu(Menus.Letra,0,10,1,1);}
            else
               {strcpy(Menus.Letra,"Test de Fugas");
               displayMenu(Menus.Letra,0,10,0,1);}
               
            if(Opcion==8)   
               {strcpy(Menus.Letra,"Personalizado");
               displayMenu(Menus.Letra,0,19,1,1);}
            else
               {strcpy(Menus.Letra,"Personalizado");
               displayMenu(Menus.Letra,0,19,0,1);}   
               
             if(Opcion==9)   
               {strcpy(Menus.Letra,"Cambio de Clave");
               displayMenu(Menus.Letra,0,28,1,1);}
            else
               {strcpy(Menus.Letra,"Cambio de Clave");
               displayMenu(Menus.Letra,0,28,0,1);}
               
            if(Opcion==10)   
               {strcpy(Menus.Letra,"Test Componentes");
               displayMenu(Menus.Letra,0,37,1,1);}
            else
               {strcpy(Menus.Letra,"Test Componentes");
               displayMenu(Menus.Letra,0,37,0,1);} 
               
               strcpy(Menus.Letra,"           ");
               displayMenu(Menus.Letra,0,46,0,1);
               
               strcpy(Menus.Letra,"           ");
               displayMenu(Menus.Letra,0,55,0,1);
         }
      }
      
      if(UP){
         if(Flanco == 0){
            Flanco = 1;delay_ms(10);Opcion--;visual=1;
         }
      }else{
         Flanco = 0;
      }
                  
      if(DOWN){
         if(Flanco2 == 0){
            Flanco2 = 1;delay_ms(10);Opcion++;visual=1;
         }     
      }else{
         Flanco2 = 0;
      }
               
       if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(30);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==4){ // Menu de Instrumental Delicado no envuelto
      
      if(visual==1)
      {
         glcd_imagen(2); //Guante
         strcpy(Menus.Letra,"Ins. Del. NE");
         displayMenu(Menus.Letra,20,0,1,2);
        
         strcpy(Menus.Letra,"Ins. Del. NE");
         displayMenu(Menus.Letra,7,55,0,1);
         
         strcpy(Menus.Letra,"Temperatura:121 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:20m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 15m");
         displayMenu(Menus.Letra,0,46,0,1);
         visual=0;
      }
      
      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=2;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=1;
         Setpoint=121;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==5){ // Menu de Instrumental delicado Envuelto
      
      if(visual==1)
      {
         glcd_imagen(4); //ErlenMeyer con Liquido
         strcpy(Menus.Letra,"Ins. Del. E");
         displayMenu(Menus.Letra,20,0,1,2);
      
         strcpy(Menus.Letra,"Ins. Del. E");
         displayMenu(Menus.Letra,7,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:121 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:20m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 20m");
         displayMenu(Menus.Letra,0,46,0,1);    
         visual=0;
      }   
         
      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }               

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=2;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=2;
         Setpoint=121;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==6){ // Menu de Instrumental no Envuelto
      
      if(visual==1)
      {
         glcd_imagen(3); //Erlen Meyer
         
         strcpy(Menus.Letra,"Ins. NE");
         displayMenu(Menus.Letra,0,0,1,2);
         
         strcpy(Menus.Letra,"Ins. NE");
         displayMenu(Menus.Letra,7,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:134 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:4m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 1m");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }            

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=4;Testerilizacionp[3]=0;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=1;Tsecadop[3]=0;
         Setpoint=134;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==7){ // Menu de Instrumental Envuelto
      
      if(visual==1)
      {
         glcd_imagen(1); //Tijeras y Camisa
         strcpy(Menus.Letra,"Ins. Env");
         displayMenu(Menus.Letra,0,0,1,2);
      
         strcpy(Menus.Letra,"Ins. Env");
         displayMenu(Menus.Letra,0,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:134 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:4m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 20m");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }                 

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=4;Testerilizacionp[3]=0;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=2;
         Setpoint=134;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==8){ // Menu de Textil Instrumental
      
      if(visual==1)
      {
         glcd_imagen(1); //Tijeras y Camisa
         strcpy(Menus.Letra,"Tex & Ins");
         displayMenu(Menus.Letra,0,0,1,2);
      
         strcpy(Menus.Letra,"Tex & Ins");
         displayMenu(Menus.Letra,0,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:134 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:5m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 15m");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }                 

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=5;Testerilizacionp[3]=0;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=5;Tsecadop[3]=1;
         Setpoint=134;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==9){ // Menu de Bowie Dick
      
      if(visual==1)
      {
         glcd_imagen(1); //Tijeras y Camisa
         strcpy(Menus.Letra,"Test B&D");
         displayMenu(Menus.Letra,0,0,1,2);
      
         strcpy(Menus.Letra,"Test B&D");
         displayMenu(Menus.Letra,0,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura:134 C");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:3.5m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= 2m");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }                 

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=3;Testerilizacionp[2]=3;Testerilizacionp[3]=0;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=2;Tsecadop[3]=0;
         Setpoint=134;
         Menu=20;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

 //_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==10){ // Menu de Test de Fugas
      
      if(visual==1)
      {
         glcd_imagen(1); //Tijeras y Camisa
         strcpy(Menus.Letra,"Test Fugas");
         displayMenu(Menus.Letra,0,0,1,2);
      
         strcpy(Menus.Letra,"Test Fugas");
         displayMenu(Menus.Letra,0,55,2,1);
         
         strcpy(Menus.Letra,"Temperatura: N/A");
         displayMenu(Menus.Letra,0,26,0,1);
         strcpy(Menus.Letra,"T. Esterilizacion:10m");
         displayMenu(Menus.Letra,0,36,0,1);
         strcpy(Menus.Letra,"T. Secado= N/A");
         displayMenu(Menus.Letra,0,46,0,1);           
         visual=0;
      }

      if(LEFT)// Si oprime Izquier|da
      { 
         delay_ms(30);Menu=1;glcd_fillScreen(OFF);visual=1;
      }                 

      if(RIGHT)// Si oprime derecha
      {
         Testerilizacionp[0]=0;Testerilizacionp[1]=0;Testerilizacionp[2]=0;Testerilizacionp[3]=1;
         visual=1;
         Tsecadop[0]=0;Tsecadop[1]=0;Tsecadop[2]=0;Tsecadop[3]=0;
         Setpoint=30;
         Menu=30;
         tiempo_secado=0;
         glcd_fillScreen(OFF);
         desfoguelento=0;
         IniciaCiclo=ON;
         enable_interrupts(global);
      }
    
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==11){ // Menu de Proceso Personalizado
        
         if(visual==1)
         {
            if(Opcion>5)
            {Opcion=1;}
            if(Opcion<1)
            {Opcion=5;}
            
            strcpy(Menus.Letra,"Proceso Personalizado");
            displayMenu(Menus.Letra,0,0,1,1);
      
            if(Opcion==1)   
               {strcpy(Menus.Letra,"T. Esterilizacion");
               displayMenu(Menus.Letra,0,10,1,1);}
            else
               {strcpy(Menus.Letra,"T. Esterilizacion");
               displayMenu(Menus.Letra,0,10,0,1);}
            
            if(Opcion==2)   
               {strcpy(Menus.Letra,"T. Secado");
               displayMenu(Menus.Letra,0,20,1,1);}
            else
               {strcpy(Menus.Letra,"T. Secado");
               displayMenu(Menus.Letra,0,20,0,1);}   
               
             if(Opcion==3)   
               {strcpy(Menus.Letra,"Temperatura");
               displayMenu(Menus.Letra,0,30,1,1);}
            else
               {strcpy(Menus.Letra,"Temperatura");
               displayMenu(Menus.Letra,0,30,0,1);}
               
            if(Opcion==4)   
               {strcpy(Menus.Letra,"Nivel de desfogue");
               displayMenu(Menus.Letra,0,40,1,1);}
            else
               {strcpy(Menus.Letra,"Nivel de desfogue");
               displayMenu(Menus.Letra,0,40,0,1);}   
               
            if(Opcion==5)   
               {strcpy(Menus.Letra,"Continuar");
               displayMenu(Menus.Letra,0,50,1,1);}
            else
               {strcpy(Menus.Letra,"Continuar");
               displayMenu(Menus.Letra,0,50,0,1);}  
                         
            visual=0;
         }
         if(UP)//Si oprime hacia arriba
         {  if(Flanco == 0) 
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {  if(Flanco2 == 0) 
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
         if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(300);Menu=Opcion+40;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
               
         if(LEFT)//Si oprime boton de Toma.
         {delay_ms(700);Menu=1;glcd_fillscreen(OFF);visual=1;}
         
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==12){ // Menu de Cambio de Contrase?a
        
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Contrasena[0],20,30,1);
         displayContrasena(Contrasena[1],40,30,0);
         displayContrasena(Contrasena[2],60,30,0);
         displayContrasena(Contrasena[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Contrasena[0],20,30,0);
            displayContrasena(Contrasena[1],40,30,1);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Contrasena[0],20,30,0);
               displayContrasena(Contrasena[1],40,30,0);
               displayContrasena(Contrasena[2],60,30,1);
               displayContrasena(Contrasena[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Contrasena[0],20,30,0);
                  displayContrasena(Contrasena[1],40,30,0);
                  displayContrasena(Contrasena[2],60,30,0);
                  displayContrasena(Contrasena[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9)   
                  {
                     Contrasena[i-1]=0;
                  }
               }
            }
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                  }
               }      
            }     
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {glcd_fillScreen(OFF);
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=91;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicaci?n del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
         }
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==91){ // Menu de Cambio de Contrase?a
        
      strcpy(Menus.Letra,"Clave Nueva");
      displayMenu(Menus.Letra,0,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Password[0],20,30,1);
         displayContrasena(Password[1],40,30,0);
         displayContrasena(Password[2],60,30,0);
         displayContrasena(Password[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Password[0],20,30,0);
            displayContrasena(Password[1],40,30,1);
            displayContrasena(Password[2],60,30,0);
            displayContrasena(Password[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Password[0],20,30,0);
               displayContrasena(Password[1],40,30,0);
               displayContrasena(Password[2],60,30,1);
               displayContrasena(Password[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Password[0],20,30,0);
                  displayContrasena(Password[1],40,30,0);
                  displayContrasena(Password[2],60,30,0);
                  displayContrasena(Password[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
            for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Password[i-1]++;
                  if(Password[i-1]>9)   
                  {
                     Password[i-1]=0;
                  }
               }
            }
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {
                     Password[i-1]--;
                     if(Password[i-1]<0)
                     {
                        Password[i-1]=9;
                     }
                  }
               }      
            }     
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {
         Opcion=1;unidad=1;
         write_eeprom(10,Password[0]);write_eeprom(11,Password[1]);write_eeprom(12,Password[2]);write_eeprom(13,Password[3]);
         delay_ms(1000);glcd_fillScreen(OFF);Menu=1;
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==13){ // Menu de Prueba de Componentes
      
      PruebaEstados=ON;
         if(Opcion>15)
         {Opcion=1;}
         if(Opcion<1)
         {Opcion=15;}
         
      if(Opcion<7){
         if(Opcion==1)   
         {
            strcpy(Menus.Letra,"EV Desfogue");
            if(EstadoDesfogue){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
               Desfogue_on;
            }
            else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
               Desfogue_off;
            }
         }
         else
         {
            strcpy(Menus.Letra,"EV Desfogue");
            if(EstadoDesfogue){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
            }
            else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
            }
         }
         
         if(Opcion==2)   
         {
            strcpy(Menus.Letra,"EV Agua");
            if(EstadoAgua){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
               EVAgua=ON;
            }
            else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
               EVAgua=OFF;
            }
         }
         else
         {
            strcpy(Menus.Letra,"EV Agua");
            if(EstadoAgua){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
            }
         }
            
         if(Opcion==3)   
         {
            strcpy(Menus.Letra,"SSR");
            if(EstadoSSR){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
               SSR_on;
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
               SSR_off;
            }
         }
         else
         {
            strcpy(Menus.Letra,"SSR");
            if(EstadoSSR){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
            }
         }
         
         if(Opcion==4)   
         {
            strcpy(Menus.Letra,"Nivel Alto");
            if(HIGH){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,36,1,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,36,1,1);
            }
         }
         else
         {
            strcpy(Menus.Letra,"Nivel Alto");
            if(HIGH){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,36,0,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,36,0,1);
            }
         }
               
         if(Opcion==5)   
         {
            strcpy(Menus.Letra,"Nivel Bajo");
            if(LOW){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,45,1,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,45,1,1);
            }
         }
         else
         {
            strcpy(Menus.Letra,"Nivel Bajo");
            if(LOW){
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,45,0,1);
            }else{
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,45,0,1);
            }
         }
         
         if(Opcion==6)   
         {
            strcpy(Menus.Letra,"Switch Puerta");
            if(DOOR){
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,54,1,1);
            }else{
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,54,1,1);
            }
         }
         else
         {
            strcpy(Menus.Letra,"Switch Puerta");
            if(DOOR){
               strcpy(Menus.Estado,"OFF");
               displayTemps(Menus.Letra,Menus.Estado,0,54,0,1);
            }else{
               strcpy(Menus.Estado,"ON");
               displayTemps(Menus.Letra,Menus.Estado,0,54,0,1);
            }
         }
      }else{
      
         if(Opcion>=7 && Opcion<13){
            if(Opcion==7)   
            {
               strcpy(Menus.Letra,"Termostato");
               if(TERM){
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
               }else{
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
               }
            }
            else
            {
               strcpy(Menus.Letra,"Termostato");
               if(TERM){
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
               }else{
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
               }
            }
            
            if(Opcion==8)   
            {
               strcpy(Menus.Letra,"Parada Emer.");
               if(STOP){
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
               }else{
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
               }
            }
            else
            {
               strcpy(Menus.Letra,"Parada Emer.");
               if(STOP){
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
               }else{
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
               }
            }
            
            if(Opcion==9)   
            {
               strcpy(Menus.Letra,"EV Paso");
               if(EstadoPaso){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
                  Paso_on;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
                  Paso_off;
               }
            }
            else
            {
               strcpy(Menus.Letra,"EV Paso");
               if(EstadoPaso){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
               }
            }
            
            if(Opcion==10)   
            {
               strcpy(Menus.Letra,"EV Vacio");
               if(EstadoVacio){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,36,1,1);
                  EVVacio=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,36,1,1);
                  EVVacio=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"EV Vacio");
               if(EstadoVacio){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,36,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,36,0,1);
               }
            }
            
            if(Opcion==11)   
            {
               strcpy(Menus.Letra,"EV Aire");
               if(EstadoAire){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,45,1,1);
                  EVAire=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,45,1,1);
                  EVAire=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"EV Aire");
               if(EstadoAire){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,45,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,45,0,1);
               }
            }
            
            if(Opcion==12)   
            {
               strcpy(Menus.Letra,"SSR 2");
               if(EstadoSSR2){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,54,1,1);
                  SSR2=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,54,1,1);
                  SSR2=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"SSR 2");
               if(EstadoSSR2){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,54,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,54,0,1);
               }
            }
         }else{
            if(Opcion==13)   
            {
               strcpy(Menus.Letra,"SSR 3");
               if(EstadoSSR3){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
                  SSR3=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,1,1);
                  SSR3=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"SSR 3");
               if(EstadoSSR3){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,9,0,1);
               }
            }
         
            if(Opcion==14)   
            {
               strcpy(Menus.Letra,"Buzzer");
               if(EstadoBuzzer){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
                  Buzzer=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,1,1);
                  Buzzer=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"Buzzer");
               if(EstadoBuzzer){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,18,0,1);
               }
            }
            
            if(Opcion==15)   
            {
               strcpy(Menus.Letra,"EV Aire Bomba");
               if(EstadoAireBomba){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
                  EVAireBomba=ON;
               }
               else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,1,1);
                  EVAireBomba=OFF;
               }
            }
            else
            {
               strcpy(Menus.Letra,"EV Aire Bomba");
               if(EstadoAireBomba){
                  strcpy(Menus.Estado,"ON");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
               }else{
                  strcpy(Menus.Estado,"OFF");
                  displayTemps(Menus.Letra,Menus.Estado,0,27,0,1);
               }
            }
         }
      }
         
         if(RIGHT)//Si oprime SET
         { 
            if(Opcion==1)
               EstadoDesfogue=!EstadoDesfogue;
            
            if(Opcion==2)
               EstadoAgua=!EstadoAgua;
               
            if(Opcion==3)
               EstadoSSR=!EstadoSSR;
            
            if(Opcion==9)
               EstadoPaso=!EstadoPaso;
            
            if(Opcion==10)
               EstadoVacio=!EstadoVacio;
               
            if(Opcion==11)
               EstadoAire=!EstadoAire;   
               
            if(Opcion==12)
               EstadoSSR2=!EstadoSSR2;
               
            if(Opcion==13)
               EstadoSSR3=!EstadoSSR3;
            
            if(Opcion==14)
               EstadoBuzzer=!EstadoBuzzer;
               
            if(Opcion==15)
               EstadoAireBomba=!EstadoAireBomba;
            delay_ms(100);
         }
      
         strcpy(Menus.Letra,"Estados");
         displayMenu(Menus.Letra,0,0,1,1);
         
      
         if(UP)//Si oprime hacia arriba
         {  
            Opcion--;delay_ms(100);glcd_fillscreen(OFF);
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            Opcion++;delay_ms(100);glcd_fillscreen(OFF);
         }
                          
         if(LEFT)//Si oprime boton de Toma.
         {
            PruebaEstados=OFF;Desfogue_off;EVAgua=OFF;SSR_off;EVAireBomba=OFF;EVAire=OFF;EVVacio=OFF;
            glcd_fillscreen(OFF);Menu=1;delay_ms(500);
         }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==41){ // Menu de Tiempo de Esterilizaci?n
     
      if(visual==1)
      {
         if(unidad<1)
         {unidad=4;}
         
         strcpy(Menus.Letra,"T. Ester.");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(unidad==4)
         {displayTiempo(Testerilizacionp[3],20,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[3],20,40,0,2);}
         
         if(unidad==3)
         {displayTiempo(Testerilizacionp[2],40,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[2],40,40,0,2);}
         
         if(unidad==2)
         {displayTiempo(Testerilizacionp[1],70,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[1],70,40,0,2);}
         
         if(unidad==1)
         {displayTiempo(Testerilizacionp[0],90,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[0],90,40,0,2);}
         
         strcpy(Menus.Letra,":");
         displayMenu(Menus.Letra,60,42,2,2);     
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Testerilizacionp[unidad-1]<9)
            {
               delay_ms(30);Testerilizacionp[unidad-1]++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Testerilizacionp[unidad-1]>0)
            {
               delay_ms(30);Testerilizacionp[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=11;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(0,Testerilizacionp[0]);write_eeprom(1,Testerilizacionp[1]);
            write_eeprom(2,Testerilizacionp[2]);write_eeprom(3,Testerilizacionp[3]);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==42){ // Menu de Tiempo de Secado
     
      if(visual==1)
      {
         if(unidad<1)
         {unidad=4;}
         
         strcpy(Menus.Letra," Secado");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(unidad==4)
         {displayTiempo(Tsecadop[3],20,40,1,2);}
         else
         {displayTiempo(Tsecadop[3],20,40,0,2);}
         
         if(unidad==3)
         {displayTiempo(Tsecadop[2],40,40,1,2);}
         else
         {displayTiempo(Tsecadop[2],40,40,0,2);}
         
         if(unidad==2)
         {displayTiempo(Tsecadop[1],70,40,1,2);}
         else
         {displayTiempo(Tsecadop[1],70,40,0,2);}
         
         if(unidad==1)
         {displayTiempo(Tsecadop[0],90,40,1,2);}
         else
         {displayTiempo(Tsecadop[0],90,40,0,2);}
         
         strcpy(Menus.Letra,":");
         displayMenu(Menus.Letra,60,42,2,2); 
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Tsecadop[unidad-1]<9)
            {
               delay_ms(30);Tsecadop[unidad-1]++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Tsecadop[unidad-1]>0)
            {
               delay_ms(30);Tsecadop[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=11;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(4,Tsecadop[0]);write_eeprom(5,Tsecadop[1]);
            write_eeprom(6,Tsecadop[2]);write_eeprom(7,Tsecadop[3]);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==43){ // Menu de Temperatura
     
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTiempo(Setpoint,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Setpoint<134)
            {
               if(Setpoint<100)
               {
                  Setpoint=100;delay_ms(100);visual=1;
               }
               else
               {
                  delay_ms(30);Setpoint++;visual=1;
               }
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Setpoint>105)
            {
               delay_ms(30);Setpoint--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=11;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(8,Setpoint);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==44){ // Menu de Desfogue
      if(Nivel>6)
         Nivel=6;
         
      if(visual==1)
      {         
         strcpy(Menus.Letra," Desfogue");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTiempo(Nivel,50,40,0,2);
         
         visual=0;
      }
      
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Nivel<6)
            {
               delay_ms(30);Nivel++;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Nivel>1)
            {
               delay_ms(30);Nivel--;visual=1;     
            }
         }
         
         if(desfoguelento==1)
         {
            if(RIGHT)//Si oprime SET
            {  
               write_eeprom(9,Nivel);
               delay_ms(20);
               Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;glcd_fillScreen(OFF);
               MuestraTiemposProgramados();
               IniciaCiclo=ON;
               NivelSeleccionado=9990*Nivel;
            }      
         }
         else
         {
            if(LEFT)//Si oprime boton de Toma.
            {
               if(Nivel<6)
               {
                  desfoguelento=1;
               }
               else
               {
                  desfoguelento=0;
               }
               delay_ms(700);Menu=11;glcd_fillscreen(OFF);Opcion=1;visual=1;
               write_eeprom(9,Nivel);
               NivelSeleccionado=9990*Nivel;
            }
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==45){ //Menu de Continuar
      
      if(visual==1)
      {
         strcpy(Menus.Letra,"Continuar");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(Opcion==2)
         {strcpy(Menus.Letra,"SI");
         displayMenu(Menus.Letra,50,25,1,2);
         strcpy(Menus.Letra,"NO");
         displayMenu(Menus.Letra,50,45,0,2);
         }
         if(Opcion==1)
         {strcpy(Menus.Letra,"SI");
         displayMenu(Menus.Letra,50,25,0,2);
         strcpy(Menus.Letra,"NO");
         displayMenu(Menus.Letra,50,45,1,2);
         }
         visual=0;
      }
      
        
      if(UP)//Si oprime hacia arriba
      {
         if(Opcion<2)
         {
            Opcion++;
         }
         visual=1;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Opcion>1)
         {
            Opcion--;
         }  
         visual=1;
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==2)
         {
            Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;enable_interrupts(global);glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Calentando");
            displayMenu(Menus.Letra,0,55,2,1);
            strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
            strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
            MuestraTiemposProgramados();
            if(Nivel<6)
               {
                  desfoguelento=1;
               }
               else
               {
                  desfoguelento=0;
               }
               IniciaCiclo=ON;
               enable_interrupts(global);
         }
         else
         {
            delay_ms(700);Menu=8;glcd_fillscreen(OFF);Opcion=1;visual=1;
         }         
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=11;glcd_fillscreen(OFF);Opcion=1;visual=1;
      }
    
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   if(Menu==20){ // Este es el menu de cuando el proceso se inicio.
   
   if(RIGHT){
      glcd_init(ON);
      delay_ms(1000);
      PosVacio=OFF;
      PosCalentando=OFF;
      PosEsterilizando=OFF;
      PosDesfogando=OFF;
      PosSecando=OFF;
   }
   
   if(TiempoCiclo>5400){
      AlarmaTiempoProlongado=ON;
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
   }else{
      AlarmaTermostato=OFF;
   }
   
   if(!DOOR){
      AlarmaPuerta=ON;
   }else{
      AlarmaPuerta=OFF;
   }

   if(!STOP){
      AlarmaEmergencia=ON;
   }
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaLlenado || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaVacio || AlarmaBomba){
      if(AlarmaEmergencia)
      {
         BorraStatus();
         strcpy(Menus.Letra,"Parada Emergencia");displayMenu(Menus.Letra,0,55,2,1);
         SSR2=OFF;
         SSR3=OFF;
         EVVacio=OFF;
         Desfogue_on;
         EVAire=OFF;                     
         Paso_off;
         EVAgua=OFF;
         EVAireBomba=OFF;
         SSR_off;
         if(!BorraCuadro){
            glcd_fillscreen(OFF);
            PosVacio=OFF;
            PosCalentando=OFF;
            PosEsterilizando=OFF;
            PosDesfogando=OFF;
            PosSecando=OFF;
            glcd_imagen(6);
            BorraCuadro=ON;
         }
      }else{
         if(AlarmaPuerta)
         {
            BorraStatus();
            strcpy(Menus.Letra,"Puerta Abierta");displayMenu(Menus.Letra,0,55,2,1);
            SSR2=OFF;
            SSR3=OFF;
            EVVacio=OFF;
            Desfogue_on;
            EVAire=OFF;                     
            Paso_off;
            EVAgua=OFF;
            EVAireBomba=OFF;
            SSR_off;
            if(!BorraCuadro){
               glcd_fillscreen(OFF);
               PosVacio=OFF;
               PosCalentando=OFF;
               PosEsterilizando=OFF;
               PosDesfogando=OFF;
               PosSecando=OFF;
               glcd_imagen(6);
               BorraCuadro=ON;
            }
         }else{
            if(AlarmaLlenado)
            {
               BorraStatus();
               strcpy(Menus.Letra,"Error:EA01");displayMenu(Menus.Letra,0,55,2,1);
               SSR2=OFF;
               SSR3=OFF;
               EVVacio=OFF;
               Desfogue_off;
               EVAire=OFF;                     
               Paso_off;
               EVAgua=OFF;
               EVAireBomba=OFF;
               SSR_off;
               if(!BorraCuadro){
                  glcd_fillscreen(OFF);
                  PosVacio=OFF;
                  PosCalentando=OFF;
                  PosEsterilizando=OFF;
                  PosDesfogando=OFF;
                  PosSecando=OFF;
                  glcd_imagen(6);
                  BorraCuadro=ON;
               }
            }else{
               if(AlarmaTermostato) // Alarma de Termostato
               {
                  BorraStatus();
                  tiempo_esterilizacion=0;tiempo_secado=0;         
                  strcpy(Menus.Letra,"Error:EA02");displayMenu(Menus.Letra,0,55,2,1);          
                  //AguaIn_off;
                  SSR_off;
                  if(!BorraCuadro){
                     glcd_fillscreen(OFF);
                     PosVacio=OFF;
                     PosCalentando=OFF;
                     PosEsterilizando=OFF;
                     PosDesfogando=OFF;
                     PosSecando=OFF;
                     glcd_imagen(6);
                     BorraCuadro=ON;
                  }
               }else{
                  if(AlarmaSobreTemperatura) // Alarma de Sobre Temperatura
                  {
                     BorraStatus();
                     tiempo_esterilizacion=0;tiempo_secado=0;         
                     strcpy(Menus.Letra,"Error:ET02");displayMenu(Menus.Letra,0,55,2,1);
                     Desfogue_on;
                     //AguaIn_off;
                     SSR_off;
                     if(!BorraCuadro){
                        glcd_fillscreen(OFF);
                        PosVacio=OFF;
                        PosCalentando=OFF;
                        PosEsterilizando=OFF;
                        PosDesfogando=OFF;
                        PosSecando=OFF;
                        glcd_imagen(6);
                        BorraCuadro=ON;
                     }
                  }else{
                     if(AlarmaTiempoProlongado) // Alarma de Error de Tiempo: Error por exceso de tiempo de calentamiento.
                     {
                        BorraStatus();
                        strcpy(Menus.Letra,"Error:ET01");displayMenu(Menus.Letra,0,55,2,1);
                        Desfogue_on;
                        //AguaIn_off;
                        SSR_off;
                        if(!BorraCuadro){
                           glcd_fillscreen(OFF);
                           PosVacio=OFF;
                           PosCalentando=OFF;
                           PosEsterilizando=OFF;
                           PosDesfogando=OFF;
                           PosSecando=OFF;
                           glcd_imagen(6);
                           BorraCuadro=ON;
                        }
                     }else{
                        if(AlarmaVacio) // Alarma de Tiempo prolongado para hacer vacio
                        {
                           BorraStatus();
                           strcpy(Menus.Letra,"Error Ciclo");displayMenu(Menus.Letra,0,55,2,1);
                           Vacio=ON;
                           SSR2=OFF;
                           SSR3=OFF;
                           EVVacio=OFF;
                           Desfogue_on;
                           EVAire=ON;                     
                           EVAgua=OFF;
                           EVAireBomba=OFF;
                           IniciaVacio=OFF;
                           //AguaIn_off;
                           SSR_off;
                           if(!BorraCuadro){
                              glcd_fillscreen(OFF);
                              PosVacio=OFF;
                              PosCalentando=OFF;
                              PosEsterilizando=OFF;
                              PosDesfogando=OFF;
                              PosSecando=OFF;
                              glcd_imagen(6);
                              BorraCuadro=ON;
                           }else{
                              if(AlarmaBomba) // Alarma de Tiempo prolongado para hacer vacio
                              {
                                 BorraStatus();
                                 strcpy(Menus.Letra,"Error Bomba");displayMenu(Menus.Letra,0,55,2,1);
                                 Vacio=ON;
                                 SSR2=OFF;
                                 SSR3=OFF;
                                 EVVacio=OFF;
                                 Desfogue_on;
                                 EVAire=ON;                     
                                 EVAgua=OFF;
                                 EVAireBomba=OFF;
                                 IniciaVacio=OFF;
                                 //AguaIn_off;
                                 SSR_off;
                                 if(!BorraCuadro){
                                    glcd_fillscreen(OFF);
                                    PosVacio=OFF;
                                    PosCalentando=OFF;
                                    PosEsterilizando=OFF;
                                    PosDesfogando=OFF;
                                    PosSecando=OFF;
                                    glcd_imagen(6);
                                    BorraCuadro=ON;
                                 }
                              }
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
      BorraCuadro=OFF;
         if(!CalderinLleno || !Precalentamiento)
         {      
            glcd_imagen(6);
            BorraStatus();
            if(!CalderinLleno){
               strcpy(Menus.Letra,"Llenando");displayMenu(Menus.Letra,0,55,2,1);
            }else{
               if(!Precalentamiento){
                  strcpy(Menus.Letra,"Pre-Calentando");displayMenu(Menus.Letra,0,55,2,1);
                  ControlaPresion();
               }
            }
            Control=0;
            IniciaVacio=OFF;
            IniciaVacio2=OFF;
         }
         else
         {
            if(!Vacio){
               if(!flag_vac){
                  flag_vac=ON;
                  SSR2=ON;
                  SSR3=ON;
                  EVVacio=OFF;
                  Desfogue_off;
                  EVAire=OFF;
                  EVAireBomba=ON;
                  Paso_off;
                  EVAgua=OFF;
                  glcd_update();
                  Envio_Esclavos();
                  delay_ms(7000);
                  IniciaVacio2=ON;
               }
               CuadroVacio(ON);
               BorraStatus();
               strcpy(Menus.Letra,"Prevacio");displayMenu(Menus.Letra,0,55,2,1);
               ControlaPresion();
               IniciaVacio=ON;
               
               if(PresionCamara>NivelVacio && PulsoPositivo==10){
                  flag_vacio=1;
                  SSR2=ON;
                  SSR3=ON;
                  EVVacio=ON;
                  Desfogue_off;
                  EVAire=OFF;                     
                  Paso_off;
                  EVAgua=OFF;
                  EVAireBomba=OFF;
                  flag_pulso=0;
               }else{
                  PulsoPositivo=5;
               }
           
               if(PulsoPositivo==5){
                  if(flag_pulso==0){
                     PulsosVacio++;
                     flag_pulso=1;
                  }
                  SSR2=ON;
                  SSR3=ON;
                  EVVacio=OFF;
                  Desfogue_off;
                  EVAire=OFF;                     
                  Paso_on;
                  EVAgua=OFF;
                  EVAireBomba=ON;
                  if(PresionCamara>=Atmosferica+25){
                    Paso_off;
                    PulsoPositivo=10;
                  }  
               }
               
               if(tbomba>=120){
                  if(PresionCamara>Atmosferica){
                     AlarmaBomba=ON;
                  }else{
                     tbomba=0;
                     IniciaVacio2=OFF;
                  }
               }
               
               if(tvacio>=TiempoVacio){            
                  AlarmaVacio=ON;
               }
                  
               if(PulsosVacio>=CantidadPulsos)
               {
                  Vacio=ON;
                  SSR2=OFF;
                  SSR3=OFF;
                  EVVacio=OFF;
                  Desfogue_off;
                  EVAire=OFF;                     
                  EVAgua=OFF;
                  EVAireBomba=OFF;
                  IniciaVacio=OFF;
                  CuadroVacio(OFF);
               }
               
            }else{
               IniciaVacio=OFF;
               if(Temperatura>=((float)Setpoint-0.3)){
                  Esterilizando=ON;
               }
               
               if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
                  Esterilizando=OFF;
               }
            
               if(!Esterilizando)
               {
                  flag_vac=OFF;
                  Paso_on;
                  CuadroCalentando(ON);
                  BorraStatus();
                  strcpy(Menus.Letra,"Calentando");displayMenu(Menus.Letra,0,55,2,1);
                  Control=1;
                  Controla();
                  BorraCuadro=OFF;
               }else{
                  if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&
                  (Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3])){ 
                     if(desfoguelento){
                        if(PresionCamara<(Atmosferica+4)){
                           SSR2=OFF;
                           SSR3=OFF;
                           EVVacio=OFF;
                           Desfogue_on;
                           EVAire=OFF;                     
                           Paso_off;
                           EVAgua=OFF;
                           EVAireBomba=OFF;
                           Secando=OFF;
                           SSR_off;
                           EnciendeBuzzer=ON;
                           DesfogueSuave=OFF;
                           BorraStatus();
                           strcpy(Menus.Letra,"Ciclo Finalizado");displayMenu(Menus.Letra,0,55,2,1);
                           if(!Ciclo){
                              Ciclo=ON;
                              Ciclos++;
                              CiclosL=make8(Ciclos,0);
                              CiclosH=make8(Ciclos,1);
                              write_eeprom(37,CiclosL);
                              delay_ms(20);
                              write_eeprom(38,CiclosH);
                              delay_ms(20);
                           }
                           
                        }else{
                           Paso_off;
                           SSR2=OFF;
                           SSR3=OFF;
                           EVVacio=OFF;                           
                           EVAire=OFF;                     
                           Paso_off;
                           EVAgua=OFF;
                           EVAireBomba=OFF;
                           Secando=OFF;
                           SSR_off;
                           Desfogando=ON;
                           DesfogueSuave=ON;
                           CuadroDesfogando(ON);
                           BorraStatus();
                           strcpy(Menus.Letra,"Despresurizando");displayMenu(Menus.Letra,0,55,2,1);
                           Control=0;
                        }
                     }else{
                        Control=0;
                        if(PresionCamara<(Atmosferica+20)){
                           if((Tsecado[0]>=Tsecadop[0])&&(Tsecado[1]>=Tsecadop[1])&&(Tsecado[2]>=Tsecadop[2])&&(Tsecado[3]>=Tsecadop[3])){
                              SSR2=OFF;
                              SSR3=OFF;
                              EVVacio=OFF;
                              Desfogue_off;
                              EVAire=ON;                     
                              Paso_off;
                              EVAgua=OFF;
                              EVAireBomba=OFF;
                              Secando=OFF;
                              SSR_off;
                              CuadroSecando(OFF);
                              if(PresionCamara>(Atmosferica-5)){
                                 EnciendeBuzzer=ON;
                                 BorraStatus();
                                 strcpy(Menus.Letra,"Ciclo Finalizado");displayMenu(Menus.Letra,0,55,2,1);
                              }

                              if(!Ciclo){
                                 Ciclo=ON;
                                 Ciclos++;
                                 CiclosL=make8(Ciclos,0);
                                 CiclosH=make8(Ciclos,1);
                                 write_eeprom(37,CiclosL);
                                 delay_ms(20);
                                 write_eeprom(38,CiclosH);
                                 delay_ms(20);
                              }
                           }else{
                              ControlaPresion();
                              Paso_off;
                              Secando=ON;
                              CuadroDesfogando(OFF);
                              CuadroSecando(ON);
                              BorraStatus();
                              strcpy(Menus.Letra,"Secando");displayMenu(Menus.Letra,0,55,2,1);
                              
                              if(PresionCamara>NivelVacio+5){
                                 SSR2=ON;
                                 SSR3=ON;
                                 EVVacio=ON;
                                 Desfogue_off;
                                 EVAire=OFF;                     
                                 Paso_off;
                                 EVAgua=OFF;
                                 EVAireBomba=OFF;
                              }
         
                              if(PresionCamara<NivelVacio){
                                 SSR2=ON;
                                 SSR3=ON;
                                 EVVacio=OFF;
                                 Desfogue_off;
                                 EVAire=ON;                     
                                 Paso_off;
                                 EVAgua=OFF;
                                 EVAireBomba=ON;
                              }
                           }
                        }else{
                           Paso_off;
                           Desfogando=ON;                           
                           CuadroEsterilizando(OFF);
                           CuadroDesfogando(ON);
                           BorraStatus();
                           strcpy(Menus.Letra,"Despresurizando");displayMenu(Menus.Letra,0,55,2,1);
                           Control=0;
                           SSR_off;
                           SSR2=ON;
                           SSR3=ON;
                           EVVacio=OFF;
                           EVAireBomba=ON;
                           Desfogue_on;
                        }
                     }
                     
                  }else{
                     Paso_on;                     
                     CuadroCalentando(OFF);
                     CuadroEsterilizando(ON);
                     BorraStatus();
                     strcpy(Menus.Letra,"Esterilizando");displayMenu(Menus.Letra,0,55,2,1);
                     Control=1;
                     Controla();
                  }
               } 
            } 
         }
   }

   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      displayT(PresionCamara,31,35,1);
      displayT(PresionPreCamara,31,44,1);
      glcd_rect(49, 35, 62, 48, YES, OFF);glcd_text57(49, 35, kpa, 1, ON);
      glcd_rect(49, 44, 62, 48, YES, OFF);glcd_text57(49, 44, kpa, 1, ON);
      displayTT(Temperatura,69,20,2);
      glcd_circle(95,40,2,NO,ON);
      strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
      displayTiempo(Setpoint,5,20,2,1);
      
      strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
      strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
      MuestraTiemposProgramados();
      // Muestra Valor de esterilizacion actual.
      displayTiempo(Testerilizacion[3],80,0,0,1);displayTiempo(Testerilizacion[2],86,0,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,0,2,1); 
      displayTiempo(Testerilizacion[1],98,0,0,1);displayTiempo(Testerilizacion[0],104,0,0,1);
      // Muestra tiempo de secado actual.
      displayTiempo(Tsecado[3],80,8,0,1);displayTiempo(Tsecado[2],86,8,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,8,2,1); 
      displayTiempo(Tsecado[1],98,8,0,1);displayTiempo(Tsecado[0],104,8,0,1);
   }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   if(Menu==30){ // Este es el menu de cuando el proceso se inicio.
   
   //Lectura_Esclavos();
   
   //Envio_Esclavos();
   // Nuevo Flujo de Programa con logica 2018 para ajustar ciclos de Pre-Vacio y de Vacio al final del ciclo//   
   
   if(RIGHT){
      glcd_init(ON);
      delay_ms(1000);
      PosVacio=OFF;
      PosCalentando=OFF;
      PosEsterilizando=OFF;
      PosDesfogando=OFF;
      PosSecando=OFF;
   }
   
   if(!TERM){
      AlarmaTermostato=ON;
   }else{
      AlarmaTermostato=OFF;
   }
   
   if(!DOOR){
      AlarmaPuerta=ON;
   }else{
      AlarmaPuerta=OFF;
   }

   if(!STOP){
      AlarmaEmergencia=ON;
   }
   
   if(AlarmaPuerta || AlarmaTermostato || AlarmaLlenado || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaBomba){
      
      if(AlarmaEmergencia){
         BorraStatus();
         strcpy(Menus.Letra,"Parada Emergencia");displayMenu(Menus.Letra,0,55,2,1);
         SSR2=OFF;
         SSR3=OFF;
         EVVacio=OFF;
         Desfogue_on;
         EVAire=OFF;                     
         Paso_off;
         EVAgua=OFF;
         EVAireBomba=OFF;
         SSR_off;
         flag_vac=OFF;
      }else{
         if(AlarmaPuerta)
         {
            BorraStatus();
            strcpy(Menus.Letra,"Puerta Abierta");displayMenu(Menus.Letra,0,55,2,1);
            SSR2=OFF;
            SSR3=OFF;
            EVVacio=OFF;
            Desfogue_on;
            EVAire=OFF;                     
            Paso_off;
            EVAgua=OFF;
            EVAireBomba=OFF;
            SSR_off;
            flag_vac=OFF;
         }else{
            if(AlarmaTermostato) // Alarma de Termostato
            {
               BorraStatus();
               tiempo_esterilizacion=0;tiempo_secado=0;         
               strcpy(Menus.Letra,"Error:EA02");displayMenu(Menus.Letra,0,55,2,1);          
               //AguaIn_off;
               SSR_off;
               flag_vac=OFF;
            }else{
               if(AlarmaBomba) // Alarma de Tiempo prolongado para hacer vacio
               {
                  BorraStatus();
                  strcpy(Menus.Letra,"Error Bomba");displayMenu(Menus.Letra,0,55,2,1);
                  Vacio=ON;
                  SSR2=OFF;
                  SSR3=OFF;
                  EVVacio=OFF;
                  Desfogue_on;
                  EVAire=ON;                     
                  EVAgua=OFF;
                  EVAireBomba=OFF;
                  IniciaVacio=OFF;
                  //AguaIn_off;
                  SSR_off;
                  if(!BorraCuadro){
                     glcd_fillscreen(OFF);
                     PosVacio=OFF;
                     PosCalentando=OFF;
                     PosEsterilizando=OFF;
                     PosDesfogando=OFF;
                     PosSecando=OFF;
                     glcd_imagen(6);
                     BorraCuadro=ON;
                  }
               }
            }      
         }
      }
   }
   else
   {      
      if(!flag_vac){
         flag_vac=ON;
         SSR2=ON;
         SSR3=ON;
         EVVacio=OFF;
         Desfogue_off;
         EVAire=OFF;
         EVAireBomba=ON;
         Paso_off;
         EVAgua=OFF;
         SSR_off;
         Envio_Esclavos();
         EVAireBomba=OFF;
         BorraStatus();
         strcpy(Menus.Letra,"Test Vacio");displayMenu(Menus.Letra,0,55,2,1);
         delay_ms(7000);
         IniciaVacio2=ON;
      }
      
      if(tbomba>=120){
         if(PresionCamara>Atmosferica){
            AlarmaBomba=ON;
         }else{
            tbomba=0;
            IniciaVacio2=OFF;
         }
      }
      
      if(PresionCamara<17){
         SSR2=OFF;
         SSR3=OFF;
         EVVacio=OFF;
         Desfogue_off;
         EVAire=OFF;
         EVAireBomba=OFF;
         Paso_off;
         SSR_off;
         Esterilizando=ON;
         IniciaVacio=OFF;
         tbomba=0;
      } 
      if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&
      (Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3])){                
         IniciaVacio=OFF;
         tbomba=0;
         SSR2=OFF;
         SSR3=OFF;
         EVVacio=OFF;
         Desfogue_off;
         EVAire=ON;
         EVAireBomba=OFF;
         Paso_off;
         SSR_off;
         EnciendeBuzzer=ON;
         if(PresionCamara<=20 && !Desfogando){
            BorraStatus();
            strcpy(Menus.Letra,"Ciclo Finalizado");displayMenu(Menus.Letra,0,55,2,1);
         }else{
            BorraStatus();
            strcpy(Menus.Letra,"Error Ciclo");displayMenu(Menus.Letra,0,55,2,1);
         }
         if(!Ciclo){
            Ciclo=ON;
            Ciclos++;
            CiclosL=make8(Ciclos,0);
            CiclosH=make8(Ciclos,1);
            write_eeprom(37,CiclosL);
            delay_ms(20);
            write_eeprom(38,CiclosH);
            delay_ms(20);
         }
         Desfogando=ON;
         Esterilizando=OFF;
      }
   } 
   
   
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {     
      Lectura=0;
      glcd_imagen(12);
      displayT(PresionCamara,31,33,1);
      displayT(PresionPreCamara,31,42,1);
      glcd_rect(51, 33, 62, 46, YES, OFF);glcd_text57(52, 33, kpa, 1, ON);
      glcd_rect(51, 42, 62, 46, YES, OFF);glcd_text57(52, 42, kpa, 1, ON);
      displayTT(Temperatura,69,20,2);
      glcd_circle(95,40,2,NO,ON);
      strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,102,40,2,2);
 
      strcpy(Menus.Letra,"T est:");displayMenu(Menus.Letra,0,0,2,1);
      strcpy(Menus.Letra,"T sec:");displayMenu(Menus.Letra,0,8,2,1);
      MuestraTiemposProgramados();
      // Muestra Valor de esterilizacion actual.
      displayTiempo(Testerilizacion[3],80,0,0,1);displayTiempo(Testerilizacion[2],86,0,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,0,2,1); 
      displayTiempo(Testerilizacion[1],98,0,0,1);displayTiempo(Testerilizacion[0],104,0,0,1);
      // Muestra tiempo de secado actual.
      displayTiempo(Tsecado[3],80,8,0,1);displayTiempo(Tsecado[2],86,8,0,1);
      strcpy(Menus.Letra,":");displayMenu(Menus.Letra,92,8,2,1); 
      displayTiempo(Tsecado[1],98,8,0,1);displayTiempo(Tsecado[0],104,8,0,1);
   }
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
 
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
      
      if(Opcion>12)
      {Opcion=1;}
      if(Opcion<1)
      {Opcion=12;}
      
      if(Opcion<=6){
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Proporcional");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Proporcional");
            displayMenu(Menus.Letra,0,0,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Integral");
            displayMenu(Menus.Letra,0,9,1,1);}
         else
            {strcpy(Menus.Letra,"Integral");
            displayMenu(Menus.Letra,0,9,0,1);}
            
         if(Opcion==3)   
            {strcpy(Menus.Letra,"Derivativo");
            displayMenu(Menus.Letra,0,18,1,1);}
         else
            {strcpy(Menus.Letra,"Derivativo");
            displayMenu(Menus.Letra,0,18,0,1);}   
         
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,27,1,1);}
         else
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,27,0,1);}
            
         if(Opcion==5)   
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,36,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,36,0,1);}   
                        
         if(Opcion==6)   
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,45,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,45,0,1);}     
      }
      else{
         if(Opcion==7)   
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,0,0,1);}     
            
         if(Opcion==8)   
            {strcpy(Menus.Letra,"Ganancia    ");
            displayMenu(Menus.Letra,0,9,1,1);}
         else
            {strcpy(Menus.Letra,"Ganancia    ");
            displayMenu(Menus.Letra,0,9,0,1);}  
            
         if(Opcion==9)   
            {strcpy(Menus.Letra,"R51         ");
            displayMenu(Menus.Letra,0,18,1,1);}
         else
            {strcpy(Menus.Letra,"R51         ");
            displayMenu(Menus.Letra,0,18,0,1);}     
         
         if(Opcion==10)   
            {strcpy(Menus.Letra,"RPT100 B     ");
            displayMenu(Menus.Letra,0,27,1,1);}
         else
            {strcpy(Menus.Letra,"RPT100 B     ");
            displayMenu(Menus.Letra,0,27,0,1);}     
            
         if(Opcion==11)   
            {strcpy(Menus.Letra,"Pulsos Vacio");
            displayMenu(Menus.Letra,0,36,1,1);}
         else
            {strcpy(Menus.Letra,"Pulsos Vacio");
            displayMenu(Menus.Letra,0,36,0,1);}   
                        
         if(Opcion==12)   
            {strcpy(Menus.Letra,"Nivel Vacio");
            displayMenu(Menus.Letra,0,45,1,1);}
         else
            {strcpy(Menus.Letra,"Nivel Vacio");
            displayMenu(Menus.Letra,0,45,0,1);}  
      }   
      
         if(UP)//Si oprime hacia arriba
         {
            delay_ms(10);
            if(Flanco == 0)
               {Flanco = 1;Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {
            delay_ms(10);
            if(Flanco2 == 0)
               {Flanco2 = 1;Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            if(Flanco1 == 0)
               {Flanco1 = 1;delay_ms(30);Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;} 
               
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;            
         }      
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==101){ // Menu de Ajuste de Ganancia
      
      if(Ganancia>40)
      {
         Ganancia=2;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"P");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ganancia,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Ganancia>40)
            {
               Ganancia=40;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ganancia+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Ganancia>1)
            {
               delay_ms(30);Ganancia-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(20,(int8)Ganancia);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==102){ // Menu de Ajuste de Integral
      
      if(Integral>40)
      {
         Integral=1;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"I");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Integral,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Integral>40)
            {
               Integral=40;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Integral+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Integral>1)
            {
               delay_ms(30);Integral-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(26,(int8)Integral);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==103){ // Menu de Ajuste de Derivativo
      
      if(Derivativo>200)
      {
         Derivativo=2;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"D");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Derivativo,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Derivativo>200)
            {
               Derivativo=200;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Derivativo+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Derivativo>1)
            {
               delay_ms(30);Derivativo-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(27,(int8)Derivativo);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==104){ // Menu de Tiempo de Ciclo
      
      if(Tciclo>40)
      {
         Tciclo=40;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Tciclo");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Tciclo,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Tciclo>40)
            {
               Tciclo=40;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Tciclo+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Tciclo>1)
            {
               delay_ms(30);Tciclo-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(25,(int8)Tciclo);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==105){ // Menu de Ajuste de Ajuste+
      
      if(Ajuste>100)
      {
         Ajuste=0;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Ajuste");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ajuste,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Ajuste>100)
            {
               Ajuste=100;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ajuste+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Ajuste>0)
            {
               delay_ms(30);Ajuste-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(21,Ajuste);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==106){ // Menu de Ajuste de Ajuste -
      
      if(Ajusten>100)
      {
         Ajusten=0;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Ajusten");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ajusten,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(Ajusten>100)
            {
               Ajusten=100;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ajusten+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(Ajusten>0)
            {
               delay_ms(30);Ajusten-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(22,Ajusten);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==107){ // Menu de Ajuste de Media Movil
      
      if(MediaMovil>80)
      {
         MediaMovil=10;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Media");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(MediaMovil,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            if(MediaMovil>79)
            {
               MediaMovil=79;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);MediaMovil+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            if(MediaMovil>1)
            {
               delay_ms(30);MediaMovil-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(24,(int8)MediaMovil);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==108){ // Menu de Ganancia Instrumentacion
      
      if(Gain>50 || Gain<30)
      {
         Gain=40;
      }
         strcpy(Menus.Letra,"Ganancia");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTT(Gain,30,30,2);

      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            Gain+=0.1;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            Gain-=0.1;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(28,(int8)Gain);
            delay_ms(10);
            Temporal=Gain-((int8)Gain);
            Temporal=Temporal*10;
            write_eeprom(29,(int8)Temporal);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==109){ // Menu de Ajuste R1
      
      if(R1>6400 || R1<6000)
      {
         R1=6200;
      }

         strcpy(Menus.Letra,"R51");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(R1,30,30,0,3);

         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            R1+=1.0;
            glcd_fillscreen(OFF);
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            R1-=1.0;
            glcd_fillscreen(OFF);
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(30,make8((int16)R1,0));
            delay_ms(10);
            write_eeprom(31,make8((int16)R1,1));
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==110){ // Menu de Ajuste RPT100
      
      if(RPT100>2.0)
      {
         RPT100=0.5;
      }
   
         strcpy(Menus.Letra,"RPT100");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTT(RPT100,30,30,2);

         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            RPT100+=0.1;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            RPT100-=0.1;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(32,(int8)RPT100);
            delay_ms(10);
            Temporal=RPT100-((int8)RPT100);
            Temporal=Temporal*10;
            write_eeprom(33,(int8)Temporal);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==111){ // Menu de Pulsos de Vacio
      
      if(CantidadPulsos>10)
         CantidadPulsos=1;
      if(CantidadPulsos<1)
         CantidadPulsos=10;
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Pulsos Vac.");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(CantidadPulsos,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            visual=1;
            CantidadPulsos++;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            visual=1;
            CantidadPulsos--;
         }
               
         if(RIGHT)//Si oprime SET
         {              
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(35,CantidadPulsos);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==112){ // Menu de Nivel de Vacio
      
      if(NivelVacio>100)
         NivelVacio=1;
      if(NivelVacio<1)
         NivelVacio=100;
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Nivel Vacio");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(NivelVacio,30,30,0,3);
         strcpy(Menus.Letra,"kPa");
         displayMenu(Menus.Letra,90,50,0,1);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(10);
            visual=1;
            NivelVacio++;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(10);
            visual=1;
            NivelVacio--;
         }
               
         if(RIGHT)//Si oprime SET
         {              
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(36,NivelVacio);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------


  while(Menu==200)
 {
   
   displayTiempo(txbuf[0],0,10,2,1);
   displayTiempo(txbuf[1],0,20,2,1);
   displayTiempo(txbuf[2],0,30,2,1);
   displayTiempo(txbuf[3],0,40,2,1);
   displayTiempo(SlaveA0Tx[0],0,50,2,1);
   Lectura_Esclavos();
   Envio_Esclavos();
   Opcion++;
   displayTiempo(SlaveA0Tx[1],50,10,2,1);
   displayTiempo(SlaveA0Tx[2],50,20,2,1);
   displayTiempo(SlaveA0Tx[3],50,30,2,1);     
   glcd_update();
 }
 
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==210){ // Menu de Presion Atmosferica  
         strcpy(Menus.Letra,"Atmosfera");
         displayMenu(Menus.Letra,0,0,1,2);
         strcpy(Menus.Letra,"kpa");displayMenu(Menus.Letra,92,30,2,1);
         
         displayTiempo(Atmosferica,30,30,0,3); 
      
         if(UP)//Si oprime hacia arriba
         {  
            delay_ms(30);Atmosferica++;visual=1;
            glcd_fillscreen(OFF);
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            delay_ms(30);Atmosferica--;visual=1;     
            glcd_fillscreen(OFF);
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(34,Atmosferica);delay_ms(10);
            reset_cpu();
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==211){ // Menu de Ciclos
         strcpy(Menus.Letra,"Ciclos");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTiempo(Ciclos,30,30,0,3); 
                  
         if(RIGHT && LEFT)//Si oprime boton de Toma.
         {
            write_eeprom(37,0);
            delay_ms(20);
            write_eeprom(38,0);
            delay_ms(20);
            reset_cpu();
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

   }
}


void main()
{
   Display_on;
   for (posicion=0;posicion<0x10;posicion++)
   {
      SlaveA0Tx[posicion] = 0x00;
   }
   
   delay_ms(1000);   
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_1);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN3);
   setup_adc(ADC_CLOCK_DIV_32 );
   enable_interrupts(global);

//Lectura EEPROM//--------------------------
   Testerilizacionp[0]=read_eeprom(0);
   delay_ms(10);
   Testerilizacionp[1]=read_eeprom(1);
   delay_ms(10);
   Testerilizacionp[2]=read_eeprom(2);
   delay_ms(10);
   Testerilizacionp[3]=read_eeprom(3);
   delay_ms(10);
   Tsecadop[0]=read_eeprom(4);
   delay_ms(10);
   Tsecadop[1]=read_eeprom(5);
   delay_ms(10);
   Tsecadop[2]=read_eeprom(6);
   delay_ms(10);
   Tsecadop[3]=read_eeprom(7);
   delay_ms(10);
   Setpoint=read_eeprom(8);
   delay_ms(10);
   Nivel=read_eeprom(9);
   delay_ms(10);
   Password[0]=read_eeprom(10);
   delay_ms(10);
   Password[1]=read_eeprom(11);
   delay_ms(10);
   Password[2]=read_eeprom(12);
   delay_ms(10);
   Password[3]=read_eeprom(13);
   delay_ms(10);
   Ganancia=read_eeprom(20);
   delay_ms(10);
   Ajuste=read_eeprom(21);
   delay_ms(10);
   Ajusten=read_eeprom(22);
   delay_ms(10);
   //Desvio=read_eeprom(23);
   MediaMovil=read_eeprom(24);
   delay_ms(10);
   Tciclo=read_eeprom(25);
   delay_ms(10);
   Integral=read_eeprom(26);
   delay_ms(10);
   Derivativo=read_eeprom(27);
   delay_ms(10);
   Temporal=read_eeprom(29);
   Temporal=Temporal/10.0;
   delay_ms(10);
   Gain=read_eeprom(28)+Temporal;
   delay_ms(10);
   R1=make16(read_eeprom(31),read_eeprom(30));
   delay_ms(10);
   Temporal=read_eeprom(33);
   Temporal=Temporal/10.0;
   delay_ms(10);
   RPT100=read_eeprom(32)+Temporal;
   delay_ms(10);
   Atmosferica=(int8)read_eeprom(34);
   delay_ms(10);
   CantidadPulsos=read_eeprom(35);
   delay_ms(10);
   NivelVacio=read_eeprom(36);
   delay_ms(10);
   CiclosL=read_eeprom(37);
   delay_ms(10);
   CiclosH=read_eeprom(38);
   delay_ms(10);
   Ciclos=make16(CiclosH,CiclosL);
   
   if(Testerilizacionp[0]>9)Testerilizacionp[0]=0;
   if(Testerilizacionp[1]>9)Testerilizacionp[1]=0;
   if(Testerilizacionp[2]>9)Testerilizacionp[2]=0;
   if(Testerilizacionp[3]>9)Testerilizacionp[3]=0;
   if(Password[0]>9)Password[0]=0;
   if(Password[1]>9)Password[1]=0;
   if(Password[2]>9)Password[2]=0;
   if(Password[3]>9)Password[3]=0;
   if(Ganancia>20.0)Ganancia=15.0;
   if(Integral>10.0)Integral=1.0;
   if(Derivativo>90.0)Derivativo=15.0;
   if(Tciclo>10)Tciclo=10;
   if(Ajuste>50)Ajuste=0;
   if(Ajusten>50)Ajusten=0;
   if(MediaMovil>80)MediaMovil=80;
   if(Gain>50)Gain=40.2;
   if(R1>6400)R1=6200;
   if(RPT100>2.0)RPT100=0.5;
   if(CantidadPulsos>10)CantidadPulsos=4;
   if(NivelVacio>100)NivelVacio=25;
   if(Ciclos>60000)Ciclos=0;
   
   TiempoVacio=CantidadPulsos*1200;
   
   EVVacio=OFF;EVAire=OFF;EVAgua=OFF;SSR2=OFF;SSR3=OFF;Buzzer=OFF;EVAireBomba=OFF;
   Envio_Esclavos();
//Mensaje Bienvenida//-----------------------------   
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   strcpy(Menus.Letra,"JP Inglobal");
   displayMenu(Menus.Letra,0,0,0,2);     
   strcpy(Menus.Letra,"Autoclave Horizontal");
   displayMenu(Menus.Letra,0,20,0,1);       
   strcpy(Menus.Letra," 85 Litros  ");
   displayMenu(Menus.Letra,0,35,0,2);
   delay_ms(3000);
   glcd_fillScreen(OFF);      //Limpia la pantalla
   displayContrasena(Contrasena[0],20,30,1);
   displayContrasena(Contrasena[1],40,30,0);
   displayContrasena(Contrasena[2],60,30,0);
   displayContrasena(Contrasena[3],80,30,0);
   
   if(UP){
      delay_ms(500);
      glcd_fillScreen(OFF);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=211;
      glcd_fillScreen(OFF);
   }
   
   if(DOWN)// && RIGHT && LEFT)
   {
      delay_ms(500);
      glcd_fillScreen(OFF);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=100;
      glcd_fillScreen(OFF);
   }
   
   for(i=0;i<MediaMovil;i++){
      Temperatura=LeerPT100(MediaMovil);
   }
   
   if(LEFT){
      delay_ms(500);
      glcd_fillScreen(OFF);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=200;
      glcd_fillScreen(OFF);
   }
   
   if(RIGHT){
      delay_ms(500);
      glcd_fillScreen(OFF);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=210;
      glcd_fillScreen(OFF);
   }

   CorrePrograma();
}   


