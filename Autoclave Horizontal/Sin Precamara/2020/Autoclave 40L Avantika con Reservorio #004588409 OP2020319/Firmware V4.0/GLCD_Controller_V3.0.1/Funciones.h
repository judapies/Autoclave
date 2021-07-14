void BorraBuffer(void){
RX_Buffer[0]=0;
RX_Buffer[1]=0;
RX_Buffer[2]=0;
RX_Buffer[3]=0;
RX_Buffer[4]=0;
RX_Buffer[5]=0;
RX_Buffer[6]=0;
RX_Buffer[7]=0;
RX_Buffer[8]=0;
RX_Buffer[9]=0;

RX_Buffer2[0]=0;
RX_Buffer2[1]=0;
RX_Buffer2[2]=0;
RX_Buffer2[3]=0;
RX_Buffer2[4]=0;
RX_Buffer2[5]=0;
RX_Buffer2[6]=0;
RX_Buffer2[7]=0;
RX_Buffer2[8]=0;
RX_Buffer2[9]=0;

}

char bgetc(void){
   char c;
   while(RX_Counter==0)
      ;
   c=Rx_Buffer2[RX_Rd_Index];
   if(++RX_Rd_Index>RX_BUFFER_SIZE)
      RX_Rd_Index=0;
   if(RX_Counter)
      RX_Counter--;
   return c;
}

void bputc(char c){
   char restart=0;
   while(TX_Counter> (TX_BUFFER_SIZE-1))
      ;
   if(TX_Counter==0)
      restart=1;
   TX_Buffer[TX_Wr_Index++]=c;
   if(TX_Wr_Index>TX_BUFFER_SIZE)
      TX_Wr_Index=0;
   
   TX_Counter++;
   
   if(restart==1)
      enable_interrupts(int_tbe);
}

void SendDataDisplay(void){
delay_us(10);
putc(0xFF);
delay_us(10);
putc(0xFF);
delay_us(10);
putc(0xFF);
delay_us(10);
}

short esBisiesto(int8 year) {
     return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

void LimitaDia(){
   if(MesTx>12)
      MesTx=10;
   if(DiaTx>31)
      DiaTx=16;
   if(YearTx>99)
      YearTx=18;   
   if(HoraTx>24)
      HoraTx=11;      
   if(MinutoTx>59)
      MinutoTx=30;      
   
      if(MesTx==2){
         if(esBisiesto(YearTx)){
            if(DiaTx>29){
               DiaTx=29;
               //glcd_fillScreen(OFF);
            }
         }else{
            if(DiaTx>28){
               DiaTx=28;
               //glcd_fillScreen(OFF);
            }
         }
      }else{
         if(MesTx<=7){
            if(MesTx % 2 ==0){
               if(DiaTx>30){
                  DiaTx=30; 
                  //glcd_fillScreen(OFF);
               }
            }
         }else{
            if(MesTx % 2 ==1){
               if(DiaTx>30){
                  DiaTx=30; 
                  //glcd_fillScreen(OFF);
               }
            }
         }
      }
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
   V0 = V0/108.57; // Se elimina la ganancia
   //V0 += 14.82;
   V0 += 14.93;
   Pt=R10/((1023/V0)-1);
            
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
   //Temperature-=39.0;
   
   //return Prom;
   return Temperature;
}

float Leer_Sensor_Presion_Camara(int media){
   promediopresion=0.0;
   V1 = sensores(2);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   //V1 = V1*(5.0/1024.0);
   //Presion=(V1-0.2)/(K);
   Presion=(((V1)/1023)-0.04)/K;
   
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
   //V2 = V2*(5.0/1024.0);
   //Presionp=(V2-0.2)/(K);
   Presionp=(((V2)/1023)-0.04)/K;
   
   if(Presionp<0.0)
   {
      Presionp=0.0;
   }
   
   if(r22>media-1)
   {r22=0;}
   PromPresionp[r22]=Presionp;r22++;
         
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
   
   if(error>1.0 && error<15 && Aumento<0.3)
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
   tmp2=(tmp/Setpoint);
   //if(tmp2>0.8)
   //tmp2=0.8;
   Tpulso=tmp2*Tciclo;  
                                      
   Tp=(Tpulso/0.005);
   Tc=(Tciclo/0.005);
   Controlando=1;   
//--------------------------------------------------------------------------------------------------------------------//   
}

void ControlaPresion(){
   float prc=0.0;   
   if(tiemporecta>=Derivativo/2.0)
   {
      ActualizaRecta();
      tiemporecta=0;
   } 
   prc=(float)PresionCamara;
   Tactual=((prc)*(prc)*(prc)*p1)+((prc)*(prc)*p2)+(p3*prc)+p4;
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   if(Esterilizando){
      error=desvio+Setpoint-Tactual;      // Cálculo del error ******
   }else{
      error=desvio+1.5+Setpoint-Tactual;      // Cálculo del error ******
   }
   
   if(error>1.0 && error<15 && Aumento<0.3)
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
   tmp2=(tmp/Setpoint);
   //if(tmp2>0.8)
   //tmp2=0.8;
   Tpulso=tmp2*Tciclo;  
                                      
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
   SlaveA0Tx[7]=HIG;
   SlaveA0Tx[8]=LOW;
   SlaveA0Tx[9]=TERM;
   SlaveA0Tx[10]=DOOR;
   SlaveA0Tx[11]=STOP;
   SlaveA0Tx[12]=(int8)error;
   SlaveA0Tx[13]=CicloSeleccionado;
   SlaveA0Tx[14]=SetTime;
   SlaveA0Tx[15]=Dia;
   SlaveA0Tx[16]=Mes;
   SlaveA0Tx[17]=Year;
   SlaveA0Tx[18]=Hora;
   SlaveA0Tx[19]=Minuto;
   SlaveA0Tx[20]=Segundo;
   SlaveA0Tx[21]=Imprime;
   SlaveA0Tx[22]=Aire;
   SlaveA0Tx[23]=make8(Ciclos,0);
   SlaveA0Tx[24]=make8(Ciclos,1);
}

void Lee_Vector(void){
   /*Dia=txbuf[0];
   Mess=txbuf[1];
   Year=txbuf[2];
   Hora=txbuf[3];
   Minuto=txbuf[4];
   Segundo=txbuf[5];
   dow=txbuf[6];*/
}

void Envio_Esclavos(void){
   Carga_Vector();
   for(i=0;i<25;i++)
   {
      direccion=0xB0;
      Envio_I2C(direccion,i,SlaveA0Tx[i]);
   } 
}
void Lectura_Esclavos(void){
   //for(i=0;i<7;i++)
   //{
   //   direccion=0xA0;                        //Dirección en el bus I2c
   //   posicion=i;                         //Posición de memoria a leer
   //   Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
   //   txbuf[i]=(int8)dato;
   //}
   //Lee_Vector();
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
   ActivaImpresion=read_eeprom(40);
   delay_ms(10);
   timpresion=read_eeprom(41);
   delay_ms(10);
   G1=read_eeprom(42);
   delay_ms(10);
   G2=read_eeprom(43);
   delay_ms(10);
   sg1=read_eeprom(44);
   delay_ms(10);
   sg2=read_eeprom(45);
   delay_ms(10);
   Atmosferica=read_eeprom(46);
   
   for(i=0;i<4;i++){//Lee valores almacenados de ciclos personalizados
      CiclosPersonalizados[i].Temperatura=read_eeprom(100+(i*8)); 
      CiclosPersonalizados[i].MinutoestH=read_eeprom(101+(i*8)); 
      CiclosPersonalizados[i].Minutoest=read_eeprom(102+(i*8)); 
      CiclosPersonalizados[i].Segundoest=read_eeprom(103+(i*8)); 
      CiclosPersonalizados[i].Minutosec=read_eeprom(104+(i*8)); 
      CiclosPersonalizados[i].Segundosec=read_eeprom(105+(i*8)); 
      CiclosPersonalizados[i].PulsosVacio=read_eeprom(106+(i*8)); 
      CiclosPersonalizados[i].NivelDesfogue=read_eeprom(107+(i*8)); 
      
      if(CiclosPersonalizados[i].Temperatura>134 || CiclosPersonalizados[i].Temperatura<105)
         CiclosPersonalizados[i].Temperatura=121;
      if(CiclosPersonalizados[i].Minutoest>59)
         CiclosPersonalizados[i].Minutoest=0;
      if(CiclosPersonalizados[i].Segundoest>59)
         CiclosPersonalizados[i].Segundoest=0;
      if(CiclosPersonalizados[i].Minutosec>59)
         CiclosPersonalizados[i].Minutosec=0;   
      if(CiclosPersonalizados[i].Segundoest>59)
         CiclosPersonalizados[i].Segundoest=0;
      if(CiclosPersonalizados[i].Segundosec>59)
         CiclosPersonalizados[i].Segundosec=0;      
      if(CiclosPersonalizados[i].PulsosVacio>15)
         CiclosPersonalizados[i].PulsosVacio=4;
      if(CiclosPersonalizados[i].NivelVacio>100 || CiclosPersonalizados[i].NivelVacio<10)
         CiclosPersonalizados[i].NivelVacio=25;   
      if(CiclosPersonalizados[i].NivelDesfogue>6)
         CiclosPersonalizados[i].NivelDesfogue=6;
   }
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
      Setpoint=134;
      write_eeprom(8,134);
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
   if(Ganancia>12.0){
      Ganancia=12.0;
      write_eeprom(20,12);
      delay_ms(10);
   }
   if(Ajuste>10){
      Ajuste=0;
      write_eeprom(21,0);
      delay_ms(10);
   }
   if(Ajusten>10){
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
   if(Integral>2.0){
      Integral=1.0;
      write_eeprom(26,1);
      delay_ms(10);
   }
   if(Derivativo>20.0){
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
   if(ActivaImpresion>1){
      ActivaImpresion=1;
      write_eeprom(40,1);
      delay_ms(10);
   }
   if(timpresion>30){
      timpresion=2;
      write_eeprom(41,2);
      delay_ms(10);
   }
   if(Atmosferica>100 || Atmosferica<50){
      Atmosferica=72;
      write_eeprom(46,72);
      delay_ms(10);
   }
   if(sg1==10){
      G1=G1*(-1);
   }
   if(sg2==10){
      G2=G2*(-1);
   }
}

void LeeDisplay(void){
   if(Dato_Exitoso==5){
         RX_Buffer[0]=RX_Buffer2[0];
         RX_Buffer[1]=RX_Buffer2[1];
         RX_Buffer[2]=RX_Buffer2[2];
         RX_Buffer[3]=RX_Buffer2[3];
         RX_Buffer[4]=RX_Buffer2[4];
         RX_Buffer[5]=RX_Buffer2[5];
         RX_Buffer[6]=RX_Buffer2[6];
         RX_Buffer[7]=RX_Buffer2[7];
         RX_Buffer[8]=RX_Buffer2[8];
         RX_Buffer[9]=RX_Buffer2[9];
         
      if(RX_Buffer[3]==0x01){// Pregunta por la pagina en la que esta el display,01 es Contraseña de Acceso
         Menu=0;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x02){//02 es Menu Principal con PantallaPriincipal=0, y con PantallaPrincipal=1 es Ciclo Libre
         Menu=1;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x03){//03 es Tiempo Esterilizacion
         Menu=2;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x04){//04 es Tiempo de Secado
         Menu=3;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x05){//05 es Temperatura
         Menu=4;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x06){//06 es Nivel
         Menu=5;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x07){//07 es Test de Componentes
         Menu=6;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x08){//08 es Fecha y hora
         Menu=7;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x09){//09 es Ciclos Personalizados
         Menu=8;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0f){//0f es Recibe caracteres de contraseña desde display
         
      }else if(RX_Buffer[3]==0x1a){//1a es Menu de Funcionamiento
         Menu=20;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x1b){//1b es Menu de clave correcta
         Menu=15;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x1c){//1c es Menu de clave incorrecta
         Menu=16;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x2c){//2c es Menu de Configuracion de Parametros
         Menu=100;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0xcc){//cc es Menu de Bienvenida
         Menu=240;
         if(MenuAntt!=240)
            reset_cpu();
      }
   }else{
      for(z=0;z<RX_BUFFER_SIZE;z++){
            //Rx_Buffer[z]=0;
            //Rx_Buffer2[z]=0;
         }
   }
}

void ConfiguraCiclo(int8 este0,este1,este2,este3,secado0,secado1,secado2,secado3,liquidos,ciclo,temperatura){
   Testerilizacionp[0]=este0;Testerilizacionp[1]=este1;Testerilizacionp[2]=este2;Testerilizacionp[3]=este3;
   Tsecadop[0]=secado0;Tsecadop[1]=secado1;Tsecadop[2]=secado2;Tsecadop[3]=secado3;
   Setpoint=temperatura;               
   tiempo_secado=0;
   desfoguelento=liquidos;
   CicloSeleccionado=ciclo;
   Testerilizacion[0]=Testerilizacionp[0];Testerilizacion[1]=Testerilizacionp[1];Testerilizacion[2]=Testerilizacionp[2];
   Testerilizacion[3]=Testerilizacionp[3];Testerilizacion[4]=Testerilizacionp[4];
   Tsecado[0]=Tsecadop[0];Tsecado[1]=Tsecadop[1];Tsecado[2]=Tsecadop[2];Tsecado[3]=Tsecadop[3];
   Imprime=15;
   Envio_Esclavos();
   delay_ms(100);
   Imprime=10;
}

void ActivaAlarma(int8 Tipo){
   
   if(tsilencio>12000){
      tsilencio=0;
      flagSilencio=OFF;
   }
   
   if(Tipo==1){ // Alarma por Error
   if(UP || DOWN || RIGHT || LEFT)
      flagSilencio=ON;
      
      if(!flagSilencio){
         flagAlarma=1;
         if(tbuzzer>200){
            tbuzzer=0;
            flagBuzzer=!flagBuzzer;
            if(flagBuzzer)
               Buzzer_on;
            else
               Buzzer_off;
         }
      }else{
         Buzzer_off;
      }
   }else if(Tipo==2){
   if(UP || DOWN || RIGHT || LEFT)
      ConteoBuzzer=21;
      
      flagAlarma=1;
      if(ConteoBuzzer<20){   
         if(tbuzzer>100){
            tbuzzer=0;
            flagBuzzer=!flagBuzzer;
            if(flagBuzzer)
               Buzzer_on;
            else
               Buzzer_off;
               
            ConteoBuzzer++;   
         }
      }else{
         Buzzer_off;
      }
   }
}

void CalientaChaqueta(){
   /*if(PresionPrecamara<130){
      SSR_on;
   }else if(PresionPreCamara>150){
      SSR_off;
   }
   */                                 
   Tp=200;
   Tc=(Tciclo/0.005);
   Controlando=1; 
}

void ApagaSalidas(){
   Desfogue_off;
   Bomba_off;
   Buzzer_off;
   SSR_off;
   Aire=OFF;
   Agua_off;
   Controlando=0;
}

void purgaAire(){
   if(!purga){
      for(i=0;i<4;i++){
         Desfogue_on;
         delay_ms(2000);
         Desfogue_off;
         delay_ms(1000);
      }
      purga=ON;
   }
}

int8 extraeDecima(int8 tiempo){
   int8 tmp=0,decima=0;
   tmp=tiempo/10;
   tmp=tmp*10;
   decima=tiempo-tmp;
   return decima;
}
