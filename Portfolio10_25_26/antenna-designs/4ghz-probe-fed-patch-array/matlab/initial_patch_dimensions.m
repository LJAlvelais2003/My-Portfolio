% Initial patch dimension estimates from the design report.
% Only W and L were used for the probe-fed patch.
clear; 
clc; 

f_GHz = input('Resonant frequency (GHz): '); 
er = input('Relative permittivity: '); 
h_mm = input('Substrate height (mm): '); 

c = 3e8; 
f = f_GHz*1e9; 
h = h_mm*1e-3; 
Z0 = 50; 

% Patch dimensions 
W = c/(2*f)*sqrt(2/(er + 1)); 
er_eff = (er + 1)/2 + (er - 1)/2*(1 + 12*h/W)^(-0.5); 

dL = 0.412*h*((er_eff + 0.3)*(W/h + 0.264))/... 
    ((er_eff - 0.258)*(W/h + 0.8)); 

L_eff = c/(2*f*sqrt(er_eff)); 
L = L_eff - 2*dL; 

% Edge impedance and bandwidth 
Z_edge = 90*(er^2/(er - 1))*(L/W)^2; 
BW = f^2*3.77*((er - 1)/er^2)*(W*h/(c*L)); 

% 50-ohm microstrip feed width 
A = Z0/60*sqrt((er + 1)/2) + ... 
    (er - 1)/(er + 1)*(0.23 + 0.11/er); 
W_h = 8*exp(A)/(exp(2*A) - 2); 

if W_h > 2 
    B = 60*pi^2/(Z0*sqrt(er)); 
    W_h = 2/pi*(B - 1 - log(2*B - 1) + ... 
        (er - 1)/(2*er)*(log(B - 1) + 0.39 - 0.61/er)); 
end 

W0 = W_h*h; 

% Inset dimensions 
D = L/pi*acos((Z0/Z_edge)^(1/4)); 
S = 1.5*W0; 
notch_width = W0 + 2*S; 

fprintf('\nInset-fed patch dimensions\n'); 
fprintf('---------------------------\n'); 
fprintf('Patch width, W:          %.4f mm\n', W*1e3); 
fprintf('Patch length, L:         %.4f mm\n', L*1e3); 
fprintf('Feedline width, W0:      %.4f mm\n', W0*1e3); 
fprintf('Inset depth, D:          %.4f mm\n', D*1e3); 
fprintf('Inset side gap, S:       %.4f mm\n', S*1e3); 
fprintf('Total notch width:       %.4f mm\n', notch_width*1e3); 
fprintf('Effective permittivity:  %.4f\n', er_eff); 


fprintf('Patch edge impedance:    %.2f ohms\n', Z_edge); 
fprintf('Estimated bandwidth:     %.2f MHz\n', BW/1e6); 

fprintf('\nS is an initial value and should be tuned in an EM simulation.\n');
