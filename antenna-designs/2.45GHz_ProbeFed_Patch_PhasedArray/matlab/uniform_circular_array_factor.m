%% Circular Aperture


Nx = 20; 
Ny = 20; 

d = 0.5;

w = ones(Nx, Ny);


pts = 1.001e3;



u = linspace(-1,1, pts);
v = linspace(-1,1, pts);


[uGrid, vGrid] = meshgrid(u,v); 


xi = (-(Nx-1)/2:(Nx-1)/2) * d;
yi = (-(Ny-1)/2:(Ny-1)/2) * d;

% cutout elements that aren't within 5lambda from origin
[ElGridx ElGridy] = meshgrid(xi, yi);

w(sqrt( (abs(ElGridx)+d/2).^2 + (abs(ElGridy) + d/2).^2)>5 ) = 0;


[~,AF_dB, ~] = compAF_UAplanar(pts, Nx, Ny, w,d);

AF_normdB = AF_dB - max(max(AF_dB));


%find elements that need to be cutout

ElNc = find(w == 0);
num_ElNc = size(ElNc, 1);

disp(num_ElNc); 

figure(1);
AF_normdB(AF_normdB < -60) = -60;
contour(uGrid, vGrid, AF_normdB, 100);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('Circular Array Pattern Contour');

figure(2);
c = AF_normdB;
c(c<=-60) = NaN;
mesh(uGrid, vGrid, c);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('3D Circular Array Pattern');

%2D Plane Cuts
%2D Plane cuts
figure(3);
u0 = find(u == 0);
v0 = find(v == 0); 

tiledlayout(1,2);
nexttile; 

theta = acos(u); 
rho = AF_normdB(u0, :);

h = polarplot(theta,rho+60, 'LineWidth',2, 'color','r');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor, \phi = 0^\circ');



nexttile;  % degrees for polar plot
rho = AF_normdB(:, v0);
h = polarplot(theta,rho+60, 'LineWidth',2, 'color','b');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor, \phi = 90^\circ');


%FUNCTIONS
function [AF, AF_dB, AFphase] = compAF_UAplanar(pts, Nx, Ny, w,d)

%w should have size mxm

%coordinate transform
u = linspace(-1,1, pts);
v = linspace(-1,1, pts);


[uGrid, vGrid] = meshgrid(u,v); 


xi = (-(Nx-1)/2:(Nx-1)/2) * d;
yi = (-(Ny-1)/2:(Ny-1)/2) * d;



AF = zeros(pts,pts);


for n = 1:Nx
    for m = 1:Ny

        AF = AF + w(n,m) *exp(1i * 2 * pi * (xi(n) * ...
            uGrid + yi(m) * vGrid ) );

    end 

end 


AF_dB = 20.* log10(abs(AF));
AFphase = angle(AF);

end
