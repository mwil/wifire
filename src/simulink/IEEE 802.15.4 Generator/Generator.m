function [t, v] = Generator( what, symbs, channel )
%GENERATOR Generates a IEEE 802.15.4 compatible signal
%   what can be
%       'Preamble': ...
%       'SFD': ...
%       'SHR': Preamble + SFD
%       'frame': Send the frame given by the vector symbs
%
%   channel can be
%       0 (default): no modulation onto carrier
%       11-26: modulation onto carrier frequency Fc = 2405 + 5 (k - 11) MHz
%              with random phase.


function [i, q] = append(i1, q1, symbol)
    [i_tmp, q_tmp] = modulate_802_15_4(symbol);
    i = [i1; i_tmp];
    q = [q1; q_tmp];
end

% Handle missing arguments
if nargin < 2,
    symbs = 1;
end

if nargin < 3,
    channel = 0;
end

global Fs Tc spc

Tc = 1/2e6;             %=0.5us, chiprate = 2 Mchips/s
Fs = 100e6;               % sampling rate
spc = 10 * Fs * Tc * 8; % samples-per-chip
chips_per_symbol = 16;  % 32/2, splitted for I and Q

symbol_count = 0;

y_i = []; y_q = [];

if strcmpi(what, 'SHR') || strcmpi(what, 'Preamble') || strcmpi(what, 'frame'),
    [y_i1, y_q1] = modulate_802_15_4(0); 
    y_i1 = y_i1 * ones(1,8); y_q1 = y_q1 * ones(1,8); % 8x the symbol 0
    y_i1 = y_i1(:); y_q1 = y_q1(:);
    y_i = [y_i; y_i1]; y_q = [y_q; y_q1];
    symbol_count = symbol_count + 8;
end

if strcmpi(what, 'SHR') || strcmpi(what, 'SFD') || strcmpi(what, 'frame'),
    symbols = bin2symbols('11100101');
    [y_i1, y_q1] = modulate_802_15_4(symbols(1));
    [y_i2, y_q2] = modulate_802_15_4(symbols(2));
    y_i = [y_i; y_i1; y_i2]; y_q = [y_q; y_q1; y_q2];
    symbol_count = symbol_count + 2;
end

if strcmpi(what, 'zero'),
    [y_i1, y_q1] = modulate_802_15_4(0); 
    y_i = [y_i; y_i1]; y_q = [y_q; y_q1];
    symbol_count = symbol_count + 1;
end

if strcmpi(what, 'one'),
    [y_i1, y_q1] = modulate_802_15_4(1); 
    y_i = [y_i; y_i1]; y_q = [y_q; y_q1];
    symbol_count = symbol_count + 1;
end

if strcmpi(what, 'eight'),
    [y_i1, y_q1] = modulate_802_15_4(8); 
    y_i = [y_i; y_i1]; y_q = [y_q; y_q1];
    symbol_count = symbol_count + 1;
end

if strcmpi(what, 'frame'),
     % frame length (40 Symbols = 20 Bytes = 18h)
     [y_i, y_q] = append(y_i, y_q, bitand(length(symbs)/2,15));
     [y_i, y_q] = append(y_i, y_q, bitand(bitshift(length(symbs)/2,-4),15));
     symbol_count = symbol_count + 2;
     
     for i=1:length(symbs),
         [y_i, y_q] = append(y_i, y_q, symbs(i));
     end
     symbol_count = symbol_count + length(symbs);

%--This is a frame known to be correct:     
%     % frame length (48 Symbols = 24 Bytes = 18h)
%     [y_i, y_q] = append(y_i, y_q, 8);
%     [y_i, y_q] = append(y_i, y_q, 1);
%     symbol_count = symbol_count + 2;
%     % frame:
%     [y_i, y_q] = append(y_i, y_q, 1);
%     [y_i, y_q] = append(y_i, y_q, 4);
%     [y_i, y_q] = append(y_i, y_q, 8);
%     [y_i, y_q] = append(y_i, y_q, 8);
%     [y_i, y_q] = append(y_i, y_q, 6);
%     [y_i, y_q] = append(y_i, y_q, 2);
%     [y_i, y_q] = append(y_i, y_q, 2);
%     [y_i, y_q] = append(y_i, y_q, 2);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 3);
%     [y_i, y_q] = append(y_i, y_q, 6);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 15);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 4);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 6);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     % CRC
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 12);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     [y_i, y_q] = append(y_i, y_q, 0);
%     symbol_count = symbol_count + 48;
end

t = 0:1/Fs:symbol_count * chips_per_symbol * 2 * Tc + Tc - 1/Fs;
% QPSK -> OQPSK
y_i = [y_i; zeros(spc/2,1)];
y_q = [zeros(spc/2,1); y_q];
% Downconversion
y_i=y_i(1:length(y_i)/length(t):end);
y_q=y_q(1:length(y_q)/length(t):end);

if channel~=0,
    Fc = (2405 + 5 * (channel - 11)) * 2e2; % * 1e6 eigentlich
    phi = rand()*pi;
    global c1 c2
    c1 = sin(2*pi*Fc*t+phi);
    c2 = cos(2*pi*Fc*t+phi);
    v = c1.' .* y_i + c2.' .* y_q;
else
    v = y_i + y_q * 1i;
end
end

