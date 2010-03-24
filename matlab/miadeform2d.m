%
% src:         input image 
% deformation: deformation field
% returns:     deformed input image

function image = deform2d(src, deformation)
	image = miamex('deform2d', src, deformation, 'bspline3')
end


