%% mitschnitt in Matlabformat wandeln;
s = read_complex_binary('../data/mitschnitt_802.15.4_2.48G_4MS'); 
re = real(s);
im = imag(s);
save('matlab-file.mat','re','im'); 
%% analyse
clear;
load('matlab-file.mat');
%re = re(1e6:2e6);
%im = im(1e6:2e6);
signal = re + j*im;
%plot(re);
%xlabel('samples');
%ylabel('counts');
e = signal .* conj(signal);  
w = zeros(1,length(signal));
for i=100:length(signal)
    w(i) = sum(e(i-31:i));
end
figure;plot(w);
ylabel('summed up signal powers');
xlabel('sample');
sig = w > 0.02; % thresholds anpassen
jam = w > 5;    % 
%figure; subplot(2,1,1), plot(sig);
%subplot(2,1,2), plot(jam);
sig_ind = [];
jam_ind = [];
for i=2:length(sig)
    if sig(i) == 1 && sig(i-1) == 0
        sig_ind = [sig_ind i];
    end
    if jam(i) == 1 && jam(i-1) == 0
        jam_ind = [jam_ind i];
    end
end
auswahl = sig_ind(1);
ctr = 1;
if (length(sig_ind) > length(jam_ind))
  for i=2:length(sig_ind)
     if (sig_ind(i) > auswahl(ctr)+500000)
         auswahl = [auswahl sig_ind(i)];
         ctr = ctr + 1;
     end
  end
end
pre = 32*10* 2;
time = jam_ind-auswahl;
samples = time - pre % gibt die Reaktionszeit in samples aus
time = time ./ 4e6 *1e6;  
react = time - (pre/4e6 * 1e6) % gibt die Reaktionszeit in us aus
