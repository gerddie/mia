#!/bin/sh 
if [ -e   /usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl ]; then 
    stylsheet=/usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl
elif  [ -e  /usr/share/xml/docbook/xsl-stylesheets/html/chunk.xsl ]; then 
    stylsheet=/usr/share/sgml/docbook/xsl-stylesheets/html/chunk.xsl
else 
   echo "dockbook stylesheet not found, did you install 'docbook-xsl'?"
   exit 1
fi 

xsltproc --param chunk.section.depth 3 \
    --param chunk.first.sections 1 \
    --stringparam html.stylesheet  progref.css \
    --output userref/ \
    $stylesheet progref.xml
