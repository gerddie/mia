lib_LTLIBRARIES = libgsl++.la 

libpwpdf_la_SOURCES = vector.cc

INCLUDES=$(GSL_CFLAGS) -I$(top_srcdir)

libpwpdf_la_LDFLAGS= -version-info $(LT_VERSION_INFO) -export-dynamic
libpwpdf_la_LIBADD = $(LDADD) $(NFFT_LIBS) -lm

pwpdfdir=$(includedir)/gsl++
pwpdf_HEADERS = \
	vector.hh
	vector_template.hh



EXTRA_DIST=vector_template.cxx

