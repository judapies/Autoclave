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
   V0 = V0/108.91; // Se elimina la ganancia
   //V0 += 14.82;
   V0 += 14.91;
   Pt=6810/((1023/V0)-1); 
            
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
   V1 = sensores(1);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 1k (+-10%)
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

void LeeEEPROM(){
//Lectura EEPROM//--------------------------
   Ganancia=read_eeprom(20);
   delay_ms(10);
   Ajuste=read_eeprom(21);
   delay_ms(10);
   Ajusten=read_eeprom(22);
   delay_ms(10);
   //Desvio=read_eeprom(23);
   MediaMovil=read_eeprom(24);
   delay_ms(10);
   Integral=read_eeprom(26);
   delay_ms(10);
   Derivativo=read_eeprom(27);
   delay_ms(10);
   CiclosL=read_eeprom(37);
   delay_ms(10);
   CiclosH=read_eeprom(38);
   delay_ms(10);
   Ciclos=make16(CiclosH,CiclosL);
   delay_ms(10);
   TTrampa=read_eeprom(39);
   delay_ms(10);
   //100 SteTemp
   //101 SteTime
   //102 Dry Time
   //103 Exh
   for(i=0;i<5;i++){      
      Set[i]=read_eeprom(100+(i*4));
      delay_ms(10);
      if(Set[i]>137)Set[i]=121;
      Time[i]=read_eeprom(101+(i*4));
      delay_ms(10);
      if(Time[i]>99)Time[i]=10;
      TimeS[i]=read_eeprom(102+(i*4));
      delay_ms(10);
      if(TimeS[i]>99)TimeS[i]=3;
      Exh[i]=read_eeprom(103+(i*4));
      delay_ms(10);
      if(Exh[i]>4)Exh[i]=1;
   }
}

void LimitaValores(){
   if(Ganancia>20.0){
      Ganancia=10.0;
      write_eeprom(20,10);
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
   if(Integral>10.0){
      Integral=1.0;
      write_eeprom(26,1);
      delay_ms(10);
   }
   if(Derivativo>90.0){
      Derivativo=20.0;
      write_eeprom(27,20);
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

void leerTeclado(){
   output_bit(PIN_B4,0);
   output_bit(PIN_B5,1);
   delay_us(50);
   if(!input(PIN_B0)){
      PARAMETERS=ON;
   }else{
      PARAMETERS=OFF;
   }
   
   if(!input(PIN_B1)){
      UP=ON;
   }else{
      UP=OFF;
   }
   
   output_bit(PIN_B5,0);
   output_bit(PIN_B4,1);
   delay_us(50);
   if(!input(PIN_B0)){
      CYCLE=ON;
   }else{
      CYCLE=OFF;
   }
   
   if(!input(PIN_B1)){
      PROGRAM=ON;
   }else{
      PROGRAM=OFF;
   }
}

void resetParams(){
   write_eeprom(20,10);
   delay_ms(10);
   write_eeprom(21,0);
   delay_ms(10);
   write_eeprom(22,0);
   delay_ms(10);
   write_eeprom(26,1);            
   delay_ms(10);
   write_eeprom(27,20);
   delay_ms(10);
   write_eeprom(39,75);
   delay_ms(10); 
   write_eeprom(37,0);
   delay_ms(10);
   write_eeprom(38,0);
   delay_ms(10);
   for(i=0;i<5;i++){      
      write_eeprom(100+(i*4),DSet[i]);
      delay_ms(10);
      write_eeprom(101+(i*4),DTime[i]);
      delay_ms(10);
      write_eeprom(102+(i*4),DTimeS[i]);
      delay_ms(10);
      write_eeprom(103+(i*4),DExh[i]);
      delay_ms(10);
   }
   Buzzer_on;
   delay_ms(1000);
   reset_cpu();
}
