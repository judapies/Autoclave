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
   Sensor[canal].V0 = Sensor[canal].V0+14.83; // Se suma voltaje de divisor fijo
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
   Temperature+=(((float)Sensor[canal].Ajuste)/10.0-((float)Sensor[canal].Ajusten)/10.0);
   
   return Temperature;
}

float Leer_Sensor_Presion_Camara(int8 media){
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

float Leer_Sensor_Presion_PreCamara(int8 media){
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
   float prc=0.0;   
   /*
   // Control por temperatura
   if(Temperatura>Setpoint+0.3){
      suministroVapor=OFF;
      Paso=ON;
   }else if(Temperatura<=Setpoint+0.2){
      suministroVapor=ON;
      Paso=ON;
   }
   ////////////////////////////
   */
   //Control por presi?n
   prc=(float)PresionCamara;
   Tactual=((prc)*(prc)*(prc)*p1)+((prc)*(prc)*p2)+(p3*prc)+p4;
   if(Tactual>Setpoint+desvio){
      suministroVapor=OFF;
      Paso=ON;
   }else if(Tactual<=Setpoint+desvio-Histeresis){
      suministroVapor=ON;
      Paso=ON;
   }
   ////////////////////////////////////
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
   SlaveB0Tx[0]=(int8)Temperatura;
   SlaveB0Tx[1]=(Temperatura-((int8)Temperatura))*10;
   SlaveB0Tx[2]=make8((int16)PresionCamara,0);
   SlaveB0Tx[3]=make8((int16)PresionCamara,1);
   SlaveB0Tx[4]=make8((int16)PresionPreCamara,0);
   SlaveB0Tx[5]=make8((int16)PresionPreCamara,1);
   SlaveB0Tx[6]=Setpoint;
   SlaveB0Tx[7]=seguroValvula;
   SlaveB0Tx[8]=seguroValvula2;
   SlaveB0Tx[9]=mEst;
   SlaveB0Tx[10]=sEst;
   SlaveB0Tx[11]=mSec;
   SlaveB0Tx[12]=sSec;
   SlaveB0Tx[13]=CicloSeleccionado;
   SlaveB0Tx[14]=SetTime;
   SlaveB0Tx[15]=Dia;
   SlaveB0Tx[16]=Mes;
   SlaveB0Tx[17]=Year;
   SlaveB0Tx[18]=Hora;
   SlaveB0Tx[19]=Minuto;
   SlaveB0Tx[20]=Segundo;
   SlaveB0Tx[21]=Imprime;
   SlaveB0Tx[22]=make8(Ciclos,0);
   SlaveB0Tx[23]=make8(Ciclos,1);
   SlaveB0Tx[24]=EstadoBuzzer;
   SlaveB0Tx[25]=suministroAire;
   SlaveB0Tx[26]=AguaEnfriador;
   SlaveB0Tx[27]=BombaVacio;
   SlaveB0Tx[28]=empaqueLimpio;
   SlaveB0Tx[29]=empaqueSucio;
   SlaveB0Tx[30]=cilindroSucio;
   SlaveB0Tx[31]=cilindroLimpio;
   SlaveB0Tx[32]=suministroVapor;
   SlaveB0Tx[33]=Paso;
   SlaveB0Tx[34]=outVacio;
   SlaveB0Tx[35]=Desfogue;
   SlaveB0Tx[36]=Aire;
   SlaveB0Tx[37]=Aux;
   SlaveB0Tx[38]=codigoAlarma;
   SlaveB0Tx[39]=codigoUsuario;
   
   SlaveC0Tx[0]=(int8)Temperatura;
   SlaveC0Tx[1]=(Temperatura-((int8)Temperatura))*10;
   SlaveC0Tx[2]=make8((int16)PresionCamara,0);
   SlaveC0Tx[3]=make8((int16)PresionCamara,1);
   SlaveC0Tx[4]=make8((int16)PresionPreCamara,0);
   SlaveC0Tx[5]=make8((int16)PresionPreCamara,1);
   SlaveC0Tx[6]=(int8)Tpulso*10;
   //SlaveC0Tx[7]=HIG;
   //SlaveC0Tx[8]=LOW;
   //SlaveC0Tx[9]=TERM;
   //SlaveC0Tx[10]=DOOR;
   SlaveC0Tx[11]=STOP;
   SlaveC0Tx[12]=(int8)error;
   SlaveC0Tx[13]=CicloSeleccionado;
   SlaveC0Tx[14]=SetTime;
   SlaveC0Tx[15]=Dia;
   SlaveC0Tx[16]=Mes;
   SlaveC0Tx[17]=Year;
   SlaveC0Tx[18]=Hora;
   SlaveC0Tx[19]=Minuto;
   SlaveC0Tx[20]=Segundo;
   SlaveC0Tx[21]=mEst;
   SlaveC0Tx[22]=sEst;
   SlaveC0Tx[23]=mSec;
   SlaveC0Tx[24]=sSec;
   SlaveC0Tx[25]=Menu;
   SlaveC0Tx[26]=Setpoint;
   SlaveC0Tx[27]=codigoAlarma;
   SlaveC0Tx[28]=255;
   SlaveC0Tx[29]=128;
   SlaveC0Tx[30]=50;
}

void Lee_Vector(void){
   //Dia=txbuf[0];   
}

void Envio_Esclavos(void){
   Carga_Vector();
   for(i=0;i<40;i++){
      direccion=0xB0;
      i2c_start();            // Comienzo comunicaci?n
      i2c_write(direccion);   // Direcci?n del esclavo en el bus I2C
      i2c_write(i);   // Direcci?n del esclavo en el bus I2C
      i2c_write(0);    // Posici?n donde se guardara el dato transmitido
      i2c_write(SlaveB0Tx[i]); // Dato a transmitir
      i2c_stop(); 
      //Envio_I2C(direccion,i,SlaveB0Tx[i]);
   } 
   
   for(i=0;i<31;i++){
      direccion=0xC0;
      Envio_I2C(direccion,i,SlaveC0Tx[i]);
   }
}
void Lectura_Esclavos(void){
   for(i=0;i<9;i++){
      direccion=0xB0;                        //Direcci?n en el bus I2c
      posicion=i;                         //Posici?n de memoria a leer
      Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
      txbuf[i]=(int8)dato;
   }
   Lectura_I2C(0XC0, 0, dato);    //Lectura por I2C
   SlaveC0Rx[0]=(int8)dato;
   subirPuertaLimpio=SlaveC0Rx[0];
   Lee_Vector();
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
   G1=read_eeprom(42);
   delay_ms(10);
   G2=read_eeprom(43);
   delay_ms(10);
   sg1=read_eeprom(44);
   delay_ms(10);
   sg2=read_eeprom(45);
   delay_ms(10);
   cerroSucioInt=read_eeprom(46);
   delay_ms(10);
   cerroLimpioInt=read_eeprom(47);
   delay_ms(10);
   pulsosConfigurados=read_eeprom(50);
   delay_ms(10);
   vacioTest=read_eeprom(51);
   delay_ms(10);
   vacioCiclo=read_eeprom(52);
   delay_ms(10);
   histe=read_eeprom(53);
   delay_ms(10);
   Histeresis=(float)histe;
   Histeresis=Histeresis/10.0;
   desviacion=read_eeprom(54);
   delay_ms(10);
   desvio=(float)desviacion;
   desvio=desvio/10.0;
   
   for(i=0;i<30;i++){
      estados[i]=read_eeprom(i+60);
      if(estados[i]>1)
         estados[i]=1;
   }
   
   for(i=0;i<20;i++){//Lee valores almacenados de ciclos personalizados
      CiclosPersonalizados[i].Temperatura=read_eeprom(100+(i*8)); 
      CiclosPersonalizados[i].Minutoest=read_eeprom(101+(i*8)); 
      CiclosPersonalizados[i].Segundoest=read_eeprom(102+(i*8)); 
      CiclosPersonalizados[i].Minutosec=read_eeprom(103+(i*8)); 
      CiclosPersonalizados[i].Segundosec=read_eeprom(104+(i*8)); 
      CiclosPersonalizados[i].PulsosVacio=read_eeprom(105+(i*8)); 
      CiclosPersonalizados[i].NivelVacio=read_eeprom(106+(i*8)); 
      CiclosPersonalizados[i].NivelDesfogue=read_eeprom(107+(i*8)); 
      
      if(CiclosPersonalizados[i].Temperatura>138 || CiclosPersonalizados[i].Temperatura<110)
         CiclosPersonalizados[i].Temperatura=121;
      if(CiclosPersonalizados[i].Minutoest>99 || CiclosPersonalizados[i].Minutoest<1)
         CiclosPersonalizados[i].Minutoest=3;
      if(CiclosPersonalizados[i].Minutosec>99 || CiclosPersonalizados[i].Minutosec<1)
         CiclosPersonalizados[i].Minutosec=1;   
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
   
   for(i=0;i<10;i++){//Lee contrase?as de usuarios
      for(j=0;j<4;j++){
         Clave[i].Password[j]=read_eeprom((300+j)+(i*4));
         if(Clave[i].Password[j]>9)
            Clave[i].Password[j]=0;
      }
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
      if(ACiclos[i]>10)
         ACiclos[i]=0;
   }
}

void LimitaValores(){
   if(Ganancia>20.0){
      Ganancia=15.0;
      write_eeprom(20,15);
      delay_ms(10);
   }
   if(Sensor[0].Ajuste>20){
      Sensor[0].Ajuste=0;
      write_eeprom(21,0);
      delay_ms(10);
   }
   if(Sensor[0].Ajusten>20){
      Sensor[0].Ajusten=0;
      write_eeprom(22,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajuste>20){
      Sensor[1].Ajuste=0;
      write_eeprom(23,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajusten>20){
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
   if(Atmosferica>100 || Atmosferica<60){
      Atmosferica=72;
      write_eeprom(31,72);
      delay_ms(10);
   }
   if(Tapertura>100 || Tapertura<80){
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
   if(cerroSucioInt>1){
      cerroSucioInt=0;
      write_eeprom(46,0);
      delay_ms(10);
      cerroSucio=OFF;
   }
   if(cerroLimpioInt>1){
      cerroLimpioInt=0;
      write_eeprom(47,0);
      delay_ms(10);
      cerroLimpio=OFF;
   }
   if(cerroSucioInt==1)
      cerroSucio=ON;
   if(cerroSucioInt==0)
      cerroSucio=OFF;   
   if(cerroLimpioInt==1)
      cerroLimpio=ON;
   if(cerroLimpioInt==0)
      cerroLimpio=OFF;  
      
   if(pulsosConfigurados>8 || pulsosConfigurados<2){
      pulsosConfigurados=4;
      write_eeprom(50,4);
      delay_ms(10);
   }
   if(vacioTest>30 || vacioTest<17){
      vacioTest=17;
      write_eeprom(51,17);
      delay_ms(10);
   }
   if(vacioCiclo>40 || vacioCiclo<17){
      vacioCiclo=25;
      write_eeprom(52,25);
      delay_ms(10);
   }    
   if(desvio>2.0){
      desvio=0.5;write_eeprom(53,5);delay_ms(10);
   }
   if(Histeresis>3.0){
      Histeresis=0.1;write_eeprom(54,1);delay_ms(10);
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
      }else if(RX_Buffer[3]==0x02){//02 es Ciclos Fijos
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
      }else if(RX_Buffer[3]==0x06){//06 es Nivel de Vacio
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
      }else if(RX_Buffer[3]==0x0b){//0b es Usuarios
         Menu=10;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0c){//0c es Configuraciones
         Menu=11;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0d){//0d es Ciclos
         Menu=12;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0e){//0d es Activacion de Ciclos
         Menu=14;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x10){//10 es Nivel de Desfogue
         Menu=13;
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
   }
}

void ActivaAlarma(int8 Tipo){
   
   if(tsilencio>12000){
      tsilencio=0;
      flagSilencio=OFF;
   }
   
   if(Tipo==1){ // Alarma por Error
      
      if(!flagSilencio){
         flagAlarma=1;
         if(tbuzzer>200){
            tbuzzer=0;
            flagBuzzer=!flagBuzzer;
            if(flagBuzzer)
               EstadoBuzzer=ON;
            else
               EstadoBuzzer=OFF;
         }
      }else{
         EstadoBuzzer=OFF;
      }
   }else if(Tipo==2){ 
      flagAlarma=1;
      if(ConteoBuzzer<20){   
         if(tbuzzer>100){
            tbuzzer=0;
            flagBuzzer=!flagBuzzer;
            if(flagBuzzer)
               EstadoBuzzer=ON;
            else
               EstadoBuzzer=OFF;
               
            ConteoBuzzer++;   
         }
      }else{
         EstadoBuzzer=OFF;
      }
   }
}

void ControlaPresion(){
   if(Setpoint>125){
      if(PresionPrecamara>=(300)){
         suministroVapor=OFF;
         Precalentamiento=ON;
      }
      if(PresionPrecamara<(290))
         suministroVapor=ON;
   }else{
      if(PresionPrecamara>=(230)){
         suministroVapor=OFF;
         Precalentamiento=ON;
      }
      if(PresionPrecamara<(220))
         suministroVapor=ON;
   }
}

void GuardaDatoEstatico(){
   int16 posCiclo=0,cicloMemoria=0;
   posCiclo=Ciclos/200;
   cicloMemoria=Ciclos-(posCiclo*200);
   if(!disp_Guarda){
      write_ext_eeprom(cicloMemoria*320,codigoUsuario);
      delay_ms(10);
      if(!Libre)
         write_ext_eeprom(1+(cicloMemoria*320),CicloSeleccionado);
      else
         write_ext_eeprom(1+(cicloMemoria*320),CicloLibre+10);
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

void borraDatoCiclo(){
   int16 posCiclo=0,cicloMemoria=0;
   posCiclo=Ciclos/200;
   cicloMemoria=Ciclos-(posCiclo*200);
   write_ext_eeprom(cicloMemoria*320,1);
   delay_ms(5);
   write_ext_eeprom(1+(cicloMemoria*320),0);
   delay_ms(5);
   write_ext_eeprom(2+(cicloMemoria*320),0);
   delay_ms(5);
   write_ext_eeprom(3+(cicloMemoria*320),0);
   delay_ms(5);
   write_ext_eeprom(4+(cicloMemoria*320),0);
   delay_ms(5);
   for(posicionDato=0;posicionDato<35;posicionDato++){
      write_ext_eeprom(5+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(6+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(7+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(8+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(9+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(10+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(11+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(12+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
      write_ext_eeprom(13+(cicloMemoria*320)+(posicionDato*9),0);
      delay_ms(5);
   }
   posicionDato=0;
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
      printf("A%Lu.txt=\"----\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==1){
      printf("A%Lu.txt=\"Parada\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==2){
      printf("A%Lu.txt=\"Puerta\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==3){
      printf("A%Lu.txt=\"Termostato\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==4){
      printf("A%Lu.txt=\"SobreT\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==5){
      printf("A%Lu.txt=\"Tiempo\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==6){
      printf("A%Lu.txt=\"Presion\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==7){
      printf("A%Lu.txt=\"Bomba\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==8){
      printf("A%Lu.txt=\"ErrorVacio\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==10){
      printf("A%Lu.txt=\"PreCale\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==11){
      printf("A%Lu.txt=\"Vacio\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==12){
      printf("A%Lu.txt=\"Calen\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==13){
      printf("A%Lu.txt=\"Desfoga\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==14){
      printf("A%Lu.txt=\"Finalizado\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==15){
      printf("A%Lu.txt=\"Secando\"",i+1);
      SendDataDisplay();
   }else if(ACiclos[i]==16){
      printf("A%Lu.txt=\"Esterilizando\"",i+1);
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
      PresionPreCamara=Leer_Sensor_Presion_PreCamara(20);
      tiempos3=0;
   }
}

void monitoreaAgua(){
   if(LOWB){
      SensadoNivelR=ON;
      if(TiempoSensadoNivelR>2){
         SensadoNivelR=OFF;
         ReservorioLleno=ON;
      }
   }else{
      SensadoNivelR=OFF;
      TiempoSensadoNivelR=0;
      ReservorioLleno=OFF;
   } 
}

void monitoreaStop(){
      if(!STOP && !IniciaCiclo && !PruebaEstados){
         if(!disp_Stop){
            disp_Stop=1;Menu=255;
         }
         codigoAlarma=200;
         Controlando=0;
         Desfogue=ON;
         Paso=OFF;
         //Suministro_off;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=ON;
         outVacio=OFF;   
         aguaEnfriador=OFF;
         bombaVacio=OFF;
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
}

void cierraPuertaS(){// A?adir logica de puertas
   abrirPuertaS=OFF;
   tVacioEmpaqueS=0;
   if(!BLOQUEOS){
      cilindroSucio=OFF;
      estadoCilindrosS=ON;
      if(DOORS){
         empaqueSucio=ON;
         //cerroSucio=ON;
      }else{
         empaqueSucio=OFF;
      }
   }else{
      if(!DOORS){
         cilindroSucio=ON;
         estadoCilindrosS=OFF;
         pulsoSubirS=OFF;
      }
   }
   suministroAire=ON;
   seguroValvula=OFF;
   seguroValvula2=OFF;
   estadoSuministro=ON;
}

void abrePuertaS(){
   abrirPuertaS=ON;
   if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10){
      empaqueSucio=OFF;
      seguroValvula=ON;
      seguroValvula2=OFF;
      if(DOORS){
         if(tVacioEmpaqueS>=10){
            cilindroSucio=ON;
            cerroSucio=OFF;
            vacioEmpaqueS=OFF;
            BombaVacio=OFF;
         }else{
            BombaVacio=ON;
            vacioEmpaqueS=ON;
         }
      }else{
         cilindroSucio=ON;
         estadoCilindrosS=OFF;
         cerroSucio=OFF;
         vacioEmpaqueS=OFF;
         BombaVacio=OFF;
         pulsoBajarS=OFF;
      }
   }
   suministroAire=ON;
   estadoSuministro=ON;
}

void cierraPuertaL(){// A?adir logica de puertas
   abrirPuertaL=OFF;
   tVacioEmpaqueL=0;
   if(!BLOQUEOL){
      cilindroLimpio=OFF;
      estadoCilindrosL=ON;
      if(DOORL){
         empaqueLimpio=ON;
         //cerroLimpio=ON;
      }else{
         empaqueLimpio=OFF;
      }
   }else{
      if(!DOORL){
         cilindroLimpio=ON;
         estadoCilindrosL=OFF;
         pulsoSubirL=OFF;
      }
   }
   seguroValvula2=OFF;
   seguroValvula=OFF;
   suministroAire=ON;
   estadoSuministro=ON;
}

void abrePuertaL(){
   abrirPuertaL=ON;
   if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10){
      empaqueLimpio=OFF;
      seguroValvula2=ON;
      seguroValvula=OFF;
      if(DOORL){
         if(tVacioEmpaqueL>=10){
            cilindroLimpio=ON;
            cerroLimpio=OFF;
            vacioEmpaqueL=OFF;
            BombaVacio=OFF;
         }else{
            BombaVacio=ON;
            vacioEmpaqueL=ON;
         }
      }else{
         cilindroLimpio=ON;
         estadoCilindrosL=OFF;
         cerroLimpio=OFF;
         vacioEmpaqueL=OFF;
         BombaVacio=OFF;
         //pulsoBajarL=OFF;
      }
   }
   suministroAire=ON;
   estadoSuministro=ON;
}

void monitoreaPuertaLimpio(){
   if(pulsoSubirL)
      cierraPuertaL();
      
   if(subirPuertaLimpio==5){//Oprimio Subir Puerta Limpio
      //cierraPuertaL();
      pulsoSubirL=ON;
   }else if(subirPuertaLimpio==10){//Solto Subir Puerta Limpio
      pulsoSubirL=OFF;
      if(!DOORL && !EMPAQUEL)
         abrePuertaL();
      else
         cierraPuertaL();
   }else if(subirPuertaLimpio==15){//Oprimio Bajar Puerta Limpio
      pulsoSubirL=OFF;
      if(Temperatura<Tapertura && Finalizo && cerroSucio)
      //if(Temperatura<Tapertura && cerroSucio)
         abrePuertaL();
   }else{
      if(!DOORL)
         abrePuertaL();
      else
         cierraPuertaL();
   }  
}

void ApagaSalidas(){
   
   Controlando=0;
}

void borraVariables(){
   borra=OFF;
   disp_Guarda=0;
   posicionDato=0;
   Ciclo=OFF;
   flag_pulso=0;
   Start=OFF;
   codigoAlarma=0;
   Desfogue=OFF;
   suministroVapor=OFF;
   BombaVacio=OFF;
   Aire=OFF;
   aguaEnfriador=OFF;
   outVacio=OFF;
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
   AlarmaEmpaqueS=OFF;
   AlarmaEmpaqueL=OFF;
   AlarmaVacio=OFF;
   AlarmaBomba=OFF;
   AlarmaTermostato=OFF;
   TiempoLlenado=0;
   TiempoCiclo=0;
   tEmpaqueS=0;
   tEmpaqueL=0;
   tBomba=0;
   tvacio=0;
   IniciaCiclo=OFF;
   flagImprimir=0;
   flagImprime=0;
   tiempoImpresion=0;
   ingreso=OFF;
   ConteoBuzzer=0;
   tbuzzer=0;
   tEnfriador=0;
   EstadoAgua2=OFF;
}

void leePulsadores(){
   if(tIniciar>=2){
      Start=!Start;
      oprimioIniciar=OFF;
      tIniciar=0;
   }
   
   if(tCiclof>=3){
      //printf("page Menu");
      printf("page Usuarios");
      SendDataDisplay();
      oprimioCiclo=OFF;      
      borraVariables();
   }
   
   if(tCicloP>=3){
      //printf("page CicloPersona");
      printf("page Usuarios");
      SendDataDisplay();
      oprimioCicloP=OFF;
      borraVariables();
   }
   
   if(tConfig>=3){
      //printf("page Ajustes");
      printf("page Usuarios");
      SendDataDisplay();
      oprimioConfig=OFF;
      borraVariables();
   }
   
   if(RX_Buffer[4]==0x10){//Oprimio Iniciar Ciclo
      oprimioIniciar=ON;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x11){//Solto Iniciar Ciclo
      oprimioIniciar=OFF;
      tIniciar=0;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x20){//Oprimio Seleccionar Ciclo
      if(!Start || Finalizo)
         oprimioCiclo=ON;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x21){//Solto Seleccionar Ciclo
      oprimioCiclo=OFF;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   /*
   if(RX_Buffer[4]==0x30){//Oprimio Ciclos Libres
      if(!Start || Finalizo)
         oprimioCicloP=ON;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x31){//Solto Ciclos Libres
      oprimioCicloP=OFF;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x40){//Oprimio Configuraciones
      if(!Start || Finalizo)
         oprimioConfig=ON;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   
   if(RX_Buffer[4]==0x41){//Solto Configuraciones
      oprimioConfig=OFF;
      RX_Buffer[4]=0x00;
      //RX_Buffer2[4]=0x00;                       
   }
   */
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
   //TiempoVacio=CantidadPulsos*180;
   if(!borra){
      borraDatoCiclo();
      borra=ON;
   }
   Imprime=15;
   Envio_Esclavos();
   delay_ms(10);
   Imprime=10;
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

