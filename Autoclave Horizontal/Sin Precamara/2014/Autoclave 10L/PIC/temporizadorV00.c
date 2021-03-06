// Manejo con cinco ventanas
// Con temporizador TMR1 y temporizaciones en tiempo real
// Pulsadores NC (botonera)
// Entradas de control NO
// Actualiza datos desde memoria interna EEPROM
// No hay configurado PLL(Debe estar en 1/4)
// Visualización inmediata al finalizar ciclo
// Modificación con realización de función trampa a partir de 25 seg de inicio.
// Realiza desfogues de 2 seg cada 30 seg.
#include <18F4550.h>
#DEVICE ADC=10
#use delay(clock=4000000)
#fuses xt,nowdt,nolvp
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOUSBDIV,NOPLL5,NOCPUDIV1,NOVREGEN// PLL5 para 20 MHz
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOUSBDIV//,NOPLL5,NOCPUDIV1,NOVREGEN// PLL5 para 20 MHz
#include <lcd.h>
#define   UP      !input(PIN_D2)
#define   LEFT   !input(PIN_D1)
#define   DOWN   !input(PIN_D3)
#define   RIGHT   !input(PIN_D0)

#define   DOOR   input(PIN_A0)   // Entrada de puerta abierta (NO)
#define   STOP   !input(PIN_A1)   // Entrada de parada de emergencia (NC)
#define   TERMST   input(PIN_A2)   // Entrada del termostato (NO)
#define   IN1_121   input(PIN_A3)   // Entrada de alarma 121 grados (NO)

#define   EV_on      output_bit(PIN_C0,1)
#define   EV_off      output_bit(PIN_C0,0)

#define   FINC_on      output_bit(PIN_C1,1)
#define   FINC_off   output_bit(PIN_C1,0)

#define   PUERTA_on   output_bit(PIN_C2,1)
#define   PUERTA_off   output_bit(PIN_C2,0)

#define   SSR_on      output_bit(PIN_A5,1)
#define   SSR_off      output_bit(PIN_A5,0)

#define   RLV2_on      output_bit(PIN_C6,1)  // RLV2 utilizada simultáneamente con
#define   RLV2_off     output_bit(PIN_C6,0)  // piloto de puerta (Puerta - C2)

//#define   AUX1_on      output_bit(PIN_C7,1)
//#define   RLV2_off   output_bit(PIN_C7,0)

#define   SAL_STOP_on   output_bit(PIN_C7,1)
#define   SAL_STOP_off   output_bit(PIN_C7,0)

signed int contador=0; // Contador de posiciones del cursor
                  // en la primera línea
unsigned int i=0, tmp, cursor_x=1, cursor_y=1, inicio=0, indice=0;
unsigned int ventana=1, solidos = 1, inicio_tmp=0, tiempos=0;
unsigned int estado_T_secado=0, estado_T_esteril=0;
unsigned int estado_trampa=0;   // Indica que no se ha realizado desfogue de trampa
unsigned int cont_trampas=0, flanco_trampa=0;
unsigned int estado_rampa=0;   // Indica que está realizando rampa
unsigned int fin=0;            // Indica si se encuentra en fin de ciclo
unsigned int matriz[2][10]={{5,0,0,0,0,0,0,0,0,8},{5,8,9,11,12,8,9,11,12,8}};
unsigned int proceso=0, minutos=0;
signed int tiempo1[4]={1,0,0,0};   // Tiempo de esterilización
signed int tiempo2[4]={1,0,0,0};   // Tiempo de secado
signed int hold_esteril=1, hold_secado=1;
unsigned long ContadorL=0, ContadorH=0;

#int_TIMER1
tempo1() {
   //set_timer1(15580);
//12489
   //set_timer1(53047);  // Se atrasa 1.5 seg en 10 min
   set_timer1(53068);  // (-21 en complemento)
   tiempos++;
   if(tiempos==20)      // Ha transcurrido un segundo
      {tiempos=0;
      // Decrementar tiempos
      if(estado_T_esteril==1)
         {if((tiempo1[0]==0)&&(tiempo1[1]==0)&&(tiempo1[2]==0)&&(tiempo1[3]==0))
            {estado_T_esteril=0;
            if(solidos==0)      // Si proceso es líquidos
               {estado_rampa=1;
               disable_interrupts(GLOBAL);}
            else      // Si proceso es sólidos
               {estado_T_secado=1;}
            }
         else
            {tiempo1[3]--;
            if(tiempo1[3]<0)
               {tiempo1[3]=9;
               tiempo1[2]--;
               if(tiempo1[2]<0)
                  {tiempo1[2]=5;
                  tiempo1[1]--;
                  if(tiempo1[1]<0)
                     {tiempo1[1]=9;
                     tiempo1[0]--;
                     if(tiempo1[0]<0)
                        {tiempo1[0]=0;
                        }
                     }
                  }
               }
            }
         }
      if(estado_T_secado==1)
         {SSR_off;   // Modificación 27 agosto 2013
         if((tiempo2[0]==0)&&(tiempo2[1]==0)&&(tiempo2[2]==0)&&(tiempo2[3]==0))
            {estado_T_secado=0;
            fin=1;
            disable_interrupts(GLOBAL);}
         else
            {EV_on;
            tiempo2[3]--;
            if(tiempo2[3]<0)
               {tiempo2[3]=9;
               tiempo2[2]--;
               if(tiempo2[2]<0)
                  {tiempo2[2]=5;
                  tiempo2[1]--;
                  if(tiempo2[1]<0)
                     {tiempo2[1]=9;
                     tiempo2[0]--;
                     if(tiempo2[0]<0)
                        {tiempo2[0]=0;
                        }
                     }
                  }
               }
            }
         }
      // Actualizar datos de ventanas 2 ó 3
      if(ventana==2)
         {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
         // Imprimir tiempo
         lcd_gotoxy(8,4);
         printf(lcd_putc,"%d%d",tiempo1[0],tiempo1[1]);
         printf(lcd_putc,":%d%d",tiempo1[2],tiempo1[3]);
         
         if(contador%2)
            {lcd_gotoxy(20,1);}
         else
            {lcd_gotoxy(1,1);}
         lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
         }
      if(ventana==3)
         {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
         // Imprimir tiempo
         lcd_gotoxy(8,4);
         printf(lcd_putc,"%d%d",tiempo2[0],tiempo2[1]);
         printf(lcd_putc,":%d%d",tiempo2[2],tiempo2[3]);
         
         if(contador%2)
            {lcd_gotoxy(20,1);}
         else
            {lcd_gotoxy(1,1);}
         lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
         }
      }
}

void mostrar_menu(int m)
{
lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
lcd_enviar(lcd_comando,LCD_CLEAR);
lcd_gotoxy(7,1);
printf(lcd_putc,"- Menu -    >>");
switch (m)
   {case 1:   lcd_gotoxy(2,2);
            printf(lcd_putc,"SELECCION DE CICLO");
            lcd_gotoxy(5,3);
            if(solidos==1)
               {lcd_gotoxy(5,3);}
            else
               {lcd_gotoxy(5,4);}
            lcd_enviar(lcd_dato,126);
            lcd_gotoxy(6,3);
            printf(lcd_putc," SOLIDOS");
            lcd_gotoxy(6,4);
            printf(lcd_putc," LIQUIDOS");
            break;
   case 2:      indice=1;
            lcd_gotoxy(1,1);
            printf(lcd_putc,"<<");
            lcd_gotoxy(3,2);
            printf(lcd_putc,"PROCESO ");
            if(solidos==1)
               {printf(lcd_putc,"SOLIDOS");}
            else
               {printf(lcd_putc,"LIQUIDOS");}
            lcd_gotoxy(2,3);
            printf(lcd_putc,"T. ESTERILIZACION");
            lcd_gotoxy(8,4);
            printf(lcd_putc,"%d%d",tiempo1[0],tiempo1[1]);
            printf(lcd_putc,":%d%d",tiempo1[2],tiempo1[3]);
            break;
   case 3:      indice=5;
            lcd_gotoxy(1,1);
            printf(lcd_putc,"<<");
            lcd_gotoxy(3,2);
            printf(lcd_putc,"PROCESO SOLIDOS");
            lcd_gotoxy(3,3);
            printf(lcd_putc,"TIEMPO DE SECADO");
            lcd_gotoxy(8,4);
            printf(lcd_putc,"%d%d",tiempo2[0],tiempo2[1]);
            printf(lcd_putc,":%d%d",tiempo2[2],tiempo2[3]);
            break;
   case 4:      //indice=5;
            lcd_gotoxy(1,1);
            printf(lcd_putc,"<<");
            lcd_gotoxy(3,2);
            printf(lcd_putc,"PROCESO LIQUIDOS");
            lcd_gotoxy(3,3);
            if(estado_rampa==1)
               {printf(lcd_putc,"DESPRESURIZANDO");
               }
            else
               {printf(lcd_putc,"DESPRESURIZACION");}
            lcd_gotoxy(7,4);
            printf(lcd_putc,"EN RAMPA");
            break;
   case 5:      lcd_gotoxy(1,1);
            printf(lcd_putc,"<<");
            lcd_gotoxy(19,1);
            printf(lcd_putc,"  ");

            if(inicio)
               //{if(hold_esteril)
               {if(estado_T_esteril)
                  {lcd_enviar(lcd_comando,LCD_CLEAR);
                  lcd_gotoxy(5,2);
                  printf(lcd_putc,"ESTERILIZANDO");
                  }
               else if(estado_T_secado)
                  {lcd_enviar(lcd_comando,LCD_CLEAR);
                  lcd_gotoxy(8,2);
                  printf(lcd_putc,"SECANDO");}
               else
                  {lcd_enviar(lcd_comando,LCD_CLEAR);
                  lcd_gotoxy(6,2);
                  printf(lcd_putc,"CALENTANDO");
                  lcd_gotoxy(8,3);
                  printf(lcd_putc,"CAMARA");}
                  lcd_gotoxy(1,1);
                  printf(lcd_putc,"<<");
               }
            else
               {lcd_gotoxy(3,2);
               printf(lcd_putc,"INICIAR PROGRAMA");
               lcd_gotoxy(10,3);
               printf(lcd_putc,"ON");
               lcd_gotoxy(10,4);
               //lcd_enviar(lcd_dato,126);
               printf(lcd_putc,"OFF");
               if(inicio==1)
                  {lcd_gotoxy(8,3);}
               else
                  {lcd_gotoxy(8,4);}
               lcd_enviar(lcd_dato,126);
               }
            /*lcd_gotoxy(3,2);
            printf(lcd_putc,"INICIAR PROGRAMA");
            lcd_gotoxy(10,3);
            printf(lcd_putc,"ON");
            lcd_gotoxy(10,4);
            //lcd_enviar(lcd_dato,126);
            printf(lcd_putc,"OFF");
            if(inicio==1)
               {lcd_gotoxy(8,3);}
            else
               {lcd_gotoxy(8,4);}
            lcd_enviar(lcd_dato,126);*/
            break;
   default:   break;
   }
}

void funcionStop (void)
   {SAL_STOP_on;
   //PUERTA_on;
   EV_on;
   proceso=0;
   inicio=0;   fin=0;
   estado_T_esteril=0;   estado_T_secado=0;
   hold_esteril=1;      hold_secado=1;
   estado_rampa=0;

   contador=0;      cursor_x=1;      cursor_y=1;   inicio=0;
   indice=0;      ventana=1;      inicio_tmp=0;

   SSR_off;
   disable_interrupts(GLOBAL);
   lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
   lcd_enviar(lcd_comando,LCD_CLEAR);
   lcd_gotoxy(3,2);
   printf(lcd_putc,"!! PARADA  DE !!");
   lcd_gotoxy(3,3);
   printf(lcd_putc,"!! EMERGENCIA !!");
   while(STOP){
   if(DOOR)         // ¿Puerta abierta?
      {PUERTA_on;RLV2_on;}      // Encender piloto de puerta abierta
   else
      {PUERTA_off;RLV2_off;}}
tiempo1[0]=read_eeprom(0);delay_ms(25);
tiempo1[1]=read_eeprom(1);delay_ms(25);
tiempo1[2]=read_eeprom(2);delay_ms(25);
tiempo1[3]=read_eeprom(3);delay_ms(25);
tiempo2[0]=read_eeprom(4);delay_ms(25);
tiempo2[1]=read_eeprom(5);delay_ms(25);
tiempo2[2]=read_eeprom(6);delay_ms(25);
tiempo2[3]=read_eeprom(7);delay_ms(25);
   mostrar_menu(ventana);
   lcd_gotoxy(1,1);
   lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
   }

void main ()
{signed int j, k;
unsigned int leer121=1;

output_c(0);
set_tris_b(0x00);
//set_tris_a(0xF8);   // 1111 1000
set_tris_a(0x1F);   // 0001 1111
set_tris_c(0);
set_tris_d(0x1F);
output_c(0);
SAL_STOP_off;
//setup_adc(ADC_OFF);
//setup_adc_ports(NO_ANALOGS);
setup_adc(ADC_CLOCK_INTERNAL );
//setup_adc_ports( ALL_ANALOG );
setup_adc_ports(AN0);
set_adc_channel(0);
delay_us(10);

  lcd_init();
  lcd_enviar(lcd_comando,lcd_home);
  lcd_enviar(lcd_comando,lcd_clear);

   lcd_gotoxy(1,1);
   printf(lcd_putc,"AUTOCLAVE 55 LITROS.");
   lcd_gotoxy(6,2);
   printf(lcd_putc,"AUTOMATICA");
   lcd_gotoxy(6,3);
   printf(lcd_putc,"JP INGLOBAL");
   lcd_gotoxy(5,4);
   printf(lcd_putc,"REF. JPAD 55.");
for(i=0;i<10;i++) // Retardo de un segundo con revisión de botón STOP
   {if(Stop)
      {funcionStop();}
   else
      {SAL_STOP_off;}
   delay_ms(100);
   }


// Actualizar datos de T1 y T2 desde memoria interna
tiempo1[0]=read_eeprom(0);delay_ms(25);
tiempo1[1]=read_eeprom(1);delay_ms(25);
tiempo1[2]=read_eeprom(2);delay_ms(25);
tiempo1[3]=read_eeprom(3);delay_ms(25);
tiempo2[0]=read_eeprom(4);delay_ms(25);
tiempo2[1]=read_eeprom(5);delay_ms(25);
tiempo2[2]=read_eeprom(6);delay_ms(25);
tiempo2[3]=read_eeprom(7);delay_ms(25);

SSR_off;
RLV2_off;
lcd_enviar(lcd_comando,lcd_clear);
int x=0;

ventana=(contador>>1)+1;
mostrar_menu(ventana);
lcd_gotoxy(1,1);
lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
enable_interrupts(INT_TIMER1);
set_timer1(53047);

  for(;;)
  {
   if(!RIGHT)
      {delay_ms(80);
      if(cursor_y==1)
         {if(contador==5)
            {contador=7;}
         if((contador==3)&&(solidos==0))
            {contador=5;}
         contador++;
         if(contador>9)
            {contador=9;}
         ventana=(contador>>1)+1;
         if(contador%2)
            {lcd_gotoxy(20,1);}
         else
            {mostrar_menu(ventana);
            lcd_gotoxy(1,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         }
      else
         {if(ventana==1)
            {cursor_y=1;contador=1;mostrar_menu(1);
            lcd_gotoxy(20,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         if(ventana==2)
            {indice++;
            if(indice>4)
               {cursor_y=1;contador=3;mostrar_menu(2);
               lcd_gotoxy(20,1);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            else
               {lcd_gotoxy(matriz[1][indice],4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
         if(ventana==3)
            {indice++;
            if(indice>8)
               {cursor_y=1;contador=5;mostrar_menu(3);
               lcd_gotoxy(20,1);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            else
               {lcd_gotoxy(matriz[1][indice],4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
         if(ventana==5)
            {if(inicio_tmp==1)
               {inicio=1;
               //estado_T_esteril=1;
               }
            cursor_y=1;contador=9;mostrar_menu(5);
            lcd_gotoxy(20,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         }
      }

   if(!LEFT)
      {delay_ms(80);
      if(cursor_y==1)
         {if(contador==6)
            {contador=4;}
         if((contador==8)&&(solidos==1))
            {contador=6;}
         contador--;
         if(contador<0)
            {contador=0;}
         ventana=(contador>>1)+1;
         if(contador%2)
            {mostrar_menu(ventana);
            lcd_gotoxy(20,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         else
            {lcd_gotoxy(1,1);}
         }
      else
         {if(ventana==1)
            {cursor_y=1;contador=0;mostrar_menu(1);
            lcd_gotoxy(1,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         if(ventana==2)
            {indice--;
            if(indice==0)
               {cursor_y=1;contador=2;mostrar_menu(2);
               lcd_gotoxy(1,1);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            else
               {lcd_gotoxy(matriz[1][indice],4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
         if(ventana==3)
            {indice--;
            if(indice==4)
               {cursor_y=1;contador=4;mostrar_menu(3);
               lcd_gotoxy(1,1);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            else
               {lcd_gotoxy(matriz[1][indice],4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
         if(ventana==5)
            {if(inicio_tmp==1)
               {inicio=1;
               //estado_T_esteril=1;
               }
            cursor_y=1;contador=8;mostrar_menu(5);
            lcd_gotoxy(1,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
         }
      }

   if((!DOWN)&&(inicio==0))
      {delay_ms(80);
      if(ventana==1)
         {if(cursor_y==1)
            {if(solidos==1)
               {cursor_y=3;
               lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
               lcd_gotoxy(5,3);lcd_enviar(lcd_dato,126);
               lcd_gotoxy(5,4);printf(lcd_putc," ");
               lcd_gotoxy(5,3);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
               }
            else
               {cursor_y=4;
               lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
               lcd_gotoxy(5,4);lcd_enviar(lcd_dato,126);
               lcd_gotoxy(5,3);printf(lcd_putc," ");
               lcd_gotoxy(5,4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
               }
            }
         else
            {cursor_y++;
            if(cursor_y>4)
               {cursor_y=4;}
            else
               {solidos=0;
               lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
               lcd_gotoxy(5,4);lcd_enviar(lcd_dato,126);
               lcd_gotoxy(5,3);printf(lcd_putc," ");
               lcd_gotoxy(5,4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
               }   
            }
         }
      if(ventana==2)
         {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
         lcd_gotoxy(matriz[1][indice],4);
         if(cursor_y!=1)
            {tiempo1[indice-1]--;
            if(tiempo1[indice-1]<0)
               {if(indice==3)
                  {tiempo1[indice-1]=5;}
               else
                  {tiempo1[indice-1]=9;}
               }
            }
         cursor_y=4;
         printf(lcd_putc,"%d",tiempo1[indice-1]);
         lcd_gotoxy(matriz[1][indice],4);
         lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
         }
      if(ventana==3)
         {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
         lcd_gotoxy(matriz[1][indice],4);
         if(cursor_y!=1)
            {tiempo2[indice-5]--;
            if(tiempo2[indice-5]<0)
               {if(indice==7)
                  {tiempo2[indice-5]=5;}
               else
                  {tiempo2[indice-5]=9;}
               }
            }
         cursor_y=4;
         printf(lcd_putc,"%d",tiempo2[indice-5]);
         lcd_gotoxy(matriz[1][indice],4);
         lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
         }
      if(ventana==5)
         {if(cursor_y==1)
            {if(inicio==0)
               {cursor_y=4;
               lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
               lcd_gotoxy(8,4);lcd_enviar(lcd_dato,126);
               lcd_gotoxy(8,3);printf(lcd_putc," ");
               lcd_gotoxy(8,4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
               }
            }
         else
            {cursor_y++;
            if(cursor_y>4)
               {cursor_y=4;}
            else
               {inicio_tmp=0;
               lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
               lcd_gotoxy(8,4);lcd_enviar(lcd_dato,126);
               lcd_gotoxy(8,3);printf(lcd_putc," ");
               lcd_gotoxy(8,4);
               lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
               }   
            }
         }
      }

   if(!UP)
      {delay_ms(80);   // 80X4=320 ms
      if(ventana==1)
         {if(cursor_y==4)
            {cursor_y=3;
            solidos=1;
            lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
            lcd_gotoxy(5,3);lcd_enviar(lcd_dato,126);
            lcd_gotoxy(5,4);printf(lcd_putc," ");
            lcd_gotoxy(5,3);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
            }
         }
      if(ventana==2)
         {if(cursor_y!=1)
            {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
            lcd_gotoxy(matriz[1][indice],4);
            tiempo1[indice-1]++;
            if(indice==3)
               {if(tiempo1[indice-1]>5)
                  {tiempo1[indice-1]=0;}}
            else
               {if(tiempo1[indice-1]>9)
                  {tiempo1[indice-1]=0;}}
            cursor_y=4;
            printf(lcd_putc,"%d",tiempo1[indice-1]);
            lcd_gotoxy(matriz[1][indice],4);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
      if(ventana==3)
         {if(cursor_y!=1)
            {lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
            lcd_gotoxy(matriz[1][indice],4);
            tiempo2[indice-5]++;

            if(indice==7)
               {if(tiempo2[indice-5]>5)
                  {tiempo2[indice-5]=0;}}
            else
               {if(tiempo2[indice-5]>9)
                  {tiempo2[indice-5]=0;}}
            cursor_y=4;
            printf(lcd_putc,"%d",tiempo2[indice-5]);
            lcd_gotoxy(matriz[1][indice],4);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);}
            }
      if(ventana==5)
         {if(cursor_y==4)
            {cursor_y=3;
            inicio_tmp=1;
            lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
            lcd_gotoxy(8,3);lcd_enviar(lcd_dato,126);
            lcd_gotoxy(8,4);printf(lcd_putc," ");
            lcd_gotoxy(8,3);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
            // Actualizar datos de T1 y T2 en memoria interna
            write_eeprom(0,tiempo1[0]);delay_ms(25);
            write_eeprom(1,tiempo1[1]);delay_ms(25);
            write_eeprom(2,tiempo1[2]);delay_ms(25);
            write_eeprom(3,tiempo1[3]);delay_ms(25);
            write_eeprom(4,tiempo2[0]);delay_ms(25);
            write_eeprom(5,tiempo2[1]);delay_ms(25);
            write_eeprom(6,tiempo2[2]);delay_ms(25);
            write_eeprom(7,tiempo2[3]);delay_ms(25);
            }
         }
      }
/* //No hay SW de puerta ni termostato
   if(DOOR)         // ¿Puerta abierta?
      {PUERTA_on;RLV2_on;      // Encender piloto de puerta abierta
      SSR_off;}      // Apagar resistencia
   else
      {PUERTA_off;RLV2_off;}
   if(TERMST)
      {SSR_off;}*/

   if(inicio)
      {proceso=1;
      leer121=1;
      if(!DOOR&&!estado_T_secado)// Modif 27 agosto 2013
         {SSR_on;}
      }
   else
      {EV_off;}

// Realización de función trampa cuando contador H se ha cumplido
   /*if((estado_trampa==0)&&(flanco_trampa==1))
      {if((ContadorH%2)==0)         // Cada 30 seg realizar trampa
         {EV_on;delay_ms(500);       // Abrir electroválvula 2 segundos
         EV_off;  //delay_ms(250);   // Cerrar electroválvula
         }
      }*/

   if(proceso==1)
      {ContadorL++;
      if(ContadorL>52174)  // 52174 = 15 segundos de retardo
         {ContadorH++;
         ContadorL=0;
         if(ContadorH>=100)  // (H/4) 25 minutos de retardo aproximadamente
         ContadorH=0;
         flanco_trampa=1;}
      
      if(leer121==1)
         {if(IN1_121&&hold_esteril)
            {enable_interrupts(GLOBAL);
            leer121=0;
            estado_trampa=1;
//////////////////////////////////
            estado_T_esteril=1;
            cursor_y=1;contador=3;ventana=2;mostrar_menu(2);
            lcd_gotoxy(20,1);
            lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
//////////////////////////////////
            hold_esteril=0;
            }
         }
      }
   else
      {disable_interrupts(GLOBAL);}
   if(STOP)
      {funcionStop();}
   else
      {SAL_STOP_off;}

   if(fin)
      {SSR_off;
      FINC_on;
      EV_off;
      lcd_enviar(lcd_comando,LCD_CURSOR_OFF);
      lcd_enviar(lcd_comando,LCD_CLEAR);
      //lcd_gotoxy(4,1);
      //printf(lcd_putc,"**************");
      lcd_gotoxy(7,2);
      printf(lcd_putc,"PROCESO");
      lcd_gotoxy(6,3);
      printf(lcd_putc,"FINALIZADO");
      //lcd_gotoxy(4,4);
      //printf(lcd_putc,"**************");
      while(!stop){
      if(DOOR)         // ¿Puerta abierta?
         {PUERTA_on;RLV2_on;}      // Encender piloto de puerta abierta
      else
         {PUERTA_off;RLV2_off;}
      }
      //if(stop)
         {funcionStop();}
      FINC_off;
      }

   if(estado_rampa==1)
      {ventana=4;
      mostrar_menu(ventana);
      SSR_off;
      //for(i=0;i<90;i++)// 15 minutos de rampa (1/4 del PLL)
//
      for(minutos=0;minutos<16;minutos++)
         {for(i=0;i<21;i++)// 1.5 minutos de rampa (1/4 del PLL)
            {EV_on;
            for(k=0;k<1;k++)      // Retardo de k segundos (con límite j<4)
               //{for(j=0;j<4;j++)   // Retardo de 1 segundo
               {for(j=0;j<1;j++)   // Retardo de 1/4 segundo
                  {delay_ms(62);
                  if(stop)
                     {funcionStop();}
                  if(DOOR)         // ¿Puerta abierta?
                     {PUERTA_on;RLV2_on;      // Encender piloto de puerta abierta
                     SSR_off;}      // Apagar resistencia
                  else
                     {PUERTA_off;RLV2_off;}
                  }
                  if(!inicio){goto salida;}
               }
            EV_off;
            for(k=0;k<2;k++)      // Retardo de k segundos
               {for(j=0;j<4;j++)   // Retardo de 1 segundo
                  {delay_ms(62);
                  if(stop)
                     {funcionStop();}
                  if(DOOR)         // ¿Puerta abierta?
                     {PUERTA_on;RLV2_on;      // Encender piloto de puerta abierta
                     SSR_off;}      // Apagar resistencia
                  else
                     {PUERTA_off;RLV2_off;}
                  }
                  if(!inicio){goto salida;}
               }
            }
         }
//
      estado_rampa=0;
      fin=1;
      }
salida:
      if(hold_secado&&estado_T_secado)
         {cursor_y=1;contador=5;ventana=3;mostrar_menu(3);
         lcd_gotoxy(20,1);
         lcd_enviar(lcd_comando,LCD_CURSOR_BLINK);
         hold_secado=0;}
      //output_c(ventana);
  }
}

/* Arreglos por resolver

*/
