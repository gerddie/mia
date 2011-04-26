IF (FFTWF_FOUND AND WITH_FFTWF) 
  SET(filters2d_fftNew cst fft)
ENDIF (FFTWF_FOUND AND WITH_FFTWF) 

SET (NOTHING
  classmap
  distance
  frequency
  harmmean
  ianiso
  kuwahara
  labelmap
  lnfft
  midpoint
  regiongrow
  rgg

  variation
  watershed
  wmean
  ws
)

SET(filters2dNew_base 
  adaptmed
  admean
  aniso
  bandpass
  binarize
  convert
  crop
  downscale
  gradnorm
  invert
  kmeans
  label 
  mask
  median
  mlv
  morphological
  ngfnorm
  noise
  scale
  selectbig
  sepconv
  thresh
)

SET(filters2dNew ${filters2dNew_base} ${filters2d_fftNew})

PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-filter "${filters2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/filter/2dimage"
  )
