%% Datos AutoClave Acondesa.
%% Fecha: 3 de Julio de 2013
%% Primeara toma de Datos 
clc,clear
Datos = xlsread('Datos_Autoclave_Acondesa.xlsx');
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

%% Segunda toma de Datos
clc,clear
Datos = xlsread('Datos_Autoclave_Acondesa.xlsx',2);
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