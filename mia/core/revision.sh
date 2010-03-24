#!/bin/bash
# only use this if we are in a git repository

[ -e ~/.profile ] && . ~/.profile

if [ -d $1/.git ]; then
    rev=`git describe`
    val="#define LIBMIA_REVISION "\"$rev\"
    echo $val >revision.hh.new
    if [ -e revision.hh ] ; then 
       if diff revision.hh.new revision.hh >/dev/null 2>&1 ; then 
	   rm -f revision_info.hh.new
       else
	   mv revision.hh.new revision.hh
       fi
    else 
	   mv revision.hh.new revision.hh
    fi 
elif [ ! -f $1/revision.hh ] ; then 
    echo "WARNING: Unable to obtain revision info from git tree"
    echo "WARNING: Either you're not using a working copy of the git version"
    echo "WARNING: or the git  client is not available"
    echo "WARNING: As a workaround you may create a 'revision.hh' by hand in the <source root>/mia/core directory"
    echo "WARNING: with the line '#define LIBMIA_REVISION \"unknown\"'"
    echo "WARNING: otherwise build will fail with 'revision.cc: revision.hh: No such file or directory'"
fi


