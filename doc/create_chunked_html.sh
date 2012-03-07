#!/bin/sh 

if [ -e   /usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl ]; then 
    stylesheet=/usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl
elif  [ -e  /usr/share/xml/docbook/stylesheet/docbook-xsl/html/chunk.xsl ]; then
    stylesheet=/usr/share/xml/docbook/stylesheet/docbook-xsl/html/chunk.xsl 
else 
   echo "dockbook stylesheet not found, did you install 'docbook-xsl'?"
   echo "If you did then please look up the 'html/chunk.xsl', add an acoording"
   echo " test in 'doc/create_chunked_html.sh' and file a bug report at"
   echo "https://sourceforge.net/apps/trac/mia/ (thanks)"
   exit 1
fi 

xsltproc --param chunk.section.depth 3 \
    --param chunk.first.sections 1 \
    --stringparam html.stylesheet  progref.css \
    --output userref/ \
    $stylesheet progref.xml
