clc; clear all;

%% Testing coarse and fine freq comp

fs = .5e6;
M = 4;
fftOrder = 2^8;

cd1 = comm.ConstellationDiagram('Title', 'Before CFC and FFC');
cd2 = comm.ConstellationDiagram('Title', 'After CFC and FFC');
cd3 = comm.ConstellationDiagram('Title', 'After CFC/timing');
%data
data = randi([0 1], 1e5, 1);
MQAM = 4;
numsyms = length(data)/MQAM;
modulatedData = qammod( ...
    data, ...
    MQAM, ...
    'InputType', ...
    'bit', ...
    'UnitAveragePower', ...
    true);

carr_synch_damp = 1;
carr_synch_BW = 0.05;

% carrier_synch = comm.CarrierSynchronizer( ...
%     "SamplesPerSymbol", 1, ...
%     "Modulation", 'QAM', ...
%     "DampingFactor", carr_synch_damp, ...
%     "NormalizedLoopBandwidth", carr_synch_BW);  % try 0.01..0.05);


%TX symbols
%making filts
sps = 4;
beta = 0.7;

%Parameters for Loop filter
%to start, Loop BW is chosen as .1
LoopBW = 0.005;
zeta = 0.9;

%channel
span = 8;
srrcfilt = rcosdesign(beta, span, sps, "sqrt");

txFiltSignal = upfirdn(modulatedData,srrcfilt,sps,1);
%Create Simulated Channel with large frequency offset

freqOffset = comm.PhaseFrequencyOffset( ...
    FrequencyOffset=10000, ...
    SampleRate=fs*sps);
offsetsig=freqOffset(txFiltSignal);


%Apply CFC 
[compsig, Offset]= CFC(offsetsig, fs*sps, 4, fftOrder);

%RX matched filter
rxFiltSignalComp = upfirdn(compsig, srrcfilt, ...
    1, sps);
rxFiltSignalUncomp = upfirdn(offsetsig, srrcfilt, ...
    1, sps);

% 
% rxFiltSignalComp = rxFiltSignalComp(1+span*sps/2 : end-span*sps/2);
% rxFiltSignalUncomp = rxFiltSignalUncomp(1+span*sps/2 : end-span*sps/2);

rxFiltSignalComp = rxFiltSignalComp(1+span : end-span);
rxFiltSignalUncomp = rxFiltSignalUncomp(1+span : end-span);

totComp = FineFreqSync(rxFiltSignalComp,MQAM,carr_synch_BW, carr_synch_damp, sps);
% totUncomp = FineFreqSync(rxFiltSignalComp,MQAM,carr_synch_BW, carr_synch_damp, sps);
totUncomp = rxFiltSignalUncomp;

cd1(totUncomp);
cd2(totComp);

plot(linspace(1, length(Offset), length(Offset)), Offset);





%% Course Frequency Compensation
% Define resolution using fftOrder, M is]modulation order and fs
%fft order should be base 2
%This shit pretty slow I think it would be faster to
%use a hilbert transform
function [FCompSig, Offset] = CFC(sig, fs, M, fftOrder)
    
    %preallocation
    % floor(length(sig)/fftOrder) must be greater than zero for this to
    % work
    estOffset = zeros(floor(length(sig)/fftOrder),1);
    index2Hz = fs / (M*fftOrder);
    k = 1;
    sig = sig(:);
    if isempty(sig) || isempty(fs) || isempty(M) || isempty(fftOrder)
        error('need more arguments for CFC function');
    end
    if (length(sig)<fftOrder)
        disp('FFT Order is too large for number of samples');
    end

    for est = 1: length(estOffset)
        timeIndex = (k:k+fftOrder-1)';
        %fft sample index pointer
        k = k + fftOrder;
        %Determine Freq Offset
        sigNoMod = sig(timeIndex).^M;
        % sigNoMod = sigNoMod - mean(sigNoMod);


        freqHist = abs(fft(sigNoMod));

        %most likely offset is the maximum
        [~,maxInd] = max(freqHist);
        offsetInd = maxInd -1;
        if offsetInd>fftOrder/2
            offsetInd = offsetInd - fftOrder;
        end
        estOffset(est) =  offsetInd * index2Hz;
        Offset(est) = estOffset(est);

    end
    %apply 
    n= (0:length(sig)-1)';
    meanOffset = mean(estOffset);
    FCompSig = sig .* exp(-1i*2*pi * meanOffset * n /fs);
end

%% Fine Frequency Comp 
%consists of 4 functional units in a closed loop:
%Based on costas loop structure.
%Phase error detector, Loop filter, Direct digital synthesizer,
%and phase rotator
%This really only works for 4 QAM btw, higher order modulations will
%Struggle

function [FFCSig, phErr] = FineFreqSync(sig, M, LoopBW, zeta, sps)
%sig: desired signal to FFS
%M: Modulation order, hopefully works on 4 QAM 
%LoopBW: loop filter bandwidth: defines pull in range
%zeta: Loop filter damping factor, also define pull in range
%sps: samples per symbol, pretty obvious

%Loop Filter:
%Type-II second-order PLL

LoopFilter = dsp.IIRFilter('Structure', 'Direct form II transposed',...
    'Numerator',[1 0], 'Denominator', [1 -1]);
Integrator = dsp.IIRFilter('Structure', 'Direct form II transposed',...
    'Numerator',[0 1], 'Denominator', [1 -1]);

%range estimates provided by textbook, basically performance metrics set
%by chosen Loop Filter parameters
%can set as an output if desired
NoramlizedPullInRange = min(1, 2*pi*sqrt(2)*zeta*LoopBW);
MaxFrequencyLockDelay = (4*NoramlizedPullInRange^2)/(LoopBW^3);
NormalizedFrequencyLockDelay = 32*zeta^2/LoopBW;
MaxPhaseLockDelay = 1.3 / LoopBW;

%Phase correction stuff
PhaseRecoveryLoopBandwidth = LoopBW * sps;
PhaseRecoveryGain = sps;
PhaseErrorDetectorGain = log2(M);
DigitalSynthesizerGain = -1;
theta = PhaseRecoveryLoopBandwidth /...
    ((zeta + 0.25/zeta)*sps);
delta = 1 + 2 * zeta *theta + theta * theta;

%Loop Filter Gains:
%G1 is proportional gain 
%G3 is integral gain

G1 = (4*zeta*theta/delta) / (sps*PhaseRecoveryGain);
G3 = ((4/sps)*theta^2/delta)/ (sps * PhaseRecoveryGain);
sig=sig(:);
%preallocate
output = zeros(length(sig), 1);
Phase = 0; previousSample = complex(0);
loopFiltOut = complex(0);

%Carrier Correction
LoopFilter.release(); Integrator.release();
for i = 1:length(sig)-1
    %Correct the next sample based on the phase of the previous,
    %Costas loop style

    %Phase Shift
    output(i) = sig(i+1) * exp(1i * Phase);

    %PED
    phErr = sign(real(previousSample)) .* imag(previousSample)...
        -sign(imag(previousSample)) .* real(previousSample);

    %Loop Filter
    loopFiltOut = step(LoopFilter, phErr*G3);

    %DDS
    DDSOut = step(Integrator, phErr*G1+loopFiltOut);
    Phase = DigitalSynthesizerGain * DDSOut;
    previousSample = output(i);

end

FFCSig = output(:);
phErr = phErr(:);


end


function [FFCSig, phErr] = FineFreqSyncFaster(sig, M, LoopBW, zeta, sps)
%sig: desired signal to FFS
%M: Modulation order, hopefully works on 4 QAM 
%LoopBW: loop filter bandwidth: defines pull in range
%zeta: Loop filter damping factor, also define pull in range
%sps: samples per symbol, pretty obvious

%Loop Filter:
%Type-II second-order PLL

loopFilterState = 0;
integratorState = 0;

%range estimates provided by textbook, basically performance metrics set
%by chosen Loop Filter parameters
%can set as an output if desired
NoramlizedPullInRange = min(1, 2*pi*sqrt(2)*zeta*LoopBW);
MaxFrequencyLockDelay = (4*NoramlizedPullInRange^2)/(LoopBW^3);
NormalizedFrequencyLockDelay = 32*zeta^2/LoopBW;
MaxPhaseLockDelay = 1.3 / LoopBW;

%Phase correction stuff
PhaseRecoveryLoopBandwidth = LoopBW * sps;
PhaseRecoveryGain = sps;
PhaseErrorDetectorGain = log2(M);
DigitalSynthesizerGain = -1;
theta = PhaseRecoveryLoopBandwidth /...
    ((zeta + 0.25/zeta)*sps);
delta = 1 + 2 * zeta *theta + theta * theta;

%Loop Filter Gains:
%G1 is proportional gain 
%G3 is integral gain

G1 = (4*zeta*theta/delta) / (sps*PhaseRecoveryGain);
G3 = ((4/sps)*theta^2/delta)/ (sps * PhaseRecoveryGain);
sig=sig(:);
%preallocate
output = zeros(length(sig), 1);
Phase = 0; previousSample = complex(0);
loopFiltOut = complex(0);

%Carrier Correction
LoopFilter.release(); Integrator.release();
for i = 1:length(sig)-1
    %Correct the next sample based on the phase of the previous,
    %Costas loop style

    %Phase rotator
    output(i) = sig(i+1) * exp(1i * Phase);

    %PED
    phErr = sign(real(previousSample)) .* imag(previousSample)...
        -sign(imag(previousSample)) .* real(previousSample);

    %Loop Filter
    loopFiltOut = loopFilterState + phErr*G3; 
    loopFilterState = loopFiltOut;

    %DDS
    DDSOut = integratorState + G1 * phErr;
    integratorState = DDSOut;
    Phase = DigitalSynthesizerGain * DDSOut;
    previousSample = output(i);

end

FFCSig = output(:);
phErr = phErr(:);


end