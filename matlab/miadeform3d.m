%
% src:         input image 
% deformation: deformation field
% returns:     deformed input image

function image = deform3d(src, deformation)
	miamex('deform3d', src, deformation, 'bspline3')
end

