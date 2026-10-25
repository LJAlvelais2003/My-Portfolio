%%HW 5
 
%Part 1 - Square aperture

%400 elements, 20x20, side length of 10 lambda (.5 lambda interelement
%spacing)

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




[AF, AF_dB, AFphase] = compAF_UAplanar(pts, Nx, Ny, w, d);


AF_normdB = AF_dB-max(max(AF_dB));


AF_normdB(AF_normdB<-60) = -60;
figure(1);
clf; 

%contour plot
contour(uGrid, vGrid, AF_normdB, 100);
colorbar;
clim([-40 0]);
xlabel('u');
ylabel('v');
title('Normalized Array Factor Contour');


%3D plot

c = AF_normdB;
c(c <= -60) = NaN;

figure(2);
mesh(uGrid, vGrid, c);
colorbar;
clim([-60 0]);
xlabel('u');
ylabel('v');
title('Normalized 3D Array Factor');

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



nexttile;   % degrees for polar plot
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
 