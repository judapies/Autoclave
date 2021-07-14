%% JP BIOINGENIERIA SAS
%% Calculo de consumo de vapor
clc,clear
M=150;
Cp=4.19;
dt=134-15;
Q=M*Cp*dt;
hf=2162;
h=0.5;
ws=Q/(hf*h);
%% Velocidad con caida de presion
V=0.597939; %134°C
%V=0.865341; %121°C
u=0.0286;
d=0.02096;
l=0.5;
dp=620;
v=sqrt(dp*2*d*V/(u*l));
%% Diametro de Tuberia
vel=40;
ms=80;
D=2*sqrt((ws*V)/(3600*pi*vel));