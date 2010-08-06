FUNCTION nrreg2d
    src_img = READ_PNG('..\testdata\circle.png')
    ref_img = READ_PNG('..\testdata\c.png')
    model='naviera'
    timestep = 'fluid'
    cost='ssd'
    maxiter = 100
    eps = 0.001
    startsize = 16
    dummy = MIA_DEBUG_LEVEL('message')
    field = MIA_NRREG2D(src_img, ref_img, model, timestep, cost, eps, startsize, maxiter)

    defo = MIA_DEFORM2D(src_img, field, 'bspline3')
    RETURN, 1

END