SET(filters3dNew 	    
  binarize 
  bandpass 
  convert 
  crop 
  downscale 
  growmask
  invert
  kmeans 
  label
  mask
  median 
  mlv 
  morphological 
  reorient 
  scale
  selectbig
  sepconv 
  )

PLUGINGROUP_WITH_TEST_AND_PREFIX(3d-filter "${filters3dNew}" 
  "${MIA3DLIBS}" "${PLUGIN_INSTALL_PATH}/filter/3dimage"
  )


