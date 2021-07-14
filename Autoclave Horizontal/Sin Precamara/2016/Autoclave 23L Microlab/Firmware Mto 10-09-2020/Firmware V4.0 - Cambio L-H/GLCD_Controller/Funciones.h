
typedef struct{
   char Letra[30];
   char Estado[4];
}MENUU;

MENUU Menus;


void displayTemperatura(int16 digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==2)
   {
      sprintf(voltage, "%04Lu", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }
   else
   {
      sprintf(voltage, "%04Lu", digito);
   
      if(sombreado==4)
      {
         glcd_rect(x, y, x+(tamano*16), y+(tamano*8), YES, OFF);
         glcd_text57(x+1, y+1, voltage, tamano, ON);
      }
      if(sombreado==1)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, voltage, tamano, OFF);}
      if(sombreado==0)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, voltage, tamano, ON);}
   }
   glcd_update();
}


void displayTemps(char palabra[30], char digito[3], int x, int y, int sombreado, int tamano)
{  
   if(sombreado==1)
   {
      //sprintf(voltage, "%03Lu", digito);
      glcd_rect(0, y, 127, y+(tamano*8), YES, ON);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);
      strcpy(Menus.Letra,digito);
      glcd_text57(x+90, y+1, Menus.Letra, tamano, OFF);
      //strcpy(Menus.Letra,"C");
      //glcd_text57(x+120, y+1, Menus.Letra, tamano, OFF);
      //glcd_circle(x+115,y-1,1,NO,OFF);
   }
   if(sombreado==0)
   {
      //sprintf(voltage, "%03Lu", digito);
      glcd_rect(0, y, 127, y+(tamano*8), YES, OFF);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);
      strcpy(Menus.Letra,digito);
      glcd_text57(x+90, y+1, Menus.Letra, tamano, ON);
      //strcpy(Menus.Letra,"C");
      //glcd_text57(x+120, y+1, Menus.Letra, tamano, ON);
      //glcd_circle(x+115,y-1,1,NO,ON);
   }
   glcd_update();
}

void displayFecha(int8 dia, int8 mes, int8 ano,int x, int y, int8 sombreado, int tamano){  
   char voltage[4];
   if(sombreado==1){
      sprintf(voltage, "%02u", dia); 
      glcd_rect(x-1, y-1, x+(tamano*12), y+(tamano*7), YES, ON);
      glcd_text57(x, y, voltage, tamano, OFF);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
      
      sprintf(voltage, "%02u", mes); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*30), y, Menus.Letra, tamano, ON);
      
      sprintf(voltage, "%02u", ano); 
      glcd_text57(x+(tamano*36), y, voltage, tamano, ON);
   }else if(sombreado==2){
      sprintf(voltage, "%02u", dia); 
      glcd_text57(x, y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
      
      glcd_rect(x-1+(tamano*18), y-1, x+(tamano*30), y+(tamano*7), YES, ON);
      sprintf(voltage, "%02u", mes); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, OFF);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*30), y, Menus.Letra, tamano, ON);
      
      sprintf(voltage, "%02u", ano); 
      glcd_text57(x+(tamano*36), y, voltage, tamano, ON);
   }else if(sombreado==3){
      sprintf(voltage, "%02u", dia); 
      glcd_text57(x, y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
            
      sprintf(voltage, "%02u", mes); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*30), y, Menus.Letra, tamano, ON);
     
      glcd_rect(x-1+(tamano*36), y-1, x+(tamano*48), y+(tamano*7), YES, ON);
      sprintf(voltage, "%02u", ano); 
      glcd_text57(x+(tamano*36), y, voltage, tamano, OFF);
   }else{
      glcd_rect(x, y-1, 127, y+(tamano*7), YES, OFF);
      sprintf(voltage, "%02u", dia); 
      glcd_text57(x, y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
            
      sprintf(voltage, "%02u", mes); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, ON);
      strcpy(Menus.Letra,"/");
      glcd_text57(x+(tamano*30), y, Menus.Letra, tamano, ON);
          
      sprintf(voltage, "%02u", ano); 
      glcd_text57(x+(tamano*36), y, voltage, tamano, ON);
   }
   glcd_update();
}

void displayHora(int8 hora, int8 minuto, int x, int y, int8 sombreado, int tamano){  
   char voltage[4];
   if(sombreado==4){
      sprintf(voltage, "%02u", hora); 
      glcd_rect(x-1, y-1, x+(tamano*12), y+(tamano*7), YES, ON);
      glcd_text57(x, y, voltage, tamano, OFF);
      strcpy(Menus.Letra,":");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
      
      sprintf(voltage, "%02u", minuto); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, ON);
   }else if(sombreado==5){
      sprintf(voltage, "%02u", hora); 
      glcd_text57(x, y, voltage, tamano, ON);
      strcpy(Menus.Letra,":");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
      
      glcd_rect(x-1+(tamano*18), y-1, x+(tamano*30), y+(tamano*7), YES, ON);
      sprintf(voltage, "%02u", minuto); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, OFF);
   }else{
      glcd_rect(x, y-1, 127, y+(tamano*7), YES, OFF);
      sprintf(voltage, "%02u", hora); 
      glcd_text57(x, y, voltage, tamano, ON);
      strcpy(Menus.Letra,":");
      glcd_text57(x+(tamano*12), y, Menus.Letra, tamano, ON);
            
      sprintf(voltage, "%02u", minuto); 
      glcd_text57(x+(tamano*18), y, voltage, tamano, ON);
   }
   glcd_update();
}

void displayTT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%03.1f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*8), YES, OFF);
   /*glcd_rect(x, y, x+53, y+15, YES, ON);*/glcd_text57(x, y, voltage, w, ON);
}

void Limpia(int x1,int x2,int y){
   glcd_rect(x1, y, x2, y+7, YES, OFF);
}

void displayT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%03.0f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*8), YES, OFF);
   /*glcd_rect(x, y, x+53, y+15, YES, ON);*/glcd_text57(x, y, voltage, w, ON);
}

void displayContrasena(int digito, int x, int y, int sombreado)
{
   char voltage[9];
   sprintf(voltage, "%i", digito);
   
   if(sombreado==1)
      {glcd_rect(x, y, x+18, y+24, YES, ON);glcd_text57(x+2, y+1, voltage, 2, OFF);}
   else   
      {glcd_rect(x, y, x+18, y+24, NO, OFF);glcd_text57(x+2, y+1, voltage, 2, ON);}

}

void displayMenu(char palabra[30], int x, int y, int sombreado, int tamano)
{
   strcpy(Menus.Letra,palabra);
   if(sombreado==2)
      {glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   if(sombreado==1)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);}
   if(sombreado==0)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   glcd_update();
}

void displayTiempo(int digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==3){
      sprintf(voltage, "%02u", digito); 
      glcd_rect(x, y, x+(tamano*12), y+(tamano*7), YES, OFF);
      glcd_text57(x+1, y+1, voltage, tamano, ON);
   }else if(sombreado==2){
      sprintf(voltage, "%03u", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }else{
      sprintf(voltage, "%u", digito);
   
      if(sombreado==1)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, voltage, tamano, OFF);}
      if(sombreado==0)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, voltage, tamano, ON);}
   }
   glcd_update();
}

void glcd_imagen(int8 x)
{
   char i,j;
   signed char k; 
   
   if(x==5){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(Proceso[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
}

void CuadroCalentando(short estado){
   if(estado){
      glcd_rect(15, 18, 30, 53, NO, ON);
      flagCalentando=OFF;
   }else{
      if(!flagCalentando){
         glcd_rect(15, 18, 30, 53, NO, OFF);
         flagCalentando=ON;
      }
   }
}

void CuadroEsterilizando(short estado){
   if(estado){
      glcd_rect(25, 18, 60, 26, NO, ON);
      flagEsterilizando=OFF;
   }else{
      if(!flagEsterilizando){
         glcd_rect(25, 18, 60, 26, NO, OFF);
         flagEsterilizando=ON;
      }
   }
}

void CuadroDesfogando(short estado){
   if(estado){
      glcd_rect(55, 18, 68, 53, NO, ON);
      flagDesfogando=OFF;
   }else{
      if(!flagDesfogando){
         glcd_rect(55, 18, 68, 53, NO, OFF);
         flagDesfogando=ON;
      }
   }
}

void BorraStatus(){
   glcd_rect(0, 55, 127, 63, YES, OFF);
}

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
}
float LeerPT100(int media)
{
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/Gain; // Se elimina la ganancia
   Pt=(R1)/((1024.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-RPT100; //Ajuste de impedancia de cableado de PT100
   //Temperature=(Pt-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
            
   if(l>(media-1))
   {l=0;}
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
   V1 = sensores(2);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
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
   V2 = sensores(3);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
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
   if(tiemporecta>=Derivativo/2.0)
   {
      ActualizaRecta();
      tiemporecta=0;
   } 
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
   
   if(error>1.8 && error<15 && Aumento<0.3)
   {
      if(Tpulso<4.0)
      {
         flagTC=1;
         if(TiempoControl>=Derivativo)
         {
            TiempoControl=0;
            if(Aumento<-0.5){
               Ganancia+=Integral+0.2;
            }else{
               if(Aumento<-0.1){
                  Ganancia+=Integral-0.2;
               }else{
                  Ganancia+=Integral-0.5;
               }
            }
         }
      }else{
         flagTC=0;
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
                                      
   Tp=(Tpulso/0.005);
   Tc=(Tciclo/0.005);
   Controlando=1;   
//--------------------------------------------------------------------------------------------------------------------//   
}

void Envio_I2C(direccion, posicion, dato){

   i2c_start();            // Comienzo comunicación
   i2c_write(direccion);   // Dirección del esclavo en el bus I2C
   i2c_write(posicion);    // Posición donde se guardara el dato transmitido
   i2c_write(dato);        // Dato a transmitir
   i2c_stop();             // Fin comunicación
 }

void Lectura_I2C (byte direccion, byte posicion, byte &dato) {

   i2c_start();            // Comienzo de la comunicación
   i2c_write(direccion);   // Dirección del esclavo en el bus I2C
   i2c_write(posicion);    // Posición de donde se leerá el dato en el esclavo
   i2c_start();            // Reinicio
   i2c_write(direccion+1); // Dirección del esclavo en modo lectura
   dato=i2c_read(0);       // Lectura del dato
   i2c_stop();             // Fin comunicación
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
   for(i=0;i<13;i++)
   {
      direccion=0xA0;
      Envio_I2C(direccion,i,SlaveA0Tx[i]);
   } 
}
void Lectura_Esclavos(void){
   for(i=0;i<18;i++)
   {
      direccion=0xA0;                        //Dirección en el bus I2c
      posicion=i;                         //Posición de memoria a leer
      Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
      txbuf[i]=(int8)dato;
   }
   Lee_Vector();
}

void LeeEEPROM(){
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
   CiclosL=read_eeprom(37);
   delay_ms(10);
   CiclosH=read_eeprom(38);
   delay_ms(10);
   Ciclos=make16(CiclosH,CiclosL);
   delay_ms(10);
   TTrampa=read_eeprom(39);
   delay_ms(10);
}

void LimitaValores(){
   if(Testerilizacionp[0]>9 || Testerilizacionp[0]<0){
      Testerilizacionp[0]=0;
      write_eeprom(0,0);
      delay_ms(10);
   }
   if(Testerilizacionp[1]>9 || Testerilizacionp[1]<0){
      Testerilizacionp[1]=0;
      write_eeprom(1,0);
      delay_ms(10);
   }
   if(Testerilizacionp[2]>9 || Testerilizacionp[2]<0){
      Testerilizacionp[2]=0;
      write_eeprom(2,0);
      delay_ms(10);
   }
   if(Testerilizacionp[3]>9 || Testerilizacionp[3]<0){
      Testerilizacionp[3]=0;
      write_eeprom(3,0);
      delay_ms(10);
   }
   if(Tsecadop[0]>9 || Tsecadop[0]<0){
      Tsecadop[0]=0;
      write_eeprom(4,0);
      delay_ms(10);
   }
   if(Tsecadop[1]>9 || Tsecadop[1]<0){
      Tsecadop[1]=0;
      write_eeprom(5,0);
      delay_ms(10);
   }
   if(Tsecadop[2]>9 || Tsecadop[2]<0){
      Tsecadop[2]=0;
      write_eeprom(6,0);
      delay_ms(10);
   }
   if(Tsecadop[3]>9 || Tsecadop[3]<0){
      Tsecadop[3]=0;
      write_eeprom(7,0);
      delay_ms(10);
   }
   if(Setpoint>134){
      Setpoint=121;
      write_eeprom(8,121);
      delay_ms(10);
   }
   if(Nivel>6){
      Nivel=3;
      write_eeprom(9,3);
      delay_ms(10);
   }
   if(Password[0]>9 || Password[0]<0){
      Password[0]=0;
      write_eeprom(10,0);
      delay_ms(10);
   }
   if(Password[1]>9 || Password[1]<0){
      Password[1]=0;
      write_eeprom(11,0);
      delay_ms(10);
   }
   if(Password[2]>9 || Password[2]<0){
      Password[2]=0;
      write_eeprom(12,0);
      delay_ms(10);
   }
   if(Password[3]>9 || Password[3]<0){
      Password[3]=0;
      write_eeprom(13,0);
      delay_ms(10);
   }
   if(Ganancia>20.0){
      Ganancia=15.0;
      write_eeprom(20,15);
      delay_ms(10);
   }
   if(Ajuste>50){
      Ajuste=0;
      write_eeprom(21,0);
      delay_ms(10);
   }
   if(Ajusten>50){
      Ajusten=0;
      write_eeprom(22,0);
      delay_ms(10);
   }
   if(MediaMovil>80){
      MediaMovil=80;
      write_eeprom(24,80);
      delay_ms(10);
   }
   if(Tciclo>10){
      Tciclo=10;
      write_eeprom(25,10);
      delay_ms(10);
   }
   if(Integral>10.0){
      Integral=1.0;
      write_eeprom(26,1);
      delay_ms(10);
   }
   if(Derivativo>90.0){
      Derivativo=15.0;
      write_eeprom(27,15);
      delay_ms(10);
   }
   if(Gain>50){
      Gain=40.2;
      write_eeprom(28,40);
      delay_ms(10);
      write_eeprom(29,2);
      delay_ms(10);
   }
   if(R1>6400){
      R1=6200;
      write_eeprom(30,0x38);
      delay_ms(10);
      write_eeprom(31,0x18);
      delay_ms(10);
   }
   if(RPT100>2.0){
      RPT100=0.5;
      write_eeprom(32,0);
      delay_ms(10);
      write_eeprom(33,5);
      delay_ms(10);
   }
   if(Ciclos>60000){
      Ciclos=0;
      write_eeprom(37,0);
      delay_ms(10);
      write_eeprom(38,0);
      delay_ms(10);
   }
   if(TTrampa>100){
      TTrampa=80;
      write_eeprom(39,80);
      delay_ms(10);
   }
}

short PidePassword(int8 MenuAnt, int8 MenuActual, int8 MenuPost, int8 clave0,int8 clave1, int8 clave2,int8 clave3){
      if(n==0)
      {
         displayContrasena(Contrasena[0],20,30,1);displayContrasena(Contrasena[1],40,30,0);
         displayContrasena(Contrasena[2],60,30,0);displayContrasena(Contrasena[0],80,30,0);
         strcpy(Menus.Letra,"Clave");
         displayMenu(Menus.Letra,20,0,1,3);
         n=1;
      }
      
      if(UP){
         if(Flanco == 0){
            Flanco = 1;delay_ms(5);
            for(i=1;i<=4;i++){
               if(unidad==i){
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9){
                     Contrasena[i-1]=0;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }else{
         Flanco = 0;
      }
            
      if(DOWN){
         if(Flanco2 == 0){
            Flanco2 = 1;delay_ms(5);
            for(i=1;i<=4;i++){
               if(unidad==i){
                  Contrasena[i-1]--;
                  if(Contrasena[i-1]<0){
                     Contrasena[i-1]=9;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }      
         }     
      }else{
         Flanco2 = 0;
      }
            
      if(RIGHT){
         if(Flanco1 == 0){
            Flanco1 = 1;delay_ms(5);unidad++;
            for(i=1;i<=4;i++){
               if(unidad==i){
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }else{
         Flanco1 = 0;
      }
      
      if(LEFT){
         delay_ms(100);
         if(LEFT){
            Menu=MenuAnt;glcd_fillScreen(OFF);n=0;Opcion=1;unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            glcd_fillScreen(OFF);glcd_update();
            return 1;
         }
      }
    
      if(unidad>4){
         glcd_fillScreen(OFF);unidad=1;
         
         if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2 && MenuActual==0){ // Si Ingresa clave para reset general del sistema.
            write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
            write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
            reset_cpu();
         }
      
         if((Contrasena[0]==clave0)&&(Contrasena[1]==clave1)&&(Contrasena[2]==clave2)&&(Contrasena[3]==clave3)){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=MenuPost;glcd_update();delay_ms(1000);glcd_fillScreen(OFF);
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            if(MenuActual==0)n=0;
            return 1;
         }else{
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            unidad=1;glcd_update();delay_ms(1000);Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;glcd_fillScreen(OFF);
            glcd_rect(0, 0, 127, 25, YES, ON);strcpy(Menus.Letra,"Clave");glcd_text57(25, 1, Menus.Letra, 3, OFF);displayContrasena(Contrasena[0],20,30,1);
            displayContrasena(Contrasena[1],40,30,0);displayContrasena(Contrasena[2],60,30,0);displayContrasena(Contrasena[0],80,30,0);
            Menu=MenuAnt;glcd_fillScreen(OFF);n=0;
            return 0;
         }
      }
}
