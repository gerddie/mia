#!/bin/tcsh 


set files=`find . -name "test-*" -a -type f`
mkdir -p ValgrindTests

set outdir=`pwd`/ValgrindTests

foreach f ( $files) 
    set path=$f:h 
    set command=$f:t
    pushd "$path" 
    if ( ! -f "${outdir}/z-valgrind-${command}.log" ) then 
	echo "run $command" 
	set logfile="${outdir}/valgrind-${command}.log" 
	/usr/bin/valgrind --leak-check=full "./$command" >& "$logfile"
	/bin/grep "definitely lost: 0 " "$logfile" >/dev/null
	set nomemleak=$? 
	/bin/grep "All heap blocks were freed -- no leaks are possible" "$logfile" >/dev/null
	set nomemleak2=$? 
	/bin/grep "Invalid" "$logfile" >/dev/null
	set nonmemaccess=$? 
	if ( ( $nomemleak2 == 0 || $nomemleak == 0 ) && $nonmemaccess == 1 )  then 
	    /bin/mv "$logfile" "${outdir}/z-valgrind-${command}.log"
	    echo $command seems to be clean 
	endif
	endif
    popd 
end
