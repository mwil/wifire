function s = shape(chip, spc)
% The pulse shaping.

t = 0:1/spc:1-1/spc;
if chip == 1,
    s = sin(pi*t);
else
    s= -sin(pi*t);
end
end