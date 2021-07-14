%% Datos AutoClave Mccain. Corpoica
%% Fecha: 25 de Junio de 2013
%% Primeara toma de Datos con ajuste de R=1.8omhs, Valor Medido de
%% R=2.6ohms
clc,clear
Datos = xlsread('Datos_Mccain.xlsx');
%% Tiempo de Esterilización 10 minutos
Temperatura = Datos(:,2);
Presion = Datos(:,3);
subplot(221)
plot(Temperatura,'-sr')
title('Primera Muestra de Datos')
xlabel('Tiempo (Minutos)')
ylabel('Temperatura °C')
grid
subplot(222)
plot(Presion,'-sg')
title('Primera Muestra de Datos')
xlabel('Tiempo (Minutos)')
ylabel('Presión PSI')
grid

%% Segunda toma de Datos con ajuste de R=2.2ohms||2.2ohms, Valor Medido de
%% R=1.8ohms
clc,clear
Datos = xlsread('Datos_Mccain.xlsx',2);
%% Tiempo de Esterilización 5 minutos
Temperatura = Datos(:,2);
Presion = Datos(:,3);
subplot(223)
plot(Temperatura,'-sb')
title('Segunda Muestra de Datos')
xlabel('Tiempo (Minutos)')
ylabel('Temperatura °C')
grid
subplot(224)
plot(Presion,'-sk')
title('Segunda Muestra de Datos')
xlabel('Tiempo (Minutos)')
ylabel('Presión PSI')
grid