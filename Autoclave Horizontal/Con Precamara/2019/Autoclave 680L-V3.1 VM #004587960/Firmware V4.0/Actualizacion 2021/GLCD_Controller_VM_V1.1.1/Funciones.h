
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

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}

float LeerPT100(int8 canal,int media)
{
   float pa=-0.00036062,pb=0.30947,pc=72.816;
   if(PresionCamara<140){
      Prom=0.0;
      Sensor[canal].V0 = sensores(canal);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
      Sensor[canal].V0 = Sensor[canal].V0/Sensor[canal].Gain; // Se elimina la ganancia
      Sensor[canal].BN=(Sensor[canal].V0*(R8+R9))+(1023*R9);
      Sensor[canal].Pt=(R10/((AN/Sensor[canal].BN)-1))-R7;
      Sensor[canal].Pt=Sensor[canal].Pt-Sensor[canal].RPT100; //Ajuste de impedancia de cableado de PT100   
               
      if(Sensor[canal].l>(media-1))
      {Sensor[canal].l=0;}
      
      Sensor[canal].promedio[Sensor[canal].l]=Sensor[canal].Pt;Sensor[canal].l++;
            
      for(Sensor[canal].h=0;Sensor[canal].h<media;Sensor[canal].h++)
      {
         Prom+=Sensor[canal].promedio[Sensor[canal].h];
      } 
      Prom=Prom/media;   
      //Temperature=(Prom-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
      Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.
      Temperature+=((float)Sensor[canal].Ajuste-(float)Sensor[canal].Ajusten);
      
      return Temperature;
   }else{
      Temperature=(PresionCamara*PresionCamara*pa)+(PresionCamara*pb)+pc;
      return Temperature;
   }
}

float Leer_Sensor_Presion_Camara(int media){
   promediopresion=0.0;
   V1 = sensores(2);
   Presion=(0.06364*V1)-12.5; //R=240, V=5; Sensor de Presion Absoluta de 0-50PSIA
   Presion=Presion*6.89476;
   
   if(Presion<0.0)
      Presion=0.0;
   
   if(r>media-1)
      r=0;
   PromPresion[r]=Presion;r++;
         
   for(t=0;t<=(media-1);t++){
      promediopresion+=PromPresion[t];
   }
   
   promediopresion=promediopresion/media;   
   
   return promediopresion;   
}

float Leer_Sensor_Presion_PreCamara(int media){
   promediopresionp=0.0;
   V2 = sensores(3);
   //Presionp=(0.073842*V2)-14.504; //R=240, V=5; Sensor de Presion Manometrica de 0-58PSIG
   if(V2<1010){
      Presionp=(0.06364*V2)-12.5; //R=240, V=5; Sensor de Presion Manometrica de 0-50PSIA
      Presionp=Presionp*6.89476;
   }else{
      Presionp=0.0;
   }
   
   if(Presionp<0.0)
      Presionp=0.0;
   
   if(r22>media-1)
      r22=0;
   PromPresionp[r22]=Presionp;r22++;
         
   for(t2=0;t2<=(media-1);t2++){
      promediopresionp+=PromPresionp[t2];
   }
   
   promediopresionp=promediopresionp/media;   
   
   return promediopresionp;
   
   /////////////////**************
   /*promediopresionp=0.0;
   V2 = sensores(3);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
   V2 = V2*(5.0/1024.0);
   Presionp=(V2-0.2)/(K);
   
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
   
   return promediopresionp;*/
}

void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   //error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
   
   if(error<-0.3)   
      Ganancia=Ganancia2;   
   
   if(Ganancia<1)   
      Ganancia=1;
   
   if(error<0.0)     // Anti Wind_DOWN    
      error=0.0;     
      
   tmp=error*Ganancia;  // Control Proporcional.
         
   if(tmp>Setpoint)   // Anti Wind-UP         
      tmp=Setpoint;   
               
   if(tmp<0.0)     // Anti Wind_DOWN      
      tmp=0.0;   
   
   // Tpulso(t)= Tciclo*(y(t)-ymin)/(ymax - ymin); calculo de ciclo util para control de resistencia.ymax=140.ymin=0;
   // Tpulso es float, hay que pasar ese ciclo a una salida de un puerto usando el TMR1.
   Tpulso=(tmp/Setpoint)*Tciclo;  
   
   if(Tpulso<(Tciclo/5))
      Tpulso=0;
   
   if(Tpulso>(Tciclo-2))
      Tpulso=Tciclo;
   
   Tp=(Tpulso/0.005);
   Tc=(Tciclo/0.005);
   Controlando=1;   
   if(Temperatura>Setpoint+0.1){
      Suministro_off;
      Paso_off;
   }else if(Temperatura<=Setpoint){
      Suministro_on;
      Paso_on;
   }
//--------------------------------------------------------------------------------------------------------------------//   
}

void Envio_I2C(direccion,int16 posicion2, dato){

   i2c_start();            // Comienzo comunicación
   i2c_write(direccion);   // Dirección del esclavo en el bus I2C
   i2c_write(make8(posicion2,0));    // Posición donde se guardara el dato transmitido
   i2c_write(make8(posicion2,1));
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
   //SlaveA0Tx[7]=HIG;
   SlaveA0Tx[8]=LOW;
   SlaveA0Tx[9]=TERM;
   SlaveA0Tx[10]=DOOR;
   SlaveA0Tx[11]=STOP;
   SlaveA0Tx[12]=(int8)error;
   SlaveA0Tx[13]=CicloSeleccionado;
   SlaveA0Tx[14]=SetTime;
   SlaveA0Tx[15]=DiaTx;
   SlaveA0Tx[16]=MesTx;
   SlaveA0Tx[17]=YearTx;
   SlaveA0Tx[18]=HoraTx;
   SlaveA0Tx[19]=MinutoTx;
   SlaveA0Tx[20]=SegundoTx;
   SlaveA0Tx[21]=Imprime;
   SlaveA0Tx[22]=make8(Ciclos,0);
   SlaveA0Tx[23]=make8(Ciclos,1);
   SlaveA0Tx[24]=EstadoBuzzer;
   SlaveA0Tx[25]=codigoAlarma;
}

void Lee_Vector(void){
   //Dia=txbuf[0];
   if(txbuf[8]==44){ 
      Mes=txbuf[1];
      Year=txbuf[2];
      Hora=txbuf[3];
      Minuto=txbuf[4];
      Segundo=txbuf[5];
      dow=txbuf[6];
      Dia=txbuf[7];
   }
}

void Envio_Esclavos(void){
   Carga_Vector();
   for(i=0;i<26;i++){
      direccion=0xC0;
      Envio_I2C(direccion,i,SlaveA0Tx[i]);
   } 
}

void Lectura_Esclavos(void){
   for(i=0;i<9;i++){
      direccion=0xC0;                        //Dirección en el bus I2c
      posicion=i;                         //Posición de memoria a leer
      //Lectura_I2C(direccion, posicion, dato);    //Lectura por I2C
      i2c_start();            // Comienzo de la comunicación
      i2c_write(direccion);   // Dirección del esclavo en el bus I2C
      i2c_write(i);    // Posición de donde se leerá el dato en el esclavo
      i2c_write(0);    // Posición de donde se leerá el dato en el esclavo
      i2c_start();            // Reinicio
      i2c_write(direccion+1); // Dirección del esclavo en modo lectura
      dato=i2c_read(0);       // Lectura del dato
      i2c_stop();  
      txbuf[i]=(int8)dato;
   }
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
   G1=read_eeprom(42);
   delay_ms(10);
   G2=read_eeprom(43);
   delay_ms(10);
   sg1=read_eeprom(44);
   delay_ms(10);
   sg2=read_eeprom(45);
   
   for(i=0;i<30;i++){
      estados[i]=read_eeprom(i+50);
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
      
      if(CiclosPersonalizados[i].Temperatura>134 || CiclosPersonalizados[i].Temperatura<105)
         CiclosPersonalizados[i].Temperatura=121;
      if(CiclosPersonalizados[i].Minutoest>59)
         CiclosPersonalizados[i].Minutoest=3;
      if(CiclosPersonalizados[i].Minutosec>59)
         CiclosPersonalizados[i].Minutosec=0;   
      if(CiclosPersonalizados[i].Segundoest>59)
         CiclosPersonalizados[i].Segundoest=0;
      if(CiclosPersonalizados[i].Segundosec>59)
         CiclosPersonalizados[i].Segundosec=0;      
      if(CiclosPersonalizados[i].PulsosVacio>15)
         CiclosPersonalizados[i].PulsosVacio=7;
      if(CiclosPersonalizados[i].PulsosVacio<7)
         CiclosPersonalizados[i].PulsosVacio=7;
      if(CiclosPersonalizados[i].NivelVacio>100 || CiclosPersonalizados[i].NivelVacio<25)
         CiclosPersonalizados[i].NivelVacio=25;   
      if(CiclosPersonalizados[i].NivelDesfogue>6)
         CiclosPersonalizados[i].NivelDesfogue=6;
   }
   
   for(i=0;i<10;i++){//Lee contraseñas de usuarios
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
   if(Sensor[0].Ajuste>50){
      Sensor[0].Ajuste=11;
      write_eeprom(21,11);
      delay_ms(10);
   }
   if(Sensor[0].Ajusten>50){
      Sensor[0].Ajusten=0;
      write_eeprom(22,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajuste>50){
      Sensor[1].Ajuste=0;
      write_eeprom(23,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajusten>50){
      Sensor[1].Ajusten=0;
      write_eeprom(24,0);
      delay_ms(10);
   }
   if(MediaMovil>80){
      MediaMovil=40;
      write_eeprom(25,40);
      delay_ms(10);
   }
   if(Tciclo>10){
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
   if(Atmosferica>100){
      Atmosferica=69;
      write_eeprom(31,69);
      delay_ms(10);
   }
   if(Tapertura>100){
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

void ApagaSalidas(){
   Desfogue_off;
   Paso_off;
   Vacio_off;
   Aire_off;
   Suministro_off;
   BombaVacio_off;
   Buzzer_off;
   Agua_off;
   Controlando=0;
}

void ControlaPresion(){
   if(PresionPrecamara>=130){
      Suministro_off;
      Precalentamiento=ON;
   }
   if(PresionPrecamara<120)
      Suministro_on;
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
posicion=0;
direccion=0xB0;
Buzzer_on;
//Inicio de Transmision
i2c_start();            // Comienzo comunicación
i2c_write(direccion);   // Dirección del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
i2c_write(0xff);        // Dato a transmitir
i2c_stop();
++posicion;
i2c_start();            // Comienzo comunicación
i2c_write(direccion);   // Dirección del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
i2c_write(0xff);        // Dato a transmitir
i2c_stop();
++posicion;
//Se envian 2 veces ff para indicar que se inicia la transmision.

   for(i=0;i<5;i++){
      tmp=read_ext_eeprom((cicloVisto*320)+i);
      //if(tmp>250)
      //   tmp=20;
      i2c_start();            // Comienzo comunicación
      i2c_write(direccion);   // Dirección del esclavo en el bus I2C
      i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
      i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
      i2c_write(tmp);        // Dato a transmitir
      i2c_stop(); 
      ++posicion;
   }
   for(i=0;i<35;i++){//Lee Valores para completar la Tabla
      for(j=5;j<14;j++){//Lee Valores para completar la Tabla
         tmp=read_ext_eeprom((cicloVisto*320)+j+(i*9));
         //if(tmp>250)
         //   tmp=20;
         i2c_start();            // Comienzo comunicación
         i2c_write(direccion);   // Dirección del esclavo en el bus I2C
         i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
         i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
         i2c_write(tmp);        // Dato a transmitir
         i2c_stop(); 
         ++posicion;
      }
   }

//Fin de Transmision
i2c_start();            // Comienzo comunicación
i2c_write(direccion);   // Dirección del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
i2c_write(0x1f);        // Dato a transmitir
i2c_stop();
++posicion;
i2c_start();            // Comienzo comunicación
i2c_write(direccion);   // Dirección del esclavo en el bus I2C
i2c_write(make8(posicion,0));    // Posición donde se guardara el dato transmitido
i2c_write(make8(posicion,1));    // Posición donde se guardara el dato transmitido
i2c_write(0x1f);        // Dato a transmitir
i2c_stop();
++posicion;
//Se envian 2 veces 1f para indicar que se inicia la transmision.
delay_ms(200);
Buzzer_off;
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

void sistemaBloqueo(short activa){
   if(activa && DOOR){
      output_bit(PIN_C1,1);
      output_bit(PIN_C2,0);
      Cilindro=ON;
   }else if(!activa){
      output_bit(PIN_C1,0);
      output_bit(PIN_C2,1);
   }
}

void ConfiguraCiclo(int8 este0,este1,este2,este3,secado0,secado1,secado2,secado3,liquidos,ciclo,temperatura){
   Testerilizacionp[0]=este0;Testerilizacionp[1]=este1;Testerilizacionp[2]=este2;Testerilizacionp[3]=este3;
   Tsecadop[0]=secado0;Tsecadop[1]=secado1;Tsecadop[2]=secado2;Tsecadop[3]=secado3;
   Setpoint=temperatura;               
   tiempo_secado=0;
   if(Tsecadop[3]==0 && Tsecadop[2]==0){
      desfoguelento=1;
   }else{
      desfoguelento=0;
   }
   CicloSeleccionado=ciclo;  
   Testerilizacion[0]=Testerilizacionp[0];Testerilizacion[1]=Testerilizacionp[1];Testerilizacion[2]=Testerilizacionp[2];Testerilizacion[3]=Testerilizacionp[3];
   Tsecado[0]=Tsecadop[0];Tsecado[1]=Tsecadop[1];Tsecado[2]=Tsecadop[2];Tsecado[3]=Tsecadop[3];
   if(!borra){
      borraDatoCiclo();
      borra=ON;
   }
}

