#use rs232(baud=2400, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
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
//dato[0]=getc();
//dato[1]=getc();
return dato[0];// datos del lector (dato[0]==(0x51) && dato[1]==(0x15)
}

//////////////check////////////////

int8 check()
{
putc(0x57);putc(0xAB);putc(0x30);
delay_ms(100);
//dato[0]=getc();
return dato[0]; //El lector envia(0x14)
}

  
///////////////mount()///////////////

int8 mount()
{
putc(0x57);putc(0xAB);putc(0x31);
delay_ms(100);
//dato[0]=getc();
return dato[0];// El lector envia(0x14)
}

         
///////////////////name()/////////////////

void name(char nombre)
{
putc(0x57);putc(0xAB);putc(0x2F);putc(0x2F);
printf(nombre);
putc(0x00);
}         
////////////////create()/////////////////

int8 create()
{
putc(0x57);
putc(0xAB);
putc(0x34);
delay_ms(100);
//dato[0]=getc();
return dato[0];// El lector envia(0x14)
}            


void escribir(char Write)
{
////////////////setdata()///////////////////


putc(0x57);
putc(0xAB);
putc(0x3C);
putc(strlen(Write));
putc(0x00);
delay_ms(100);
//dato[0]=getc();// El lector envia (0x1E)


/////////////////writedata()////////////////


putc(0x57);
putc(0xAB);
putc(0x2D);
printf(Write);
delay_ms(100);
//dato[0]=getc();//El lector envia longitud de escritura(strlen(plan))
                 
 
///////////////////////update()/////////////


putc(0x57);
putc(0xAB);
putc(0x3D);
delay_ms(100);
//dato[0]=getc();//El lector envia (0x14)


}                      
                    
//////////////////////close()////////////

int8 close()
{
putc(0x57);
putc(0xAB);
putc(0x36);
putc(0x01);
delay_ms(100);
//dato[0]=getc();//El lector envia (0x14)
return dato[0];
}

