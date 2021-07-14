%% Calculos para espesor de Tanque de Autoclave 
%% Ing. JuDaPiEs
clc,clear
P=2000;%kPa
R=127;%mm
S=310e3;%kPa, para acero inoxidable 304
E=0.5;%
t=(P*R)/((S*E)-(0.6*P))%mm