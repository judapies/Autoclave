// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave con Calderin
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// JP Inglobal. 2015

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#use RS232(BAUD=19200, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7, stream=Printer)
#include <LCD420.c>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Adafruit_ThermalL.h>
//#include <Adafruit_Thermal.h>

// Definici?n de Entradas
#define   PRES          !input(PIN_D7)

// Definici?n de Salidas
#define   Resistencia_on     output_bit(PIN_C0,1)
#define   Resistencia_off    output_bit(PIN_C0,0)

void main()
{
   lcd_init();
   enable_interrupts(global);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"  IMPRESORA   ");
   delay_ms(2000);
   delay_ms(100);
      putc(27);
      putc(64);
      delay_us(10);
      lcd_gotoxy(1,1);
      printf(lcd_putc,"  Imprimiendo   ");
      putc(10);
      fprintf(printer,"Hola Buenas tardes como estan\n");
      delay_ms(1000);
      fprintf(printer,"Prueba de impresion\n");

   while(true){
      
   }
}
