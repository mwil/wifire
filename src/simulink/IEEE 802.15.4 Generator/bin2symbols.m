function symbols = bin2symbols( s )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
symbols = [];
for i=1:4:length(s),
    symbols = [symbols; bin2dec(fliplr(s(i:i+3)))];
end

end

