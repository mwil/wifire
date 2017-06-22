function c = crc(in)
    r = uint16(0);
    for i=1:length(in),
        tmp = in(i);
        for j = 1:4,
            if bitxor(bitand(r,1),bitand(tmp,1)),
                r = bitxor(bitshift(r,-1), hex2dec('8408'));
            else
                r = bitshift(r,-1);
            end
            tmp = bitshift(tmp,-1);
        end
    end
    c = [bitand(r,15) bitand(bitshift(r,-4),15) bitand(bitshift(r,-8),15) bitand(bitshift(r,-12),15)];
end
