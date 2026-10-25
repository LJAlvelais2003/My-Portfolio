%% timing synchronization 
clc; clear;

%% random qam data for testing
cd = comm.ConstellationDiagram('Title', 'Before Timing Synchronization');
cd2 = comm.ConstellationDiagram('Title', 'After Timing Synchronization');
data = randi([0 1], 1e3, 1);
MQAM = 16;
modulatedData = qammod( ...
    data, ...
    MQAM, ...
    'InputType', ...
    'bit', ...
    'UnitAveragePower', ...
    true);
cd(modulatedData);
%TX symbols
%making filts
sps = 64;
beta = 0.7;

%Parameters for Loop filter
%to start, Loop BW is chosen as .1
LoopBW = 0.01;
zeta = 0.7;

%This parameter I just chose randomly.
span = 8;
srrcfilt = rcosdesign(beta, span, sps, "sqrt");
txFiltSignal = upfirdn(modulatedData,srrcfilt,sps,1);

%Create Simulated Channel with timing offset
%tau is timing offset
tau = 10;
VFD = dsp.VariableFractionalDelay;
offset_sig = VFD(txFiltSignal, tau);
% %phase delay
% offset_sig = offset_sig .* exp(1i * 3*pi / 4);
cd(offset_sig);


rxFiltSignal = upfirdn(offset_sig, srrcfilt, ...
    1, sps);
%remove delay
rxFiltSignal = rxFiltSignal(1+span : end-span);
%call timing sync function

for i = 1:10000;
    LoopBW = 0.000015 *i;
    SynchedSamples = TimingSync(sps, rxFiltSignal, LoopBW, zeta);
    disp(LoopBW);
    cd2(SynchedSamples);

end





%% Timing offset correction 
%Each block of the timing correction will be implemented with 
%Actual Timing synchronizer
function SynchedSamples = TimingSync(sps, samples, LoopBW, zeta)
%preallocate before feedback takes place

%for interpolator
interp_sz = 3;
int_state = zeros(interp_sz, 1);
yint = zeros(interp_sz, 1);
samples = samples(:);
 

%for TED
TEDBuffer = zeros(sps, 1);
Trigger = 0;
TriggerHistoryTED = 0;

%for Loop Filter
LoopPreviousInput = 0;
LoopFiltState = 0;

%for Interpolator Controller
g = 0;
Counter = 0;
TriggerHistoryContr = 0;
mu = 0;


for i = 1:length(samples)
    %Interpolator
    [yint(i), int_state] = Interpolator(samples(i), mu, int_state);
    %TED
    [e, TEDBuffer, TriggerHistoryTED] = zcTED(yint(i), ...
        Trigger, ...
        sps, ...
        TEDBuffer, ...
        TriggerHistoryTED);
    %Loop Filter
    %Calculate Loop Gains
    [G1, G2] = LoopGains(sps, LoopBW, zeta);
    [g, LoopFiltState, LoopPreviousInput] = LoopFilt(e, ...
    LoopPreviousInput, ...
    LoopFiltState,...
    G2, ...
    G1);

    %Interpolation Controller
    [mu, Trigger, Counter, TriggerHistoryContr, d] = InterpController(g, ...
    sps,...
    Counter,...
    TriggerHistoryContr);


end

%Return the interpolated samples with corrected timing offset
SynchedSamples  = yint(:);

end



%% Interpolator
% Define interpolator coefficients
function [yint, int_state] = Interpolator(ynew, mu, int_state)

alpha = 0.5;
InterpFilterCoeff = ...
[
    0,      0,          1,          0;              % Constant
   -alpha,  1+alpha,   -(1-alpha), -alpha;          % Linear
    alpha, -alpha,     -alpha,      alpha           % Quadratic
];


% Filter input data
ySeq = [ynew; int_state(:)];    % Update delay line

% Produce filter output
yint = sum((InterpFilterCoeff * ySeq) .* [1; mu; mu^2]);

int_state = ySeq(1:3);       % Save filter input data

end


%% TED
function [e, TEDBuffer, TriggerHistory] = zcTED(yint, ...
    Trigger, ...
    N, ...
    TEDBuffer, ...
    TriggerHistory)

%N is the sps
%depends on output of interpolator controller
%TEDBuffer holds recent interpolated samples
%...for midpoint calculation

%handle case for where TEDBuffer = emptyset
if isempty(TEDBuffer)
    TEDBuffer = zeros(N,1);
else
    TEDBuffer = TEDBuffer(:);
end

%handle case for TriggerHistory = emptyset
if isempty(TriggerHistory)
    TriggerHistory = zeros(N, 1);
else
    TriggerHistory = TriggerHistory(:);
end

% ZC-TED calculation occurs on a strobe
if Trigger && all(~TriggerHistory(2:end))

    % Calculate the midsample point for odd or even samples per symbol
    %buffer moves from right to left for some reason
    t1 = TEDBuffer(floor(end/2) + 1 - rem(N,2));
    t2 = TEDBuffer(floor(end/2) + 1);
    midSample = (t1 + t2)/2;

    %ZC
    e = real(midSample) * (sign(real(TEDBuffer(1))) - ...
        sign(real(yint))) + ...
        imag(midSample) * (sign(imag(TEDBuffer(1))) - ...
        sign(imag(yint)));

    %Gardner
    % e = real(midSample) *( real (TEDBuffer(1)) - real(yint)) +...
    % imag(midSample) * (imag(TEDBuffer(1))-imag(yint));

else
    e = 0;
end

% Update TED buffer to manage symbol stuffs
switch sum([TriggerHistory(2:end); Trigger])

    case 0
        % No update required

    case 1
        % Shift TED buffer regularly if ONE trigger across N samples
        TEDBuffer = [TEDBuffer(2:end); yint];

    otherwise  % > 1
        % Stuff a missing sample if TWO triggers across N samples
        TEDBuffer = [TEDBuffer(3:end); 0; yint];
end

TriggerHistory = [TriggerHistory(2:end); Trigger];

end

%% Loop Filter

%first determine gains of loop filter
function [G1, G2] = LoopGains(N, LoopBW, zeta)
%G1 is proportional gain 
%G2 is integrator gain
%N is samples per symbol
theta = LoopBW / (N * (zeta+0.25/zeta));
delta = 1+2*zeta*theta+theta^2;

G1 = (4*zeta*theta/delta) / N;
G2 = (4*theta^2 / delta) / N;
end


function [g, LoopFiltState, LoopPreviousInput] = LoopFilt(e, ...
    LoopPreviousInput, ...
    LoopFiltState,...
    IntegratorGain, ...
    ProportionalGain)

%zeta is damping factor
%M is modulation order
%handle case for empty inputs

if isempty(LoopFiltState)
    LoopFiltState = 0;
end

if isempty(LoopPreviousInput)
    LoopPreviousInput = 0;
end

% Loop filter
loopFiltOut = LoopPreviousInput + LoopFiltState;
g = e*ProportionalGain + loopFiltOut;   % Filter error signal
LoopFiltState = loopFiltOut;
LoopPreviousInput = e*IntegratorGain;
end


%% Interpolator Controller
function [mu, Trigger, Counter, TriggerHistory, d] = InterpController( ...
    g, ...
    N,...
    Counter,...
    TriggerHistory)
% Interpolation Controller with modulo-1 counter
if isempty(Counter)
    Counter = 0;
end
if isempty(TriggerHistory)
    TriggerHistory = zeros(N, 1);
    TriggerHistory = TriggerHistory(:);
end
d = g + 1/N;
Trigger = (Counter < d);    % Check if a trigger condition
TriggerHistory = [TriggerHistory(2:end); Trigger];
if Trigger    % Update mu if a trigger
    mu = Counter / d;
else
    mu = 0;
end
Counter = mod(Counter - d, 1);   % Update counter
end