#!/bin/tcsh 

set files=`find . -name "test-*" -a -type f`
mkdir -p ValgrindTests

set outdir=`pwd`/ValgrindTests

foreach f ( $files) 
	set path=$f:h 
	set command=$f:t
	pushd "$path" 
	echo "run $command" 
	/usr/bin/valgrind --leak-check=full "./$command" >& "${outdir}/valgrind-${command}.log" 
	popd 
end
