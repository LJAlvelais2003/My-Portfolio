
clc;
clear all;


%params
a = 5; %in lambda
d = 0.5;
pts = 1.001e3;

SLL = 20;
nbar = 4;


%calc un  and gamma
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



%Plot pattern of continuous Taylor Aperture.

rho = linspace(1,a, pts);
p_cont = pi * rho/a;

pts2 = 501;


uCont = linspace(-1, 1, pts2);
vCont = linspace(-1, 1, pts2);

[uContGrid, vContGrid] = meshgrid(uCont, vCont);


U = 2*a*sqrt(uContGrid.^2 + vContGrid.^2);

g = @(p) Taylor_samp(gamma,un,nbar, p);

Fint = @(p) p.* g(p) .* besselj(0, U .* p);


F = integral(Fint, 0, pi, 'ArrayValued',true);

F_dB = 20 * log10(abs(F));

F_dBnorm = F_dB - max(F_dB(:));

F_dBnorm(F_dBnorm < -60)= -60;
%now to plot
c = F_dBnorm;
c(c<-60) = NaN;

figure(1);
clf;
mesh(uCont,vCont,c);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('3D Mesh Plot of Continuous  Circular Taylor Aperture Array Factor');

figure(2);
contour(uCont, vCont, c,100);
colorbar; 
clim([-60 0]);
xlabel('u');
ylabel('v');
title('Contour Plot of Continuous Circular Taylor Aperture Array Factor');


%2D plane cuts;



u0 = find(uCont == 0);
v0 = find(vCont == 0);


theta = acos(uCont);



figure(3);
tiledlayout(1,2);
nexttile;
rho = F_dBnorm (u0, :);
h = polarplot(theta,rho+60, 'LineWidth',2, 'color','r');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor of Continuous Circular Taylor Aperture, \phi = 0^\circ');



nexttile; 
rho = F_dBnorm (:, v0);
h = polarplot(theta,rho+60, 'LineWidth',2, 'color','b');
haxes = get(h, 'Parent');
haxes.RTickLabel = {'-60','-40','-20','0'};
title('Normalized Array Factor of Continuous Circlar Taylor Aperture, \phi = 90^\circ');








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