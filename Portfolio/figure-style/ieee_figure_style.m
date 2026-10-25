function ieee_figure_style(fig)
%IEEE_FIGURE_STYLE Apply a simple color style to an engineering figure.
% Call after plotting, then export the figure using the desired format.
if nargin < 1
    fig = gcf;
end
set(fig, 'Color', 'w', 'Units', 'inches', ...
    'Position', [1 1 3.5 2.8], 'PaperPositionMode', 'auto');
set(findall(fig, '-property', 'FontName'), 'FontName', 'Arial');
set(findall(fig, '-property', 'FontSize'), 'FontSize', 8);
set(findall(fig, '-property', 'FontWeight'), 'FontWeight', 'normal');
ax = findall(fig, 'Type', 'axes');
styles = {'-', '--', ':', '-.'};
palette = [0.000 0.447 0.741;
           0.850 0.325 0.098;
           0.929 0.694 0.125;
           0.494 0.184 0.556;
           0.466 0.674 0.188;
           0.301 0.745 0.933;
           0.635 0.078 0.184];
for k = 1:numel(ax)
    set(ax(k), 'Color', 'w', 'XColor', 'k', 'YColor', 'k', ...
        'LineWidth', 0.75, 'GridColor', [0.65 0.65 0.65], ...
        'GridAlpha', 1, 'MinorGridAlpha', 0.35);
    title(ax(k), '');
    lines = flipud(findall(ax(k), 'Type', 'line'));
    for m = 1:numel(lines)
        set(lines(m), 'Color', palette(mod(m-1, size(palette,1))+1,:), 'LineWidth', 1, ...
            'LineStyle', styles{mod(m-1, numel(styles))+1});
    end
end
colormap(fig, parula(256));
end
