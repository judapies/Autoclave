
typedef struct{
   char Letra[15];
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
   
   if(sombreado==2)
   {
      sprintf(voltage, "%03u", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }
   else
   {
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
   
   if(x==6){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(GraficaVacio[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==7){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(CuadradoVacio[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==8){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(CuadradoCalentando[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==9){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(CuadradoEsterilizando[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==10){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(CuadradoDesfogando[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==11){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(CuadradoSecando[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
   if(x==12){
      glcd_line(4,54,4,15,ON);
      glcd_line(4,54,85,54,ON);
      glcd_line(4,35,18,51,ON);
      glcd_line(18,51,81,51,ON);
      glcd_line(81,51,89,37,ON);
   }
}

void CuadroVacio(short estado){
   if(estado){
      if(!PosVacio){
         glcd_fillscreen(OFF);
         glcd_imagen(7);
         PosVacio=ON;
         PosCalentando=OFF;
         PosEsterilizando=OFF;
         PosDesfogando=OFF;
         PosSecando=OFF;
      }
   }
}

void CuadroCalentando(short estado){
   if(estado){
      if(!PosCalentando){
         glcd_fillscreen(OFF);
         glcd_imagen(8);
         PosVacio=OFF;
         PosCalentando=ON;
         PosEsterilizando=OFF;
         PosDesfogando=OFF;
         PosSecando=OFF;
      }
   }
}

void CuadroEsterilizando(short estado){
   if(estado){
      if(!PosEsterilizando){
         glcd_fillscreen(OFF);
         glcd_imagen(9);
         PosVacio=OFF;
         PosCalentando=OFF;
         PosEsterilizando=ON;
         PosDesfogando=OFF;
         PosSecando=OFF;
      }
   }
}

void CuadroDesfogando(short estado){
   if(estado){
      if(!PosDesfogando){
         glcd_fillscreen(OFF);
         glcd_imagen(10);
         PosVacio=OFF;
         PosCalentando=OFF;
         PosEsterilizando=OFF;
         PosDesfogando=ON;
         PosSecando=OFF;
      }
   }
}

void CuadroSecando(short estado){
   if(estado){
      if(!PosSecando){
         glcd_fillscreen(OFF);
         glcd_imagen(11);
         PosVacio=OFF;
         PosCalentando=OFF;
         PosEsterilizando=OFF;
         PosDesfogando=OFF;
         PosSecando=ON;
      }
   }
}

void BorraStatus(){
   glcd_rect(0, 55, 127, 63, YES, OFF);
}

