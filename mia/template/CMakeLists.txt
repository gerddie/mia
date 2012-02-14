SET(MIA_TEMPLATES
bandpass.cxx
bandpass.hh
binarize.cxx
binarize.hh
convert.cxx
convert.hh
dimtrait.hh
divcurl.cxx
divcurl.hh
filter_chain.hh
fullcost.cxx
fullcost.hh
invert.cxx
invert.hh
lsd.hh
mi.hh
multicost.cxx
multicost.hh
nonrigidregister.cxx
nonrigidregister.hh
normalize.hh
seededwatershed.hh
similarity_profile.cxx
similarity_profile.hh
ssd.hh
transformfactory.cxx
transformfactory.hh
watershed.hh)

INSTALL(FILES ${MIA_TEMPLATES} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/template)

