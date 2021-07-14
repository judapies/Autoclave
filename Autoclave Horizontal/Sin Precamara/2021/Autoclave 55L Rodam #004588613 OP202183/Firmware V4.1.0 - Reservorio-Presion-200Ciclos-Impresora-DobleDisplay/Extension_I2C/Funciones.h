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
putc(0xFF,display);
delay_us(10);
putc(0xFF,display);
delay_us(10);
putc(0xFF,display);
delay_us(10);
}

void Carga_Vector(void){
   txbuf[0]=Abre;
}

void Lee_Vector(void){
   if(rxbuf[32]==255 && rxbuf[33]==128 && rxbuf[34]==50){
      Temporal=rxbuf[1];
      Temperatura=rxbuf[0];
      Temperatura=Temperatura+(Temporal/10);
      PresionCamara=make16(rxbuf[3],rxbuf[2]);
      PresionPreCamara=0;//make16(rxbuf[5],rxbuf[4]);
      Tpulso=rxbuf[6];
      NivelAlto=rxbuf[7];
      NivelBajo=rxbuf[8];
      Termostato=rxbuf[9];
      Puerta=rxbuf[10];
      Parada=rxbuf[11];
      error=rxbuf[12];
      Ciclo=rxbuf[13];
      SetTime=rxbuf[14];
      Dia=rxbuf[15];
      Mes=rxbuf[16];
      Year=rxbuf[17];
      Hora=rxbuf[18];
      Minuto=rxbuf[19];
      Segundo=rxbuf[20];
      Imprime=rxbuf[21];
      PuertaL=rxbuf[22];
      ciclosl=rxbuf[23];
      ciclosh=rxbuf[24];
      mEst=rxbuf[25];
      sEst=rxbuf[26];
      mSec=rxbuf[27];
      sSec=rxbuf[28];
      Menu2=rxbuf[29];
      Setpoint=rxbuf[30];
      codigoAlarma=rxbuf[31];
      rxbuf[32]=0;
      rxbuf[33]=0;
      rxbuf[34]=0;
      CiclosTotales=make16(ciclosh,ciclosl);
   }
}

void LeeEEPROM(){
//Lectura EEPROM//--------------------------   
}

void LimitaValores(){
}

void LeeDisplay(void){
   if(Dato_Exitoso==5){
         Rx_Buffer[0]=Rx_Buffer2[0];
         Rx_Buffer[1]=Rx_Buffer2[1];
         Rx_Buffer[2]=Rx_Buffer2[2];
         Rx_Buffer[3]=Rx_Buffer2[3];
         Rx_Buffer[4]=Rx_Buffer2[4];
         Rx_Buffer[5]=Rx_Buffer2[5];
         Rx_Buffer[6]=Rx_Buffer2[6];
         Rx_Buffer[7]=Rx_Buffer2[7];
         Rx_Buffer[8]=Rx_Buffer2[8];
         Rx_Buffer[9]=Rx_Buffer2[9];
         
      if(Rx_Buffer[3]==0x01){// Pregunta por la pagina en la que esta el display,01 es Contraseña de Acceso
         Menu=0;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x02){//02 es Menu Principal con PantallaPriincipal=0, y con PantallaPrincipal=1 es Ciclo Libre
         Menu=1;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x03){//03 es Tiempo Esterilizacion
         Menu=2;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x04){//04 es Tiempo de Secado
         Menu=3;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x05){//05 es Temperatura
         Menu=4;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x06){//06 es Nivel
         Menu=5;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x07){//07 es Test de Componentes
         Menu=6;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x08){//08 es Fecha y hora
         Menu=7;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x0f){//0f es Recibe caracteres de contraseña desde display
         
      }else if(Rx_Buffer[3]==0x1a){//1a es Menu de Funcionamiento
         Menu=20;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0x2c){//2c es Menu de Configuracion de Parametros
         Menu=100;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(Rx_Buffer[3]==0xcc){//cc es Menu de Bienvenida
         Menu=240;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
         //if(MenuAntt!=240)
            //reset_cpu();
      }
   }else{
      for(z=0;z<RX_BUFFER_SIZE;z++){
            //Rx_Buffer[z]=0;
            //Rx_Buffer2[z]=0;
         }
   }
}

void ApagaSalidas(){
}

