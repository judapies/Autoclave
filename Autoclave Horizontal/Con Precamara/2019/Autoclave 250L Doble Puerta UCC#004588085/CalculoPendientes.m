%% JP BIOINGENIERIA SAS
%% Calculo pendientes 4-20mA
clc,clear
R=240;
V=5;
Imin=4e-3;
Imax=20e-3;
ADCl=1023*(Imin*R)/V;
ADCH=1023*(Imax*R)/V;
Pmax=58.0151;
Pmin=0;
m=(Pmax-Pmin)/(ADCH-ADCl);
b=Pmax-(m*ADCH);
ADC=200;
P=m*ADC+b;
G=1+(49.4e3/328);
%326=1
%328=2