%% Circular Aperture
clc;
clear all;


Nx = 20; 
Ny = 20; 
a = 5; %in lambda
d = 0.5;

SLL = 20;
nbar = 4;




pts = 1.001e3;


u = linspace(-1,1, pts);
v = linspace(-1,1, pts);


[uGrid, vGrid] = meshgrid(u,v); 


xi = (-(Nx-1)/2:(Nx-1)/2) * d;
yi = (-(Ny-1)/2:(Ny-1)/2) * d;

% cutout elements that aren't within 5lambda from origin
[ElGridx ElGridy] = meshgrid(xi, yi);

w(sqrt( (abs(ElGridx)+d/2).^2 + (abs(ElGridy) + d/2).^2)>5 ) = 0;

Elx_Nc = ElGridx( sqrt((abs(ElGridx)+d/2).^2 + (abs(ElGridy) + d/2).^2)>5);
Ely_Nc = ElGridy( sqrt(abs((ElGridx+d/2)).^2 + abs((ElGridy + d/2)).^2)>5);


num_ElNc = numel(Elx_Nc);


%compute continuous and discrete aperture distribution


j1 = @(x) besselj(1, x);
mu = zeros(1, nbar);
        
for i = 1:nbar
    interval = [i*pi, (i + 0.5)*pi];
    mu(i) = fzero(j1, interval);
end

gamma = mu/pi;
A = acosh(10^(SLL/20))/pi;

% New null locations in lecture notation
n = 1:nbar-1;

un = gamma(nbar) * sqrt( ...
    (A^2 + (n - 0.5).^2) / ...
    (A^2 + (nbar - 0.5)^2));


%discrete distr

[~,w] = circ_TDw(a, gamma, un,nbar, SLL, xi, yi,d);

%cont distr.
xFine = linspace(-a, a, 401);
[Xfine, Yfine] = meshgrid(xFine, xFine);

rhoFine = hypot(Xfine, Yfine);
inside = rhoFine <= a;

G = zeros(size(rhoFine));
G(inside) = Taylor_samp(gamma, un, nbar, ...
    pi*rhoFine(inside)/a);

G = G/max(G(:));


%plot distr.

%disc
figure(1);
tiledlayout(1,2);
nexttile;
imagesc(xi, yi,w);
axis image;
axis xy;
clim([0 1]);
colorbar;
xlabel('x/\lambda');
ylabel('y/\lambda');
title('Discrete Taylor Aperture Distribution');


%cont 
nexttile;

imagesc(xFine,xFine, G);
axis image;
axis xy;
clim([0 1]);
colorbar;
xlabel('x/\lambda');
ylabel('y/\lambda');
title('Continuos Taylor Aperture Distribution');

%compute and plot array factor
%dis


[~,AF_dB, ~] = compAF_UAplanar(pts, Nx, Ny, w,d);

AF_normdB = AF_dB - max(max(AF_dB));

figure(2);
AF_normdB(AF_normdB < -60) = -60;
contour(uGrid, vGrid, AF_normdB, 100);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('Contour Plot of Sampled Circular Taylor Aperture Array Factor');

figure(3);
c = AF_normdB;
c(c<=-60) = NaN;
mesh(uGrid, vGrid, c);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('3D Mesh Plot of Sampled Circular Aperture Array Factor');

%2D Plane Cuts
%2D Plane cuts
figure(4);
u0 = find(u == 0);
v0 = find(v == 0); 

tiledlayout(1,2);
nexttile; 

theta = acos(u); 
rho = AF_normdB(u0, :);

h = polarplot(theta,rho+60, 'LineWidth',2, 'color','r');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor of Sampled Circular Taylor Aperture, \phi = 0^\circ');



nexttile;  % degrees for polar plot
rho = AF_normdB(:, v0);
h = polarplot(theta,rho+60, 'LineWidth',2, 'color','b');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor of Sampled Circular Taylor Aperture, \phi = 90^\circ');













%FUNCTIONS



function [g, w] = circ_TDw(a,gamma,un, nbar, SLL, xi, yi,d)

    pts = 1e4;
    rho = linspace(0, a, pts);
    p = pi*rho/a;



    

    [xGrid, yGrid] = meshgrid(xi, yi);


 

    pmn = sqrt( (xGrid).^2 + (yGrid).^2);
    
    %continuous aperture.
    g_rho = Taylor_samp(gamma,un,nbar, p);
    g = g_rho./max(g_rho(:));
    %map it to 2D circle

    %discrete rectangular grid
    w = zeros(size(xGrid));
    p_dis = pmn ./ a * pi;

    w_dis = Taylor_samp(gamma, un, nbar, p_dis);
    %remove elements outside of aperture
    w_dis(sqrt( (abs(xGrid)+d/2).^2 + (abs(yGrid) + d/2).^2)>5 ) = 0;

    w = w_dis./max(w_dis(:));

    

end


function [w] = Taylor_samp(gamma,un, nbar, p)
    
    %p <= pi


    %S0 = 1/2
    sum = 0.5 * ones(size(p));

    for i = 1:nbar-1
    
        num = 1;
        den = 1;
    
        for m = 1:nbar-1
    
            num = num * (1 - (gamma(i)/un(m))^2);
    
            if i ~= m
                den = den * (1 - (gamma(i)/gamma(m))^2);
            end
    
        end
    
        % Finite limit at each original positive null
        S = -0.5 * besselj(0, pi*gamma(i)) * num/den;
    
        frac = S / besselj(0, pi*gamma(i))^2;
    
        sum = sum + frac * besselj(0, gamma(i)*p);
    
    end
    
    w = 2/pi^2 * sum;
    

end




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