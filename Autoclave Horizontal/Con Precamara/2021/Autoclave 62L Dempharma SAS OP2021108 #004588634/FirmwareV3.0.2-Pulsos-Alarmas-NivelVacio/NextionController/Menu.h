void menuPrincipal(){
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(1500);
      printf("page Usuarios");
      SendDataDisplay();
      ApagaSalidas();
   }else if(Menu==0){ //Menu de Contraseña de acceso.
      entro=OFF;
      ApagaSalidas();
      //printf("bkcmd=0");
      //SendDataDisplay();
      if(RX_Buffer[4]==0x11){//11
         if(TipoClave!=0){
            printf("page Menu");
            SendDataDisplay();
            PantallaPrincipal=0;
         }
      }
      
      if(TipoClave==3){
         printf("titulo.txt=\"Clave Nueva\"");
         SendDataDisplay();
      }
      
      if(TipoClave==2){
         printf("titulo.txt=\"Clave Actual\"");
         SendDataDisplay();        
      }
      
      if(TipoClave==1){
         printf("titulo.txt=\"Clave Tecnico\"");
         SendDataDisplay();
      }
      
      if(TipoClave==0){
         printf("titulo.txt=\"%s\"",usuario);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x11){//11, Regresar
         if(TipoClave==1){
            printf("page Ajustes");
            SendDataDisplay();       
         }else{
            printf("page Usuarios");
            SendDataDisplay();       
         }
      }
      
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         delay_ms(800);
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32 &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){ // Si Ingresa clave para reset general del sistema.
               for(i=0;i<40;i++){//Lee contraseñas de usuarios
                  write_eeprom(300+i,0);delay_ms(20);
               }
               reset_cpu();
            }
            
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
               if((RX_Buffer[4]==Clave[codigoUsuario].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario].Password[1]+0x30)&&
               (RX_Buffer[6]==Clave[codigoUsuario].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario].Password[3]+0x30)
               &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){
                  printf("page ClaveCorrecta");
                  SendDataDisplay();
                  ingreso=ON;
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               }else{
                  printf("page ClaveBad");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               } 
            }
         }else if(TipoClave==1){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34&&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
               RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
            }else{
               printf("page Menu");
               SendDataDisplay();
               RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
            }
         }else if(TipoClave==2){
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
               if((RX_Buffer[4]==Clave[codigoUsuario2].Password[0]+0x30)&&(RX_Buffer[5]==Clave[codigoUsuario2].Password[1]+0x30)&&
               (RX_Buffer[6]==Clave[codigoUsuario2].Password[2]+0x30)&&(RX_Buffer[7]==Clave[codigoUsuario2].Password[3]+0x30)
               &&RX_Buffer[8]==0x00 &&RX_Buffer[9]==0xff){
                  printf("page Clave");
                  SendDataDisplay();
                  printf("titulo.txt=\"Clave Nueva\"");
                  SendDataDisplay();
                  TipoClave=3;
                  RX_Buffer[3]=0x00;
                  RX_Buffer2[3]=0x00;
               }else{
                  printf("page Menu");
                  SendDataDisplay();
                  RX_Buffer[3]=0x00;
                  RX_Buffer2[3]=0x00;
                  codigoUsuario=codigoUsuarioant;
               } 
            }
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page Menu");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(300+(codigoUsuario2*4),RX_Buffer[4]-0x30);delay_ms(20);
               write_eeprom(301+(codigoUsuario2*4),RX_Buffer[5]-0x30);delay_ms(20);
               write_eeprom(302+(codigoUsuario2*4),RX_Buffer[6]-0x30);delay_ms(20);
               write_eeprom(303+(codigoUsuario2*4),RX_Buffer[7]-0x30);delay_ms(20);
               GuardaEEPROM=ON;
               codigoUsuario=codigoUsuarioant;
            }
            RX_Buffer[3]=0x00;
            RX_Buffer2[3]=0x00;
         }
      }
    }else if(Menu==1){ //Menu Principal.
      tCiclof=0;
      oprimioCiclo=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      codigoUsuarioant=codigoUsuario;
      
      if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Calentamiento
         delay_ms(5);
         if(RX_Buffer[4]==0x01 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//01, Calentamiento
            printf("page Funcionamiento");
            SendDataDisplay();
            ConfiguraCiclo(0,15,0,1,0,1,121,pulsosConfigurados,vacioCiclo,0);
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//02, No Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x02 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,3,0,1,0,2,134,pulsosConfigurados,vacioCiclo,0);            
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x03  && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//03, No Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x03 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();     
            ConfiguraCiclo(0,15,0,1,0,3,121,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//04, Envueltos 134
         delay_ms(5);
         if(RX_Buffer[4]==0x04 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,4,0,15,0,4,134,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//05, Envueltos 121
         delay_ms(5);
         if(RX_Buffer[4]==0x05 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,15,0,5,121,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//06, Envuelto Doble 1
         delay_ms(5);
         if(RX_Buffer[4]==0x06 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,7,0,20,0,6,134,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//07, Envuelto Doble 2
         delay_ms(5);
         if(RX_Buffer[4]==0x07 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,7,0,30,0,7,134,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//08, Prion
         delay_ms(5);
         if(RX_Buffer[4]==0x08 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,20,0,15,0,8,134,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//09, Bowie & Dick
         delay_ms(5);
         if(RX_Buffer[4]==0x09 && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(30,3,0,1,0,9,134,pulsosConfigurados,vacioCiclo,0);  
            Op=OFF;
            Op2=OFF;
            MenuAnt=Menu;
            Fugas=OFF;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){//0a, Test de Fugas
         delay_ms(5);
         if(RX_Buffer[4]==0x0a && RX_Buffer[5]==0x0f && estados[19+RX_Buffer[4]]){
            printf("page Funcionamiento");
            SendDataDisplay();       
            ConfiguraCiclo(0,15,0,0,0,10,30,1,vacioTest,1);
            MenuAnt=Menu;
            Libre=OFF;
            //TiempoVacio=CantidadPulsos*180;
         }
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Ciclos Fijos
         printf("page Menu");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         if(codigoUsuario==0 || codigoUsuario==9){       
            printf("page Ajustes");
            SendDataDisplay();     
         }
      }
   }else if(Menu==2){ //Menu de Tiempo Esterilizacion     
         printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }           
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar            
               Segundos=OFF;
               Minutos=OFF;
               printf("page CicloPersona");
               SendDataDisplay();  
               if(!GuardaEEPROM){
                  CiclosPersonalizados[CicloLibre-1].Minutoest=mEstp;
                  CiclosPersonalizados[CicloLibre-1].Segundoest=sEstp;
                  write_eeprom(101+((CicloLibre-1)*8),mEstp);
                  delay_ms(10);
                  write_eeprom(102+((CicloLibre-1)*8),sEstp);
                  delay_ms(10);
                  mEst=mEstp;
                  sEst=sEstp;
                  GuardaEEPROM=ON;
               }
         }
         
         if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla arriba Oprimida
            if(Segundos){
               sEstp++;
            }
            if(Minutos){
               mEstp++;
            }  
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla abajo oprimida
            if(Segundos && sEstp>0){
               sEstp--;
            }
            if(Minutos){
               mEstp--;
            }
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;
         }
         
         if(mEstp<1)mEstp=99;
         if(mEstp>99)mEstp=1;
         if(sEstp>59)sEstp=0;
            
         printf("tminest.txt=\"%02u\"",mEstp);
         SendDataDisplay();
         printf("tsecest.txt=\"%02u\"",sEstp);
         SendDataDisplay();
    }else if(Menu==3){ //Menu de Tiempo de Secado
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();            
            
         if(RX_Buffer[4]==0x0a){//Selecciono Minutos
            Minutos=ON;
            Segundos=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Segundos
            Segundos=ON;
            Minutos=OFF;
         }
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Minutosec=mSecp;
               CiclosPersonalizados[CicloLibre-1].Segundosec=sSecp;
               write_eeprom(103+((CicloLibre-1)*8),mSecp);
               delay_ms(10);
               write_eeprom(104+((CicloLibre-1)*8),sSecp);
               delay_ms(10);
               mSec=mSecp;
               sSec=sSecp;
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla Derecha Oprimida
            if(Segundos){
               sSecp++;
            }
            if(Minutos){
               mSecp++;
            }  
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00; 
         }
         
         if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla Izquierda Oprimida
            if(Segundos && sSecp>0){
               sSecp--;
            }
            if(Minutos){
               mSecp--;
            } 
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
         if(mSecp<1)mSecp=99;
         if(mSecp>99)mSecp=1;
         if(sSecp>59)mSecp=0;
            
      printf("tminsec.txt=\"%02u\"",mSecp);
      SendDataDisplay();
      printf("tsecsec.txt=\"%02u\"",sSecp);
      SendDataDisplay();
    }else if(Menu==4){ //Menu de Temperatura
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Setpoint<110)
            Setpoint=138;
         if(Setpoint>138)
            Setpoint=110;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;
            printf("page CicloPersona");
            SendDataDisplay();  
            PantallaPrincipal=1;
            Op=OFF;
            Op2=OFF;
            if(!GuardaEEPROM){
               CiclosPersonalizados[CicloLibre-1].Temperatura=Setpoint;
               write_eeprom(100+((CicloLibre-1)*8),Setpoint);
               delay_ms(10);
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Setpoint++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Setpoint--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tsettem.txt=\"%03u\"",Setpoint);
      SendDataDisplay();
    }else if(Menu==5){//Menu de Nivel de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Nivel<25)
            Nivel=100;
         if(Nivel>100)
            Nivel=25;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].NivelVacio=Nivel;
               write_eeprom(106+((CicloLibre-1)*8),Nivel);
               delay_ms(20);            
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Nivel++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Nivel--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tnivel.txt=\"%Lu\"",Nivel);
      SendDataDisplay();
   }else if(Menu==6){ // Menu de Prueba de Componentes
         PruebaEstados=ON;
         
         printf("t0.txt=\"Test %02u\"",Codigo);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x01){//Oprimio EV Suministro
            EstadoSuministro=!EstadoSuministro;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSuministro){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            suministroAire=ON;
         }else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            suministroAire=OFF;
         }
         
         if(RX_Buffer[4]==0x02){//Oprimio EV Desfogue
            EstadoDesfogue=!EstadoDesfogue;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoDesfogue){
            printf("b1.bco=2016");
            SendDataDisplay(); 
            Desfogue=ON;
         }else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            Desfogue=OFF;
         }
         
         if(RX_Buffer[4]==0x03){//Oprimio EV Paso
            EstadoPaso=!EstadoPaso;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoPaso){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            Paso=ON;
         }else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            Paso=OFF;
         }
        
         if(RX_Buffer[4]==0x04){//Oprimio Vacio
            EstadoVacio=!EstadoVacio;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoVacio){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            outVacio=ON;
         }else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            outVacio=OFF;
         }
         
         if(RX_Buffer[4]==0x05){//Oprimio Aire
            EstadoAire=!EstadoAire;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAire){
            printf("b4.bco=2016");
            SendDataDisplay(); 
            Aire=ON;
         }else{
            printf("b4.bco=63488");
            SendDataDisplay(); 
            Aire=OFF;
         }
         
         if(RX_Buffer[4]==0x06){//Oprimio Bomba Vacio
            EstadoBomba=!EstadoBomba;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBomba){
            printf("b5.bco=2016");
            SendDataDisplay(); 
            if(ReservorioLleno)
               bombaVacio=ON;
            else
               bombaVacio=OFF;
         }else{
            printf("b5.bco=63488");
            SendDataDisplay(); 
            bombaVacio=OFF;
         }
         
         if(RX_Buffer[4]==0x07){//Oprimio EV Agua
            EstadoAguaEnfriador=!EstadoAguaEnfriador;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAguaEnfriador){
            printf("b7.bco=2016");
            SendDataDisplay(); 
            aguaEnfriador=ON;
         }else{
            printf("b7.bco=63488");
            SendDataDisplay(); 
            aguaEnfriador=OFF;
         }
         
         if(RX_Buffer[4]==0x08){//Oprimio EV Suministro Vapor
            EstadoSuministroV=!EstadoSuministroV;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoSuministroV){
            printf("b6.bco=2016");
            SendDataDisplay(); 
            suministroVapor=ON;
         }else{
            printf("b6.bco=63488");
            SendDataDisplay(); 
            suministroVapor=OFF;
         }
         
         if(RX_Buffer[4]==0x09){//Oprimio Abrir Cierre
            EstadoCilindrosL=!EstadoCilindrosL;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoCilindrosL){
            printf("b12.bco=2016");
            SendDataDisplay(); 
            cilindroLimpio=OFF;
         }else{
            printf("b12.bco=63488");
            SendDataDisplay(); 
            cilindroLimpio=ON;
         }
         
         if(RX_Buffer[4]==0x0a){//Oprimio Empaque
            EstadoEmpaqueL=!EstadoEmpaqueL;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoEmpaqueL && DOORL){
            printf("b16.bco=2016");
            SendDataDisplay(); 
            empaqueLimpio=ON;            
         }else{
            printf("b16.bco=63488");
            SendDataDisplay(); 
            empaqueLimpio=OFF;
         }
         
         if(RX_Buffer[4]==0x0b){//Oprimio Abrir Cierre
            EstadoCilindrosS=!EstadoCilindrosS;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoCilindrosS){
            printf("b17.bco=2016");
            SendDataDisplay(); 
            cilindroSucio=OFF;
         }else{
            printf("b17.bco=63488");
            SendDataDisplay(); 
            cilindroSucio=ON;
         }
         
         if(RX_Buffer[4]==0x0c){//Oprimio Empaque
            EstadoEmpaqueS=!EstadoEmpaqueS;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoEmpaqueS && DOORS){
            printf("b25.bco=2016");
            SendDataDisplay(); 
            empaqueSucio=ON;            
         }else{
            printf("b25.bco=63488");
            SendDataDisplay(); 
            empaqueSucio=OFF;
         }
         
         if(RX_Buffer[4]==0x0d){//Oprimio Buzzer
            EstadoBuzzer=!EstadoBuzzer;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBuzzer){
            printf("b21.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b21.bco=63488");
            SendDataDisplay(); 
         }
         
         if(RX_Buffer[4]==0x0e){//Oprimio Aux1
            EstadoAux1=!EstadoAux1;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAux1){
            seguroValvula=ON;
            printf("b22.bco=2016");
            SendDataDisplay(); 
         }else{
            seguroValvula=OFF;
            printf("b22.bco=63488");
            SendDataDisplay(); 
         }
         
         if(RX_Buffer[4]==0x0f){//Oprimio Aux2
            EstadoAux2=!EstadoAux2;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoAux2){
            seguroValvula2=ON;
            printf("b28.bco=2016");
            SendDataDisplay(); 
         }else{
            seguroValvula2=OFF;
            printf("b28.bco=63488");
            SendDataDisplay(); 
         }
         
         if(DOORS){
            printf("b8.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b8.bco=63488");
            SendDataDisplay(); 
         }
         
         if(DOORL){
            printf("b26.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b26.bco=63488");
            SendDataDisplay(); 
         }
         
         if(EMPAQUEL){
            printf("b9.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b9.bco=63488");
            SendDataDisplay(); 
         }
         
         if(EMPAQUES){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }
         
         if(STOP){
            printf("b10.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b10.bco=63488");
            SendDataDisplay(); 
         }
         
         if(LOWB){
            printf("b23.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b23.bco=63488");
            SendDataDisplay(); 
         }
         
         if(BLOQUEOL){
            printf("b24.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b24.bco=63488");
            SendDataDisplay(); 
         }
         
         if(BLOQUEOS){
            printf("b27.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b27.bco=63488");
            SendDataDisplay(); 
         }         
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"A4:%3.0f %03Lu\"",sensores(4),PresionPreCamara);
         SendDataDisplay();
         printf("b18.txt=\"A5:%3.0f %03Lu\"",sensores(5),PresionCamara);
         SendDataDisplay();
         printf("b19.txt=\"T2:%3.1f\"",Temperatura2);
         SendDataDisplay();
         printf("b20.txt=\"T1:%3.1f\"",Temperatura);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x21)//Oprimio salir
         {
            printf("page Menu");
            SendDataDisplay();
            PantallaPrincipal=0;
            PruebaEstados=OFF;
            ApagaSalidas();
            // Falta apagar Todo
         }
     }else if(Menu==7){ //Menu de Configuración de Fecha y Hora
      
         if(RX_Buffer[4]==0x0a)//Selecciono Hora
            Opcion=4;
         
         if(RX_Buffer[4]==0x0b)//Selecciono Minuto
            Opcion=5;
         
         if(RX_Buffer[4]==0x0c)//Selecciono Dia
            Opcion=1;
         
         if(RX_Buffer[4]==0x0d)//Selecciono Hora
            Opcion=2;
         
         if(RX_Buffer[4]==0x0e)//Selecciono Year
            Opcion=3;
         
         if(Opcion>5)
            Opcion=1;
         if(Opcion<1)
            Opcion=5;
            
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            printf("page Menu");
            SendDataDisplay();  
            PantallaPrincipal=0;
            Op=OFF;
            Op2=OFF;
            
            if(esBisiesto(YearTx))
               Modulo=Bisiesto[MesTx];
            else
               Modulo=Regular[MesTx];
         
            dowTx=((YearTx-1)%7+((YearTx-1)/4-3*((YearTx-1)/100+1)/4)%7+Modulo+DiaTx%7)%7;
            SetTime=5;
            Envio_Esclavos();
            delay_ms(100);   
            SetTime=10;
            rtc_set_datetime(DiaTx,MesTx,YearTx,dowTx,HoraTx,MinutoTx);
         }
         
         if(RX_Buffer[4]==0x2a){//Tecla Arriba Oprimida
               if(Opcion==1){
                  if(MesTx==2){
                     if(esBisiesto(YearTx)){
                        if(DiaTx<29)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }else{
                        if(DiaTx<28)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }
                  }else{
                     if(MesTx<=7){
                        if(MesTx % 2 ==0){
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }else{
                           if(DiaTx<31)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }    
                     }else{
                        if(MesTx % 2 ==0){
                           if(DiaTx<31)
                              DiaTx++;  
                           else
                              DiaTx=1;
                        }else{
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;
                        }    
                     }
                  }
               }else if(Opcion==2){
                  if(MesTx<12)
                     MesTx++;
                  else
                     MesTx=1;
               }else if(Opcion==3){
                  if(YearTx<99)
                     YearTx++;
                  else 
                     YearTx=0;
               }else if(Opcion==4){
                  if(HoraTx<24)
                     HoraTx++;
                  else
                     HoraTx=0;
               }else if(Opcion==5){
                  if(MinutoTx<59)
                     MinutoTx++;
                  else
                     MinutoTx=0;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x2b){//Tecla Abajo Oprimida 
               if(Opcion==1){
                  if(DiaTx>0)
                     DiaTx--;
               }else if(Opcion==2){
                  if(MesTx>1)
                     MesTx--;
                  else
                     MesTx=12;
               }else if(Opcion==3){
                  if(YearTx>0)
                     YearTx--;
                  else
                     YearTx=99;
               }else if(Opcion==4){
                  if(HoraTx>0)
                     HoraTx--;
                  else
                     HoraTx=23;
               }else if(Opcion==5){
                  if(MinutoTx>0)
                     MinutoTx--;
                  else
                     MinutoTx=59;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
               //BorraBuffer();
         }
      
      printf("thora.txt=\"%02u\"",HoraTx);
      SendDataDisplay();
      printf("tminutos.txt=\"%02u\"",MinutoTx);
      SendDataDisplay();
      printf("tdia.txt=\"%02u\"",DiaTx);
      SendDataDisplay();
      printf("tmes.txt=\"%02u\"",MesTx);
      SendDataDisplay();
      printf("tyear.txt=\"%02u\"",YearTx);
      SendDataDisplay();
      LimitaDia();  
    }else if(Menu==8){ //Menu Ciclos Personalizados
      tCicloP=0;
      oprimioCicloP=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[4]<0x15 && estados[RX_Buffer[4]-1]){//Personalizados
         printf("ciclo.txt=\"CICLO P%02u\"",RX_Buffer[4]+0x0a);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutoest,CiclosPersonalizados[RX_Buffer[4]-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].Minutosec,CiclosPersonalizados[RX_Buffer[4]-1].Segundosec);
         SendDataDisplay();
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[RX_Buffer[4]-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelVacio);
         SendDataDisplay();
         //printf("desfogue.txt=\"%u\"",CiclosPersonalizados[RX_Buffer[4]-1].NivelDesfogue);
         printf("desfogue.txt=\"N/A\"");
         SendDataDisplay();
         CicloLibre=RX_Buffer[4];
      }else if(CicloLibre==0x00){
         printf("ciclo.txt=\"CICLO P--\"");
         SendDataDisplay();          
         printf("temp.txt=\"--\"");
         SendDataDisplay();
         printf("test.txt=\"--:--\"");
         SendDataDisplay();
         printf("tsec.txt=\"--:--\"");
         SendDataDisplay();
         printf("pulsos.txt=\"--\"");
         SendDataDisplay();
         printf("vacio.txt=\"--\"");
         SendDataDisplay();
         printf("desfogue.txt=\"--\"");
         SendDataDisplay();
      }else if(CicloLibre>0 && CicloLibre<21 && estados[CicloLibre-1]){
         printf("ciclo.txt=\"CICLO P%02u\"",CicloLibre+0x0a);
         SendDataDisplay();          
         printf("temp.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].Temperatura);
         SendDataDisplay();
         printf("test.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutoest,CiclosPersonalizados[CicloLibre-1].Segundoest);
         SendDataDisplay();
         printf("tsec.txt=\"%02u:%02u\"",CiclosPersonalizados[CicloLibre-1].Minutosec,CiclosPersonalizados[CicloLibre-1].Segundosec);
         SendDataDisplay();
         printf("pulsos.txt=\"%02u\"",CiclosPersonalizados[CicloLibre-1].PulsosVacio);
         SendDataDisplay();
         printf("vacio.txt=\"%03u kPa\"",CiclosPersonalizados[CicloLibre-1].NivelVacio);
         SendDataDisplay();
         //printf("desfogue.txt=\"%u\"",CiclosPersonalizados[CicloLibre-1].NivelDesfogue);
         printf("desfogue.txt=\"N/A\"");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page Menu");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Ajustes");
            SendDataDisplay();     
         }
      }
      
      if(RX_Buffer[4]==0x20 && CicloLibre>0){//20, Temperatura
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Temperatura");
            SendDataDisplay();     
            Setpoint=CiclosPersonalizados[CicloLibre-1].Temperatura;
         }
      }
      
      if(RX_Buffer[4]==0x30 && CicloLibre>0){//30, Test
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page TiempoEst");
            SendDataDisplay();     
            mEstp=CiclosPersonalizados[CicloLibre-1].Minutoest;
            sEstp=CiclosPersonalizados[CicloLibre-1].Segundoest;
         }
      }
      
      if(RX_Buffer[4]==0x40 && CicloLibre>0){//40, Tsec
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page TiempoSec");
            SendDataDisplay();   
            mSecp=CiclosPersonalizados[CicloLibre-1].Minutosec;
            sSecp=CiclosPersonalizados[CicloLibre-1].Segundosec;
         }
      }
      
      if(RX_Buffer[4]==0x50 && CicloLibre>0){//50, PulsosVacio
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Pulsos");
            SendDataDisplay();     
            Pulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
         }
      }
      
      if(RX_Buffer[4]==0x60 && CicloLibre>0){//60, NivelVacio
         if(codigoUsuario==0 || codigoUsuario==9){  
            printf("page Nivel");
            SendDataDisplay();     
            Nivel=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         }
      }
      
      if(RX_Buffer[4]==0x80 && CicloLibre>0){//80, NivelDesfogue
         //printf("page NivelDes");
         //SendDataDisplay();     
         //NivelDes=CiclosPersonalizados[CicloLibre-1].NivelDesfogue;
      }
      
      if(RX_Buffer[4]==0x70 && CicloLibre>0){//70, Iniciar Ciclo
         printf("page Funcionamiento");
         SendDataDisplay();       
         Libre=ON;
         ConfiguraCiclo(CiclosPersonalizados[CicloLibre-1].Segundoest,CiclosPersonalizados[CicloLibre-1].Minutoest,
         CiclosPersonalizados[CicloLibre-1].Segundosec,CiclosPersonalizados[CicloLibre-1].MinutoSec,0,CicloLibre+10,
         CiclosPersonalizados[CicloLibre-1].Temperatura,CiclosPersonalizados[CicloLibre-1].PulsosVacio,CiclosPersonalizados[CicloLibre-1].NivelVacio
         ,0);
         NivelVacio=CiclosPersonalizados[CicloLibre-1].NivelVacio;
         CantidadPulsos=CiclosPersonalizados[CicloLibre-1].PulsosVacio;
         Fugas=OFF;
      }
   }else if(Menu==9){//Menu de Pulsos de Vacio
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(Pulsos<4)
            Pulsos=15;
         if(Pulsos>15)
            Pulsos=4;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].PulsosVacio=Pulsos;
               write_eeprom(105+((CicloLibre-1)*8),Pulsos);
               delay_ms(20);            
               //NivelSeleccionado=9990*Nivel;
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            Pulsos++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            Pulsos--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
            //BorraBuffer();
         }
         
      printf("tpulsos.txt=\"%u\"",Pulsos);
      SendDataDisplay();
   }else if(Menu==10){//Menu de Usuarios
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      if(ingreso)
         TipoClave=2;
      else
         TipoClave=0;
      
      if(RX_Buffer[4]==0x01){//01, Admin
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Admin";
            codigoUsuario=0;
         }
         codigoUsuario2=0;
      }else if(RX_Buffer[4]==0x02){//02, Operador 1
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 1";
            codigoUsuario=1;
         }
         codigoUsuario2=1;
      }else if(RX_Buffer[4]==0x03){//03, Operador 2
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 2";
            codigoUsuario=2;
         }
         codigoUsuario2=2;
      }else if(RX_Buffer[4]==0x04){//04, Operador 3
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 3";
            codigoUsuario=3;
         }
         codigoUsuario2=3;
      }else if(RX_Buffer[4]==0x05){//05, Operador 4
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 4";
            codigoUsuario=4;
         }
         codigoUsuario2=4;
      }else if(RX_Buffer[4]==0x06){//06, Operador 5
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 5";
            codigoUsuario=5;
         }
         codigoUsuario2=5;
      }else if(RX_Buffer[4]==0x07){//07, Operador 6
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 6";
            codigoUsuario=6;
         }
         codigoUsuario2=6;
      }else if(RX_Buffer[4]==0x08){//08, Operador 7
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 7";
            codigoUsuario=7;
         }
         codigoUsuario2=7;
      }else if(RX_Buffer[4]==0x09){//09, Operador 8
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Operador 8";
            codigoUsuario=8;
         }
         codigoUsuario2=8;
      }else if(RX_Buffer[4]==0x0a){//0a, Tecnico
         printf("page Clave");
         SendDataDisplay();       
         if(!ingreso){
            usuario="Tecnico";
            codigoUsuario=9;
         }
         codigoUsuario2=9;
      }
      
      if(RX_Buffer[4]==0xaa && ingreso){//aa, Regresar
         printf("page Ajustes");
         SendDataDisplay();       
         codigoUsuario2=10;
      }
   }else if(Menu==11){//Menu de Configuraciones
      tConfig=0;
      oprimioConfig=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      printf("tuser.txt=\"%s\"",usuario);
      SendDataDisplay();
      TipoClave=0;
      
      if(RX_Buffer[4]==0x01){//01, Ciclos
         printf("page Ciclos");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x02){//02, Test de Componentes
         printf("page Test");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x03){//03, Modificación de clave Usuarios
         printf("page Usuarios");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x04){//03, Modificación de clave Usuarios
         printf("page Activa");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0x11 && CodigoUsuario==9){//11, Menu Oculto Tecnico
         printf("page Clave");
         SendDataDisplay(); 
         TipoClave=1;
      }
      
      if(RX_Buffer[4]==0x05){//05, Hora y Fecha
         printf("page Fecha");
         SendDataDisplay();     
         DiaTx=Dia;
         MesTx=Mes;
         YearTx=Year;
         HoraTx=Hora;
         MinutoTx=Minuto;
      }
      
      if(RX_Buffer[4]==0xa0){//a0, Menu Principal
         printf("page Menu");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xb0){//b0, Personalizados
         printf("page CicloPersona");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[4]==0xc0){//c0, Configuraciones
         printf("page Ajustes");
         SendDataDisplay();     
      }
   }else if(Menu==12){//Menu de Visualizacion de Ciclos Realizados
      //printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      //SendDataDisplay();
      GuardaEEPROM=OFF;
      
      i2c_start();            // Comienzo de la comunicación
      i2c_write(0xB0);   // Dirección del esclavo en el bus I2C
      i2c_write(9);    // Posición de donde se leerá el dato en el esclavo
      i2c_write(0);    // Posición de donde se leerá el dato en el esclavo
      i2c_start();            // Reinicio
      i2c_write(direccion+1); // Dirección del esclavo en modo lectura
      dato=i2c_read(0);       // Lectura del dato
      i2c_stop();
      
      printf("estado.txt=\"Oprima Exportar para imprimir\"");
      SendDataDisplay();     
      
      if(RX_Buffer[6]==0x01){//01, Ciclos
         printf("page Menu");
         SendDataDisplay();     
      }
      
      if(RX_Buffer[6]==0x02){//02, Izquierda
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisto>0)
            cicloVisto--;
         printf("h0.val=%Lu",cicloVisto);
         SendDataDisplay();     
         actualizaTabla();
      }
      
      if(RX_Buffer[6]==0x03){//03, Arriba
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisualizado<28)
            cicloVisualizado++;
         printf("h1.val=%Lu",cicloVisualizado);
         SendDataDisplay();    
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x04){//04, Abajo
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisualizado>0)
            cicloVisualizado--;
         printf("h1.val=%Lu",cicloVisualizado);
         SendDataDisplay();  
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x05){//05, Derecha
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         if(cicloVisto<200)
            cicloVisto++;
         printf("h0.val=%Lu",cicloVisto);
         SendDataDisplay();  
         actualizaTabla();
         //exportarData();
      }
      
      if(RX_Buffer[6]==0x06){//06, Imprime
         RX_Buffer[6]=0x00;  
         RX_Buffer2[6]=0x00;
         printf("t0.txt=\"!IMPRIMIENDO!\"");
         SendDataDisplay(); 
         exportarData();
      }
      
      if(RX_Buffer[4]>0x00 || RX_Buffer[5]>0x00 || UP || DOWN || RIGHT || LEFT){
         if(UP){
            if(cicloVisualizado<28){
               cicloVisualizado++;
            }
            delay_ms(20);
            printf("h0.val=%Lu",cicloVisto+1);
            SendDataDisplay();   
            printf("h1.val=%Lu",cicloVisualizado+1);
            SendDataDisplay(); 
         }else if(DOWN){
            if(cicloVisualizado>0){
               cicloVisualizado--;
            }
            delay_ms(20);
            printf("h0.val=%Lu",cicloVisto+1);
            SendDataDisplay();   
            printf("h1.val=%Lu",cicloVisualizado+1);
            SendDataDisplay(); 
         }else if(RIGHT){
            if(cicloVisto<200){
               cicloVisto++;
            }
            delay_ms(20);
            printf("h0.val=%Lu",cicloVisto+1);
            SendDataDisplay();   
            printf("h1.val=%Lu",cicloVisualizado+1);
            SendDataDisplay(); 
         }else if(LEFT){
            if(cicloVisto>0){
               cicloVisto--;
            }
            delay_ms(20);
            printf("h0.val=%Lu",cicloVisto+1);
            SendDataDisplay();   
            printf("h1.val=%Lu",cicloVisualizado+1);
            SendDataDisplay(); 
         }else{
            cicloVisualizado=RX_Buffer[4];
            cicloVisto=RX_Buffer[5];
            RX_Buffer[4]=0x00;
            RX_Buffer[5]=0x00;
            RX_Buffer2[4]=0x00;
            RX_Buffer2[5]=0x00;
         }
         actualizaTabla();  
      }
      
      //Imprime Valores Estaticos
      printf("fecha.txt=\"%02u/%02u/%02u\"",diaCiclos,mesCiclos,yearCiclos);
      SendDataDisplay();
      
      printf("ciclo.txt=\"%02u\"",cicloCiclos);
      SendDataDisplay();
      
      printf("t0.txt=\"Ciclo %Lu\"",cicloVisto);
      SendDataDisplay();
      
      if(usuarioCiclos==0){
         printf("usuario.txt=\"Admin\"");
         SendDataDisplay();
      }else if(usuarioCiclos>0 && usuarioCiclos<9){
         printf("usuario.txt=\"Operador%u\"",usuarioCiclos);
         SendDataDisplay();
      }else if(usuarioCiclos==9){
         printf("usuario.txt=\"Tecnico\"");
         SendDataDisplay();
      }
      
      //Imprime Valores Dinamicos
      for(i=0;i<7;i++){
         printf("hora%Lu.txt=\"%02Lu.  %02u:%02u\"",i+1,cicloVisualizado+i+1,horaCiclos[i],minutoCiclos[i]);
         SendDataDisplay();
         printf("T%Lu.txt=\"%3.1f°C\"",i+1,TCiclos[i]);
         SendDataDisplay();
         printf("Pc%Lu.txt=\"%03LukPa\"",i+1,PcCiclos[i]);
         SendDataDisplay();
         printf("Ppc%Lu.txt=\"%03LukPa\"",i+1,PpcCiclos[i]);
         SendDataDisplay();         
         codificaAlarma();
      }  
   }else if(Menu==13){//Menu de Nivel de Desfogue
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
         if(NivelDes<1)
            NivelDes=6;
         if(NivelDes>6)
            NivelDes=1;
         
         if(RX_Buffer[4]==0x11){//Selecciono Regresar
            Segundos=OFF;
            Minutos=OFF;            
            tiempo_secado=0;
            printf("page CicloPersona");
            SendDataDisplay();  
            if(!GuardaEEPROM){    
               CiclosPersonalizados[CicloLibre-1].NivelDesfogue=NivelDes;
               write_eeprom(107+((CicloLibre-1)*8),NivelDes);
               delay_ms(20);            
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            NivelDes++;
            RX_Buffer[4]=0x00;            
            RX_Buffer2[4]=0x00;                       
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            NivelDes--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
         
      printf("tnivel.txt=\"%u\"",NivelDes);
      SendDataDisplay();
   }else if(Menu==14){//Menu de Activacion de Ciclos
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      
      if(RX_Buffer[4]==0xaa){
         for(i=0;i<30;i++){
            write_eeprom(60+i,estados[i]);
            delay_ms(10);
         }
         printf("page Ajustes");
         SendDataDisplay(); 
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;
      }
      
      for(m=0;m<30;m++){
         if(RX_Buffer[4]==m+1){
            estados[m]=!estados[m];
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(estados[m]){
            printf("b%u.bco=2016",m);
            SendDataDisplay();             
         }else{
            printf("b%u.bco=63488",m);
            SendDataDisplay(); 
         }
      }
   }else if(Menu==15){//Menu de Clave Correcta
      if(!entro){
         delay_ms(2000);
         restart_wdt();
         entro=ON;
      }
      printf("page Menu");
      //printf("page Funcionamiento");
      SendDataDisplay();
   }else if(Menu==16){//Menu de Clave InCorrecta
      if(!entro){
         delay_ms(2000);
         restart_wdt();
         entro=ON;
      }
      printf("page Clave");
      SendDataDisplay();
   }else if(Menu==20){ // Este es el menu de cuando el proceso se inicio.FUNCIONAMIENTO
   
   GuardaDatoEstatico();
   guardaCiclo=ON;
   if(!Libre){
      printf("t2f.txt=\"%u\"",CicloSeleccionado);
      SendDataDisplay();
      printf("t3f.txt=\"%s\"",Texto[CicloSeleccionado-1]);
      SendDataDisplay();
   }else{
      printf("t2f.txt=\"%u\"",CicloLibre+10);
      SendDataDisplay();
      printf("t3f.txt=\"Personalizado\"");
      SendDataDisplay();
   }
   
   leePulsadores();
   /*
   if(PresionCamara>Atmosferica+15){
      cierraPuertaS();
      cierraPuertaL();
   }
   */
   if(!Start){
   
      /*
      if(pulsoSubirS)
         cierraPuertaS();
      
      if(pulsoBajarS)
         abrePuertaS();
         
      if(RX_Buffer[4]==0x0a){//Oprimio Subir Puerta
         //cierraPuertaS();
         pulsoSubirS=ON;
         pulsoBajarS=OFF;
         RX_Buffer[4]=0x00;
         //RX_Buffer2[4]=0x00;                       
      }
      
      if(RX_Buffer[4]==0x0b){//Solto Subir Puerta
         RX_Buffer[4]=0x00;            
         //RX_Buffer2[4]=0x00;           
         pulsoSubirS=OFF;
         pulsoBajarS=OFF;
         if(!DOORS && !EMPAQUES)
            abrePuertaS();
         else
            cierraPuertaS();
      }
      
      if(RX_Buffer[4]==0x0c){//Oprimio Bajar Puerta
         pulsoSubirS=OFF;
         RX_Buffer[4]=0x00;
         //RX_Buffer2[4]=0x00; 
         if(Temperatura<Tapertura && cerroLimpio)
            pulsoBajarS=ON;
      }   
      
      if(RX_Buffer[4]==0x0d){//Solto Bajar Puerta
         RX_Buffer[4]=0x00;            
         //RX_Buffer2[4]=0x00;                       
      }
      */
   }
         
   if(PresionPreCamara>350 || PresionCamara>350){
      AlarmaPresion=ON;
   }
   
   if(TiempoCiclo>1800 && !Fugas){//ANALIZAR BIEN ESTA ALARMA
      if(TiempoCiclo>5400){
         AlarmaTiempoProlongado=ON;
      }
      if(PresionPrecamara<10){
         AlarmaTiempoProlongado=ON;
      }
   }else{
      AlarmaTiempoProlongado=OFF;
   }
   
   if(Temperatura>Setpoint+4.0 && !Fugas){
      AlarmaSobreTemperatura=ON;
   }
   
   if(TiempoLlenado>600){
      AlarmaLlenado=ON;
   }else{
      AlarmaLlenado=OFF;
   }
   
   //if(!DOORS && !EMPAQUES){      
   if(!DOORS){      
      Paso=OFF;
      //if(!Secando && !Finalizo && !Desfogando){
      if(!Finalizo){
         AlarmaPuertaS=ON;
      }
      cerroSucio=OFF;
   }else{
      AlarmaPuertaS=OFF;
      cerroSucio=ON;
   }
   /*
   if(!DOORL && !EMPAQUEL){      
      Paso=OFF;
      //if(!Secando && !Finalizo && !Desfogando){
      if(!Finalizo){
         AlarmaPuertaL=ON;
      }
      cerroLimpio=OFF;
   }else{
      AlarmaPuertaL=OFF;
      cerroLimpio=ON;
   }
   */
   if(tStop>=300){
      AlarmaEmergencia=ON;
   }
   /*
   if(tEmpaqueS>10)
      AlarmaEmpaqueS=ON;
      
   if(tEmpaqueL>10)
      AlarmaEmpaqueL=ON;   
   */
   if(AlarmaPuertaS || AlarmaPuertaL || AlarmaTermostato || AlarmaSobreTemperatura || AlarmaTiempoProlongado || AlarmaEmergencia || AlarmaPresion || AlarmaBomba || AlarmaEmpaqueL || AlarmaEmpaqueS
         || AlarmaVacio || AlarmaLlenado || !Start){
      Esterilizando=OFF;
      Secando=OFF;
      ActivaAlarma(1);
      printf("b2.pic=123");
      SendDataDisplay();
      if(AlarmaEmergencia){
         codigoAlarma=1;
         printf("Funcionamiento.t4f.txt=\" Parada de Emergencia\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         Paso=OFF;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         if(!EMPAQUES && PresionCamara<Atmosferica+4)
            empaqueSucio=OFF;
         
         if(!EMPAQUEL && PresionCamara<Atmosferica+4)
            empaqueLimpio=OFF;
            
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         Esterilizando=OFF;
         Desfogando=OFF;
         DesfogueSuave=OFF;
         //IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=20;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaPuertaS || AlarmaPuertaL){
         codigoAlarma=2;
         printf("Funcionamiento.t4f.txt=\" Puerta Abierta\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Paso=OFF;
         suministroVapor=OFF;
         if(AlarmaEmpaqueL)
            empaqueLimpio=OFF;
         if(AlarmaEmpaqueS)
            empaqueSucio=OFF;
         //Suministro_off;
         BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         IniciaLlenado=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaLlenado){
         codigoAlarma=9;
         printf("Funcionamiento.t4f.txt=\" Tiempo Llenado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         Aire=OFF;
         Desfogue=OFF;
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         IniciaLlenado=OFF;
         IniciaCiclo=OFF;
         Esterilizando=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=21;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaTermostato){
         codigoAlarma=3;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Termostato\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         Desfogue=ON;                  
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Esterilizando=OFF;
         Desfogando=OFF;
         IniciaCiclo=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=22;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaSobreTemperatura){                  
         codigoAlarma=4;
         tiempo_esterilizacion=0;tiempo_secado=0;         
         printf("Funcionamiento.t4f.txt=\" Sobretemperatura\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();  
         suministroVapor=OFF;
         Desfogue=ON;
         Paso=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=23;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaTiempoProlongado){
         codigoAlarma=5;
         printf("Funcionamiento.t4f.txt=\" Tiempo Prolongado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Desfogue=ON;
         Paso=OFF;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=24;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaPresion){ // Alarma de Error de SobrePresion: Error por exceso de Presion.
         codigoAlarma=6;
         printf("Funcionamiento.t4f.txt=\" SobrePresion\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         if(PresionPreCamara>(Atmosferica+4) && DOORS)
            Paso=ON;
         else
            Paso=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=25;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaBomba){ // Alarma de Error de que bomba no ha arrancado.
         codigoAlarma=7;
         printf("Funcionamiento.t4f.txt=\" Error Bomba\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         if(!flagImprimir){
            Imprime=26;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaVacio){ // Alarma de Error de que genera el Vacio en el tiempo estipulado.
         codigoAlarma=8;
         printf("Funcionamiento.t4f.txt=\" Error Vacio\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         if(PresionCamara<Atmosferica)
            Aire=ON;
         else
            Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         
         if(!flagImprimir){
            Imprime=27;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(AlarmaEmpaqueS || AlarmaEmpaqueL){ // Alarma de Error de que el empaque no se presuriza
         codigoAlarma=9;
         printf("Funcionamiento.t4f.txt=\" Error Empaque\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Cancelado\"");
         SendDataDisplay();
         if(AlarmaEmpaqueL && PresionCamara<Atmosferica+4)
            empaqueLimpio=OFF;
         if(AlarmaEmpaqueS && PresionCamara<Atmosferica+4)
            empaqueSucio=OFF;
         Desfogue=ON;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
            incrementaCiclos();
         }
         
         if(!flagImprimir){
            Imprime=29;
            Envio_Esclavos();
            delay_ms(100);
            Imprime=10;
            flagImprimir=1;
         }
         if(PresionCamara<=Atmosferica+15 && PresionCamara>Atmosferica-10 && Temperatura<=Tapertura)
            Start=OFF;
      }else if(!Start){ // Inicio o pausa de Ciclo
         printf("Funcionamiento.t4f.txt=\" Ciclo no Iniciado\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" Pausado\"");
         SendDataDisplay();
         Desfogue=OFF;
         suministroVapor=OFF;
         //if(!abrirPuertaL && !abrirPuertaS)
            BombaVacio=OFF;
         Aire=OFF;
         aguaEnfriador=OFF;
         outVacio=OFF;
         Controlando=0;
         EstadoBuzzer=OFF;
         codigoAlarma=19;
      }
   }else if(Start){
      printf("b2.pic=122");
      SendDataDisplay();
      if(flagImprime){
         Imprime=5;
         Envio_Esclavos();
         delay_ms(100);
         Imprime=10;
         flagImprime=0;
      }
      
      if(!Fugas){//Si selecciono un ciclo diferente al test de fugas
         EstadoBuzzer=OFF;
         IniciaCiclo=ON;
      
      if(!Precalentamiento){ 
         SuministroVapor=ON;
         aguaEnfriador=OFF;
         Paso=OFF;
         Desfogue=OFF;
         printf("Funcionamiento.t4f.txt=\" Ninguna\"");
         SendDataDisplay();
         printf("Funcionamiento.t5f.txt=\" PreCalentando\"");
         SendDataDisplay();
         ControlaPresion(); 
         Controlando=0;
         IniciaVacio=OFF;
         IniciaVacio2=OFF;
         codigoAlarma=10;
         if(codigoAnt!=codigoAlarma){
            codigoAnt=codigoAlarma;
            GuardaDatoCiclo();
         }
      }else{
         if(!Vacio){ //Si no ha realizado los pulsos de vacio
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            if(!flag_vac){//Ciclo inicial para purga de Bomba
               flag_vac=ON;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;
               //Paso=OFF;
               aguaEnfriador=ON;
               delay_ms(70);
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               IniciaVacio2=ON;
               PulsoPositivo=10; //Prueba para ver rendimiento de bomba de vacio************
            }
            printf("Funcionamiento.t4f.txt=\" Ninguna\"");
            SendDataDisplay();
            ControlaPresion();
            IniciaVacio=ON;
               
            if(PresionCamara>NivelVacio && PulsoPositivo==10){//Pulso Negativo
               flag_vacio=1;
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               outVacio=ON;
               Desfogue=OFF;
               Aire=OFF;                     
               Paso=OFF;
               if(PulsosVacio>=2)
                  aguaEnfriador=ON;
               else
                  aguaEnfriador=OFF;
               if(flag_pulso==0){
                  PulsosVacio++;
                  flag_pulso=1;
               }
               printf("Funcionamiento.t5f.txt=\" Pulso de Vacio %u\"",PulsosVacio);
               SendDataDisplay();
            }else{
               PulsoPositivo=5;
            }
            
            if(PulsosVacio>=CantidadPulsos && PulsoPositivo==5){//Si realiza los pulsos de vacio, continua con el proceso de calentamiento
               Vacio=ON;
               outVacio=OFF;
               BombaVacio=OFF;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;                                 
               IniciaVacio=OFF;
               Paso=ON;
               aguaEnfriador=OFF;
               PulsoPositivo=10;
            }
              
            if(PulsoPositivo==5){//Pulso Positivo
               tvacio=0;
               flag_pulso=0;
               printf("Funcionamiento.t5f.txt=\" Pulso Positivo %u\"",PulsosVacio);
               SendDataDisplay();
               if(ReservorioLleno)
                  BombaVacio=ON;
               else
                  BombaVacio=OFF;
               outVacio=OFF;
               Desfogue=OFF;
               Aire=OFF;                     
               Paso=ON;     
               AguaEnfriador=OFF;
               if(PresionCamara>=Atmosferica+30){
                  Paso=OFF;
                  PulsoPositivo=10;
               }  
            }
               
            if(tbomba>=120 && PulsosVacio==0){//Si no se ha generado ningun pulso de vacio despues de 120 segundos se genera alarma
               if(PresionCamara>Atmosferica){
                  AlarmaBomba=ON;
               }else{
                  tbomba=0;
                  IniciaVacio2=OFF;
               }
             }
               
            if(tvacio>=TiempoVacio){//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
               AlarmaVacio=ON;
            }
         }else{
            IniciaVacio=OFF;        
            if(Temperatura>=((float)Setpoint-0.3)){
               Esterilizando=ON;
            }
            
            if(Temperatura<((float)Setpoint-0.9) && !Desfogando){
               Esterilizando=OFF;
            }
            
            if(!Esterilizando){
               codigoAlarma=12;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               flag_vac=OFF;
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Calentando\"");
               SendDataDisplay();
               Controlando=1;
               Controla();
               Paso=ON;
               BombaVacio=OFF;
               outVacio=OFF;
               Aire=OFF;
               Desfogue=OFF;
               aguaEnfriador=OFF;
            }else{
               TiempoCiclo=0;
               if(mEst==0 && sEst==0){ 
                  if(desfoguelento){
                     Controlando=0;
                     if(PresionCamara<(Atmosferica+2)){
                        Secando=OFF;
                        DesfogueSuave=OFF;
                        //if(!abrirPuertaL){
                           BombaVacio=OFF;
                        //}
                        outVacio=OFF;
                        Aire=OFF;
                        Desfogue=ON;
                        Paso=OFF;
                        //Suministro_off;
                        aguaEnfriador=OFF;
                        if(Temperatura<=Tapertura){
                           ActivaAlarma(2);
                           printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                           SendDataDisplay();
                           printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                           SendDataDisplay();
                           Finalizo=ON;
                           codigoAlarma=14;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }                          
                           if(!flagImprimir){//Imprime Ciclo Finalizado
                              Imprime=28;
                              Envio_Esclavos();
                              delay_ms(100);
                              Imprime=10;
                              flagImprimir=1;
                           }
                        }
                        incrementaCiclos();
                     }else if(PresionCamara>(Atmosferica+10)){
                        codigoAlarma=13;
                        if(codigoAnt!=codigoAlarma){
                           codigoAnt=codigoAlarma;
                           GuardaDatoCiclo();                  
                        }
                        Desfogando=ON;
                        DesfogueSuave=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Controlando=0;
                        Desfogue=ON;
                        Paso=ON;
                        BombaVacio=OFF;
                        aguaEnfriador=OFF;
                        outVacio=OFF;
                        Aire=OFF; 
                        suministroVapor=OFF;
                     }
                  }else{                     
                     Paso=OFF;
                     Controlando=0;
                     Control=0;
                     if(PresionCamara<(Atmosferica+35)){  
                        if(sSec==0 && mSec==0){                          
                           codigoAlarma=15;
                           if(codigoAnt!=codigoAlarma){
                              codigoAnt=codigoAlarma;
                              GuardaDatoCiclo();                  
                           }
                           if(!DOORS){
                              Desfogue=OFF;
                              Paso=OFF;
                           }else{
                              Desfogue=ON;                              
                              Paso=OFF;
                           }    

                           BombaVacio=OFF;
                           aguaEnfriador=OFF;
                           outVacio=OFF;
                           if(PresionCamara<=Atmosferica-2){
                              Aire=ON;
                           }else{
                              Aire=OFF;
                           }
                           Desfogue=ON;
                           Paso=OFF;
                           suministroVapor=OFF;
                           Secando=OFF;
                           //if(Temperatura<=Tapertura){
                              ActivaAlarma(2);
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                              SendDataDisplay();
                              Finalizo=ON;
                              codigoAlarma=14;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                              }
                              if(!flagImprimir){//Imprime Ciclo Finalizado
                                 Imprime=28;
                                 Envio_Esclavos();
                                 delay_ms(100);
                                 Imprime=10;
                                 flagImprimir=1;
                              }
                           //}
                           incrementaCiclos();
                           Secando=OFF;
                        }else{
                           if(Secando){
                              printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                              SendDataDisplay();
                              printf("Funcionamiento.t5f.txt=\" Secando\"");
                              SendDataDisplay();
                           }
                           ControlaPresion();
                           Paso=OFF;
                           
                           if(PresionCamara>NivelVacio+5){//Si la presión de la camara es mayor al nivel de vacio
                              if(ReservorioLleno)
                                 BombaVacio=ON;
                              else
                                 BombaVacio=ON;
                              outVacio=ON;
                              //aguaEnfriador=ON;
                              Desfogue=OFF;
                              Aire=OFF;
                              Paso=OFF;
                           }
                           
                           if(PresionCamara<=NivelVacio+10){//Si alcanzo el nivel de vacio requerido
                              Secando=ON;
                           }
                           
                           if(PresionCamara<=NivelVacio){//Si alcanzo el nivel de vacio requerido
                              codigoAlarma=15;
                              if(codigoAnt!=codigoAlarma){
                                 codigoAnt=codigoAlarma;
                                 GuardaDatoCiclo();                  
                              }
                              Secando=ON;
                              EstadoAgua2=ON;
                              
                              if(ReservorioLleno)
                                 BombaVacio=ON;
                              else
                                 BombaVacio=OFF;
                              //aguaEnfriador=ON;
                              outVacio=ON;
                              Desfogue=OFF;
                              Aire=ON;                     
                              Paso=OFF;
                           }
                           
                           if(tEnfriador>=100){
                              aguaEnfriador=OFF;
                              estadoAgua2=OFF;
                           }else{
                              aguaEnfriador=ON;
                           }
                        }
                     }else if(PresionCamara>(Atmosferica+37)){
                        codigoAlarma=13;
                        if(codigoAnt!=codigoAlarma){
                           codigoAnt=codigoAlarma;
                           GuardaDatoCiclo();                  
                        }
                        Desfogando=ON;
                        printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                        SendDataDisplay();
                        printf("Funcionamiento.t5f.txt=\" Despresurizando\"");
                        SendDataDisplay();
                        Control=0;
                        Controlando=0;
                        Paso=OFF;
                        Desfogue=ON;
                        //if(ReservorioLleno)
                        //   BombaVacio=ON;
                        //else
                           BombaVacio=OFF;   
                        aguaEnfriador=ON;
                        outVacio=OFF;
                        Aire=OFF;
                        suministroVapor=OFF;
                     }
                  }
               }else{
                  codigoAlarma=16;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Esterilizando\"");
                  SendDataDisplay();
                  //Paso_on;
                  Control=1;
                  Controla();
               }
            }
         }
      } 
      }else{
      EstadoBuzzer=OFF;
      IniciaCiclo=ON;
         if(!flag_vac){
            codigoAlarma=11;
            if(codigoAnt!=codigoAlarma){
               codigoAnt=codigoAlarma;
               GuardaDatoCiclo();               
            }
            flag_vac=ON;
            if(ReservorioLleno)
               BombaVacio=ON;
            else
               BombaVacio=OFF;
            Desfogue=OFF;
            Aire=OFF;
            aguaEnfriador=OFF;
            Paso=OFF;
            suministroVapor=OFF;
            delay_ms(70);
            IniciaVacio2=ON;
            outVacio=ON;
         }
         
         if(tbomba>=120){
            if(PresionCamara>Atmosferica){
               AlarmaBomba=ON;
            }else{
               tbomba=0;
               IniciaVacio2=OFF;
            }
         }
         
         if(tvacio>=TiempoVacio){//Si excede el tiempo maximo para generar los pulsos de vacio genera alarma            
               AlarmaVacio=ON;
         }
         
         if(PresionCamara<vacioTest){
            BombaVacio=OFF;
            outVacio=OFF;
            Desfogue=OFF;
            Aire=OFF;
            aguaEnfriador=OFF;
            Paso=OFF;
            suministroVapor=OFF;
            Esterilizando=ON;
            IniciaVacio=OFF;
            tbomba=0;
         } 
         
         if(!Finalizo){
            if(!Esterilizando){
               if(codigoAlarma==8){
                  ActivaAlarma(2);
               }else{
                  if(ReservorioLleno)
                     BombaVacio=ON;
                  else
                     BombaVacio=OFF;
                  
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Vacio\"");
                  SendDataDisplay();
               }
            }else{
               if(PresionCamara>40){
                  printf("Funcionamiento.t4f.txt=\" Error Ciclo\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Finalizado\"");
                  SendDataDisplay();
                  codigoAlarma=8;
                  if(codigoAnt!=codigoAlarma){
                     codigoAnt=codigoAlarma;
                     GuardaDatoCiclo();                  
                  }
                  if(!flagImprimir){//Imprime Ciclo Finalizado
                     Imprime=26;
                     Envio_Esclavos();
                     delay_ms(100);
                     Imprime=10;
                     flagImprimir=1;
                  }
                  IniciaVacio=OFF;
                  tbomba=0;
                  BombaVacio=OFF;
                  outVacio=OFF;
                  Desfogue=OFF;
                  Aire=ON;
                  Paso=OFF;
                  suministroVapor=OFF;
                  aguaEnfriador=OFF;
                  ActivaAlarma(2);
                  incrementaCiclos();
                  Esterilizando=OFF;
               }else{
                  printf("Funcionamiento.t4f.txt=\" Ninguna\"");
                  SendDataDisplay();
                  printf("Funcionamiento.t5f.txt=\" Manteniendo Vacio\"");
                  SendDataDisplay();
               }
            }
         }
         
         if(mEst==0 && sEst==0){  
            if(PresionFinal<=40){
               printf("Funcionamiento.t4f.txt=\" Ninguna\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Finalizado\"");
               SendDataDisplay();
               codigoAlarma=14;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               if(!flagImprimir){//Imprime Ciclo Finalizado
                  Imprime=28;
                  Envio_Esclavos();
                  delay_ms(100);
                  Imprime=10;
                  flagImprimir=1;
               }
            }else{
               printf("Funcionamiento.t4f.txt=\" Error Ciclo\"");
               SendDataDisplay();
               printf("Funcionamiento.t5f.txt=\" Finalizado\"");
               SendDataDisplay();
               codigoAlarma=8;
               if(codigoAnt!=codigoAlarma){
                  codigoAnt=codigoAlarma;
                  GuardaDatoCiclo();                  
               }
               if(!flagImprimir){//Imprime Ciclo Finalizado
                  Imprime=26;
                  Envio_Esclavos();
                  delay_ms(100);
                  Imprime=10;
                  flagImprimir=1;
               }
            }
            IniciaVacio=OFF;
            tbomba=0;
            BombaVacio=OFF;
            outVacio=OFF;
            Desfogue=OFF;
            Aire=ON;
            Paso=OFF;
            suministroVapor=OFF;
            aguaEnfriador=OFF;
            ActivaAlarma(2);
            if(!Finalizo){
               Finalizo=ON;
               PresionFinal=PresionCamara;
            }
            incrementaCiclos();
            Esterilizando=OFF;
         }
      }
   }

   if(tguarda>=180){
      if(!Ciclo)
         GuardaDatoCiclo();
      tguarda=0;
   }

   if(Grafica==1){
      printf("add 12,0,%1.0f",Temperatura*0.5);
      SendDataDisplay();
      printf("add 12,1,%1.0f",PresionCamara*0.5);
      SendDataDisplay();
      printf("add 12,2,%1.0f",PresionPreCamara*0.5);
      SendDataDisplay();
      Grafica=0;
   }
   if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
   {                      
      Lectura=0;
      //printf("tuser.txt=\"%s\"",usuario);
      //SendDataDisplay();
      printf("t6f.txt=\"%3.1f\"",Temperatura);
      SendDataDisplay();
      printf("Funcionamiento.t7f.txt=\"%03LukPa\"",PresionCamara);
      SendDataDisplay();
      printf("Funcionamiento.t12f.txt=\"%03LukPa\"",PresionPreCamara);
      SendDataDisplay();
      printf("Funcionamiento.t9f.txt=\"%03u°C\"",Setpoint);
      SendDataDisplay();      
      printf("Funcionamiento.t10f.txt=\"%02u:%02u\"",mEst,sEst);
      SendDataDisplay();      
      printf("Funcionamiento.t11f.txt=\"%02u:%02u\"",mSec,sSec);
      SendDataDisplay();      
      
      printf("fecha.txt=\"%s Fecha:%02u/%02u/20%02u  Hora:%02u:%02u %2.1f°C\"",usuario,Dia,Mes,Year,Hora,Minuto,Tactual);
      SendDataDisplay();
   }
   }
}
