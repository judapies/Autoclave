////////////////////////////////////////////////////////////////////////////////////
//                           LCD.C                                                //
//                FUNCIONES PARA GESTION DE DISPLAY LCD                           //
//         8BITS, FUENTE 5X7, 2,3,4 LINEAS X 16, 20 CARACTERES:                   //
//       lcd_init() Debe ser invocada antes que las otras funciones.              //
//       lcd_putc(c) Visualiza c en la siguiente posición del display.            //
//                Caracteres especiales de control:                               //
//                      \f Borra display                                          //
//                      \n Salta a la siguiente línea                             //
//                      \b Retrocede una posición.                                //
//       lcd_gotoxy(x,y) Selecciona una nueva posición de escritura en el display.//
//                  donde x es el caracter de 1 a 16 o 20 según Display,          //
//                  donde y   es el número de la línea de 1 a 4                   //
//                   (la esquina superior izquierda es 1,1)                       //
//       lcd_getc(x,y) Devuelve el caracter de la posición x,y del display.       //
//      NOTA: Para emplear display de N*16 o N*20 Carácteres Por Línea            //
//       poner los valores correspondientes en lcd (quitar los comentarios abajo) //
//      PINES DE CONTROL RA0(rs(pin4)),RA1(rw(pin5)),RA3(en(pin6))                //
//      PINES DE DATOS DE RB0(db0(pin7)) A RB7(db7(pin14))                        //
//                  MODIF.: ING. FRANK GIRALDO - UD                               //
//                  MODIF.: ING. FAIVER H. TRUJILLO                               //
////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------//
#bit lcd_enable = 0XF83.7   // Puerto A(0XF80) pin #2
#bit lcd_rw = 0XF83.6      // Puerto A(0XF80) pin #1
#bit lcd_rs = 0XF83.5      // Puerto A(0XF80) pin #0
#byte lcd_b = 0XF81 //portb          B(0XF81)
int line;
//
//CONFIGURACION LCD 4*16 LINEAS
//#define LCD_LINEA1 0x80   // Direccion de memoria para la primera linea
//#define LCD_LINEA2 0x40   // Direccion de memoria para la segunda linea
//#define LCD_LINEA3 0x10   // Direccion de memoria para la tercera linea
//#define LCD_LINEA4 0x50   // Direccion de memoria para la cuarta linea
//
//
//CONFIGURACION LCD 4*20 LINEAS
#define LCD_LINEA1 0x80 // Direccion de memoria para la primera linea
#define LCD_LINEA2 0x40 // Direccion de memoria para la segunda linea
#define LCD_LINEA3 0x14 // Direccion de memoria para la tercera linea
#define LCD_LINEA4 0x54 // Direccion de memoria para la cuarta linea
//
//
#define LCD_DATO 1
#define LCD_COMANDO 0
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_DISPLAY_OFF 0x08
#define LCD_DISPLAY_ON 0x0C
#define LCD_CURSOR_ON 0x0E
#define LCD_CURSOR_OFF 0x0C
#define LCD_CURSOR_BLINK 0x0F
#define LCD_INC 0x06
#define LCD_DEC 0x04
#define LCD_CURSOR_SHIFT_LEFT 0x10
#define LCD_CURSOR_SHIFT_RIGHT 0x14
#define LCD_DISPLAY_SHIFT_RIGHT 0x1C
#define LCD_DISPLAY_SHIFT_LEFT 0x18
#define LCDCGRAM 0x40
//
int lcd_leer()
{
int valor;
set_tris_d(0x1F);
set_tris_b(0xFF);

lcd_rw = 1;
delay_cycles(1);
lcd_enable = 1;
delay_cycles(1);
valor = lcd_b;
lcd_enable = 0;
delay_us(40);

set_tris_b(0x00);
return valor;
}
//
void lcd_size(int ln, int ch){

}
//
void lcd_enviar(int dir, int valor){
set_tris_d(0x1F);
set_tris_b(0x00);

lcd_rs = 0;
while( bit_test(lcd_leer(),7) );
lcd_rs = dir;
delay_cycles(1);
lcd_rw = 0;
delay_cycles(1);
lcd_enable = 0;
lcd_b = valor;
delay_cycles(1);
lcd_enable = 1;
delay_us(2);
lcd_enable = 0;
delay_us(40);
}
//
void lcd_init()
{
int i;
set_tris_d(0x1F);
set_tris_b(0x00);

lcd_enable = 0;
lcd_rw = 0;
lcd_rs = 0;
delay_ms(15);

for(i=0; i<3; i++)
{
lcd_enviar(LCD_COMANDO,0x38);
delay_ms(5);
}
lcd_enviar(LCD_COMANDO,LCD_DISPLAY_ON);
lcd_enviar(LCD_COMANDO,0x06);
lcd_enviar(LCD_COMANDO,LCD_CLEAR);
lcd_enviar(LCD_COMANDO,LCD_HOME);
line=0;
}
//
void lcd_gotoxy( byte x, byte y) {
byte dir;
dir=0;
line=y;
if(x==0)
   x=1;
else{
}
if(y!=0){
      if(y==1){
      dir=LCD_LINEA1;
      dir+=x-1;
            }
      else{   if(y==2){
            dir=LCD_LINEA2;
            dir+=x-1;
                  }
                   else{   if(y==3){
                        dir=LCD_LINEA3;
                        dir+=x-1;
                              }
                        else{   if(y==4){
                              dir=LCD_LINEA4;
                              dir+=x-1;
                                    }
                           }
                     }
         }
      }
lcd_enviar(LCD_COMANDO,0x80|dir);
}
//
void lcd_putc( char c) {
switch (c) {
case '\f' : lcd_enviar(LCD_COMANDO,0x01);
         delay_ms(2);
         break;
case '\n' : ++line;
         if(line>=1 || line<=4)
            lcd_gotoxy(1,line);
         else{
            line=0;
            lcd_gotoxy(1,1);
            }
         break;
case '\b' : lcd_enviar(LCD_COMANDO,0x10);
         break;
default :   lcd_enviar(LCD_DATO,c);
         break;
}
}
//
char lcd_getc( int x, int y) {
char valor;
lcd_gotoxy(x,y);

lcd_rs = 1;
valor = lcd_leer();
lcd_rs = 0;
return valor;
}
