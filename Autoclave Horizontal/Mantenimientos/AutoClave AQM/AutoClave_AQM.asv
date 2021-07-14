%% Comprotamiento de Auto Clave AQM
%% Proceso # 2
clc,clear
tiempo = [1054 1059 1102:1117 1119:1127 1130:3:1148 1148 1149]-1054;

Tmaquina = [91.1 98.4 120.4 121.3 121.2 121.3 121.3 121.3 121.3 121.4 121.4 121.4 121.4 121.4 121.4 121.3...
    121.2 121.2 121.4 121.3 121.4 121.2 121.3 121.4 121.3 121.4 121.4 121.4 121.4 121.3 121.4 121.4 121.3...
    122.1 117 110.1];

Tcontrol = [87 97 117 118 118 119 120 120 120 120 120 120 121 121 121 121 121 121 121 121 121 120 120 ...
    120 121 121 121 121 121 121 121 121 121 115 109 106];

Presion = [0 8 18 19 19 20 21 21 21 21 21 21 22 22 22 22 22 22 22 22 22 22 22 22 21 22 22 22 22 22 22 22 22 10 2 0];
subplot(2,1,1)
plot(tiempo,Tmaquina,'-or',tiempo,Tcontrol,'-xb');legend('Temperatura de la maquina','Temperatura del modulo de control')
title('Temperatura medida');xlabel('Tiempo (minutos)');ylabel('Temperatura (°C) ')
grid
subplot(2,1,2)
plot(tiempo,Presion,'-sg'),title('Presión de la Auto Clave');xlabel('Tiempo (minutos)');ylabel('Presión (PSI) ')
grid
