#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
byte dato[2];

//////////////reset////////////////

void reset()
{
putc(0x57);putc(0xAB);putc(0x05);
delay_ms(200);
}

//////////////set_usb////////////////

int8 set_usb()
{
putc(0x57);putc(0xAB);putc(0x15);putc(0x06);
dato[0]=getc();
//dato[1]=getc();
delay_ms(100);
return dato[0];// datos del lector (dato[0]==(0x51) && dato[1]==(0x15)
}

//////////////check////////////////

int8 check()
{
putc(0x57);putc(0xAB);putc(0x30);
dato[0]=getc();
delay_ms(100);
return dato[0]; //El lector envia(0x14)
}

  
///////////////mount()///////////////

int8 mount()
{
putc(0x57);putc(0xAB);putc(0x31);
dato[0]=getc();
delay_ms(100);
return dato[0];// El lector envia(0x14)
}

         
///////////////////name()/////////////////

void name(char nombre[])
{
putc(0x57);putc(0xAB);putc(0x2F);putc(0x2F);
printf("CE%02u%02u%02u.TXT",rxbuf[4],rxbuf[5],rxbuf[13]);
putc(0x00);
delay_ms(100);
}         
////////////////create()/////////////////

int8 create()
{
putc(0x57);
putc(0xAB);
putc(0x34);
dato[0]=getc();
delay_ms(100);
return dato[0];// El lector envia(0x14)
}            


void escribir(char Write[]){
////////////////setdata()///////////////////
putc(0x57);
putc(0xAB);
putc(0x3C);
putc(strlen(Write));
putc(0x00);
delay_ms(10);
//dato[0]=getc();// El lector envia (0x1E)
/////////////////writedata()////////////////
putc(0x57);
putc(0xAB);
putc(0x2D);
printf(Write);
delay_ms(10);
//dato[0]=getc();//El lector envia longitud de escritura(strlen(plan)) 
///////////////////////update()/////////////
putc(0x57);
putc(0xAB);
putc(0x3D);
delay_ms(10);
//dato[0]=getc();//El lector envia (0x14)
}                      

void writeInt8(int8 datoint){
////////////////setdata()///////////////////
putc(0x57);
putc(0xAB);
putc(0x3C);
putc(0x02);
putc(0x00);
delay_ms(10);
//dato[0]=getc();// El lector envia (0x1E)
/////////////////writedata()////////////////
putc(0x57);
putc(0xAB);
putc(0x2D);
printf("%02u",datoint);
//dato[0]=getc();//El lector envia longitud de escritura(strlen(plan)) 
///////////////////////update()/////////////
putc(0x57);
putc(0xAB);
putc(0x3D);
delay_ms(10);
//dato[0]=getc();//El lector envia (0x14)
}                      

void writeInt16(int16 datoint){
////////////////setdata()///////////////////
putc(0x57);
putc(0xAB);
putc(0x3C);
putc(0x03);
putc(0x00);
delay_ms(10);
//dato[0]=getc();// El lector envia (0x1E)
/////////////////writedata()////////////////
putc(0x57);
putc(0xAB);
putc(0x2D);
printf("%03Lu",datoint);
//dato[0]=getc();//El lector envia longitud de escritura(strlen(plan)) 
///////////////////////update()/////////////
putc(0x57);
putc(0xAB);
putc(0x3D);
delay_ms(10);
//dato[0]=getc();//El lector envia (0x14)
}                      

void writeFloat(float datoint){
////////////////setdata()///////////////////
putc(0x57);
putc(0xAB);
putc(0x3C);
if(datoint<10)
   putc(0x03);
else if(datoint<100)
   putc(0x04);
else
   putc(0x05);
putc(0x00);
delay_ms(10);
//dato[0]=getc();// El lector envia (0x1E)
/////////////////writedata()////////////////
putc(0x57);
putc(0xAB);
putc(0x2D);
printf("%2.1f",datoint);
//dato[0]=getc();//El lector envia longitud de escritura(strlen(plan)) 
///////////////////////update()/////////////
putc(0x57);
putc(0xAB);
putc(0x3D);
delay_ms(10);
//dato[0]=getc();//El lector envia (0x14)
}

//////////////////////close()////////////

int8 close()
{
putc(0x57);
putc(0xAB);
putc(0x36);
putc(0x01);
dato[0]=getc();//El lector envia (0x14)
delay_ms(100);
return dato[0];
}

