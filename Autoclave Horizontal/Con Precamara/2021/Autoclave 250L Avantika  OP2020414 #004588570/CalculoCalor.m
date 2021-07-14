%% JP BIOINGENIERIA SAS
%% Calculo de consumo de vapor
clc,clear
M=12; %Masa del Agua del reservorio en Kg
Cp=4.216; % Calor especifico del vapor a 134°C kJ/kg
dt=120-15; % Delta de temperatura del vapor
Q=M*Cp*dt; %Calor del vapor en kj
tiempo=1; % tiempo en segundos
W=Q/tiempo; % Potencia dada en kW
FlujoMasico=10e3/(Cp*dt);
