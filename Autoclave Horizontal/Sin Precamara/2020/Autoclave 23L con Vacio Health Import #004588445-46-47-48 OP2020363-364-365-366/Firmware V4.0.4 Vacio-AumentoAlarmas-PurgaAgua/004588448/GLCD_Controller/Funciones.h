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
            }
         }else{
            if(DiaTx>28){
               DiaTx=28;
            }
         }
      }else{
         if(MesTx<=7){
            if(MesTx % 2 ==0){
               if(DiaTx>30){
                  DiaTx=30;                   
               }
            }
         }else{
            if(MesTx % 2 ==1){
               if(DiaTx>30){
                  DiaTx=30; 
               }
            }
         }
      }
}

void ActualizaRecta(){
   X[y]=Temperature;
   y++;
   if(y>=2){
      y=0;
      Aumento=X[1]-X[0];
   }
}

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}

float LeerPT100(int8 canal,int media)
{
   Prom=0.0; 
   Sensor[canal].V0 = sensores(canal);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   Sensor[canal].V0 = Sensor[canal].V0/Sensor[canal].Gain; // Se elimina la ganancia
   Sensor[canal].V0 = Sensor[canal].V0+14.78; // Se suma voltaje de divisor fijo
   //Sensor[canal].BN=(Sensor[canal].V0*(R8+R9))+(1023*R9);
   Sensor[canal].Pt = R10/((1023/Sensor[canal].V0)-1);
   //Sensor[canal].Pt=Sensor[canal].Pt-Sensor[canal].RPT100; //Ajuste de impedancia de cableado de PT100   
            
   if(Sensor[canal].l>(media-1))
   {Sensor[canal].l=0;}
   
   Sensor[canal].promedio[Sensor[canal].l]=Sensor[canal].Pt;Sensor[canal].l++;
         
   for(Sensor[canal].h=0;Sensor[canal].h<media;Sensor[canal].h++)
   {
      Prom+=Sensor[canal].promedio[Sensor[canal].h];
   } 
   Prom=Prom/media;   
   //Temperature=(Prom-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.
   Sensor[canal].ajp=(float)Sensor[canal].Ajuste/10.0;
   Sensor[canal].ajn=(float)Sensor[canal].Ajusten/10.0;
   Temperature+=(Sensor[canal].ajp-Sensor[canal].ajn);
   
   return Temperature;
}

float Leer_Sensor_Presion_Camara(int media){
   promediopresion=0.0;
   V1 = sensores(5);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   //V1 = V1*(5.0/1024.0);
   //Presion=(V1-0.2)/(K);
   Presion=(((V1)/1023)-0.04)/K;
   
   if(Presion<0.0)
      Presion=0.0;
   
   if(r>media-1)
      r=0;
   PromPresion[r]=Presion;r++;
         
   for(t=0;t<=(media-1);t++)
      promediopresion+=PromPresion[t];
    
   promediopresion=promediopresion/media;   
   
   return promediopresion;
}

float Leer_Sensor_Presion_PreCamara(int media){
   promediopresionp=0.0;
   V2 = sensores(4);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   //V2 = V2*(5.0/1024.0);
   //Presionp=(V2-0.2)/(K);
   Presionp=(((V2)/1023)-0.04)/K;
   
   if(Presionp<0.0)
      Presionp=0.0;
   
   if(r22>media-1)
      r22=0;
   PromPresionp[r22]=Presionp;r22++;
         
   for(t2=0;t2<=(media-1);t2++)
      promediopresionp+=PromPresionp[t2]; 
      
   promediopresionp=promediopresionp/media;   
   
   return promediopresionp;
}

void Controla(){
   if(tiemporecta>=Derivativo/2.0){
      ActualizaRecta();
      tiemporecta=0;
   } 
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   error=desvio+Setpoint-Temperatura;      // C?lculo del error ******
   
   if(error>1.0 && error<25 && Aumento<0.3){
      if(Tpulso<7.0){
         flagTC=1;
         if(TiempoControl>=Derivativo){
            TiempoControl=0;
            if(Aumento<-0.5){
               Ganancia+=Integral+1;
            }else{
               if(Aumento<-0.1){
                  Ganancia+=Integral+0.5;
               }else{
                  Ganancia+=Integral;
               }
            }
         }
      }else{
         flagTC=0;
      }
   }
   
   if(error<-0.3)
      Ganancia=Ganancia2;
   
   if(Ganancia<1)
      Ganancia=1;
   
   if(error<0.0)     // Anti Wind_DOWN    
      error=0.0;
   //tmp=(Setpoint-Prom)*Ganancia;  // Control Proporcional.
   tmp=error*Ganancia;  // Control Proporcional.
         
   if(tmp>Setpoint)   // Anti Wind-UP      
      tmp=Setpoint;
               
   if(tmp<0.0)     // Anti Wind_DOWN    
      tmp=0.0;
   
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
   SlaveA0Tx[22]=make8(Ciclos,0);
   SlaveA0Tx[23]=make8(Ciclos,1);
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
   for(i=0;i<24;i++){
      direccion=0xB0;
      i2c_start();            // Comienzo comunicaci?n
      i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
      i2c_write(i);   // Direcci?n del esclavo en el bus I2C
      i2c_write(0);    // Posici?n donde se guardara el dato transmitido
      i2c_write(SlaveA0Tx[i]); // Dato a transmitir
      i2c_stop(); 
      //Envio_I2C(direccion,i,SlaveB0Tx[i]);
   } 
}
void Lectura_Esclavos(void){
   //for(i=0;i<7;i++)
   //{
   //   direccion=0xA0;                        //Direcci?n en el bus I2c
   //   posicion=i;                         //Posici?n de memoria a leer
   //   Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
   //   txbuf[i]=(int8)dato;
   //}
   //Lee_Vector();
}

int8 extraeDecimal(float data){
   float entero=0;
   float tmp=0.0;
   
   entero=(int8)data;
   tmp=(data-entero)*10;
   return (int8)tmp;
}

float inttofloat(int8 decimal){
   float tmp;
   tmp=decimal;
   return tmp/10;
}

void LeeEEPROM(){
//Lectura EEPROM//--------------------------
   Password[0]=read_eeprom(10);
   delay_ms(10);
   Password[1]=read_eeprom(11);
   delay_ms(10);
   Password[2]=read_eeprom(12);
   delay_ms(10);
   Password[3]=read_eeprom(13);
   delay_ms(10);
   Integral=read_eeprom(18);
   delay_ms(10);
   Derivativo=read_eeprom(19);
   delay_ms(10);
   Ganancia=read_eeprom(20);
   delay_ms(10);
   Sensor[0].Ajuste=read_eeprom(21);
   delay_ms(10);
   Sensor[0].Ajusten=read_eeprom(22);
   delay_ms(10);
   Sensor[1].Ajuste=read_eeprom(23);
   delay_ms(10);
   Sensor[1].Ajusten=read_eeprom(24);
   delay_ms(10);   
   MediaMovil=read_eeprom(25);
   delay_ms(10);
   Tciclo=read_eeprom(26);
   delay_ms(10);
   Temporal=read_eeprom(27);
   Temporal=Temporal/10.0;
   delay_ms(10);
   Sensor[0].RPT100=read_eeprom(28)+Temporal;
   delay_ms(10);
   Temporal=read_eeprom(29);
   Temporal=Temporal/10.0;
   delay_ms(10);
   Sensor[1].RPT100=read_eeprom(30)+Temporal;
   delay_ms(10);
   Atmosferica=read_eeprom(31);
   delay_ms(10);
   Tapertura=read_eeprom(32);
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
   if(G2>40)
      G2=40;
   sg1=read_eeprom(44);
   delay_ms(10);
   sg2=read_eeprom(45);
   delay_ms(10);
   NivelDesfogue=read_eeprom(46);
   delay_ms(10);
   pulsosConfigurados=read_eeprom(47);
   delay_ms(10);
   vacioTest=read_eeprom(48);
   delay_ms(10);
   vacioCiclo=read_eeprom(49);
   
   for(i=0;i<4;i++){//Lee valores almacenados de ciclos personalizados
      CiclosPersonalizados[i].Temperatura=read_eeprom(100+(i*8)); 
      CiclosPersonalizados[i].Minutoest=read_eeprom(101+(i*8)); 
      CiclosPersonalizados[i].Segundoest=read_eeprom(102+(i*8)); 
      CiclosPersonalizados[i].Minutosec=read_eeprom(103+(i*8)); 
      CiclosPersonalizados[i].Segundosec=read_eeprom(104+(i*8)); 
      CiclosPersonalizados[i].PulsosVacio=read_eeprom(105+(i*8)); 
      CiclosPersonalizados[i].NivelVacio=read_eeprom(106+(i*8)); 
      CiclosPersonalizados[i].NivelDesfogue=read_eeprom(107+(i*8)); 
      
      if(CiclosPersonalizados[i].Temperatura>134 || CiclosPersonalizados[i].Temperatura<105)
         CiclosPersonalizados[i].Temperatura=121;
      if(CiclosPersonalizados[i].Minutoest>59)
         CiclosPersonalizados[i].Minutoest=1;
      if(CiclosPersonalizados[i].Segundoest>59)
         CiclosPersonalizados[i].Segundoest=0;
      if(CiclosPersonalizados[i].Minutosec>59)
         CiclosPersonalizados[i].Minutosec=1;   
      if(CiclosPersonalizados[i].Segundoest>59)
         CiclosPersonalizados[i].Segundoest=0;
      if(CiclosPersonalizados[i].Segundosec>59)
         CiclosPersonalizados[i].Segundosec=0;      
      if(CiclosPersonalizados[i].PulsosVacio>8)
         CiclosPersonalizados[i].PulsosVacio=2;
      if(CiclosPersonalizados[i].NivelVacio>99 || CiclosPersonalizados[i].NivelVacio<10)
         CiclosPersonalizados[i].NivelVacio=25;   
      if(CiclosPersonalizados[i].NivelDesfogue>6)
         CiclosPersonalizados[i].NivelDesfogue=6;
   }
   
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
      if(ACiclos[i]>30)
         ACiclos[i]=0;
   }
}

void LimitaValores(){
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
   if(Integral>4.0 || Integral<1){
      Integral=1.0;
      write_eeprom(18,1);
      delay_ms(10);
   }
   if(Derivativo>40.0 || Derivativo<5){
      Derivativo=15.0;
      write_eeprom(19,15);
      delay_ms(10);
   }
   if(Ganancia>30.0 || Ganancia<1){
      Ganancia=30.0;
      write_eeprom(20,30);
      delay_ms(10);
   }
   if(Sensor[0].Ajuste>100){
      Sensor[0].Ajuste=0;
      write_eeprom(21,0);
      delay_ms(10);
   }
   if(Sensor[0].Ajusten>100){
      Sensor[0].Ajusten=0;
      write_eeprom(22,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajuste>100){
      Sensor[1].Ajuste=0;
      write_eeprom(23,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajusten>100){
      Sensor[1].Ajusten=0;
      write_eeprom(24,0);
      delay_ms(10);
   }
   if(MediaMovil>80 || MediaMovil<10){
      MediaMovil=40;
      write_eeprom(25,40);
      delay_ms(10);
   }
   if(Tciclo>10 || Tciclo<5){
      Tciclo=10;
      write_eeprom(26,10);
      delay_ms(10);
   }
   if(Sensor[0].RPT100>2.0){
      Sensor[0].RPT100=0.5;
      write_eeprom(28,0);
      delay_ms(10);
      write_eeprom(27,5);
      delay_ms(10);
   }
   if(Sensor[1].RPT100>2.0){
      Sensor[1].RPT100=0.5;
      write_eeprom(30,0);
      delay_ms(10);
      write_eeprom(29,5);
      delay_ms(10);
   }
   if(Atmosferica>100 || Atmosferica<60){
      Atmosferica=75;
      write_eeprom(31,69);
      delay_ms(10);
   }
   if(Tapertura>100 || Tapertura<70){
      Tapertura=85;
      write_eeprom(32,85);
      delay_ms(10);
   }
   if(Ciclos>60000){
      Ciclos=0;
      write_eeprom(37,0);
      delay_ms(10);
      write_eeprom(38,0);
      delay_ms(10);
   }
   if(sg1==10){
      G1=G1*(-1);
   }
   if(sg2==10){
      G2=G2*(-1);
   }
   if(TTrampa>100){
      TTrampa=90;
      write_eeprom(39,90);
      delay_ms(10);
   }
   if(NivelDesfogue>6){
      NivelDesfogue=3;
      write_eeprom(46,3);
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
   if(pulsosConfigurados>8 || pulsosConfigurados<2){
      pulsosConfigurados=3;
      write_eeprom(47,3);
      delay_ms(10);
   }
   if(vacioTest>30 || vacioTest<17){
      vacioTest=18;
      write_eeprom(48,18);
      delay_ms(10);
   }
   if(vacioCiclo>30 || vacioCiclo<17){
      vacioCiclo=25;
      write_eeprom(49,25);
      delay_ms(10);
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
         
      if(RX_Buffer[3]==0x01){// Pregunta por la pagina en la que esta el display,01 es Contrase?a de Acceso
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
      }else if(RX_Buffer[3]==0x0a){//0a es Pulsos de Vacio
         Menu=9;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0d){//0d es Ciclos
         Menu=12;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0f){//0f es Recibe caracteres de contrase?a desde display
         
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
   }else if(Tipo==3){
      flagAlarma=1;
      if(tbuzzer<400){   
         Buzzer_on;
      }else if(tbuzzer>400 && tbuzzer<2000){
         Buzzer_off;
      }else if(tbuzzer>2000){
         Buzzer_off;
         tbuzzer=0;
      }
   }
}

void ControlaPresion(){
   if(Setpoint<125){
      if(PresionPreCamara<180){
         SSR_on;
      }else if(PresionPreCamara>205){
         SSR_off;
         if(!Precalentamiento && guardaCiclo){
            Precalentamiento=ON;
            if(DOOR){
               Paso_on;
               delay_ms(1500);
               Paso_off;
            }
         }
      }
   }else{
      if(PresionPreCamara<280){
         SSR_on;
      }else if(PresionPreCamara>300){
         SSR_off;
         if(!Precalentamiento && guardaCiclo){
            Precalentamiento=ON;
            if(DOOR){
               Paso_on;
               delay_ms(1500);
               Paso_off;
            }
         }
      }
   }
}

void ApagaSalidas(){
   Desfogue_off;
   Agua_off;
   Buzzer_off;
   SSR_off;
   EV8_off;
   Agua_off;
   Paso_off;
   SSR2_off;
   Aire_off;
   AireB_off;
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

void GuardaDatoEstatico(){
   int16 posCiclo=0,cicloMemoria=0;
   posCiclo=Ciclos/200;
   cicloMemoria=Ciclos-(posCiclo*200);
   if(!disp_Guarda){
      write_ext_eeprom(cicloMemoria*320,1);
      delay_ms(10);
      if(!Libre)
         write_ext_eeprom(1+(cicloMemoria*320),CicloSeleccionado);
      else
         write_ext_eeprom(1+(cicloMemoria*320),CicloLibre+6);
      delay_ms(10);
      write_ext_eeprom(2+(cicloMemoria*320),Dia);
      delay_ms(10);
      write_ext_eeprom(3+(cicloMemoria*320),Mes);
      delay_ms(10);
      write_ext_eeprom(4+(cicloMemoria*320),Year);
      delay_ms(10); 
      disp_Guarda=1;
   }   
}

void GuardaDatoCiclo(){
   int16 posCiclo=0,cicloMemoria=0;
   posCiclo=Ciclos/200;
   if(!Ciclo){
      cicloMemoria=Ciclos-(posCiclo*200);
      write_ext_eeprom(5+(cicloMemoria*320)+(posicionDato*9),(int8)Temperatura);
      delay_ms(10);
      write_ext_eeprom(6+(cicloMemoria*320)+(posicionDato*9),extraeDecimal(Temperatura));
      delay_ms(10);
      write_ext_eeprom(7+(cicloMemoria*320)+(posicionDato*9),make8(PresionCamara,1));
      delay_ms(10);
      write_ext_eeprom(8+(cicloMemoria*320)+(posicionDato*9),make8(PresionCamara,0));
      delay_ms(10);
      write_ext_eeprom(9+(cicloMemoria*320)+(posicionDato*9),make8(PresionPreCamara,1));
      delay_ms(10);
      write_ext_eeprom(10+(cicloMemoria*320)+(posicionDato*9),make8(PresionPreCamara,0));
      delay_ms(10);
      write_ext_eeprom(11+(cicloMemoria*320)+(posicionDato*9),Hora);
      delay_ms(10);
      write_ext_eeprom(12+(cicloMemoria*320)+(posicionDato*9),Minuto);
      delay_ms(10);
      write_ext_eeprom(13+(cicloMemoria*320)+(posicionDato*9),codigoAlarma);
      delay_ms(10);
   
      if(posicionDato<35)
         ++posicionDato;
   }
}

void borraDatoCiclo(){
   int16 posCiclo=0,cicloMemoria=0;
   posCiclo=Ciclos/200;
   cicloMemoria=Ciclos-(posCiclo*200);
   write_ext_eeprom(cicloMemoria*320,1);
   delay_ms(1);
   write_ext_eeprom(1+(cicloMemoria*320),0);
   delay_ms(1);
   write_ext_eeprom(2+(cicloMemoria*320),0);
   delay_ms(1);
   write_ext_eeprom(3+(cicloMemoria*320),0);
   delay_ms(1);
   write_ext_eeprom(4+(cicloMemoria*320),0);
   delay_ms(1); 
   for(posicionDato=0;posicionDato<35;posicionDato++){
      write_ext_eeprom(5+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(6+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(7+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(8+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(9+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(10+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(11+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(12+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
      write_ext_eeprom(13+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(1);
   }
   posicionDato=0;
}

void incrementaCiclos(){
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
}

void exportarData(){
int16 posicion=0;
int8 tmp=0;
posicion=40;
direccion=0xB0;
//Buzzer_on;
//Inicio de Transmision
i2c_start();            // Comienzo comunicaci?n
i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
i2c_write(0xff);        // Dato a transmitir
i2c_stop();
++posicion;
i2c_start();            // Comienzo comunicaci?n
i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
i2c_write(0xff);        // Dato a transmitir
i2c_stop();
++posicion;
//Se envian 2 veces ff para indicar que se inicia la transmision.

   for(i=0;i<5;i++){
      tmp=read_ext_eeprom((cicloVisto*320)+i);
      //if(tmp>250)
      //   tmp=20;
      i2c_start();            // Comienzo comunicaci?n
      i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
      i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
      i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
      i2c_write(tmp);        // Dato a transmitir
      i2c_stop(); 
      ++posicion;
   }
   for(i=0;i<35;i++){//Lee Valores para completar la Tabla
      for(j=5;j<14;j++){//Lee Valores para completar la Tabla
         tmp=read_ext_eeprom((cicloVisto*320)+j+(i*9));
         //if(tmp>250)
         //   tmp=20;
         i2c_start();            // Comienzo comunicaci?n
         i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
         i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
         i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
         i2c_write(tmp);        // Dato a transmitir
         i2c_stop(); 
         ++posicion;
      }
   }

//Fin de Transmision
i2c_start();            // Comienzo comunicaci?n
i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
i2c_write(0x1f);        // Dato a transmitir
i2c_stop();
++posicion;
i2c_start();            // Comienzo comunicaci?n
i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posici?n donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posici?n donde se guardara el dato transmitido
i2c_write(0x1f);        // Dato a transmitir
i2c_stop();
++posicion;
//Se envian 2 veces 1f para indicar que se inicia la transmision.
delay_ms(200);
//Buzzer_off;
}

void codificaAlarma(){
   if(ACiclos[i]==0){
      printf("A%u.txt=\"----\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==1){
      printf("A%u.txt=\"Parada\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==2){
      printf("A%u.txt=\"Puerta\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==3){
      printf("A%u.txt=\"Termostato\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==4){
      printf("A%u.txt=\"SobreT\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==5){
      printf("A%u.txt=\"Tiempo\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==6){
      printf("A%u.txt=\"Presion\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==7){
      printf("A%u.txt=\"Bomba\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==8){
      printf("A%u.txt=\"Err. Vacio\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==9){
      printf("A%u.txt=\"Llenado\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==10){
      printf("A%u.txt=\"Llenando\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==11){
      printf("A%u.txt=\"Vacio\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==12){
      printf("A%u.txt=\"Calen\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==13){
      printf("A%u.txt=\"Desfoga\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==14){
      printf("A%u.txt=\"Finalizado\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==15){
      printf("A%u.txt=\"Secando\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==16){
      printf("A%u.txt=\"Esterilizando\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==17){
      printf("A%u.txt=\"Enfriando\"",i+1);
      SendDataDisplay();
   }
}

void graficaCurva(int16 tGrafica){
   if(tiempos2>=tGrafica){
      Grafica=1;
      tiempos2=0;
   }
}

void muestreo(int16 tmuestreo){
   if(tiempos3>=tmuestreo){
      Temperatura=LeerPt100(0,MediaMovil);
      Temperatura2=LeerPt100(1,MediaMovil);
      PresionCamara=Leer_Sensor_Presion_Camara(10);
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(5);
      rtc_get_date(Dia,Mes,Year,dow);
      rtc_get_time(Hora,Minuto,Segundo);
      if(sensores(3)<512)
         LOW=ON;
      else
         LOW=OFF;
      
      if(sensores(2)<512)//3
         HIG=ON;
      else
         HIG=OFF;
      tiempos3=0;
   }
}

void monitoreaStop(){
   if(Menu==255){
      printf("t2.txt=\"Activada P:%03LdkPa T:%2.1f?C\"",presionManometrica,Temperatura);
      SendDataDisplay();
      ActivaAlarma(1);
   }
   
   if(STOP && Menu!=20 && !PruebaEstados){
      delay_ms(200);
      if(STOP){
         if(!disp_Stop){
            disp_Stop=1;Menu=255;
            printf("page Emergencia");
            SendDataDisplay();
         }
         Controlando=0;
         Desfogue_on;
         SSR_off;
         SSR2_off;
         Paso_off;
         BombaVacio_off;
         Vacio_off;
         if(PresionCamara<Atmosferica)
            Aire_on;
         else
            Aire_off;
         AireB_off;
         //Agua_off;
         //EV8_off;
         DesfogueSuave=OFF;
         //ActivaAlarma(1);
         BorraBuffer();
      }
   }
   /*
   else{
      if(disp_Stop){
         reset_cpu();
      }
   } 
   */
}

void chancleteaBomba(){
   for(i=0;i<10;i++){
      BombaVacio_on;
      delay_ms(100);
      BombaVacio_off;
      delay_ms(100);
   }
}

void purgaBomba(){
   Vacio_off;
   Desfogue_off;
   Aire_off;
   AireB_on;
   Paso_off;
   //Agua_off;
   delay_ms(70);
   chancleteaBomba();
   BombaVacio_on;
   delay_ms(7000);
}

void salidasPulsoNegativo(){
   Vacio_on;
   Desfogue_off;
   Aire_off;
   AireB_off;
   Paso_off;
   BombaVacio_on;
}

void salidasPulsoPositivo(){
   Vacio_off;
   Desfogue_off;
   Aire_off;
   AireB_on;
   Paso_on;
   Agua_off;
   EV8_off;
   BombaVacio_on; 
}

void enciendeBombaAgua(){
   if(tBombaAgua>120){
      EV8_off;
      flagBombaAgua=OFF;
      flagBombaAguaOff=ON;
      if(tBombaAguaOff>=60){
         tBombaAgua=0;
      }
   }else{
      EV8_on;
      flagBombaAgua=ON;
      flagBombaAguaOff=OFF;
      tBombaAguaOff=0;
   }
}

void apagaBombaAgua(){
   EV8_off;
   flagBombaAgua=OFF;
   flagBombaAguaOff=OFF;
   tBombaAguaOff=0;
   tBombaAgua=0;
}

void actualizaTabla(){
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

void monitoreaNivelAlto(){
   if(LRES){
      printf("Funcionamiento.t4f.txt=\" Ninguna\"");
      SendDataDisplay();
      if(HIG){
         SensadoNivel=ON;
         if(TiempoSensadoNivel>=3){
            SensadoNivel=OFF;
            CalderinLleno=ON;
            IniciaLlenado=OFF;                  
            Agua_off;
            //EV8_off;
            apagaBombaAgua();
         }
     }else{
        SensadoNivel=OFF;
        TiempoSensadoNivel=0;
        Agua_on;
        //EV8_on;
        enciendeBombaAgua();
     }  
  }else{
      printf("Funcionamiento.t4f.txt=\" Reservorio Vacio\"");
      SendDataDisplay();
      Agua_off;
      //EV8_off;
      apagaBombaAgua();
  }
}

void borraVariables(){
   guardaCiclo=OFF;
   CalderinLleno=OFF;
   Ciclo=OFF;
   flag_pulso=0;
   codigoAlarma=0;
   Controlando=0;
   EstadoBuzzer=OFF;
   Precalentamiento=OFF;
   IniciaVacio=OFF;
   IniciaVacio2=OFF;
   Vacio=OFF;
   flag_vac=OFF;
   PulsoPositivo=10;
   PulsosVacio=0;
   tvacio=0;
   Esterilizando=OFF;
   Desfogando=OFF;
   mEst=0;
   sEst=0;
   Secando=OFF;
   DesfogueSuave=OFF;
   Secando=OFF;
   mSec=0;
   sSec=0;
   Control=0;
   Finalizo=OFF;
   AlarmaLlenado=OFF;
   AlarmaSobreTemperatura=OFF;
   AlarmaTiempoProlongado=OFF;
   AlarmaPresion=OFF;
   AlarmaEmergencia=OFF;
   AlarmaVacio=OFF;
   AlarmaBomba=OFF;
   AlarmaTermostato=OFF;
   TiempoLlenado=0;
   TiempoCiclo=0;      
   tBomba=0;
   tvacio=0;
   IniciaCiclo=OFF;
   flagImprimir=0;
   flagImprime=0;
   tiempoImpresion=0;
   ConteoBuzzer=0;
   tbuzzer=0;
   EstadoAgua2=OFF;
   entro=OFF;
}

void monitoreaAgua(){ 
   if(!guardaCiclo && CalderinLleno){
      ControlaPresion();
      Agua_off;
      apagaBombaAgua();
   }else if(!guardaCiclo && !CalderinLleno){
      if(LRES){
         Agua_on;
         enciendeBombaAgua();
      }else{
         Agua_off;
         apagaBombaAgua();
      }
   }
   
   if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
      SensadoNivelL=ON; 
      Controlando=0;   
   }else{
      NivelBajo=ON;
      tNivelBajo=0;
      SensadoNivelL=OFF;
   }
  
   if(tNivelBajo>3){
      SensadoNivelL=OFF;
      NivelBajo=OFF;
      TiempoSensadoNivel=0;
      SensadoNivel=OFF;
      if(!Esterilizando)
         CalderinLleno=OFF;
   }
  
   if(NivelBajo){
      if(HIG){
         SensadoNivel=ON;
         if(TiempoSensadoNivel>=3){
            SensadoNivel=OFF;
            CalderinLleno=ON;
            IniciaLlenado=OFF;                  
         }
      }else{
         SensadoNivel=OFF;
         TiempoSensadoNivel=0;
      }
   }  
}

void revisePresion(){
   if(DOOR){
         if(PresionCamara>=Atmosferica+22){
            ActivaAlarma(3);
            if(flagBuzzer){
               printf("fecha.txt=\"Pc:%03Ld kPa Pp:%03Lu kPa T:%2.1f?C Hora:%02u:%02u:%02u\"",presionManometrica,PresionPrecamara,Temperatura,Hora,Minuto,Segundo);
               SendDataDisplay();
            }else{
               printf("fecha.txt=\"!AUTOCLAVE PRESURIZADA, NO ABRA LA PUERTA!\"");
               SendDataDisplay();
            }
         }else if(PresionCamara>Atmosferica+3 && PresionCamara<Atmosferica+22){
            ActivaAlarma(3);
            if(flagBuzzer){
               printf("fecha.txt=\"Pc:%03Ld kPa Pp:%03Lu kPa T:%2.1f?C Hora:%02u:%02u:%02u\"",presionManometrica,PresionPrecamara,Temperatura,Hora,Minuto,Segundo);
               SendDataDisplay();
            }else{
               printf("fecha.txt=\"!AUTOCLAVE PRESURIZADA, ABRA CON PRECAUCION!\"");
               SendDataDisplay();
            }
         }else if(PresionCamara<= Atmosferica+3){   
            if(LRES){
               printf("fecha.txt=\"Pc:%03Ld kPa Pp:%03Lu kPa T:%2.1f?C Hora:%02u:%02u:%02u\"",presionManometrica,PresionPrecamara,Temperatura,Hora,Minuto,Segundo);
               SendDataDisplay();
               Buzzer_off;
            }else{
               printf("fecha.txt=\"!Aplique agua en el reservorio!\"");
               SendDataDisplay();
               Buzzer_off;
            }
         }
      }else{
         if(LRES){
            printf("fecha.txt=\"Pc:%03Ld kPa Pp:%03Lu kPa T:%2.1f?C Hora:%02u:%02u:%02u\"",presionManometrica,PresionPrecamara,Temperatura,Hora,Minuto,Segundo);
            SendDataDisplay();
            Buzzer_off;
         }else{
            printf("fecha.txt=\"!Aplique agua en el reservorio!\"");
            SendDataDisplay();
            Buzzer_off;
         }
      }
}

void ConfiguraCiclo(int8 segest,minest,segsec,minsec,liquidos,ciclo,temperatura,pulsos,nvacio,fug){
   sEstp=segest;
   mEstp=minest;
   sSecp=segsec;
   mSecp=minsec;
   Setpoint=temperatura;               
   tiempo_secado=0;
   desfoguelento=liquidos;
   CicloSeleccionado=ciclo;
   sEst=sEstp;
   mEst=mEstp;
   sSec=sSecp;
   mSec=mSecp;
   CantidadPulsos=pulsos;
   NivelVacio=nvacio;
   Fugas=fug;
   if(!borra){
      borraDatoCiclo();
      borra=ON;
   }
   //TiempoVacio=CantidadPulsos*180;
   Imprime=15;
   Envio_Esclavos();
   delay_ms(100);
   Imprime=10;
   //Ganancia=Ganancia*(Setpoint/134);
}

void purgaAguaTanque(){
   if(PresionCamara>Atmosferica+30 && !purgo){
      purgaAgua=ON;
   }else{
      purgaAgua=OFF;
      Desfogue_off;
   }
   
   if(purgaAgua){
      if(tPurgaAgua>0 && tPurgaAgua<11){
         Desfogue_on;
      }else if(tPurgaAgua>=11 && tPurgaAgua<131){
         Desfogue_off;
      }else if(tPurgaAgua>=131 && tPurgaAgua<141){
         Desfogue_on;
      }else if(tPurgaAgua>=141 && tPurgaAgua<201){
         Desfogue_off;
      }else if(tPurgaAgua>=201 && tPurgaAgua<211){
         Desfogue_on;
      }else if(tPurgaAgua>=211 && tPurgaAgua<271){
         Desfogue_off;
      }else if(tPurgaAgua>=271 && tPurgaAgua<281){
         Desfogue_on;
      }else if(tPurgaAgua>0){
         Desfogue_off;
         purgaAgua=OFF;
         purgo=ON;
      }
   }

}
