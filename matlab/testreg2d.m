%
% testing calling the 2D registration 
%
%

%
% let me see some output 
%
miamex('debug', 'message')

% create two random images 
src = rand(256,128) * 255;
ref = rand(256,128) * 255;

%
% do the registration 
%
field = miareg2d(src, ref, 'fluid', 20, 'ssd', 0.001, 16, 100)

deformed = miadeform2d(src, field)
