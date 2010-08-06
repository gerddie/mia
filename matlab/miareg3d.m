% MIAREG2D calculate a vector field describing non-rigid registration of image src to image ref
% src: Source image
% ref: reference image
% method: description
% soriter: number of iterations for solving internal system of linear equations
% cost:   cost function 
% epsilon: breaking condition for accuracy 
% startsize: multigrid start size 
% niter: maximum number of outer iterations 
% returns: floating point deformation field

function field = miareg3d(src, ref, method, soriter, cost, epsilon, startsize, niter)

  if ~isreal(src) 
    throw MException('miareg3d', 'Source image is of unsupported complex non-real type')
  end

  if ~isreal(ref) 
    throw MException('miareg3d', 'Reference image is of unsupported complex non-real type')
  end

  if method == 'fluid'
    kernel=sprintf('naviera:iter=%d', int32(soriter))
    field = miamex('reg3d', src, ref,  kernel, 'fluid', cost, epsilon, startsize, niter)
    return    
  end

  if method == 'elastic'
    kernel=sprintf('naviera:maxiter=%d', int32(soriter))
    field = miamex('reg3d', src, ref, kernel, 'direct', cost, epsilon, startsize, niter)
    return    
  end

  throw MException('miareg3d', 'unknown registration method "%s" called', method)

end
