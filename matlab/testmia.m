%
%first set the test status of the library
%

%miamex('set_test')

%
% run the echo test 
%

a = int32(2)
b = miamex('echo', a)
if b ~= a
	disp('echo test failed')
end

%
% call an unknown funtion  - should fail, but how do I check this? 
%
miamex('fail')


%
% now call a gaussian filter 
% 
input2d = [0 0 0; 0 1 0; 0 0 0]
test_output2d = [0.0625 0.125 0.0625; 0.125 0.25 0.125; 0.0625 0.125 0.0625]

output2d = miamex('filter2d', 'gauss:w=1', input2d)

if output2d ~= test_output2d
         disp('filter2d test failed')
end





